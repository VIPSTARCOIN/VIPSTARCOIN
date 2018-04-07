// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2009-2016 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "pow.h"

#include "arith_uint256.h"
#include "chain.h"
#include "primitives/block.h"
#include "uint256.h"
#include "util.h"

// ppcoin: find last block index up to pindex
const CBlockIndex* GetLastBlockIndex(const CBlockIndex* pindex, bool fProofOfStake)
{
    //CBlockIndex will be updated with information about the proof type later
    while (pindex && pindex->pprev && (pindex->IsProofOfStake() != fProofOfStake))
        pindex = pindex->pprev;
    return pindex;
}

inline arith_uint256 GetLimit(const Consensus::Params& params, bool fProofOfStake)
{
    return fProofOfStake ? UintToArith256(params.posLimit) : UintToArith256(params.powLimit);
}

unsigned int GetNextWorkRequired(const CBlockIndex* pindexLast, const CBlockHeader *pblock, const Consensus::Params& params, bool fProofOfStake)
{

    unsigned int nTargetLimit = GetLimit(params, fProofOfStake).GetCompact();

    // genesis block
    if (pindexLast == NULL)
        return nTargetLimit;

    // first block
    const CBlockIndex* pindexPrev = GetLastBlockIndex(pindexLast, fProofOfStake);
    if (pindexPrev->pprev == NULL)
        return nTargetLimit;

    // second block
    const CBlockIndex* pindexPrevPrev = GetLastBlockIndex(pindexPrev->pprev, fProofOfStake);
    if (pindexPrevPrev->pprev == NULL)
        return nTargetLimit;

    // Return min difficulty on regtest
    if (params.fPowAllowMinDifficultyBlocks)
        return nTargetLimit;

    return CalculateNextWorkRequired(pindexPrev, params, fProofOfStake);
}

/**
 * eHRC (enhanced Hash Rate Compensation)
 * Short, medium and long samples averaged together and compared against the target time span.
 * Adjust every block but limted to 9% change maximum.
 * Difficulty is calculated separately for PoW and PoS blocks in that PoW skips PoS blocks and vice versa.
 */
unsigned int CalculateNextWorkRequired(const CBlockIndex* pindexLast, const Consensus::Params& params, bool fProofOfStake)
{
    int nHeight = pindexLast->nHeight + 1;
    arith_uint256 nTargetLimit = GetLimit(params, fProofOfStake);
    int shortSample = 15;
    int mediumSample = 200;
    int longSample = 1000;
    int pindexFirstShortTime = 0;
    int pindexFirstMediumTime = 0;
    int nActualTimespan = 0;
    int nActualTimespanShort = 0;
    int nActualTimespanMedium = 0;
    int nActualTimespanLong = 0;

    // Make sure there's enough PoW or PoS blocks for eHRC long sample
    const CBlockIndex* pindexCheck = pindexLast;
    for (int i = 0; i <= longSample + 1;) {

        // Hit the start of the chain before finding enough blocks
        if (pindexCheck->pprev == NULL)
            return nTargetLimit.GetCompact();

        // Only increment if we have a block of the current type
        if (fProofOfStake) {
            if (pindexCheck->IsProofOfStake())
                i++;
        } else if (pindexCheck->IsProofOfWork()) {
            i++;
        }

        pindexCheck = pindexCheck->pprev;
    }

    const CBlockIndex* pindexFirstLong = pindexLast;

    if (nHeight <= params.nDiffAdjustChange) {
        for (int i = 0; pindexFirstLong && i < longSample; i++) {
            pindexFirstLong = pindexFirstLong->pprev;

            // If we have a block of the wrong type skip this iteration
            if (fProofOfStake) {
                if (pindexFirstLong->IsProofOfWork())
                    continue;
            } else if (pindexFirstLong->IsProofOfStake()) {
                continue;
            }

            if (i == shortSample - 1)
                pindexFirstShortTime = pindexFirstLong->GetBlockTime();

            if (i == mediumSample - 1)
                pindexFirstMediumTime = pindexFirstLong->GetBlockTime();
        }
    } else {
        for (int i = 0; pindexFirstLong && i < longSample;) {
            pindexFirstLong = pindexFirstLong->pprev;

            // If we have a block of the wrong type skip this iteration
            if (fProofOfStake) {
                if (pindexFirstLong->IsProofOfWork())
                    continue;
            } else if (pindexFirstLong->IsProofOfStake()) {
                continue;
            }

            if (i == shortSample - 1)
                pindexFirstShortTime = pindexFirstLong->GetBlockTime();

            if (i == mediumSample - 1)
                pindexFirstMediumTime = pindexFirstLong->GetBlockTime();

             i++;
        }
    }

    if (pindexLast->GetBlockTime() - pindexFirstShortTime != 0)
        nActualTimespanShort = (pindexLast->GetBlockTime() - pindexFirstShortTime) / shortSample;

    if (pindexLast->GetBlockTime() - pindexFirstMediumTime != 0)
        nActualTimespanMedium = (pindexLast->GetBlockTime() - pindexFirstMediumTime) / mediumSample;

    if (pindexLast->GetBlockTime() - pindexFirstLong->GetBlockTime() != 0)
        nActualTimespanLong = (pindexLast->GetBlockTime() - pindexFirstLong->GetBlockTime()) / longSample;

    int nActualTimespanSum = nActualTimespanShort + nActualTimespanMedium + nActualTimespanLong;

    if (nActualTimespanSum != 0)
        nActualTimespan = nActualTimespanSum / 3;

    if (pindexLast->nHeight >= params.nDiffDamping) {
        // Apply .25 damping
        nActualTimespan = nActualTimespan + (3 * params.nTargetTimespan);
        nActualTimespan /= 4;
    }

    // 9% difficulty limiter
    int nActualTimespanMax = params.nTargetTimespan * 494 / 453;
    int nActualTimespanMin = params.nTargetTimespan * 453 / 494;

    if(nActualTimespan < nActualTimespanMin)
        nActualTimespan = nActualTimespanMin;

    if(nActualTimespan > nActualTimespanMax)
        nActualTimespan = nActualTimespanMax;

    arith_uint256 bnNew;
    bnNew.SetCompact(pindexLast->nBits);
    bnNew *= nActualTimespan;
    bnNew /= params.nTargetTimespan;

    if (bnNew <= 0 || bnNew > nTargetLimit)
        bnNew = nTargetLimit;

    return bnNew.GetCompact();
}

bool CheckProofOfWork(uint256 hash, unsigned int nBits, const Consensus::Params& params, bool fProofOfStake)
{
    bool fNegative;
    bool fOverflow;
    arith_uint256 bnTarget;

    bnTarget.SetCompact(nBits, &fNegative, &fOverflow);

    // Check range
    if (fNegative || bnTarget == 0 || fOverflow || bnTarget > GetLimit(params, fProofOfStake))
        return false;

    // Check proof of work matches claimed amount
    if (UintToArith256(hash) > bnTarget)
        return false;

    return true;
}

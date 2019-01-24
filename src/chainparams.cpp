// Copyright (c) 2010 Satoshi Nakamoto
// Copyright (c) 2009-2016 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "chainparams.h"
#include "consensus/merkle.h"
#include "consensus/consensus.h"

#include "tinyformat.h"
#include "util.h"
#include "utilstrencodings.h"
#include "base58.h"

#include <assert.h>

#include <boost/assign/list_of.hpp>

#include "chainparamsseeds.h"

///////////////////////////////////////////// // qtum
#include <libdevcore/SHA3.h>
#include <libdevcore/RLP.h>
#include "arith_uint256.h"
/////////////////////////////////////////////

static CBlock CreateGenesisBlock(const char* pszTimestamp, const CScript& genesisOutputScript, uint32_t nTime, uint32_t nNonce, uint32_t nBits, int32_t nVersion, const CAmount& genesisReward)
{
    CMutableTransaction txNew;
    txNew.nVersion = 1;
    txNew.vin.resize(1);
    txNew.vout.resize(1);
    txNew.vin[0].scriptSig = CScript() << 00 << 488804799 << CScriptNum(4) << std::vector<unsigned char>((const unsigned char*)pszTimestamp, (const unsigned char*)pszTimestamp + strlen(pszTimestamp));
    txNew.vout[0].nValue = genesisReward;
    txNew.vout[0].scriptPubKey = genesisOutputScript;

    CBlock genesis;
    genesis.nTime    = nTime;
    genesis.nBits    = nBits;
    genesis.nNonce   = nNonce;
    genesis.nVersion = nVersion;
    genesis.vtx.push_back(MakeTransactionRef(std::move(txNew)));
    genesis.hashPrevBlock.SetNull();
    genesis.hashMerkleRoot = BlockMerkleRoot(genesis);
    genesis.hashStateRoot = uint256(h256Touint(dev::h256("e965ffd002cd6ad0e2dc402b8044de833e06b23127ea8c3d80aec91410771495")));
    genesis.hashUTXORoot = uint256(h256Touint(dev::sha3(dev::rlp("")))); // qtum
    return genesis;
}

/**
 * Build the genesis block. Note that the output of its generation
 * transaction cannot be spent since it did not originally exist in the
 * database.
 */
static CBlock CreateGenesisBlock(uint32_t nTime, uint32_t nNonce, uint32_t nBits, int32_t nVersion, const CAmount& genesisReward)
{
    const char* pszTimestamp = "vip quality on the world!!";
    const CScript genesisOutputScript = CScript() << ParseHex("766970207175616c697479206f6e2074686520776f726c642121766970207175616c697479206f6e2074686520776f726c642121766970207175616c697479206f") << OP_CHECKSIG;
    return CreateGenesisBlock(pszTimestamp, genesisOutputScript, nTime, nNonce, nBits, nVersion, genesisReward);
}

static void MineGenesis(CBlockHeader& genesisBlock, const uint256& powLimit, bool noProduction)
{
    if(noProduction)
        genesisBlock.nTime = std::time(0);
    genesisBlock.nNonce = 0;

    LogPrintf("NOTE: Genesis nTime = %u \n", genesisBlock.nTime);
    LogPrintf("WARN: Genesis nNonce (BLANK!) = %u \n", genesisBlock.nNonce);

    arith_uint256 besthash;
    memset(&besthash,0xFF,32);
    arith_uint256 hashTarget = UintToArith256(powLimit);
    LogPrintf("Target: %s\n", hashTarget.GetHex().c_str());
    arith_uint256 newhash = UintToArith256(genesisBlock.GetHash());
    while (newhash > hashTarget) {
        genesisBlock.nNonce++;
        if (genesisBlock.nNonce == 0) {
            LogPrintf("NONCE WRAPPED, incrementing time\n");
            ++genesisBlock.nTime;
        }
        // If nothing found after trying for a while, print status
        if ((genesisBlock.nNonce & 0xfff) == 0)
            LogPrintf("nonce %08X: hash = %s (target = %s)\n",
                   genesisBlock.nNonce, newhash.ToString().c_str(),
                   hashTarget.ToString().c_str());

        if(newhash < besthash) {
            besthash = newhash;
            LogPrintf("New best: %s\n", newhash.GetHex().c_str());
        }
        newhash = UintToArith256(genesisBlock.GetHash());
    }
    LogPrintf("Genesis nTime = %u \n", genesisBlock.nTime);
    LogPrintf("Genesis nNonce = %u \n", genesisBlock.nNonce);
    LogPrintf("Genesis nBits: %08x\n", genesisBlock.nBits);
    LogPrintf("Genesis Hash = %s\n", newhash.ToString().c_str());
    LogPrintf("Genesis hashStateRoot = %s\n", genesisBlock.hashStateRoot.ToString().c_str());
    LogPrintf("Genesis Hash Merkle Root = %s\n", genesisBlock.hashMerkleRoot.ToString().c_str());
}


/**
 * Main network
 */
/**
 * What makes a good checkpoint block?
 * + Is surrounded by blocks with reasonable timestamps
 *   (no blocks before with a timestamp after, none after with
 *    timestamp before)
 * + Contains no strange transactions
 */

class CMainParams : public CChainParams {
public:
    CMainParams() {
        strNetworkID = "main";
        consensus.nSubsidyHalvingInterval = 525600 ; // PoS Halving every 1 years
        consensus.powLimit = uint256S("0000ffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff");
        consensus.posLimit = uint256S("00000000ffffffffffffffffffffffffffffffffffffffffffffffffffffffff");
        consensus.checkpointPubKey = "714b98243f1e0f15567a4afec1c2b9b756fc3762e7c560dc11aa94c2014694876ab8719f624a101fd583e735f4bad5a2f956e1b86ec929ec20e80d98946b9644dc";
        consensus.vAlertPubKey = ParseHex("04f429354a48cec0b163f358ded4779ecd561a4b69688f6208ba5a202dbb9e6e3d24a0d8b9896b48db4c4dac256255aac46010f672c39206e2331f16b6d3875eae");
        consensus.nTargetTimespan = 120;
        consensus.fPowAllowMinDifficultyBlocks = false;
        consensus.fPowNoRetargeting = false;
        consensus.fPoSNoRetargeting = false;
        consensus.nRuleChangeActivationThreshold = 15120; // 75% of 2016
        consensus.nMinerConfirmationWindow = 20160;
        consensus.nDiffAdjustChange = 1000;
        consensus.nDiffDamping = 1000;
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].bit = 28;
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].nStartTime = 1199145601; // January 1, 2008
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].nTimeout = 1230767999; // December 31, 2008

        // Deployment of BIP68, BIP112, and BIP113.
        consensus.vDeployments[Consensus::DEPLOYMENT_CSV].bit = 0;
        consensus.vDeployments[Consensus::DEPLOYMENT_CSV].nStartTime = 0;
        consensus.vDeployments[Consensus::DEPLOYMENT_CSV].nTimeout = 999999999999ULL;

        // Deployment of SegWit (BIP141, BIP143, and BIP147)
        consensus.vDeployments[Consensus::DEPLOYMENT_SEGWIT].bit = 1;
        consensus.vDeployments[Consensus::DEPLOYMENT_SEGWIT].nStartTime = 0;
        consensus.vDeployments[Consensus::DEPLOYMENT_SEGWIT].nTimeout = 999999999999ULL;

        // The best chain should have at least this much work.
        consensus.nMinimumChainWork = uint256S("0x00000000000000000000000000000000000000000003f3a1e45d1da0b713be57"); // 413775

        // By default assume that the signatures in ancestors of this block are valid.
        consensus.defaultAssumeValid = uint256S("0x00000000000a318bfa940ffb8a4597b3fb44734065110cc0de1ee84b8265bb72"); // 413775

        /**
         * The message start string is designed to be unlikely to occur in normal data.
         * The characters are rarely used upper ASCII, not valid as UTF-8, and produce
         * a large 32-bit integer with any alignment.
         */
        pchMessageStart[0] = 0x01;
        pchMessageStart[1] = 0x2C;
        pchMessageStart[2] = 0xE7;
        pchMessageStart[3] = 0xB5;
        nDefaultPort = 31915;
        nPruneAfterHeight = 100000;
        startNewChain = false;

        genesis = CreateGenesisBlock(1522223204, 135269, 0x1f00ffff, 1, 1 * COIN);

        if (startNewChain)
            MineGenesis(genesis, consensus.powLimit, false);

        consensus.hashGenesisBlock = genesis.GetHash();
        assert(consensus.hashGenesisBlock == uint256S("0x0000d068e1d30f79fb64446137106be9c6ee69a6a722295c131506b1ee09b77c"));
        assert(genesis.hashMerkleRoot == uint256S("0x3821b1e4634626248389576cae0c9310c365a46ace444a6e0bfb9e1f421bb625"));

        // Note that of those with the service bits flag, most only support a subset of possible options
        vSeeds.push_back(CDNSSeedData("dnsseed.vipstarcoin.jp", "dnsseed.vipstarcoin.jp"));

        base58Prefixes[PUBKEY_ADDRESS] = std::vector<unsigned char>(1,70);
        base58Prefixes[SCRIPT_ADDRESS] = std::vector<unsigned char>(1,50);
        base58Prefixes[SECRET_KEY] =     std::vector<unsigned char>(1,128);
        base58Prefixes[EXT_PUBLIC_KEY] = boost::assign::list_of(0x04)(0x88)(0xB2)(0x1E).convert_to_container<std::vector<unsigned char> >();
        base58Prefixes[EXT_SECRET_KEY] = boost::assign::list_of(0x04)(0x88)(0xAD)(0xE4).convert_to_container<std::vector<unsigned char> >();

        vFixedSeeds = std::vector<SeedSpec6>(pnSeed6_main, pnSeed6_main + ARRAYLEN(pnSeed6_main));

        fMiningRequiresPeers = true;
        fDefaultConsistencyChecks = false;
        fRequireStandard = true;
        fMineBlocksOnDemand = false;

        checkpointData = (CCheckpointData) {
            boost::assign::map_list_of
            ( 0, uint256S("0000d068e1d30f79fb64446137106be9c6ee69a6a722295c131506b1ee09b77c"))
            ( 1000, uint256S("0000966ea0fed775d527fb31902270f01cafdfe53403be884080e2203bd43c74"))
            ( 244000, uint256S("0000000000261d023630c17c99b686bc251eca6e044c75191d83d7f35cff530d"))
            ( 400000, uint256S("00000000005845c3019379551f73d16a71ebbf4307cae40b16caa3d36440462f"))
        };

        chainTxData = ChainTxData{
            // Data from rpc: getchaintxstats (03df34411f2a5e4c057b1afea599a26370556c21328d3435505a2ba98b7163d4, height 413038).
            /* nTime    */ 1548243888,
            /* nTxCount */ 769819,
            /* dTxRate  */ 0.03008501992807923
        };
        consensus.nMPoSRewardRecipients = 10;
        consensus.nFirstMPoSBlock = 1000 + consensus.nMPoSRewardRecipients + 
                                    COINBASE_MATURITY;
    }
};
static CMainParams mainParams;

/**
 * Testnet (v3)
 */
class CTestNetParams : public CChainParams {
public:
    CTestNetParams() {
        strNetworkID = "test";
        consensus.nSubsidyHalvingInterval = 525600 ; // PoS Halving every 1 years
        consensus.powLimit = uint256S("0000ffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff");
        consensus.posLimit = uint256S("00000000ffffffffffffffffffffffffffffffffffffffffffffffffffffffff");
        consensus.checkpointPubKey = "04c7617702e41c0da3a6af3e6a3aa5305e1df312308637abaa86775bb09d3ed797a02351a298a464940a7460c0833ba7ead0ff45c8a735e9b46e0862e56bb79f98";
        consensus.vAlertPubKey = ParseHex("04208bb802d593cd4d4aeb95a00c99928787b49f694638362fb0c86b38186f6f60524050978ee24dba821aa4ce85a721bb6d05a7ef0e52c9b6f9a5ae278f45b8b7");
        consensus.nTargetTimespan = 120;
        consensus.fPowAllowMinDifficultyBlocks = false;
        consensus.fPowNoRetargeting = false;
        consensus.fPoSNoRetargeting = false;
        consensus.nRuleChangeActivationThreshold = 1512; // 75% for testchains
        consensus.nMinerConfirmationWindow = 2016;
        consensus.nDiffAdjustChange = 0;
        consensus.nDiffDamping = 0;
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].bit = 28;
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].nStartTime = 1199145601; // January 1, 2008
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].nTimeout = 1230767999; // December 31, 2008


        // Deployment of BIP68, BIP112, and BIP113.
        consensus.vDeployments[Consensus::DEPLOYMENT_CSV].bit = 0;
        consensus.vDeployments[Consensus::DEPLOYMENT_CSV].nStartTime = 0;
        consensus.vDeployments[Consensus::DEPLOYMENT_CSV].nTimeout = 999999999999ULL;

        // Deployment of SegWit (BIP141, BIP143, and BIP147)
        consensus.vDeployments[Consensus::DEPLOYMENT_SEGWIT].bit = 1;
        consensus.vDeployments[Consensus::DEPLOYMENT_SEGWIT].nStartTime = 0;
        consensus.vDeployments[Consensus::DEPLOYMENT_SEGWIT].nTimeout = 999999999999ULL;

        // The best chain should have at least this much work.
        //consensus.nMinimumChainWork = uint256S("0x0000000000000000000000000000000000000000000000000000000000010000"); // qtum

        // By default assume that the signatures in ancestors of this block are valid.
        //consensus.defaultAssumeValid = uint256S("0x00000000000128796ee387cf110ccb9d2f36cffaf7f73079c995377c65ac0dcc"); //1079274

        pchMessageStart[0] = 0x1a;
        pchMessageStart[1] = 0x2b;
        pchMessageStart[2] = 0x3c;
        pchMessageStart[3] = 0x4d;
        nDefaultPort = 32915;
        nPruneAfterHeight = 1000;
        startNewChain = false;

        genesis = CreateGenesisBlock(1522223204, 135269, 0x1f00ffff, 1, 1 * COIN);

        if (startNewChain)
            MineGenesis(genesis, consensus.powLimit, false);

        consensus.hashGenesisBlock = genesis.GetHash();
        assert(consensus.hashGenesisBlock == uint256S("0x0000d068e1d30f79fb64446137106be9c6ee69a6a722295c131506b1ee09b77c"));

        vFixedSeeds.clear();
        vSeeds.clear();

        vSeeds.push_back(CDNSSeedData("test.dnsseed.vipstarcoin.jp", "test.dnsseed.vipstarcoin.jp"));

        base58Prefixes[PUBKEY_ADDRESS] = std::vector<unsigned char>(1,132);
        base58Prefixes[SCRIPT_ADDRESS] = std::vector<unsigned char>(1,110);
        base58Prefixes[SECRET_KEY] =     std::vector<unsigned char>(1,239);
        base58Prefixes[EXT_PUBLIC_KEY] = boost::assign::list_of(0x04)(0x35)(0x87)(0xCF).convert_to_container<std::vector<unsigned char> >();
        base58Prefixes[EXT_SECRET_KEY] = boost::assign::list_of(0x04)(0x35)(0x83)(0x94).convert_to_container<std::vector<unsigned char> >();

        vFixedSeeds = std::vector<SeedSpec6>(pnSeed6_test, pnSeed6_test + ARRAYLEN(pnSeed6_test));

        fMiningRequiresPeers = true;
        fDefaultConsistencyChecks = false;
        fRequireStandard = false;
        fMineBlocksOnDemand = false;

        checkpointData = (CCheckpointData) {
            boost::assign::map_list_of
            ( 0, uint256S("0000d068e1d30f79fb64446137106be9c6ee69a6a722295c131506b1ee09b77c")),
        };

        chainTxData = ChainTxData{
        	0,
        	0,
        	0
        };

        consensus.nMPoSRewardRecipients = 10;
        consensus.nFirstMPoSBlock = 450 + consensus.nMPoSRewardRecipients +
                                    COINBASE_MATURITY;
    }
};
static CTestNetParams testNetParams;

/**
 * Regression test
 */
class CRegTestParams : public CChainParams {
public:
    CRegTestParams() {
        strNetworkID = "regtest";
        consensus.nSubsidyHalvingInterval = 150;
        consensus.powLimit = uint256S("7fffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff");
        consensus.posLimit = uint256S("7fffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff");
        consensus.vAlertPubKey = ParseHex("04208bb802d593cd4d4aeb95a00c99928787b49f694638362fb0c86b38186f6f60524050978ee24dba821aa4ce85a721bb6d05a7ef0e52c9b6f9a5ae278f45b8b7");
        consensus.nTargetTimespan = 60;
        consensus.fPowAllowMinDifficultyBlocks = true;
        consensus.fPowNoRetargeting = true;
        consensus.fPoSNoRetargeting = true;
        consensus.nRuleChangeActivationThreshold = 108; // 75% for testchains
        consensus.nMinerConfirmationWindow = 144; // Faster than normal for regtest (144 instead of 2016)
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].bit = 28;
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].nStartTime = 0;
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].nTimeout = 999999999999ULL;
        consensus.vDeployments[Consensus::DEPLOYMENT_CSV].bit = 0;
        consensus.vDeployments[Consensus::DEPLOYMENT_CSV].nStartTime = 0;
        consensus.vDeployments[Consensus::DEPLOYMENT_CSV].nTimeout = 999999999999ULL;
        consensus.vDeployments[Consensus::DEPLOYMENT_SEGWIT].bit = 1;
        consensus.vDeployments[Consensus::DEPLOYMENT_SEGWIT].nStartTime = 0;
        consensus.vDeployments[Consensus::DEPLOYMENT_SEGWIT].nTimeout = 999999999999ULL;

        // The best chain should have at least this much work.
        consensus.nMinimumChainWork = uint256S("0x00");

        // By default assume that the signatures in ancestors of this block are valid.
        consensus.defaultAssumeValid = uint256S("0x00");

        pchMessageStart[0] = 0x2a;
        pchMessageStart[1] = 0x3b;
        pchMessageStart[2] = 0x4c;
        pchMessageStart[3] = 0x5d;
        nDefaultPort = 33915;
        nPruneAfterHeight = 1000;
        startNewChain = false;

        genesis = CreateGenesisBlock(1522223204, 0, 0x207fffff, 1, 1 * COIN);

        if (startNewChain)
            MineGenesis(genesis, consensus.powLimit, false);

        consensus.hashGenesisBlock = genesis.GetHash();
        assert(consensus.hashGenesisBlock == uint256S("0x00054237d8dc843bd3f7ab66e2b608cfbfbbeed213e30eca9ca8c627dc8aace5"));

        vFixedSeeds.clear(); //!< Regtest mode doesn't have any fixed seeds.
        vSeeds.clear();      //!< Regtest mode doesn't have any DNS seeds.

        fMiningRequiresPeers = false;
        fDefaultConsistencyChecks = true;
        fRequireStandard = false;
        fMineBlocksOnDemand = false;

        checkpointData = (CCheckpointData){
            boost::assign::map_list_of
            ( 0, uint256S("00054237d8dc843bd3f7ab66e2b608cfbfbbeed213e30eca9ca8c627dc8aace5"))
        };

        chainTxData = ChainTxData{
            0,
            0,
            0
        };

        consensus.nMPoSRewardRecipients = 10;
        consensus.nFirstMPoSBlock = 1325;

        base58Prefixes[PUBKEY_ADDRESS] = std::vector<unsigned char>(1,132);
        base58Prefixes[SCRIPT_ADDRESS] = std::vector<unsigned char>(1,110);
        base58Prefixes[SECRET_KEY] =     std::vector<unsigned char>(1,239);
        base58Prefixes[EXT_PUBLIC_KEY] = boost::assign::list_of(0x04)(0x35)(0x87)(0xCF).convert_to_container<std::vector<unsigned char> >();
        base58Prefixes[EXT_SECRET_KEY] = boost::assign::list_of(0x04)(0x35)(0x83)(0x94).convert_to_container<std::vector<unsigned char> >();
    }

    void UpdateBIP9Parameters(Consensus::DeploymentPos d, int64_t nStartTime, int64_t nTimeout)
    {
        consensus.vDeployments[d].nStartTime = nStartTime;
        consensus.vDeployments[d].nTimeout = nTimeout;
    }
};
static CRegTestParams regTestParams;

static CChainParams *pCurrentParams = 0;

const CChainParams &Params() {
    assert(pCurrentParams);
    return *pCurrentParams;
}

CChainParams& Params(const std::string& chain)
{
    if (chain == CBaseChainParams::MAIN)
            return mainParams;
    else if (chain == CBaseChainParams::TESTNET)
            return testNetParams;
    else if (chain == CBaseChainParams::REGTEST)
            return regTestParams;
    else
        throw std::runtime_error(strprintf("%s: Unknown chain %s.", __func__, chain));
}

void SelectParams(const std::string& network)
{
    SelectBaseParams(network);
    pCurrentParams = &Params(network);
}

void UpdateRegtestBIP9Parameters(Consensus::DeploymentPos d, int64_t nStartTime, int64_t nTimeout)
{
    regTestParams.UpdateBIP9Parameters(d, nStartTime, nTimeout);
}

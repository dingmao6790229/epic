// Copyright (c) 2019 EPI-ONE Core Developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "params.h"
#include "vertex.h"
#include "utilstrencodings.h"

// 1 day per diffculty cycle on average
static constexpr uint32_t TARGET_TIMESPAN = 24 * 60 * 60;
// 10 seconds per milestone block
static constexpr uint32_t TIME_INTERVAL = 10;
// number of milestones between two difficulty adjustment
static constexpr uint32_t INTERVAL = TARGET_TIMESPAN / static_cast<double>(TIME_INTERVAL);
// transaction per second
static constexpr uint32_t TPS = 1000;
// threshold for rejecting an old block
static constexpr uint32_t PUNTUALITY_THRESHOLD = 100;
// max amount of money allowed in one output
static constexpr uint64_t MAX_MONEY = 9999999999L;
// version of genesis block
static constexpr uint32_t GENESIS_BLOCK_VERSION = 1;
// an easy enough difficulty target
static constexpr uint32_t EASIEST_COMP_DIFF_TARGET = 0x2100ffffL;
// transaction sortition: coefficient for computing allowed distance
static constexpr float SORTITION_COEFFICIENT = 0.01;
// transaction sortition: number of block to go back
static constexpr size_t SORTITION_THRESHOLD = 1000;
// coefficient of taking additional reward for milestone
static constexpr uint32_t REWARD_COEFFICIENT = 50;
// capacity of transactions in a block
static constexpr size_t BLK_CAPACITY = 128;

std::shared_ptr<Vertex> Params::CreateGenesis() const {
    const std::vector<unsigned char> parsed = ParseHex(genesisHexStr);
    VStream vs(parsed);
    Block genesisBlock;
    vs >> genesisBlock;
    genesisBlock.FinalizeHash();
    genesisBlock.CalculateOptimalEncodingSize();

    auto genesisVertex         = std::make_shared<Vertex>(genesisBlock);
    genesisVertex->validity[0] = Vertex::VALID;

    arith_uint256 msTarget    = maxTarget;
    arith_uint256 blockTarget = maxTarget;
    uint64_t hashRate         = (arith_uint256{maxTarget} / (msTarget + 1)).GetLow64() / timeInterval;
    auto chainwork            = maxTarget / arith_uint256().SetCompact(genesisBlock.GetDifficultyTarget());

    static auto genesisMs = std::make_shared<Milestone>(0, chainwork, msTarget, blockTarget, hashRate,
                                                        genesisBlock.GetTime(), std::vector<VertexWPtr>{genesisVertex});

    genesisVertex->LinkMilestone(genesisMs);
    SetGenesisParams(genesisVertex);
    return genesisVertex;
}

unsigned char Params::GetKeyPrefix(KeyPrefixType type) const {
    return keyPrefixes[type];
}

Coin Params::GetReward(size_t height) const {
    if (height == 0) {
        return 0;
    }

    auto epoch = (height - 1) / rewardAdjustInterval;
    return round(baseReward / (float) (epoch + 1));
}

MainNetParams::MainNetParams() {
    version              = GENESIS_BLOCK_VERSION;
    targetTimespan       = TARGET_TIMESPAN;
    timeInterval         = TIME_INTERVAL;
    interval             = INTERVAL;
    targetTPS            = TPS;
    punctualityThred     = PUNTUALITY_THRESHOLD;
    maxTarget            = arith_uint256().SetCompact(EASIEST_COMP_DIFF_TARGET);
    maxMoney             = MAX_MONEY;
    baseReward           = 10000;
    rewardAdjustInterval = 3000000;
    msRewardCoefficient  = REWARD_COEFFICIENT;
    cycleLen             = 42;
    sortitionCoefficient = SORTITION_COEFFICIENT;
    sortitionThreshold   = SORTITION_THRESHOLD;
    deleteForkThreshold  = punctualityThred;
    blockCapacity        = BLK_CAPACITY;
    magic                = 0x44535A5A;

    keyPrefixes = {
        0,  // keyPrefixes[PUBKEY_ADDRESS]
        128 // keyPrefixes[SECRET_KEY]
    };

    genesisHexStr =
        "0100e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855e3b0c44298fc1c149afbf4c8996fb92427ae41e464"
        "9b934ca495991b7852b855e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b8555b9fa07329a2149b758dbec2"
        "530cd81cbe05b33cdb32b6b03470fb6601ef3255388ff95cffff00211800000027635f00c6d49a0091a1ca007a69d500ec1246014feac3"
        "02c244b30398815f04ac8ae204dcc73f05231fca0704788f085f42a30847ba3f09a47c4d09ba957609cb5f9209cdaec10ae3a1ac0dcf39"
        "290f8460d60f5ae76910fdd42e115a4cc0112d1384124fe98e139b08b014a7f7b714cbe5d814b8c61216e07e6716ec3f7418417d8c18c2"
        "d5c218ca7956196736bb1b11a11b1c300b9f1ca171a41c94b1c81c3b3a811da693351f0101e3b0c44298fc1c149afbf4c8996fb92427ae"
        "41e4649b934ca495991b7852b855ffffffffffffffff00484704ffff001d0104454974206973206e6f772074656e20706173742074656e"
        "20696e20746865206576656e696e6720616e6420776520617265207374696c6c20776f726b696e6721014200142ac277ce311a053c91e4"
        "7fd2c4759b263e1b31b4";
}

TestNetSpadeParams::TestNetSpadeParams() {
    version              = 10;
    targetTimespan       = 100;
    timeInterval         = TIME_INTERVAL;
    interval             = targetTimespan / (double) timeInterval;
    targetTPS            = 100;
    punctualityThred     = PUNTUALITY_THRESHOLD;
    maxTarget            = arith_uint256().SetCompact(EASIEST_COMP_DIFF_TARGET);
    maxMoney             = MAX_MONEY;
    baseReward           = 10000000000;
    rewardAdjustInterval = 3000000;
    msRewardCoefficient  = REWARD_COEFFICIENT;
    cycleLen             = 4;
    sortitionCoefficient = SORTITION_COEFFICIENT;
    sortitionThreshold   = 100;
    deleteForkThreshold  = punctualityThred;
    blockCapacity        = BLK_CAPACITY;
    magic                = 0x524F434B;

    keyPrefixes = {
        0,  // keyPrefixes[PUBKEY_ADDRESS]
        128 // keyPrefixes[SECRET_KEY]
    };

    genesisHexStr =
        "0a00e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855e3b0c44298fc1c149afbf4c8996fb92427ae41e464"
        "9b934ca495991b7852b855e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b8555b9fa07329a2149b758dbec2"
        "530cd81cbe05b33cdb32b6b03470fb6601ef3255388ff95cffff0021030000003c8dcb0244c0c70c51e6ae0e4b592f0f0101e3b0c44298"
        "fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855ffffffffffffffff00484704ffff001d0104454974206973206e6f77"
        "2074656e20706173742074656e20696e20746865206576656e696e6720616e6420776520617265207374696c6c20776f726b696e672101"
        "4200142ac277ce311a053c91e47fd2c4759b263e1b31b4";
}

TestNetDiamondParams::TestNetDiamondParams() {
    version              = 11;
    targetTimespan       = 100;
    timeInterval         = TIME_INTERVAL;
    interval             = targetTimespan / (double) timeInterval;
    targetTPS            = 1000;
    punctualityThred     = PUNTUALITY_THRESHOLD;
    maxTarget            = arith_uint256().SetCompact(EASIEST_COMP_DIFF_TARGET);
    maxMoney             = MAX_MONEY;
    baseReward           = 10000000000;
    rewardAdjustInterval = 3000000;
    msRewardCoefficient  = REWARD_COEFFICIENT;
    cycleLen             = 0;
    sortitionCoefficient = SORTITION_COEFFICIENT;
    sortitionThreshold   = 100;
    deleteForkThreshold  = punctualityThred;
    blockCapacity        = BLK_CAPACITY;
    magic                = 0x524F4C4C;

    keyPrefixes = {
        0,  // keyPrefixes[PUBKEY_ADDRESS]
        128 // keyPrefixes[SECRET_KEY]
    };

    genesisHexStr = "0b00e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855e3b0c44298fc1c149afbf4c8996fb9"
                    "2427ae41e4649b934ca495991b7852b855e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855"
                    "5b9fa07329a2149b758dbec2530cd81cbe05b33cdb32b6b03470fb6601ef3255388ff95cffff0021050000000101e3b0c4"
                    "4298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855ffffffffffffffff00484704ffff001d01044549"
                    "74206973206e6f772074656e20706173742074656e20696e20746865206576656e696e6720616e64207765206172652073"
                    "74696c6c20776f726b696e6721014200142ac277ce311a053c91e47fd2c4759b263e1b31b4";
}

UnitTestParams::UnitTestParams() {
    version              = 100;
    targetTimespan       = 99;
    timeInterval         = 3; // cannot be less than 3
    interval             = targetTimespan / (double) timeInterval;
    targetTPS            = 100;
    punctualityThred     = 20;
    maxTarget            = arith_uint256().SetCompact(EASIEST_COMP_DIFF_TARGET);
    maxMoney             = MAX_MONEY;
    baseReward           = 100;
    rewardAdjustInterval = 5;
    msRewardCoefficient  = 1;
    cycleLen             = 0;
    sortitionCoefficient = 1;
    sortitionThreshold   = 2;
    deleteForkThreshold  = 10;
    blockCapacity        = 10;
    magic                = 0x54455354;

    keyPrefixes = {
        0,  // keyPrefixes[PUBKEY_ADDRESS]
        128 // keyPrefixes[SECRET_KEY]
    };

    genesisHexStr = "6400e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855e3b0c44298fc1c149afbf4c8996fb9"
                    "2427ae41e4649b934ca495991b7852b855e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855"
                    "5b9fa07329a2149b758dbec2530cd81cbe05b33cdb32b6b03470fb6601ef3255388ff95cffff0021000000000101e3b0c4"
                    "4298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855ffffffffffffffff00484704ffff001d01044549"
                    "74206973206e6f772074656e20706173742074656e20696e20746865206576656e696e6720616e64207765206172652073"
                    "74696c6c20776f726b696e6721014200142ac277ce311a053c91e47fd2c4759b263e1b31b4";
}

void UnitTestParams::SetGenesisParams(const std::shared_ptr<Vertex>& genesisVertex) const {
    genesisVertex->snapshot->hashRate        = 1;
    genesisVertex->snapshot->blockTarget     = maxTarget;
    genesisVertex->snapshot->milestoneTarget = maxTarget;
}

static std::unique_ptr<const Params> pparams;

const Params& GetParams() {
    assert(pparams);
    return *pparams;
}

void SelectParams(ParamsType type, bool withGenesis) {
    if (type == ParamsType::MAINNET) {
        pparams = std::make_unique<MainNetParams>();
    } else if (type == ParamsType::SPADE) {
        pparams = std::make_unique<TestNetSpadeParams>();
    } else if (type == ParamsType::DIAMOND) {
        pparams = std::make_unique<TestNetDiamondParams>();
    } else if (type == ParamsType::UNITTEST) {
        pparams = std::make_unique<UnitTestParams>();
    } else {
        throw std::invalid_argument("Invalid param type!");
    }

    if (withGenesis) {
        GENESIS_VERTEX = pparams->CreateGenesis();
        GENESIS        = GENESIS_VERTEX->cblock;
    }
}

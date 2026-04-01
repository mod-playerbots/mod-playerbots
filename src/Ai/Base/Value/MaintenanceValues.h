/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_MAINTANCEVALUE_H
#define _PLAYERBOT_MAINTANCEVALUE_H

#include "Value.h"

#include <vector>

class PlayerbotAI;

class CanMoveAroundValue : public BoolCalculatedValue
{
public:
    CanMoveAroundValue(PlayerbotAI* botAI) : BoolCalculatedValue(botAI, "can move around", 2 * 2000) {}

    bool Calculate() override;
};

class ShouldHomeBindValue : public BoolCalculatedValue
{
public:
    ShouldHomeBindValue(PlayerbotAI* botAI) : BoolCalculatedValue(botAI, "should home bind", 2 * 2000) {}

    bool Calculate() override;
};

class ShouldRepairValue : public BoolCalculatedValue
{
public:
    ShouldRepairValue(PlayerbotAI* botAI) : BoolCalculatedValue(botAI, "should repair", 2 * 2000) {}

    bool Calculate() override;
};

class CanRepairValue : public BoolCalculatedValue
{
public:
    CanRepairValue(PlayerbotAI* botAI) : BoolCalculatedValue(botAI, "can repair", 2 * 2000) {}

    bool Calculate() override;
};

class ShouldSellValue : public BoolCalculatedValue
{
public:
    ShouldSellValue(PlayerbotAI* botAI) : BoolCalculatedValue(botAI, "should sell", 2 * 2000) {}

    bool Calculate() override;
};

class CanSellValue : public BoolCalculatedValue
{
public:
    CanSellValue(PlayerbotAI* botAI) : BoolCalculatedValue(botAI, "can sell", MINUTE * IN_MILLISECONDS) {}

    bool Calculate() override;
};

class AhSellListValue : public CalculatedValue<std::vector<uint32>>
{
public:
    AhSellListValue(PlayerbotAI* botAI)
        : CalculatedValue<std::vector<uint32>>(botAI, "ah sell list", MINUTE * IN_MILLISECONDS) {}

    std::vector<uint32> Calculate() override;

private:
    bool IsItemSellableOnAh(Item* item) const;
    uint32 ComputeBagFingerprint();
    uint32 _lastFingerprint{0};
    std::vector<uint32> _cachedList;
};

class ShouldAHSellValue : public BoolCalculatedValue
{
public:
    ShouldAHSellValue(PlayerbotAI* botAI) : BoolCalculatedValue(botAI, "should ah sell", MINUTE * IN_MILLISECONDS) {}

    bool Calculate() override;
};

class AhBuyListValue : public CalculatedValue<std::vector<uint8>>
{
public:
    AhBuyListValue(PlayerbotAI* botAI)
        : CalculatedValue<std::vector<uint8>>(botAI, "ah buy list", HOUR * IN_MILLISECONDS) {}

    std::vector<uint8> Calculate() override;
};

class ShouldAHBuyValue : public BoolCalculatedValue
{
public:
    ShouldAHBuyValue(PlayerbotAI* botAI) : BoolCalculatedValue(botAI, "should ah buy", MINUTE * IN_MILLISECONDS) {}

    bool Calculate() override;
};

class CanTrainValue : public BoolCalculatedValue
{
public:
    CanTrainValue(PlayerbotAI* botAI) : BoolCalculatedValue(botAI, "can train", 2 * 2000) {}

    bool Calculate() override;
};

class CanFightEqualValue : public BoolCalculatedValue
{
public:
    CanFightEqualValue(PlayerbotAI* botAI) : BoolCalculatedValue(botAI, "can fight equal", 2 * 2000) {}

    bool Calculate() override;
};

class CanFightEliteValue : public BoolCalculatedValue
{
public:
    CanFightEliteValue(PlayerbotAI* botAI) : BoolCalculatedValue(botAI, "can fight elite") {}

    bool Calculate() override;
};

class CanFightBossValue : public BoolCalculatedValue
{
public:
    CanFightBossValue(PlayerbotAI* botAI) : BoolCalculatedValue(botAI, "can fight boss") {}

    bool Calculate() override;
};

#endif

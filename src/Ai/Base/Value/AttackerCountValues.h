/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_ATTACKERCOUNTVALUES_H
#define PLAYERBOTS_ATTACKERCOUNTVALUES_H

#include "NamedObjectContext.h"
#include "Value.h"

class PlayerbotAI;
class Unit;

class AttackerCountValue : public Uint8CalculatedValue, public Qualified
{
public:
    AttackerCountValue(PlayerbotAI* botAI, std::string const name = "attackers count")
        : Uint8CalculatedValue(botAI, name)
    {
    }

    Unit* GetTarget();
    uint8 Calculate() override;
};

class MyAttackerCountValue : public Uint8CalculatedValue, public Qualified
{
public:
    MyAttackerCountValue(PlayerbotAI* botAI, std::string const name = "my attackers count")
        : Uint8CalculatedValue(botAI, name)
    {
    }

    Unit* GetTarget();
    uint8 Calculate() override;
};

class HasAggroValue : public BoolCalculatedValue, public Qualified
{
public:
    HasAggroValue(PlayerbotAI* botAI, std::string const name = "has aggro") : BoolCalculatedValue(botAI, name) {}

    Unit* GetTarget();
    bool Calculate() override;
};

class BalancePercentValue : public Uint8CalculatedValue, public Qualified
{
public:
    BalancePercentValue(PlayerbotAI* botAI, std::string const name = "balance percentage")
        : Uint8CalculatedValue(botAI, name)
    {
    }

    Unit* GetTarget();
    uint8 Calculate() override;
};

#endif

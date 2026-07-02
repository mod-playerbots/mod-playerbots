/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_RTIVALUE_H
#define PLAYERBOTS_RTIVALUE_H

#include "Value.h"

class PlayerbotAI;

class RtiValue : public ManualSetValue<std::string>
{
public:
    RtiValue(PlayerbotAI* botAI);

    std::string const Save() override;
    bool Load(std::string const text) override;
};

class RtiCcValue : public ManualSetValue<std::string>
{
public:
    RtiCcValue(PlayerbotAI* botAI);

    std::string const Save() override;
    bool Load(std::string const text) override;
};

#endif

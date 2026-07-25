/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_BOSSAURAACTIONS_H
#define PLAYERBOTS_BOSSAURAACTIONS_H

#include <string>

#include "Action.h"

class PlayerbotAI;

class BossFireResistanceAction : public Action
{
public:
    BossFireResistanceAction(PlayerbotAI* botAI, std::string const bossName)
        : Action(botAI, bossName + " fire resistance action"), bossName(bossName)
    {
    }
    bool Execute(Event event) override;
    bool isUseful() override;

private:
    std::string bossName;
};

class BossFrostResistanceAction : public Action
{
public:
    BossFrostResistanceAction(PlayerbotAI* botAI, std::string const bossName)
        : Action(botAI, bossName + " frost resistance action"), bossName(bossName)
    {
    }
    bool Execute(Event event) override;
    bool isUseful() override;

private:
    std::string bossName;
};

class BossNatureResistanceAction : public Action
{
public:
    BossNatureResistanceAction(PlayerbotAI* botAI, std::string const bossName)
        : Action(botAI, bossName + " nature resistance action"), bossName(bossName)
    {
    }
    bool Execute(Event event) override;
    bool isUseful() override;

private:
    std::string bossName;
};

class BossShadowResistanceAction : public Action
{
public:
    BossShadowResistanceAction(PlayerbotAI* botAI, std::string const bossName)
        : Action(botAI, bossName + " shadow resistance action"), bossName(bossName)
    {
    }
    bool Execute(Event event) override;
    bool isUseful() override;

private:
    std::string bossName;
};

#endif

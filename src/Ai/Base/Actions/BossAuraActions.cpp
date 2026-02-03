/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#include <HunterBuffStrategies.h>
#include <PaladinBuffStrategies.h>
#include <PlayerbotAI.h>

#include "BossAuraActions.h"
#include "BossAuraTriggers.h"
#include "HunterActions.h"
#include "PaladinActions.h"
#include "CreateNextAction.h"

const std::string ADD_STRATEGY_CHAR = "+";

bool BossFireResistanceAction::isUseful()
{
    BossFireResistanceTrigger bossFireResistanceTrigger(botAI, bossName);
    return bossFireResistanceTrigger.IsActive();
}

bool BossFireResistanceAction::Execute(Event)
{
    PaladinFireResistanceStrategy paladinFireResistanceStrategy(botAI);
    botAI->ChangeStrategy(ADD_STRATEGY_CHAR + paladinFireResistanceStrategy.getName(), BotState::BOT_STATE_COMBAT);
    botAI->DoSpecificAction(CreateNextAction<CastFireResistanceAuraAction>(1.0f).factory, Event(), true);
    return true;
}

bool BossFrostResistanceAction::isUseful()
{
    BossFrostResistanceTrigger bossFrostResistanceTrigger(botAI, bossName);
    return bossFrostResistanceTrigger.IsActive();
}

bool BossFrostResistanceAction::Execute(Event)
{
    PaladinFrostResistanceStrategy paladinFrostResistanceStrategy(botAI);
    botAI->ChangeStrategy(ADD_STRATEGY_CHAR + paladinFrostResistanceStrategy.getName(), BotState::BOT_STATE_COMBAT);
    botAI->DoSpecificAction(CreateNextAction<CastFrostResistanceAuraAction>(1.0f).factory, Event(), true);
    return true;
}

bool BossNatureResistanceAction::isUseful()
{
    BossNatureResistanceTrigger bossNatureResistanceTrigger(botAI, bossName);
    return bossNatureResistanceTrigger.IsActive();
}

bool BossNatureResistanceAction::Execute(Event)
{
    HunterNatureResistanceStrategy hunterNatureResistanceStrategy(botAI);
    botAI->ChangeStrategy(ADD_STRATEGY_CHAR + hunterNatureResistanceStrategy.getName(), BotState::BOT_STATE_COMBAT);
    botAI->DoSpecificAction(CreateNextAction<CastAspectOfTheWildAction>(1.0f).factory, Event(), true);
    return true;
}

bool BossShadowResistanceAction::isUseful()
{
    BossShadowResistanceTrigger bossShadowResistanceTrigger(botAI, bossName);
    return bossShadowResistanceTrigger.IsActive();
}

bool BossShadowResistanceAction::Execute(Event)
{
    PaladinShadowResistanceStrategy paladinShadowResistanceStrategy(botAI);
    botAI->ChangeStrategy(ADD_STRATEGY_CHAR + paladinShadowResistanceStrategy.getName(), BotState::BOT_STATE_COMBAT);
    botAI->DoSpecificAction(CreateNextAction<CastShadowResistanceAuraAction>(1.0f).factory, Event(), true);
    return true;
}

/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#include "RaidZulAmanMultipliers.h"
#include "RaidZulAmanActions.h"
#include "RaidZulAmanHelpers.h"
#include "ChooseTargetActions.h"
#include "DKActions.h"
#include "DruidBearActions.h"
#include "FollowActions.h"
#include "GenericSpellActions.h"
#include "HunterActions.h"
#include "MageActions.h"
#include "PaladinActions.h"
#include "Playerbots.h"
#include "PriestActions.h"
#include "RaidBossHelpers.h"
#include "ReachTargetActions.h"
#include "RogueActions.h"
#include "ShamanActions.h"
#include "WarlockActions.h"
#include "WarriorActions.h"

using namespace ZulAmanHelpers;

// Akil'zon <Eagle Avatar>

float AkilzonDisableCombatFormationMoveMultiplier::GetValue(Action* action)
{
    if (!AI_VALUE2(Unit*, "find target", "akil'zon"))
        return 1.0f;

    if (dynamic_cast<CombatFormationMoveAction*>(action) &&
        !dynamic_cast<SetBehindTargetAction*>(action))
        return 0.0f;

    return 1.0f;
}

float AkilzonStayInEyeOfTheStormMultiplier::GetValue(Action* action)
{
    if (!AI_VALUE2(Unit*, "find target", "akil'zon") /* ||
        !GetElectricalStormTarget(bot)*/)
        return 1.0f;

    auto it = akilzonStormTimer.find(bot->GetMap()->GetInstanceId());
    if (it == akilzonStormTimer.end() ||
        !IsInStormWindow(it->second, std::time(nullptr)))
        return 1.0f;

    if (dynamic_cast<CastReachTargetSpellAction*>(action) ||
        dynamic_cast<CastKillingSpreeAction*>(action) ||
        dynamic_cast<CastBlinkBackAction*>(action) ||
        dynamic_cast<CastDisengageAction*>(action) ||
        dynamic_cast<SetBehindTargetAction*>(action) ||
        dynamic_cast<FleeAction*>(action) ||
        dynamic_cast<FollowAction*>(action) ||
        dynamic_cast<ReachTargetAction*>(action))
        return 0.0f;

    return 1.0f;
}

// Nalorakk <Bear Avatar>

float NalorakkDisableTankActionsMultiplier::GetValue(Action* action)
{
    if (!botAI->IsTank(bot))
        return 1.0f;

    Unit* nalorakk = AI_VALUE2(Unit*, "find target", "nalorakk");
    if (!nalorakk)
        return 1.0f;

    if (dynamic_cast<TankFaceAction*>(action))
        return 0.0f;

    if (bot->GetVictim() == nullptr)
        return 1.0f;

    bool shouldTankBoss = false;

    if (botAI->IsMainTank(bot) &&
        !nalorakk->HasAura(static_cast<uint32>(ZulAmanSpells::SPELL_BEARFORM)))
        shouldTankBoss = true;

    if (botAI->IsAssistTankOfIndex(bot, 0, true) &&
        nalorakk->HasAura(static_cast<uint32>(ZulAmanSpells::SPELL_BEARFORM)))
        shouldTankBoss = true;

    if (!shouldTankBoss &&
        (dynamic_cast<TankAssistAction*>(action) ||
         dynamic_cast<CastTauntAction*>(action) ||
         dynamic_cast<CastGrowlAction*>(action) ||
         dynamic_cast<CastHandOfReckoningAction*>(action) ||
         dynamic_cast<CastDarkCommandAction*>(action)))
        return 0.0f;

    return 1.0f;
}

float NalorakkControlMisdirectionMultiplier::GetValue(Action* action)
{
    if (bot->getClass() != CLASS_HUNTER ||
        !AI_VALUE2(Unit*, "find target", "nalorakk"))
        return 1.0f;

    if (dynamic_cast<CastMisdirectionOnMainTankAction*>(action))
        return 0.0f;

    return 1.0f;
}

// Jan'alai <Dragonhawk Avatar>

float JanalaiDisableTankActionsMultiplier::GetValue(Action* action)
{
    if (!botAI->IsTank(bot) ||
        !AI_VALUE2(Unit*, "find target", "jan'alai"))
        return 1.0f;

    if (dynamic_cast<TankFaceAction*>(action))
        return 0.0f;

    if (bot->GetVictim() == nullptr)
        return 1.0f;

    if (botAI->IsMainTank(bot) &&
        dynamic_cast<TankAssistAction*>(action))
        return 0.0f;

    if (botAI->IsAssistTank(bot) &&
        !GetFirstAliveUnitByEntry(
            botAI, static_cast<uint32>(ZulAmanNPCs::NPC_AMANI_DRAGONHAWK_HATCHLING)) &&
        dynamic_cast<TankAssistAction*>(action))
        return 0.0f;

    return 1.0f;
}

float JanalaiDisableCombatFormationMoveMultiplier::GetValue(Action* action)
{
    if (!AI_VALUE2(Unit*, "find target", "jan'alai"))
        return 1.0f;

    if (dynamic_cast<CombatFormationMoveAction*>(action) &&
        !dynamic_cast<SetBehindTargetAction*>(action))
        return 0.0f;

    return 1.0f;
}

float JanalaiStayAwayFromFireBombsMultiplier::GetValue(Action* action)
{
    if (!AI_VALUE2(Unit*, "find target", "jan'alai"))
        return 1.0f;

    if (!HasFireBombNearby(botAI, bot))
        return 1.0f;

    if (dynamic_cast<CastReachTargetSpellAction*>(action) ||
        dynamic_cast<CastKillingSpreeAction*>(action) ||
        dynamic_cast<CastBlinkBackAction*>(action) ||
        dynamic_cast<CastDisengageAction*>(action) ||
        dynamic_cast<FleeAction*>(action) ||
        dynamic_cast<FollowAction*>(action) ||
        dynamic_cast<ReachTargetAction*>(action))
        return 0.0f;

    return 1.0f;
}

float JanalaiDoNotCrowdControlHatchersMultiplier::GetValue(Action* action)
{
    if (!AI_VALUE2(Unit*, "find target", "amani'shi hatcher"))
        return 1.0f;

    if (dynamic_cast<CastCrowdControlSpellAction*>(action) ||
        dynamic_cast<CastPolymorphAction*>(action))
        return 0.0f;

    return 1.0f;
}

float JanalaiDelayBloodlustAndHeroismMultiplier::GetValue(Action* action)
{
    if (bot->getClass() != CLASS_SHAMAN)
        return 1.0f;

    if (!AI_VALUE2(Unit*, "find target", "jan'alai"))
        return 1.0f;

    if (AI_VALUE2(Unit*, "find target", "amani dragonhawk hatchling"))
        return 1.0f;

    if (dynamic_cast<CastBloodlustAction*>(action) ||
        dynamic_cast<CastHeroismAction*>(action))
        return 0.0f;

    return 1.0f;
}

// Halazzi <Lynx Avatar>

float HalazziDisableTankActionsMultiplier::GetValue(Action* action)
{
    if (!botAI->IsTank(bot) ||
        !AI_VALUE2(Unit*, "find target", "halazzi"))
        return 1.0f;

    if (dynamic_cast<TankFaceAction*>(action))
        return 0.0f;

    if (bot->GetVictim() != nullptr &&
        dynamic_cast<TankAssistAction*>(action))
        return 0.0f;

    return 1.0f;
}

float HalazziControlMisdirectionMultiplier::GetValue(Action* action)
{
    if (bot->getClass() != CLASS_HUNTER ||
        !AI_VALUE2(Unit*, "find target", "halazzi"))
        return 1.0f;

    if (dynamic_cast<CastMisdirectionOnMainTankAction*>(action))
        return 0.0f;

    return 1.0f;
}

// Hex Lord Malacrass

float HexLordMalacrassAvoidWhirlwindMultiplier::GetValue(Action* action)
{
    if (botAI->IsMainTank(bot))
        return 1.0f;

    Unit* malacrass = AI_VALUE2(Unit*, "find target", "hex lord malacrass");
    if (!malacrass ||
        !malacrass->HasAura(static_cast<uint32>(ZulAmanSpells::SPELL_HEX_LORD_WHIRLWIND)))
        return 1.0f;

    if (dynamic_cast<CastReachTargetSpellAction*>(action) ||
        dynamic_cast<CastKillingSpreeAction*>(action) ||
        dynamic_cast<ReachTargetAction*>(action))
        return 0.0f;

    return 1.0f;
}

float HexLordMalacrassStopAttackingDuringSpellReflectionMultiplier::GetValue(Action* action)
{
    if (!botAI->IsCaster(bot))
        return 1.0f;

    Unit* malacrass = AI_VALUE2(Unit*, "find target", "hex lord malacrass");
    if (!malacrass ||
        !malacrass->HasAura(static_cast<uint32>(ZulAmanSpells::SPELL_HEX_LORD_SPELL_REFLECTION)))
        return 1.0f;

    auto castSpellAction = dynamic_cast<CastSpellAction*>(action);
    if (!castSpellAction)
        return 1.0f;

    if (castSpellAction->getThreatType() == Action::ActionThreatType::Aoe ||
        (bot->GetVictim() == malacrass &&
         castSpellAction->getThreatType() == Action::ActionThreatType::Single))
        return 0.0f;

    return 1.0f;
}

float HexLordMalacrassDoNotDispelUnstableAfflictionMultiplier::GetValue(Action* action)
{
    if (bot->getClass() != CLASS_PRIEST &&
        bot->getClass() != CLASS_PALADIN &&
        bot->getClass() != CLASS_WARLOCK)
        return 1.0f;

    if (!AI_VALUE2(Unit*, "find target", "hex lord malacrass"))
        return 1.0f;

    Group* group = bot->GetGroup();
    if (!group)
        return 1.0f;

    bool hasUnstableAffliction = false;
    for (GroupReference* ref = bot->GetGroup()->GetFirstMember(); ref != nullptr; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (!member || !member->IsAlive())
            continue;

        if (member->HasAura(static_cast<uint32>(ZulAmanSpells::SPELL_UNSTABLE_AFFLICTION)))
        {
            hasUnstableAffliction = true;
            break;
        }
    }

    if (!hasUnstableAffliction)
        return 1.0f;

    if (dynamic_cast<CastDevourMagicCleanseAction*>(action) ||
        dynamic_cast<CastDispelMagicAction*>(action) ||
        dynamic_cast<CastDispelMagicOnPartyAction*>(action) ||
        dynamic_cast<CastMassDispelAction*>(action) ||
        dynamic_cast<CastPurgeAction*>(action))
        return 0.0f;

    return 1.0f;
}

// Zul'jin

float ZuljinDisableTankFaceMultiplier::GetValue(Action* action)
{
    if (!botAI->IsTank(bot))
        return 1.0f;

    Unit* zuljin = AI_VALUE2(Unit*, "find target", "zul'jin");
    if (!zuljin ||
        zuljin->HasAura(static_cast<uint32>(ZulAmanSpells::SPELL_SHAPE_OF_THE_DRAGONHAWK)))
        return 1.0f;

    if (dynamic_cast<TankFaceAction*>(action))
        return 0.0f;

    return 1.0f;
}

float ZuljinAvoidWhirlwindMultiplier::GetValue(Action* action)
{
    if (botAI->IsMainTank(bot))
        return 1.0f;

    Unit* zuljin = AI_VALUE2(Unit*, "find target", "zul'jin");
    if (!zuljin ||
        !zuljin->HasAura(static_cast<uint32>(ZulAmanSpells::SPELL_ZULJIN_WHIRLWIND)))
        return 1.0f;

    if (dynamic_cast<CastReachTargetSpellAction*>(action) ||
        dynamic_cast<CastKillingSpreeAction*>(action) ||
        dynamic_cast<ReachTargetAction*>(action))
        return 0.0f;

    return 1.0f;
}

float ZuljinDisableAvoidAoeMultiplier::GetValue(Action* action)
{
    Unit* zuljin = AI_VALUE2(Unit*, "find target", "zul'jin");
    if (!zuljin ||
        !zuljin->HasAura(static_cast<uint32>(ZulAmanSpells::SPELL_SHAPE_OF_THE_EAGLE)))
        return 1.0f;

    if (dynamic_cast<AvoidAoeAction*>(action))
        return 0.0f;

    return 1.0f;
}

float ZuljinDelayBloodlustAndHeroismMultiplier::GetValue(Action* action)
{
    if (bot->getClass() != CLASS_SHAMAN)
        return 1.0f;

    Unit* zuljin = AI_VALUE2(Unit*, "find target", "zul'jin");
    if (!zuljin ||
        zuljin->HasAura(static_cast<uint32>(ZulAmanSpells::SPELL_SHAPE_OF_THE_EAGLE)))
        return 1.0f;

    if (dynamic_cast<CastBloodlustAction*>(action) ||
        dynamic_cast<CastHeroismAction*>(action))
        return 0.0f;

    return 1.0f;
}

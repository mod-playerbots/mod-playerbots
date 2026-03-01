/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#include "AggressiveTargetValue.h"

#include "AiObjectContext.h"
#include "Player.h"
#include "PlayerbotAI.h"
#include "ServerFacade.h"
#include "SharedDefines.h"

#include "PlayerbotMgr.h"

Player* AggressiveTargetValue::getContextualisedMaster() noexcept
{
    Player* master = this->GetMaster();

    if (master == nullptr)
    {
        return nullptr;
    }

    if (master == bot)
    {
        return nullptr;
    }

    if (master->GetMapId() != this->bot->GetMapId())
    {
        return nullptr;
    }

    if (master->IsBeingTeleported())
    {
        return nullptr;
    }

    const PlayerbotAI* const masterAI = PlayerbotsMgr::instance().GetPlayerbotAI(master);

    if (masterAI == nullptr)
    {
        return nullptr;
    }

    return master;
}

Unit* AggressiveTargetValue::Calculate()
{
    Player* const master = this->getContextualisedMaster();

    Value<GuidVector>* const possibleTargetsValue = context->GetValue<GuidVector>("possible targets");

    if (possibleTargetsValue == nullptr)
    {
        return nullptr;
    }

    const GuidVector targets = possibleTargetsValue->Get();

    if (targets.empty())
    {
        return nullptr;
    }

    const float aggroRange = PlayerbotAIConfig::instance().aggroDistance;
    float distance = 0;
    Unit* result = nullptr;

    for (const ObjectGuid guid : targets)
    {
        Unit* const unit = botAI->GetUnit(guid);

        if (unit == nullptr)
        {
            return nullptr;
        }

        if (!unit->IsAlive())
        {
            return nullptr;
        }

        if (!unit->IsInWorld() || unit->IsDuringRemoveFromWorld())
        {
            return nullptr;
        }

        const Creature* const creature = dynamic_cast<Creature*>(unit);

        if (
            creature != nullptr
            && !creature->GetCreatureTemplate()->lootid
            && this->bot->GetReactionTo(unit) >= REP_NEUTRAL
        )
        {
            return nullptr;
        }

        if (!this->bot->IsHostileTo(unit) && unit->GetNpcFlags() != UNIT_NPC_FLAG_NONE)
        {
            return nullptr;
        }

        if (abs(this->bot->GetPositionZ() - unit->GetPositionZ()) > INTERACTION_DISTANCE)
        {
            return nullptr;
        }

        if (
            !this->bot->InBattleground()
            && master != nullptr
            && this->botAI->HasStrategy("follow", BotState::BOT_STATE_NON_COMBAT)
            && ServerFacade::instance().GetDistance2d(master, unit) > aggroRange
        )
        {
            return nullptr;
        }

        if (!this->bot->IsWithinLOSInMap(unit))
        {
            return nullptr;
        }

        if (this->bot->GetDistance(unit) > aggroRange)
        {
            return nullptr;
        }

        const float newdistance = this->bot->GetDistance(unit);

        if (result == nullptr || (newdistance < distance))
        {
            distance = newdistance;
            result = unit;
        }
    }

    return result;
}

/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "RaidDirectTargeting.h"

#include "EncounterHelpers.h"
#include "Group.h"
#include "Pet.h"
#include "Playerbots.h"
#include "RtiTargetValue.h"
#include "Unit.h"

bool RaidDirectAttackAction::AttackDirect(Unit* target)
{
    if (!target)
        return false;

    // Above the early return: the generic pet attack node is off, so a pet keeps whatever it
    // latched onto. No-op when the pet is already there.
    EncounterHelpers::CommandPetAttack(botAI, target);

    if (AI_VALUE(Unit*, "current target") == target)
        return false;

    return Attack(target);
}

Unit* GetRaidHumanSkull(PlayerbotAI* botAI, bool (*belongs)(Unit*))
{
    Player* bot = botAI ? botAI->GetBot() : nullptr;
    Group* group = bot ? bot->GetGroup() : nullptr;
    if (!group || !belongs)
        return nullptr;

    ObjectGuid const skull = group->GetTargetIcon(RtiTargetValue::skullIndex);
    if (skull.IsEmpty())
        return nullptr;

    // In combat is the pre-pull guard: Attack() has no out-of-combat check of its own.
    Unit* unit = botAI->GetUnit(skull);
    if (!unit || !unit->IsAlive() || !unit->IsInCombat() || unit->HasUnitFlag(UNIT_FLAG_NOT_SELECTABLE) ||
        !belongs(unit) || !bot->IsValidAttackTarget(unit))
        return nullptr;

    return unit;
}

bool RaidPetOffPick(PlayerbotAI* botAI, Unit* pick)
{
    Player* bot = botAI ? botAI->GetBot() : nullptr;
    Guardian* pet = bot ? bot->GetGuardianPet() : nullptr;
    return pet && pick && pet->IsAlive() && pet->GetReactState() != REACT_PASSIVE &&
           bot->IsValidAttackTarget(pick) && pet->GetVictim() != pick;
}

void RaidRestoreLegacyRti(PlayerbotAI* botAI, uint32 mapId)
{
    Player* bot = botAI ? botAI->GetBot() : nullptr;
    if (!bot || bot->GetMapId() != mapId)
        return;

    Value<std::string>* rti = botAI->GetAiObjectContext()->GetValue<std::string>("rti");
    std::string const mark = rti->Get();
    if (mark == "cross" || mark == "square" || mark == "triangle" || mark == "diamond")
        rti->Set("skull");
}

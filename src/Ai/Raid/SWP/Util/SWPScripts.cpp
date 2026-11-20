/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "InstanceScript.h"
#include "ObjectAccessor.h"
#include "Playerbots.h"
#include "Player.h"
#include "ScriptMgr.h"
#include "Spell.h"
#include "SWPEncounter_Felmyst.h"
#include "SWPEncounter_Kalec.h"
#include "SWPEncounter_KJ.h"
#include "SWPEncounter_Muru.h"
#include "SWPEncounter_Twins.h"
#include "SWPShared.h"
#include "SWPTriggers.h"
#include <list>
#include <vector>

using namespace SwpHelpers;

namespace
{

PlayerbotAI* FindFirstSunwellCombatBotInGroup(Player* referencePlayer)
{
    if (!referencePlayer)
        return nullptr;

    if (PlayerbotAI* botAI = GET_PLAYERBOT_AI(referencePlayer);
        botAI && botAI->HasStrategy("sunwell", BOT_STATE_COMBAT))
    {
        return botAI;
    }

    Group* group = referencePlayer->GetGroup();
    if (!group)
        return nullptr;

    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (!member || member == referencePlayer || member->GetMapId() != SWP_MAP_ID)
            continue;

        if (PlayerbotAI* botAI = GET_PLAYERBOT_AI(member);
            botAI && botAI->HasStrategy("sunwell", BOT_STATE_COMBAT))
        {
            return botAI;
        }
    }

    return nullptr;
}

bool IsCastingOrChanneling(Player* player)
{
    return player && (player->GetCurrentSpell(CURRENT_GENERIC_SPELL) ||
        player->GetCurrentSpell(CURRENT_CHANNELED_SPELL));
}

Player* GetFirstPlayerSpellTarget(Spell* spell, Unit* caster)
{
    if (!spell || !caster)
        return nullptr;

    if (Unit* unitTarget = spell->m_targets.GetUnitTarget())
        return unitTarget->ToPlayer();

    std::list<TargetInfo> const& targets = *spell->GetUniqueTargetInfo();
    if (targets.empty())
        return nullptr;

    for (TargetInfo const& targetInfo : targets)
    {
        if (Player* target = ObjectAccessor::GetPlayer(*caster, targetInfo.targetGUID))
            return target;
    }

    return nullptr;
}

// Each test is the one the move's trigger uses: the fog move, the group move (the early move
// included), and the sidestep.
bool IsFelmystMoveDueForBot(Player* player, PlayerbotAI* botAI, Creature* felmyst)
{
    FogOfCorruptionState fogState;
    if (TryGetActiveFogOfCorruptionState(player, felmyst, fogState))
        return true;

    Position groupPosition;
    if ((IsFelmystAirGroupMoveActive(player, felmyst) || HasFelmystEarlyMoveStarted(botAI) ||
         IsFelmystEarlyMoveDue(player, felmyst)) &&
        TryGetFelmystAirGroupPosition(felmyst, groupPosition) &&
        player->GetExactDist2d(groupPosition) > FELMYST_GROUP_ARRIVAL_DISTANCE)
    {
        return true;
    }

    return felmyst->IsFlying() && !GetFelmystOwnDemonicVapor(player) &&
        IsNearFelmystDemonicVaporHazard(player);
}

void RequestInterruptForBotsNeedingFelmystMovement(Creature* felmyst)
{
    // No state before the bots' triggers first see the encounter in progress
    if (!felmyst || !FindFelmystState(felmyst->GetInstanceId()))
        return;

    Map::PlayerList const& players = felmyst->GetMap()->GetPlayers();
    for (Map::PlayerList::const_iterator it = players.begin(); it != players.end(); ++it)
    {
        Player* player = it->GetSource();
        if (!player || !player->IsAlive() || !IsCastingOrChanneling(player))
            continue;

        PlayerbotAI* botAI = GET_PLAYERBOT_AI(player);
        if (!botAI || !botAI->HasStrategy("sunwell", BOT_STATE_COMBAT))
            continue;

        if (IsFelmystMoveDueForBot(player, botAI, felmyst))
            botAI->RequestSpellInterrupt();
    }
}

void RequestInterruptForBotsWithFelmystEncapsulate(Creature* felmyst)
{
    if (!felmyst || felmyst->IsFlying())
        return;

    Map::PlayerList const& players = felmyst->GetMap()->GetPlayers();
    for (Map::PlayerList::const_iterator it = players.begin(); it != players.end(); ++it)
    {
        Player* player = it->GetSource();
        if (!player || !player->IsAlive() || !IsCastingOrChanneling(player))
            continue;

        PlayerbotAI* botAI = GET_PLAYERBOT_AI(player);
        if (!botAI || !botAI->HasStrategy("sunwell", BOT_STATE_COMBAT))
            continue;

        Player* encapsulateTarget = GetFelmystEncapsulateTarget(player);
        if (ShouldMoveAwayFromFelmystEncapsulateTarget(player, felmyst, encapsulateTarget))
            botAI->RequestSpellInterrupt();
    }
}

void RequestInterruptForEredarTwinsBot(Player* player)
{
    if (!player || !player->IsAlive() || !IsCastingOrChanneling(player))
        return;

    PlayerbotAI* botAI = GET_PLAYERBOT_AI(player);
    if (botAI && botAI->HasStrategy("sunwell", BOT_STATE_COMBAT))
        botAI->RequestSpellInterrupt();
}

// The Conflagration target moves away from the raid, and a bot that pulls aggro on Alythess
// steps aside from its stack.
void RequestInterruptForEredarTwinsAlythessTargets(Creature* alythess)
{
    if (!alythess)
        return;

    ObjectGuid const conflagTargetGuid = GetEredarTwinsConflagrationTargetGuid(alythess, nullptr);
    if (!conflagTargetGuid.IsEmpty())
        RequestInterruptForEredarTwinsBot(ObjectAccessor::GetPlayer(*alythess, conflagTargetGuid));

    Unit* victim = alythess->GetVictim();
    Player* player = victim ? victim->ToPlayer() : nullptr;
    if (!IsHoldingAlythessAggro(player, alythess))
        return;

    Position const aside = GetEredarTwinsStepAsidePosition(
        alythess, player->GetPositionZ() > EREDAR_TWINS_BALCONY_Z);
    if (player->GetExactDist2d(aside) > EREDAR_TWINS_STEP_ASIDE_ARRIVAL_DISTANCE)
        RequestInterruptForEredarTwinsBot(player);
}

// A bot in a long cast can't move until it ends, so a new void zone (about to summon a fiend) stops
// the casts of bots within the fiend's clearance.
void RequestInterruptForBotsNearNewMuruVoidZone(Creature* voidZone)
{
    if (!voidZone || !voidZone->HasAura(Id(SwpSpells::SPELL_VOID_ZONE_VISUAL)))
        return;

    Map::PlayerList const& players = voidZone->GetMap()->GetPlayers();
    for (Map::PlayerList::const_iterator it = players.begin(); it != players.end(); ++it)
    {
        Player* player = it->GetSource();
        if (!player || !player->IsAlive() || !IsCastingOrChanneling(player) ||
            player->GetExactDist(voidZone) >= DARK_FIEND_CLEARANCE)
        {
            continue;
        }

        PlayerbotAI* botAI = GET_PLAYERBOT_AI(player);
        if (botAI && botAI->HasStrategy("sunwell", BOT_STATE_COMBAT))
            botAI->RequestSpellInterrupt();
    }
}

// In phase 2, where bots keep clear of fiends, a fiend's victim in a long cast stops it to move.
void RequestInterruptForMuruDarkFiendVictim(Creature* darkFiend)
{
    if (!darkFiend)
        return;

    Unit* victimUnit = darkFiend->GetVictim();
    Player* victim = victimUnit ? victimUnit->ToPlayer() : nullptr;
    if (!victim || !victim->IsAlive() || !IsCastingOrChanneling(victim) ||
        victim->GetExactDist(darkFiend) >= DARK_FIEND_CLEARANCE)
    {
        return;
    }

    InstanceScript* instance = darkFiend->GetInstanceScript();
    Creature* muru = instance ? instance->GetCreature(Id(SwpBosses::DATA_MURU)) : nullptr;
    if (IsMuruPhaseActive(muru))
        return;

    PlayerbotAI* botAI = GET_PLAYERBOT_AI(victim);
    if (botAI && botAI->HasStrategy("sunwell", BOT_STATE_COMBAT))
        botAI->RequestSpellInterrupt();
}

} // end anonymous namespace

class KalecgosPortalSpellListenerScript : public AllSpellScript
{
public:
    KalecgosPortalSpellListenerScript() : AllSpellScript("KalecgosPortalSpellListenerScript") {}

    void OnSpellCast(
        Spell* /*spell*/, Unit* caster, SpellInfo const* spellInfo, bool /*skipCheck*/) override
    {
        Player* player = caster ? caster->ToPlayer() : nullptr;
        if (!player)
            return;

        switch (spellInfo->Id)
        {
            case Id(SwpSpells::SPELL_SPECTRAL_BLAST_PORTAL):
            {
                if (PlayerbotAI* botAI = FindFirstSunwellCombatBotInGroup(player))
                    RecordSpectralBlastTarget(player, botAI);
                break;
            }
            case Id(SwpSpells::SPELL_TELEPORT_SPECTRAL):
            {
                if (FindFirstSunwellCombatBotInGroup(player))
                    RecordSpectralRealmEnter(player);
                break;
            }
            default:
                break;
        }
    }
};

class FelmystSpellListenerScript : public AllSpellScript
{
public:
    FelmystSpellListenerScript() : AllSpellScript("FelmystSpellListenerScript") {}

    void OnSpellPrepare(Spell* spell, Unit* caster, SpellInfo const* spellInfo) override
    {
        if (!caster || spellInfo->Id != Id(SwpSpells::SPELL_ENCAPSULATE))
            return;

        if (Player* target = GetFirstPlayerSpellTarget(spell, caster))
        {
            if (!FindFirstSunwellCombatBotInGroup(target))
                return;

            RecordFelmystIncomingEncapsulateTarget(target);
        }
    }

    void OnSpellCast(
        Spell* spell, Unit* caster, SpellInfo const* spellInfo, bool /*skipCheck*/) override
    {
        if (!caster || spellInfo->Id != Id(SwpSpells::SPELL_SUMMON_DEMONIC_VAPOR))
            return;

        Player* target = GetFirstPlayerSpellTarget(spell, caster);
        if (FindFirstSunwellCombatBotInGroup(target))
            RecordFelmystDemonicVaporSummon(target);

        if (!IsCastingOrChanneling(target))
            return;

        PlayerbotAI* botAI = GET_PLAYERBOT_AI(target);
        if (botAI && botAI->HasStrategy("sunwell", BOT_STATE_COMBAT))
            botAI->RequestSpellInterrupt();
    }
};

// A mage mid-cast when a Fury Mage gains Spell Fury stops the cast to steal it.
// Records a completed Conflagration cast, so its target holds position while the missile flies.
class EredarTwinsSpellListenerScript : public AllSpellScript
{
public:
    EredarTwinsSpellListenerScript() : AllSpellScript("EredarTwinsSpellListenerScript") {}

    void OnSpellCast(
        Spell* spell, Unit* caster, SpellInfo const* spellInfo, bool /*skipCheck*/) override
    {
        if (!caster || spellInfo->Id != Id(SwpSpells::SPELL_CONFLAGRATION))
            return;

        Player* target = GetFirstPlayerSpellTarget(spell, caster);
        if (target && FindFirstSunwellCombatBotInGroup(target))
            RecordEredarTwinsConflagrationImpact(target);
    }
};

class MuruSpellFurySpellListenerScript : public AllSpellScript
{
public:
    MuruSpellFurySpellListenerScript() : AllSpellScript("MuruSpellFurySpellListenerScript") {}

    void OnSpellCast(
        Spell* /*spell*/, Unit* caster, SpellInfo const* spellInfo, bool /*skipCheck*/) override
    {
        if (!caster || spellInfo->Id != Id(SwpSpells::SPELL_SPELL_FURY))
            return;

        uint32 const spellsteal = Id(SwpSpells::SPELL_SPELLSTEAL);
        Map::PlayerList const& players = caster->GetMap()->GetPlayers();
        for (Map::PlayerList::const_iterator it = players.begin(); it != players.end(); ++it)
        {
            Player* player = it->GetSource();
            if (!player || !player->IsAlive() || player->getClass() != CLASS_MAGE ||
                !player->IsWithinCombatRange(caster, RANGED_ABILITY_REACH) ||
                !IsCastingOrChanneling(player))
            {
                continue;
            }

            PlayerbotAI* botAI = GET_PLAYERBOT_AI(player);
            if (!botAI || !botAI->HasStrategy("sunwell", BOT_STATE_COMBAT))
                continue;

            if (!player->HasSpell(spellsteal) || player->HasSpellCooldown(spellsteal) ||
                player->HasAura(Id(SwpSpells::SPELL_SPELL_FURY)))
            {
                continue;
            }

            botAI->RequestSpellInterrupt();
        }
    }
};

class KiljaedenDarknessSpellListenerScript : public AllSpellScript
{
public:
    KiljaedenDarknessSpellListenerScript()
        : AllSpellScript("KiljaedenDarknessSpellListenerScript") {}

    void OnSpellCast(
        Spell* /*spell*/, Unit* caster, SpellInfo const* spellInfo, bool /*skipCheck*/) override
    {
        if (!caster || spellInfo->Id != Id(SwpSpells::SPELL_DARKNESS_OF_A_THOUSAND_SOULS))
            return;

        Map::PlayerList const& players = caster->GetMap()->GetPlayers();
        for (Map::PlayerList::const_iterator it = players.begin(); it != players.end(); ++it)
        {
            Player* player = it->GetSource();
            if (!player || !player->IsAlive() || !IsCastingOrChanneling(player) ||
                HasKiljaedenDragonAura(player))
            {
                continue;
            }

            PlayerbotAI* botAI = GET_PLAYERBOT_AI(player);
            if (botAI && botAI->HasStrategy("sunwell", BOT_STATE_COMBAT))
                botAI->RequestSpellInterrupt();
        }
    }
};

class SunwellBossUpdateScript : public AllCreatureScript
{
public:
    SunwellBossUpdateScript() : AllCreatureScript("SunwellBossUpdateScript") {}

    void OnAllCreatureUpdate(Creature* creature, uint32 /*diff*/) override
    {
        if (!creature)
            return;

        switch (creature->GetEntry())
        {
            case Id(SwpNpcs::NPC_FELMYST):
            {
                RequestInterruptForBotsNeedingFelmystMovement(creature);
                RequestInterruptForBotsWithFelmystEncapsulate(creature);
                break;
            }
            case Id(SwpNpcs::NPC_GRAND_WARLOCK_ALYTHESS):
            {
                RequestInterruptForEredarTwinsAlythessTargets(creature);
                break;
            }
            case Id(SwpNpcs::NPC_DARKNESS):
            {
                RequestInterruptForBotsNearNewMuruVoidZone(creature);
                break;
            }
            case Id(SwpNpcs::NPC_DARK_FIEND):
            {
                RequestInterruptForMuruDarkFiendVictim(creature);
                break;
            }
            default:
                break;
        }
    }
};

class KiljaedenArmageddonTargetCreatureScript : public AllCreatureScript
{
public:
    KiljaedenArmageddonTargetCreatureScript()
        : AllCreatureScript("KiljaedenArmageddonTargetCreatureScript") {}

    void OnAllCreatureUpdate(Creature* creature, uint32 /*diff*/) override
    {
        if (!creature || creature->GetEntry() != Id(SwpNpcs::NPC_ARMAGEDDON_TARGET))
            return;

        KiljaedenEncounterState const* state = FindKiljaedenState(creature->GetInstanceId());
        if (state && state->trackedArmageddonTargets.contains(creature->GetGUID()))
            return;

        bool hasSunwellStrategy = false;
        std::vector<PlayerbotAI*> botsToInterrupt;
        Map::PlayerList const& players = creature->GetMap()->GetPlayers();
        for (Map::PlayerList::const_iterator it = players.begin(); it != players.end(); ++it)
        {
            Player* player = it->GetSource();
            if (PlayerbotAI* botAI = GET_PLAYERBOT_AI(player);
                botAI && botAI->HasStrategy("sunwell", BOT_STATE_COMBAT))
            {
                hasSunwellStrategy = true;

                if (!player->IsAlive() || !IsCastingOrChanneling(player) ||
                    HasKiljaedenDragonAura(player))
                {
                    continue;
                }

                if (creature->GetExactDist2d(player) > ARMAGEDDON_SAFE_DISTANCE)
                    continue;

                botsToInterrupt.push_back(botAI);
            }
        }

        if (!hasSunwellStrategy)
            return;

        KiljaedenState(
            creature->GetInstanceId()).trackedArmageddonTargets.insert(creature->GetGUID());

        AddKiljaedenArmageddon(
            creature->GetInstanceId(), creature->GetPosition(),
            ARMAGEDDON_HAZARD_DURATION_MS, ARMAGEDDON_SAFE_DISTANCE);

        for (PlayerbotAI* botAI : botsToInterrupt)
            botAI->RequestSpellInterrupt();
    }

    void OnCreatureRemoveWorld(Creature* creature) override
    {
        if (!creature || creature->GetEntry() != Id(SwpNpcs::NPC_ARMAGEDDON_TARGET))
            return;

        if (KiljaedenEncounterState* state = FindKiljaedenState(creature->GetInstanceId()))
            state->trackedArmageddonTargets.erase(creature->GetGUID());
    }
};

void AddSC_SunwellBotScripts()
{
    // AllSpellScript
    new KalecgosPortalSpellListenerScript();
    new FelmystSpellListenerScript();
    new EredarTwinsSpellListenerScript();
    new MuruSpellFurySpellListenerScript();
    new KiljaedenDarknessSpellListenerScript();
    // AllCreatureScript
    new SunwellBossUpdateScript();
    new KiljaedenArmageddonTargetCreatureScript();
}

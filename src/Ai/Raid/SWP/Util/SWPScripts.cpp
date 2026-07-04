/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#include <unordered_set>
#include <vector>

#include "SWPData.h"
#include "SWPEncounter_Brut.h"
#include "SWPEncounter_Felmyst.h"
#include "SWPEncounter_Kalec.h"
#include "SWPEncounter_KJ.h"
#include "SWPEncounter_Muru.h"
#include "SWPEncounter_Twins.h"
#include "ObjectAccessor.h"
#include "Playerbots.h"
#include "Player.h"
#include "ScriptMgr.h"
#include "Spell.h"

using namespace SunwellHelpers;

static PlayerbotAI* FindFirstSunwellCombatBotInGroup(Player* referencePlayer)
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
        if (!member || member == referencePlayer || member->GetMapId() != SUNWELL_MAP_ID)
            continue;

        if (PlayerbotAI* botAI = GET_PLAYERBOT_AI(member);
            botAI && botAI->HasStrategy("sunwell", BOT_STATE_COMBAT))
        {
            return botAI;
        }
    }

    return nullptr;
}

static PlayerbotAI* FindFirstSunwellSurfaceCombatBotInGroup(Player* referencePlayer)
{
    if (!referencePlayer)
        return nullptr;

    if (!IsInSpectralRealm(referencePlayer))
    {
        if (PlayerbotAI* botAI = GET_PLAYERBOT_AI(referencePlayer);
            botAI && botAI->HasStrategy("sunwell", BOT_STATE_COMBAT))
        {
            return botAI;
        }
    }

    Group* group = referencePlayer->GetGroup();
    if (!group)
        return nullptr;

    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (!member || member == referencePlayer || IsInSpectralRealm(member) ||
            member->GetMapId() != SUNWELL_MAP_ID)
        {
            continue;
        }

        if (PlayerbotAI* botAI = GET_PLAYERBOT_AI(member);
            botAI && botAI->HasStrategy("sunwell", BOT_STATE_COMBAT))
        {
            return botAI;
        }
    }

    return nullptr;
}

static Player* GetFirstPlayerSpellTarget(Spell* spell, Unit* caster)
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

static void RequestInterruptForBotsNeedingFelmystFogMovement(
    Unit* contextUnit, Player* groupReference)
{
    if (!contextUnit)
        return;

    Group* group = nullptr;
    if (groupReference)
        group = groupReference->GetGroup();

    Map::PlayerList const& players = contextUnit->GetMap()->GetPlayers();

    for (Map::PlayerList::const_iterator it = players.begin(); it != players.end(); ++it)
    {
        Player* player = it->GetSource();
        if (!player || !player->IsAlive() || (group && player->GetGroup() != group))
            continue;

        PlayerbotAI* botAI = GET_PLAYERBOT_AI(player);
        if (!botAI || !botAI->HasStrategy("sunwell", BOT_STATE_COMBAT))
            continue;

        Unit* felmyst = PAI_VALUE2(Unit*, "find target", "felmyst");
        if (!felmyst || !felmyst->IsFlying())
            continue;

        FelmystFogOfCorruptionState fogState;
        if (!TryGetActiveFelmystFogOfCorruptionState(player, felmyst, fogState))
            continue;

        std::array<Position, 3> destinations;
        uint8 destinationCount = 0;
        if (!TryGetFelmystFogSafeDestinations(
                player, fogState.lane, destinations, destinationCount))
        {
            continue;
        }

        botAI->RequestSpellInterrupt();
    }
}

static void RequestInterruptForBotsWithDelayedFelmystEncapsulate(Creature* felmyst)
{
    if (!felmyst || felmyst->IsFlying())
        return;

    Map::PlayerList const& players = felmyst->GetMap()->GetPlayers();
    for (Map::PlayerList::const_iterator it = players.begin(); it != players.end(); ++it)
    {
        Player* player = it->GetSource();
        if (!player || !player->IsAlive())
            continue;

        PlayerbotAI* botAI = GET_PLAYERBOT_AI(player);
        if (!botAI || !botAI->HasStrategy("sunwell", BOT_STATE_COMBAT))
            continue;

        if (!player->GetCurrentSpell(CURRENT_GENERIC_SPELL) &&
            !player->GetCurrentSpell(CURRENT_CHANNELED_SPELL))
        {
            continue;
        }

        Player* encapsulateTarget = GetFelmystEncapsulateTarget(player);
        if (!encapsulateTarget)
            continue;

        if (player != encapsulateTarget &&
            player->GetExactDist2d(encapsulateTarget) > FELMYST_ENCAPSULATE_SAFE_DISTANCE)
        {
            continue;
        }

        botAI->RequestSpellInterrupt();
    }
}

static void RequestInterruptForBotsWithDelayedEredarTwinsConflagration(Creature* alythess)
{
    if (!alythess)
        return;

    Map::PlayerList const& players = alythess->GetMap()->GetPlayers();
    for (Map::PlayerList::const_iterator it = players.begin(); it != players.end(); ++it)
    {
        Player* player = it->GetSource();
        if (!player || !player->IsAlive())
            continue;

        PlayerbotAI* botAI = GET_PLAYERBOT_AI(player);
        if (!botAI || !botAI->HasStrategy("sunwell", BOT_STATE_COMBAT))
            continue;

        if (!player->GetCurrentSpell(CURRENT_GENERIC_SPELL) &&
            !player->GetCurrentSpell(CURRENT_CHANNELED_SPELL))
        {
            continue;
        }

        if (GetEredarTwinsConflagrationTarget(player) != player)
            continue;

        botAI->RequestSpellInterrupt();
    }
}

static void TrackIncomingEredarTwinsConflagration(Creature* alythess)
{
    if (!alythess)
        return;

    Spell* currentSpell = alythess->GetCurrentSpell(CURRENT_GENERIC_SPELL);
    if (!currentSpell || !currentSpell->m_spellInfo ||
        currentSpell->m_spellInfo->Id != static_cast<uint32>(SunwellSpells::SPELL_CONFLAGRATION))
    {
        return;
    }

    Unit* unitTarget = currentSpell->m_targets.GetUnitTarget();
    if (!unitTarget)
        return;

    Player* target = unitTarget->ToPlayer();
    if (!target || !FindFirstSunwellCombatBotInGroup(target))
        return;

    RecordEredarTwinsIncomingConflagrationTarget(target);
}

class KalecgosSpellListenerScript : public AllSpellScript
{
public:
    KalecgosSpellListenerScript() : AllSpellScript("KalecgosSpellListenerScript") {}

    void OnSpellCast(
        Spell* /*spell*/, Unit* caster, SpellInfo const* spellInfo, bool /*skipCheck*/) override
    {
        switch (spellInfo->Id)
        {
            case static_cast<uint32>(SunwellSpells::SPELL_SPECTRAL_BLAST_PORTAL):
            case static_cast<uint32>(SunwellSpells::SPELL_TELEPORT_SPECTRAL):
            case static_cast<uint32>(SunwellSpells::SPELL_TELEPORT_NORMAL_REALM):
                break;

            default:
                return;
        }

        Player* player = caster->ToPlayer();
        if (!player)
            return;

        switch (spellInfo->Id)
        {
            case static_cast<uint32>(SunwellSpells::SPELL_SPECTRAL_BLAST_PORTAL):
                if (PlayerbotAI* botAI = FindFirstSunwellSurfaceCombatBotInGroup(player))
                    RecordKalecgosSpectralBlastTarget(botAI, player);
                break;

            case static_cast<uint32>(SunwellSpells::SPELL_TELEPORT_SPECTRAL):
                if (PlayerbotAI* botAI = FindFirstSunwellCombatBotInGroup(player))
                    RecordKalecgosSpectralRealmEnter(botAI, player);
                break;

            case static_cast<uint32>(SunwellSpells::SPELL_TELEPORT_NORMAL_REALM):
                if (FindFirstSunwellCombatBotInGroup(player))
                    UpdateKalecgosRealmState(player, false, getMSTime());
                break;

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
        if (spellInfo->Id != static_cast<uint32>(SunwellSpells::SPELL_ENCAPSULATE))
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
        if (spellInfo->Id == static_cast<uint32>(SunwellSpells::SPELL_FOG_OF_CORRUPTION) ||
            spellInfo->Id == static_cast<uint32>(SunwellSpells::SPELL_FELMYST_STRAFE_TOP) ||
            spellInfo->Id == static_cast<uint32>(SunwellSpells::SPELL_FELMYST_STRAFE_MIDDLE) ||
            spellInfo->Id == static_cast<uint32>(SunwellSpells::SPELL_FELMYST_STRAFE_BOTTOM))
        {
            Player* targetPlayer = GetFirstPlayerSpellTarget(spell, caster);
            Player* groupReference = targetPlayer;
            if (Player* casterPlayer = caster->ToPlayer())
                groupReference = casterPlayer;

            if (groupReference)
            {
                if (!FindFirstSunwellCombatBotInGroup(groupReference))
                    return;
            }
            else
            {
                bool hasSunwellStrategy = false;
                Map::PlayerList const& players = caster->GetMap()->GetPlayers();
                for (Map::PlayerList::const_iterator it = players.begin(); it != players.end(); ++it)
                {
                    Player* player = it->GetSource();
                    if (PlayerbotAI* botAI = GET_PLAYERBOT_AI(player);
                        botAI && botAI->HasStrategy("sunwell", BOT_STATE_COMBAT))
                    {
                        hasSunwellStrategy = true;
                        break;
                    }
                }

                if (!hasSunwellStrategy)
                    return;
            }

            RequestInterruptForBotsNeedingFelmystFogMovement(caster, groupReference);

            return;
        }

        Player* target = GetFirstPlayerSpellTarget(spell, caster);
        if (!target)
            return;

        switch (spellInfo->Id)
        {
            case static_cast<uint32>(SunwellSpells::SPELL_ENCAPSULATE):
                if (!FindFirstSunwellCombatBotInGroup(target))
                    return;

                RecordFelmystIncomingEncapsulateTarget(target);
                break;

            case static_cast<uint32>(SunwellSpells::SPELL_SUMMON_DEMONIC_VAPOR):
                if (PlayerbotAI* botAI = GET_PLAYERBOT_AI(target);
                    botAI && botAI->HasStrategy("sunwell", BOT_STATE_COMBAT))
                {
                    botAI->RequestSpellInterrupt();
                }
                break;

            default:
                break;
        }
    }
};

class EredarTwinsSpellListenerScript : public AllSpellScript
{
public:
    EredarTwinsSpellListenerScript() : AllSpellScript("EredarTwinsSpellListenerScript") {}

    void OnSpellCast(
        Spell* spell, Unit* caster, SpellInfo const* spellInfo, bool /*skipCheck*/) override
    {
        if (caster->GetEntry() != static_cast<uint32>(SunwellNpcs::NPC_GRAND_WARLOCK_ALYTHESS) ||
            spellInfo->Id != static_cast<uint32>(SunwellSpells::SPELL_CONFLAGRATION))
        {
            return;
        }

        Player* target = GetFirstPlayerSpellTarget(spell, caster);
        if (!target || !FindFirstSunwellCombatBotInGroup(target))
            return;

        RecordEredarTwinsIncomingConflagrationTarget(target);
    }
};

class KiljaedenArmageddonTargetTrackerScript : public AllCreatureScript
{
public:
    KiljaedenArmageddonTargetTrackerScript()
        : AllCreatureScript("KiljaedenArmageddonTargetTrackerScript") {}

    void OnAllCreatureUpdate(Creature* creature, uint32 /*diff*/) override
    {
        if (!creature ||
            creature->GetEntry() != static_cast<uint32>(SunwellNpcs::NPC_ARMAGEDDON_TARGET))
        {
            return;
        }

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

                if (!player->IsAlive() || HasKiljaedenDragonAura(player))
                    continue;

                if (creature->GetExactDist2d(player) > KILJAEDEN_ARMAGEDDON_SAFE_DISTANCE)
                    continue;

                botsToInterrupt.push_back(botAI);
            }
        }

        if (!hasSunwellStrategy ||
            !kiljaedenTrackedArmageddonTargets.insert(creature->GetGUID()).second)
        {
            return;
        }

        AddKiljaedenArmageddon(
            creature->GetInstanceId(), creature->GetPosition(),
            KILJAEDEN_ARMAGEDDON_HAZARD_DURATION_MS, KILJAEDEN_ARMAGEDDON_SAFE_DISTANCE);

        for (PlayerbotAI* botAI : botsToInterrupt)
            botAI->RequestSpellInterrupt();
    }

    void OnCreatureRemoveWorld(Creature* creature) override
    {
        if (!creature ||
            creature->GetEntry() != static_cast<uint32>(SunwellNpcs::NPC_ARMAGEDDON_TARGET))
        {
            return;
        }

        kiljaedenTrackedArmageddonTargets.erase(creature->GetGUID());
    }
};

class SunwellDelayedInterruptScript : public AllCreatureScript
{
public:
    SunwellDelayedInterruptScript() : AllCreatureScript("SunwellDelayedInterruptScript") {}

    void OnAllCreatureUpdate(Creature* creature, uint32 /*diff*/) override
    {
        if (!creature)
            return;

        switch (creature->GetEntry())
        {
            case static_cast<uint32>(SunwellNpcs::NPC_FELMYST):
                RequestInterruptForBotsNeedingFelmystFogMovement(creature, nullptr);
                RequestInterruptForBotsWithDelayedFelmystEncapsulate(creature);
                break;

            case static_cast<uint32>(SunwellNpcs::NPC_GRAND_WARLOCK_ALYTHESS):
                TrackIncomingEredarTwinsConflagration(creature);
                RequestInterruptForBotsWithDelayedEredarTwinsConflagration(creature);
                break;

            default:
                break;
        }
    }
};

class KiljaedenSpellListenerScript : public AllSpellScript
{
public:
    KiljaedenSpellListenerScript() : AllSpellScript("KiljaedenSpellListenerScript") {}

    void OnSpellCast(
        Spell* /*spell*/, Unit* caster, SpellInfo const* spellInfo, bool /*skipCheck*/) override
    {
        if (spellInfo->Id == static_cast<uint32>(SunwellSpells::SPELL_DARKNESS_OF_A_THOUSAND_SOULS))
        {
            Map::PlayerList const& players = caster->GetMap()->GetPlayers();
            for (Map::PlayerList::const_iterator it = players.begin(); it != players.end(); ++it)
            {
                Player* player = it->GetSource();
                if (!player || !player->IsAlive() || HasKiljaedenDragonAura(player))
                    continue;

                PlayerbotAI* botAI = GET_PLAYERBOT_AI(player);
                if (!botAI || !botAI->HasStrategy("sunwell", BOT_STATE_COMBAT))
                    continue;

                if (PAI_VALUE2(Unit*, "find target", "kil'jaeden") != caster)
                    continue;

                botAI->RequestSpellInterrupt();
            }

            return;
        }
    }
};

void AddSC_SunwellPlateauBotScripts()
{
    new KalecgosSpellListenerScript();
    new FelmystSpellListenerScript();
    new EredarTwinsSpellListenerScript();
    new SunwellDelayedInterruptScript();
    new KiljaedenArmageddonTargetTrackerScript();
    new KiljaedenSpellListenerScript();
}

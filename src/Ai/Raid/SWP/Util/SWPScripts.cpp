/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "ObjectAccessor.h"
#include "Playerbots.h"
#include "Player.h"
#include "ScriptMgr.h"
#include "Spell.h"
#include "SWPData.h"
#include "SWPEncounter_Brut.h"
#include "SWPEncounter_Felmyst.h"
#include "SWPEncounter_Kalec.h"
#include "SWPEncounter_KJ.h"
#include "SWPEncounter_Muru.h"
#include "SWPEncounter_Twins.h"
#include <unordered_set>
#include <vector>

using namespace SwpHelpers;

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

        FogOfCorruptionState fogState;
        if (!TryGetActiveFogOfCorruptionState(player, felmyst, fogState))
            continue;

        Position ignored;
        if (!TryGetFelmystFogSafeDestination(player, fogState.lane, ignored))
            continue;

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

        constexpr float safeDistance = 20.0f;
        if (player != encapsulateTarget &&
            player->GetExactDist2d(encapsulateTarget) > safeDistance)
        {
            continue;
        }

        botAI->RequestSpellInterrupt();
    }
}

static void RequestInterruptForEredarTwinsAlythessTargets(Creature* alythess)
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

        if (GetEredarTwinsConflagrationTarget(player) == player ||
            (GetEredarTwinsBlazeTarget(player) == player && PlayerbotAI::IsRanged(player)))
        {
            botAI->RequestSpellInterrupt();
        }
    }
}

class KalecgosSpellListenerScript : public AllSpellScript
{
public:
    KalecgosSpellListenerScript() : AllSpellScript("KalecgosSpellListenerScript") {}

    void OnSpellCast(
        Spell* /*spell*/, Unit* caster, SpellInfo const* spellInfo, bool /*skipCheck*/) override
    {
        Player* player = caster->ToPlayer();
        if (!player)
            return;

        switch (spellInfo->Id)
        {
            case Id(SwpSpells::SPELL_SPECTRAL_BLAST_PORTAL):
                if (PlayerbotAI* botAI = FindFirstSunwellCombatBotInGroup(player))
                    RecordSpectralBlastTarget(player, botAI);
                break;

            case Id(SwpSpells::SPELL_TELEPORT_SPECTRAL):
                if (FindFirstSunwellCombatBotInGroup(player))
                    RecordSpectralRealmEnter(player);
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
        if (spellInfo->Id != Id(SwpSpells::SPELL_ENCAPSULATE))
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
        if (spellInfo->Id == Id(SwpSpells::SPELL_FOG_OF_CORRUPTION) ||
            spellInfo->Id == Id(SwpSpells::SPELL_FELMYST_STRAFE_TOP) ||
            spellInfo->Id == Id(SwpSpells::SPELL_FELMYST_STRAFE_MIDDLE) ||
            spellInfo->Id == Id(SwpSpells::SPELL_FELMYST_STRAFE_BOTTOM))
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
                for (Map::PlayerList::const_iterator it = players.begin();
                     it != players.end(); ++it)
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
            case Id(SwpSpells::SPELL_SUMMON_DEMONIC_VAPOR):
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

    void OnSpellPrepare(Spell* spell, Unit* caster, SpellInfo const* spellInfo) override
    {
        if (caster->GetEntry() != Id(SwpNpcs::NPC_GRAND_WARLOCK_ALYTHESS))
            return;

        Player* target = GetFirstPlayerSpellTarget(spell, caster);
        if (!target || !FindFirstSunwellCombatBotInGroup(target))
            return;

        if (spellInfo->Id == Id(SwpSpells::SPELL_CONFLAGRATION))
            RecordIncomingEredarTwinsConflagrationTarget(target);
        else if (spellInfo->Id == Id(SwpSpells::SPELL_BLAZE))
            RecordEredarTwinsBlazeTarget(target);
    }
};

class KiljaedenSpellListenerScript : public AllSpellScript
{
public:
    KiljaedenSpellListenerScript() : AllSpellScript("KiljaedenSpellListenerScript") {}

    void OnSpellCast(
        Spell* /*spell*/, Unit* caster, SpellInfo const* spellInfo, bool /*skipCheck*/) override
    {
        if (spellInfo->Id == Id(SwpSpells::SPELL_DARKNESS_OF_A_THOUSAND_SOULS))
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
                RequestInterruptForBotsNeedingFelmystFogMovement(creature, nullptr);
                RequestInterruptForBotsWithDelayedFelmystEncapsulate(creature);
                break;

            case Id(SwpNpcs::NPC_GRAND_WARLOCK_ALYTHESS):
                RequestInterruptForEredarTwinsAlythessTargets(creature);
                break;

            default:
                break;
        }
    }
};

class KiljaedenArmageddonTargetTrackerScript : public AllCreatureScript
{
public:
    KiljaedenArmageddonTargetTrackerScript()
        : AllCreatureScript("KiljaedenArmageddonTargetTrackerScript") {}

    void OnAllCreatureUpdate(Creature* creature, uint32 /*diff*/) override
    {
        if (!creature || creature->GetEntry() != Id(SwpNpcs::NPC_ARMAGEDDON_TARGET))
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
        if (!creature || creature->GetEntry() != Id(SwpNpcs::NPC_ARMAGEDDON_TARGET))
            return;

        kiljaedenTrackedArmageddonTargets.erase(creature->GetGUID());
    }
};

void AddSC_SunwellPlateauBotScripts()
{
    new KalecgosSpellListenerScript();
    new FelmystSpellListenerScript();
    new EredarTwinsSpellListenerScript();
    new KiljaedenSpellListenerScript();
    new SunwellBossUpdateScript();
    new KiljaedenArmageddonTargetTrackerScript();
}

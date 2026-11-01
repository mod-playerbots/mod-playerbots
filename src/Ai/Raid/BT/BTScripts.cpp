/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "AllSpellScript.h"
#include "BTHelpers.h"
#include "InstanceScript.h"
#include "Pet.h"
#include "Playerbots.h"
#include "Spell.h"

using namespace BtHelpers;

// A volcano erupts the moment it appears, on Supremus's target. A bot mid-cast where it will burn
// stops the cast so it can flee.
class SupremusVolcanicEruptionSpellListenerScript : public AllSpellScript
{
public:
    SupremusVolcanicEruptionSpellListenerScript()
        : AllSpellScript("SupremusVolcanicEruptionSpellListenerScript") {}

    void OnSpellCast(
        Spell* spell, Unit* caster, SpellInfo const* spellInfo, bool /*skipCheck*/) override
    {
        if (!caster || spellInfo->Id != Id(BtSpells::SPELL_VOLCANIC_ERUPTION))
            return;

        Unit* target = spell->m_targets.GetUnitTarget();
        if (!target)
            return;

        Map::PlayerList const& players = caster->GetMap()->GetPlayers();
        for (Map::PlayerList::const_iterator it = players.begin(); it != players.end(); ++it)
        {
            Player* player = it->GetSource();
            if (!player || !player->IsAlive() ||
                player->GetExactDist2d(target) >= SUPREMUS_VOLCANO_HAZARD_RADIUS)
            {
                continue;
            }

            Spell* ownCast = player->GetCurrentSpell(CURRENT_GENERIC_SPELL);
            if (!ownCast || ownCast->getState() != SPELL_STATE_PREPARING ||
                ownCast->GetCastTimeRemaining() <= 0)
            {
                continue;
            }

            PlayerbotAI* botAI = GET_PLAYERBOT_AI(player);
            if (!botAI || !botAI->HasStrategy("blacktemple", BOT_STATE_COMBAT))
                continue;

            botAI->RequestSpellInterrupt();
        }
    }
};

// A mage mid-cast when Essence of Desire gains Rune Shield stops the cast to steal the shield.
class ReliquaryOfSoulsRuneShieldSpellListenerScript : public AllSpellScript
{
public:
    ReliquaryOfSoulsRuneShieldSpellListenerScript()
        : AllSpellScript("ReliquaryOfSoulsRuneShieldSpellListenerScript") {}

    void OnSpellCast(
        Spell* /*spell*/, Unit* caster, SpellInfo const* spellInfo, bool /*skipCheck*/) override
    {
        if (!caster || spellInfo->Id != Id(BtSpells::SPELL_RUNE_SHIELD))
            return;

        uint32 const spellsteal = Id(BtSpells::SPELL_SPELLSTEAL);
        Map::PlayerList const& players = caster->GetMap()->GetPlayers();
        for (Map::PlayerList::const_iterator it = players.begin(); it != players.end(); ++it)
        {
            Player* player = it->GetSource();
            if (!player || !player->IsAlive() || player->getClass() != CLASS_MAGE ||
                !player->IsWithinCombatRange(caster, SPELLSTEAL_RANGE))
            {
                continue;
            }

            if (!player->GetCurrentSpell(CURRENT_GENERIC_SPELL) &&
                !player->GetCurrentSpell(CURRENT_CHANNELED_SPELL))
            {
                continue;
            }

            PlayerbotAI* botAI = GET_PLAYERBOT_AI(player);
            if (!botAI || !botAI->HasStrategy("blacktemple", BOT_STATE_COMBAT))
                continue;

            // A stolen Rune Shield can still be up when the next is cast.
            if (!player->HasSpell(spellsteal) || player->HasSpellCooldown(spellsteal) ||
                player->HasAura(Id(BtSpells::SPELL_RUNE_SHIELD)))
            {
                continue;
            }

            botAI->RequestSpellInterrupt();
        }
    }
};

// During a period when melee cannot interrupt Malande (Blessing of Protection, or a hazard patch on
// her), ranged that can interrupt Lady Malande's Circle of Healing stop their own casts to do it.
class IllidariCouncilCircleOfHealingSpellListenerScript : public AllSpellScript
{
public:
    IllidariCouncilCircleOfHealingSpellListenerScript()
        : AllSpellScript("IllidariCouncilCircleOfHealingSpellListenerScript") {}

    void OnSpellPrepare(Spell* spell, Unit* caster, SpellInfo const* spellInfo) override
    {
        if (!caster || spellInfo->Id != Id(BtSpells::SPELL_CIRCLE_OF_HEALING) ||
            caster->GetEntry() != Id(BtNpcs::NPC_LADY_MALANDE))
        {
            return;
        }

        InstanceScript* instance = caster->GetInstanceScript();
        if (!instance)
            return;

        Creature* gathios = instance->GetCreature(Id(BtCreatureData::DATA_GATHIOS_THE_SHATTERER));
        Creature* zerevor =
            instance->GetCreature(Id(BtCreatureData::DATA_HIGH_NETHERMANCER_ZEREVOR));
        if (!ShouldRangedInterruptCircleOfHealing(
                caster, gathios && gathios->IsAlive() ? gathios : nullptr,
                zerevor && zerevor->IsAlive() ? zerevor : nullptr))
        {
            return;
        }

        int32 const latestCastEnd = spell->GetCastTime() - CIRCLE_OF_HEALING_CASTER_MARGIN_MS;
        Map::PlayerList const& players = caster->GetMap()->GetPlayers();
        for (Map::PlayerList::const_iterator it = players.begin(); it != players.end(); ++it)
        {
            Player* player = it->GetSource();
            if (!player || !player->IsAlive())
                continue;

            Spell* ownCast = player->GetCurrentSpell(CURRENT_GENERIC_SPELL);
            if (!ownCast || ownCast->getState() != SPELL_STATE_PREPARING ||
                ownCast->GetCastTimeRemaining() <= latestCastEnd)
            {
                continue;
            }

            PlayerbotAI* botAI = GET_PLAYERBOT_AI(player);
            if (!botAI || !botAI->HasStrategy("blacktemple", BOT_STATE_COMBAT) ||
                !PlayerbotAI::IsRangedDps(player) || IsZerevorMageTank(botAI) ||
                !CanInterrupt(player, caster))
            {
                continue;
            }

            botAI->RequestSpellInterrupt();
        }
    }

private:
    static bool CanInterrupt(Player* player, Unit* malande)
    {
        uint32 interrupt = 0;
        float range = 0.0f;
        switch (player->getClass())
        {
            case CLASS_MAGE:
            {
                interrupt = Id(BtSpells::SPELL_COUNTERSPELL);
                range = COUNTERSPELL_RANGE;
                break;
            }
            case CLASS_SHAMAN:
            {
                interrupt = Id(BtSpells::SPELL_WIND_SHEAR);
                range = WIND_SHEAR_RANGE;
                break;
            }
            case CLASS_WARLOCK:
                return GetReadySpellLock(player->GetPet(), malande) != 0;
            default:
                return false;
        }

        return player->HasSpell(interrupt) && !player->HasSpellCooldown(interrupt) &&
            player->IsWithinCombatRange(malande, range) && player->IsWithinLOSInMap(malande);
    }
};

// The mechanic tracker records phase changes on its own ticks, which, like all other actions, are
// rejected while the bot is casting. Therefore, record Demon Form at the transformation and count
// the phase from there.
class IllidanStormrageDemonTransformSpellListenerScript : public AllSpellScript
{
public:
    IllidanStormrageDemonTransformSpellListenerScript()
        : AllSpellScript("IllidanStormrageDemonTransformSpellListenerScript") {}

    void OnSpellCast(
        Spell* /*spell*/, Unit* caster, SpellInfo const* spellInfo, bool /*skipCheck*/) override
    {
        if (!caster || spellInfo->Id != Id(BtSpells::SPELL_DEMON_TRANSFORM_1) ||
            caster->GetEntry() != Id(BtNpcs::NPC_ILLIDAN_STORMRAGE))
        {
            return;
        }

        // Only where the tracker already keeps state, so a raid without bots leaves none.
        if (FindBtState(caster->GetInstanceId()))
            RecordIllidanPhase(caster->GetInstanceId(), IllidanPhase::Demon);
    }
};

void AddSC_BlackTempleBotScripts()
{
    new SupremusVolcanicEruptionSpellListenerScript();
    new ReliquaryOfSoulsRuneShieldSpellListenerScript();
    new IllidariCouncilCircleOfHealingSpellListenerScript();
    new IllidanStormrageDemonTransformSpellListenerScript();
}

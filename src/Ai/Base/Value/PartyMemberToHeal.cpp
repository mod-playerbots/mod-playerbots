/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "PartyMemberToHeal.h"
#include "BattlegroundEY.h"
#include "BattlegroundWS.h"
#include "Playerbots.h"
#include "ServerFacade.h"

class IsTargetOfHealingSpell : public SpellEntryPredicate
{
public:
    bool Check(SpellInfo const* spellInfo) override
    {
        for (uint8 i = 0; i < 3; ++i)
        {
            if (spellInfo->Effects[i].Effect == SPELL_EFFECT_HEAL ||
                spellInfo->Effects[i].Effect == SPELL_EFFECT_HEAL_MAX_HEALTH ||
                spellInfo->Effects[i].Effect == SPELL_EFFECT_HEAL_MECHANICAL)
                return true;
        }

        return false;
    }
};

Unit* PartyMemberToHeal::Calculate()
{
    IsTargetOfHealingSpell predicate;

    Group* group = bot->GetGroup();
    Battleground* bg = bot->GetBattleground();
    bool isWarsong = bg && bg->GetBgTypeID(true) == BATTLEGROUND_WS;
    ObjectGuid carrierGuid;
    if (isWarsong && bg->GetStatus() == STATUS_IN_PROGRESS && PlayerbotAI::IsHeal(bot))
        carrierGuid = static_cast<BattlegroundWS*>(bg)->GetFlagPickerGUID(bg->GetOtherTeamId(bot->GetTeamId()));
    if (!group)
        return bot;

    bool isRaid = group->isRaidGroup();
    MinValueCalculator calc(100);

    // If focus heal targets strategy is active, only heal those targets
    if (botAI->HasStrategy("focus heal targets", BOT_STATE_COMBAT))
    {
        std::list<ObjectGuid> const focusHealTargets =
            AI_VALUE(std::list<ObjectGuid>, "focus heal targets");

        for (ObjectGuid const& focusHealTarget : focusHealTargets)
        {
            Player* player = ObjectAccessor::FindPlayer(focusHealTarget);
            if (!player || !player->IsInWorld() || !player->IsAlive() || !player->IsInSameGroupWith(bot))
                continue;

            float health = player->GetHealthPct();
            if (isRaid || health < sPlayerbotAIConfig.MediumHealth ||
                !IsTargetOfSpellCast(player, predicate))
            {
                float probeValue = 100.0f;
                if (player->GetDistance2d(bot) > sPlayerbotAIConfig.HealDistance)
                    probeValue = health + 30.0f;
                else
                    probeValue = health + player->GetDistance2d(bot) / 10.0f;

                if (probeValue < calc.minValue && Check(player))
                    calc.probe(probeValue, player);
            }
        }

        return (Unit*)calc.param;
    }

    for (GroupReference* gref = group->GetFirstMember(); gref; gref = gref->next())
    {
        Player* player = gref->GetSource();
        if (!player || player->IsGameMaster())
            continue;
        if (player && player->IsAlive())
        {
            float health = player->GetHealthPct();
            if (isRaid || health < sPlayerbotAIConfig.MediumHealth || !IsTargetOfSpellCast(player, predicate))
            {
                float probeValue = 100.0f;
                if (player->GetDistance2d(bot) > sPlayerbotAIConfig.HealDistance)
                {
                    probeValue = health + 30.0f;
                }
                else
                {
                    probeValue = health + player->GetDistance2d(bot) / 10.0f;
                }
                constexpr float carrierPriority = 15.0f;
                if (!carrierGuid.IsEmpty() && player->GetGUID() == carrierGuid &&
                    player->GetHealthPct() < sPlayerbotAIConfig.MediumHealth &&
                    player->GetDistance2d(bot) <= sPlayerbotAIConfig.HealDistance && player->IsInWorld() &&
                    player->GetMap() == bot->GetMap())
                    probeValue -= carrierPriority;
                // delay Check player to here for better performance
                if (probeValue < calc.minValue && Check(player))
                    calc.probe(probeValue, player);
            }
        }

        Pet* pet = player->GetPet();
        if (pet && pet->IsAlive())
        {
            float health = ((Unit*)pet)->GetHealthPct();
            float probeValue = 100.0f;
            if (isRaid || health < sPlayerbotAIConfig.MediumHealth)
                probeValue = health + 30.0f;
            // delay Check pet to here for better performance
            if (probeValue < calc.minValue && Check(pet))
            {
                calc.probe(probeValue, pet);
            }
        }

        Unit* charm = player->GetCharm();
        if (charm && charm->IsAlive())
        {
            float health = charm->GetHealthPct();
            float probeValue = 100.0f;
            if (isRaid || health < sPlayerbotAIConfig.MediumHealth)
                probeValue = health + 30.0f;
            // delay Check charm to here for better performance
            if (probeValue < calc.minValue && Check(charm))
            {
                calc.probe(probeValue, charm);
            }
        }
    }
    return static_cast<Unit*>(calc.param);
}

bool PartyMemberToHeal::Check(Unit* player)
{
    // return player && player != bot && player->GetMapId() == bot->GetMapId() && player->IsInWorld() &&
    //     ServerFacade::instance().GetDistance2d(bot, player) < (player->IsPlayer() && PlayerbotAI::IsTank((Player*)player) ? 50.0f
    //     : 40.0f);
    return player->GetMapId() == bot->GetMapId() && !player->IsCharmed() &&
           bot->GetDistance2d(player) < sPlayerbotAIConfig.HealDistance * 2 && bot->IsWithinLOSInMap(player);
}

Unit* HealerLowMana::Calculate()
{
    Group* group = bot->GetGroup();
    if (!group)
        return nullptr;

    MinValueCalculator calc(100);

    for (GroupReference* gref = group->GetFirstMember(); gref; gref = gref->next())
    {
        Player* player = gref->GetSource();
        if (!player || player == bot)
            continue;
        if (player->IsGameMaster() || !player->IsAlive())
            continue;
        if (!PlayerbotAI::IsHeal(player))
            continue;

        float mana = player->GetPowerPct(POWER_MANA);
        if (mana < calc.minValue)
            calc.probe(mana, player);
    }

    return (Unit*)calc.param;
}

Unit* PartyMemberToProtect::Calculate()
{
    Group* group = bot->GetGroup();
    if (!group)
        return nullptr;

    constexpr float protectHealthPct = 30.0f;
    MinValueCalculator calc(protectHealthPct);

    for (GroupReference* gref = group->GetFirstMember(); gref; gref = gref->next())
    {
        Player* player = gref->GetSource();
        if (!player || player == bot)
            continue;
        if (player->IsGameMaster() || !player->IsAlive())
            continue;
        if (player->HasAura(BG_WS_SPELL_WARSONG_FLAG) || player->HasAura(BG_WS_SPELL_SILVERWING_FLAG) ||
            player->HasAura(BG_EY_NETHERSTORM_FLAG_SPELL))
            continue;

        float health = player->GetHealthPct();
        if (health < calc.minValue && player->IsInMap(bot) && !player->getAttackers().empty() &&
            !PlayerbotAI::IsTank(player) && Check(player))
            calc.probe(health, player);
    }

    return static_cast<Unit*>(calc.param);
}

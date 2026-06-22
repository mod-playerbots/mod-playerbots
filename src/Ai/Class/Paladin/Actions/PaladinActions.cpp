/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#include "PaladinActions.h"

#include "AiFactory.h"
#include "Event.h"
#include "GenericBuffUtils.h"
#include "PaladinBlessingHelper.h"
#include "PaladinGreaterBlessingAction.h"
#include "PaladinHelper.h"
#include "Playerbots.h"
#include "SharedDefines.h"

static bool IsBlessingTargetCandidate(Player* bot, Player* player)
{
    if (!player || !player->IsAlive() || player->GetMapId() != bot->GetMapId())
        return false;

    if (player->IsGameMaster())
        return false;

    return bot->GetDistance(player) < sPlayerbotAIConfig.spellDistance * 2 &&
           bot->IsWithinLOS(player->GetPositionX(), player->GetPositionY(),
                            player->GetPositionZ());
}

static bool IsGreaterBlessingMode(Player* bot)
{
    return ai::gbless::IsEligibleGroupForAutoBlessings(bot->GetGroup());
}

template <typename Predicate>
static Unit* FindBlessingTarget(
    Player* bot, PlayerbotAI* botAI, Predicate&& predicate)
{
    std::vector<Player*> masters;
    std::vector<Player*> healers;
    std::vector<Player*> tanks;
    std::vector<Player*> others;

    Player* master = botAI->GetMaster();
    auto addPlayer = [&](Player* player)
    {
        if (!IsBlessingTargetCandidate(bot, player))
            return;

        if (player == master)
            masters.push_back(player);
        else if (botAI->IsHeal(player))
            healers.push_back(player);
        else if (botAI->IsTank(player))
            tanks.push_back(player);
        else
            others.push_back(player);
    };

    if (Group* group = bot->GetGroup())
    {
        for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
            addPlayer(ref->GetSource());
    }
    else
    {
        addPlayer(bot);
    }

    std::vector<std::vector<Player*>*> orderedLists = {
        &masters, &healers, &tanks, &others };
    for (std::vector<Player*>* players : orderedLists)
    {
        for (Player* player : *players)
        {
            if (predicate(player))
                return player;
        }
    }

    return nullptr;
}

inline std::string const GetActualBlessingOfMight(Unit* target)
{
    if (!target->ToPlayer())
        return "blessing of might";

    uint8 tab = AiFactory::GetPlayerSpecTab(target->ToPlayer());
    switch (target->getClass())
    {
        case CLASS_MAGE:
        case CLASS_PRIEST:
        case CLASS_WARLOCK:
            return "blessing of wisdom";
            break;
        case CLASS_SHAMAN:
            if (tab == SHAMAN_TAB_ELEMENTAL || tab == SHAMAN_TAB_RESTORATION)
                return "blessing of wisdom";
            break;
        case CLASS_DRUID:
            if (tab == DRUID_TAB_RESTORATION || tab == DRUID_TAB_BALANCE)
                return "blessing of wisdom";
            break;
        case CLASS_PALADIN:
            if (tab == PALADIN_TAB_HOLY)
                return "blessing of wisdom";
            break;
    }

    return "blessing of might";
}

inline std::string const GetActualBlessingOfWisdom(Unit* target)
{
    if (!target->ToPlayer())
        return "blessing of might";

    uint8 tab = AiFactory::GetPlayerSpecTab(target->ToPlayer());
    switch (target->getClass())
    {
        case CLASS_WARRIOR:
        case CLASS_ROGUE:
        case CLASS_DEATH_KNIGHT:
        case CLASS_HUNTER:
            return "blessing of might";
            break;
        case CLASS_SHAMAN:
            if (tab == SHAMAN_TAB_ENHANCEMENT)
                return "blessing of might";
            break;
        case CLASS_DRUID:
            if (tab == DRUID_TAB_FERAL)
                return "blessing of might";
            break;
        case CLASS_PALADIN:
            if (tab == PALADIN_TAB_PROTECTION || tab == PALADIN_TAB_RETRIBUTION)
                return "blessing of might";
            break;
    }

    return "blessing of wisdom";
}

// A strategy only leads with its blessing on targets whose role it actually
// suits; everyone else falls through to the role priority below.
static bool PreferredLeadAppliesTo(
    ai::gbless::BaseBlessingCategory preferred, ai::gbless::RoleProfile role)
{
    using namespace ai::gbless;
    switch (preferred)
    {
        case BASE_KINGS:     return true;
        case BASE_MIGHT:     return role == ROLE_PHYSICAL_DPS || role == ROLE_HYBRID_DPS;
        case BASE_WISDOM:    return role == ROLE_CASTER || role == ROLE_HYBRID_DPS;
        case BASE_SANCTUARY: return ai::blessing::IsTankRoleProfile(role);
        default:             return false;
    }
}

// Highest-priority blessing (for the target's role) that this bot can cast and
// that another paladin isn't already providing, so we complement rather than
// duplicate. The strategy's preferred blessing leads where it applies.
// BASE_NONE means nothing to add.
static ai::gbless::BaseBlessingCategory GetDesiredBlessingCategory(
    Unit* target, PlayerbotAI* botAI, ai::gbless::BaseBlessingCategory preferred,
    bool tanksWantSanctuary)
{
    using namespace ai::gbless;

    Player* targetPlayer = target ? target->ToPlayer() : nullptr;
    if (!targetPlayer)
        return BASE_NONE;

    RoleProfile role = ResolveRoleProfile(targetPlayer);

    auto canProvide = [&](BaseBlessingCategory category)
    {
        return category != BASE_NONE &&
               botAI->HasSpell(BlessingSpellName(ToSingleVariant(category))) &&
               !ai::blessing::AnotherPaladinHasEqualOrStronger(botAI, target, category);
    };

    if (PreferredLeadAppliesTo(preferred, role) && canProvide(preferred))
        return preferred;

    bool promoteSanctuary = tanksWantSanctuary && ai::blessing::IsTankRoleProfile(role);
    if (promoteSanctuary && canProvide(BASE_SANCTUARY))
        return BASE_SANCTUARY;

    for (BaseBlessingCategory category : BASE_BLESSING_PRIORITIES[role].priorities)
    {
        if (promoteSanctuary && category == BASE_SANCTUARY)
            continue;
        if (canProvide(category))
            return category;
    }

    return BASE_NONE;
}

static std::string GetActualPaladinPartyBlessing(
    Unit* target, PlayerbotAI* botAI, std::string const& preferred)
{
    bool tanksWantSanctuary = !ai::blessing::GroupHasRenewedHopePriest(botAI->GetBot());
    ai::gbless::BaseBlessingCategory category = GetDesiredBlessingCategory(
        target, botAI, ai::blessing::CategoryFromName(preferred), tanksWantSanctuary);
    if (category == ai::gbless::BASE_NONE)
        return "";

    return ai::gbless::BlessingSpellName(ai::gbless::ToSingleVariant(category));
}

Unit* GetPaladinPartyBlessingTarget(
    Player* bot, PlayerbotAI* botAI, std::string const& preferred)
{
    if (IsGreaterBlessingMode(bot))
        return nullptr;

    ai::gbless::BaseBlessingCategory preferredCategory = ai::blessing::CategoryFromName(preferred);
    bool tanksWantSanctuary = !ai::blessing::GroupHasRenewedHopePriest(bot);
    return FindBlessingTarget(bot, botAI, [&](Player* player)
    {
        ai::gbless::BaseBlessingCategory category = GetDesiredBlessingCategory(
            player, botAI, preferredCategory, tanksWantSanctuary);
        return category != ai::gbless::BASE_NONE &&
               !ai::blessing::HasOwnBlessingCategory(botAI, player, category);
    });
}

bool CastBlessingOfMightAction::Execute(Event /*event*/)
{
    Unit* target = GetTarget();
    if (!target)
        return false;

    std::string castName = GetActualBlessingOfMight(target);
    return botAI->CastSpell(castName, target);
}

bool CastBlessingOfWisdomAction::Execute(Event /*event*/)
{
    Unit* target = GetTarget();
    if (!target)
        return false;

    std::string castName = GetActualBlessingOfWisdom(target);
    return botAI->CastSpell(castName, target);
}

Value<Unit*>* CastBlessingOnPartyAction::GetTargetValue()
{
    return context->GetValue<Unit*>(
        "party member without aura",
        "blessing of sanctuary,greater blessing of sanctuary,"
        "blessing of kings,greater blessing of kings,"
        "blessing of might,greater blessing of might,"
        "blessing of wisdom,greater blessing of wisdom"
    );
}

Unit* CastBlessingOnPartyAction::GetTarget()
{
    return GetPaladinPartyBlessingTarget(bot, botAI, spell);
}

bool CastBlessingOnPartyAction::isUseful()
{
    Unit* target = GetTarget();
    if (!target || !target->IsInWorld() || target->GetMapId() != bot->GetMapId())
        return false;

    std::string const blessing = GetActualPaladinPartyBlessing(target, botAI, spell);
    return !blessing.empty() && AI_VALUE2(bool, "spell cast useful", blessing);
}

bool CastBlessingOnPartyAction::isPossible()
{
    Unit* target = GetTarget();
    if (!target)
        return false;

    std::string const blessing = GetActualPaladinPartyBlessing(target, botAI, spell);
    return !blessing.empty() && botAI->CanCastSpell(blessing, target);
}

bool CastBlessingOnPartyAction::Execute(Event /*event*/)
{
    Unit* target = GetTarget();
    if (!target)
        return false;

    std::string const blessing = GetActualPaladinPartyBlessing(target, botAI, spell);
    if (blessing.empty())
        return false;

    return botAI->CastSpell(blessing, target);
}

bool CastSealSpellAction::isUseful()
{
    return AI_VALUE2(bool, "combat", "self target");
}

Value<Unit*>* CastTurnUndeadAction::GetTargetValue()
{
    return context->GetValue<Unit*>("cc target", getName());
}

Unit* CastHandOfFreedomOnPartyAction::GetTarget()
{
    bool const selfImpaired = botAI->IsMovementImpaired(bot);
    bool const hasSelfHand =
        selfImpaired && ai::paladin::HasAnyPaladinHandFromCaster(bot, bot);

    if (!bot->GetGroup())
    {
        if (selfImpaired && !hasSelfHand)
            return bot;

        return nullptr;
    }

    if (selfImpaired && !hasSelfHand)
        return bot;

    return CastBuffSpellAction::GetTarget();
}

Value<Unit*>* CastHandOfFreedomOnPartyAction::GetTargetValue()
{
    return context->GetValue<Unit*>("party member snared target");
}

bool CastHandOfFreedomOnPartyAction::isUseful()
{
    Unit* target = GetTarget();
    if (!target)
        return false;

    return CastBuffSpellAction::isUseful() &&
           !ai::paladin::HasAnyPaladinHandFromCaster(target, bot);
}

Unit* CastRighteousDefenseAction::GetTarget()
{
    Unit* current_target = AI_VALUE(Unit*, "current target");
    if (!current_target)
        return nullptr;

    return current_target->GetVictim();
}

bool CastDivineSacrificeAction::isUseful()
{
    return GetTarget() && (GetTarget() != nullptr) && CastSpellAction::isUseful() &&
           !botAI->HasAura("divine guardian", GetTarget(), false, false, -1, true);
}

bool CastCancelDivineSacrificeAction::Execute(Event /*event*/)
{
    botAI->RemoveAura("divine sacrifice");
    return true;
}

bool CastCancelDivineSacrificeAction::isUseful()
{
    return botAI->HasAura("divine sacrifice", GetTarget(), false, true, -1, true);
}

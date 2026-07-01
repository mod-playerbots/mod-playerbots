/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#ifndef PLAYERBOTS_PALADINBLESSINGHELPER_H
#define PLAYERBOTS_PALADINBLESSINGHELPER_H

#include <algorithm>

#include "PaladinGreaterBlessingAction.h"
#include "Playerbots.h"
#include "SpellAuraEffects.h"

// Shared blessing utilities used by both the greater-blessing assignment and the
// single-blessing dispatcher: blessing-category mapping, aura presence/strength
// comparison (Improved Might/Wisdom aware), and role classification.
namespace ai::blessing
{
    using namespace ai::gbless;

    // Discipline priest Renewed Hope talent (ranks 1/2); its 3% raid damage
    // reduction doesn't stack with Blessing of Sanctuary.
    static constexpr uint32 SPELL_RENEWED_HOPE_R1 = 57470;
    static constexpr uint32 SPELL_RENEWED_HOPE_R2 = 57472;

    // True if any priest in the bot's group/raid has the Renewed Hope talent.
    inline bool GroupHasRenewedHopePriest(Player* bot)
    {
        Group* group = bot ? bot->GetGroup() : nullptr;
        if (!group)
            return false;

        for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
        {
            Player* member = ref->GetSource();
            if (member && member->getClass() == CLASS_PRIEST &&
                (member->HasAura(SPELL_RENEWED_HOPE_R1) ||
                 member->HasAura(SPELL_RENEWED_HOPE_R2)))
                return true;
        }

        return false;
    }

    inline BaseBlessingCategory CategoryFromName(std::string const& name)
    {
        if (name == "blessing of kings")     return BASE_KINGS;
        if (name == "blessing of might")     return BASE_MIGHT;
        if (name == "blessing of wisdom")    return BASE_WISDOM;
        if (name == "blessing of sanctuary") return BASE_SANCTUARY;
        return BASE_NONE;
    }

    inline bool IsTankRoleProfile(RoleProfile role)
    {
        return role == ROLE_DRUID_TANK || role == ROLE_WARRIOR_DK_TANK ||
               role == ROLE_PALADIN_TANK;
    }

    // Does the target already carry one of OUR (this bot's) blessings of this category?
    inline bool HasOwnBlessingCategory(
        PlayerbotAI* botAI, Unit* target, BaseBlessingCategory category)
    {
        for (BlessingType type : { ToSingleVariant(category), ToGreaterVariant(category) })
            if (botAI->HasAura(BlessingSpellName(type), target, false, true))  // cast by us
                return true;

        return false;
    }

    inline bool HasMyExactBlessing(PlayerbotAI* botAI, Unit* target, BlessingType type)
    {
        std::string name = BlessingSpellName(type);
        if (name.empty())
            return false;

        return botAI->HasAura(name, target, false, true);
    }

    inline int32 GetAuraStrength(Aura const* aura, AuraType auraType)
    {
        if (!aura)
            return 0;

        int32 amount = 0;
        for (uint8 effect = 0; effect < MAX_SPELL_EFFECTS; ++effect)
        {
            AuraEffect* auraEffect = aura->GetEffect(effect);
            if (!auraEffect || auraEffect->GetAuraType() != auraType)
                continue;

            amount = std::max(amount, auraEffect->GetAmount());
        }

        return amount;
    }

    inline int32 GetExistingBlessingStrength(
        PlayerbotAI* botAI, Unit* target, BaseBlessingCategory category)
    {
        if (category != BASE_MIGHT && category != BASE_WISDOM)
            return 0;

        AuraType auraType =
            category == BASE_MIGHT ? SPELL_AURA_MOD_ATTACK_POWER : SPELL_AURA_MOD_POWER_REGEN;
        int32 strongestAmount = 0;

        for (BlessingType type : { ToSingleVariant(category), ToGreaterVariant(category) })
        {
            Aura* aura = botAI->GetAura(BlessingSpellName(type), target);
            strongestAmount = std::max(strongestAmount, GetAuraStrength(aura, auraType));
        }

        return strongestAmount;
    }

    inline bool HasSameFamilyBlessing(
        PlayerbotAI* botAI, Unit* target, BaseBlessingCategory category)
    {
        for (BlessingType type : { ToSingleVariant(category), ToGreaterVariant(category) })
        {
            if (botAI->HasAura(BlessingSpellName(type), target))
                return true;
        }

        return false;
    }

    inline int32 GetBlessingCastStrength(Player* caster, BlessingType type, uint32 spellId)
    {
        if (!caster || !spellId)
            return 0;

        BaseBlessingCategory category = BaseBlessingOf(type);
        if (category != BASE_MIGHT && category != BASE_WISDOM)
            return 0;

        SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(spellId);
        if (!spellInfo)
            return 0;

        AuraType auraType =
            category == BASE_MIGHT ? SPELL_AURA_MOD_ATTACK_POWER : SPELL_AURA_MOD_POWER_REGEN;
        int32 amount = 0;
        for (uint8 effect = 0; effect < MAX_SPELL_EFFECTS; ++effect)
        {
            if (spellInfo->Effects[effect].ApplyAuraName != auraType)
                continue;

            amount = std::max(amount, spellInfo->Effects[effect].BasePoints + 1);
        }

        if (amount <= 0)
            return 0;

        switch (category)
        {
            case BASE_MIGHT:
                if (caster->HasAura(SPELL_IMPROVED_MIGHT_R2))
                    return amount * 125 / 100;
                if (caster->HasAura(SPELL_IMPROVED_MIGHT_R1))
                    return amount * 112 / 100;
                break;
            case BASE_WISDOM:
                if (caster->HasAura(SPELL_IMPROVED_WISDOM_R2))
                    return amount * 120 / 100;
                if (caster->HasAura(SPELL_IMPROVED_WISDOM_R1))
                    return amount * 110 / 100;
                break;
            default:
                break;
        }

        return amount;
    }

    // True if the target already has a same-family blessing at least as strong as
    // what this bot would cast (Improved Might/Wisdom aware). Considers any caster.
    inline bool HasEquivalentOrStrongerSameFamilyBlessing(
        PlayerbotAI* botAI, Unit* target, BlessingType castType, uint32 spellId)
    {
        BaseBlessingCategory category = BaseBlessingOf(castType);
        if (category != BASE_MIGHT && category != BASE_WISDOM)
            return HasSameFamilyBlessing(botAI, target, category);

        int32 castStrength = GetBlessingCastStrength(botAI->GetBot(), castType, spellId);
        if (castStrength <= 0)
            return false;

        return GetExistingBlessingStrength(botAI, target, category) >= castStrength;
    }

    // True if another paladin already provides an equal-or-stronger same-family
    // blessing of this category, so we complement instead of duplicating/downgrading.
    inline bool AnotherPaladinHasEqualOrStronger(
        PlayerbotAI* botAI, Unit* target, BaseBlessingCategory category)
    {
        BlessingType type = ToSingleVariant(category);
        uint32 spellId = 0;
        if (category == BASE_MIGHT || category == BASE_WISDOM)
            spellId = botAI->GetAiObjectContext()->GetValue<uint32>(
                "spell id", BlessingSpellName(type))->Get();

        return HasEquivalentOrStrongerSameFamilyBlessing(botAI, target, type, spellId) &&
               !HasOwnBlessingCategory(botAI, target, category);
    }
}

#endif

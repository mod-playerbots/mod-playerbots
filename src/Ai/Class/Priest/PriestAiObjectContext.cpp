/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#include "PriestAiObjectContext.h"

#include "GenericPriestStrategy.h"
#include "HolyPriestStrategy.h"
#include "NamedObjectContext.h"
#include "PriestActions.h"
#include "PriestNonCombatStrategy.h"
#include "PriestTriggers.h"
#include "PullStrategy.h"
#include "ShadowPriestStrategy.h"

class PriestStrategyFactoryInternal : public NamedObjectContext<Strategy>
{
public:
    PriestStrategyFactoryInternal()
    {
        creators["nc"] = &PriestStrategyFactoryInternal::nc;
        creators["pull"] = &PriestStrategyFactoryInternal::pull;
        creators["aoe"] = &PriestStrategyFactoryInternal::shadow_aoe;
        creators["shadow aoe"] = &PriestStrategyFactoryInternal::shadow_aoe;
        creators["dps debuff"] = &PriestStrategyFactoryInternal::shadow_debuff;
        creators["shadow debuff"] = &PriestStrategyFactoryInternal::shadow_debuff;
        creators["cure"] = &PriestStrategyFactoryInternal::cure;
        creators["buff"] = &PriestStrategyFactoryInternal::buff;
        creators["boost"] = &PriestStrategyFactoryInternal::boost;
        creators["rshadow"] = &PriestStrategyFactoryInternal::rshadow;
        creators["cc"] = &PriestStrategyFactoryInternal::cc;
        creators["healer dps"] = &PriestStrategyFactoryInternal::healer_dps;
    }

private:
    static Strategy* cc(PlayerbotAI* botAI) { return new PriestCcStrategy(botAI); }
    static Strategy* rshadow(PlayerbotAI* botAI) { return new PriestShadowResistanceStrategy(botAI); }
    static Strategy* boost(PlayerbotAI* botAI) { return new PriestBoostStrategy(botAI); }
    static Strategy* buff(PlayerbotAI* botAI) { return new PriestBuffStrategy(botAI); }
    static Strategy* nc(PlayerbotAI* botAI) { return new PriestNonCombatStrategy(botAI); }
    static Strategy* shadow_aoe(PlayerbotAI* botAI) { return new ShadowPriestAoeStrategy(botAI); }
    static Strategy* pull(PlayerbotAI* botAI) { return new PullStrategy(botAI, "shoot"); }
    static Strategy* shadow_debuff(PlayerbotAI* botAI) { return new ShadowPriestDebuffStrategy(botAI); }
    static Strategy* cure(PlayerbotAI* botAI) { return new PriestCureStrategy(botAI); }
    static Strategy* healer_dps(PlayerbotAI* botAI) { return new PriestHealerDpsStrategy(botAI); }
};

class PriestCombatStrategyFactoryInternal : public NamedObjectContext<Strategy>
{
public:
    PriestCombatStrategyFactoryInternal() : NamedObjectContext<Strategy>(false, true)
    {
        creators["heal"] = &PriestCombatStrategyFactoryInternal::heal;
        creators["shadow"] = &PriestCombatStrategyFactoryInternal::dps;
        creators["dps"] = &PriestCombatStrategyFactoryInternal::dps;
        creators["holy dps"] = &PriestCombatStrategyFactoryInternal::holy_dps;
        creators["holy heal"] = &PriestCombatStrategyFactoryInternal::holy_heal;
    }

private:
    static Strategy* heal(PlayerbotAI* botAI) { return new HealPriestStrategy(botAI); }
    static Strategy* dps(PlayerbotAI* botAI) { return new ShadowPriestStrategy(botAI); }
    static Strategy* holy_dps(PlayerbotAI* botAI) { return new HolyPriestStrategy(botAI); }
    static Strategy* holy_heal(PlayerbotAI* botAI) { return new HolyHealPriestStrategy(botAI); }
};

class PriestTriggerFactoryInternal : public NamedObjectContext<Trigger>
{
public:
    PriestTriggerFactoryInternal()
    {
        creators["devouring plague"] = &PriestTriggerFactoryInternal::devouring_plague;
        creators["shadow word: pain"] = &PriestTriggerFactoryInternal::shadow_word_pain;
        creators["shadow word: pain on attacker"] = &PriestTriggerFactoryInternal::shadow_word_pain_on_attacker;
        creators["dispel magic"] = &PriestTriggerFactoryInternal::dispel_magic;
        creators["dispel magic on party"] = &PriestTriggerFactoryInternal::dispel_magic_party_member;
        creators["cure disease"] = &PriestTriggerFactoryInternal::cure_disease;
        creators["party member cure disease"] = &PriestTriggerFactoryInternal::party_member_cure_disease;
        creators["power word: fortitude"] = &PriestTriggerFactoryInternal::power_word_fortitude;
        creators["power word: fortitude on party"] = &PriestTriggerFactoryInternal::power_word_fortitude_on_party;
        creators["divine spirit"] = &PriestTriggerFactoryInternal::divine_spirit;
        creators["divine spirit on party"] = &PriestTriggerFactoryInternal::divine_spirit_on_party;
        creators["inner fire"] = &PriestTriggerFactoryInternal::inner_fire;
        creators["vampiric touch"] = &PriestTriggerFactoryInternal::vampiric_touch;
        creators["vampiric touch on attacker"] = &PriestTriggerFactoryInternal::vampiric_touch_on_attacker;
        creators["shadowform"] = &PriestTriggerFactoryInternal::shadowform;
        creators["vampiric embrace"] = &PriestTriggerFactoryInternal::vampiric_embrace;
        creators["power infusion"] = &PriestTriggerFactoryInternal::power_infusion;
        creators["inner focus"] = &PriestTriggerFactoryInternal::inner_focus;
        creators["shadow protection"] = &PriestTriggerFactoryInternal::shadow_protection;
        creators["shadow protection on party"] = &PriestTriggerFactoryInternal::shadow_protection_on_party;
        creators["shackle undead"] = &PriestTriggerFactoryInternal::shackle_undead;
        creators["prayer of fortitude on party"] = &PriestTriggerFactoryInternal::prayer_of_fortitude_on_party;
        creators["prayer of spirit on party"] = &PriestTriggerFactoryInternal::prayer_of_spirit_on_party;
        creators["holy fire"] = &PriestTriggerFactoryInternal::holy_fire;
        creators["touch of weakness"] = &PriestTriggerFactoryInternal::touch_of_weakness;
        creators["hex of weakness"] = &PriestTriggerFactoryInternal::hex_of_weakness;
        creators["shadowguard"] = &PriestTriggerFactoryInternal::shadowguard;
        creators["fear ward"] = &PriestTriggerFactoryInternal::fear_ward;
        creators["feedback"] = &PriestTriggerFactoryInternal::feedback;
        creators["binding heal"] = &PriestTriggerFactoryInternal::binding_heal;
        creators["chastise"] = &PriestTriggerFactoryInternal::chastise;
        creators["silence"] = &PriestTriggerFactoryInternal::silence;
        creators["silence on enemy healer"] = &PriestTriggerFactoryInternal::silence_on_enemy_healer;
        creators["shadowfiend"] = &PriestTriggerFactoryInternal::shadowfiend;
        creators["mind sear channel check"] = &PriestTriggerFactoryInternal::mind_sear_channel_check;
    }

private:
    static Trigger* vampiric_embrace(PlayerbotAI* botAI) { return new VampiricEmbraceTrigger(botAI); }
    static Trigger* shadowform(PlayerbotAI* botAI) { return new ShadowformTrigger(botAI); }
    static Trigger* vampiric_touch(PlayerbotAI* botAI) { return new VampiricTouchTrigger(botAI); }
    static Trigger* vampiric_touch_on_attacker(PlayerbotAI* botAI) { return new VampiricTouchOnAttackerTrigger(botAI); }
    static Trigger* devouring_plague(PlayerbotAI* botAI) { return new DevouringPlagueTrigger(botAI); }
    static Trigger* shadow_word_pain(PlayerbotAI* botAI) { return new PowerWordPainTrigger(botAI); }
    static Trigger* shadow_word_pain_on_attacker(PlayerbotAI* botAI)
    {
        return new PowerWordPainOnAttackerTrigger(botAI);
    }
    static Trigger* dispel_magic(PlayerbotAI* botAI) { return new DispelMagicTrigger(botAI); }
    static Trigger* dispel_magic_party_member(PlayerbotAI* botAI) { return new DispelMagicPartyMemberTrigger(botAI); }
    static Trigger* cure_disease(PlayerbotAI* botAI) { return new CureDiseaseTrigger(botAI); }
    static Trigger* party_member_cure_disease(PlayerbotAI* botAI) { return new PartyMemberCureDiseaseTrigger(botAI); }
    static Trigger* power_word_fortitude(PlayerbotAI* botAI) { return new PowerWordFortitudeTrigger(botAI); }
    static Trigger* power_word_fortitude_on_party(PlayerbotAI* botAI)
    {
        return new PowerWordFortitudeOnPartyTrigger(botAI);
    }
    static Trigger* divine_spirit(PlayerbotAI* botAI) { return new DivineSpiritTrigger(botAI); }
    static Trigger* divine_spirit_on_party(PlayerbotAI* botAI) { return new DivineSpiritOnPartyTrigger(botAI); }
    static Trigger* inner_fire(PlayerbotAI* botAI) { return new InnerFireTrigger(botAI); }
    static Trigger* power_infusion(PlayerbotAI* botAI) { return new PowerInfusionTrigger(botAI); }
    static Trigger* inner_focus(PlayerbotAI* botAI) { return new InnerFocusTrigger(botAI); }
    static Trigger* shadow_protection_on_party(PlayerbotAI* botAI) { return new ShadowProtectionOnPartyTrigger(botAI); }
    static Trigger* shadow_protection(PlayerbotAI* botAI) { return new ShadowProtectionTrigger(botAI); }
    static Trigger* shackle_undead(PlayerbotAI* botAI) { return new ShackleUndeadTrigger(botAI); }
    static Trigger* prayer_of_fortitude_on_party(PlayerbotAI* botAI) { return new PrayerOfFortitudeTrigger(botAI); }
    static Trigger* prayer_of_spirit_on_party(PlayerbotAI* botAI) { return new PrayerOfSpiritTrigger(botAI); }
    static Trigger* feedback(PlayerbotAI* botAI) { return new FeedbackTrigger(botAI); }
    static Trigger* fear_ward(PlayerbotAI* botAI) { return new FearWardTrigger(botAI); }
    static Trigger* shadowguard(PlayerbotAI* botAI) { return new ShadowguardTrigger(botAI); }
    static Trigger* hex_of_weakness(PlayerbotAI* botAI) { return new HexOfWeaknessTrigger(botAI); }
    static Trigger* touch_of_weakness(PlayerbotAI* botAI) { return new TouchOfWeaknessTrigger(botAI); }
    static Trigger* holy_fire(PlayerbotAI* botAI) { return new HolyFireTrigger(botAI); }
    static Trigger* shadowfiend(PlayerbotAI* botAI) { return new ShadowfiendTrigger(botAI); }
    static Trigger* silence_on_enemy_healer(PlayerbotAI* botAI) { return new SilenceEnemyHealerTrigger(botAI); }
    static Trigger* silence(PlayerbotAI* botAI) { return new SilenceTrigger(botAI); }
    static Trigger* chastise(PlayerbotAI* botAI) { return new ChastiseTrigger(botAI); }
    static Trigger* binding_heal(PlayerbotAI* botAI) { return new BindingHealTrigger(botAI); }
    static Trigger* mind_sear_channel_check(PlayerbotAI* botAI) { return new MindSearChannelCheckTrigger(botAI); }
};

SharedNamedObjectContextList<Strategy> PriestAiObjectContext::sharedStrategyContexts;
SharedNamedObjectContextList<Trigger> PriestAiObjectContext::sharedTriggerContexts;
SharedNamedObjectContextList<UntypedValue> PriestAiObjectContext::sharedValueContexts;

PriestAiObjectContext::PriestAiObjectContext(PlayerbotAI* botAI)
    : AiObjectContext(botAI, sharedStrategyContexts, sharedTriggerContexts, sharedValueContexts)
{
}

void PriestAiObjectContext::BuildSharedContexts()
{
    BuildSharedStrategyContexts(sharedStrategyContexts);
    BuildSharedTriggerContexts(sharedTriggerContexts);
    BuildSharedValueContexts(sharedValueContexts);
}

void PriestAiObjectContext::BuildSharedStrategyContexts(SharedNamedObjectContextList<Strategy>& strategyContexts)
{
    AiObjectContext::BuildSharedStrategyContexts(strategyContexts);
    strategyContexts.Add(new PriestStrategyFactoryInternal());
    strategyContexts.Add(new PriestCombatStrategyFactoryInternal());
}

void PriestAiObjectContext::BuildSharedTriggerContexts(SharedNamedObjectContextList<Trigger>& triggerContexts)
{
    AiObjectContext::BuildSharedTriggerContexts(triggerContexts);
    triggerContexts.Add(new PriestTriggerFactoryInternal());
}

void PriestAiObjectContext::BuildSharedValueContexts(SharedNamedObjectContextList<UntypedValue>& valueContexts)
{
    AiObjectContext::BuildSharedValueContexts(valueContexts);
}

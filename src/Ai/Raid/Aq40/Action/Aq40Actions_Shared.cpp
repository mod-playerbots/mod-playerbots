#include "Aq40Actions.h"

#include <sstream>
#include <string>
#include <unordered_map>
#include <vector>

#include "../Aq40BossHelper.h"
#include "../Util/Aq40Helpers_Shared.h"

namespace
{
struct Aq40ManagedResistanceState
{
    bool natureCombatEnabled = false;
    bool natureNonCombatEnabled = false;
    bool shamanNatureCombatEnabled = false;
    bool priestShadowNonCombatEnabled = false;
    bool paladinShadowCombatEnabled = false;
};

enum class Aq40TwinShadowResistanceMode : uint8
{
    None = 0,
    PriestBuff,
    PaladinAura,
};

struct Aq40TwinShadowResistanceContext
{
    bool required = false;
    Aq40TwinShadowResistanceMode mode = Aq40TwinShadowResistanceMode::None;
    Player* priestProvider = nullptr;
    Player* paladinProvider = nullptr;
    std::vector<Player*> warlocks;
};

std::unordered_map<uint64, Aq40ManagedResistanceState> sManagedResistanceStateByBot;
std::unordered_map<uint64, bool> sAq40CleanupReportedDirtyByBot;

bool HasAnyManagedResistanceFlags(Aq40ManagedResistanceState const& state)
{
    return state.natureCombatEnabled || state.natureNonCombatEnabled || state.shamanNatureCombatEnabled ||
           state.priestShadowNonCombatEnabled || state.paladinShadowCombatEnabled;
}

Aq40TwinShadowResistanceContext BuildTwinShadowResistanceContext(Player* bot, PlayerbotAI* botAI)
{
    Aq40TwinShadowResistanceContext context;
    if (!bot || !botAI || !botAI->GetAiObjectContext() || !Aq40BossHelper::IsInAq40(bot))
        return context;

    GuidVector const activeUnits =
        Aq40BossHelper::GetActiveCombatUnits(botAI, botAI->GetAiObjectContext()->GetValue<GuidVector>("attackers")->Get());
    if (!Aq40BossHelper::HasAnyNamedUnit(botAI, activeUnits, { "emperor vek'lor", "emperor vek'nilash" }))
        return context;

    context.required = true;

    Group* group = bot->GetGroup();
    if (!group)
        return context;

    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (!member || !member->IsAlive() || !member->IsInWorld() || !Aq40BossHelper::IsSameInstance(bot, member))
            continue;

        if (member->getClass() == CLASS_WARLOCK)
            context.warlocks.push_back(member);

        if (member->getClass() == CLASS_PRIEST)
        {
            if (!context.priestProvider || member->GetGUID().GetRawValue() < context.priestProvider->GetGUID().GetRawValue())
                context.priestProvider = member;

            continue;
        }

        if (member->getClass() != CLASS_PALADIN)
            continue;

        if (!context.paladinProvider || member->GetGUID().GetRawValue() < context.paladinProvider->GetGUID().GetRawValue())
            context.paladinProvider = member;
    }

    if (context.priestProvider)
        context.mode = Aq40TwinShadowResistanceMode::PriestBuff;
    else if (context.paladinProvider)
        context.mode = Aq40TwinShadowResistanceMode::PaladinAura;

    return context;
}

bool IsTwinPaladinProvider(Player* bot, Aq40TwinShadowResistanceContext const& context)
{
    return bot && context.mode == Aq40TwinShadowResistanceMode::PaladinAura && context.paladinProvider == bot;
}

bool HasTwinShadowProtectionAura(PlayerbotAI* botAI, Player* target)
{
    return botAI && target &&
           (botAI->HasAura("shadow protection", target) || botAI->HasAura("prayer of shadow protection", target));
}

bool HasTwinShadowProtectionMissing(PlayerbotAI* botAI, Aq40TwinShadowResistanceContext const& context)
{
    if (!botAI || context.mode != Aq40TwinShadowResistanceMode::PriestBuff)
        return false;

    for (Player* warlockTank : context.warlocks)
    {
        if (warlockTank && !HasTwinShadowProtectionAura(botAI, warlockTank))
            return true;
    }

    return false;
}

char const* ToString(Aq40TwinShadowResistanceMode mode)
{
    switch (mode)
    {
        case Aq40TwinShadowResistanceMode::None: return "none";
        case Aq40TwinShadowResistanceMode::PriestBuff: return "priest_buff";
        case Aq40TwinShadowResistanceMode::PaladinAura: return "paladin_aura";
    }

    return "none";
}

}    // namespace

namespace Aq40Helpers
{

bool HasManagedResistanceState(Player* bot)
{
    if (!bot)
        return false;

    auto const itr = sManagedResistanceStateByBot.find(bot->GetGUID().GetRawValue());
    return itr != sManagedResistanceStateByBot.end() && HasAnyManagedResistanceFlags(itr->second);
}

bool ClearManagedResistanceStrategies(Player* bot, PlayerbotAI* botAI)
{
    if (!bot || !botAI)
        return false;

    auto itr = sManagedResistanceStateByBot.find(bot->GetGUID().GetRawValue());
    if (itr == sManagedResistanceStateByBot.end())
        return false;

    Aq40ManagedResistanceState& managedState = itr->second;
    bool cleaned = false;

    if (managedState.natureCombatEnabled && botAI->HasStrategy("rnature", BotState::BOT_STATE_COMBAT))
    {
        botAI->ChangeStrategy("-rnature", BotState::BOT_STATE_COMBAT);
        managedState.natureCombatEnabled = false;
        cleaned = true;
    }

    if (managedState.natureNonCombatEnabled && botAI->HasStrategy("rnature", BotState::BOT_STATE_NON_COMBAT))
    {
        botAI->ChangeStrategy("-rnature", BotState::BOT_STATE_NON_COMBAT);
        managedState.natureNonCombatEnabled = false;
        cleaned = true;
    }

    if (managedState.shamanNatureCombatEnabled && botAI->HasStrategy("nature resistance", BotState::BOT_STATE_COMBAT))
    {
        botAI->ChangeStrategy("-nature resistance", BotState::BOT_STATE_COMBAT);
        managedState.shamanNatureCombatEnabled = false;
        cleaned = true;
    }

    if (managedState.paladinShadowCombatEnabled && botAI->HasStrategy("rshadow", BotState::BOT_STATE_COMBAT))
    {
        botAI->ChangeStrategy("-rshadow", BotState::BOT_STATE_COMBAT);
        managedState.paladinShadowCombatEnabled = false;
        cleaned = true;
    }

    if (managedState.priestShadowNonCombatEnabled && botAI->HasStrategy("rshadow", BotState::BOT_STATE_NON_COMBAT))
    {
        botAI->ChangeStrategy("-rshadow", BotState::BOT_STATE_NON_COMBAT);
        managedState.priestShadowNonCombatEnabled = false;
        cleaned = true;
    }

    if (!HasAnyManagedResistanceFlags(managedState))
    {
        sManagedResistanceStateByBot.erase(itr);
        cleaned = true;
    }

    return cleaned;
}

}    // namespace Aq40Helpers

namespace
{

void LogAq40CleanupTransition(Player* bot, bool wasDirty)
{
    if (!bot)
        return;

    uint64 const botGuid = bot->GetGUID().GetRawValue();
    auto itr = sAq40CleanupReportedDirtyByBot.find(botGuid);
    bool const previousDirty = itr != sAq40CleanupReportedDirtyByBot.end() && itr->second;

    if (wasDirty)
    {
        if (!previousDirty)
            LOG_INFO("playerbots", "AQ40 cleanup: bot={} cleaned stale recovery state and can resume follow", bot->GetName());

        sAq40CleanupReportedDirtyByBot[botGuid] = true;
        return;
    }

    if (previousDirty)
        LOG_INFO("playerbots", "AQ40 cleanup: bot={} recovery state already clean", bot->GetName());

    sAq40CleanupReportedDirtyByBot[botGuid] = false;
}

}    // namespace

namespace Aq40BossActions
{
Unit* FindUnitByAnyName(PlayerbotAI* botAI, GuidVector const& attackers, std::initializer_list<char const*> names)
{
    return Aq40BossHelper::FindUnitByAnyName(botAI, attackers, names);
}

std::vector<Unit*> FindUnitsByAnyName(PlayerbotAI* botAI, GuidVector const& attackers,
                                      std::initializer_list<char const*> names)
{
    return Aq40BossHelper::FindUnitsByAnyName(botAI, attackers, names);
}
}    // namespace Aq40BossActions
bool Aq40ManageResistanceStrategiesAction::Execute(Event /*event*/)
{
    if (!bot)
        return false;

    Aq40ManagedResistanceState& managedState = sManagedResistanceStateByBot[bot->GetGUID().GetRawValue()];

    GuidVector const attackers = context->GetValue<GuidVector>("attackers")->Get();
    GuidVector const activeUnits = Aq40BossHelper::GetActiveCombatUnits(botAI, attackers);
    bool const inAq40 = Aq40BossHelper::IsInAq40(bot);
    bool const needNatureResistance =
        inAq40 && Aq40BossHelper::HasAnyNamedUnit(botAI, activeUnits,
            { "princess huhuran", "viscidus", "glob of viscidus", "toxic slime" });
    Aq40TwinShadowResistanceContext const twinShadowContext = BuildTwinShadowResistanceContext(bot, botAI);
    bool const needTwinShadowResistance = twinShadowContext.required &&
                                          twinShadowContext.mode != Aq40TwinShadowResistanceMode::None;

    bool acted = false;

    if (bot->getClass() == CLASS_HUNTER)
    {
        bool const hasNatureStrategyCombat = botAI->HasStrategy("rnature", BotState::BOT_STATE_COMBAT);
        bool const hasNatureStrategyNonCombat = botAI->HasStrategy("rnature", BotState::BOT_STATE_NON_COMBAT);

        if (needNatureResistance)
        {
            if (!hasNatureStrategyCombat)
            {
                botAI->ChangeStrategy("+rnature", BotState::BOT_STATE_COMBAT);
                managedState.natureCombatEnabled = true;
                acted = true;
            }
            if (!hasNatureStrategyNonCombat)
            {
                botAI->ChangeStrategy("+rnature", BotState::BOT_STATE_NON_COMBAT);
                managedState.natureNonCombatEnabled = true;
                acted = true;
            }

            if (!botAI->HasAura("aspect of the wild", bot))
                acted = botAI->DoSpecificAction("aspect of the wild", Event(), true) || acted;
        }
        else if (managedState.natureCombatEnabled || managedState.natureNonCombatEnabled)
        {
            if (managedState.natureCombatEnabled && hasNatureStrategyCombat)
            {
                botAI->ChangeStrategy("-rnature", BotState::BOT_STATE_COMBAT);
                managedState.natureCombatEnabled = false;
                acted = true;
            }
            if (managedState.natureNonCombatEnabled && hasNatureStrategyNonCombat)
            {
                botAI->ChangeStrategy("-rnature", BotState::BOT_STATE_NON_COMBAT);
                managedState.natureNonCombatEnabled = false;
                acted = true;
            }
        }
    }

    if (bot->getClass() == CLASS_SHAMAN)
    {
        bool const hasNatureTotemStrategyCombat = botAI->HasStrategy("nature resistance", BotState::BOT_STATE_COMBAT);

        if (needNatureResistance)
        {
            if (!hasNatureTotemStrategyCombat)
            {
                botAI->ChangeStrategy("+nature resistance", BotState::BOT_STATE_COMBAT);
                managedState.shamanNatureCombatEnabled = true;
                acted = true;
            }

            if (!botAI->HasAura("nature resistance totem", bot))
                acted = botAI->DoSpecificAction("nature resistance totem", Event(), true) || acted;
        }
        else if (managedState.shamanNatureCombatEnabled)
        {
            if (hasNatureTotemStrategyCombat)
            {
                botAI->ChangeStrategy("-nature resistance", BotState::BOT_STATE_COMBAT);
                acted = true;
            }
            managedState.shamanNatureCombatEnabled = false;
        }
    }

    if (bot->getClass() == CLASS_PRIEST)
    {
        bool const shouldProvideTwinShadow = twinShadowContext.mode == Aq40TwinShadowResistanceMode::PriestBuff &&
                                             twinShadowContext.priestProvider == bot;
        bool const hasShadowStrategyNonCombat = botAI->HasStrategy("rshadow", BotState::BOT_STATE_NON_COMBAT);

        if (shouldProvideTwinShadow)
        {
            if (!hasShadowStrategyNonCombat)
            {
                botAI->ChangeStrategy("+rshadow", BotState::BOT_STATE_NON_COMBAT);
                managedState.priestShadowNonCombatEnabled = true;
                acted = true;
            }

            if (HasTwinShadowProtectionMissing(botAI, twinShadowContext))
            {
                for (Player* warlockTank : twinShadowContext.warlocks)
                {
                    if (!warlockTank || HasTwinShadowProtectionAura(botAI, warlockTank))
                        continue;

                    if (botAI->CastSpell("shadow protection", warlockTank))
                    {
                        Aq40Helpers::LogAq40Info(bot, "resistance_strategy",
                            std::string("shadow:priest:") + Aq40Helpers::GetAq40LogUnit(warlockTank),
                            std::string("boss=twin shadow=1 mode=priest_buff action=shadow_protection target=") +
                                Aq40Helpers::GetAq40LogUnit(warlockTank),
                            1000);
                        acted = true;
                        break;
                    }
                }
            }
        }
        else if (managedState.priestShadowNonCombatEnabled)
        {
            if (hasShadowStrategyNonCombat)
            {
                botAI->ChangeStrategy("-rshadow", BotState::BOT_STATE_NON_COMBAT);
                acted = true;
            }

            managedState.priestShadowNonCombatEnabled = false;
        }
    }

    if (bot->getClass() == CLASS_PALADIN)
    {
        bool const shouldProvideTwinShadow = needTwinShadowResistance && IsTwinPaladinProvider(bot, twinShadowContext);
        bool const hasShadowStrategyCombat = botAI->HasStrategy("rshadow", BotState::BOT_STATE_COMBAT);

        if (shouldProvideTwinShadow)
        {
            if (!hasShadowStrategyCombat)
            {
                botAI->ChangeStrategy("+rshadow", BotState::BOT_STATE_COMBAT);
                managedState.paladinShadowCombatEnabled = true;
                acted = true;
            }

            if (!botAI->HasAura("shadow resistance aura", bot))
                acted = botAI->DoSpecificAction("shadow resistance aura", Event(), true) || acted;
        }
        else if (managedState.paladinShadowCombatEnabled)
        {
            if (hasShadowStrategyCombat)
            {
                botAI->ChangeStrategy("-rshadow", BotState::BOT_STATE_COMBAT);
                acted = true;
            }

            managedState.paladinShadowCombatEnabled = false;
        }
    }

    if (!HasAnyManagedResistanceFlags(managedState))
        sManagedResistanceStateByBot.erase(bot->GetGUID().GetRawValue());

    if (acted)
    {
        std::ostringstream fields;
        fields << "boss=resistance nature=" << (needNatureResistance ? 1 : 0)
               << " shadow=" << (needTwinShadowResistance ? 1 : 0)
               << " shadow_mode=" << ToString(twinShadowContext.mode);
        Aq40Helpers::LogAq40Info(bot, "resistance_strategy",
            std::string("nature:") + (needNatureResistance ? "1" : "0") +
                ":shadow:" + (needTwinShadowResistance ? "1" : "0") +
                ":mode:" + ToString(twinShadowContext.mode),
            fields.str());
    }

    return acted;
}

bool Aq40ManageResistanceStrategiesAction::isUseful()
{
    if (!bot)
        return false;

    auto const managedStateItr = sManagedResistanceStateByBot.find(bot->GetGUID().GetRawValue());
    Aq40ManagedResistanceState const* managedState =
        managedStateItr != sManagedResistanceStateByBot.end() ? &managedStateItr->second : nullptr;

    GuidVector const attackers = context->GetValue<GuidVector>("attackers")->Get();
    GuidVector const activeUnits = Aq40BossHelper::GetActiveCombatUnits(botAI, attackers);
    bool const inAq40 = Aq40BossHelper::IsInAq40(bot);
    bool const needNatureResistance =
        inAq40 && Aq40BossHelper::HasAnyNamedUnit(botAI, activeUnits,
            { "princess huhuran", "viscidus", "glob of viscidus", "toxic slime" });
    Aq40TwinShadowResistanceContext const twinShadowContext = BuildTwinShadowResistanceContext(bot, botAI);

    if (bot->getClass() == CLASS_HUNTER)
    {
        bool const hasNatureStrategyCombat = botAI->HasStrategy("rnature", BotState::BOT_STATE_COMBAT);
        bool const hasNatureStrategyNonCombat = botAI->HasStrategy("rnature", BotState::BOT_STATE_NON_COMBAT);
        return (needNatureResistance &&
                (!hasNatureStrategyCombat || !hasNatureStrategyNonCombat || !botAI->HasAura("aspect of the wild", bot))) ||
               (!needNatureResistance && managedState &&
                (managedState->natureCombatEnabled || managedState->natureNonCombatEnabled));
    }

    if (bot->getClass() == CLASS_SHAMAN)
    {
        bool const hasNatureTotemStrategyCombat = botAI->HasStrategy("nature resistance", BotState::BOT_STATE_COMBAT);
        return (needNatureResistance &&
                (!hasNatureTotemStrategyCombat || !botAI->HasAura("nature resistance totem", bot))) ||
               (!needNatureResistance && managedState && managedState->shamanNatureCombatEnabled);
    }

    if (bot->getClass() == CLASS_PRIEST)
    {
        bool const isPriestProvider = twinShadowContext.mode == Aq40TwinShadowResistanceMode::PriestBuff &&
                                      twinShadowContext.priestProvider == bot;
        bool const hasShadowStrategyNonCombat = botAI->HasStrategy("rshadow", BotState::BOT_STATE_NON_COMBAT);
        return (isPriestProvider && (!hasShadowStrategyNonCombat || HasTwinShadowProtectionMissing(botAI, twinShadowContext))) ||
               (!isPriestProvider && managedState && managedState->priestShadowNonCombatEnabled);
    }

    if (bot->getClass() == CLASS_PALADIN)
    {
        bool const isPaladinProvider = IsTwinPaladinProvider(bot, twinShadowContext);
        bool const hasShadowStrategyCombat = botAI->HasStrategy("rshadow", BotState::BOT_STATE_COMBAT);
        return (isPaladinProvider && (!hasShadowStrategyCombat || !botAI->HasAura("shadow resistance aura", bot))) ||
               (!isPaladinProvider && managedState && managedState->paladinShadowCombatEnabled);
    }

    return false;
}

bool Aq40EraseTimersAndTrackersAction::isUseful()
{
    return bot && bot->IsAlive() && Aq40BossHelper::IsInAq40(bot) &&
           Aq40Helpers::ShouldRunOutOfCombatMaintenance(bot, botAI);
}

bool Aq40EraseTimersAndTrackersAction::Execute(Event /*event*/)
{
    if (!bot || !Aq40BossHelper::IsInAq40(bot))
        return false;

    if (!Aq40Helpers::ShouldRunOutOfCombatMaintenance(bot, botAI))
        return false;

    bool const hadManagedResistance = Aq40Helpers::ClearManagedResistanceStrategies(bot, botAI);
    bool const hadPersistentEncounterState = Aq40Helpers::ResetEncounterState(bot);
    bool const recoveredFollowState =
        Aq40Helpers::TryRecoverAq40FollowState(bot, botAI, "encounter_reset", "shared:follow_recovery", true);
    bool const recoveredDirtyState =
        hadManagedResistance || hadPersistentEncounterState || recoveredFollowState;

    LogAq40CleanupTransition(bot, recoveredDirtyState);
    return true;
}

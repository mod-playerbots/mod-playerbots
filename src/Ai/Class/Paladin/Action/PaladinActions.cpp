/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#include "PaladinActions.h"

#include "AiFactory.h"
#include "Config.h"
#include "Event.h"
#include "GenericBuffUtils.h"
#include "PaladinHelper.h"
#include "GenericBuffUtils.h"
#include "Group.h"
#include "ObjectAccessor.h"
#include "PaladinBlessingStateValue.h"
#include "PaladinBlessingUtils.h"
#include "Player.h"
#include "PlayerbotAI.h"
#include "Playerbots.h"
#include "SharedDefines.h"

using ai::buff::MakeAuraQualifierForBuff;

// Readable tag to identify the group in the logs based on the leader
static inline std::string MakeGroupTag(Group* group)
{
    if (!group)
        return "g0";
    ObjectGuid leader = group->GetLeaderGUID();
    uint32 low = leader.GetCounter();
    return std::string("g") + std::to_string(low);
}

// Log ONLY bots whose master is the leader of their group/raid == my raid
static inline bool ShouldLogForThisBot(Player* bot)
{
    if (!bot)
        return false;

    // Skip extra work if debug logging for playerbots is disabled
    if (!sLog->ShouldLog("playerbots", LogLevel::LOG_LEVEL_DEBUG))
        return false;

    PlayerbotAI* ai = GET_PLAYERBOT_AI(bot);
    Player* master = ai ? ai->GetMaster() : nullptr;
    Group* group = bot->GetGroup();
    return master && group && (group->GetLeaderGUID() == master->GetGUID());
}

// Detect tank role on the target
static inline bool IsTankRole(Player* player)
{
    return player && PlayerbotAI::IsTank(player, /*bySpec=*/false);
}

static inline bool IsProtectionPaladin(Player* player)
{
    return player && player->getClass() == CLASS_PALADIN && AiFactory::GetPlayerSpecTab(player) == PALADIN_TAB_PROTECTION;
}

static inline bool ShouldSkipMightOnTanks(PaladinBlessingState const& blessingState)
{
    return blessingState.paladinCount == 2u && blessingState.bstats.hasWearer && blessingState.bmana.hasWearer;
}

// Target eligible for Might: Physical + Hunter + Shaman Enhancement. Tank: Only if >= 3 Paladins (3rd Blessing).
static inline bool ShouldReceiveMight(Player* targetPlayer, PaladinBlessingState const& blessingState)
{
    if (!targetPlayer)
        return false;

    if (targetPlayer->getClass() == CLASS_PALADIN &&
        AiFactory::GetPlayerSpecTab(targetPlayer) == PALADIN_TAB_PROTECTION && blessingState.paladinCount < 3u &&
        !(blessingState.paladinCount == 2u && blessingState.bstats.hasWearer && blessingState.bdps.hasWearer))
        return false;

    return ai::paladin::GetActualBlessingOfMight(targetPlayer, /*log=*/false) == "blessing of might";
}

static inline PaladinBlessingState GetBlessingState(PlayerbotAI* botAI)
{
    if (!botAI)
        return PaladinBlessingState{};

    return botAI->GetAiObjectContext()->GetValue<PaladinBlessingState>("paladin blessing state")->Get();
}

static inline uint32 GetSpellId(PlayerbotAI* botAI, std::string const& name)
{
    if (!botAI)
        return 0u;

    return botAI->GetAiObjectContext()->GetValue<uint32>("spell id", name)->Get();
}

Value<Unit*>* CastBlessingOnPartyAction::GetTargetValue()
{
    return context->GetValue<Unit*>("party member without aura", MakeAuraQualifierForBuff(spell));
}

bool CastBlessingOfMightAction::Execute(Event /*event*/)
{
    Unit* target = GetTarget();
    if (!target)
        return false;

    std::string castName = ai::paladin::GetActualBlessingOfMight(target);
    auto RP = ai::chat::MakeGroupAnnouncer(bot);

    castName = ai::buff::UpgradeToGroupIfAppropriate(bot, botAI, castName, /*announceOnMissing=*/true, RP);
    return botAI->CastSpell(castName, target);
}

Value<Unit*>* CastBlessingOfMightOnPartyAction::GetTargetValue()
{
    return context->GetValue<Unit*>(
        "party member without aura",
        "blessing of might,greater blessing of might,blessing of sanctuary,greater blessing of sanctuary");
}

bool CastBlessingOfMightOnPartyAction::Execute(Event /*event*/)
{
    PaladinBlessingState const blessingState = GetBlessingState(botAI);
    // Single-role coordinator: only one paladin with bdps does party-wide buffs
    if (!blessingState.IsDesignated(bot, PaladinBlessingRole::Bdps))
        return false;
    // Log only my bots
    if (ShouldLogForThisBot(bot))
    {
        const std::string groupTag = MakeGroupTag(bot->GetGroup());
        LOG_DEBUG("playerbots", "[RoleCoord:{}] role=bdps -> {} is allowed to pose Might", groupTag, bot->GetName());
    }

    Unit* target = GetTarget();
    if (!target)
        return false;

    // If the current target is not eligible for Might, retarget a party member who is
    Player* targetPlayer = target->ToPlayer();
    if (targetPlayer && ShouldSkipMightOnTanks(blessingState) && IsTankRole(targetPlayer))
        targetPlayer = nullptr;
    if (!targetPlayer || !ShouldReceiveMight(targetPlayer, blessingState))
    {
        Group* group = bot->GetGroup();
        if (group)
        {
            Unit* bestCandidate = nullptr;
            for (GroupReference* memberRef = group->GetFirstMember(); memberRef; memberRef = memberRef->next())
            {
                Player* memberPlayer = memberRef->GetSource();
                if (!memberPlayer || !memberPlayer->IsInWorld() || !memberPlayer->IsAlive())
                    continue;
                if (ShouldSkipMightOnTanks(blessingState) && IsTankRole(memberPlayer))
                    continue;
                if (!ShouldReceiveMight(memberPlayer, blessingState))
                    continue;
                // avoid resting Might if already present (greater/mono)
                if (botAI->HasAura("blessing of might", memberPlayer) || botAI->HasAura("greater blessing of might", memberPlayer))
                    continue;
                bestCandidate = memberPlayer;
                break;
            }
            if (bestCandidate)
            {
                target = bestCandidate;
                targetPlayer = bestCandidate->ToPlayer();
                if (ShouldLogForThisBot(bot))
                    LOG_DEBUG("playerbots", "[Might] Retargeting towards {}", targetPlayer->GetName());
            }
        }
        // if still no valid target -> abandon
        if (!targetPlayer || !ShouldReceiveMight(targetPlayer, blessingState))
            return false;
    }

    std::string castName = "blessing of might";
    auto RP = ai::chat::MakeGroupAnnouncer(bot);

    castName = ai::buff::UpgradeToGroupIfAppropriate(bot, botAI, castName, /*announceOnMissing=*/true, RP);
    return botAI->CastSpell(castName, target);
}

bool CastBlessingOfWisdomAction::Execute(Event /*event*/)
{
    Unit* target = GetTarget();
    if (!target)
        return false;

    std::string castName = ai::paladin::GetActualBlessingOfWisdom(target);
    auto RP = ai::chat::MakeGroupAnnouncer(bot);

    castName = ai::buff::UpgradeToGroupIfAppropriate(bot, botAI, castName, /*announceOnMissing=*/true, RP);
    return botAI->CastSpell(castName, target);
}

Value<Unit*>* CastBlessingOfWisdomOnPartyAction::GetTargetValue()
{
    return context->GetValue<Unit*>(
        "party member without aura",
        "blessing of wisdom,greater blessing of wisdom,blessing of sanctuary,greater blessing of sanctuary");
}

bool CastBlessingOfWisdomOnPartyAction::Execute(Event /*event*/)
{
    PaladinBlessingState const blessingState = GetBlessingState(botAI);
    // Single-role coordinator: only one paladin with "bmana" does party-wide buffs
    if (!blessingState.IsDesignated(bot, PaladinBlessingRole::Bmana))
        return false;

    if (ShouldLogForThisBot(bot))
    {
        const std::string groupTag = MakeGroupTag(bot->GetGroup());
        LOG_DEBUG("playerbots", "[RoleCoord:{}] role=bmana -> {} is allowed to buff Wisdom", groupTag, bot->GetName());
    }

    Unit* target = GetTarget();
    if (!target)
        return false;

    Player* targetPlayer = target->ToPlayer();

    if (Group* group = bot->GetGroup(); group && targetPlayer && !group->IsMember(targetPlayer->GetGUID()))
        return false;	

    if (botAI->HasStrategy("bmana", BOT_STATE_NON_COMBAT) && targetPlayer && IsTankRole(targetPlayer))
    {
        LOG_DEBUG("playerbots", "[Wisdom/bmana] Skip tank {} (Kings only)", target->GetName());
        return false;
    }

    // Force Wisdom into party mode (no redirection based on class)
    std::string castName = "blessing of wisdom";
    auto RP = ai::chat::MakeGroupAnnouncer(bot);
    castName = ai::buff::UpgradeToGroupIfAppropriate(bot, botAI, castName, /*announceOnMissing=*/true, RP);
    return botAI->CastSpell(castName, target);
}

Value<Unit*>* CastBlessingOfSanctuaryOnPartyAction::GetTargetValue()
{
    return context->GetValue<Unit*>("party member without aura", "blessing of sanctuary,greater blessing of sanctuary");
}

bool CastBlessingOfSanctuaryOnPartyAction::Execute(Event /*event*/)
{
    uint32 sanctSpellId = GetSpellId(botAI, "blessing of sanctuary");
    if (!sanctSpellId || !bot->HasSpell(sanctSpellId))
        return false;

    Unit* target = GetTarget();
    if (!target)
        target = bot;

    Player* targetPlayer = target ? target->ToPlayer() : nullptr;

    // Small helpers to check relevant auras
    const auto HasKingsAura = [&](Unit* u) -> bool
    { return botAI->HasAura("blessing of kings", u) || botAI->HasAura("greater blessing of kings", u); };
    const auto HasSanctAura = [&](Unit* u) -> bool
    { return botAI->HasAura("blessing of sanctuary", u) || botAI->HasAura("greater blessing of sanctuary", u); };

    Group* group = bot->GetGroup();
    if (group && targetPlayer && !group->IsMember(targetPlayer->GetGUID()))
    {
        LOG_DEBUG("playerbots", "[Sanct] Initial target not in group, ignoring");
        target = bot;
        targetPlayer = bot->ToPlayer();
    }

    if (Player* self = bot->ToPlayer())
    {
        bool selfHasSanct = HasSanctAura(self);
        bool needSelf = IsTankRole(self) && !selfHasSanct;

        LOG_DEBUG("playerbots", "[Sanct] {} isTank={} selfHasSanct={} needSelf={}", bot->GetName(), IsTankRole(self),
                  selfHasSanct, needSelf);

        if (needSelf)
        {
            target = self;
            targetPlayer = self;
        }
    }

    // Try to re-target a valid tank in group if needed
    bool targetOk = false;
    if (targetPlayer)
    {
        bool hasSanct = HasSanctAura(targetPlayer);
        targetOk = IsTankRole(targetPlayer) && !hasSanct;
    }

    if (Group* group = bot->GetGroup(); !targetOk && group)
    {
        for (GroupReference* memberRef = group->GetFirstMember(); memberRef; memberRef = memberRef->next())
        {
            Player* memberPlayer = memberRef->GetSource();
            if (!memberPlayer)
                continue;

            if (!memberPlayer->IsInWorld() || !memberPlayer->IsAlive())
                continue;

            if (!IsTankRole(memberPlayer))
                continue;

            bool hasSanct = HasSanctAura(memberPlayer);
            if (!hasSanct)
            {
                target = memberPlayer;  // prioritize this tank
                targetPlayer = memberPlayer;
                targetOk = true;
                break;
            }
        }
    }

    // If after retargeting we still don't have a valid tank without Sanctuary, we stop and avoid calling the
    // resolver/logger no sanct
    if (!targetOk)
        return false;

    {
        bool hasKings = HasKingsAura(target);
        bool hasSanct = HasSanctAura(target);
        bool knowSanct = bot->HasSpell(sanctSpellId);
        LOG_DEBUG("playerbots", "[Sanct] Final target={} hasKings={} hasSanct={} knowSanct={}", target->GetName(),
                  hasKings, hasSanct, knowSanct);
    }

    // For safety, if ever a non-tank arrives here
    if (targetPlayer && !IsTankRole(targetPlayer))
        return false;

    std::string castName = "blessing of sanctuary";

    bool ok = botAI->CastSpell(castName, target);
    LOG_DEBUG("playerbots", "[Sanct] Cast {} on {} result={}", castName, target->GetName(), ok);
    return ok;
}

Value<Unit*>* CastBlessingOfKingsOnPartyAction::GetTargetValue()
{
    return context->GetValue<Unit*>("party member without aura", "blessing of kings,greater blessing of kings");
}

bool CastBlessingOfKingsOnPartyAction::Execute(Event /*event*/)
{
    PaladinBlessingState const blessingState = GetBlessingState(botAI);
    // Allow Kings on party if the bot is elected for bstats OR for bmana (bmana places MONO Kings on tanks)
    const bool electedBstats = blessingState.IsDesignated(bot, PaladinBlessingRole::Bstats);
    const bool electedBmana = blessingState.IsDesignated(bot, PaladinBlessingRole::Bmana);
    const bool electedBdps = blessingState.IsDesignated(bot, PaladinBlessingRole::Bdps);
    const bool hasBstats = botAI->HasStrategy("bstats", BOT_STATE_NON_COMBAT);
    const bool hasBmana = botAI->HasStrategy("bmana", BOT_STATE_NON_COMBAT);
    const bool hasBdps = botAI->HasStrategy("bdps", BOT_STATE_NON_COMBAT);
    const bool actAsBstats = electedBstats && hasBstats;
    const bool actAsBmana = electedBmana && hasBmana;
    const bool actAsBdpsTankKings =
        (blessingState.paladinCount == 2u && blessingState.bstats.hasWearer && electedBdps && hasBdps);
    if (!actAsBstats && !actAsBmana && !actAsBdpsTankKings)
        return false;

    if (ShouldLogForThisBot(bot))
    {
        const std::string groupTag = MakeGroupTag(bot->GetGroup());
        std::string roleTag = actAsBmana ? "bmana" : (actAsBstats ? "bstats" : "bdps");
        LOG_DEBUG("playerbots", "[RoleCoord:{}] role={} -> {} is allowed to buff Kings", groupTag, roleTag,
                  bot->GetName());
    }

    Unit* target = GetTarget();
    if (!target)
        return false;

    Player* targetPlayer = target->ToPlayer();

    Group* group = bot->GetGroup();
    if (!group)
        return false;

    // Only one paladin in the party, case with active bstats, avoid self-targeting and ensure that Kings covers the
    // others, Greater if possible.
    if (actAsBstats && blessingState.IsSolo())
    {
        // If the initial target is not a "non-tank without Kings", we retarget
        auto lacksKings = [&](Unit* u) -> bool
        { return u && !botAI->HasAura("blessing of kings", u) && !botAI->HasAura("greater blessing of kings", u); };
        auto isEligibleNonTank = [&](Player* p) -> bool
        { return p && p->IsInWorld() && p->IsAlive() && !IsTankRole(p); };

        Player* tp0 = target->ToPlayer();
        bool ok0 = tp0 && isEligibleNonTank(tp0) && lacksKings(tp0) && (tp0->GetGUID() != bot->GetGUID());
        if (!ok0)
        {
            Unit* bestCandidate = nullptr;
            for (GroupReference* memberRef = group->GetFirstMember(); memberRef; memberRef = memberRef->next())
            {
                Player* memberPlayer = memberRef->GetSource();
                if (!isEligibleNonTank(memberPlayer))
                    continue;

                if (!lacksKings(memberPlayer))
                    continue;

                if (memberPlayer->GetGUID() == bot->GetGUID())
                    continue;  // do not target itself

                 bestCandidate = memberPlayer;
                break;
            }
            if (bestCandidate)
            {
                target = bestCandidate;
                targetPlayer = bestCandidate->ToPlayer();
                if (ShouldLogForThisBot(bot))
                    LOG_DEBUG("playerbots", "[Kings/bstats-solo] Retargeting towards {}", targetPlayer->GetName());
            }
        }
    }

    // Solo paladin, never buff itself to not remove his sanctuary buff
    if (botAI->HasStrategy("bstats", BOT_STATE_NON_COMBAT) && blessingState.IsSolo())
        if (target->GetGUID() == bot->GetGUID())
        {
            LOG_DEBUG("playerbots", "[Kings/bstats-solo] Skip self to keep Sanctuary on {}", bot->GetName());
            return false;
        }

    targetPlayer = target->ToPlayer();
    if (targetPlayer && !group->IsMember(targetPlayer->GetGUID()))
        return false;

    if (target->GetGUID() == bot->GetGUID() && IsProtectionPaladin(bot->ToPlayer()) &&
        (botAI->HasAura("blessing of sanctuary", bot) || botAI->HasAura("greater blessing of sanctuary", bot)))
    {
        LOG_DEBUG("playerbots", "[Kings] Skip self to keep Sanctuary on protection paladin {}", bot->GetName());
        return false;
    }

    if (target->GetGUID() == bot->GetGUID() && IsProtectionPaladin(bot->ToPlayer()) &&
        (botAI->HasAura("blessing of sanctuary", bot) || botAI->HasAura("greater blessing of sanctuary", bot)))
    {
        LOG_DEBUG("playerbots", "[Kings] Skip self to keep Sanctuary on protection paladin {}", bot->GetName());
        return false;
    }

    if (actAsBstats && !blessingState.IsSolo())
    {
        const bool isTwoPaladinsBstatsBdps =
            (blessingState.paladinCount == 2u && blessingState.bstats.hasWearer && blessingState.bdps.hasWearer);
        Player* candidate = targetPlayer;
        if (candidate && IsTankRole(candidate))
            candidate = nullptr;

        if (!candidate)
        {
            if (isTwoPaladinsBstatsBdps)
            {
                for (GroupReference* memberRef = group->GetFirstMember(); memberRef; memberRef = memberRef->next())
                {
                    Player* memberPlayer = memberRef->GetSource();
                    if (!memberPlayer || !memberPlayer->IsInWorld() || !memberPlayer->IsAlive())
                        continue;
                    if (IsTankRole(memberPlayer))
                        continue;
                    if (botAI->HasAura("blessing of kings", memberPlayer) ||
                        botAI->HasAura("greater blessing of kings", memberPlayer))
                        continue;
                    if (ai::paladin::GetActualBlessingOfMight(memberPlayer, /*log=*/false) != "blessing of might")
                        continue;
                    candidate = memberPlayer;
                    break;
                }
            }

            for (GroupReference* memberRef = group->GetFirstMember(); memberRef; memberRef = memberRef->next())
            {
                Player* memberPlayer = memberRef->GetSource();
                if (!memberPlayer || !memberPlayer->IsInWorld() || !memberPlayer->IsAlive())
                    continue;
                if (IsTankRole(memberPlayer))
                    continue;
                if (botAI->HasAura("blessing of kings", memberPlayer) ||
                    botAI->HasAura("greater blessing of kings", memberPlayer))
                    continue;
                candidate = memberPlayer;
                break;
            }
        }

        if (candidate)
        {
            target = candidate;
            targetPlayer = candidate;
        }
    }

    // If we act in bmana mode => Kings MONO on TANKS only
    if (actAsBmana && (!targetPlayer || !IsTankRole(targetPlayer)))
    {
        LOG_DEBUG("playerbots", "[Kings] Skip non-tank {}", target->GetName());
        return false;
    }

    if (targetPlayer)
    {
        const bool isTank = IsTankRole(targetPlayer);
        uint32 sanctSpellId = GetSpellId(botAI, "blessing of sanctuary");
        uint32 greaterSanctSpellId = GetSpellId(botAI, "greater blessing of sanctuary");
        const bool isProtPaladin = IsProtectionPaladin(targetPlayer);
        const bool hasSanctFromMe =
            (sanctSpellId && target->HasAura(sanctSpellId, bot->GetGUID())) ||
            (greaterSanctSpellId && target->HasAura(greaterSanctSpellId, bot->GetGUID()));

        const bool hasSanctAny =
            botAI->HasAura("blessing of sanctuary", target) || botAI->HasAura("greater blessing of sanctuary", target);

        if (!actAsBmana && !actAsBdpsTankKings && isTank && hasSanctFromMe && !isProtPaladin)
        {
            LOG_DEBUG("playerbots", "[Kings] Skip: {} has my Sanctuary and is a tank", target->GetName());
            return false;
        }

        if (actAsBstats && isTank && hasSanctAny && !isProtPaladin)
        {
            LOG_DEBUG("playerbots", "[Kings] Skip (bstats): {} already has Sanctuary and is a tank", target->GetName());
            return false;
        }
    }

    std::string castName = "blessing of kings";

    bool allowGreater = true;

    if (actAsBmana || actAsBdpsTankKings)
        allowGreater = false;

    // In solo-paladin (bstats), we want to favor the Greater to cover the target class.
    if (actAsBstats && blessingState.IsSolo())
        allowGreater = true;

    const bool isTwoPaladinsBstatsBdps =
        (blessingState.paladinCount == 2u && blessingState.bstats.hasWearer && blessingState.bdps.hasWearer);

    if (allowGreater && actAsBstats && targetPlayer && !blessingState.IsSolo() && !isTwoPaladinsBstatsBdps)
    {
        switch (targetPlayer->getClass())
        {
            case CLASS_WARRIOR:
            case CLASS_PALADIN:
            case CLASS_DRUID:
            case CLASS_DEATH_KNIGHT:
                allowGreater = false;
                break;
            default:
                break;
        }
    }

    if (allowGreater)
    {
        auto RP = ai::chat::MakeGroupAnnouncer(bot);
        castName = ai::buff::UpgradeToGroupIfAppropriate(bot, botAI, castName, /*announceOnMissing=*/true, RP);
    }

    return botAI->CastSpell(castName, target);
}

bool CastSealSpellAction::isUseful() { return AI_VALUE2(bool, "combat", "self target"); }

Value<Unit*>* CastTurnUndeadAction::GetTargetValue() { return context->GetValue<Unit*>("cc target", getName()); }

Unit* CastHandOfFreedomOnPartyAction::GetTarget()
{
    bool const selfImpaired = botAI->IsMovementImpaired(bot);
    bool const hasSelfHand = selfImpaired && ai::paladin::HasAnyPaladinHandFromCaster(bot, bot);

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

    return CastBuffSpellAction::isUseful() && !ai::paladin::HasAnyPaladinHandFromCaster(target, bot);
}

Unit* CastRighteousDefenseAction::GetTarget()
{
    Unit* currentTarget = AI_VALUE(Unit*, "current target");
    if (!currentTarget)
        return NULL;

    return currentTarget->GetVictim();
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

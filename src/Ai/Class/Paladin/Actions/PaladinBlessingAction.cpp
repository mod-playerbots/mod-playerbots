/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */
#include "PaladinBlessingAction.h"
#include "AiFactory.h"
#include "AiObjectContext.h"
#include "GenericBuffUtils.h"
#include "PaladinBlessingHelper.h"
#include "Pet.h"
#include "PlayerbotAI.h"
#include "Playerbots.h"
#include "SharedDefines.h"
#include "SpellInfo.h"
#include "SpellMgr.h"
#include <algorithm>
#include <array>
#include <cstddef>
#include <optional>
#include <unordered_map>
#include <vector>

using namespace ai::blessing;

namespace ai::blessing
{

RoleProfile ResolveRoleProfile(Player* player)
{
    if (!player)
        return ROLE_CASTER;

    switch (player->getClass())
    {
        case CLASS_WARRIOR:
        case CLASS_DEATH_KNIGHT:
            return PlayerbotAI::IsTank(player) ? ROLE_WARRIOR_DK_TANK : ROLE_PHYSICAL_DPS;
        case CLASS_ROGUE:
            return ROLE_PHYSICAL_DPS;
        case CLASS_HUNTER:
            return ROLE_HYBRID_DPS;
        case CLASS_SHAMAN:
            return AiFactory::GetPlayerSpecTab(player) == SHAMAN_TAB_ENHANCEMENT
                ? ROLE_HYBRID_DPS : ROLE_CASTER;
        case CLASS_PALADIN:
            if (PlayerbotAI::IsTank(player))
                return ROLE_PALADIN_TANK;
            return AiFactory::GetPlayerSpecTab(player) == PALADIN_TAB_HOLY
                ? ROLE_CASTER : ROLE_HYBRID_DPS;
        case CLASS_DRUID:
            if (AiFactory::GetPlayerSpecTab(player) == DRUID_TAB_FERAL)
                return PlayerbotAI::IsTank(player) ? ROLE_DRUID_TANK : ROLE_HYBRID_DPS;
            return ROLE_CASTER;
        default:
            return ROLE_CASTER;
    }
}

bool IsEligibleGroupForAutoBlessings(Group const* group)
{
    return ai::buff::IsGroupEligibleForBuffMode(group, sPlayerbotAIConfig.AutoGreaterBlessings);
}

PaladinBlessingPlanner::PaladinBlessingPlanner(PlayerbotAI* botAI)
    : _botAI(botAI), _bot(botAI->GetBot())
{
}

PendingBlessing PaladinBlessingPlanner::Plan()
{
    if (!_bot->IsAlive())
        return {};

    BlessingConstraints constraints;
    std::unordered_map<uint8, ClassGroup> classGroups;
    bool renewedHopePresent = false;   // a Disc priest's Renewed Hope suppresses Sanctuary
    bool recentLogin = false;          // any member still in post-login grace
    _coverage.clear();
    _humanCaster.clear();

    if (Group* group = _bot->GetGroup())
    {
        for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
            ProcessMember(ref->GetSource(), constraints, classGroups, renewedHopePresent, recentLogin);
    }
    else
    {
        ProcessMember(_bot, constraints, classGroups, renewedHopePresent, recentLogin);
    }

    if (recentLogin)
        return {};

    return SelectCast(constraints, classGroups, renewedHopePresent);
}

// Provider/role facts and per-class demand are gathered regardless of alive state, so the
// deterministic assignment stays stable across deaths and reses; only alive, assist-valid members
// (and their pets) are scanned into _coverage and become actual blessing targets.
void PaladinBlessingPlanner::ProcessMember(
    Player* member, BlessingConstraints& constraints,
    std::unordered_map<uint8, ClassGroup>& classGroups, bool& renewedHopePresent, bool& recentLogin)
{
    if (!member || !member->IsInWorld() || member->GetMap() != _bot->GetMap())
        return;

    if (ai::buff::IsWithinPostLoginBuffGrace(member))
        recentLogin = true;

    auto addDemand = [&](Unit* unit)
    {
        auto& classGroup = classGroups[unit->getClass()];
        classGroup.cls = unit->getClass();
        classGroup.members.push_back(unit);
        if (unit->IsAlive() && _bot->IsValidAssistTarget(unit))
            _coverage[unit->GetGUID()] = ai::blessing::scanBlessingAuras(unit, _humanCaster);
    };

    if (member->getClass() == CLASS_PRIEST &&
        (member->HasAura(SPELL_RENEWED_HOPE_R1) || member->HasAura(SPELL_RENEWED_HOPE_R2)))
        renewedHopePresent = true;

    if (member->getClass() == CLASS_PALADIN)
    {
        PlayerbotAI* memberAI = GET_PLAYERBOT_AI(member);
        PaladinFlags flags;
        flags.guid           = member->GetGUID();
        flags.isHuman        = (memberAI == nullptr);
        flags.hasKings       = member->HasSpell(SPELL_BLESSING_OF_KINGS);
        flags.hasSanctuary   = member->HasSpell(SPELL_BLESSING_OF_SANCTUARY);
        flags.mightStrength  = ai::blessing::HighestKnownBlessingStrength(member, BASE_MIGHT);
        flags.wisdomStrength = ai::blessing::HighestKnownBlessingStrength(member, BASE_WISDOM);
        constraints.paladins.push_back(flags);

        if (memberAI)
        {
            ObjectGuid const guid = member->GetGUID();
            auto& forced = constraints.forced;
            if (memberAI->HasStrategy("bmight",  BOT_STATE_NON_COMBAT)) forced.push_back({ guid, BASE_MIGHT });
            if (memberAI->HasStrategy("bwisdom", BOT_STATE_NON_COMBAT)) forced.push_back({ guid, BASE_WISDOM });
            if (memberAI->HasStrategy("bkings",  BOT_STATE_NON_COMBAT)) forced.push_back({ guid, BASE_KINGS });
            if (memberAI->HasStrategy("bsanc",   BOT_STATE_NON_COMBAT)) forced.push_back({ guid, BASE_SANCTUARY });
        }
    }

    addDemand(member);

    if (Pet* pet = member->GetPet();
        pet && pet->IsAlive() && pet->IsInWorld() && pet->GetMap() == _bot->GetMap())
        addDemand(pet);
}

PendingBlessing PaladinBlessingPlanner::SelectCast(
    BlessingConstraints const& constraints,
    std::unordered_map<uint8, ClassGroup> const& classGroups, bool renewedHopePresent)
{
    _intended.clear();

    // Bot paladins (the only actors), sorted by GUID; find my own index.
    std::vector<PaladinFlags> bots;
    bots.reserve(constraints.paladins.size());
    for (auto const& paladin : constraints.paladins)
        if (!paladin.isHuman)
            bots.push_back(paladin);
    std::sort(bots.begin(), bots.end(),
        [](PaladinFlags const& lhs, PaladinFlags const& rhs) { return lhs.guid < rhs.guid; });
    std::optional<std::size_t> selfIdx;
    for (std::size_t i = 0; i < bots.size(); ++i)
        if (bots[i].guid == _bot->GetGUID())
        {
            selfIdx = i;
            break;
        }
    if (!selfIdx)
        return {};

    std::unordered_map<uint8, BaseBlessingCategory> classMajority;
    classMajority.reserve(classGroups.size());
    std::vector<BaseBlessingCategory> myCats;
    myCats.reserve(BLESSING_PRIORITY_SLOTS);
    for (auto const& [cls, classGroup] : classGroups)
        classMajority[cls] = PlanClass(classGroup, bots, *selfIdx, constraints, renewedHopePresent, myCats);

    // classGroups is unordered; sort so the order I try my owed categories is stable.
    std::sort(myCats.begin(), myCats.end());

    if (myCats.empty())
        return {};

    PendingBlessing const greater = SelectGreater(classGroups, classMajority, myCats);
    if (greater.spellId)
        return greater;
    return SelectSingle(classGroups, classMajority, myCats);
}

BaseBlessingCategory PaladinBlessingPlanner::PlanClass(
    ClassGroup const& group,
    std::vector<PaladinFlags> const& bots, std::size_t selfIdx,
    BlessingConstraints const& constraints, bool renewedHopePresent,
    std::vector<BaseBlessingCategory>& myCats)
{
    std::size_t const memberCount = group.members.size();
    std::size_t const paladinCount = bots.size();

    // A human GREATER blessing is class-wide → yield that category for the WHOLE class.  A
    // human SINGLE covers only one character → handled per-member (that member skips it below).
    std::array<bool, BLESSING_CATEGORY_SLOTS> humanGreaterCoversClass{};
    for (Unit* member : group.members)
    {
        auto it = _coverage.find(member->GetGUID());
        if (it == _coverage.end())
            continue;
        for (auto const& blessing : it->second)
            if (blessing.isGreater && blessing.isHumanPaladinCaster)
                humanGreaterCoversClass[blessing.category] = true;
    }

    // Each member's top-N castable wants + priority-weighted class demand.
    std::vector<RoleProfile> roles(memberCount);
    std::vector<std::array<bool, BLESSING_CATEGORY_SLOTS>> wantedSet(memberCount);
    std::array<int, BLESSING_CATEGORY_SLOTS> demand{};
    std::array<bool, BLESSING_CATEGORY_SLOTS> botCanCast{};

    for (BaseBlessingCategory category : { BASE_MIGHT, BASE_WISDOM, BASE_KINGS, BASE_SANCTUARY })
        botCanCast[category] = SomeBotCanCast(bots, category);

    // Greaters land class-wide, so a pet sharing its class with a player adds no demand (it would
    // flip the player's blessing) and takes its wants as exception singles; mage-class pets are casters.
    bool const petsFollow = std::any_of(group.members.begin(), group.members.end(),
        [](Unit* unit) { return unit->IsPlayer(); });

    for (std::size_t k = 0; k < memberCount; ++k)
    {
        Unit* const member = group.members[k];
        RoleProfile const role = member->IsPlayer()
            ? ai::blessing::ResolveRoleProfile(member->ToPlayer())
            : (member->getClass() == CLASS_MAGE ? ROLE_CASTER : ROLE_PHYSICAL_DPS);
        roles[k] = role;
        bool const addsDemand = member->IsPlayer() || !petsFollow;
        std::size_t taken = 0;
        for (BaseBlessingCategory category : BASE_BLESSING_PRIORITIES[role].priorities)
        {
            if (category == BASE_NONE)
                break;
            if (renewedHopePresent && category == BASE_SANCTUARY && role != ROLE_PALADIN_TANK)
                continue;
            if (humanGreaterCoversClass[category])
                continue;
            if (HasHumanBlessing(member, category, _coverage))
                continue;
            if (!botCanCast[category])
                continue;
            wantedSet[k][category] = true;
            if (addsDemand)
                demand[category] += RoleWeight(role, category);
            if (++taken == paladinCount)
                break;
        }
    }

    // Distinct greater category per paladin for this class; mine, plus what every member receives.
    std::vector<BaseBlessingCategory> const greaters =
        AssignClassGreaters(bots, demand, humanGreaterCoversClass, constraints.forced, renewedHopePresent);
    BaseBlessingCategory const myGreater = greaters[selfIdx];

    std::array<bool, BLESSING_CATEGORY_SLOTS> coveredByGreaters{};
    for (BaseBlessingCategory greater : greaters)
        if (greater != BASE_NONE)
            coveredByGreaters[greater] = true;
    for (BaseBlessingCategory category : { BASE_MIGHT, BASE_WISDOM, BASE_KINGS, BASE_SANCTUARY })
        if (humanGreaterCoversClass[category])
            coveredByGreaters[category] = true;

    // What this bot delivers to each member: my greater if it's wanted, else an exception single.
    for (std::size_t k = 0; k < memberCount; ++k)
    {
        BaseBlessingCategory const intended = (myGreater != BASE_NONE && wantedSet[k][myGreater])
            ? myGreater
            : ResolveExceptionCategory(roles[k], wantedSet[k], coveredByGreaters, greaters, bots, selfIdx);

        if (intended != BASE_NONE)
        {
            _intended[group.members[k]->GetGUID()] = intended;
            if (std::find(myCats.begin(), myCats.end(), intended) == myCats.end())
                myCats.push_back(intended);
        }
    }

    return myGreater;
}

bool PaladinBlessingPlanner::CanBlessNow(Unit* member, uint32 spellId) const
{
    if (!member || !member->IsAlive() || !_bot->IsValidAssistTarget(member))
        return false;
    // Range gate before the costly CanCastSpell, which reports out-of-range as castable.
    SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(spellId);
    if (!spellInfo || !_bot->IsWithinDistInMap(member, spellInfo->GetMaxRange(true)))
        return false;
    return _botAI->CanCastSpell(spellId, member);
}

namespace
{
    // The core refuses our aura under a stronger "highest wins" buff from elsewhere (Battle Shout vs
    // Might), but CheckCast only tests that for single-target spells, not the area greaters.
    bool OutrankedByOtherBuff(Unit* caster, Unit* member, SpellInfo const* spell)
    {
        uint8 auraMask = 0;
        for (uint8 i = 0; i < MAX_SPELL_EFFECTS; ++i)
            if (spell->Effects[i].IsAura())
                auraMask |= 1 << i;
        for (uint8 i = 0; i < MAX_SPELL_EFFECTS; ++i)
            if ((auraMask & (1 << i)) && !member->IsHighestExclusiveAuraEffect(spell,
                    spell->Effects[i].ApplyAuraName, spell->Effects[i].CalcValue(caster), auraMask))
                return true;
        return false;
    }
}

PendingBlessing PaladinBlessingPlanner::SelectGreater(
    std::unordered_map<uint8, ClassGroup> const& groups,
    std::unordered_map<uint8, BaseBlessingCategory> const& classMajority,
    std::vector<BaseBlessingCategory> const& myCats)
{
    for (BaseBlessingCategory const cat : myCats)
    {
        // No id = can't cast this greater (untrained or config-disabled); no reagent = nothing to
        // cast.  Either way SelectSingle handles the category.
        std::optional<uint32> greaterId;
        if (ai::blessing::IsEligibleGroupForAutoBlessings(_bot->GetGroup()))
            greaterId = HighestKnownBlessingRank(_bot, cat, true);
        if (!greaterId || !ai::buff::HasRequiredReagents(_bot, *greaterId))
            continue;
        SpellInfo const* greaterInfo = sSpellMgr->GetSpellInfo(*greaterId);
        if (!greaterInfo)
            continue;

        int32 const myStrength = GetBlessingCastStrength(_bot, ToGreaterVariant(cat), *greaterId);

        for (auto const& [cls, classGroup] : groups)
        {
            // One greater per class: only its majority category (minority specs → SelectSingle).
            auto majorityIt = classMajority.find(cls);
            if (majorityIt == classMajority.end() || majorityIt->second != cat)
                continue;

            // First member who wants `cat` from me, lacks it (not covered by me, a human, or a
            // stronger blessing), and that I can cast on now.  A greater is class-wide, so any works.
            Unit* target = nullptr;
            bool classNeedsGreater = false;
            for (Unit* member : classGroup.members)
            {
                auto intendedIt = _intended.find(member->GetGUID());
                if (intendedIt == _intended.end() || intendedIt->second != cat)
                    continue;
                if (IsCoveredByMe(_botAI, member, cat, _coverage))
                    continue;
                if (HasHumanBlessing(member, cat, _coverage))
                    continue;
                if (HasStrongerBlessing(member, cat, myStrength, _coverage))
                    continue;
                if (OutrankedByOtherBuff(_bot, member, greaterInfo))
                    continue;
                classNeedsGreater = true;
                if (CanBlessNow(member, *greaterId))
                {
                    target = member;
                    break;
                }
            }

            if (!classNeedsGreater || !target)
                continue;

            return { *greaterId, target->GetGUID(), cat, ReagentNoticeKind::Clear };
        }
    }
    return {};
}

PendingBlessing PaladinBlessingPlanner::SelectSingle(
    std::unordered_map<uint8, ClassGroup> const& groups,
    std::unordered_map<uint8, BaseBlessingCategory> const& classMajority,
    std::vector<BaseBlessingCategory> const& myCats)
{
    for (BaseBlessingCategory const cat : myCats)
    {
        std::optional<uint32> const singleId = HighestKnownBlessingRank(_bot, cat, false);
        if (!singleId)
            continue;

        std::optional<uint32> greaterId;
        if (ai::blessing::IsEligibleGroupForAutoBlessings(_bot->GetGroup()))
            greaterId = HighestKnownBlessingRank(_bot, cat, true);
        bool const hasReagent = greaterId && ai::buff::HasRequiredReagents(_bot, *greaterId);

        int32 const myStrength = GetBlessingCastStrength(_bot, ToSingleVariant(cat), *singleId);

        for (auto const& [cls, classGroup] : groups)
        {
            // With a reagent, only minority-spec exceptions need a single (the majority gets a
            // greater); without one, every member.
            auto majorityIt = classMajority.find(cls);
            bool const isException = (majorityIt == classMajority.end() || cat != majorityIt->second);
            if (hasReagent && !isException)
                continue;

            for (Unit* member : classGroup.members)
            {
                auto intendedIt = _intended.find(member->GetGUID());
                if (intendedIt == _intended.end() || intendedIt->second != cat)
                    continue;
                if (IsCoveredByMe(_botAI, member, cat, _coverage))
                    continue;
                if (HasHumanBlessing(member, cat, _coverage))
                    continue;
                if (HasStrongerBlessing(member, cat, myStrength, _coverage))
                    continue;
                if (!CanBlessNow(member, *singleId))
                    continue;

                ReagentNoticeKind notice = ReagentNoticeKind::None;
                if (greaterId)
                    notice = hasReagent ? ReagentNoticeKind::Clear : ReagentNoticeKind::Announce;

                return { *singleId, member->GetGUID(), cat, notice };
            }
        }
    }
    return {};
}

}

Unit* CastBlessingAction::GetTarget()
{
    return botAI->GetUnit(_pending.target);
}

bool CastBlessingAction::isUseful()
{
    if (botAI->IsInVehicle() && !botAI->IsInVehicle(false, false, true))
        return false;

    _pending = PaladinBlessingPlanner(botAI).Plan();
    return _pending.spellId && GetTarget();
}

bool CastBlessingAction::isPossible() { return true; }

bool CastBlessingAction::Execute(Event /*event*/)
{
    PendingBlessing const planned = _pending;
    if (!planned.spellId)
        return false;

    Unit* target = botAI->GetUnit(planned.target);
    if (!target)
        return false;

    botAI->forceRebuff.NoteBuffWork();

    bool const cast = botAI->CastSpell(planned.spellId, target);

    if (planned.notice == ReagentNoticeKind::Clear)
        ai::buff::ClearMissingBuffReagentNotice(botAI, BlessingSpellName(ToGreaterVariant(planned.category)));
    else if (planned.notice == ReagentNoticeKind::Announce)
        ai::buff::TryAnnounceMissingBuffReagents(
            botAI, BlessingSpellName(ToSingleVariant(planned.category)),
            BlessingSpellName(ToGreaterVariant(planned.category)));

    return cast;
}

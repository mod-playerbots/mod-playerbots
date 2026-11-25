/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "Naxx40Horsemen.h"

#include "Action.h"
#include "AttackAction.h"
#include "EncounterHelpers.h"
#include "FoodActions.h"
#include "FourHorsemen40.h"
#include "Group.h"
#include "HunterActions.h"
#include "InstanceScript.h"
#include "MageActions.h"
#include "Map.h"
#include "MoveSpline.h"
#include "MovementActions.h"
#include "NamedObjectContext.h"
#include "ObjectAccessor.h"
#include "PetsAction.h"
#include "PlayerScript.h"
#include "Playerbots.h"
#include "ReachTargetActions.h"
#include "Strategy.h"
#include "Timer.h"
#include "Trigger.h"
#include "WorldBuffAction.h"

namespace Naxx40Horsemen
{
namespace
{
constexpr uint32 NAXXRAMAS_MAP = 533;
constexpr auto NAXX40_DIFFICULTY = static_cast<Difficulty>(2);
constexpr uint32 DATA_HORSEMEN_ENCOUNTER = 12;
constexpr float STAGING_START_RANGE = 12.0f;
constexpr float STAGING_RANGE = 110.0f;
constexpr float PULL_ARRIVAL = 0.20f;
constexpr uint32 MUSTER_POLL_INTERVAL = 400;
constexpr uint32 OPENING_REST_LIMIT = 35 * IN_MILLISECONDS;
constexpr float OPENING_REST_PERCENT = 99.0f;
constexpr uint32 STALE_OPENING_INTERVAL = 10 * IN_MILLISECONDS;
constexpr uint32 ROOM_READY_PERCENT = 90;
constexpr float ROOM_MIN_X = 2450.0f;
constexpr float ROOM_MAX_X = 2630.0f;
constexpr float ROOM_MIN_Y = -3014.0f;
constexpr float ROOM_MAX_Y = -2880.0f;
constexpr float ROOM_MIN_Z = 239.0f;
constexpr float ROOM_MAX_Z = 248.0f;
Position const ROOM_ENTRANCE(2555.9f, -2996.4f, 241.4f);
Position const PULL_POSITION(2516.0f, -2983.0f, 241.276f);
std::string const OPENING_KEY = "playerbots.naxx40.horsemen.opening-control";
std::string const RESTORATION_KEY = "playerbots.naxx40.horsemen.restore-scheduled";

constexpr char PREPARE_OPENING[] = "naxx40 horsemen prepare opening";
constexpr char STAGE_PULL_HOLDER[] = "naxx40 horsemen stage pull holder";
constexpr char COORDINATED_PULL[] = "naxx40 horsemen coordinated pull";
constexpr char RESTORE_STANCE[] = "naxx40 horsemen restore stance";
constexpr char APPLY_TANK_WORLD_BUFFS[] = "naxx40 horsemen tank world buffs";
constexpr char OPENING_EAT[] = "naxx40 horsemen opening eat";
constexpr char OPENING_DRINK[] = "naxx40 horsemen opening drink";

enum class OpeningPhase
{
    Muster,
    Staging,
    Ready
};

struct OpeningControl : DataMap::Base
{
    ObjectGuid group;
    ObjectGuid coordinator;
    ObjectGuid holder;
    OpeningPhase phase = OpeningPhase::Muster;
    uint32 lastDrive = 0;
    uint32 lastMusterPoll = 0;
    uint32 restingSince = 0;
    bool rebuffStarted = false;
};

struct ScheduledRestoration : DataMap::Base
{
};

struct MusterStatus
{
    bool roomReady = false;
    bool rebuffing = false;
    bool rested = true;
};

bool IsAutonomous(Player* player) { return player && !IsRealPlayer(player) && !IsSelfBot(player); }

bool IsHorsemenActive(Player* player)
{
    return IsNaxx40(player) && EncounterHelpers::IsBossInProgress(player, NAXXRAMAS_MAP, DATA_HORSEMEN_ENCOUNTER);
}

bool IsOpeningArea(Player* player)
{
    if (!IsNaxx40(player) || !player->GetGroup() || !player->IsAlive() || player->IsInCombat() ||
        player->GetExactDist(ROOM_ENTRANCE) > STAGING_RANGE)
        return false;

    InstanceScript const* instance = player->GetInstanceScript();
    return instance && instance->GetBossState(DATA_HORSEMEN_ENCOUNTER) == NOT_STARTED &&
           !instance->IsEncounterInProgress();
}

OpeningControl* GetRaidOpening(Player* player)
{
    OpeningControl* control = player->GetMap()->CustomData.Get<OpeningControl>(OPENING_KEY);
    return control && control->group == player->GetGroup()->GetGUID() ? control : nullptr;
}

OpeningControl* GetOpeningControl(Player* player)
{
    OpeningControl* control = GetRaidOpening(player);
    if (!control || !control->coordinator)
        return nullptr;

    uint32 const now = getMSTime();
    bool const resting = control->phase == OpeningPhase::Muster && control->restingSince &&
                         getMSTimeDiff(control->restingSince, now) < OPENING_REST_LIMIT;
    return resting || getMSTimeDiff(control->lastDrive, now) < STALE_OPENING_INTERVAL ? control : nullptr;
}

bool CanDriveOpening(Player* player)
{
    if (!IsOpeningArea(player) || !IsAutonomous(player))
        return false;

    OpeningControl const* control = GetOpeningControl(player);
    if (control)
        return control->coordinator == player->GetGUID();
    OpeningControl const* previous = GetRaidOpening(player);
    return (previous && previous->coordinator == player->GetGUID()) ||
           player->GetExactDist(ROOM_ENTRANCE) <= STAGING_START_RANGE;
}

PlayerbotAI* GetOpeningMemberAI(Player* player, Player* member)
{
    if (!member || !member->IsInWorld() || !member->IsAlive() || !member->IsInMap(player) ||
        !member->InSamePhase(player))
        return nullptr;
    return GET_PLAYERBOT_AI(member);
}

bool HasActiveMusterDriver(PlayerbotAI& ai)
{
    auto* const driver = ai.GetAiObjectContext()->GetValue<Player*>("dungeon clear party tank");
    return driver && driver->Get();
}

bool IsInsideGate(Player const& player)
{
    float const x = player.GetPositionX();
    float const y = player.GetPositionY();
    float const z = player.GetPositionZ();
    return x >= ROOM_MIN_X && x <= ROOM_MAX_X && y >= ROOM_MIN_Y && y <= ROOM_MAX_Y && z >= ROOM_MIN_Z &&
           z <= ROOM_MAX_Z;
}

bool CanRestForOpening(Player* player)
{
    if (!IsOpeningArea(player) || !IsAutonomous(player) || !IsInsideGate(*player))
        return false;
    OpeningControl const* control = GetOpeningControl(player);
    return control && control->phase == OpeningPhase::Muster && control->restingSince && !control->rebuffStarted &&
           getMSTimeDiff(control->restingSince, getMSTime()) < OPENING_REST_LIMIT;
}

bool NeedsOpeningFood(Player* player)
{
    return CanRestForOpening(player) && player->GetHealthPct() < OPENING_REST_PERCENT;
}

bool NeedsOpeningDrink(Player* player)
{
    return CanRestForOpening(player) && player->GetMaxPower(POWER_MANA) &&
           player->GetPowerPct(POWER_MANA) < OPENING_REST_PERCENT;
}

MusterStatus ReadMuster(Player* player)
{
    MusterStatus muster;
    uint32 living = 0;
    uint32 inside = 0;
    bool supportInside = true;
    for (GroupReference* ref = player->GetGroup()->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        PlayerbotAI* const ai = GetOpeningMemberAI(player, member);
        if (!ai)
            continue;

        ++living;
        if (IsInsideGate(*member))
            ++inside;
        else if (PlayerbotAI::IsTank(member) || PlayerbotAI::IsHeal(member))
            supportInside = false;

        muster.rebuffing |= ai->forceRebuff.IsPending();
        if (IsAutonomous(member))
            muster.rested &=
                member->GetHealthPct() >= OPENING_REST_PERCENT &&
                (!member->GetMaxPower(POWER_MANA) || member->GetPowerPct(POWER_MANA) >= OPENING_REST_PERCENT);
    }
    muster.roomReady = supportInside && living && inside * 100 >= living * ROOM_READY_PERCENT;
    return muster;
}

void BeginOpeningRebuff(Player* player)
{
    for (GroupReference* ref = player->GetGroup()->GetFirstMember(); ref; ref = ref->next())
        if (PlayerbotAI* ai = GetOpeningMemberAI(player, ref->GetSource());
            ai && ai->HasStrategy("force rebuff", BOT_STATE_NON_COMBAT))
            ai->forceRebuff.Begin(false);
}

bool IsOpeningReadyForPull(Player* player)
{
    if (!IsOpeningArea(player))
        return false;

    OpeningControl const* control = GetOpeningControl(player);
    if (!control || control->phase != OpeningPhase::Ready || control->coordinator != player->GetGUID())
        return false;

    Player* holder = ObjectAccessor::GetPlayer(*player, control->holder);
    return IsAutonomous(holder) && holder->IsAlive() && holder->GetExactDist(PULL_POSITION) <= PULL_ARRIVAL &&
           holder->movespline->Finalized();
}

bool ShouldStagePullHolder(Player* player)
{
    if (!IsOpeningArea(player) || !IsAutonomous(player))
        return false;

    OpeningControl const* control = GetOpeningControl(player);
    return control && control->phase == OpeningPhase::Ready && control->holder == player->GetGUID() &&
           player->GetExactDist(PULL_POSITION) > PULL_ARRIVAL;
}

bool HasStanceToRestore(Player* player)
{
    return IsNaxx40(player) && player->IsAlive() && !player->IsInCombat() &&
           FourHorsemen40::HasTemporaryStance(*player);
}

bool HasTankWorldBuffs(Player* player)
{
    if (!FourHorsemen40::ProtectsTankSupplies(player) || player->IsInCombat())
        return false;
    return GET_PLAYERBOT_AI(player)->HasStrategy("worldbuff", BOT_STATE_NON_COMBAT);
}

class PrepareOpeningAction final : public Action
{
public:
    explicit PrepareOpeningAction(PlayerbotAI* ai) : Action(ai, PREPARE_OPENING) {}

    bool isUseful() override { return CanDriveOpening(bot); }

    bool Execute(Event /*event*/) override
    {
        if (!CanDriveOpening(bot))
            return false;

        OpeningControl* control = GetOpeningControl(bot);
        if (!control)
        {
            control = bot->GetMap()->CustomData.GetDefault<OpeningControl>(OPENING_KEY);
            ObjectGuid const group = bot->GetGroup()->GetGUID();
            bool const continuingMuster =
                control->group == group && control->coordinator && control->phase == OpeningPhase::Muster;
            uint32 const restingSince = continuingMuster ? control->restingSince : 0;
            bool const rebuffStarted = continuingMuster && control->rebuffStarted;
            *control = {};
            control->group = group;
            control->coordinator = bot->GetGUID();
            control->restingSince = restingSince;
            control->rebuffStarted = rebuffStarted;
        }
        uint32 const now = getMSTime();
        control->lastDrive = now;

        if (control->phase == OpeningPhase::Muster)
        {
            if (control->lastMusterPoll && getMSTimeDiff(control->lastMusterPoll, now) < MUSTER_POLL_INTERVAL)
                return false;

            control->lastMusterPoll = now;
            MusterStatus const muster = ReadMuster(bot);
            control->rebuffStarted |= muster.rebuffing;
            if (!muster.roomReady || muster.rebuffing)
                return false;

            if (!control->rebuffStarted && HasActiveMusterDriver(*botAI))
            {
                if (!control->restingSince)
                    control->restingSince = now;
                if (!muster.rested && getMSTimeDiff(control->restingSince, now) < OPENING_REST_LIMIT)
                    return false;

                // Generic objective arrival can engage a rider without a rebuff signal.
                // Use the normal Playerbots rebuff window before hand-placing the raid.
                BeginOpeningRebuff(bot);
                control->rebuffStarted = true;
                return false;
            }
            control->phase = OpeningPhase::Staging;
        }

        FourHorsemen40::Preparation const preparation = FourHorsemen40::PrepareOpening(*botAI);
        control->phase =
            preparation == FourHorsemen40::Preparation::Ready ? OpeningPhase::Ready : OpeningPhase::Staging;
        control->holder = FourHorsemen40::GetOpeningHolder(*bot);
        // Another action may need to restore this bot's stance before staging can finish.
        return false;
    }
};

class StagePullHolderAction final : public Action
{
public:
    explicit StagePullHolderAction(PlayerbotAI* ai) : Action(ai, STAGE_PULL_HOLDER) {}

    bool isUseful() override { return ShouldStagePullHolder(bot); }

    bool Execute(Event /*event*/) override { return isUseful() && FourHorsemen40::StagePullHolder(*botAI); }
};

class CoordinatedPullAction final : public Action
{
public:
    explicit CoordinatedPullAction(PlayerbotAI* ai) : Action(ai, COORDINATED_PULL) {}

    bool isUseful() override { return IsOpeningReadyForPull(bot); }

    bool Execute(Event /*event*/) override
    {
        if (!isUseful() || !FourHorsemen40::PullOpening(*botAI))
            return false;

        if (IsHorsemenActive(bot))
        {
            // A persistent objective may already have completed its muster on a retry.
            OpeningControl* control = GetOpeningControl(bot);
            if (control)
                control->coordinator.Clear();
        }
        return true;
    }
};

class RestoreStanceAction final : public Action
{
public:
    explicit RestoreStanceAction(PlayerbotAI* ai) : Action(ai, RESTORE_STANCE) {}

    bool isUseful() override { return HasStanceToRestore(bot); }

    bool Execute(Event /*event*/) override { return isUseful() && FourHorsemen40::RestoreWarriorStance(*botAI); }
};

class ApplyTankWorldBuffsAction final : public Action
{
public:
    explicit ApplyTankWorldBuffsAction(PlayerbotAI* ai) : Action(ai, APPLY_TANK_WORLD_BUFFS) {}

    bool isUseful() override { return HasTankWorldBuffs(bot); }

    bool Execute(Event /*event*/) override
    {
        if (isUseful())
            for (uint32 spell : WorldBuffAction::NeedWorldBuffs(bot))
                if (!FourHorsemen40::ConflictsWithTankSupplies(spell))
                    bot->AddAura(spell, bot);
        return false;
    }
};

template <class NativeAction, char const* name, bool (*condition)(Player*)>
class OpeningRestAction final : public NativeAction
{
public:
    explicit OpeningRestAction(PlayerbotAI* ai) : NativeAction(ai) {}

    std::string const getName() override { return name; }
    bool isUseful() override { return condition(this->bot) && NativeAction::isUseful(); }
};

class OpeningTrigger final : public Trigger
{
public:
    OpeningTrigger(PlayerbotAI* ai, char const* name, bool (*condition)(Player*))
        : Trigger(ai, name), _condition(condition)
    {
    }

    bool IsActive() override { return _condition(bot); }

private:
    bool (*const _condition)(Player*);
};

class HorsemenOpeningMultiplier final : public Multiplier
{
public:
    explicit HorsemenOpeningMultiplier(PlayerbotAI* ai) : Multiplier(ai, "naxx40 horsemen opening control") {}

    float GetValue(Action* action) override
    {
        if (action->getName() == "world buff" && HasTankWorldBuffs(bot))
            return 0.0f;
        if (IsHorsemenActive(bot))
        {
            if (action->getName() == "dungeon clear recover stranded" ||
                action->getName() == "dungeon clear assist camp combat" || dynamic_cast<CastBlinkBackAction*>(action) ||
                dynamic_cast<CastDisengageAction*>(action))
                return 0.0f;
            return 1.0f;
        }
        if (!IsOpeningArea(bot))
            return 1.0f;

        OpeningControl const* control = GetRaidOpening(bot);
        if (!control)
            return 1.0f;

        std::string const name = action->getName();
        if (name == "dungeon clear objective arrive")
            return 0.0f;
        if (!control->coordinator || control->phase == OpeningPhase::Muster)
            return 1.0f;

        if (name == "dungeon clear recover stranded")
            return 0.0f;
        if (!IsInsideGate(*bot))
            return 1.0f;
        if (name == "open loot" || name == "loot" || dynamic_cast<MovementAction*>(action) ||
            dynamic_cast<CastReachTargetSpellAction*>(action) || dynamic_cast<AttackAction*>(action) ||
            dynamic_cast<PetAttackAction*>(action))
            return 0.0f;
        return 1.0f;
    }
};

void ScheduleRestore(Player* player)
{
    PlayerbotAI* ai = GET_PLAYERBOT_AI(player);
    if (!ai || IsNaxx40(player) || player->CustomData.Get<ScheduledRestoration>(RESTORATION_KEY) ||
        !FourHorsemen40::HasTemporaryStance(*player))
        return;

    player->CustomData.GetDefault<ScheduledRestoration>(RESTORATION_KEY);
    player->m_Events.AddEventAtOffset(
        [player]
        {
            player->CustomData.Erase(RESTORATION_KEY);
            PlayerbotAI* ai = GET_PLAYERBOT_AI(player);
            if (!ai || !player->IsInWorld() || !player->IsAlive() || IsNaxx40(player))
                return;
            FourHorsemen40::RestoreWarriorStance(*ai);
            ScheduleRestore(player);
        },
        Seconds(1));
}

class HorsemenCleanupScript final : public PlayerScript
{
public:
    HorsemenCleanupScript()
        : PlayerScript("HorsemenCleanupScript", {PLAYERHOOK_ON_MAP_CHANGED, PLAYERHOOK_ON_PLAYER_RESURRECT})
    {
    }

    void OnPlayerMapChanged(Player* player) override { ScheduleRestore(player); }

    void OnPlayerResurrect(Player* player, float /*restorePercent*/, bool& /*applySickness*/) override
    {
        ScheduleRestore(player);
    }
};
}  // namespace

bool IsNaxx40(Player* player)
{
    return player->IsInWorld() && player->GetMapId() == NAXXRAMAS_MAP &&
           player->GetMap()->GetDifficulty() == NAXX40_DIFFICULTY;
}

void RegisterActions(NamedObjectContext<Action>& actions)
{
    actions.creators[PREPARE_OPENING] = [](PlayerbotAI* ai) { return new PrepareOpeningAction(ai); };
    actions.creators[STAGE_PULL_HOLDER] = [](PlayerbotAI* ai) { return new StagePullHolderAction(ai); };
    actions.creators[COORDINATED_PULL] = [](PlayerbotAI* ai) { return new CoordinatedPullAction(ai); };
    actions.creators[RESTORE_STANCE] = [](PlayerbotAI* ai) { return new RestoreStanceAction(ai); };
    actions.creators[APPLY_TANK_WORLD_BUFFS] = [](PlayerbotAI* ai) { return new ApplyTankWorldBuffsAction(ai); };
    actions.creators[OPENING_EAT] = [](PlayerbotAI* ai)
    { return new OpeningRestAction<EatAction, OPENING_EAT, NeedsOpeningFood>(ai); };
    actions.creators[OPENING_DRINK] = [](PlayerbotAI* ai)
    { return new OpeningRestAction<DrinkAction, OPENING_DRINK, NeedsOpeningDrink>(ai); };
}

void RegisterTriggers(NamedObjectContext<Trigger>& triggers)
{
    triggers.creators[PREPARE_OPENING] = [](PlayerbotAI* ai)
    { return new OpeningTrigger(ai, PREPARE_OPENING, CanDriveOpening); };
    triggers.creators[STAGE_PULL_HOLDER] = [](PlayerbotAI* ai)
    { return new OpeningTrigger(ai, STAGE_PULL_HOLDER, ShouldStagePullHolder); };
    triggers.creators[COORDINATED_PULL] = [](PlayerbotAI* ai)
    { return new OpeningTrigger(ai, COORDINATED_PULL, IsOpeningReadyForPull); };
    triggers.creators[RESTORE_STANCE] = [](PlayerbotAI* ai)
    { return new OpeningTrigger(ai, RESTORE_STANCE, HasStanceToRestore); };
    triggers.creators[APPLY_TANK_WORLD_BUFFS] = [](PlayerbotAI* ai)
    { return new OpeningTrigger(ai, APPLY_TANK_WORLD_BUFFS, HasTankWorldBuffs); };
    triggers.creators[OPENING_EAT] = [](PlayerbotAI* ai)
    { return new OpeningTrigger(ai, OPENING_EAT, NeedsOpeningFood); };
    triggers.creators[OPENING_DRINK] = [](PlayerbotAI* ai)
    { return new OpeningTrigger(ai, OPENING_DRINK, NeedsOpeningDrink); };
}

void InitTriggers(std::vector<TriggerNode*>& triggers)
{
    triggers.push_back(new TriggerNode(PREPARE_OPENING, {NextAction(PREPARE_OPENING, ACTION_RAID + 3)}));
    triggers.push_back(new TriggerNode(STAGE_PULL_HOLDER, {NextAction(STAGE_PULL_HOLDER, ACTION_RAID + 4)}));
    triggers.push_back(new TriggerNode(COORDINATED_PULL, {NextAction(COORDINATED_PULL, ACTION_RAID + 5)}));
    triggers.push_back(new TriggerNode(RESTORE_STANCE, {NextAction(RESTORE_STANCE, ACTION_RAID + 1)}));
    triggers.push_back(new TriggerNode(APPLY_TANK_WORLD_BUFFS, {NextAction(APPLY_TANK_WORLD_BUFFS, ACTION_DEFAULT)}));
    triggers.push_back(new TriggerNode(OPENING_EAT, {NextAction(OPENING_EAT, ACTION_NORMAL)}));
    triggers.push_back(new TriggerNode(OPENING_DRINK, {NextAction(OPENING_DRINK, ACTION_NORMAL + 1)}));
}

void InitMultipliers(PlayerbotAI* ai, std::vector<Multiplier*>& multipliers)
{
    multipliers.push_back(new HorsemenOpeningMultiplier(ai));
}

void AddScripts()
{
    new HorsemenCleanupScript();
    FourHorsemen40::AddRearHealingScript();
}
}  // namespace Naxx40Horsemen

void AddSC_Naxx40HorsemenBotScripts() { Naxx40Horsemen::AddScripts(); }

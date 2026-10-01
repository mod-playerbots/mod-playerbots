/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_AQ40ACTIONS_H
#define PLAYERBOTS_AQ40ACTIONS_H

#include <array>
#include <chrono>
#include <map>

#include "AttackAction.h"
#include "Multiplier.h"
#include "Trigger.h"

namespace Aq40
{
enum Encounter : uint32
{
    None,
    Skeram,
    Trio,
    Sartura,
    Fankriss,
    Viscidus,
    Huhuran,
    Twins,
    Ouro,
    Cthun
};

enum CreatureIds : uint32
{
    SkeramBoss = 15263,
    Kri = 15511,
    Yauj = 15543,
    Vem = 15544,
    Brood = 15621,
    SarturaBoss = 15516,
    Guard = 15984,
    FankrissBoss = 15510,
    Worm = 15630,
    Hatchling = 15962,
    ViscidusBoss = 15299,
    Glob = 15667,
    Toxin = 15925,
    HuhuranBoss = 15509,
    Veknilash = 15275,
    Veklor = 15276,
    Scarab = 15316,
    Scorpion = 15317,
    OuroBoss = 15517,
    Mound = 15712,
    OuroScarab = 15718,
    Eye = 15589,
    Body = 15727,
    SmallEye = 15726,
    SmallClaw = 15725,
    GiantEye = 15334,
    GiantClaw = 15728,
    Flesh = 15802,
    StomachExit = 15800,
    PoisonCloud = 15933,
    Defender = 15277
};

enum SpellIds : uint32
{
    MindControl = 785,
    Whirlwind = 26083,
    GuardWhirlwind = 26038,
    MortalWound = 25646,
    Frozen = 25937,
    ViscidusInvisible = 25905,
    Frenzy = 26051,
    Sting = 26180,
    AcidSpit = 26050,
    HuhuranBolt = 26052,
    Mutate = 802,
    ExplodeBug = 804,
    TwinTeleport = 26638,
    TwinTeleportCast = 799,
    ShadowFrostReflect = 19595,
    FireArcaneReflect = 13022,
    DarkGlare = 26029,
    EyeBeam = 26134,
    RedEye = 22518,
    DigestiveAcid = 26476,
    Carapace = 26156,
    Weakness = 22581,
    YaujHeal = 25807,
    SandBlast = 26102,
    Blizzard = 26607,
    ToxinAura = 26575,
    ArcaneExplosion = 26192,
    TreeOfLife = 33891
};

// Viscidus: Toxin (25989) is a 5-yard persistent cloud that never despawns, so the room fills up.
// The raid is pulled from a ramp that descends from the entrance (z -24) to the floor (z -45..-53).
constexpr float ToxinRadius = 5.0f;
constexpr float ToxinMargin = 3.0f;
constexpr float ViscidusRampZ = -40.0f;
constexpr float ViscidusRampY = 985.0f;
constexpr float ViscidusRingOffset = 18.0f;
constexpr float ViscidusRingTolerance = 6.0f;
constexpr float ViscidusStrandedOffset = 40.0f;
inline Position const ViscidusRoomCenter{-7992.36f, 908.19f, -52.62f};

bool InStomach(Player* player);
bool Whirling(Unit* unit);
uint32 Stacks(Unit* unit, uint32 spell);
}  // namespace Aq40

// State is owned by one group bot's action context, not a process-wide instance cache.
// All persistent references are GUIDs; dead owners remain eligible to avoid reset on every death.
class Aq40ControlAction : public AttackAction
{
public:
    Aq40ControlAction(PlayerbotAI* ai) : AttackAction(ai, "aq40 control") {}
    static Aq40ControlAction* Get(PlayerbotAI* ai);
    void Refresh(PlayerbotAI* observer);
    bool Execute(Event event) override;
    Aq40::Encounter Encounter() const { return _encounter; }
    std::vector<Creature*> Units(uint32 entry = 0);
    std::vector<Player*> Members(bool tanks = false);
    Player* Member(ObjectGuid guid);
    Creature* Boss();
    Player* TankFor(Unit* unit);
    Unit* Target(Player* player);
    bool IsTankRole(Player* player);
    bool IsTwinLock(Player* player);
    bool IsTwinWaiter(Unit* unit) const { return unit && _twinWaiter && unit->GetGUID() == _twinWaiter; }
    bool IsTwinPlatformTank(Unit* unit) const
    {
        return unit && ((_twinTanks[0] && unit->GetGUID() == _twinTanks[0]) ||
                        (_twinTanks[1] && unit->GetGUID() == _twinTanks[1]));
    }
    // A tank-spec bot without a platform melees Vek'nilash as the second name on his threat list.
    bool IsTwinSpareTank(Player* player);
    Position TwinUnstuckSpot(Unit* emperor);
    Position TwinLockSpot(uint32 side) const;
    bool TwinSwapWindow() const;
    uint32 TwinSideOf(Position const& pos) const;
    // Healing threat splits onto both emperors, and Vek'nilash cannot be taunted back. For the first
    // seconds of the pull healers only save players in real danger while the tanks build threat.
    bool TwinOpeningHold(Unit* target) const
    {
        return _encounter == Aq40::Twins && target && target->GetHealthPct() >= 60.0f &&
               std::chrono::steady_clock::now() - _encounterStart < std::chrono::seconds(8);
    }
    bool Held(Unit* unit);
    bool Danger(Player* player, Position& goal);
    bool Formation(Player* player, Position& goal);
    bool UrgentHeal(PlayerbotAI* ai);
    bool Interrupt(PlayerbotAI* ai);
    bool GiantEyeResponse(PlayerbotAI* ai);
    bool Special(PlayerbotAI* ai);
    bool AllowedDamage(Player* player, Unit* unit);
    bool StomachExitNeeded(Player* player);
    // C'Thun: each raid member's spot from Aq40Rules::CthunSlots, and what it may hit from there.
    bool CthunSpot(Player* player, Position& spot);
    bool CthunMeleeBurn(Player* player);
    bool CthunEyeMelee(Player* player);
    Unit* CthunTarget(Player* player);
    bool CthunPhase2();
    bool NeedSwap(Unit* boss, uint32 aura, uint32 stacks);
    bool SkeramPlatform(Player* player, Position& platform);
    Position ViscidusRing(Player* player, Creature* boss);
    bool ToxinEscape(Player* player, Position& goal);

private:
    void Reset();
    void AssignTanks();
    void AssignSkeram(std::vector<Player*> const& tanks);
    Unit* DamageTarget(Player* player);
    Position TwinWaitSpot(Player* player, Unit* veklor);
    ObjectGuid _group;
    Aq40::Encounter _encounter = Aq40::None;
    std::vector<ObjectGuid> _units;
    std::map<ObjectGuid, ObjectGuid> _owners;
    std::array<ObjectGuid, 3> _platformTanks{};
    std::array<Position, 3> _platforms{};
    bool _platformsReady = false;
    std::map<ObjectGuid, uint32> _skeramBodies;
    std::map<ObjectGuid, Position> _skeramPositions;
    std::array<ObjectGuid, 2> _twinTanks{};
    ObjectGuid _twinLock;                    // the warlock holding Vek'lor right now
    std::array<ObjectGuid, 2> _twinLocks{};  // per side: Square = side 0 (Vek'nilash's), Moon = side 1
    std::chrono::steady_clock::time_point _twinLastSwap{};
    ObjectGuid _twinWaiter;
    bool _twinTeleported = false;
    std::map<ObjectGuid, std::pair<float, std::chrono::steady_clock::time_point>> _waitAngles;
    std::array<Position, 2> _twinLast{};
    bool _twinLastReady = false;
    std::array<Position, 2> _twinSides{};
    std::map<ObjectGuid, uint32> _raidSides;
    bool _sidesReady = false;
    ObjectGuid _focus;
    ObjectGuid _mainTank;
    std::vector<ObjectGuid> _soakers;
    std::map<ObjectGuid, float> _angles;
    std::map<ObjectGuid, uint32> _cthunSpots;
    bool _cthunRedeal = false;
    ObjectGuid _cthunPuller;  // takes the three opening Eye Beams; stays put while the raid spreads
    float _lastGlare = 0.0f;
    float _glareDirection = 0.0f;
    bool _glareActive = false;
    std::chrono::steady_clock::time_point _refresh{};
    std::chrono::steady_clock::time_point _encounterStart{};
};

class Aq40SkeramInterruptAction : public Action
{
public:
    Aq40SkeramInterruptAction(PlayerbotAI* ai) : Action(ai, "aq40 skeram interrupt") {}
    bool Execute(Event event) override;
};

// Before the Twin Emperors pull, the raid-marked warlocks walk to their start spots on their own:
// neither "follow" (drags them back to the master) nor "stay" (keeps pulling them back mid-fight) works.
bool Aq40TwinPrepullSpot(PlayerbotAI* ai, Position& spot);
bool Aq40CthunPrepullSpot(PlayerbotAI* ai, Position& spot);

class Aq40TwinPrepullTrigger : public Trigger
{
public:
    Aq40TwinPrepullTrigger(PlayerbotAI* ai) : Trigger(ai, "aq40 twins prepull", 1) {}
    bool IsActive() override;
};

class Aq40TwinPrepullAction : public MovementAction
{
public:
    Aq40TwinPrepullAction(PlayerbotAI* ai) : MovementAction(ai, "aq40 twins prepull") {}
    bool Execute(Event event) override;
};

class Aq40Trigger : public Trigger
{
public:
    Aq40Trigger(PlayerbotAI* ai) : Trigger(ai, "aq40 encounter", 500) {}
    bool IsActive() override;
};

class Aq40MoveAction : public MovementAction
{
public:
    Aq40MoveAction(PlayerbotAI* ai, bool safety, bool tactical = false)
        : MovementAction(ai, safety     ? "aq40 safety"
                             : tactical ? "aq40 tactics"
                                        : "aq40 position"),
          _safety(safety),
          _tactical(tactical)
    {
    }
    bool Execute(Event event) override;

private:
    bool _safety;
    bool _tactical;
    std::chrono::steady_clock::time_point _exitAttempt{};
    std::chrono::steady_clock::time_point _spitAttempt{};
};

class Aq40Multiplier : public Multiplier
{
public:
    Aq40Multiplier(PlayerbotAI* ai) : Multiplier(ai, "aq40 encounter") {}
    float GetValue(Action* action) override;
};

#endif

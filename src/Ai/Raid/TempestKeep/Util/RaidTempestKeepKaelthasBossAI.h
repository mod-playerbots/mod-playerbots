#ifndef _PLAYERBOT_RAIDTEMPESTKEEPKAELTHASBOSSAI_H_
#define _PLAYERBOT_RAIDTEMPESTKEEPKAELTHASBOSSAI_H_

#include "ScriptedCreature.h"

enum KTYells
{
};

enum KTPhases
{
    PHASE_NONE                          = 0,
    PHASE_SINGLE_ADVISOR                = 1,
    PHASE_WEAPONS                       = 2,
    PHASE_TRANSITION                    = 3,
    PHASE_ALL_ADVISORS                  = 4,
    PHASE_FINAL                         = 5
};

enum KTActions
{
};

struct boss_kaelthas : public BossAI
{
    boss_kaelthas(Creature* creature);

    void PrepareAdvisors();
    void SetRoomState(GOState state);
    void Reset() override;
    void AttackStart(Unit* who) override;
    void MoveInLineOfSight(Unit* who) override;
    void KilledUnit(Unit* victim) override;
    void JustSummoned(Creature* summon) override;
    void SpellHit(Unit* caster, SpellInfo const* spell) override;
    void MovementInform(uint32 type, uint32 point) override;
    void ExecuteMiddleEvent();
    void IntroduceNewAdvisor(KTYells talkIntroduction, KTActions kaelAction);
    void PhaseEnchantedWeaponsExecute();
    void PhaseAllAdvisorsExecute();
    void PhaseKaelExecute();
    void UpdateAI(uint32 diff) override;
    bool CheckEvadeIfOutOfCombatArea() const override;
    void JustDied(Unit* killer) override;

    uint32 GetPhase() const { return _phase; } // This is the only addition to the existing class

private:
    uint32 _phase;
    uint8 _advisorsAlive;
    bool _transitionSceneReached = false;
};

#endif

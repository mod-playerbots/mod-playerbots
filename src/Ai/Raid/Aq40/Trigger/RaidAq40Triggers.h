#ifndef _PLAYERBOT_RAIDAQ40TRIGGERS_H
#define _PLAYERBOT_RAIDAQ40TRIGGERS_H

#include "Playerbots.h"
#include "../RaidAq40BossHelper.h"
#include "Trigger.h"

class Aq40BotIsNotInCombatTrigger : public Trigger
{
public:
    Aq40BotIsNotInCombatTrigger(PlayerbotAI* botAI) : Trigger(botAI, "aq40 bot is not in combat") {}
    bool IsActive() override;
};

// Low-priority AQ40 housekeeping trigger used to toggle encounter-specific
// resistance strategies on and off as bosses change.
class Aq40ResistanceStrategyTrigger : public Trigger
{
public:
    Aq40ResistanceStrategyTrigger(PlayerbotAI* botAI)
        : Trigger(botAI, "aq40 resistance strategy check") {}
    bool IsActive() override;
};

class Aq40SkeramActiveTrigger : public Trigger
{
public:
    Aq40SkeramActiveTrigger(PlayerbotAI* botAI) : Trigger(botAI, "aq40 skeram active") {}
    bool IsActive() override;
};

class Aq40SkeramBlinkTrigger : public Trigger
{
public:
    Aq40SkeramBlinkTrigger(PlayerbotAI* botAI) : Trigger(botAI, "aq40 skeram blinked") {}
    bool IsActive() override;
};

class Aq40SkeramArcaneExplosionTrigger : public Trigger
{
public:
    Aq40SkeramArcaneExplosionTrigger(PlayerbotAI* botAI) : Trigger(botAI, "aq40 skeram interrupt cast") {}
    bool IsActive() override;
};

class Aq40SkeramMindControlTrigger : public Trigger
{
public:
    Aq40SkeramMindControlTrigger(PlayerbotAI* botAI) : Trigger(botAI, "aq40 skeram mc detected") {}
    bool IsActive() override;
};

class Aq40SkeramExecutePhaseTrigger : public Trigger
{
public:
    Aq40SkeramExecutePhaseTrigger(PlayerbotAI* botAI) : Trigger(botAI, "aq40 skeram execute phase") {}
    bool IsActive() override;
};

class Aq40SarturaActiveTrigger : public Trigger
{
public:
    Aq40SarturaActiveTrigger(PlayerbotAI* botAI) : Trigger(botAI, "aq40 sartura active") {}
    bool IsActive() override;
};

class Aq40SarturaWhirlwindTrigger : public Trigger
{
public:
    Aq40SarturaWhirlwindTrigger(PlayerbotAI* botAI) : Trigger(botAI, "aq40 sartura whirlwind") {}
    bool IsActive() override;
};

class Aq40BugTrioActiveTrigger : public Trigger
{
public:
    Aq40BugTrioActiveTrigger(PlayerbotAI* botAI) : Trigger(botAI, "aq40 bug trio active") {}
    bool IsActive() override;
};

class Aq40BugTrioHealCastTrigger : public Trigger
{
public:
    Aq40BugTrioHealCastTrigger(PlayerbotAI* botAI) : Trigger(botAI, "aq40 bug trio heal cast") {}
    bool IsActive() override;
};

class Aq40BugTrioFearTrigger : public Trigger
{
public:
    Aq40BugTrioFearTrigger(PlayerbotAI* botAI) : Trigger(botAI, "aq40 bug trio fear risk") {}
    bool IsActive() override;
};

class Aq40BugTrioPoisonCloudTrigger : public Trigger
{
public:
    Aq40BugTrioPoisonCloudTrigger(PlayerbotAI* botAI) : Trigger(botAI, "aq40 bug trio poison cloud") {}
    bool IsActive() override;
};

class Aq40FankrissActiveTrigger : public Trigger
{
public:
    Aq40FankrissActiveTrigger(PlayerbotAI* botAI) : Trigger(botAI, "aq40 fankriss active") {}
    bool IsActive() override;
};

class Aq40FankrissSpawnedTrigger : public Trigger
{
public:
    Aq40FankrissSpawnedTrigger(PlayerbotAI* botAI) : Trigger(botAI, "aq40 fankriss spawn active") {}
    bool IsActive() override;
};

class Aq40FankrissMortalWoundTrigger : public Trigger
{
public:
    Aq40FankrissMortalWoundTrigger(PlayerbotAI* botAI) : Trigger(botAI, "aq40 fankriss mortal wound") {}
    bool IsActive() override;
};

class Aq40TrashActiveTrigger : public Trigger
{
public:
    Aq40TrashActiveTrigger(PlayerbotAI* botAI) : Trigger(botAI, "aq40 trash active") {}
    bool IsActive() override;
};

class Aq40TrashDangerousAoeTrigger : public Trigger
{
public:
    Aq40TrashDangerousAoeTrigger(PlayerbotAI* botAI) : Trigger(botAI, "aq40 trash dangerous aoe") {}
    bool IsActive() override;
};

class Aq40HuhuranActiveTrigger : public Trigger
{
public:
    Aq40HuhuranActiveTrigger(PlayerbotAI* botAI) : Trigger(botAI, "aq40 huhuran active") {}
    bool IsActive() override;
};

class Aq40HuhuranPoisonPhaseTrigger : public Trigger
{
public:
    Aq40HuhuranPoisonPhaseTrigger(PlayerbotAI* botAI) : Trigger(botAI, "aq40 huhuran poison phase") {}
    bool IsActive() override;
};

class Aq40TwinApproachTrigger : public Trigger
{
public:
    Aq40TwinApproachTrigger(PlayerbotAI* botAI) : Trigger(botAI, "aq40 twin approach") {}
    bool IsActive() override;
};

class Aq40TwinPrePullReadyTrigger : public Trigger
{
public:
    Aq40TwinPrePullReadyTrigger(PlayerbotAI* botAI) : Trigger(botAI, "aq40 twin prepull ready") {}
    bool IsActive() override;
};

class Aq40TwinDualPullTrigger : public Trigger
{
public:
    Aq40TwinDualPullTrigger(PlayerbotAI* botAI) : Trigger(botAI, "aq40 twin dual pull") {}
    bool IsActive() override;
};

class Aq40TwinSwapPrepTrigger : public Trigger
{
public:
    Aq40TwinSwapPrepTrigger(PlayerbotAI* botAI) : Trigger(botAI, "aq40 twin swap prep") {}
    bool IsActive() override;
};

class Aq40TwinActiveTrigger : public Trigger
{
public:
    Aq40TwinActiveTrigger(PlayerbotAI* botAI) : Trigger(botAI, "aq40 twin active") {}
    bool IsActive() override;
};

class Aq40TwinBlizzardTrigger : public Trigger
{
public:
    Aq40TwinBlizzardTrigger(PlayerbotAI* botAI) : Trigger(botAI, "aq40 twin blizzard") {}
    bool IsActive() override;
};

class Aq40TwinExplodeBugTrigger : public Trigger
{
public:
    Aq40TwinExplodeBugTrigger(PlayerbotAI* botAI) : Trigger(botAI, "aq40 twin explode bug") {}
    bool IsActive() override;
};

class Aq40TwinArcaneBurstRiskTrigger : public Trigger
{
public:
    Aq40TwinArcaneBurstRiskTrigger(PlayerbotAI* botAI) : Trigger(botAI, "aq40 twin arcane burst risk") {}
    bool IsActive() override;
};

class Aq40TwinSplitRiskTrigger : public Trigger
{
public:
    Aq40TwinSplitRiskTrigger(PlayerbotAI* botAI) : Trigger(botAI, "aq40 twin split risk") {}
    bool IsActive() override;
};

class Aq40TwinPostSwapHoldTrigger : public Trigger
{
public:
    Aq40TwinPostSwapHoldTrigger(PlayerbotAI* botAI) : Trigger(botAI, "aq40 twin post swap hold") {}
    bool IsActive() override;
};

class Aq40OuroActiveTrigger : public Trigger
{
public:
    Aq40OuroActiveTrigger(PlayerbotAI* botAI) : Trigger(botAI, "aq40 ouro active") {}
    bool IsActive() override;
};

class Aq40OuroScarabsTrigger : public Trigger
{
public:
    Aq40OuroScarabsTrigger(PlayerbotAI* botAI) : Trigger(botAI, "aq40 ouro scarabs present") {}
    bool IsActive() override;
};

class Aq40OuroSweepTrigger : public Trigger
{
public:
    Aq40OuroSweepTrigger(PlayerbotAI* botAI) : Trigger(botAI, "aq40 ouro sweep risk") {}
    bool IsActive() override;
};

class Aq40OuroSandBlastRiskTrigger : public Trigger
{
public:
    Aq40OuroSandBlastRiskTrigger(PlayerbotAI* botAI) : Trigger(botAI, "aq40 ouro sand blast risk") {}
    bool IsActive() override;
};

class Aq40OuroSubmergeTrigger : public Trigger
{
public:
    Aq40OuroSubmergeTrigger(PlayerbotAI* botAI) : Trigger(botAI, "aq40 ouro submerge hazard") {}
    bool IsActive() override;
};

class Aq40ViscidusActiveTrigger : public Trigger
{
public:
    Aq40ViscidusActiveTrigger(PlayerbotAI* botAI) : Trigger(botAI, "aq40 viscidus active") {}
    bool IsActive() override;
};

class Aq40ViscidusFrozenTrigger : public Trigger
{
public:
    Aq40ViscidusFrozenTrigger(PlayerbotAI* botAI) : Trigger(botAI, "aq40 viscidus frozen") {}
    bool IsActive() override;
};

class Aq40ViscidusGlobTrigger : public Trigger
{
public:
    Aq40ViscidusGlobTrigger(PlayerbotAI* botAI) : Trigger(botAI, "aq40 viscidus globs present") {}
    bool IsActive() override;
};

class Aq40CthunActiveTrigger : public Trigger
{
public:
    Aq40CthunActiveTrigger(PlayerbotAI* botAI) : Trigger(botAI, "aq40 cthun active") {}
    bool IsActive() override;
};

class Aq40CthunPhase2Trigger : public Trigger
{
public:
    Aq40CthunPhase2Trigger(PlayerbotAI* botAI) : Trigger(botAI, "aq40 cthun phase2") {}
    bool IsActive() override;
};

class Aq40CthunAddsPresentTrigger : public Trigger
{
public:
    Aq40CthunAddsPresentTrigger(PlayerbotAI* botAI) : Trigger(botAI, "aq40 cthun adds present") {}
    bool IsActive() override;
};

class Aq40CthunDarkGlareTrigger : public Trigger
{
public:
    Aq40CthunDarkGlareTrigger(PlayerbotAI* botAI) : Trigger(botAI, "aq40 cthun dark glare") {}
    bool IsActive() override;
};

class Aq40CthunInStomachTrigger : public Trigger
{
public:
    Aq40CthunInStomachTrigger(PlayerbotAI* botAI) : Trigger(botAI, "aq40 cthun in stomach") {}
    bool IsActive() override;
};

class Aq40CthunVulnerableTrigger : public Trigger
{
public:
    Aq40CthunVulnerableTrigger(PlayerbotAI* botAI) : Trigger(botAI, "aq40 cthun vulnerable") {}
    bool IsActive() override;
};

class Aq40CthunEyeCastTrigger : public Trigger
{
public:
    Aq40CthunEyeCastTrigger(PlayerbotAI* botAI) : Trigger(botAI, "aq40 cthun eye cast") {}
    bool IsActive() override;
};

#endif

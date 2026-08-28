/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_GRUULTRIGGERS_H
#define PLAYERBOTS_GRUULTRIGGERS_H

#include "Trigger.h"

class GruulsLairNoEncounterInProgress : public Trigger
{
public:
    GruulsLairNoEncounterInProgress(PlayerbotAI* botAI)
        : Trigger(botAI, "gruul's lair no encounter in progress") {}
    bool IsActive() override;
};

class HighKingMaulgarThreeOgresNeedMeleeTanksTrigger : public Trigger
{
public:
    HighKingMaulgarThreeOgresNeedMeleeTanksTrigger(PlayerbotAI* botAI)
        : Trigger(botAI, "high king maulgar three ogres need melee tanks") {}
    bool IsActive() override;
};

class HighKingMaulgarKroshNeedsMageTankTrigger : public Trigger
{
public:
    HighKingMaulgarKroshNeedsMageTankTrigger(PlayerbotAI* botAI)
        : Trigger(botAI, "high king maulgar krosh needs mage tank") {}
    bool IsActive() override;
};

class HighKingMaulgarKigglerNeedsMoonkinTankTrigger : public Trigger
{
public:
    HighKingMaulgarKigglerNeedsMoonkinTankTrigger(PlayerbotAI* botAI)
        : Trigger(botAI, "high king maulgar kiggler needs moonkin tank") {}
    bool IsActive() override;
};

class HighKingMaulgarDeterminingKillOrderTrigger : public Trigger
{
public:
    HighKingMaulgarDeterminingKillOrderTrigger(PlayerbotAI* botAI)
        : Trigger(botAI, "high king maulgar determining kill order") {}
    bool IsActive() override;
};

class HighKingMaulgarBossChannelingWhirlwindTrigger : public Trigger
{
public:
    HighKingMaulgarBossChannelingWhirlwindTrigger(PlayerbotAI* botAI)
        : Trigger(botAI, "high king maulgar boss channeling whirlwind") {}
    bool IsActive() override;
};

class HighKingMaulgarKroshCastsBlastWaveTrigger : public Trigger
{
public:
    HighKingMaulgarKroshCastsBlastWaveTrigger(PlayerbotAI* botAI)
        : Trigger(botAI, "high king maulgar krosh casts blast wave") {}
    bool IsActive() override;
};

class HighKingMaulgarWildFelStalkerSpawnedTrigger : public Trigger
{
public:
    HighKingMaulgarWildFelStalkerSpawnedTrigger(PlayerbotAI* botAI)
        : Trigger(botAI, "high king maulgar wild fel stalker spawned") {}
    bool IsActive() override;
};

class HighKingMaulgarPullingOgreCouncilTrigger : public Trigger
{
public:
    HighKingMaulgarPullingOgreCouncilTrigger(PlayerbotAI* botAI)
        : Trigger(botAI, "high king maulgar pulling ogre council") {}
    bool IsActive() override;
};

class HighKingMaulgarBossCastsIntimidatingRoarTrigger : public Trigger
{
public:
    HighKingMaulgarBossCastsIntimidatingRoarTrigger(PlayerbotAI* botAI)
        : Trigger(botAI, "high king maulgar boss casts intimidating roar") {}
    bool IsActive() override;
};

class GruulTheDragonkillerShouldBeTankedTrigger : public Trigger
{
public:
    GruulTheDragonkillerShouldBeTankedTrigger(PlayerbotAI* botAI)
        : Trigger(botAI, "gruul the dragonkiller should be tanked") {}
    bool IsActive() override;
};

class GruulTheDragonkillerRangedShouldSpreadTrigger : public Trigger
{
public:
    GruulTheDragonkillerRangedShouldSpreadTrigger(PlayerbotAI* botAI)
        : Trigger(botAI, "gruul the dragonkiller ranged should spread") {}
    bool IsActive() override;
};

class GruulTheDragonkillerIncomingShatterTrigger : public Trigger
{
public:
    GruulTheDragonkillerIncomingShatterTrigger(PlayerbotAI* botAI)
        : Trigger(botAI, "gruul the dragonkiller incoming shatter") {}
    bool IsActive() override;
};

#endif

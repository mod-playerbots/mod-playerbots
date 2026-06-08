// RaidOnyxiaActions.h
#ifndef _PLAYERBOT_RAIDONYXIAACTIONS_H_
#define _PLAYERBOT_RAIDONYXIAACTIONS_H_

#include "AttackAction.h"
#include "GenericSpellActions.h"
#include "MovementActions.h"

class PlayerbotAI;

class RaidOnyxiaMoveToSideAction : public MovementAction
{
public:
    RaidOnyxiaMoveToSideAction(PlayerbotAI* botAI, std::string const name = "ony move to side")
        : MovementAction(botAI, name)
    {
    }
    bool Execute(Event event) override;
};

class RaidOnyxiaSpreadOutAction : public MovementAction
{
public:
    RaidOnyxiaSpreadOutAction(PlayerbotAI* botAI, std::string const name = "ony spread out")
        : MovementAction(botAI, name)
    {
    }
    bool Execute(Event event) override;
};

struct SafeZone
{
    Position pos;
    float radius;
};

class RaidOnyxiaMoveToSafeZoneAction : public MovementAction
{
public:
    RaidOnyxiaMoveToSafeZoneAction(PlayerbotAI* botAI, std::string const name = "ony move to safe zone")
        : MovementAction(botAI, name)
    {
    }
    bool Execute(Event event) override;

private:
    static std::vector<SafeZone> GetSafeZonesForBreath(uint32 spellId)
    {
        // Safe zone coordinates based on the map
        // Assumes Onyxia's lair map coordinates

        switch (spellId)
        {
            case 17086:  // N to S
            case 18351:  // S to N
                return {
                    SafeZone{Position(-10.0f, -180.0f, -87.0f), 5.0f},
                    SafeZone{Position(-20.0f, -250.0f, -88.0f), 5.0f}
                };  // Bottom Safe Zone

            case 18576:  // E to W
            case 18609:  // W to E
                return {
                    SafeZone{Position(20.0f, -210.0f, -85.5f), 5.0f},
                    SafeZone{Position(-75.0f, -210.0f, -83.4f), 5.0f},
                };  // Left Safe Zone

            case 18564:  // SE to NW
            case 18584:  // NW to SE
                return {
                    SafeZone{Position(-60.0f, -195.0f, -85.0f), 5.0f},
                    SafeZone{Position(10.0f, -240.0f, -85.9f), 5.0f},
                };  // NW Safe Zone

            case 18596:  // SW to NE
            case 18617:  // NE to SW
                return {
                    SafeZone{Position(7.0f, -185.0f, -86.2f), 5.0f},
                    SafeZone{Position(-60.0f, -240.0f, -85.2f), 5.0f},
                };  // NE Safe Zone

            default:
                return {
                    SafeZone{Position(-40.0f, -214.0f, -86.6f), 5.0f}
                };  // Fallback center - shouldn't ever happen
        }
    }
};

class RaidOnyxiaKillWhelpsAction : public AttackAction
{
public:
    RaidOnyxiaKillWhelpsAction(PlayerbotAI* botAI, std::string const name = "ony kill whelps")
        : AttackAction(botAI, name)
    {
    }

    bool Execute(Event event) override;
};

class OnyxiaAvoidEggsAction : public MovementAction
{
public:
    OnyxiaAvoidEggsAction(PlayerbotAI* botAI) : MovementAction(botAI, "ony avoid eggs move") {}

    bool Execute(Event event) override;
};

#endif

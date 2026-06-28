#ifndef _PLAYERBOT_RAIDAQ40HELPERS_SHARED_H_
#define _PLAYERBOT_RAIDAQ40HELPERS_SHARED_H_

#include <string>

#include "ObjectGuid.h"
#include "Player.h"
#include "PlayerbotAI.h"

namespace Aq40Helpers
{
struct RadialMovePosition
{
    float x = 0.0f;
    float y = 0.0f;
};

bool HasManagedResistanceState(Player* bot);
bool HasManagedResistanceStrategy(Player* bot, PlayerbotAI* botAI);
bool ClearManagedResistanceStrategies(Player* bot, PlayerbotAI* botAI);
bool IsResistanceManagementNeeded(Player* bot, PlayerbotAI* botAI, GuidVector const& attackers);
bool ShouldRunOutOfCombatMaintenance(Player* bot, PlayerbotAI* botAI);
bool ShouldSuppressTwinPrePullMaintenance(Player* bot, PlayerbotAI* botAI, char const* trigger = nullptr);
bool HasPersistentEncounterState(Player* bot);
bool ResetEncounterState(Player* bot);
bool TryRecoverAq40FollowState(Player* bot, PlayerbotAI* botAI, std::string const& eventKey,
                               std::string const& stateKey, bool executeFollowMovement = false);
bool SetRaidTargetIcon(Player* bot, Unit* target, uint8 iconId, std::string const& boss,
                       std::string const& marker);
bool ClearRaidTargetIcon(Player* bot, uint8 iconId, std::string const& boss, std::string const& marker);
Unit* ResolveRaidTargetIcon(Player* bot, PlayerbotAI* botAI, uint8 iconId);
bool SetRti(PlayerbotAI* botAI, std::string const& rtiName);
bool SetRtiTarget(PlayerbotAI* botAI, std::string const& rtiName, Unit* target);
std::string GetAq40LogToken(std::string value);
std::string GetAq40LogUnit(Unit* unit);
std::string GetAq40LogRole(Player* bot, PlayerbotAI* botAI);
RadialMovePosition GetRadialMovePosition(Player* bot, Unit* source, float desiredDistance);
void LogAq40Info(Player* bot, std::string const& eventKey, std::string const& stateKey,
                 std::string const& fields = "", uint32 throttleMs = 0);
void LogAq40Warn(Player* bot, std::string const& eventKey, std::string const& stateKey,
                 std::string const& fields = "", uint32 throttleMs = 0);
void LogAq40Target(Player* bot, std::string const& boss, std::string const& reason, Unit* target,
                   uint32 throttleMs = 0);
}    // namespace Aq40Helpers

#endif

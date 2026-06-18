#ifndef _PLAYERBOT_RAIDAQ40HELPERS_SHARED_H_
#define _PLAYERBOT_RAIDAQ40HELPERS_SHARED_H_

#include <string>

#include "ObjectGuid.h"
#include "Player.h"
#include "PlayerbotAI.h"

namespace Aq40Helpers
{
bool HasManagedResistanceState(Player* bot);
bool HasManagedResistanceStrategy(Player* bot, PlayerbotAI* botAI);
bool ClearManagedResistanceStrategies(Player* bot, PlayerbotAI* botAI);
bool IsResistanceManagementNeeded(Player* bot, PlayerbotAI* botAI, GuidVector const& attackers);
bool ShouldRunOutOfCombatMaintenance(Player* bot, PlayerbotAI* botAI);
bool HasPersistentEncounterState(Player* bot);
bool ResetEncounterState(Player* bot);
std::string GetAq40LogToken(std::string value);
std::string GetAq40LogUnit(Unit* unit);
std::string GetAq40LogRole(Player* bot, PlayerbotAI* botAI);
void LogAq40Info(Player* bot, std::string const& eventKey, std::string const& stateKey,
                 std::string const& fields = "", uint32 throttleMs = 0);
void LogAq40Warn(Player* bot, std::string const& eventKey, std::string const& stateKey,
                 std::string const& fields = "", uint32 throttleMs = 0);
void LogAq40Target(Player* bot, std::string const& boss, std::string const& reason, Unit* target,
                   uint32 throttleMs = 0);
}    // namespace Aq40Helpers

#endif

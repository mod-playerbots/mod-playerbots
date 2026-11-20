/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "RaidTocMultipliers.h"
#include "AiObjectContext.h"
#include "Playerbots.h"

float TocStarfallMultiplier::GetValue(Action* action)
{
    if (action->getName() != "starfall")
        return 1.0f;

    Player* bot = botAI->GetBot();
    if (bot->GetMapId() != 649)  // Trial of the Crusader only
        return 1.0f;

    // Look the boss up by name to cover the per-difficulty entry variants.
    AiObjectContext* context = botAI->GetAiObjectContext();
    Unit* anub = context->GetValue<Unit*>("find target", "anub'arak")->Get();
    if (!anub || !anub->IsAlive() || !anub->IsInCombat())
        return 1.0f;

    // Phase 3 starts at 30% health (matches the boss script). Before that the
    // splash would break the frost spheres and the add handling.
    return anub->GetHealthPct() > 30.0f ? 0.0f : 1.0f;
}

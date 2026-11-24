/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "NexTriggers.h"
#include "Playerbots.h"
#include "ThreatHelpers.h"

bool FactionCommanderWhirlwindTrigger::IsActive()
{
    Unit* boss = nullptr;
    uint8 faction = bot->GetTeamId();

    switch (bot->GetMap()->GetDifficulty())
    {
        case DUNGEON_DIFFICULTY_NORMAL:
            if (faction == TEAM_ALLIANCE)
            {
                boss = FindThreatTarget(bot, "horde commander");
            }
            else //if (faction == TEAM_HORDE)
            {
                boss = FindThreatTarget(bot, "alliance commander");
            }
            break;
        case DUNGEON_DIFFICULTY_HEROIC:
            if (faction == TEAM_ALLIANCE)
            {
                boss = FindThreatTarget(bot, "commander kolurg");
            }
            else //if (faction == TEAM_HORDE)
            {
                boss = FindThreatTarget(bot, "commander stoutbeard");
            }
            break;
        default:
            break;
    }

    if (boss && boss->HasUnitState(UNIT_STATE_CASTING))
    {
        if (boss->FindCurrentSpellBySpellId(SPELL_WHIRLWIND))
        {
            return true;
        }
    }
    return false;
}

bool TelestraFirebombTrigger::IsActive()
{
    if (PlayerbotAI::IsMelee(bot)) { return false; }

    Unit* boss = FindThreatTarget(bot, "grand magus telestra");
    // Avoid split phase with the fake Telestra units, only match the true boss id
    return boss && boss->GetEntry() == NPC_TELESTRA;
}

bool TelestraSplitPhaseTrigger::IsActive()
{
    Unit* boss = FindThreatTarget(bot, "grand magus telestra");
    // Only match split phase with the fake Telestra units
    return boss && boss->GetEntry() != NPC_TELESTRA;
}

bool ChaoticRiftTrigger::IsActive()
{
    Unit* boss = FindThreatTarget(bot, "anomalus");
    return boss && boss->HasAura(BUFF_RIFT_SHIELD);
}

bool OrmorokSpikesTrigger::IsActive()
{
    if (!FindThreatTarget(bot, "ormorok the tree-shaper") || !PlayerbotAI::IsTank(bot)) { return false; }

    GuidVector objects = AI_VALUE(GuidVector, "closest game objects");
    for (auto i = objects.begin(); i != objects.end(); ++i)
    {
        GameObject* go = botAI->GetGameObject(*i);
        if (go && go->GetEntry() == GO_CRYSTAL_SPIKE)
        {
            return true;
        }
    }
    return false;
}

bool OrmorokStackTrigger::IsActive()
{
    return FindThreatTarget(bot, "ormorok the tree-shaper") && !PlayerbotAI::IsTank(bot);
}

bool IntenseColdTrigger::IsActive()
{
    // Adjust as needed - too much interrupting loses dps time,
    // but too many stacks is deadly. Assuming 3-5 is a good number to clear
    int stackThreshold = 5;
    return FindThreatTarget(bot, "keristrasza") && botAI->GetAura("intense cold", bot, false, false, stackThreshold);
}

bool KeristraszaPositioningTrigger::IsActive()
{
    // Include healers here for now, otherwise they stand in things
    return FindThreatTarget(bot, "keristrasza") && !PlayerbotAI::IsTank(bot) && !PlayerbotAI::IsRangedDps(bot);
}

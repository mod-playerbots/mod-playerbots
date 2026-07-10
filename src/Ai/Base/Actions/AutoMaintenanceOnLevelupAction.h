/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_AUTOMAINTENANCEONLEVELUPACTION_H
#define PLAYERBOTS_AUTOMAINTENANCEONLEVELUPACTION_H

#include "Action.h"

class PlayerbotAI;

class AutoMaintenanceOnLevelupAction : public Action
{
public:
    AutoMaintenanceOnLevelupAction(PlayerbotAI* botAI, std::string const name = "auto maintenance on levelup")
        : Action(botAI, name)
    {
    }

    bool Execute(Event event);

protected:
    void AutoTeleportForLevel();
    void AutoPickTalents();
    void AutoLearnSpell();
    void AutoUpgradeEquip();
    void LearnSpells(std::ostringstream* out);
    void LearnTrainerSpells(std::ostringstream* out);
    void LearnQuestSpells(std::ostringstream* out);
    std::string const FormatSpell(SpellInfo const* sInfo);
};

#endif

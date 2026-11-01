/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_PORTALACTION_H
#define PLAYERBOTS_PORTALACTION_H

#include "Action.h"

class PortalAction : public Action
{
public:
    PortalAction(PlayerbotAI* botAI) : Action(botAI, "portal") {}
    bool isUseful() override;

    bool Execute(Event event) override;

private:
    std::vector<SpellInfo const*> GetKnownPortals() const;
    bool HasReagents(SpellInfo const* spellInfo) const;
    void Tell(std::string const& key, std::string const& defaultText,
              std::map<std::string, std::string> const& placeholders = {});
};

#endif

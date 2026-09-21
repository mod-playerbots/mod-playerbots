/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_CHECKMAILACTION_H
#define PLAYERBOTS_CHECKMAILACTION_H

#include "Action.h"
#include "DatabaseEnvFwd.h"

class PlayerbotAI;

struct Mail;

class CheckMailAction : public Action
{
public:
    CheckMailAction(PlayerbotAI* botAI) : Action(botAI, "check mail") {}

    bool Execute(Event event) override;
    bool isUseful() override;

private:
    void ProcessMail(Mail* mail, Player* owner, CharacterDatabaseTransaction trans);
};

#endif

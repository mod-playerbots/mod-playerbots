/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "AllSpellScript.h"
#include "Playerbots.h"
#include "ZGHelpers.h"

using namespace ZulGurubHelpers;

// A bot's engine does not run while it is casting, so the gazed bot is asked from here to drop a cast under way.
class BloodlordMandokirThreateningGazeSpellListenerScript : public AllSpellScript
{
public:
    BloodlordMandokirThreateningGazeSpellListenerScript()
        : AllSpellScript("BloodlordMandokirThreateningGazeSpellListenerScript")
    {
    }

    void OnSpellPrepare(Spell* spell, Unit* /*caster*/, SpellInfo const* spellInfo) override
    {
        if (spellInfo->Id != static_cast<uint32>(ZulGurubSpells::SPELL_THREATENING_GAZE))
            return;

        Unit* target = spell->m_targets.GetUnitTarget();
        Player* player = target ? target->ToPlayer() : nullptr;
        if (!player)
            return;

        PlayerbotAI* botAI = GET_PLAYERBOT_AI(player);
        if (botAI && botAI->HasStrategy("zulgurub", BOT_STATE_COMBAT))
            botAI->RequestSpellInterrupt();
    }
};

void AddSC_ZulGurubBotScripts() { new BloodlordMandokirThreateningGazeSpellListenerScript(); }

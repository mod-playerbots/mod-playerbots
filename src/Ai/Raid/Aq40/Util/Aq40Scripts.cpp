/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "AllSpellScript.h"
#include "Aq40Helpers.h"
#include "Playerbots.h"
#include "Spell.h"

using namespace Aq40Helpers;

// The emperors swap instantly, attack the nearest player 2 s later, and Arcane Burst returns 2.3 s after that. Wake
// every AQ40 bot on the teleport so the melee under Vek'lor run and the landing tank taunts in time.
class TwinTeleportBotScript : public AllSpellScript
{
public:
    TwinTeleportBotScript() : AllSpellScript("TwinTeleportBotScript") {}

    void OnSpellPrepare(Spell* /*spell*/, Unit* caster, SpellInfo const* spellInfo) override
    {
        if (!spellInfo ||
            (spellInfo->Id != Id(Aq40Spells::SPELL_TWIN_TELEPORT) &&
             spellInfo->Id != Id(Aq40Spells::SPELL_TWIN_TELEPORT_VISUAL)) ||
            !caster || caster->GetMapId() != AQ40_MAP_ID || !IsTwinEmperor(caster))
            return;
        for (auto const& ref : caster->GetMap()->GetPlayers())
        {
            Player* player = ref.GetSource();
            if (!player || !player->IsAlive() || HasTrueFulfillment(player))
                continue;
            PlayerbotAI* botAI = GET_PLAYERBOT_AI(player);
            if (!botAI || !botAI->HasStrategy("aq40", BOT_STATE_COMBAT))
                continue;
            botAI->SetNextCheckDelay(0);
        }
    }
};

void AddSC_Aq40BotScripts()
{
    new TwinTeleportBotScript();
}

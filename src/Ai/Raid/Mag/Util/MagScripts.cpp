/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "MagHelpers.h"
#include "AllSpellScript.h"
#include "ObjectAccessor.h"
#include "Player.h"
#include "Playerbots.h"
#include "ScriptMgr.h"
#include "Spell.h"
#include "Timer.h"

using namespace MagtheridonHelpers;

class MagtheridonBotSpellScript : public AllSpellScript
{
public:
    MagtheridonBotSpellScript() : AllSpellScript("MagtheridonBotSpellScript") {}

    void OnSpellCast(
        Spell* /*spell*/, Unit* caster, SpellInfo const* spellInfo, bool /*skipCheck*/) override
    {
        if (spellInfo->Id == static_cast<uint32>(MagtheridonSpells::SPELL_DEBRIS_SPAWN))
        {
            // Debris is a one-shot that has no prior warning other than a visual effect,
            // which necessitates this spell hook to track debris spawn positions
            const uint32 instanceId = caster->GetMap()->GetInstanceId();
            const uint32 now = getMSTime();

            activeDebrisPositions[instanceId].push_back({ caster->GetPosition(), now });

            constexpr uint32 debrisLifetimeMs = 10000;
            auto& positions = activeDebrisPositions[instanceId];
            positions.erase(std::remove_if(positions.begin(), positions.end(),
                [now](DebrisData const& d) {
                    return getMSTimeDiff(d.spawnTime, now) > debrisLifetimeMs;
                }),
                positions.end());

            // Interrupt casts for bots that could be standing in incoming debris
            Map::PlayerList const& players = caster->GetMap()->GetPlayers();
            for (Map::PlayerList::const_iterator it = players.begin(); it != players.end(); ++it)
            {
                Player* player = it->GetSource();
                if (!player || !player->IsAlive())
                    continue;

                PlayerbotAI* botAI = GET_PLAYERBOT_AI(player);
                if (!botAI || !botAI->HasStrategy("magtheridon", BOT_STATE_COMBAT))
                    continue;

                if (IsPositionInActiveDebris(
                        instanceId, player->GetPositionX(), player->GetPositionY()))
                {
                    botAI->RequestSpellInterrupt();
                }
            }
        }
        else if (spellInfo->Id == static_cast<uint32>(MagtheridonSpells::SPELL_QUAKE))
        {
            // To account for Blast Nova delay caused by Quake's DelayAll(6999ms)
            auto it = blastNovaTimer.find(caster->GetMap()->GetInstanceId());
            if (it != blastNovaTimer.end())
                it->second += 7;
        }
    }
};

void AddSC_MagtheridonBotScripts()
{
    new MagtheridonBotSpellScript();
}

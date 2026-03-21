#include "RaidTempestKeepHelpers.h"
#include "ObjectAccessor.h"
#include "Player.h"
#include "ScriptMgr.h"
#include "Spell.h"
#include "Timer.h"

using namespace TempestKeepHelpers;

class BossListenerScript : public AllSpellScript
{
public:
    BossListenerScript() : AllSpellScript("BossListenerScript") { }

    void OnSpellCast(Spell* spell, Unit* caster, SpellInfo const* spellInfo, bool /*skipCheck*/) override
    {
        if (spellInfo->Id != SPELL_ARCANE_ORB)
            return;

        std::list<TargetInfo> const& targets = *spell->GetUniqueTargetInfo();
        if (targets.empty())
            return;

        Player* target = ObjectAccessor::GetPlayer(*caster, targets.front().targetGUID);
        if (!target)
            return;

        auto& orbs = voidReaverArcaneOrbs[caster->GetMap()->GetInstanceId()];
        uint32 currentTime = getMSTime();

        ArcaneOrbData orbData;
        orbData.destination = target->GetPosition();
        orbData.castTime = currentTime;

        orbs.push_back(orbData);

        orbs.erase(std::remove_if(orbs.begin(), orbs.end(),
            [currentTime](const ArcaneOrbData& orb) {
                return getMSTimeDiff(orb.castTime, currentTime) > 5000;
            }), orbs.end());
    }
};

void AddSC_TempestKeepBotScripts()
{
    new BossListenerScript();
}

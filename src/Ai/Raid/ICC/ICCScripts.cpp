#include "ICCScripts.h"
#include "Player.h"
#include "ICCTriggers.h"
#include "ScriptMgr.h"
#include "Spell.h"
#include "SpellInfo.h"
#include "Timer.h"
#include <algorithm>

namespace IcecrownHelpers
{
    std::unordered_map<uint32, std::vector<MalleableGooImpact>> malleableGooImpacts;
    std::map<ObjectGuid, uint32> festergutGooWaitUntil;
    std::unordered_map<uint32, DefileCastInfo> defileCast;
    std::unordered_map<uint32, VileGasVictim> rotfaceVileGas;
    std::map<ObjectGuid, uint32> rotfaceVileGasWaitUntil;
}

class IccPutricideListenerScript : public AllSpellScript
{
public:
    IccPutricideListenerScript() : AllSpellScript("IccPutricideListenerScript") { }

    void OnSpellCast(Spell* spell, Unit* caster, SpellInfo const* spellInfo, bool /*skipCheck*/) override
    {
        if (!caster || !spellInfo)
            return;

        if (spellInfo->Id != SPELL_MALLEABLE_GOO_10N &&
            spellInfo->Id != SPELL_MALLEABLE_GOO_25N &&
            spellInfo->Id != SPELL_MALLEABLE_GOO_10H &&
            spellInfo->Id != SPELL_MALLEABLE_GOO_25H &&
            spellInfo->Id != SPELL_MALLEABLE_GOO_BALCONY)
            return;

        // Malleable Goo is cast triggered, so m_UniqueTargetInfo is not yet
        // populated at this point; read the explicit unit target directly.
        Unit* target = spell->m_targets.GetUnitTarget();
        if (!target || !target->IsPlayer())
            return;

        uint32 now = getMSTime();

        IcecrownHelpers::MalleableGooImpact impact;
        impact.position = target->GetPosition();
        impact.castTime = now;

        auto& impacts = IcecrownHelpers::malleableGooImpacts[caster->GetMap()->GetInstanceId()];
        impacts.push_back(impact);

        // Evict stale entries to keep the list bounded. Retention covers the
        // longest consumer window (Festergut avoid: 8s) + slack.
        impacts.erase(
            std::remove_if(impacts.begin(), impacts.end(),
                           [now](IcecrownHelpers::MalleableGooImpact const& i)
                           { return getMSTimeDiff(i.castTime, now) > 9000; }),
            impacts.end());
    }
};

class IccRotfaceListenerScript : public AllSpellScript
{
public:
    IccRotfaceListenerScript() : AllSpellScript("IccRotfaceListenerScript") { }

    void OnSpellCast(Spell* spell, Unit* caster, SpellInfo const* spellInfo, bool /*skipCheck*/) override
    {
        if (!caster || !spellInfo)
            return;

        if (spellInfo->Id != SPELL_VILE_GAS_H)
            return;

        // Professor Putricide casts vile gas from the balcony during Rotface
        // heroic. Filtering on caster entry keeps this hook scoped to the
        // Rotface encounter only (Festergut also uses 'vile gas' as the gas
        // spore aura name but a different spell ID).
        if (caster->GetEntry() != NPC_PROFESSOR_PUTRICIDE)
            return;

        Unit* target = spell->m_targets.GetUnitTarget();
        if (!target || !target->IsPlayer())
            return;

        IcecrownHelpers::VileGasVictim& entry = IcecrownHelpers::rotfaceVileGas[caster->GetMap()->GetInstanceId()];
        entry.victimGuid = target->GetGUID();
        entry.castTime = getMSTime();
    }
};

class IccLichKingListenerScript : public AllSpellScript
{
public:
    IccLichKingListenerScript() : AllSpellScript("IccLichKingListenerScript") { }

    // OnSpellPrepare fires at cast START (Spell::prepare). OnSpellCast fires
    // at cast END, which for Defile (2s cast time) is too late - the puddle
    // is already spawning and bots have no time to move out.
    void OnSpellPrepare(Spell* spell, Unit* caster, SpellInfo const* spellInfo) override
    {
        if (!caster || !spellInfo)
            return;

        if (spellInfo->Id != DEFILE_CAST_ID)
            return;

        Unit* target = spell->m_targets.GetUnitTarget();
        if (!target || !target->IsPlayer())
            return;

        IcecrownHelpers::DefileCastInfo& entry =
            IcecrownHelpers::defileCast[caster->GetMap()->GetInstanceId()];
        entry.targetGuid = target->GetGUID();
        entry.castTime = getMSTime();
    }
};

void AddSC_IcecrownBotScripts()
{
    new IccPutricideListenerScript();
    new IccRotfaceListenerScript();
    new IccLichKingListenerScript();
}

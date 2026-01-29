#include "RaidAq20Utils.h"

#include "SpellAuras.h"

uint32 const OSSIRIAN_BUFF = 25176;
uint32 const OSSIRIAN_DEBUFFS[] = {25177, 25178, 25180, 25181, 25183};
uint32 const OSSIRIAN_CRYSTAL_GO_ENTRY = 180619;

bool RaidAq20Utils::IsOssirianBuffActive(Unit& ossirian)
{
    return ossirian.HasAura(OSSIRIAN_BUFF);
}

int32_t RaidAq20Utils::GetOssirianDebuffTimeRemaining(Unit& ossirian)
{
    int32_t retVal = 0xffffff;

    for (uint32_t debuff : OSSIRIAN_DEBUFFS)
    {
        const AuraApplication* const auraApplication = ossirian.GetAuraApplication(debuff);

        if (auraApplication == nullptr)
        {
            continue;
        }

        const Aura* const aura = auraApplication->GetBase();

        if (aura == nullptr)
        {
            continue;
        }

        int32_t duration = aura->GetDuration();

        if (retVal > duration)
        {
            retVal = duration;
        }
    }

    return retVal;
}

GameObject* RaidAq20Utils::GetNearestCrystal(Unit& ossirian)
{
    return ossirian.FindNearestGameObject(OSSIRIAN_CRYSTAL_GO_ENTRY, 200.0f);
}

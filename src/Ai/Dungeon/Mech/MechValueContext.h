#ifndef PLAYERBOTS_MECHVALUECONTEXT_H
#define PLAYERBOTS_MECHVALUECONTEXT_H

#include "NamedObjectContext.h"
#include "Value.h"
#include "MechShared.h"

class SepethreaFixatingFlameValue : public CalculatedValue<ObjectGuid>
{
public:
    SepethreaFixatingFlameValue(PlayerbotAI* botAI)
        : CalculatedValue<ObjectGuid>(botAI, "sepethrea fixating flame", 200) {}

protected:
    ObjectGuid Calculate() override { return MechanarFlames::FindFixatingFlameGuid(bot); }
};

class TbcDungeonMechValueContext : public NamedObjectContext<UntypedValue>
{
public:
    TbcDungeonMechValueContext()
    {
        creators["sepethrea fixating flame"] =
            &TbcDungeonMechValueContext::sepethrea_fixating_flame;
    }

private:
    static UntypedValue* sepethrea_fixating_flame(PlayerbotAI* botAI)
    {
        return new SepethreaFixatingFlameValue(botAI);
    }
};

#endif

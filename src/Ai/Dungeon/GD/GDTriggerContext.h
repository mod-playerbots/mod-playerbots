#ifndef PLAYERBOTS_GDTRIGGERCONTEXT_H
#define PLAYERBOTS_GDTRIGGERCONTEXT_H

#include "NamedObjectContext.h"
#include "AiObjectContext.h"
#include "GDTriggers.h"

class WotlkDungeonGDTriggerContext : public NamedObjectContext<Trigger>
{
    public:
        WotlkDungeonGDTriggerContext()
        {
            creators["poison nova"] = &WotlkDungeonGDTriggerContext::poison_nova;
            creators["snake wrap"] = &WotlkDungeonGDTriggerContext::snake_wrap;
            creators["whirling slash"] = &WotlkDungeonGDTriggerContext::whirling_slash;
        }
    private:
        static Trigger* poison_nova(PlayerbotAI* ai) { return new SladranPoisonNovaTrigger(ai); }
        static Trigger* snake_wrap(PlayerbotAI* ai) { return new SladranSnakeWrapTrigger(ai); }
        static Trigger* whirling_slash(PlayerbotAI* ai) { return new GaldarahWhirlingSlashTrigger(ai); }
};

#endif

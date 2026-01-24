/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#include "PaladinTriggers.h"
#include "BotSpellService.h"

#include "PaladinActions.h"
#include "PlayerbotAIConfig.h"
#include "Playerbots.h"

bool SealTrigger::IsActive()
{
    Unit* target = GetTarget();
    return !botAI->GetServices().GetSpellService().HasAura("seal of justice", target) && !botAI->GetServices().GetSpellService().HasAura("seal of command", target) &&
           !botAI->GetServices().GetSpellService().HasAura("seal of vengeance", target) && !botAI->GetServices().GetSpellService().HasAura("seal of corruption", target) &&
           !botAI->GetServices().GetSpellService().HasAura("seal of righteousness", target) && !botAI->GetServices().GetSpellService().HasAura("seal of light", target) &&
           (!botAI->GetServices().GetSpellService().HasAura("seal of wisdom", target) || AI_VALUE2(uint8, "mana", "self target") > 70);
}

bool CrusaderAuraTrigger::IsActive()
{
    Unit* target = GetTarget();
    return AI_VALUE2(bool, "mounted", "self target") && !botAI->GetServices().GetSpellService().HasAura("crusader aura", target);
}

bool BlessingTrigger::IsActive()
{
    Unit* target = GetTarget();
    return SpellTrigger::IsActive() && !botAI->GetServices().GetSpellService().HasAnyAuraOf(target, "blessing of might", "blessing of wisdom",
                                                            "blessing of kings", "blessing of sanctuary", nullptr);
}

/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#include "PullStrategy.h"

#include "AiObjectContext.h"
#include "PassiveMultiplier.h"
#include "Player.h"
#include "PlayerbotAI.h"
#include "Playerbots.h"
#include "SpellMgr.h"

class PullStrategyActionNodeFactory : public NamedObjectFactory<ActionNode>
{
public:
    PullStrategyActionNodeFactory()
    {
        creators["pull start"] = &pull_start;
    }

private:
    static ActionNode* pull_start(PlayerbotAI* /*botAI*/)
    {
        return new ActionNode("pull start", {}, {}, { NextAction("pull action", ACTION_NORMAL) });
    }
};

PullStrategy::PullStrategy(PlayerbotAI* botAI, std::string const action, std::string const preAction)
    : Strategy(botAI), action(action), preAction(preAction)
{
    actionNodeFactories.Add(new PullStrategyActionNodeFactory());
}

PullStrategy* PullStrategy::Get(PlayerbotAI* botAI)
{
    if (!botAI)
        return nullptr;

    if (PullStrategy* strategy = dynamic_cast<PullStrategy*>(botAI->GetStrategy("pull", BOT_STATE_NON_COMBAT)))
    {
        if (strategy->IsPullPendingToStart() || strategy->HasPullStarted() || strategy->HasTarget())
            return strategy;
    }

    return dynamic_cast<PullStrategy*>(botAI->GetStrategy("pull", BOT_STATE_COMBAT));
}

Unit* PullStrategy::GetTarget() const
{
    ObjectGuid const guid = botAI->GetAiObjectContext()->GetValue<ObjectGuid>("pull target")->Get();
    if (guid.IsEmpty())
        return nullptr;

    Unit* target = botAI->GetUnit(guid);
    Player* bot = botAI->GetBot();
    if (!bot || !target || !target->IsAlive() || !target->IsInWorld() ||
        target->GetMapId() != bot->GetMapId())
        return nullptr;

    return target;
}

bool PullStrategy::HasTarget() const { return GetTarget() != nullptr; }

void PullStrategy::SetTarget(Unit* target)
{
    botAI->GetAiObjectContext()->GetValue<ObjectGuid>("pull target")->Set(target ? target->GetGUID() : ObjectGuid::Empty);
}

std::string PullStrategy::GetPullActionName() const
{
    return action;
}

std::string PullStrategy::GetSpellName() const
{
    Player* bot = botAI->GetBot();
    std::string spellName = GetPullActionName();
    if (!bot || spellName != "shoot")
        return spellName;

    Item* equippedWeapon = bot->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_RANGED);
    if (!equippedWeapon)
        return spellName;

    ItemTemplate const* itemTemplate = equippedWeapon->GetTemplate();
    if (!itemTemplate)
        return spellName;

    switch (itemTemplate->SubClass)
    {
        case ITEM_SUBCLASS_WEAPON_THROWN:
            return "throw";
        case ITEM_SUBCLASS_WEAPON_GUN:
            return "shoot gun";
        case ITEM_SUBCLASS_WEAPON_BOW:
            return "shoot bow";
        case ITEM_SUBCLASS_WEAPON_CROSSBOW:
            return "shoot crossbow";
        default:
            return spellName;
    }
}

float PullStrategy::GetRange() const
{
    Player* bot = botAI->GetBot();
    std::string const spellName = GetSpellName();
    if (bot && !spellName.empty())
    {
        uint32 const spellId = botAI->GetAiObjectContext()->GetValue<uint32>("spell id", spellName)->Get();
        if (SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(spellId))
            return bot->GetSpellMaxRangeForTarget(GetTarget(), spellInfo) - CONTACT_DISTANCE;
    }

    return (action == "shoot" ? botAI->GetRange("shoot") : botAI->GetRange("spell")) - CONTACT_DISTANCE;
}

std::string PullStrategy::GetPreActionName() const
{
    return preAction;
}

bool PullStrategy::CanDoPullAction(Unit* target)
{
    Player* bot = botAI->GetBot();
    if (!bot || !target)
        return false;

    if (!target->IsInWorld() || target->GetMapId() != bot->GetMapId())
        return false;

    if (bot->getClass() != CLASS_DRUID && bot->getClass() != CLASS_PALADIN &&
        GetPullActionName() == "shoot" && !bot->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_RANGED))
    {
        return false;
    }

    std::string const spellName = GetSpellName();
    if (spellName.empty())
        return false;

    return true;
}

void PullStrategy::RequestPull(Unit* target, bool resetTime)
{
    SetTarget(target);
    pendingToStart = true;
    if (resetTime)
        pullStartTime = time(nullptr);
}

void PullStrategy::OnPullStarted() { pendingToStart = false; }

void PullStrategy::OnPullEnded()
{
    pullStartTime = 0;
    pendingToStart = false;
    SetTarget(nullptr);
}

PullMultiplier::PullMultiplier(PlayerbotAI* botAI) : Multiplier(botAI, "pull") {}

float PullMultiplier::GetValue(Action* action)
{
    PullStrategy const* strategy = PullStrategy::Get(botAI);
    if (!strategy || !strategy->HasTarget() || !action)
        return 1.0f;

    if (!strategy->IsPullPendingToStart() && !strategy->HasPullStarted())
        return 1.0f;

    std::string const actionName = action->getName();
    if (actionName == "pull my target" ||
        actionName == "pull rti target" ||
        actionName == "reach pull" ||
        actionName == "pull start" ||
        actionName == "pull action" ||
        actionName == "return to pull position" ||
        actionName == "pull end" ||
        actionName == "follow" ||
        actionName == "set facing")
        return 1.0f;

    return 0.0f;
}

class MagePullMultiplier : public PassiveMultiplier
{
public:
    MagePullMultiplier(PlayerbotAI* botAI, std::string const action) : PassiveMultiplier(botAI), actionName(action) {}

    float GetValue(Action* action) override;

private:
    std::string const actionName;
};

float MagePullMultiplier::GetValue(Action* action)
{
    if (!action)
        return 1.0f;

    PullStrategy const* strategy = PullStrategy::Get(botAI);
    if (!strategy || !strategy->HasTarget())
        return 1.0f;

    std::string const name = action->getName();
    if (actionName == name || name == "pull action" || name == "pull start" || name == "pull end" ||
        name == "pull my target" || name == "pull rti target" ||
        name == "reach spell" || name == "reach pull" ||
        name == "return to pull position" || name == "follow" ||
        name == "set facing" || name == "change strategy")
        return 1.0f;

    return PassiveMultiplier::GetValue(action);
}

std::vector<NextAction> PullStrategy::getDefaultActions()
{
    return {
        NextAction("pull action", 105.0f),
    };
}

void PullStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    triggers.push_back(new TriggerNode(
        "pull start",
        {
            NextAction("pull start", 106.0f),
            NextAction("pull action", ACTION_MOVE)
        }
    ));

    triggers.push_back(new TriggerNode(
        "pull end",
        {
            NextAction("pull end", 107.0f)
        }
    ));
}

void PullStrategy::InitMultipliers(std::vector<Multiplier*>& multipliers)
{
    multipliers.push_back(new PullMultiplier(botAI));
    multipliers.push_back(new MagePullMultiplier(botAI, action));
}

void PossibleAddsStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    Strategy::InitTriggers(triggers);

    triggers.push_back(
        new TriggerNode(
            "possible adds",
            {
                NextAction("flee with pet", 60)
            }
        )
    );
}

void PullBackStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    Strategy::InitTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "return to pull position",
        {
            NextAction("return to pull position", ACTION_MOVE + 5.0f)
        }
    ));
}

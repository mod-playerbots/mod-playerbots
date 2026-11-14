/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

/*
 * Ported from the CMaNGOS playerbots project (https://github.com/cmangos/playerbots), GPL v2,
 * with modifications for AzerothCore.
 * Original authors:
 *   ike3 <ike@email.org> - original author
 *   Sebastiaan Keek (mostlikely4r) <sebastiaan.keek@gmail.com>
 *   David Parra Ausina (Flekz) <davidparraausina@gmail.com>
 */

#include "PullStrategy.h"
#include "AiObjectContext.h"
#include "ObjectMgr.h"
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
    ObjectGuid const guid = botAI->GetAiObjectContext()->GetValue<ObjectGuid>("pull strategy target")->Get();
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
    botAI->GetAiObjectContext()->GetValue<ObjectGuid>("pull strategy target")->Set(target ? target->GetGUID() : ObjectGuid::Empty);
}

std::string PullStrategy::GetPullActionName() const
{
    return action;
}

// Class strategies pick their pull for the stored target, and RequestPull() stores it only after the checks.
std::string PullStrategy::GetPullActionNameFor(Unit* target)
{
    Value<ObjectGuid>* const storedTarget = botAI->GetAiObjectContext()->GetValue<ObjectGuid>("pull strategy target");
    ObjectGuid const previousTarget = storedTarget->Get();
    SetTarget(target);
    std::string const actionName = GetPullActionName();
    storedTarget->Set(previousTarget);
    return actionName;
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
        // Bows, guns and crossbows all use Shoot (3018): no player spell is named "shoot gun" etc.
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

namespace
{
// "Requires No Ammo" (Thori'dal, the Stars' Fury): the core unloads ammo and lets the shot through.
constexpr uint32 SPELL_REQUIRES_NO_AMMO = 46699;

// The ammo checks of Spell::CheckItems: a bow, gun or crossbow shoots only with loaded ammo of its
// kind that the bot still holds, unless the bot has "Requires No Ammo".
bool HasAmmoFor(Player* bot, Item const* weapon)
{
    uint32 neededAmmo;
    switch (weapon->GetTemplate()->SubClass)
    {
        case ITEM_SUBCLASS_WEAPON_BOW:
        case ITEM_SUBCLASS_WEAPON_CROSSBOW:
            neededAmmo = ITEM_SUBCLASS_ARROW;
            break;
        case ITEM_SUBCLASS_WEAPON_GUN:
            neededAmmo = ITEM_SUBCLASS_BULLET;
            break;
        default:
            return true;  // thrown weapons and wands use no ammo
    }

    if (bot->HasAura(SPELL_REQUIRES_NO_AMMO))
        return true;

    uint32 const ammoId = bot->GetUInt32Value(PLAYER_AMMO_ID);
    ItemTemplate const* ammo = ammoId ? sObjectMgr->GetItemTemplate(ammoId) : nullptr;
    return ammo && ammo->Class == ITEM_CLASS_PROJECTILE && ammo->SubClass == neededAmmo &&
           bot->HasItemCount(ammoId, 1);
}
}

bool PullStrategy::CanDoPullAction(Unit* target)
{
    Player* bot = botAI->GetBot();
    if (!bot || !target)
        return false;

    if (!target->IsInWorld() || target->GetMapId() != bot->GetMapId())
        return false;

    std::string const actionName = GetPullActionNameFor(target);

    if (bot->getClass() != CLASS_DRUID && bot->getClass() != CLASS_PALADIN &&
        actionName == "shoot" && !bot->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_RANGED))
    {
        return false;
    }

    if (actionName == "shoot")
    {
        Item const* weapon = bot->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_RANGED);
        if (weapon && !HasAmmoFor(bot, weapon))
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

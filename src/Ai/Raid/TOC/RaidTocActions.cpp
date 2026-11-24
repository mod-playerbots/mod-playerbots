/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "RaidTocActions.h"
#include "AiFactory.h"
#include "Event.h"
#include "Playerbots.h"
#include "Position.h"
#include <cmath>

namespace
{
    // Twin Val'kyr encounter (Trial of the Crusader, 4th boss).
    constexpr uint32 NPC_LIGHTBANE = 34497;        // Fjola Lightbane
    constexpr uint32 NPC_DARKBANE = 34496;         // Eydis Darkbane
    constexpr uint32 NPC_LIGHT_ESSENCE_PORTAL = 34568;
    constexpr uint32 NPC_DARK_ESSENCE_PORTAL = 34567;

    constexpr uint32 SPELL_TOUCH_OF_LIGHT = 67297;     // cast by Lightbane on dark-essence players
    constexpr uint32 SPELL_TOUCH_OF_DARKNESS = 67282;  // cast by Darkbane on light-essence players

    constexpr uint32 SPELL_LIGHT_ESSENCE = 65686;
    constexpr uint32 SPELL_LIGHT_ESSENCE_2 = 65811;
    constexpr uint32 SPELL_DARK_ESSENCE = 65684;
    constexpr uint32 SPELL_DARK_ESSENCE_2 = 65827;
}

bool TocTwinValkyrTouchAction::HasLightTouch() const { return bot->HasAura(SPELL_TOUCH_OF_LIGHT); }

bool TocTwinValkyrTouchAction::HasDarkTouch() const { return bot->HasAura(SPELL_TOUCH_OF_DARKNESS); }

bool TocTwinValkyrTouchAction::HasLightEssence() const
{
    return bot->HasAura(SPELL_LIGHT_ESSENCE) || bot->HasAura(SPELL_LIGHT_ESSENCE_2);
}

bool TocTwinValkyrTouchAction::HasDarkEssence() const
{
    return bot->HasAura(SPELL_DARK_ESSENCE) || bot->HasAura(SPELL_DARK_ESSENCE_2);
}

Unit* TocTwinValkyrTouchAction::FindPortal(uint32 entry) const { return bot->FindNearestCreature(entry, 120.0f); }

bool TocTwinValkyrTouchAction::Execute(Event /*event*/)
{
    switch (phase)
    {
        case PHASE_IDLE:
            if (HasLightTouch())
            {
                // Touch of Light sits on dark essence -> cleanse on the light portal.
                originalSpot.Relocate(bot->GetPosition());
                cleansingPortalEntry = NPC_LIGHT_ESSENCE_PORTAL;
                originalPortalEntry = NPC_DARK_ESSENCE_PORTAL;
                phase = PHASE_TO_CLEANSING_PORTAL;
            }
            else if (HasDarkTouch())
            {
                // Touch of Darkness sits on light essence -> cleanse on the dark portal.
                originalSpot.Relocate(bot->GetPosition());
                cleansingPortalEntry = NPC_DARK_ESSENCE_PORTAL;
                originalPortalEntry = NPC_LIGHT_ESSENCE_PORTAL;
                phase = PHASE_TO_CLEANSING_PORTAL;
            }
            break;

        case PHASE_TO_CLEANSING_PORTAL:
            // Touching the opposite portal removes the debuff.
            if (!HasLightTouch() && !HasDarkTouch())
            {
                phase = PHASE_TO_ORIGINAL_PORTAL;
                break;
            }
            if (Unit* portal = FindPortal(cleansingPortalEntry))
                return MoveNear(portal, 2.0f);
            break;

        case PHASE_TO_ORIGINAL_PORTAL:
            // Take the original essence back.
            if ((originalPortalEntry == NPC_LIGHT_ESSENCE_PORTAL && HasLightEssence()) ||
                (originalPortalEntry == NPC_DARK_ESSENCE_PORTAL && HasDarkEssence()))
            {
                phase = PHASE_RETURN_TO_SPOT;
                break;
            }
            if (Unit* portal = FindPortal(originalPortalEntry))
                return MoveNear(portal, 2.0f);
            break;

        case PHASE_RETURN_TO_SPOT:
            if (bot->GetDistance(originalSpot.GetPositionX(), originalSpot.GetPositionY(),
                                 originalSpot.GetPositionZ()) > 3.0f)
                return MoveTo(bot->GetMapId(), originalSpot.GetPositionX(), originalSpot.GetPositionY(),
                              originalSpot.GetPositionZ());
            phase = PHASE_IDLE;
            break;
    }

    return false;
}

bool TocTwinValkyrTouchAction::isUseful()
{
    // Tanks never get a Touch and must stay planted or the twins turn with
    // them, so only non-tanks run the portal dance.
    if (PlayerbotAI::IsTank(bot))
        return false;

    if (phase != PHASE_IDLE)
        return true;

    return HasLightTouch() || HasDarkTouch();
}

bool TocOffhealAction::HealerHasTouch() const
{
    Group* group = bot->GetGroup();
    if (!group)
        return false;

    for (GroupReference* itr = group->GetFirstMember(); itr != nullptr; itr = itr->next())
    {
        Player* member = itr->GetSource();
        if (!member || !member->IsAlive())
            continue;

        if (!PlayerbotAI::IsHeal(member))
            continue;

        if (member->HasAura(SPELL_TOUCH_OF_LIGHT) || member->HasAura(SPELL_TOUCH_OF_DARKNESS))
            return true;
    }

    return false;
}

bool TocOffhealAction::GroupHasBetterOffhealer() const
{
    // Elemental shamans and balance druids heal as part of their normal dps
    // rotation. When one of them is around, ret paladins and cat druids should
    // not sacrifice their damage to off-heal - their priority is lower.
    Group* group = bot->GetGroup();
    if (!group)
        return false;

    for (GroupReference* itr = group->GetFirstMember(); itr != nullptr; itr = itr->next())
    {
        Player* member = itr->GetSource();
        if (!member || !member->IsAlive() || member == bot)
            continue;

        uint8 const tab = AiFactory::GetPlayerSpecTab(member);

        if (member->getClass() == CLASS_SHAMAN && tab == 0 /* elemental */)
            return true;

        if (member->getClass() == CLASS_DRUID && tab == 0 /* balance */)
            return true;
    }

    return false;
}

bool TocOffhealAction::Execute(Event /*event*/)
{
    // Only hybrids that actually own an offheal strategy can help: retribution
    // paladins and cat druids. Tanks and real healers are not touched.
    if (PlayerbotAI::IsTank(bot) || PlayerbotAI::IsHeal(bot))
        return false;

    if (bot->getClass() != CLASS_PALADIN && bot->getClass() != CLASS_DRUID)
        return false;

    // Elemental shaman / balance druid outrank us: if one is present, leave
    // the healing to them and keep doing damage.
    if (GroupHasBetterOffhealer())
        return false;

    bool const needOffheal = HealerHasTouch();
    bool const hasOffheal = botAI->HasStrategy("offheal", BOT_STATE_COMBAT);

    if (needOffheal && !hasOffheal)
    {
        // Switching the strategy makes the heal actions outrank the current
        // damage cast, which interrupts it - the hybrid starts healing right
        // away instead of finishing the spell.
        botAI->ChangeStrategy("+offheal", BOT_STATE_COMBAT);
    }
    else if (!needOffheal && hasOffheal)
    {
        botAI->ChangeStrategy("-offheal", BOT_STATE_COMBAT);
    }

    return true;
}

bool TocOffhealAction::isUseful()
{
    // Only during the Twin Val'kyr encounter (the strategy is gated by map).
    return bot->GetMapId() == 649 && !PlayerbotAI::IsTank(bot) && !PlayerbotAI::IsHeal(bot) &&
           (bot->getClass() == CLASS_PALADIN || bot->getClass() == CLASS_DRUID);
}

bool TocIcehowlDodgeAction::Execute(Event /*event*/)
{
    // Look the boss up by name: TOC uses different entries per difficulty.
    Unit* icehowl = AI_VALUE2(Unit*, "find target", "icehowl");
    if (!icehowl || !icehowl->IsAlive())
        return false;

    constexpr uint32 SPELL_FEROCIOUS_BUTT = 66770;

    if (bot->HasAura(SPELL_FEROCIOUS_BUTT))
    {
        // Picked: sidestep perpendicular to the boss direction so the charge
        // lane misses us. Pick one side consistently (always the bot's right).
        float const toBoss = bot->GetAngle(icehowl);
        float const sideAngle = toBoss + M_PI_2;
        float const dist = 14.0f;
        float const x = bot->GetPositionX() + dist * std::cos(sideAngle);
        float const y = bot->GetPositionY() + dist * std::sin(sideAngle);
        return MoveTo(bot->GetMapId(), x, y, bot->GetPositionZ());
    }

    // Not picked: clear the charge lane by moving away from the target.
    Player* picked = nullptr;
    float minDist = 9999.0f;
    if (Group* group = bot->GetGroup())
    {
        for (GroupReference* itr = group->GetFirstMember(); itr != nullptr; itr = itr->next())
        {
            Player* member = itr->GetSource();
            if (!member || !member->IsAlive() || !member->HasAura(SPELL_FEROCIOUS_BUTT))
                continue;

            float const dist = bot->GetDistance(member);
            if (dist < minDist)
            {
                minDist = dist;
                picked = member;
            }
        }
    }

    if (!picked)
        return false;

    // Stand at least 20 yards away from the picked player.
    if (minDist >= 20.0f)
        return false;

    return MoveAway(picked, 22.0f);
}

bool TocIcehowlDodgeAction::isUseful()
{
    if (bot->GetMapId() != 649)
        return false;

    constexpr uint32 SPELL_FEROCIOUS_BUTT = 66770;
    return bot->HasAura(SPELL_FEROCIOUS_BUTT);
}

bool TocAnubSubmergeAction::Execute(Event /*event*/)
{
    // Arena reference points from the boss script's AnubLocs (frost spheres).
    // "Down" is south (lower y), where the raid baits the spikes; the tank goes
    // the other way so the boss breaks the northern spheres.
    constexpr float CENTER_X = 752.0f;
    constexpr float CENTER_Y = 137.0f;
    constexpr float OFFSET = 32.0f;

    float targetX = CENTER_X;
    float targetY = CENTER_Y - OFFSET;  // raid default: south

    if (PlayerbotAI::IsTank(bot))
    {
        targetY = CENTER_Y + OFFSET;  // tank: north
    }
    else if (PlayerbotAI::IsHeal(bot))
    {
        // Holy paladin sticks with the tank so it does not die alone.
        if (Unit* tank = AI_VALUE(Unit*, "main tank"))
        {
            targetX = tank->GetPositionX();
            targetY = tank->GetPositionY();
        }
        else
        {
            targetY = CENTER_Y + OFFSET;
        }
    }

    if (bot->GetDistance(targetX, targetY, bot->GetPositionZ()) < 6.0f)
        return false;

    return MoveTo(bot->GetMapId(), targetX, targetY, bot->GetPositionZ());
}

bool TocAnubSubmergeAction::isUseful()
{
    if (bot->GetMapId() != 649)
        return false;

    Unit* anub = AI_VALUE2(Unit*, "find target", "anub'arak");
    if (!anub || !anub->IsAlive() || !anub->IsInCombat())
        return false;

    constexpr uint32 SPELL_SUBMERGE = 65981;
    return anub->HasAura(SPELL_SUBMERGE) || anub->HasUnitFlag(UNIT_FLAG_NOT_SELECTABLE);
}

bool TocJaraxxusAddsAction::HasInterruptMelee(Player* player)
{
    // Melee classes with a kick: their job is to keep Fel Fireball interrupted,
    // so they must not be pulled off the boss by a portal swap.
    if (!PlayerbotAI::IsMelee(player))
        return false;

    switch (player->getClass())
    {
        case CLASS_WARRIOR:      // Pummel / Shield Bash
        case CLASS_ROGUE:        // Kick
        case CLASS_DEATH_KNIGHT: // Mind Freeze
        case CLASS_PALADIN:      // Rebuke
            return true;
        default:
            return false;
    }
}

bool TocJaraxxusAddsAction::Execute(Event /*event*/)
{
    // Portal first: the Mistress of Pain it spawns heals the encounter.
    Unit* add = nullptr;
    for (uint32 entry : {34825 /* Nether Portal */, 34813 /* Infernal Volcano */})
    {
        if (Unit* found = bot->FindNearestCreature(entry, 120.0f))
        {
            if (found->IsAlive())
            {
                add = found;
                break;
            }
        }
    }

    if (!add)
        return false;

    // Skull it so the swap is coordinated.
    if (Group* group = bot->GetGroup())
    {
        ObjectGuid currentSkull = group->GetTargetIcon(7);
        if (currentSkull.IsEmpty() || currentSkull != add->GetGUID())
            group->SetTargetIcon(7, bot->GetGUID(), add->GetGUID());
    }

    return Attack(add);
}

bool TocJaraxxusAddsAction::isUseful()
{
    if (bot->GetMapId() != 649)
        return false;

    // Tanks hold the boss, of course.
    if (PlayerbotAI::IsTank(bot))
        return false;

    // Melee interrupt classes stay on the boss so the Fel Fireball keeps being
    // kicked instead of the whole group wiping to an uninterrupted cast.
    if (HasInterruptMelee(bot))
        return false;

    Unit* jaraxxus = AI_VALUE2(Unit*, "find target", "lord jaraxxus");
    if (!jaraxxus || !jaraxxus->IsAlive() || !jaraxxus->IsInCombat())
        return false;

    for (uint32 entry : {34825, 34813})
    {
        if (Unit* add = bot->FindNearestCreature(entry, 120.0f))
            if (add->IsAlive())
                return true;
    }

    return false;
}

//
// Jormungar pair: carry the Burning Bite over a poisoned player, then drag the
// boss away from the raid.
//

bool TocBurningBileAction::Execute(Event /*event*/)
{
    // Paralytic Toxin (any difficulty variant) marks the player that needs the
    // burning tank to walk over them.
    static constexpr uint32 PARALYTIC_TOXIN[] = {66823, 67618, 67619, 67620};
    static constexpr uint32 SPELL_BURNING_BITE = 66879;

    if (!bot->HasAura(SPELL_BURNING_BITE))
        return false;

    Group* group = bot->GetGroup();
    if (!group)
        return false;

    // 1) Somebody is paralysed: walk over them so the fire burns the toxin off.
    Player* poisoned = nullptr;
    for (GroupReference* itr = group->GetFirstMember(); itr != nullptr; itr = itr->next())
    {
        Player* member = itr->GetSource();
        if (!member || member == bot || !member->IsAlive())
            continue;

        for (uint32 auraId : PARALYTIC_TOXIN)
        {
            if (member->HasAura(auraId))
            {
                poisoned = member;
                break;
            }
        }

        if (poisoned)
            break;
    }

    if (poisoned)
        return MoveNear(poisoned, 2.0f);

    // 2) Nobody poisoned: keep the boss away from the raid so Burning Spray
    //    does not hit the group.
    Player* nearest = nullptr;
    float minDist = 99999.0f;
    for (GroupReference* itr = group->GetFirstMember(); itr != nullptr; itr = itr->next())
    {
        Player* member = itr->GetSource();
        if (!member || member == bot || !member->IsAlive())
            continue;

        float const dist = bot->GetDistance(member);
        if (dist < minDist)
        {
            minDist = dist;
            nearest = member;
        }
    }

    if (nearest && minDist < 25.0f)
        return MoveAway(nearest, 25.0f);

    return false;
}

bool TocBurningBileAction::isUseful()
{
    if (bot->GetMapId() != 649)
        return false;

    // Only the tank holding Dreadscale carries Burning Bite.
    if (!PlayerbotAI::IsTank(bot))
        return false;

    Unit* dreadscale = AI_VALUE2(Unit*, "find target", "dreadscale");
    if (!dreadscale || !dreadscale->IsAlive() || !dreadscale->IsInCombat())
        return false;

    static constexpr uint32 SPELL_BURNING_BITE = 66879;
    return bot->HasAura(SPELL_BURNING_BITE);
}

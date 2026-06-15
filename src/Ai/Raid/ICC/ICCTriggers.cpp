#include "ICCTriggers.h"
#include "ICCActions.h"
#include "NearestNpcsValue.h"
#include "PlayerbotAIConfig.h"
#include "ObjectAccessor.h"
#include "GenericTriggers.h"
#include "Playerbots.h"
#include "Trigger.h"
#include "GridNotifiers.h"
#include "Vehicle.h"
#include "ICCScripts.h"

//Lord Marrogwar
bool IccLmTrigger::IsActive()
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "lord marrowgar");
    if (!boss)
        return false;

    if (bot->HasAura(SPELL_EXPERIENCED))
        bot->RemoveAura(SPELL_EXPERIENCED);

    return true;
}

//Lady Deathwhisper
bool IccDarkReckoningTrigger::IsActive()
{
    if (bot->HasAura(SPELL_DARK_RECKONING))
        return true;

    return false;
}

bool IccLadyDeathwhisperTrigger::IsActive()
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "lady deathwhisper");
    if (!boss)
        return false;

    return true;
}

//Gunship Battle
bool IccRottingFrostGiantTankPositionTrigger::IsActive()
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "rotting frost giant");
    if (!boss)
        return false;

    if (bot->HasAura(SPELL_EXPERIENCED))
        bot->RemoveAura(SPELL_EXPERIENCED);

    return true;
}

bool IccInCannonTrigger::IsActive()
{
    Unit* vehicleBase = bot->GetVehicleBase();
    Vehicle* vehicle = bot->GetVehicle();
    if (!vehicleBase || !vehicle)
        return false;

    uint32 entry = vehicleBase->GetEntry();
    return entry == NPC_CANNONA || entry == NPC_CANNONH;
}

bool IccGunshipCannonNearTrigger::IsActive()
{
    if (bot->GetVehicle())
        return false;

    Unit* mount1 = bot->FindNearestCreature(NPC_CANNONA, 100.0f);
    Unit* mount2 = bot->FindNearestCreature(NPC_CANNONH, 100.0f);

    if (!mount1 && !mount2)
        return false;

    // If cannons have Below Zero aura, don't try to enter them
    Unit* friendlyCannon = nullptr;
    if (mount1 && mount1->IsFriendlyTo(bot))
        friendlyCannon = mount1;
    else if (mount2 && mount2->IsFriendlyTo(bot))
        friendlyCannon = mount2;

    if (friendlyCannon && friendlyCannon->HasAura(SPELL_BELOW_ZERO))
        return false;

    if (!botAI->IsDps(bot))
        return false;

    return true;
}

bool IccGunshipRocketJumpTrigger::IsActive()
{
    // The rocket jump mechanic is only needed when the gunship battle is active.
    // We detect which ship we are on by checking which enemy boss is present:
    //  - Saurfang hostile  => we are on the Alliance ship
    //  - Muradin hostile   => we are on the Horde ship
    // Using the hostile boss (not cannon friendliness) avoids conflicting with
    // the cannon-near trigger that fires on the same condition.
    Unit* saurfang = bot->FindNearestCreature(NPC_HIGH_OVERLORD_SAURFANG, 100.0f);
    if (saurfang && saurfang->IsAlive() && saurfang->IsHostileTo(bot))
        return true;

    Unit* muradin = bot->FindNearestCreature(NPC_MURADIN_BRONZEBEARD, 100.0f);
    if (muradin && muradin->IsAlive() && muradin->IsHostileTo(bot))
        return true;

    return false;
}

bool IccGunshipRocketPackSetupTrigger::IsActive()
{
    // Fires any time a bot is standing on a friendly gunship deck, regardless of
    // combat state. Lets bots walk to Zafod and equip the rocket pack before the
    // encounter starts (and keep it ready if they acquire it mid-fight).
    Unit* cannonA = bot->FindNearestCreature(NPC_CANNONA, 100.0f);
    if (cannonA && cannonA->IsFriendlyTo(bot))
        return true;

    Unit* cannonH = bot->FindNearestCreature(NPC_CANNONH, 100.0f);
    if (cannonH && cannonH->IsFriendlyTo(bot))
        return true;

    return false;
}

//DBS
bool IccDbsTrigger::IsActive()
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "deathbringer saurfang");
    if (!boss)
        return false;

    if (bot->HasAura(SPELL_EXPERIENCED))
        bot->RemoveAura(SPELL_EXPERIENCED);

    return true;
}

bool IccDbsMainTankRuneOfBloodTrigger::IsActive()
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "deathbringer saurfang");

    if (!boss)
        return false;

    if (!botAI->IsAssistTankOfIndex(bot, 0))
        return false;

    Unit* mt = AI_VALUE(Unit*, "main tank");
    if (!mt)
        return false;

    Aura* aura = botAI->GetAura("rune of blood", mt);
    if (!aura)
        return false;

    return true;
}

bool IccDogsTrigger::IsActive()
{
    if (AI_VALUE2(Unit*, "find target", "stinky") || AI_VALUE2(Unit*, "find target", "precious"))
        return true;

    return false;
}

//FESTERGUT
bool IccFestergutGroupPositionTrigger::IsActive()
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "festergut");
    if (!boss)
        return false;

    if (bot->HasAura(SPELL_EXPERIENCED))
        bot->RemoveAura(SPELL_EXPERIENCED);

    return true;
}

bool IccFestergutSporeTrigger::IsActive()
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "festergut");
    if (!boss || botAI->IsTank(bot))
        return false;

    // Check for spore aura (ID: 69279) on any bot in the group
    Group* group = bot->GetGroup();
    if (!group)
        return false;

    for (GroupReference* itr = group->GetFirstMember(); itr != nullptr; itr = itr->next())
    {
        Player* member = itr->GetSource();
        if (!member)
            continue;

        if (member->HasAura(SPELL_GAS_SPORE))
            return true;
    }

    return false;
}

bool IccFestergutAvoidMalleableGooTrigger::IsActive()
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "festergut");
    if (!boss)
        return false;

    // Tanks hold the boss at the fixed tank spot; goo can land on tanks but
    // moving would lose threat and let goo land on melee stack anyway.
    if (botAI->IsTank(bot))
        return false;

    // During spore phase, position switching handles goo avoidance — free-dodge
    // would pull bots out of their assigned spore spots.
    Group* group = bot->GetGroup();
    if (group)
    {
        for (GroupReference* itr = group->GetFirstMember(); itr != nullptr; itr = itr->next())
        {
            Player* member = itr->GetSource();
            if (member && member->HasAura(SPELL_GAS_SPORE))
                return false;
        }
    }

    constexpr uint32 impactLifetimeMs = 8000;
    constexpr float gooDangerRadius = 12.0f;

    uint32 now = getMSTime();
    float botX = bot->GetPositionX();
    float botY = bot->GetPositionY();
    ObjectGuid botGuid = bot->GetGUID();

    auto impactIt = IcecrownHelpers::malleableGooImpacts.find(bot->GetMap()->GetInstanceId());
    if (impactIt != IcecrownHelpers::malleableGooImpacts.end())
    {
        for (auto const& impact : impactIt->second)
        {
            if (getMSTimeDiff(impact.castTime, now) > impactLifetimeMs)
                continue;
            float dx = botX - impact.position.GetPositionX();
            float dy = botY - impact.position.GetPositionY();
            if (dx * dx + dy * dy < gooDangerRadius * gooDangerRadius)
            {
                // Lock bot into wait mode until this impact expires - prevents
                // group-position from yanking it back into the danger zone.
                uint32 waitUntil = impact.castTime + impactLifetimeMs;
                auto& slot = IcecrownHelpers::festergutGooWaitUntil[botGuid];
                if (waitUntil > slot)
                    slot = waitUntil;
                return true;
            }
        }
    }

    auto it = IcecrownHelpers::festergutGooWaitUntil.find(botGuid);
    if (it != IcecrownHelpers::festergutGooWaitUntil.end())
    {
        if (now < it->second)
            return true;
        IcecrownHelpers::festergutGooWaitUntil.erase(it);
    }

    return false;
}

//ROTFACE
bool IccRotfaceTankPositionTrigger::IsActive()
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "rotface");
    if (!boss || !(botAI->IsTank(bot) || botAI->IsMelee(bot)))
        return false;

    if (bot->HasAura(SPELL_EXPERIENCED))
        bot->RemoveAura(SPELL_EXPERIENCED);

    return true;
}

bool IccRotfaceGroupPositionTrigger::IsActive()
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "rotface");
    if (!boss)
        return false;

    return true;
}

bool IccRotfaceMoveAwayFromExplosionTrigger::IsActive()
{
    Creature* boss = bot->FindNearestCreature(NPC_BIG_OOZE, 100.0f);
    bool castingNow = boss && boss->IsAlive() &&
        boss->HasUnitState(UNIT_STATE_CASTING) && boss->FindCurrentSpellBySpellId(SPELL_UNSTABLE_OOZE_EXPLOSION);

    if (castingNow)
    {
        _wasCasting = true;
        _castEndTime = 0;
        return true;
    }

    // Cast just ended — record the time
    if (_wasCasting)
    {
        _wasCasting = false;
        if (_castEndTime == 0)
            _castEndTime = time(nullptr);
    }

    // Stay active for 6 seconds after cast ended (2s wait + return movement)
    if (_castEndTime > 0 && time(nullptr) - _castEndTime < 6)
        return true;

    _castEndTime = 0;
    return false;
}

bool IccRotfaceAvoidVileGasTrigger::IsActive()
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "rotface");
    if (!boss)
        return false;

    uint32 const now = getMSTime();

    auto vgIt = IcecrownHelpers::rotfaceVileGas.find(bot->GetMap()->GetInstanceId());
    bool const isVictim =
        vgIt != IcecrownHelpers::rotfaceVileGas.end() &&
        vgIt->second.victimGuid == bot->GetGUID() &&
        getMSTimeDiff(vgIt->second.castTime, now) < 8000;
    if (isVictim)
        return true;

    if (botAI->HasAura("Vile Gas", bot))
        return true;

    auto const& waitMap = IcecrownHelpers::rotfaceVileGasWaitUntil;
    auto it = waitMap.find(bot->GetGUID());
    if (it != waitMap.end() && now < it->second)
        return true;

    return false;
}

//PP

bool IccPutricideGrowingOozePuddleTrigger::IsActive()
{
    // Early return if boss doesn't exist
    Unit* boss = AI_VALUE2(Unit*, "find target", "professor putricide");
    if (!boss)
        return false;

    const GuidVector& npcs = AI_VALUE(GuidVector, "nearest hostile npcs");
    for (auto const& npc : npcs)
    {
        if (Unit* unit = botAI->GetUnit(npc))
        {
            const uint32 entry = unit->GetEntry();
            if (entry == NPC_GROWING_OOZE_PUDDLE)
                return true;
        }
    }

    return false;
}

bool IccPutricideVolatileOozeTrigger::IsActive()
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "volatile ooze");
    if (!boss)
        return false;

    if (botAI->HasAura("Gaseous Bloat", bot))
        return false;

    return true;
}

bool IccPutricideGasCloudTrigger::IsActive()
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "gas cloud");
    if (!boss)
        return false;

    Unit* boss1 = AI_VALUE2(Unit*, "find target", "volatile ooze");

    bool hasGaseousBloat = botAI->HasAura("Gaseous Bloat", bot);

    if (hasGaseousBloat && boss1)
        return true;

    if (boss1)
        return false;

    return true;
}

bool IccPutricideMutatedPlagueTrigger::IsActive()
{
    return AI_VALUE2(Unit*, "find target", "professor putricide") != nullptr;
}

bool IccPutricideMalleableGooTrigger::IsActive()
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "professor putricide");
    if (!boss)
        return false;

    Difficulty const diff = bot->GetRaidDifficulty();

    // Heroic cheat buffs — apply to all group members (bots + real players)
    if (boss && sPlayerbotAIConfig.EnableICCBuffs &&
        (diff == RAID_DIFFICULTY_10MAN_HEROIC || diff == RAID_DIFFICULTY_25MAN_HEROIC))
    {
        if (Group* buffGroup = bot->GetGroup())
        {
            for (GroupReference* itr = buffGroup->GetFirstMember(); itr; itr = itr->next())
            {
                Player* member = itr->GetSource();
                if (!member || !member->IsAlive() || !member->IsInWorld())
                    continue;

                if (!member->HasAura(SPELL_EXPERIENCED))
                    member->AddAura(SPELL_EXPERIENCED, member);

                if (!member->HasAura(SPELL_AGEIS_OF_DALARAN))
                    member->AddAura(SPELL_AGEIS_OF_DALARAN, member);

                if (!PlayerbotAI::IsTank(member) && !member->HasAura(SPELL_NO_THREAT))
                    member->AddAura(SPELL_NO_THREAT, member);

                if (PlayerbotAI::IsTank(member) && !member->HasAura(SPELL_SPITEFULL_FURY) &&
                    boss->GetVictim() != member)
                    member->AddAura(SPELL_SPITEFULL_FURY, member);
            }
        }
    }

    return true;
}

bool IccPutricideAbominationTrigger::IsActive()
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "professor putricide");
    if (!boss)
        return false;

    if (!botAI->IsAssistTank(bot))
        return false;

    // Already piloting - keep action firing until vehicle drops.
    if (Unit* veh = bot->GetVehicleBase())
    {
        uint32 e = veh->GetEntry();
        if (e == NPC_MUTATED_ABOMINATION_10 || e == NPC_MUTATED_ABOMINATION_25)
            return true;
    }

    // Phase 3: boss takes toy back. No transformation.
    if (boss->HealthBelowPct(35))
        return false;

    // Someone else already piloting - do not drink.
    if (Group* group = bot->GetGroup())
    {
        for (GroupReference* itr = group->GetFirstMember(); itr; itr = itr->next())
        {
            Player* m = itr->GetSource();
            if (!m || m == bot || !m->IsAlive())
                continue;
            if (Unit* vb = m->GetVehicleBase())
            {
                uint32 e = vb->GetEntry();
                if (e == NPC_MUTATED_ABOMINATION_10 || e == NPC_MUTATED_ABOMINATION_25)
                    return false;
            }
        }
    }

    // Require at least one Growing Ooze Puddle nearby.
    GuidVector const npcs = AI_VALUE(GuidVector, "nearest hostile npcs");
    for (auto const& g : npcs)
    {
        if (Unit* u = botAI->GetUnit(g))
            if (u->GetEntry() == NPC_GROWING_OOZE_PUDDLE)
                return true;
    }

    return false;
}

//BPC
bool IccBpcKelesethTankTrigger::IsActive()
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "prince keleseth");
    if (!boss)
        return false;

    if (bot->HasAura(SPELL_EXPERIENCED))
        bot->RemoveAura(SPELL_EXPERIENCED);

    if (!botAI->IsAssistTank(bot))
        return false;

    Aura* aura = botAI->GetAura("Shadow Prison", bot, false, true);
    if (aura)
        if (aura->GetStackAmount() > 18)
            return false;

    return true;
}

bool IccBpcMainTankTrigger::IsActive()
{
    if (!botAI->IsTank(bot))
        return false;

    Unit* valanar = AI_VALUE2(Unit*, "find target", "prince valanar");
    Unit* taldaram = AI_VALUE2(Unit*, "find target", "prince taldaram");
    Unit* keleseth = AI_VALUE2(Unit*, "find target", "prince keleseth");

    if (!(valanar || taldaram || keleseth))
        return false;

    if (bot->HasAura(SPELL_EXPERIENCED))
        bot->RemoveAura(SPELL_EXPERIENCED);

    return true;
}

bool IccBpcEmpoweredVortexTrigger::IsActive()
{
    // Tanks should ignore this mechanic
    if (botAI->IsMainTank(bot) || botAI->IsAssistTank(bot))
        return false;

    Unit* valanar = AI_VALUE2(Unit*, "find target", "prince valanar");
    if (!valanar)
        return false;

    Aura* aura = botAI->GetAura("Shadow Prison", bot, false, true);
    if (aura)
        if (aura->GetStackAmount() > 12)
            return false;

    Aura* auraValanar = botAI->GetAura("Invocation of Blood", valanar);
    if (!auraValanar)
        return false;

    return true;
}

bool IccBpcKineticBombTrigger::IsActive()
{
    Unit* valanar = AI_VALUE2(Unit*, "find target", "prince valanar");
    Unit* taldaram = AI_VALUE2(Unit*, "find target", "prince taldaram");
    Unit* keleseth = AI_VALUE2(Unit*, "find target", "prince keleseth");

    if (!(valanar || taldaram || keleseth))
        return false;

    if (!botAI->IsRanged(bot) || botAI->IsHeal(bot))
        return false;

    // Allow up to 18 stacks for bomb-assigned bots (multiplier handles assignment)
    if (Aura* aura = botAI->GetAura("Shadow Prison", bot, false, true))
    {
        if (aura->GetStackAmount() > 18)
            return false;
    }

    static const std::array<uint32, 4> bombEntries = {NPC_KINETIC_BOMB1, NPC_KINETIC_BOMB2, NPC_KINETIC_BOMB3,
                                                      NPC_KINETIC_BOMB4};
    const GuidVector bombs = AI_VALUE(GuidVector, "possible targets no los");

    bool bombFound = false;

    for (const auto entry : bombEntries)
    {
        for (auto const& guid : bombs)
        {
            if (Unit* unit = botAI->GetUnit(guid))
            {
                if (unit->GetEntry() == entry)
                {
                    // Check if bomb is within valid Z-axis range
                    if (unit->GetPositionZ() - bot->GetPositionZ() < 35.0f)
                    {
                        bombFound = true;
                        break;
                    }
                }
            }
        }
        if (bombFound)
            break;
    }

    return botAI->IsRangedDps(bot) && bombFound;
}

bool IccBpcBallOfFlameTrigger::IsActive()
{
    Unit* valanar = AI_VALUE2(Unit*, "find target", "prince valanar");
    Unit* taldaram = AI_VALUE2(Unit*, "find target", "prince taldaram");
    Unit* keleseth = AI_VALUE2(Unit*, "find target", "prince keleseth");

    if (!(valanar || taldaram || keleseth))
        return false;

    Aura* auraTaldaram = botAI->GetAura("Invocation of Blood", taldaram);
    if (!auraTaldaram)
        return false;

    return true;
}

// BQL
bool IccBqlGroupPositionTrigger::IsActive()
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "blood-queen lana'thel");
    if (!boss)
        return false;

    Unit* valanar = AI_VALUE2(Unit*, "find target", "prince valanar");
    Unit* taldaram = AI_VALUE2(Unit*, "find target", "prince taldaram");
    Unit* keleseth = AI_VALUE2(Unit*, "find target", "prince keleseth");

    if (valanar || taldaram || keleseth)
        return false;

    if (bot->HasAura(SPELL_EXPERIENCED))
        bot->RemoveAura(SPELL_EXPERIENCED);

    return true;
}

bool IccBqlPactOfDarkfallenTrigger::IsActive()
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "blood-queen lana'thel");
    if (!boss)
        return false;

    Unit* valanar = AI_VALUE2(Unit*, "find target", "prince valanar");
    Unit* taldaram = AI_VALUE2(Unit*, "find target", "prince taldaram");
    Unit* keleseth = AI_VALUE2(Unit*, "find target", "prince keleseth");

    if (valanar || taldaram || keleseth)
        return false;

    Aura* aura = botAI->GetAura("Pact of the Darkfallen", bot);
    if (!aura)
        return false;

    return true;
}

bool IccBqlVampiricBiteTrigger::IsActive()
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "blood-queen lana'thel");
    if (!boss)
        return false;

    Unit* valanar = AI_VALUE2(Unit*, "find target", "prince valanar");
    Unit* taldaram = AI_VALUE2(Unit*, "find target", "prince taldaram");
    Unit* keleseth = AI_VALUE2(Unit*, "find target", "prince keleseth");

    if (valanar || taldaram || keleseth)
        return false;

    Aura* aura = botAI->GetAura("Frenzied Bloodthirst", bot);
    if (!aura)
        return false;

    return true;
}

// Sister Svalna
bool IccValkyreSpearTrigger::IsActive()
{
    // Check if there's a spear nearby
    if (bot->FindNearestCreature(NPC_SPEAR, 100.0f))
        return true;

    return false;
}

bool IccSisterSvalnaTrigger::IsActive()
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "sister svalna");
    if (!boss)
        return false;

    if (bot->GetExactDist2d(boss) > 30.0f)
        return false;

    return true;
}

// VDW
bool IccValithriaGroupTrigger::IsActive()
{
    Unit* boss = bot->FindNearestCreature(NPC_VALITHRIA_DREAMWALKER, 100.0f);
    if (!boss)
        return false;

    if (bot->HasAura(SPELL_EXPERIENCED))
        bot->RemoveAura(SPELL_EXPERIENCED);

    return true;
}

bool IccValithriaZombieKiteTrigger::IsActive()
{
    Unit* boss = bot->FindNearestCreature(NPC_VALITHRIA_DREAMWALKER, 100.0f);
    if (!boss)
        return false;

    if (botAI->IsTank(bot))
        return false;

    std::list<Creature*> zombies;
    bot->GetCreatureListWithEntryInGrid(zombies, NPC_BLISTERING_ZOMBIE, 100.0f);
    for (Creature* z : zombies)
    {
        if (z && z->IsAlive() && z->GetVictim() == bot)
            return true;
    }

    return false;
}

bool IccValithriaPortalTrigger::IsActive()
{
    Unit* boss = bot->FindNearestCreature(NPC_VALITHRIA_DREAMWALKER, 100.0f);
    if (!boss)
        return false;

    Aura* aura = botAI->GetAura("Twisted Nightmares", bot, false, true);
    if (aura && aura->GetStackAmount() >= 25)
        return false;

    // Only healers should use portals
    if (!botAI->IsHeal(bot) || bot->HasAura(SPELL_DREAM_STATE))
        return false;

    Creature* worm = bot->FindNearestCreature(NPC_ROT_WORM, 100.0f);
    Creature* zombie = bot->FindNearestCreature(NPC_BLISTERING_ZOMBIE, 100.0f);

    if ((worm && worm->GetVictim() == bot) || (zombie && zombie->GetVictim() == bot))
        return false;

    Group* group = bot->GetGroup();
    if (!group)
        return false;

    // Collect healer GUIDs and check for druids
    std::vector<ObjectGuid> healerGuids;
    std::vector<ObjectGuid> druidGuids;
    int healerCount = 0;

    for (GroupReference* itr = group->GetFirstMember(); itr != nullptr; itr = itr->next())
    {
        Player* member = itr->GetSource();
        if (!member || !member->IsAlive() || botAI->IsRealPlayer())
            continue;

        if (botAI->IsHeal(member) && !botAI->IsRealPlayer())
        {
            healerCount++;
            healerGuids.push_back(member->GetGUID());
            // Check if druid (class 11)
            if (member->getClass() == CLASS_DRUID)
                druidGuids.push_back(member->GetGUID());
        }
    }

    // Sort GUIDs to ensure consistent ordering
    std::sort(healerGuids.begin(), healerGuids.end());
    std::sort(druidGuids.begin(), druidGuids.end());

    // Find position of current bot's GUID in the sorted list
    auto botGuidPos = std::find(healerGuids.begin(), healerGuids.end(), bot->GetGUID());
    if (botGuidPos == healerGuids.end())
        return false;

    int healerIndex = std::distance(healerGuids.begin(), botGuidPos);

    // Find if this bot is a druid
    bool isDruid = (bot->getClass() == CLASS_DRUID);

    // Determine raid healer assignment
    bool shouldHealRaid = false;
    int druidCount = druidGuids.size();

    if (druidCount > 0)
    {
        // If we have druids, they should heal raid
        if (isDruid)
        {
            // If there are more druids than raid healers needed, extra druids can heal boss
            int raidHealersNeeded = healerCount > 3 ? 2 : 1;
            int druidIndex =
                std::distance(druidGuids.begin(), std::find(druidGuids.begin(), druidGuids.end(), bot->GetGUID()));
            if (druidIndex < raidHealersNeeded)
                shouldHealRaid = true;
            else
                shouldHealRaid = false;
        }
        else if (healerCount > 3 && druidCount == 1)
        {
            // If only 1 druid and need 2 raid healers, pick the last non-druid healer as well
            if (healerIndex == (healerCount - 1) && !isDruid)
                shouldHealRaid = true;
        }
    }
    else
    {
        // No druids, assign raid healers as before
        if (healerCount > 3)
            shouldHealRaid = (healerIndex >= (healerCount - 2));
        else
            shouldHealRaid = (healerIndex == (healerCount - 1));
    }

    // Raid healers should not use portals
    if (shouldHealRaid)
        return false;

    // Find the nearest portal creature
    Creature* portal1 = bot->FindNearestCreature(NPC_DREAM_PORTAL, 100.0f);
    if (!portal1)
        portal1 = bot->FindNearestCreature(NPC_DREAM_PORTAL_PRE_EFFECT, 100.0f);

    Creature* portal2 = bot->FindNearestCreature(NPC_NIGHTMARE_PORTAL, 100.0f);
    if (!portal2)
        portal2 = bot->FindNearestCreature(NPC_NIGHTMARE_PORTAL_PRE_EFFECT, 100.0f);

    return portal1 || portal2;
}

bool IccValithriaHealTrigger::IsActive()
{
    Unit* boss = bot->FindNearestCreature(NPC_VALITHRIA_DREAMWALKER, 100.0f);
    if (!boss)
        return false;

    // Only healers should use healing
    if (!botAI->IsHeal(bot) || bot->HasAura(SPELL_DREAM_STATE) || bot->HealthBelowPct(50))
        return false;

    Creature* worm = bot->FindNearestCreature(NPC_ROT_WORM, 100.0f);
    Creature* zombie = bot->FindNearestCreature(NPC_BLISTERING_ZOMBIE, 100.0f);

    if ((worm && worm->GetVictim() == bot) || (zombie && zombie->GetVictim() == bot))
        return false;

    Group* group = bot->GetGroup();
    if (!group)
        return false;

    // Collect healer GUIDs and check for druids
    std::vector<ObjectGuid> healerGuids;
    std::vector<ObjectGuid> druidGuids;
    int healerCount = 0;

    for (GroupReference* itr = group->GetFirstMember(); itr != nullptr; itr = itr->next())
    {
        Player* member = itr->GetSource();
        if (!member || !member->IsAlive() || botAI->IsRealPlayer())
            continue;

        if (botAI->IsHeal(member) && !botAI->IsRealPlayer())
        {
            healerCount++;
            healerGuids.push_back(member->GetGUID());
            // Check if druid (class 11)
            if (member->getClass() == CLASS_DRUID)
                druidGuids.push_back(member->GetGUID());
        }
    }

    // Sort GUIDs to ensure consistent ordering
    std::sort(healerGuids.begin(), healerGuids.end());
    std::sort(druidGuids.begin(), druidGuids.end());

    // Find position of current bot's GUID in the sorted list
    auto botGuidPos = std::find(healerGuids.begin(), healerGuids.end(), bot->GetGUID());
    if (botGuidPos == healerGuids.end())
        return false;

    int healerIndex = std::distance(healerGuids.begin(), botGuidPos);

    // Find if this bot is a druid
    bool isDruid = (bot->getClass() == CLASS_DRUID);

    // Determine raid healer assignment
    bool shouldHealRaid = false;
    int druidCount = druidGuids.size();

    if (druidCount > 0)
    {
        // If we have druids, they should heal raid
        if (isDruid)
        {
            // If there are more druids than raid healers needed, extra druids can heal boss
            int raidHealersNeeded = healerCount > 3 ? 2 : 1;
            int druidIndex =
                std::distance(druidGuids.begin(), std::find(druidGuids.begin(), druidGuids.end(), bot->GetGUID()));
            if (druidIndex < raidHealersNeeded)
                shouldHealRaid = true;
            else
                shouldHealRaid = false;  // extra druids can heal boss
        }
        else if (healerCount > 3 && druidCount == 1)
        {
            // If only 1 druid and need 2 raid healers, pick the last non-druid healer as well
            if (healerIndex == (healerCount - 1) && !isDruid)
                shouldHealRaid = true;
        }
    }
    else
    {
        // No druids, assign raid healers as before
        if (healerCount > 3)
            shouldHealRaid = (healerIndex >= (healerCount - 2));
        else
            shouldHealRaid = (healerIndex == (healerCount - 1));
    }

    // If assigned to raid healing, return false to not heal Valithria
    if (shouldHealRaid)
        return false;

    if (bot->GetHealthPct() < 50.0f)
        return false;

    // For Valithria healers, check portal logic
    // If no portal is found within 100 yards, we should heal
    if (!bot->FindNearestCreature(NPC_DREAM_PORTAL, 100.0f) && !bot->FindNearestCreature(NPC_NIGHTMARE_PORTAL, 100.0f))
        return true;

    if (bot->FindNearestCreature(NPC_DREAM_PORTAL, 10.0f) || bot->FindNearestCreature(NPC_NIGHTMARE_PORTAL, 10.0f))
        return false;

    // If portal is far but within 100 yards, heal while moving to it
    return true;
}

bool IccValithriaDreamCloudTrigger::IsActive()
{
    if (!bot->HasAura(SPELL_DREAM_STATE) || bot->HealthBelowPct(50))
        return false;

    return true;
}

//SINDRAGOSA
bool IccSindragosaGroupPositionTrigger::IsActive()
{
    Unit* boss = bot->FindNearestCreature(NPC_SINDRAGOSA, 200.0f);  // sindra
    if (!boss)
        return false;

    Difficulty diff = bot->GetRaidDifficulty();

    if (sPlayerbotAIConfig.EnableICCBuffs && diff && (diff == RAID_DIFFICULTY_10MAN_HEROIC || diff == RAID_DIFFICULTY_25MAN_HEROIC))
    {
        //-------CHEAT-------
        // Apply to every alive group member so real players benefit too,
        if (Group* group = bot->GetGroup())
        {
            for (GroupReference* itr = group->GetFirstMember(); itr; itr = itr->next())
            {
                Player* member = itr->GetSource();
                if (!member || !member->IsAlive() || !member->IsInWorld())
                    continue;

                if (!member->HasAura(SPELL_EXPERIENCED))
                    member->AddAura(SPELL_EXPERIENCED, member);

                if (!member->HasAura(SPELL_AGEIS_OF_DALARAN))
                    member->AddAura(SPELL_AGEIS_OF_DALARAN, member);

                if (!botAI->IsTank(member) && !member->HasAura(SPELL_NO_THREAT))
                    member->AddAura(SPELL_NO_THREAT, member);

                if (botAI->IsMainTank(member) && boss->GetVictim() != member &&
                    !member->HasAura(SPELL_SPITEFULL_FURY))
                    member->AddAura(SPELL_SPITEFULL_FURY, member);
            }
        }
        //-------CHEAT-------
    }

    // Air phase: give all tanks nitro boosts so they can quickly reposition to tombs
    if (boss->IsInCombat() && botAI->IsTank(bot) &&
        boss->GetExactDist2d(ICC_SINDRAGOSA_FLYING_POSITION.GetPositionX(), ICC_SINDRAGOSA_FLYING_POSITION.GetPositionY()) < 50.0f)
    {
        if (!bot->HasAura(SPELL_NITRO_BOOSTS))
            bot->AddAura(SPELL_NITRO_BOOSTS, bot);
    }

    // Last phase: tanks must keep tanking, never run to a tomb spot. Strip
    // Frost Beacon so the tomb-positioning logic doesn't apply to them.
    if (botAI->IsTank(bot) && bot->HasAura(SPELL_FROST_BEACON) && boss->HealthBelowPct(35) &&
        boss->GetExactDist2d(ICC_SINDRAGOSA_FLYING_POSITION.GetPositionX(), ICC_SINDRAGOSA_FLYING_POSITION.GetPositionY()) >= 30.0f)
        bot->RemoveAura(SPELL_FROST_BEACON);

    if (!boss || bot->HasAura(SPELL_FROST_BEACON) || boss->GetExactDist2d(ICC_SINDRAGOSA_FLYING_POSITION.GetPositionX(), ICC_SINDRAGOSA_FLYING_POSITION.GetPositionY()) < 50.0f)
        return false;

    return true;
}

bool IccSindragosaFrostBeaconTrigger::IsActive()
{
    Unit* boss = bot->FindNearestCreature(NPC_SINDRAGOSA, 200.0f);
    if (!boss)
        return false;

    Group* group = bot->GetGroup();
    if (!group)
        return false;

    if (bot->HasAura(SPELL_FROST_BEACON))
        return true;

    Group::MemberSlotList const& groupSlot = group->GetMemberSlots();
    for (Group::member_citerator itr = groupSlot.begin(); itr != groupSlot.end(); ++itr)
    {
        Player* member = ObjectAccessor::FindPlayer(itr->guid);
        if (!member || !member->IsAlive())
            continue;

        if (member->HasAura(SPELL_FROST_BEACON))  // If any player has Frost Beacon, keep trigger active
            return true;
    }

    return false;
}

bool IccSindragosaHotTrigger::IsActive()
{
    if (!botAI->IsHeal(bot))
        return false;

    if (bot->HasAura(SPELL_FROST_BEACON))
        return false;

    Unit* boss = bot->FindNearestCreature(NPC_SINDRAGOSA, 200.0f);
    if (!boss)
        return false;

    Group* group = bot->GetGroup();
    if (!group)
        return false;

    for (GroupReference* itr = group->GetFirstMember(); itr != nullptr; itr = itr->next())
    {
        Player* member = itr->GetSource();
        if (member && member->IsAlive() && member->HasAura(SPELL_FROST_BEACON))
            return true;
    }

    return false;
}

bool IccSindragosaBlisteringColdTrigger::IsActive()
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "sindragosa");
    if (!boss)
        return false;

    if (botAI->IsMainTank(bot))
        return false;

    // Don't move if any bot in group has ice tomb
    Group* group = bot->GetGroup();
    if (!group)
        return false;

    float dist = bot->GetExactDist2d(boss->GetPositionX(), boss->GetPositionY());

    if (dist >= 33.0f)
        return false;

    bool isCasting = false;
    if (boss && boss->HasUnitState(UNIT_STATE_CASTING))
        isCasting = true;

    bool isBlisteringCold = false;
    if (boss && (boss->FindCurrentSpellBySpellId(SPELL_BLISTERING_COLD1) ||
                 boss->FindCurrentSpellBySpellId(SPELL_BLISTERING_COLD2) ||
                 boss->FindCurrentSpellBySpellId(SPELL_BLISTERING_COLD3) ||
                 boss->FindCurrentSpellBySpellId(SPELL_BLISTERING_COLD4)))
        isBlisteringCold = true;

    return isCasting && isBlisteringCold;
}

bool IccSindragosaUnchainedMagicTrigger::IsActive()
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "sindragosa");
    if (!boss)
        return false;

    Aura* aura = botAI->GetAura("unchained magic", bot, false, false);
    if (!aura)
        return false;

    bool isBlisteringCold = false;
    if (boss && (boss->FindCurrentSpellBySpellId(SPELL_BLISTERING_COLD1) ||
                 boss->FindCurrentSpellBySpellId(SPELL_BLISTERING_COLD2) ||
                 boss->FindCurrentSpellBySpellId(SPELL_BLISTERING_COLD3) ||
                 boss->FindCurrentSpellBySpellId(SPELL_BLISTERING_COLD4)))
        isBlisteringCold = true;

    if (boss && boss->HasUnitState(UNIT_STATE_CASTING) && isBlisteringCold)
        return false;

    return true;
}

bool IccSindragosaChilledToTheBoneTrigger::IsActive()
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "sindragosa");
    if (!boss)
        return false;

    Aura* aura = botAI->GetAura("Chilled to the Bone", bot, false, false);
    if (!aura)
        return false;

    bool isBlisteringCold = false;
    if (boss && (boss->FindCurrentSpellBySpellId(SPELL_BLISTERING_COLD1) ||
                 boss->FindCurrentSpellBySpellId(SPELL_BLISTERING_COLD2) ||
                 boss->FindCurrentSpellBySpellId(SPELL_BLISTERING_COLD3) ||
                 boss->FindCurrentSpellBySpellId(SPELL_BLISTERING_COLD4)))
        isBlisteringCold = true;

    if (boss && boss->HasUnitState(UNIT_STATE_CASTING) && isBlisteringCold)
        return false;

    return true;
}

bool IccSindragosaMysticBuffetTrigger::IsActive()
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "sindragosa");
    if (!boss)
        return false;

    if (boss->GetVictim() == bot)
        return false;

    Aura* aura = botAI->GetAura("mystic buffet", bot, false, true);
    if (!aura)
        return false;

    if (bot->HasAura(SPELL_FROST_BEACON))
        return false;

    // Blistering Cold takes priority over tomb-hiding in the last phase:
    // skip hiding so the bot can run to the safe spot instead.
    if (boss->HasUnitState(UNIT_STATE_CASTING) &&
        (boss->FindCurrentSpellBySpellId(SPELL_BLISTERING_COLD1) ||
         boss->FindCurrentSpellBySpellId(SPELL_BLISTERING_COLD2) ||
         boss->FindCurrentSpellBySpellId(SPELL_BLISTERING_COLD3) ||
         boss->FindCurrentSpellBySpellId(SPELL_BLISTERING_COLD4)))
        return false;

    if (aura->GetStackAmount() >= 1)
        return true;

    return false;
}

bool IccSindragosaMainTankMysticBuffetTrigger::IsActive()
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "sindragosa");
    if (!boss)
        return false;

    Aura* aura = botAI->GetAura("mystic buffet", bot, false, false);
    if (botAI->IsTank(bot) && aura) //main tank will delete mystic buffet until I find a better way to swap tanks, atm it is not great since while swapping they will wipe group 7/10 times.
        bot->RemoveAura(aura->GetId());

    if (botAI->IsTank(bot) && boss->GetVictim() == bot)
        return false;

    // Only for assist tank
    if (!botAI->IsAssistTankOfIndex(bot, 0))
        return false;

    // Don't swap if we have frost beacon
    if (bot->HasAura(SPELL_FROST_BEACON))   // Frost Beacon
        return false;

    Unit* mt = AI_VALUE(Unit*, "main tank");
    if (!mt)
        return false;

    // Check main tank stacks
    Aura* mtAura = botAI->GetAura("mystic buffet", mt, false, true);
    if (!mtAura || mtAura->GetStackAmount() < 6)
        return false;

    // Check our own stacks - don't taunt if we have too many
    Aura* selfAura = botAI->GetAura("mystic buffet", bot, false, true);
    if (selfAura && selfAura->GetStackAmount() > 6)
        return false;

    // Only taunt if we're in position
    float distToTankPos = bot->GetExactDist2d(ICC_SINDRAGOSA_TANK_POSITION);
    if (distToTankPos > 3.0f)
        return false;

    return true;
}

// TODO never triggers since mystic buffet is bypassed in action
bool IccSindragosaTankSwapPositionTrigger::IsActive()
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "sindragosa");
    if (!boss)
        return false;

    if (boss && boss->GetVictim() == bot)
        return false;

    // Only for assist tank
    if (!botAI->IsAssistTankOfIndex(bot, 0))
        return false;

    // Don't move to position if we have frost beacon
    if (bot->HasAura(SPELL_FROST_BEACON))
        return false;

    // Check our own stacks - don't try to tank if we have too many
    Aura* selfAura = botAI->GetAura("mystic buffet", bot, false, true);
    if (selfAura && selfAura->GetStackAmount() > 6)
        return false;

    // Check if main tank has high stacks
    Unit* mt = AI_VALUE(Unit*, "main tank");
    if (!mt)
        return false;

    Aura* mtAura = botAI->GetAura("mystic buffet", mt, false, true);
    if (!mtAura)
        return false;

    uint32 mtStacks = mtAura->GetStackAmount();
    if (mtStacks < 6)  // Only start moving when MT has 5+ stacks
        return false;

    // Check if we're already in position
    float distToTankPos = bot->GetExactDist2d(ICC_SINDRAGOSA_TANK_POSITION);
    if (distToTankPos <= 3.0f)
        return false;

    return true;  // Move to position if all conditions are met
}

bool IccSindragosaFrostBombTrigger::IsActive()
{
    Unit* boss = bot->FindNearestCreature(NPC_SINDRAGOSA, 200.0f);
    if (!boss)
        return false;

    if (!bot->IsAlive())  // Skip if dead
        return false;

    // Tombed bots intentionally pass through: the action pins their group to
    // the current tomb's zone so when freed they don't migrate to the wrong
    // zone. The action returns false for tombed bots without moving them.
    if (boss->GetExactDist2d(ICC_SINDRAGOSA_FLYING_POSITION.GetPositionX(), ICC_SINDRAGOSA_FLYING_POSITION.GetPositionY()) < 50.0f && !boss->HealthBelowPct(25) && !boss->HealthAbovePct(99))
        return true;

    return false;
}

//LK

bool IccLichKingShadowTrapTrigger::IsActive()
{
    Unit* vdw = bot->FindNearestCreature(NPC_VALITHRIA_DREAMWALKER, 100.0f);
    if (vdw)
        return false;

    Unit* boss = AI_VALUE2(Unit*, "find target", "the lich king");
    if (!boss)
        return false;

    bool hasPlague = botAI->HasAura("Necrotic Plague", bot);
    if (hasPlague)
        return false;

    if (!botAI->IsMainTank(bot))
        return false;

    if (boss->HealthBelowPct(65))
        return false;

    // search for all nearby traps
    GuidVector npcs = AI_VALUE(GuidVector, "nearest hostile npcs");
    std::vector<Unit*> nearbyTraps;

    for (auto& npc : npcs)
    {
        Unit* unit = botAI->GetUnit(npc);
        if (!unit || !unit->IsAlive())
            continue;

        if (unit->GetEntry() == NPC_SHADOW_TRAP && bot->GetExactDist2d(unit) < 11.0f)  // shadow trap
            return true;
    }

    return false;
}

bool IccLichKingNecroticPlagueTrigger::IsActive()
{
    Unit* vdw = bot->FindNearestCreature(NPC_VALITHRIA_DREAMWALKER, 100.0f);
    if (vdw)
        return false;

    if (!AI_VALUE2(Unit*, "find target", "the lich king"))
        return false;

    return botAI->HasAura("Necrotic Plague", bot);
}

bool IccLichKingWinterTrigger::IsActive()
{
    Unit* vdw = bot->FindNearestCreature(NPC_VALITHRIA_DREAMWALKER, 100.0f);
    if (vdw)
        return false;

    Unit* boss = AI_VALUE2(Unit*, "find target", "the lich king");
    if (!boss)
        return false;

    auto const hasWinterAura = [&]() -> bool
    {
        return boss->HasAura(SPELL_REMORSELESS_WINTER1) || boss->HasAura(SPELL_REMORSELESS_WINTER2) ||
               boss->HasAura(SPELL_REMORSELESS_WINTER3) || boss->HasAura(SPELL_REMORSELESS_WINTER4) ||
               boss->HasAura(SPELL_REMORSELESS_WINTER5) || boss->HasAura(SPELL_REMORSELESS_WINTER6) ||
               boss->HasAura(SPELL_REMORSELESS_WINTER7) || boss->HasAura(SPELL_REMORSELESS_WINTER8);
    };

    auto const isCastingWinter = [&]() -> bool
    {
        if (!boss->HasUnitState(UNIT_STATE_CASTING))
            return false;

        return boss->FindCurrentSpellBySpellId(SPELL_REMORSELESS_WINTER1) ||
               boss->FindCurrentSpellBySpellId(SPELL_REMORSELESS_WINTER2) ||
               boss->FindCurrentSpellBySpellId(SPELL_REMORSELESS_WINTER3) ||
               boss->FindCurrentSpellBySpellId(SPELL_REMORSELESS_WINTER4) ||
               boss->FindCurrentSpellBySpellId(SPELL_REMORSELESS_WINTER5) ||
               boss->FindCurrentSpellBySpellId(SPELL_REMORSELESS_WINTER6) ||
               boss->FindCurrentSpellBySpellId(SPELL_REMORSELESS_WINTER7) ||
               boss->FindCurrentSpellBySpellId(SPELL_REMORSELESS_WINTER8);
    };

    return hasWinterAura() || isCastingWinter();
}

bool IccLichKingAddsTrigger::IsActive()
{
    Unit* vdw = bot->FindNearestCreature(NPC_VALITHRIA_DREAMWALKER, 100.0f);
    if (vdw)
        return false;

    if (bot->HasAura(SPELL_HARVEST_SOUL_VALKYR))
        return false;

    if (botAI->HasAura("Necrotic Plague", bot))
        return false;

    if (bot->FindNearestCreature(NPC_TERENAS_MENETHIL_HC, 55.0f) ||
        bot->FindNearestCreature(NPC_TERENAS_MENETHIL, 55.0f))
        return true;

    Unit* lk = AI_VALUE2(Unit*, "find target", "the lich king");
    if (!lk)
        return false;

    return true;
}

bool IccLichKingSpiritBombTrigger::IsActive()
{
    return IccLichKingSpiritBombAction::IsBombThreatActive(botAI, bot);
}

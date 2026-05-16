/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#include "RaidBlackTempleTriggers.h"

#include "AiFactory.h"
#include "Playerbots.h"
#include "RaidBlackTempleActions.h"
#include "RaidBlackTempleHelpers.h"
#include "RaidBossHelpers.h"
#include "SharedDefines.h"

using namespace BlackTempleHelpers;

// General

bool BlackTempleBotIsNotInCombatTrigger::IsActive()
{
    return !bot->IsInCombat() && bot->GetMapId() == BLACK_TEMPLE_MAP_ID;
}

// High Warlord Naj'entus

bool HighWarlordNajentusPullingBossTrigger::IsActive()
{
    if (bot->getClass() != CLASS_HUNTER)
        return false;

    Unit* najentus = AI_VALUE2(Unit*, "find target", "high warlord naj'entus");
    return najentus && najentus->GetHealthPct() > 95.0f;
}

bool HighWarlordNajentusBossEngagedByTanksTrigger::IsActive()
{
    return botAI->IsTank(bot) &&
           AI_VALUE2(Unit*, "find target", "high warlord naj'entus");
}

bool HighWarlordNajentusCastsNeedleSpinesTrigger::IsActive()
{
    return botAI->IsRanged(bot) &&
           AI_VALUE2(Unit*, "find target", "high warlord naj'entus");
}

bool HighWarlordNajentusPlayerIsImpaledTrigger::IsActive()
{
    if (botAI->IsTank(bot))
        return false;

    if (!AI_VALUE2(Unit*, "find target", "high warlord naj'entus"))
        return false;

    Group* group = bot->GetGroup();
    if (!group)
        return false;

    Player* impaledPlayer = nullptr;

    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (!member || member == bot)
            continue;

        if (member->HasAura(
                static_cast<uint32>(BlackTempleSpells::SPELL_IMPALING_SPINE)))
        {
            impaledPlayer = member;
            break;
        }
    }

    Player* closestBot = nullptr;
    float closestDist = std::numeric_limits<float>::max();

    if (impaledPlayer)
    {
        for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
        {
            Player* member = ref->GetSource();
            if (!member || !member->IsAlive() || member == impaledPlayer ||
                !GET_PLAYERBOT_AI(member) || botAI->IsTank(member))
            {
                continue;
            }

            float dist = member->GetDistance(impaledPlayer);
            if (dist < closestDist)
            {
                closestDist = dist;
                closestBot = member;
            }
        }
    }

    return closestBot == bot;
}

bool HighWarlordNajentusBossHasTidalShieldTrigger::IsActive()
{
    Unit* najentus = AI_VALUE2(Unit*, "find target", "high warlord naj'entus");
    if (!najentus || !najentus->HasAura(
            static_cast<uint32>(BlackTempleSpells::SPELL_TIDAL_SHIELD)))
    {
        return false;
    }

    return botAI->HasItemInInventory(
        static_cast<uint32>(BlackTempleItems::ITEM_NAJENTUS_SPINE));
}

// Supremus

bool SupremusPullingBossOrChangingPhaseTrigger::IsActive()
{
    if (bot->getClass() != CLASS_HUNTER)
        return false;

    Unit* supremus = AI_VALUE2(Unit*, "find target", "supremus");
    if (!supremus)
        return false;

    auto it = supremusPhaseTimer.find(supremus->GetMap()->GetInstanceId());
    if (it == supremusPhaseTimer.end())
        return false;

    const time_t now = time(nullptr);
    const time_t elapsed = now - it->second;

    // Active during first 10 seconds, or during 60-70, 120-130, etc.
    return (elapsed < 10) || ((elapsed % 60) < 10 && elapsed >= 60);
}

bool SupremusBossEngagedByRangedTrigger::IsActive()
{
    if (!botAI->IsRanged(bot))
        return false;

    Unit* supremus = AI_VALUE2(Unit*, "find target", "supremus");
    return supremus && !supremus->HasAura(
               static_cast<uint32>(BlackTempleSpells::SPELL_SNARE_SELF));
}

bool SupremusBossIsFixatedOnBotTrigger::IsActive()
{
    Unit* supremus = AI_VALUE2(Unit*, "find target", "supremus");
    return supremus && supremus->GetVictim() == bot &&
           supremus->HasAura(static_cast<uint32>(
               BlackTempleSpells::SPELL_SNARE_SELF));
}

bool SupremusVolcanoIsNearbyTrigger::IsActive()
{
    return AI_VALUE2(Unit*, "find target", "supremus") &&
           HasSupremusVolcanoNearby(botAI, bot);
}

bool SupremusNeedToManagePhaseTimerTrigger::IsActive()
{
    if (!botAI->IsDps(bot))
        return false;

    if (!AI_VALUE2(Unit*, "find target", "supremus"))
        return false;

    return IsMechanicTrackerBot(botAI, bot, BLACK_TEMPLE_MAP_ID);
}

// Shade of Akama

bool ShadeOfAkamaKillingChannelersStartsPhase2Trigger::IsActive()
{
    if (!botAI->IsDps(bot) || !botAI->IsMelee(bot))
        return false;

    constexpr float searchRadius = 30.0f;
    Unit* channeler = bot->FindNearestCreature(
        static_cast<uint32>(BlackTempleNpcs::NPC_ASHTONGUE_CHANNELER),
        searchRadius, true);

    return channeler && !channeler->HasUnitFlag(UNIT_FLAG_NOT_SELECTABLE);
}

// Teron Gorefiend

bool TeronGorefiendPullingBossTrigger::IsActive()
{
    if (bot->getClass() != CLASS_HUNTER)
        return false;

    Unit* gorefiend =
        AI_VALUE2(Unit*, "find target", "teron gorefiend");

    return gorefiend && gorefiend->GetHealthPct() > 95.0f;
}

bool TeronGorefiendBossEngagedByTanksTrigger::IsActive()
{
    return botAI->IsTank(bot) &&
           AI_VALUE2(Unit*, "find target", "teron gorefiend");
}

bool TeronGorefiendBossEngagedByRangedTrigger::IsActive()
{
    return botAI->IsRanged(bot) &&
           AI_VALUE2(Unit*, "find target", "teron gorefiend");
}

bool TeronGorefiendBossIsCastingShadowOfDeathTrigger::IsActive()
{
    if (bot->getClass() != CLASS_HUNTER && bot->getClass() != CLASS_MAGE &&
        bot->getClass() != CLASS_PALADIN && bot->getClass() != CLASS_ROGUE)
    {
        return false;
    }

    Unit* gorefiend = AI_VALUE2(Unit*, "find target", "teron gorefiend");
    if (!gorefiend)
        return false;

    if (botAI->HasAura("feign death", bot))
    {
        botAI->RemoveAura("feign death");
        return true;
    }
    else if (botAI->HasAura("ice block", bot))
    {
        botAI->RemoveAura("ice block");
        return true;
    }
    else if (!botAI->IsHeal(bot) && botAI->HasAura("divine shield", bot))
    {
        botAI->RemoveAura("divine shield");
        return true;
    }

    if (!gorefiend->HasUnitState(UNIT_STATE_CASTING))
        return false;

    Spell* spell = gorefiend->GetCurrentSpell(CURRENT_GENERIC_SPELL);
    if (!spell || spell->m_spellInfo->Id !=
            static_cast<uint32>(BlackTempleSpells::SPELL_SHADOW_OF_DEATH))
    {
        return false;
    }

    Unit* target = spell->m_targets.GetUnitTarget();
    return target && target->GetGUID() == bot->GetGUID();
}

bool TeronGorefiendBotHasShadowOfDeathTrigger::IsActive()
{
    Aura* aura = bot->GetAura(
        static_cast<uint32>(BlackTempleSpells::SPELL_SHADOW_OF_DEATH));
    return aura && aura->GetDuration() < 12000;
}

bool TeronGorefiendBotTransformedIntoVengefulSpiritTrigger::IsActive()
{
    return bot->HasAura(
        static_cast<uint32>(BlackTempleSpells::SPELL_SPIRITUAL_VENGEANCE));
}

// Gurtogg Bloodboil

bool GurtoggBloodboilPullingBossTrigger::IsActive()
{
    if (bot->getClass() != CLASS_HUNTER)
        return false;

    Unit* gurtogg = AI_VALUE2(Unit*, "find target", "gurtogg bloodboil");
    if (!gurtogg)
        return false;

    auto it = gurtoggPhaseTimer.find(gurtogg->GetMap()->GetInstanceId());
    if (it == gurtoggPhaseTimer.end())
        return false;

    const time_t elapsed = std::time(nullptr) - it->second;
    return elapsed < 10;
}

bool GurtoggBloodboilBossEngagedByTanksTrigger::IsActive()
{
    if (!botAI->IsTank(bot))
        return false;

    Unit* gurtogg = AI_VALUE2(Unit*, "find target", "gurtogg bloodboil");
    return gurtogg && !gurtogg->HasAura(
        static_cast<uint32>(BlackTempleSpells::SPELL_BOSS_FEL_RAGE));
}

bool GurtoggBloodboilBossCastsBloodboilTrigger::IsActive()
{
    if (!botAI->IsRanged(bot))
        return false;

    Unit* gurtogg = AI_VALUE2(Unit*, "find target", "gurtogg bloodboil");
    return gurtogg && !gurtogg->HasAura(
        static_cast<uint32>(BlackTempleSpells::SPELL_BOSS_FEL_RAGE));
}

bool GurtoggBloodboilBotHasFelRageTrigger::IsActive()
{
    if (!botAI->IsRanged(bot))
        return false;

    Unit* gurtogg = AI_VALUE2(Unit*, "find target", "gurtogg bloodboil");
    if (!gurtogg || !gurtogg->HasAura(
            static_cast<uint32>(BlackTempleSpells::SPELL_BOSS_FEL_RAGE)))
    {
        return false;
    }

    if (Group* group = bot->GetGroup())
    {
        for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
        {
            Player* member = ref->GetSource();
            if (member && member->HasAura
                    (static_cast<uint32>(BlackTempleSpells::SPELL_PLAYER_FEL_RAGE)))
            {
                return true;
            }
        }
    }

    return false;
}

bool GurtoggBloodboilNeedToManagePhaseTimerTrigger::IsActive()
{
    return AI_VALUE2(Unit*, "find target", "gurtogg bloodboil") &&
           IsMechanicTrackerBot(botAI, bot, BLACK_TEMPLE_MAP_ID);
}

// Reliquary of Souls

bool ReliquaryOfSoulsAggroResetsUponPhaseChangeTrigger::IsActive()
{
    return bot->getClass() == CLASS_HUNTER &&
           AI_VALUE2(Unit*, "find target", "reliquary of the lost");
}

bool ReliquaryOfSoulsEssenceOfSufferingFixatesOnClosestTargetTrigger::IsActive()
{
    return AI_VALUE2(Unit*, "find target", "essence of suffering");
}

bool ReliquaryOfSoulsEssenceOfSufferingDisablesHealingTrigger::IsActive()
{
    if (!botAI->IsHeal(bot))
        return false;

    if (bot->getClass() == CLASS_PRIEST &&
        AiFactory::GetPlayerSpecTab(bot) == PRIEST_TAB_DISCIPLINE)
    {
        return false;
    }

    return AI_VALUE2(Unit*, "find target", "essence of suffering");
}

bool ReliquaryOfSoulsEssenceOfDesireHasRuneShieldTrigger::IsActive()
{
    if (bot->getClass() != CLASS_MAGE)
        return false;

    Unit* desire = AI_VALUE2(Unit*, "find target", "essence of desire");
    return desire && desire->HasAura(
        static_cast<uint32>(BlackTempleSpells::SPELL_RUNE_SHIELD));
}

bool ReliquaryOfSoulsEssenceOfDesireCastingDeadenTrigger::IsActive()
{
    if (!botAI->IsTank(bot) || bot->getClass() != CLASS_WARRIOR)
        return false;

    Unit* desire = AI_VALUE2(Unit*, "find target", "essence of desire");
    if (!desire || !desire->HasUnitState(UNIT_STATE_CASTING))
        return false;

    Spell* spell = desire->GetCurrentSpell(CURRENT_GENERIC_SPELL);
    if (!spell || spell->m_spellInfo->Id !=
            static_cast<uint32>(BlackTempleSpells::SPELL_DEADEN))
    {
        return false;
    }

    Unit* target = spell->m_targets.GetUnitTarget();
    return target && target->GetGUID() == bot->GetGUID();
}

// Mother Shahraz

bool MotherShahrazPullingBossTrigger::IsActive()
{
    if (bot->getClass() != CLASS_HUNTER)
        return false;

    Unit* shahraz = AI_VALUE2(Unit*, "find target", "mother shahraz");
    return shahraz && shahraz->GetHealthPct() > 95.0f;
}

bool MotherShahrazBossEngagedByTanksTrigger::IsActive()
{
    if (!botAI->IsTank(bot))
        return false;

    if (!AI_VALUE2(Unit*, "find target", "mother shahraz"))
        return false;

    return !bot->HasAura(
        static_cast<uint32>(BlackTempleSpells::SPELL_FATAL_ATTRACTION));
}

bool MotherShahrazTanksArePositioningBossTrigger::IsActive()
{
    if (!botAI->IsMelee(bot) || !botAI->IsDps(bot))
        return false;

    Unit* shahraz = AI_VALUE2(Unit*, "find target", "mother shahraz");
    if (!shahraz || shahraz->GetHealthPct() < 90.0f)
        return false;

    TankPositionState tankState = GetShahrazTankPositionState(botAI, bot);
    return tankState != TankPositionState::Positioned;
}

bool MotherShahrazSinisterBeamKnocksBackPlayersTrigger::IsActive()
{
    if (!botAI->IsRanged(bot))
        return false;

    if (!AI_VALUE2(Unit*, "find target", "mother shahraz"))
        return false;

    return !bot->HasAura(
        static_cast<uint32>(BlackTempleSpells::SPELL_FATAL_ATTRACTION));
}

bool MotherShahrazBotsAreLinkedByFatalAttractionTrigger::IsActive()
{
    return bot->HasAura(
        static_cast<uint32>(BlackTempleSpells::SPELL_FATAL_ATTRACTION));
}

// Illidari Council

bool IllidariCouncilPullingBossesTrigger::IsActive()
{
    if (bot->getClass() != CLASS_HUNTER)
        return false;

    Unit* gathios = AI_VALUE2(Unit*, "find target", "gathios the shatterer");
    return gathios && gathios->GetHealthPct() > 95.0f;
}

bool IllidariCouncilGathiosEngagedByMainTankTrigger::IsActive()
{
    return botAI->IsMainTank(bot) &&
           AI_VALUE2(Unit*, "find target", "gathios the shatterer");
}

bool IllidariCouncilGathiosCastingJudgementOfCommandTrigger::IsActive()
{
    if (bot->getClass() != CLASS_WARRIOR || !botAI->IsMainTank(bot))
        return false;

    Unit* gathios = AI_VALUE2(Unit*, "find target", "gathios the shatterer");
    if (!gathios || !gathios->HasUnitState(UNIT_STATE_CASTING) || !gathios->HasAura(
            static_cast<uint32>(BlackTempleSpells::SPELL_SEAL_OF_COMMAND)))
    {
        return false;
    }

    Spell* spell = gathios->GetCurrentSpell(CURRENT_GENERIC_SPELL);
    if (!spell || spell->m_spellInfo->Id !=
            static_cast<uint32>(BlackTempleSpells::SPELL_JUDGEMENT))
    {
        return false;
    }

    Unit* target = spell->m_targets.GetUnitTarget();
    return target && target->GetGUID() == bot->GetGUID();
}

bool IllidariCouncilMalandeEngagedByFirstAssistTankTrigger::IsActive()
{
    return botAI->IsAssistTankOfIndex(bot, 0, false) &&
           AI_VALUE2(Unit*, "find target", "lady malande");
}

bool IllidariCouncilDarkshadowEngagedBySecondAssistTankTrigger::IsActive()
{
    if (!botAI->IsAssistTankOfIndex(bot, 1, false))
        return false;

    Unit* darkshadow = AI_VALUE2(Unit*, "find target", "veras darkshadow");
    return darkshadow && !darkshadow->HasAura(
        static_cast<uint32>(BlackTempleSpells::SPELL_VANISH));
}

bool IllidariCouncilZerevorEngagedByMageTankTrigger::IsActive()
{
    if (bot->getClass() != CLASS_MAGE || GetZerevorMageTank(bot) != bot)
        return false;

    return AI_VALUE2(Unit*, "find target", "high nethermancer zerevor");
}

bool IllidariCouncilMageTankNeedsDedicatedHealerTrigger::IsActive()
{
    return botAI->IsAssistHealOfIndex(bot, 0, true) &&
           AI_VALUE2(Unit*, "find target", "high nethermancer zerevor");
}

bool IllidariCouncilZerevorCastsDangerousAoesTrigger::IsActive()
{
    if (!botAI->IsRanged(bot))
        return false;

    if (!AI_VALUE2(Unit*, "find target", "high nethermancer zerevor"))
        return false;

    return !HasDangerousCouncilAura(bot);
}

bool IllidariCouncilPetsScrewUpThePullTrigger::IsActive()
{
    if (bot->getClass() != CLASS_HUNTER && bot->getClass() != CLASS_WARLOCK)
        return false;

    Pet* pet = bot->GetPet();
    if (!pet || !pet->IsAlive())
        return false;

    return AI_VALUE2(Unit*, "find target", "gathios the shatterer");
}

bool IllidariCouncilDeterminingDpsAssignmentsTrigger::IsActive()
{
    if (botAI->IsHeal(bot))
        return false;

    if (!AI_VALUE2(Unit*, "find target", "gathios the shatterer"))
        return false;

    if (botAI->IsMainTank(bot) || botAI->IsAssistTankOfIndex(bot, 0, false) ||
        GetZerevorMageTank(bot) == bot)
    {
        return false;
    }

    Unit* darkshadow = AI_VALUE2(Unit*, "find target", "veras darkshadow");
    if (botAI->IsTank(bot) && botAI->IsAssistTankOfIndex(bot, 1, false) &&
        darkshadow && !darkshadow->HasAura(
            static_cast<uint32>(BlackTempleSpells::SPELL_VANISH)))
    {
        return false;
    }

    return true;
}

bool IllidariCouncilNeedToManageDpsTimerTrigger::IsActive()
{
    if (!botAI->IsDps(bot))
        return false;

    if (!AI_VALUE2(Unit*, "find target", "gathios the shatterer"))
        return false;

    return IsMechanicTrackerBot(
        botAI, bot, BLACK_TEMPLE_MAP_ID, GetZerevorMageTank(bot));
}

// Illidan Stormrage <The Betrayer>

bool IllidanStormrageTankNeedsAggroTrigger::IsActive()
{
    if (bot->getClass() != CLASS_HUNTER)
        return false;

    Unit* illidan = AI_VALUE2(Unit*, "find target", "illidan stormrage");
    return illidan && illidan->GetHealth() > 1;
}

bool IllidanStormrageBossCastsFlameCrashInFrontOfMainTankTrigger::IsActive()
{
    if (!botAI->IsMainTank(bot))
        return false;

    Unit* illidan = AI_VALUE2(Unit*, "find target", "illidan stormrage");
    if (!illidan)
        return false;

    int phase = GetIllidanPhase(illidan);
    return phase == 1 || phase == 3 || phase == 5;
}

bool IllidanStormrageBotHasParasiticShadowfiendTrigger::IsActive()
{
    Unit* illidan = AI_VALUE2(Unit*, "find target", "illidan stormrage");
    if (!illidan || illidan->GetHealth() == 1 ||
        illidan->GetVictim() == bot)
    {
        return false;
    }

    int phase = GetIllidanPhase(illidan);
    if (phase == 2 || phase == 4)
        return false;

    if (botAI->IsMainTank(bot))
        return false;

    if (phase == 5 && FindNearestTrap(botAI, bot))
        return false;

    Player* infected = GetBotWithParasiticShadowfiend(bot);
    if (!infected)
        return false;

    if (infected == bot ||
        (phase != 1 && bot->getClass() == CLASS_HUNTER))
    {
        return true;
    }

    return false;
}

bool IllidanStormrageParasiticShadowfiendsRunWildTrigger::IsActive()
{
    if (bot->getClass() != CLASS_SHAMAN)
        return false;

    Unit* illidan = AI_VALUE2(Unit*, "find target", "illidan stormrage");
    if (!illidan || illidan->GetHealth() == 1 || GetIllidanPhase(illidan) == 2)
        return false;

    ObjectGuid guid = bot->m_SummonSlot[SUMMON_SLOT_TOTEM_EARTH];
    if (guid.IsEmpty())
        return true;

    Creature* totem = bot->GetMap()->GetCreature(guid);
    return !totem || totem->GetDistance(bot) > 20.0f ||
           totem->GetUInt32Value(UNIT_CREATED_BY_SPELL) !=
               static_cast<uint32>(BlackTempleSpells::SPELL_EARTHBIND_TOTEM);
}

bool IllidanStormrageBossSummonedFlamesOfAzzinothTrigger::IsActive()
{
    if (!botAI->IsAssistTankOfIndex(bot, 0, true) &&
        !botAI->IsAssistTankOfIndex(bot, 1, true))
    {
        return false;
    }

    Unit* illidan = AI_VALUE2(Unit*, "find target", "illidan stormrage");
    return illidan && GetIllidanPhase(illidan) == 2;
}

bool IllidanStormragePetsDieToFireTrigger::IsActive()
{
    Unit* illidan = AI_VALUE2(Unit*, "find target", "illidan stormrage");
    if (!illidan || illidan->GetHealth() == 1)
        return false;

    Pet* pet = bot->GetPet();
    return pet && pet->IsAlive();
}

bool IllidanStormrageGrateIsSafeFromFlamesTrigger::IsActive()
{
    if (botAI->IsAssistTankOfIndex(bot, 0, true) ||
        botAI->IsAssistTankOfIndex(bot, 1, true))
    {
        return false;
    }

    Unit* illidan = AI_VALUE2(Unit*, "find target", "illidan stormrage");
    return illidan && GetIllidanPhase(illidan) == 2;
}

bool IllidanStormrageBotStruckByDarkBarrageTrigger::IsActive()
{
    if (bot->getClass() != CLASS_MAGE && bot->getClass() != CLASS_PALADIN &&
        bot->getClass() != CLASS_ROGUE)
    {
        return false;
    }

    if (!AI_VALUE2(Unit*, "find target", "illidan stormrage"))
        return false;

    if (botAI->HasAura("ice block", bot))
    {
        botAI->RemoveAura("ice block");
        return true;
    }
    else if (!botAI->IsHeal(bot) && botAI->HasAura("divine shield", bot))
    {
        botAI->RemoveAura("divine shield");
        return true;
    }

    return bot->HasAura(
        static_cast<uint32>(BlackTempleSpells::SPELL_DARK_BARRAGE));
}

bool IllidanStormrageBossIsPreparingToLandTrigger::IsActive()
{
    if (botAI->IsMainTank(bot))
        return false;

    Unit* illidan = AI_VALUE2(Unit*, "find target", "illidan stormrage");
    return illidan && GetIllidanPhase(illidan) == 0;
}

bool IllidanStormrageBossDealsSplashDamageTrigger::IsActive()
{
    if (!botAI->IsRanged(bot))
        return false;

    Unit* illidan = AI_VALUE2(Unit*, "find target", "illidan stormrage");
    if (!illidan || illidan->HasAura
            (static_cast<uint32>(BlackTempleSpells::SPELL_CAGED)))
    {
        return false;
    }

    int phase = GetIllidanPhase(illidan);

    if (phase == 4 && GetIllidanWarlockTank(bot) == bot)
        return false;

    return phase == 3 || phase == 4 || phase == 5;
}

bool IllidanStormrageThisExpansionHatesMeleeTrigger::IsActive()
{
    if (!botAI->IsMelee(bot))
        return false;

    Unit* illidan = AI_VALUE2(Unit*, "find target", "illidan stormrage");
    return illidan && GetIllidanPhase(illidan) == 4;
}

bool IllidanStormrageBossTransformsIntoDemonTrigger::IsActive()
{
    if (bot->getClass() != CLASS_WARLOCK)
        return false;

    Unit* illidan = AI_VALUE2(Unit*, "find target", "illidan stormrage");
    if (!illidan || GetIllidanPhase(illidan) != 4)
        return false;

    return GetIllidanWarlockTank(bot) == bot;
}

bool IllidanStormrageBossSpawnsAddsTrigger::IsActive()
{
    if (botAI->IsHeal(bot))
        return false;

    Unit* illidan = AI_VALUE2(Unit*, "find target", "illidan stormrage");
    if (!illidan || illidan->GetHealth() == 1)
        return false;

    if (botAI->IsTank(bot) && GetIllidanPhase(illidan) != 4)
        return false;

    return true;
}

bool IllidanStormrageMaievPlacedShadowTrapTrigger::IsActive()
{
    Unit* illidan = AI_VALUE2(Unit*, "find target", "illidan stormrage");
    if (!illidan || illidan->GetHealth() == 1 ||
        GetIllidanPhase(illidan) != 5)
    {
        return false;
    }

    GameObject* trap = FindNearestTrap(botAI, bot);
    if (!trap)
        return false;

    Group* group = bot->GetGroup();
    if (!group)
        return false;

    Player* closestBot = nullptr;
    float closestDist = std::numeric_limits<float>::max();

    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (!member || !member->IsAlive() || illidan->GetVictim() == member ||
            !GET_PLAYERBOT_AI(member) || botAI->IsMainTank(member))
        {
            continue;
        }

        float dist = member->GetDistance(trap);
        if (dist < closestDist)
        {
            closestDist = dist;
            closestBot = member;
        }
    }

    return closestBot == bot;
}

bool IllidanStormrageNeedToManageDpsTimerAndRtiTrigger::IsActive()
{
    if (!botAI->IsDps(bot))
        return false;

    Unit* illidan = AI_VALUE2(Unit*, "find target", "illidan stormrage");
    if (!illidan || illidan->GetHealth() == 1)
        return false;

    return IsMechanicTrackerBot(
        botAI, bot, BLACK_TEMPLE_MAP_ID, GetIllidanWarlockTank(bot));
}

// Destroying hazards behind phases is not gated behind CheatMask
// The strategy simply cannot work without doing this
bool IllidanStormrageNeedToClearHazardsBetweenPhasesTrigger::IsActive()
{
    if (!botAI->IsDps(bot))
        return false;

    Unit* illidan = AI_VALUE2(Unit*, "find target", "illidan stormrage");
    if (!illidan || illidan->GetHealth() == 1)
        return false;

    int phase = GetIllidanPhase(illidan);
    if (phase != 0 && phase != 2 && phase != 4)
        return false;

    return IsMechanicTrackerBot(
        botAI, bot, BLACK_TEMPLE_MAP_ID, GetIllidanWarlockTank(bot));
}

bool IllidanStormrageCheatTrigger::IsActive()
{
    if (!botAI->HasCheat(BotCheatMask::raid) || !botAI->IsDps(bot))
        return false;

    Unit* illidan = AI_VALUE2(Unit*, "find target", "illidan stormrage");
    if (!illidan)
        return false;

    int phase = GetIllidanPhase(illidan);
    return phase == 2 || phase == 4;
}

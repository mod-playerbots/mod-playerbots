#include "Playerbots.h"
#include "RaidNaxxActions.h"
#include "RaidNaxxSpellIds.h"
#include "Spell.h"
#include "Timer.h"

//bool HeiganDanceAction::CalculateSafe()
//{
//    Unit* boss = AI_VALUE2(Unit*, "find target", "heigan the unclean");
//    if (!boss)
//    {
//        return false;
//    }
//    uint32 now = getMSTime();
//    platform_phase = boss->IsWithinDist2d(platform.first, platform.second, 10.0f);
//    if (last_eruption_ms != 0 && now - last_eruption_ms > 15000)
//    {
//        ResetSafe();
//    }
//    if (boss->HasUnitState(UNIT_STATE_CASTING))
//    {
//        Spell* spell = boss->GetCurrentSpell(CURRENT_GENERIC_SPELL);
//        if (!spell)
//        {
//            spell = boss->GetCurrentSpell(CURRENT_CHANNELED_SPELL);
//        }
//        if (spell)
//        {
//            SpellInfo const* info = spell->GetSpellInfo();
//            bool isEruption = NaxxSpellIds::MatchesAnySpellId(info, {NaxxSpellIds::Eruption10});
//            if (!isEruption && info && info->SpellName[LOCALE_enUS])
//            {
//                // Fallback to name for custom spell data.
//                isEruption = botAI->EqualLowercaseName(info->SpellName[LOCALE_enUS], "eruption");
//            }
//            if (isEruption)
//            {
//                if (last_eruption_ms == 0 || now - last_eruption_ms > 500)
//                {
//                    NextSafe();
//                }
//                last_eruption_ms = now;
//            }
//        }
//    }
//    return true;
//}
//
//bool HeiganDanceMeleeAction::Execute(Event event)
//{
//    CalculateSafe();
//    if (!platform_phase && botAI->IsMainTank(bot) && !AI_VALUE2(bool, "has aggro", "boss target"))
//    {
//        return false;
//    }
//    assert(curr_safe >= 0 && curr_safe <= 3);
//    return MoveInside(bot->GetMapId(), waypoints[curr_safe].first, waypoints[curr_safe].second, bot->GetPositionZ(),
//                      botAI->IsMainTank(bot) ? 0 : 0, MovementPriority::MOVEMENT_COMBAT);
//}
//
//bool HeiganDanceRangedAction::Execute(Event event)
//{
//    CalculateSafe();
//    if (!platform_phase)
//    {
//        if (MoveTo(bot->GetMapId(), platform.first, platform.second, 276.54f, false, false, false, false,
//                   MovementPriority::MOVEMENT_COMBAT))
//        {
//            return true;
//        }
//        return MoveInside(bot->GetMapId(), platform.first, platform.second, 276.54f, 2.0f,
//                          MovementPriority::MOVEMENT_COMBAT);
//    }
//    botAI->InterruptSpell();
//    return MoveInside(bot->GetMapId(), waypoints[curr_safe].first, waypoints[curr_safe].second, bot->GetPositionZ(), 0,
//                      MovementPriority::MOVEMENT_COMBAT);
//}

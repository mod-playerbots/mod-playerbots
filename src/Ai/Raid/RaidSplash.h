/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_RAIDSPLASH_H
#define PLAYERBOTS_RAIDSPLASH_H

#include <set>
#include <string>

#include "RaidCombatHelpers.h"

class Action;
class CastSpellAction;
class PlayerbotAI;

// What IsRaidAreaDamage and SpellSplashes read off an action, worked out once per action. One per
// multiplier: an Action* is only unique inside one bot's AI.
class RaidSplashCache
{
public:
    struct Facts
    {
        bool aoeThreat = false;
        bool multiTarget = false;
        bool summon = false;
        bool heal = false;
        CastSpellAction* cast = nullptr;
        RaidSpellIdRef spellId;
    };

    Facts& Of(Action* action);

private:
    RaidActionCache<Facts> _facts;
};

// Whether the action's spell lands on an area, heals never count. Read off the spell because the threat
// type misses whirlwind, divine storm, consecration, death and decay, swipe and pestilence.
bool SpellSplashes(PlayerbotAI* botAI, Action* action);
// Same answer and the same "spell id" reads, with the per-action lookups kept in cache
bool SpellSplashes(PlayerbotAI* botAI, Action* action, RaidSplashCache& cache);

// The totem and trap summons whose area lives on what they summon: Magma Totem, Fire Nova and Explosive
// Trap. Their own spell reads as single target.
bool SummonSplashes(Action* action);

// Cleaves, chains and melee area abilities by action name: their own spells carry no area
extern std::set<std::string> const MULTI_TARGET_ACTIONS;

#endif

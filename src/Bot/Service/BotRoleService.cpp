/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#include "BotRoleService.h"

#include "AiFactory.h"
#include "Group.h"
#include "ObjectGuid.h"
#include "PlayerbotAI.h"
#include "Playerbots.h"
#include "Player.h"
#include "Strategy.h"
#include "Unit.h"

// ============================================================================
// Static method implementations (main logic)
// ============================================================================

bool BotRoleService::IsRangedStatic(Player* player, bool bySpec)
{
    PlayerbotAI* botAi = GET_PLAYERBOT_AI(player);
    if (!bySpec && botAi)
        return botAi->ContainsStrategy(STRATEGY_TYPE_RANGED);

    int tab = AiFactory::GetPlayerSpecTab(player);
    switch (player->getClass())
    {
        case CLASS_DEATH_KNIGHT:
        case CLASS_WARRIOR:
        case CLASS_ROGUE:
            return false;
            break;
        case CLASS_DRUID:
            if (tab == DRUID_TAB_FERAL)
            {
                return false;
            }
            break;
        case CLASS_PALADIN:
            if (tab != PALADIN_TAB_HOLY)
            {
                return false;
            }
            break;
        case CLASS_SHAMAN:
            if (tab == SHAMAN_TAB_ENHANCEMENT)
            {
                return false;
            }
            break;
    }

    return true;
}

bool BotRoleService::IsMeleeStatic(Player* player, bool bySpec)
{
    return !IsRangedStatic(player, bySpec);
}

bool BotRoleService::IsCasterStatic(Player* player, bool bySpec)
{
    return IsRangedStatic(player, bySpec) && player->getClass() != CLASS_HUNTER;
}

bool BotRoleService::IsComboStatic(Player* player)
{
    return player->getClass() == CLASS_ROGUE ||
           (player->getClass() == CLASS_DRUID && player->HasAura(768));  // cat druid
}

bool BotRoleService::IsRangedDpsStatic(Player* player, bool bySpec)
{
    return IsRangedStatic(player, bySpec) && IsDpsStatic(player, bySpec);
}

bool BotRoleService::IsHealAssistantOfIndexStatic(Player* player, int index)
{
    Group* group = player->GetGroup();
    if (!group)
    {
        return false;
    }

    int counter = 0;

    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();

        if (!member)
        {
            continue;
        }

        if (IsHealStatic(member))  // Check if the member is a healer
        {
            bool isAssistant = group->IsAssistant(member->GetGUID());

            // Check if the index matches for both assistant and non-assistant healers
            if ((isAssistant && index == counter) || (!isAssistant && index == counter))
            {
                return player == member;
            }

            counter++;
        }
    }

    return false;
}

bool BotRoleService::IsRangedDpsAssistantOfIndexStatic(Player* player, int index)
{
    Group* group = player->GetGroup();
    if (!group)
    {
        return false;
    }

    int counter = 0;

    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();

        if (!member)
        {
            continue;
        }

        if (IsRangedDpsStatic(member))  // Check if the member is a ranged DPS
        {
            bool isAssistant = group->IsAssistant(member->GetGUID());

            // Check the index for both assistant and non-assistant ranges
            if ((isAssistant && index == counter) || (!isAssistant && index == counter))
            {
                return player == member;
            }

            counter++;
        }
    }

    return false;
}

int32 BotRoleService::GetAssistTankIndexStatic(Player* player)
{
    Group* group = player->GetGroup();
    if (!group)
    {
        return -1;
    }

    int counter = 0;
    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (!member)
        {
            continue;
        }

        if (player == member)
        {
            return counter;
        }

        if (IsTankStatic(member, true) && group->IsAssistant(member->GetGUID()))
        {
            counter++;
        }
    }

    return 0;
}

bool BotRoleService::IsTankStatic(Player* player, bool bySpec)
{
    PlayerbotAI* botAi = GET_PLAYERBOT_AI(player);
    if (!bySpec && botAi)
        return botAi->ContainsStrategy(STRATEGY_TYPE_TANK);

    int tab = AiFactory::GetPlayerSpecTab(player);
    switch (player->getClass())
    {
        case CLASS_DEATH_KNIGHT:
            if (tab == DEATH_KNIGHT_TAB_BLOOD)
            {
                return true;
            }
            break;
        case CLASS_PALADIN:
            if (tab == PALADIN_TAB_PROTECTION)
            {
                return true;
            }
            break;
        case CLASS_WARRIOR:
            if (tab == WARRIOR_TAB_PROTECTION)
            {
                return true;
            }
            break;
        case CLASS_DRUID:
            if (tab == DRUID_TAB_FERAL && (player->GetShapeshiftForm() == FORM_BEAR ||
                                           player->GetShapeshiftForm() == FORM_DIREBEAR || player->HasAura(16931)))
            {
                return true;
            }
            break;
    }
    return false;
}

bool BotRoleService::IsHealStatic(Player* player, bool bySpec)
{
    PlayerbotAI* botAi = GET_PLAYERBOT_AI(player);
    if (!bySpec && botAi)
        return botAi->ContainsStrategy(STRATEGY_TYPE_HEAL);

    int tab = AiFactory::GetPlayerSpecTab(player);
    switch (player->getClass())
    {
        case CLASS_PRIEST:
            if (tab == PRIEST_TAB_DISCIPLINE || tab == PRIEST_TAB_HOLY)
            {
                return true;
            }
            break;
        case CLASS_DRUID:
            if (tab == DRUID_TAB_RESTORATION)
            {
                return true;
            }
            break;
        case CLASS_SHAMAN:
            if (tab == SHAMAN_TAB_RESTORATION)
            {
                return true;
            }
            break;
        case CLASS_PALADIN:
            if (tab == PALADIN_TAB_HOLY)
            {
                return true;
            }
            break;
    }
    return false;
}

bool BotRoleService::IsDpsStatic(Player* player, bool bySpec)
{
    PlayerbotAI* botAi = GET_PLAYERBOT_AI(player);
    if (!bySpec && botAi)
        return botAi->ContainsStrategy(STRATEGY_TYPE_DPS);

    int tab = AiFactory::GetPlayerSpecTab(player);
    switch (player->getClass())
    {
        case CLASS_MAGE:
        case CLASS_WARLOCK:
        case CLASS_HUNTER:
        case CLASS_ROGUE:
            return true;
        case CLASS_PRIEST:
            if (tab == PRIEST_TAB_SHADOW)
            {
                return true;
            }
            break;
        case CLASS_DRUID:
            if (tab == DRUID_TAB_BALANCE)
            {
                return true;
            }
            if (tab == DRUID_TAB_FERAL && !IsTankStatic(player, bySpec))
            {
                return true;
            }
            break;
        case CLASS_SHAMAN:
            if (tab != SHAMAN_TAB_RESTORATION)
            {
                return true;
            }
            break;
        case CLASS_PALADIN:
            if (tab == PALADIN_TAB_RETRIBUTION)
            {
                return true;
            }
            break;
        case CLASS_DEATH_KNIGHT:
            if (tab != DEATH_KNIGHT_TAB_BLOOD)
            {
                return true;
            }
            break;
        case CLASS_WARRIOR:
            if (tab != WARRIOR_TAB_PROTECTION)
            {
                return true;
            }
            break;
    }
    return false;
}

bool BotRoleService::IsMainTankStatic(Player* player)
{
    Group* group = player->GetGroup();
    if (!group)
    {
        return IsTankStatic(player);
    }

    ObjectGuid mainTank = ObjectGuid();
    Group::MemberSlotList const& slots = group->GetMemberSlots();

    for (Group::member_citerator itr = slots.begin(); itr != slots.end(); ++itr)
    {
        if (itr->flags & MEMBER_FLAG_MAINTANK)
        {
            mainTank = itr->guid;
            break;
        }
    }

    if (mainTank != ObjectGuid::Empty)
    {
        return player->GetGUID() == mainTank;
    }

    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (!member)
        {
            continue;
        }

        if (IsTankStatic(member) && member->IsAlive())
        {
            return player->GetGUID() == member->GetGUID();
        }
    }

    return false;
}

bool BotRoleService::IsBotMainTankStatic(Player* player)
{
    if (!player->GetSession()->IsBot() || !IsTankStatic(player))
    {
        return false;
    }

    if (IsMainTankStatic(player))
    {
        return true;
    }

    Group* group = player->GetGroup();
    if (!group)
    {
        return true;  // If no group, consider the bot as main tank
    }

    int32 botAssistTankIndex = GetAssistTankIndexStatic(player);
    if (botAssistTankIndex == -1)
    {
        return false;
    }

    for (GroupReference* gref = group->GetFirstMember(); gref; gref = gref->next())
    {
        Player* member = gref->GetSource();
        if (!member)
        {
            continue;
        }

        int32 memberAssistTankIndex = GetAssistTankIndexStatic(member);
        if (memberAssistTankIndex == -1)
        {
            continue;
        }

        if (memberAssistTankIndex == botAssistTankIndex && player == member)
        {
            return true;
        }

        if (memberAssistTankIndex < botAssistTankIndex && member->GetSession()->IsBot())
        {
            return false;
        }

        return false;
    }

    return false;
}

uint32 BotRoleService::GetGroupTankNumStatic(Player* player)
{
    Group* group = player->GetGroup();
    if (!group)
    {
        return 0;
    }
    uint32 result = 0;
    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();

        if (!member)
        {
            continue;
        }

        if (IsTankStatic(member) && member->IsAlive())
        {
            result++;
        }
    }
    return result;
}

bool BotRoleService::IsAssistTankStatic(Player* player)
{
    return IsTankStatic(player) && !IsMainTankStatic(player);
}

bool BotRoleService::IsAssistTankOfIndexStatic(Player* player, int index, bool ignoreDeadPlayers)
{
    Group* group = player->GetGroup();
    if (!group)
    {
        return false;
    }
    int counter = 0;
    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();

        if (!member)
        {
            continue;
        }

        if (ignoreDeadPlayers && !member->IsAlive())
            continue;

        if (group->IsAssistant(member->GetGUID()) && IsAssistTankStatic(member))
        {
            if (index == counter)
            {
                return player == member;
            }
            counter++;
        }
    }
    // not enough
    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();

        if (!member)
        {
            continue;
        }

        if (ignoreDeadPlayers && !member->IsAlive())
            continue;

        if (!group->IsAssistant(member->GetGUID()) && IsAssistTankStatic(member))
        {
            if (index == counter)
            {
                return player == member;
            }
            counter++;
        }
    }
    return false;
}

// ============================================================================
// Instance method implementations (IRoleService interface)
// These delegate to the static methods or use bot context where needed
// ============================================================================

bool BotRoleService::IsTank(Player* player, bool bySpec) const
{
    return IsTankStatic(player, bySpec);
}

bool BotRoleService::IsHeal(Player* player, bool bySpec) const
{
    return IsHealStatic(player, bySpec);
}

bool BotRoleService::IsDps(Player* player, bool bySpec) const
{
    return IsDpsStatic(player, bySpec);
}

bool BotRoleService::IsRanged(Player* player, bool bySpec) const
{
    return IsRangedStatic(player, bySpec);
}

bool BotRoleService::IsMelee(Player* player, bool bySpec) const
{
    return IsMeleeStatic(player, bySpec);
}

bool BotRoleService::IsCaster(Player* player, bool bySpec) const
{
    return IsCasterStatic(player, bySpec);
}

bool BotRoleService::IsRangedDps(Player* player, bool bySpec) const
{
    return IsRangedDpsStatic(player, bySpec);
}

bool BotRoleService::IsCombo(Player* player) const
{
    return IsComboStatic(player);
}

bool BotRoleService::IsBotMainTank(Player* player) const
{
    return IsBotMainTankStatic(player);
}

bool BotRoleService::IsMainTank(Player* player) const
{
    return IsMainTankStatic(player);
}

bool BotRoleService::IsAssistTank(Player* player) const
{
    return IsAssistTankStatic(player);
}

bool BotRoleService::IsAssistTankOfIndex(Player* player, int index, bool ignoreDeadPlayers) const
{
    return IsAssistTankOfIndexStatic(player, index, ignoreDeadPlayers);
}

uint32 BotRoleService::GetGroupTankNum(Player* player) const
{
    return GetGroupTankNumStatic(player);
}

int32 BotRoleService::GetAssistTankIndex(Player* player) const
{
    return GetAssistTankIndexStatic(player);
}

int32 BotRoleService::GetGroupSlotIndex(Player* player) const
{
    if (!botAI_)
    {
        return -1;
    }

    Player* bot = botAI_->GetBot();
    if (!bot)
    {
        return -1;
    }

    Group* group = bot->GetGroup();
    if (!group)
    {
        return -1;
    }
    int counter = 0;
    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();

        if (!member)
        {
            continue;
        }

        if (player == member)
        {
            return counter;
        }
        counter++;
    }
    return 0;
}

int32 BotRoleService::GetRangedIndex(Player* player) const
{
    if (!IsRangedStatic(player))
    {
        return -1;
    }

    if (!botAI_)
    {
        return -1;
    }

    Player* bot = botAI_->GetBot();
    if (!bot)
    {
        return -1;
    }

    Group* group = bot->GetGroup();
    if (!group)
    {
        return -1;
    }
    int counter = 0;
    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();

        if (!member)
        {
            continue;
        }

        if (player == member)
        {
            return counter;
        }
        if (IsRangedStatic(member))
        {
            counter++;
        }
    }
    return 0;
}

int32 BotRoleService::GetClassIndex(Player* player, uint8 cls) const
{
    if (player->getClass() != cls)
    {
        return -1;
    }

    if (!botAI_)
    {
        return -1;
    }

    Player* bot = botAI_->GetBot();
    if (!bot)
    {
        return -1;
    }

    Group* group = bot->GetGroup();
    if (!group)
    {
        return -1;
    }
    int counter = 0;
    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();

        if (!member)
        {
            continue;
        }

        if (player == member)
        {
            return counter;
        }
        if (member->getClass() == cls)
        {
            counter++;
        }
    }
    return 0;
}

int32 BotRoleService::GetRangedDpsIndex(Player* player) const
{
    if (!IsRangedDpsStatic(player))
    {
        return -1;
    }

    if (!botAI_)
    {
        return -1;
    }

    Player* bot = botAI_->GetBot();
    if (!bot)
    {
        return -1;
    }

    Group* group = bot->GetGroup();
    if (!group)
    {
        return -1;
    }
    int counter = 0;
    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();

        if (!member)
        {
            continue;
        }

        if (player == member)
        {
            return counter;
        }
        if (IsRangedDpsStatic(member))
        {
            counter++;
        }
    }
    return 0;
}

int32 BotRoleService::GetMeleeIndex(Player* player) const
{
    if (IsRangedStatic(player))
    {
        return -1;
    }

    if (!botAI_)
    {
        return -1;
    }

    Player* bot = botAI_->GetBot();
    if (!bot)
    {
        return -1;
    }

    Group* group = bot->GetGroup();
    if (!group)
    {
        return -1;
    }
    int counter = 0;
    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();

        if (!member)
        {
            continue;
        }

        if (player == member)
        {
            return counter;
        }
        if (!IsRangedStatic(member))
        {
            counter++;
        }
    }
    return 0;
}

bool BotRoleService::IsHealAssistantOfIndex(Player* player, int index) const
{
    return IsHealAssistantOfIndexStatic(player, index);
}

bool BotRoleService::IsRangedDpsAssistantOfIndex(Player* player, int index) const
{
    return IsRangedDpsAssistantOfIndexStatic(player, index);
}

bool BotRoleService::HasAggro(Unit* unit) const
{
    if (!botAI_)
    {
        return false;
    }

    Player* bot = botAI_->GetBot();
    if (!bot)
    {
        return false;
    }

    if (!unit || !unit->IsAlive())
        return false;

    bool isMT = IsMainTankStatic(bot);
    Unit* victim = unit->GetVictim();
    if (victim && (victim->GetGUID() == bot->GetGUID() ||
                   (!isMT && victim->ToPlayer() && IsTankStatic(victim->ToPlayer()))))
    {
        return true;
    }
    return false;
}

#include "AutoMaintenanceOnLevelupAction.h"

#include "BroadcastHelper.h"
#include "GuildMgr.h"
#include "PlayerbotAIConfig.h"
#include "PlayerbotFactory.h"
#include "Playerbots.h"
#include "RandomPlayerbotMgr.h"
#include "SharedDefines.h"

bool AutoMaintenanceOnLevelupAction::Execute(Event event)
{
    AutoPickTalents();
    AutoLearnSpell();
    AutoUpgradeEquip();
    AutoTeleportForLevel();
    return true;
}

void AutoMaintenanceOnLevelupAction::AutoTeleportForLevel()
{
    if (!sPlayerbotAIConfig->autoTeleportForLevel || !sRandomPlayerbotMgr->IsRandomBot(bot))
    {
        return;
    }
    if (botAI->HasRealPlayerMaster())
    {
        return;
    }
    sRandomPlayerbotMgr->RandomTeleportForLevel(bot);
    return;
}

void AutoMaintenanceOnLevelupAction::AutoPickTalents()
{
    if (!sPlayerbotAIConfig->autoPickTalents || !sRandomPlayerbotMgr->IsRandomBot(bot))
        return;

    if (bot->GetFreeTalentPoints() <= 0)
        return;

    PlayerbotFactory factory(bot, bot->GetLevel());
    factory.InitTalentsTree(true, true, true);
    factory.InitPetTalents();
}

void AutoMaintenanceOnLevelupAction::AutoLearnSpell()
{
    std::ostringstream out;
    LearnSpells(&out);

    if (!out.str().empty())
    {
        std::string const temp = out.str();
        out.seekp(0);
        out << "Learned spells: ";
        out << temp;
        out.seekp(-2, out.cur);
        out << ".";
        botAI->TellMaster(out);
    }
    return;
}

void AutoMaintenanceOnLevelupAction::LearnSpells(std::ostringstream* out)
{
    BroadcastHelper::BroadcastLevelup(botAI, bot);
    if (sPlayerbotAIConfig->autoLearnTrainerSpells && sRandomPlayerbotMgr->IsRandomBot(bot))
        LearnTrainerSpells(out);

    if (sPlayerbotAIConfig->autoLearnQuestSpells && sRandomPlayerbotMgr->IsRandomBot(bot))
        LearnQuestSpells(out);
}

void AutoMaintenanceOnLevelupAction::LearnTrainerSpells(std::ostringstream* out)
{
    PlayerbotFactory factory(bot, bot->GetLevel());
    factory.InitClassSpells();
    factory.InitAvailableSpells();
    factory.InitSkills();
    factory.InitPet();
}

void AutoMaintenanceOnLevelupAction::LearnQuestSpells(std::ostringstream* out)
{
    // retrieve all quest templates from the server
    ObjectMgr::QuestMap const& questTemplates = sObjectMgr->GetQuestTemplates();
    for (ObjectMgr::QuestMap::const_iterator i = questTemplates.begin(); i != questTemplates.end(); ++i)
    {
        Quest const* quest = i->second;

        // skip quests that are repeatable, too low level, or have no required classes
        if (!quest->GetRequiredClasses() || quest->IsRepeatable() || quest->GetMinLevel() < 10)
            continue;

        // skip if bot doesnt satisfy class, level, or race requirements
        if (!bot->SatisfyQuestClass(quest, false) || quest->GetMinLevel() > bot->GetLevel() ||
            !bot->SatisfyQuestRace(quest, false))
            continue;

        // rewardSpellCast for the quest (expected route)
        int32 spellId = quest->GetRewSpellCast();
        if (!spellId)
            continue;

        // spell info for this reward
        SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(spellId);
        if (!spellInfo)
            continue;

        SpellInfo const* triggeredInfo = nullptr;
        bool learnableSpellFound = false;

        // iterate over all spell effects to find a learnable spell
        for (uint8 i = 0; i < MAX_SPELL_EFFECTS; ++i)
        {
            if (spellInfo->Effects[i].Effect == SPELL_EFFECT_LEARN_SPELL)
            {
                // fallback: use the spell itself if TriggerSpell is null
                uint32 learnId =
                    spellInfo->Effects[i].TriggerSpell ? spellInfo->Effects[i].TriggerSpell : spellInfo->Id;

                // skip if bot already knows the spell
                if (!bot->HasSpell(learnId))
                {
                    triggeredInfo = sSpellMgr->GetSpellInfo(learnId);

                    // skip if this is a trade skill spell
                    if (triggeredInfo && triggeredInfo->Effects[0].Effect == SPELL_EFFECT_TRADE_SKILL)
                        break;

                    learnableSpellFound = true;
                    break;
                }
            }
        }

        if (!learnableSpellFound)
            continue;

        // skip if bot doesn't satisfy quest skill requirements
        if (!bot->SatisfyQuestSkill(quest, false))
            continue;

        // learn the spell for the bot
        bot->learnSpell(triggeredInfo->Id);

        // append spell to output stream
        *out << FormatSpell(triggeredInfo) << ", ";
    }

    // Note: currently leaves a trailing comma, could be trimmed later if needed
}

std::string const AutoMaintenanceOnLevelupAction::FormatSpell(SpellInfo const* sInfo)
{
    std::ostringstream out;
    std::string const rank = sInfo->Rank[0];

    if (rank.empty())
        out << "|cffffffff|Hspell:" << sInfo->Id << "|h[" << sInfo->SpellName[LOCALE_enUS] << "]|h|r";
    else
        out << "|cffffffff|Hspell:" << sInfo->Id << "|h[" << sInfo->SpellName[LOCALE_enUS] << " " << rank << "]|h|r";

    return out.str();
}

void AutoMaintenanceOnLevelupAction::AutoUpgradeEquip()
{
    if (!sPlayerbotAIConfig->autoUpgradeEquip || !sRandomPlayerbotMgr->IsRandomBot(bot))
        return;

    PlayerbotFactory factory(bot, bot->GetLevel());

    // Clean up old consumables before adding new ones
    factory.CleanupConsumables();

    factory.InitAmmo();
    factory.InitReagents();
    factory.InitFood();
    factory.InitConsumables();
    factory.InitPotions();

    if (!sPlayerbotAIConfig->equipmentPersistence || bot->GetLevel() < sPlayerbotAIConfig->equipmentPersistenceLevel)
    {
        if (sPlayerbotAIConfig->incrementalGearInit)
            factory.InitEquipment(true);
    }
}

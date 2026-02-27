#include "AutoMaintenanceOnLevelupAction.h"

#include "SpellMgr.h"

#include "PlayerbotAIConfig.h"
#include "PlayerbotFactory.h"
#include "RandomPlayerbotMgr.h"
#include "SharedDefines.h"
#include "BroadcastHelper.h"

bool AutoMaintenanceOnLevelupAction::Execute(Event /*event*/)
{
    AutoPickTalents();
    AutoLearnSpell();
    AutoUpgradeEquip();
    AutoTeleportForLevel();

    return true;
}

void AutoMaintenanceOnLevelupAction::AutoTeleportForLevel()
{
    if (!sPlayerbotAIConfig.autoTeleportForLevel || !sRandomPlayerbotMgr.IsRandomBot(bot))
    {
        return;
    }
    if (botAI->HasRealPlayerMaster())
    {
        return;
    }
    sRandomPlayerbotMgr.RandomTeleportForLevel(bot);
    return;
}

void AutoMaintenanceOnLevelupAction::AutoPickTalents()
{
    if (!sPlayerbotAIConfig.autoPickTalents || !sRandomPlayerbotMgr.IsRandomBot(bot))
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
    if (sPlayerbotAIConfig.autoLearnTrainerSpells && sRandomPlayerbotMgr.IsRandomBot(bot))
        LearnTrainerSpells(out);

    if (sPlayerbotAIConfig.autoLearnQuestSpells && sRandomPlayerbotMgr.IsRandomBot(bot))
        LearnQuestSpells(out);
}

void AutoMaintenanceOnLevelupAction::LearnTrainerSpells(std::ostringstream* out)
{
    PlayerbotFactory factory(bot, bot->GetLevel());
    factory.InitSkills();
    factory.InitClassSpells();
    factory.InitAvailableSpells();
    factory.InitPet();
}

void AutoMaintenanceOnLevelupAction::LearnQuestSpells(std::ostringstream* out)
{
    ObjectMgr::QuestMap const& questTemplates = sObjectMgr->GetQuestTemplates();
    for (ObjectMgr::QuestMap::const_iterator i = questTemplates.begin(); i != questTemplates.end(); ++i)
    {
        Quest const* quest = i->second;

        // only process class-specific quests to learn class-related spells, cuz
        // we don't want all these bunch of entries to be handled!
        if (!quest->GetRequiredClasses())
            continue;

        // skip quests that are repeatable, too low level, or above bots' level
        if (quest->IsRepeatable() || quest->GetMinLevel() < 10 || quest->GetMinLevel() > bot->GetLevel())
            continue;

        // skip if bot doesnt satisfy class, race, or skill requirements
        if (!bot->SatisfyQuestClass(quest, false) || !bot->SatisfyQuestRace(quest, false) ||
            !bot->SatisfyQuestSkill(quest, false))
            continue;

        // use the same logic and impl from Player::learnQuestRewardedSpells

        int32 spellId = quest->GetRewSpellCast();
        if (!spellId)
            continue;

        SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(spellId);
        if (!spellInfo)
            continue;

        // xinef: find effect with learn spell and check if we have this spell
        bool found = false;
        for (uint8 i = 0; i < MAX_SPELL_EFFECTS; ++i)
        {
            if (spellInfo->Effects[i].Effect == SPELL_EFFECT_LEARN_SPELL && spellInfo->Effects[i].TriggerSpell &&
                !bot->HasSpell(spellInfo->Effects[i].TriggerSpell))
            {
                // pusywizard: don't re-add profession specialties!
                if (SpellInfo const* triggeredInfo = sSpellMgr->GetSpellInfo(spellInfo->Effects[i].TriggerSpell))
                    if (triggeredInfo->Effects[0].Effect == SPELL_EFFECT_TRADE_SKILL)
                        break; // pussywizard: break and not cast the spell (found is false)

                found = true;
                break;
            }
        }

        // xinef: we know the spell, continue
        if (!found)
            continue;

        bot->CastSpell(bot, spellId, true);

        // Check if RewardDisplaySpell is set to output the proper spell learned
        // after processing quests. Output the original RewardSpell otherwise.
        uint32 rewSpellId = quest->GetRewSpell();
        if (rewSpellId)
        {
            if (SpellInfo const* rewSpellInfo = sSpellMgr->GetSpellInfo(rewSpellId))
            {
                *out << FormatSpell(rewSpellInfo) << ", ";
                continue;
            }
        }

        *out << FormatSpell(spellInfo) << ", ";
    }
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
    if (!sPlayerbotAIConfig.autoUpgradeEquip || !sRandomPlayerbotMgr.IsRandomBot(bot))
        return;

    PlayerbotFactory factory(bot, bot->GetLevel());

    // Clean up old consumables before adding new ones
    factory.CleanupConsumables();

    factory.InitAmmo();
    factory.InitReagents();
    factory.InitFood();
    factory.InitConsumables();
    factory.InitPotions();

    if (!sPlayerbotAIConfig.equipmentPersistence || bot->GetLevel() < sPlayerbotAIConfig.equipmentPersistenceLevel)
    {
        if (sPlayerbotAIConfig.incrementalGearInit)
            factory.InitEquipment(true);
    }
}

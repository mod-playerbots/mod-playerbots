#include "RaidIccStrategy.h"

#include "RaidIccMultipliers.h"

void RaidIccStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    //Lord Marrogwar
    triggers.push_back(new TriggerNode("icc lm",
        { NextAction("icc lm tank position", ACTION_RAID + 5),
                             NextAction("icc spike", ACTION_RAID + 3) }));

    //Lady Deathwhisper
    triggers.push_back(new TriggerNode("icc dark reckoning",
        { NextAction("icc dark reckoning", ACTION_MOVE + 5) }));

    triggers.push_back(new TriggerNode("icc lady deathwhisper",
        { NextAction("icc ranged position lady deathwhisper", ACTION_MOVE + 2),
                             NextAction("icc adds lady deathwhisper", ACTION_RAID + 3),
                             NextAction("icc shade lady deathwhisper", ACTION_RAID + 4) }));

    //Gunship Battle
    triggers.push_back(new TriggerNode("icc rotting frost giant tank position",
        { NextAction("icc rotting frost giant tank position", ACTION_RAID + 5) }));

    triggers.push_back(new TriggerNode("icc gunship cannon near",
        { NextAction("icc gunship enter cannon", ACTION_RAID + 6) }));

    triggers.push_back( new TriggerNode("icc in cannon",
        { NextAction("icc cannon fire", ACTION_RAID+5) }));

    triggers.push_back(new TriggerNode("icc gunship teleport ally",
        { NextAction("icc gunship teleport ally", ACTION_RAID + 4) }));

    triggers.push_back(new TriggerNode("icc gunship teleport horde",
        { NextAction("icc gunship teleport horde", ACTION_RAID + 4) }));

    //DBS
    triggers.push_back(new TriggerNode("icc dbs",
        { NextAction("icc dbs tank position", ACTION_RAID + 3),
                             NextAction("icc adds dbs", ACTION_RAID + 5) }));

    triggers.push_back(new TriggerNode("icc dbs main tank rune of blood",
        { NextAction("taunt spell", ACTION_EMERGENCY + 4) }));

    //DOGS
    triggers.push_back(new TriggerNode("icc stinky precious main tank mortal wound",
        { NextAction("taunt spell", ACTION_EMERGENCY + 4) }));

    //FESTERGUT
    triggers.push_back(new TriggerNode("icc festergut group position",
        { NextAction("icc festergut group position", ACTION_MOVE + 4) }));

    triggers.push_back(new TriggerNode("icc festergut main tank gastric bloat",
        { NextAction("taunt spell", ACTION_EMERGENCY + 6) }));

    triggers.push_back(new TriggerNode("icc festergut spore",
        { NextAction("icc festergut spore", ACTION_MOVE + 5) }));

    //ROTFACE
    triggers.push_back(new TriggerNode("icc rotface tank position",
        { NextAction("icc rotface tank position", ACTION_RAID + 5) }));

    triggers.push_back(new TriggerNode("icc rotface group position",
        { NextAction("icc rotface group position", ACTION_RAID + 6) }));

    triggers.push_back(new TriggerNode("icc rotface move away from explosion",
        { NextAction("icc rotface move away from explosion", ACTION_RAID +7) }));

    //PP
    triggers.push_back(new TriggerNode("icc putricide volatile ooze",
        { NextAction("icc putricide volatile ooze", ACTION_RAID + 4) }));

    triggers.push_back(new TriggerNode("icc putricide gas cloud",
        { NextAction("icc putricide gas cloud", ACTION_RAID + 5) }));

    triggers.push_back(new TriggerNode("icc putricide growing ooze puddle",
        { NextAction("icc putricide growing ooze puddle", ACTION_RAID + 3) }));

    triggers.push_back(new TriggerNode("icc putricide main tank mutated plague",
        { NextAction("taunt spell", ACTION_RAID + 10) }));

    triggers.push_back(new TriggerNode("icc putricide malleable goo",
        { NextAction("icc putricide avoid malleable goo", ACTION_RAID + 2) }));

    //BPC
    triggers.push_back(new TriggerNode("icc bpc keleseth tank",
        { NextAction("icc bpc keleseth tank", ACTION_RAID + 1) }));

    triggers.push_back(new TriggerNode("icc bpc main tank",
        { NextAction("icc bpc main tank", ACTION_RAID + 3) }));

    triggers.push_back(new TriggerNode("icc bpc empowered vortex",
        { NextAction("icc bpc empowered vortex", ACTION_RAID + 4) }));

    triggers.push_back(new TriggerNode("icc bpc kinetic bomb",
        { NextAction("icc bpc kinetic bomb", ACTION_RAID + 6) }));

    triggers.push_back(new TriggerNode("icc bpc ball of flame",
                        { NextAction("icc bpc ball of flame", ACTION_RAID + 7) }));

    //BQL
    triggers.push_back(new TriggerNode("icc bql group position",
        { NextAction("icc bql group position", ACTION_RAID) }));

    triggers.push_back(new TriggerNode("icc bql pact of darkfallen",
        { NextAction("icc bql pact of darkfallen", ACTION_RAID +1) }));

    triggers.push_back(new TriggerNode("icc bql vampiric bite",
        { NextAction("icc bql vampiric bite", ACTION_EMERGENCY + 5) }));

    //Sister Svalna
    triggers.push_back(new TriggerNode("icc valkyre spear",
        { NextAction("icc valkyre spear", ACTION_EMERGENCY + 5) }));

    triggers.push_back(new TriggerNode("icc sister svalna",
        { NextAction("icc sister svalna", ACTION_RAID + 5) }));

    //VDW
    triggers.push_back(new TriggerNode("icc valithria group",
        { NextAction("icc valithria group", ACTION_RAID + 1) }));

    triggers.push_back(new TriggerNode("icc valithria portal",
        { NextAction("icc valithria portal", ACTION_RAID + 5) }));

    triggers.push_back(new TriggerNode("icc valithria heal",
        { NextAction("icc valithria heal", ACTION_RAID+2) }));

    triggers.push_back(new TriggerNode("icc valithria dream cloud",
        { NextAction("icc valithria dream cloud", ACTION_RAID + 4) }));

    //SINDRAGOSA
    triggers.push_back(new TriggerNode("icc sindragosa group position",
        { NextAction("icc sindragosa group position", ACTION_RAID + 1) }));

    triggers.push_back(new TriggerNode("icc sindragosa frost beacon",
        { NextAction("icc sindragosa frost beacon", ACTION_RAID + 5) }));

    triggers.push_back(new TriggerNode("icc sindragosa blistering cold",
        { NextAction("icc sindragosa blistering cold", ACTION_EMERGENCY + 4) }));

    triggers.push_back(new TriggerNode("icc sindragosa unchained magic",
        { NextAction("icc sindragosa unchained magic", ACTION_RAID + 2) }));

    triggers.push_back(new TriggerNode("icc sindragosa chilled to the bone",
        { NextAction("icc sindragosa chilled to the bone", ACTION_RAID + 2) }));

    triggers.push_back(new TriggerNode("icc sindragosa mystic buffet",
        { NextAction("icc sindragosa mystic buffet", ACTION_RAID + 3) }));

    triggers.push_back(new TriggerNode("icc sindragosa main tank mystic buffet",
        { NextAction("taunt spell", ACTION_EMERGENCY + 3) }));

    triggers.push_back(new TriggerNode("icc sindragosa frost bomb",
        { NextAction("icc sindragosa frost bomb", ACTION_RAID + 7) }));

    triggers.push_back(new TriggerNode("icc sindragosa tank swap position",
        { NextAction("icc sindragosa tank swap position", ACTION_EMERGENCY + 2) }));

    //LICH KING
    triggers.push_back(new TriggerNode("icc lich king shadow trap",
        { NextAction("icc lich king shadow trap", ACTION_RAID + 6) }));

    triggers.push_back(new TriggerNode("icc lich king necrotic plague",
        { NextAction("icc lich king necrotic plague", ACTION_RAID + 3) }));

    triggers.push_back(new TriggerNode("icc lich king winter",
        { NextAction("icc lich king winter", ACTION_RAID +5) }));

    triggers.push_back(new TriggerNode("icc lich king adds",
        { NextAction("icc lich king adds", ACTION_RAID +2) }));
}

void RaidIccStrategy::InitMultipliers(std::vector<Multiplier*>& multipliers)
{
    multipliers.push_back(new IccLadyDeathwhisperMultiplier(botAI));
    multipliers.push_back(new IccAddsDbsMultiplier(botAI));
    multipliers.push_back(new IccDogsMultiplier(botAI));
    multipliers.push_back(new IccFestergutMultiplier(botAI));
    multipliers.push_back(new IccRotfaceMultiplier(botAI));
    multipliers.push_back(new IccAddsPutricideMultiplier(botAI));
    multipliers.push_back(new IccBpcAssistMultiplier(botAI));
    multipliers.push_back(new IccBqlMultiplier(botAI));
    multipliers.push_back(new IccValithriaDreamCloudMultiplier(botAI));
    multipliers.push_back(new IccSindragosaMultiplier(botAI));
    multipliers.push_back(new IccLichKingAddsMultiplier(botAI));
}

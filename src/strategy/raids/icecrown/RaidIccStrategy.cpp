#include "RaidIccStrategy.h"

#include "RaidIccMultipliers.h"

void RaidIccStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    //Lord Marrogwar
    triggers.push_back(new TriggerNode("icc lm",
        { new NextAction("icc lm tank position", ACTION_RAID + 5),
                             new NextAction("icc spike", ACTION_RAID + 3) }));

    //Lady Deathwhisper
    triggers.push_back(new TriggerNode("icc dark reckoning",
        { new NextAction("icc dark reckoning", ACTION_MOVE + 5) }));

    triggers.push_back(new TriggerNode("icc lady deathwhisper",
        { new NextAction("icc ranged position lady deathwhisper", ACTION_MOVE + 2),
                             new NextAction("icc adds lady deathwhisper", ACTION_RAID + 3),
                             new NextAction("icc shade lady deathwhisper", ACTION_RAID + 4) }));

    //Gunship Battle
    triggers.push_back(new TriggerNode("icc rotting frost giant tank position",
        { new NextAction("icc rotting frost giant tank position", ACTION_RAID + 5) }));

    triggers.push_back(new TriggerNode("icc gunship cannon near",
        { new NextAction("icc gunship enter cannon", ACTION_RAID + 6) }));

    triggers.push_back( new TriggerNode("icc in cannon",
        { new NextAction("icc cannon fire", ACTION_RAID+5) }));

    triggers.push_back(new TriggerNode("icc gunship teleport ally",
        { new NextAction("icc gunship teleport ally", ACTION_RAID + 4) }));

    triggers.push_back(new TriggerNode("icc gunship teleport horde",
        { new NextAction("icc gunship teleport horde", ACTION_RAID + 4) }));

    //DBS
    triggers.push_back(new TriggerNode("icc dbs",
        { new NextAction("icc dbs tank position", ACTION_RAID + 3),
                             new NextAction("icc adds dbs", ACTION_RAID + 5) }));

    triggers.push_back(new TriggerNode("icc dbs main tank rune of blood",
        { new NextAction("taunt spell", ACTION_EMERGENCY + 4) }));

    //DOGS
    triggers.push_back(new TriggerNode("icc stinky precious main tank mortal wound",
        { new NextAction("taunt spell", ACTION_EMERGENCY + 4) }));

    //FESTERGUT
    triggers.push_back(new TriggerNode("icc festergut group position",
        { new NextAction("icc festergut group position", ACTION_MOVE + 4) }));

    triggers.push_back(new TriggerNode("icc festergut main tank gastric bloat",
        { new NextAction("taunt spell", ACTION_EMERGENCY + 6) }));

    triggers.push_back(new TriggerNode("icc festergut spore",
        { new NextAction("icc festergut spore", ACTION_MOVE + 5) }));

    //ROTFACE
    triggers.push_back(new TriggerNode("icc rotface tank position",
        { new NextAction("icc rotface tank position", ACTION_RAID + 5) }));

    triggers.push_back(new TriggerNode("icc rotface group position",
        { new NextAction("icc rotface group position", ACTION_RAID + 6) }));

    triggers.push_back(new TriggerNode("icc rotface move away from explosion",
        { new NextAction("icc rotface move away from explosion", ACTION_RAID +7) }));

    //PP
    triggers.push_back(new TriggerNode("icc putricide volatile ooze",
        { new NextAction("icc putricide volatile ooze", ACTION_RAID + 4) }));

    triggers.push_back(new TriggerNode("icc putricide gas cloud",
        { new NextAction("icc putricide gas cloud", ACTION_RAID + 5) }));

    triggers.push_back(new TriggerNode("icc putricide growing ooze puddle",
        { new NextAction("icc putricide growing ooze puddle", ACTION_RAID + 3) }));

    triggers.push_back(new TriggerNode("icc putricide main tank mutated plague",
        { new NextAction("taunt spell", ACTION_RAID + 10) }));

    triggers.push_back(new TriggerNode("icc putricide malleable goo",
        { new NextAction("icc putricide avoid malleable goo", ACTION_RAID + 2) }));

    //BPC
    triggers.push_back(new TriggerNode("icc bpc keleseth tank",
        { new NextAction("icc bpc keleseth tank", ACTION_RAID + 1) }));

    triggers.push_back(new TriggerNode("icc bpc main tank",
        { new NextAction("icc bpc main tank", ACTION_RAID + 3) }));

    triggers.push_back(new TriggerNode("icc bpc empowered vortex",
        { new NextAction("icc bpc empowered vortex", ACTION_RAID + 4) }));

    triggers.push_back(new TriggerNode("icc bpc kinetic bomb",
        { new NextAction("icc bpc kinetic bomb", ACTION_RAID + 6) }));

    triggers.push_back(new TriggerNode("icc bpc ball of flame",
                        { new NextAction("icc bpc ball of flame", ACTION_RAID + 7) }));

    //BQL
    triggers.push_back(new TriggerNode("icc bql group position",
        { new NextAction("icc bql group position", ACTION_RAID) }));

    triggers.push_back(new TriggerNode("icc bql pact of darkfallen",
        { new NextAction("icc bql pact of darkfallen", ACTION_RAID +1) }));

    triggers.push_back(new TriggerNode("icc bql vampiric bite",
        { new NextAction("icc bql vampiric bite", ACTION_EMERGENCY + 5) }));

    //Sister Svalna
    triggers.push_back(new TriggerNode("icc valkyre spear",
        { new NextAction("icc valkyre spear", ACTION_EMERGENCY + 5) }));

    triggers.push_back(new TriggerNode("icc sister svalna",
        { new NextAction("icc sister svalna", ACTION_RAID + 5) }));

    //VDW
    triggers.push_back(new TriggerNode("icc valithria group",
        { new NextAction("icc valithria group", ACTION_RAID + 1) }));

    triggers.push_back(new TriggerNode("icc valithria portal",
        { new NextAction("icc valithria portal", ACTION_RAID + 5) }));

    triggers.push_back(new TriggerNode("icc valithria heal",
        { new NextAction("icc valithria heal", ACTION_RAID+2) }));

    triggers.push_back(new TriggerNode("icc valithria dream cloud",
        { new NextAction("icc valithria dream cloud", ACTION_RAID + 4) }));

    //SINDRAGOSA
    triggers.push_back(new TriggerNode("icc sindragosa group position",
        { new NextAction("icc sindragosa group position", ACTION_RAID + 1) }));

    triggers.push_back(new TriggerNode("icc sindragosa frost beacon",
        { new NextAction("icc sindragosa frost beacon", ACTION_RAID + 5) }));

    triggers.push_back(new TriggerNode("icc sindragosa blistering cold",
        { new NextAction("icc sindragosa blistering cold", ACTION_EMERGENCY + 4) }));

    triggers.push_back(new TriggerNode("icc sindragosa unchained magic",
        { new NextAction("icc sindragosa unchained magic", ACTION_RAID + 2) }));

    triggers.push_back(new TriggerNode("icc sindragosa chilled to the bone",
        { new NextAction("icc sindragosa chilled to the bone", ACTION_RAID + 2) }));

    triggers.push_back(new TriggerNode("icc sindragosa mystic buffet",
        { new NextAction("icc sindragosa mystic buffet", ACTION_RAID + 3) }));

    triggers.push_back(new TriggerNode("icc sindragosa main tank mystic buffet",
        { new NextAction("taunt spell", ACTION_EMERGENCY + 3) }));

    triggers.push_back(new TriggerNode("icc sindragosa frost bomb",
        { new NextAction("icc sindragosa frost bomb", ACTION_RAID + 7) }));

    triggers.push_back(new TriggerNode("icc sindragosa tank swap position",
        { new NextAction("icc sindragosa tank swap position", ACTION_EMERGENCY + 2) }));

    //LICH KING
    triggers.push_back(new TriggerNode("icc lich king shadow trap",
        { new NextAction("icc lich king shadow trap", ACTION_RAID + 6) }));

    triggers.push_back(new TriggerNode("icc lich king necrotic plague",
        { new NextAction("icc lich king necrotic plague", ACTION_RAID + 3) }));

    triggers.push_back(new TriggerNode("icc lich king winter",
        { new NextAction("icc lich king winter", ACTION_RAID +5) }));

    triggers.push_back(new TriggerNode("icc lich king adds",
        { new NextAction("icc lich king adds", ACTION_RAID +2) }));
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

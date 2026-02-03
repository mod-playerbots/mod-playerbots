#include "NexusStrategy.h"
#include "CreateNextAction.h"
#include "NexusActions.h"
#include "NexusMultipliers.h"

void WotlkDungeonNexStrategy::InitTriggers(std::vector<TriggerNode*> &triggers)
{
    // Horde Commander (Alliance N)/Commander Kolurg (Alliance H)
    // or
    // Alliance Commander (Horde N)/Commander Stoutbeard (Horde H)
    triggers.push_back(
        new TriggerNode(
            "faction commander whirlwind",
            {
                CreateNextAction<MoveFromWhirlwindAction>(ACTION_MOVE + 5.0f)
            }
        )
    );
    // TODO: Handle fear? (tremor totems, fear ward etc.)

    // Grand Magus Telestra
    triggers.push_back(
        new TriggerNode(
            "telestra firebomb",
            {
                CreateNextAction<FirebombSpreadAction>(ACTION_MOVE + 5.0f)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "telestra split phase",
            {
                CreateNextAction<TelestraSplitTargetAction>(ACTION_RAID + 1.0f)
            }
        )
    );
    // TODO: Add priority interrupt on the frost split's Blizzard casts

    // Anomalus
    triggers.push_back(
        new TriggerNode(
            "chaotic rift",
            {
                CreateNextAction<ChaoticRiftTargetAction>(ACTION_RAID + 1.0f)
            }
        )
    );

    // Ormorok the Tree-Shaper
    // Tank trigger to stack inside boss. Can also add return action to prevent boss repositioning
    // if it becomes too much of a problem. He usually dies before he's up against a wall though
    triggers.push_back(
        new TriggerNode(
            "ormorok spikes",
            {
                CreateNextAction<DodgeSpikesAction>(ACTION_MOVE + 5.0f)
            }
        )
    );
    // Non-tank trigger to stack. Avoiding the spikes at range is.. harder than it seems.
    // TODO: This turns hunters into melee marshmallows, have not come up with a better solution yet
    triggers.push_back(
        new TriggerNode(
            "ormorok stack",
            {
                CreateNextAction<DodgeSpikesAction>(ACTION_MOVE + 5.0f)
            }
        )
    );
    // TODO: Add handling for spell reflect... best to spam low level/weak spells but don't want
    // to hardcode spells per class, might be difficult to dynamically generate this.
    // Will revisit if I find my altbots killing themselves in heroic, just heal through it for now

    // Keristrasza
    triggers.push_back(
        new TriggerNode(
            "intense cold",
            {
                CreateNextAction<IntenseColdJumpAction>(ACTION_MOVE + 5.0f)
            }
        )
    );
    // Flank dragon positioning
    triggers.push_back(
        new TriggerNode(
            "keristrasza positioning",
            {
                CreateNextAction<RearFlankAction>(ACTION_MOVE + 4.0f)
            }
        )
    );
    // TODO: Add frost resist aura for paladins?
}

void WotlkDungeonNexStrategy::InitMultipliers(std::vector<Multiplier*> &multipliers)
{
    multipliers.push_back(new FactionCommanderMultiplier(botAI));
    multipliers.push_back(new TelestraMultiplier(botAI));
    multipliers.push_back(new AnomalusMultiplier(botAI));
    multipliers.push_back(new OrmorokMultiplier(botAI));
}

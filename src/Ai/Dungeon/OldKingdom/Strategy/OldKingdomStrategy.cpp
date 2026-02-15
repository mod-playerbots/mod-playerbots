#include "OldKingdomStrategy.h"
#include "CreateNextAction.h"
#include "OldKingdomActions.h"
#include "OldKingdomMultipliers.h"

void WotlkDungeonOKStrategy::InitTriggers(std::vector<TriggerNode*> &triggers)
{
    // Elder Nadox
    triggers.push_back(
        new TriggerNode(
            "nadox guardian",
            {
                CreateNextAction<AttackNadoxGuardianAction>(ACTION_RAID + 5.0f)
            }
        )
    );

    // Prince Taldaram
    // Flame Orb spawns in melee, doesn't have a clear direction until it starts moving.
    // Maybe not worth trying to avoid and just heal through. Only consideration is not to have ranged
    // players anywhere near melee when it spawns

    // Jedoga Shadowseeker
    triggers.push_back(
        new TriggerNode(
            "jedoga volunteer",
            {
                CreateNextAction<AttackJedogaVolunteerAction>(ACTION_RAID + 5.0f)
            }
        )
    );

    // Herald Volazj
    // Trash mobs before him have a big telegraphed shadow crash spell,
    // this can be avoided and is intended to be dodged
    triggers.push_back(
        new TriggerNode(
            "shadow crash",
            {
                CreateNextAction<AvoidShadowCrashAction>(ACTION_MOVE + 5.0f)
            }
        )
    );
    // Volazj is not implemented properly in AC, insanity phase does nothing.

    // Amanitar (Heroic Only)
    // TODO: once I get to heroics
}

void WotlkDungeonOKStrategy::InitMultipliers(std::vector<Multiplier*> &multipliers)
{
    multipliers.push_back(new ElderNadoxMultiplier(botAI));
    multipliers.push_back(new JedogaShadowseekerMultiplier(botAI));
    multipliers.push_back(new ForgottenOneMultiplier(botAI));
}

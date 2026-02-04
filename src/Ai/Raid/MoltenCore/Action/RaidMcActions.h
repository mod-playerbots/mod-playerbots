#ifndef _PLAYERBOT_RAIDMCACTIONS_H
#define _PLAYERBOT_RAIDMCACTIONS_H

#include "AttackAction.h"
#include "BossAuraActions.h"
#include "MovementActions.h"
#include "PlayerbotAI.h"

class LucifronShadowResistanceAction : public BossShadowResistanceAction
{
public:
    LucifronShadowResistanceAction(PlayerbotAI* botAI)
        : BossShadowResistanceAction(botAI, "lucifron")
    {
    }
};

class MagmadarFireResistanceAction : public BossFireResistanceAction
{
public:
    MagmadarFireResistanceAction(PlayerbotAI* botAI)
        : BossFireResistanceAction(botAI, "magmadar")
    {
    }
};

class GehennasShadowResistanceAction : public BossShadowResistanceAction
{
public:
    GehennasShadowResistanceAction(PlayerbotAI* botAI)
        : BossShadowResistanceAction(botAI, "gehennas")
    {
    }
};

class GarrFireResistanceAction : public BossFireResistanceAction
{
public:
    GarrFireResistanceAction(PlayerbotAI* botAI)
        : BossFireResistanceAction(botAI, "garr")
    {
    }
};

class BaronGeddonFireResistanceAction : public BossFireResistanceAction
{
public:
    BaronGeddonFireResistanceAction(PlayerbotAI* botAI)
        : BossFireResistanceAction(botAI, "baron geddon")
    {
    }
};

class SulfuronHarbingerFireResistanceAction : public BossFireResistanceAction
{
public:
    SulfuronHarbingerFireResistanceAction(PlayerbotAI* botAI)
        : BossFireResistanceAction(botAI, "sulfuron harbinger")
    {
    }
};

class GolemaggTheIncineratorFireResistanceAction : public BossFireResistanceAction
{
public:
    GolemaggTheIncineratorFireResistanceAction(PlayerbotAI* botAI)
        : BossFireResistanceAction(botAI, "golemagg the incinerator")
    {
    }
};

class MajordomoExecutusShadowResistanceAction : public BossShadowResistanceAction
{
public:
    MajordomoExecutusShadowResistanceAction(PlayerbotAI* botAI)
        : BossShadowResistanceAction(botAI, "majordomo executus")
    {
    }
};

class RagnarosFireResistanceAction : public BossFireResistanceAction
{
public:
    RagnarosFireResistanceAction(PlayerbotAI* botAI)
        : BossFireResistanceAction(botAI, "ragnaros")
    {
    }
};

class McMoveFromGroupAction : public MovementAction
{
public:
    McMoveFromGroupAction(PlayerbotAI* botAI, std::string const name = "mc move from group")
        : MovementAction(botAI, name) {}
    bool Execute(Event event) override;
};

class McMoveFromBaronGeddonAction : public MovementAction
{
public:
    McMoveFromBaronGeddonAction(PlayerbotAI* botAI, std::string const name = "mc move from baron geddon")
        : MovementAction(botAI, name) {}
    bool Execute(Event event) override;
};

class McShazzrahMoveAwayAction : public MovementAction
{
public:
    McShazzrahMoveAwayAction(PlayerbotAI* botAI, std::string const name = "mc shazzrah move away")
        : MovementAction(botAI, name) {}
    bool Execute(Event event) override;
};

class McGolemaggMarkBossAction : public Action
{
public:
    McGolemaggMarkBossAction(PlayerbotAI* botAI, std::string const name = "mc golemagg mark boss")
        : Action(botAI, name) {};
    bool Execute(Event event) override;
};

class McGolemaggTankAction : public AttackAction
{
public:
    McGolemaggTankAction(PlayerbotAI* botAI, std::string const name)
        : AttackAction(botAI, name) {}
protected:
    bool MoveUnitToPosition(Unit* target, const Position& tankPosition, float maxDistance, float stepDistance = 3.0f);
    bool FindCoreRagers(Unit*& coreRager1, Unit*& coreRager2) const;
};

class McGolemaggMainTankAttackGolemaggAction : public McGolemaggTankAction
{
public:
    McGolemaggMainTankAttackGolemaggAction(PlayerbotAI* botAI, std::string const name = "mc golemagg main tank attack golemagg")
        : McGolemaggTankAction(botAI, name) {};
    bool Execute(Event event) override;
};

class McGolemaggAssistTankAttackCoreRagerAction : public McGolemaggTankAction
{
public:
    McGolemaggAssistTankAttackCoreRagerAction(PlayerbotAI* botAI, std::string const name = "mc golemagg assist tank attack core rager")
        : McGolemaggTankAction(botAI, name) {};
    bool Execute(Event event) override;
};

#endif

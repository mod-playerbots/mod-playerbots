/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_RST_H
#define PLAYERBOTS_RST_H

#include "PlayerbotAI.h"
#include "Playerbots.h"
#include "Trigger.h"

inline constexpr uint32 RS_MAP_RUBY_SANCTUM = 724;

enum CreatureIdsRS
{

    NPC_BALTHARUS_THE_WARBORN           = 39751,
    NPC_BALTHARUS_THE_WARBORN_CLONE     = 39899,

    NPC_SAVIANA_RAGEFIRE                = 39747,

    NPC_GENERAL_ZARITHRIAN              = 39746,
    NPC_ONYX_FLAMECALLER                = 39814,

    NPC_CHARSCALE_INVOKER               = 40417,
    NPC_CHARSCALE_INVOKER_H             = 40418,
    NPC_CHARSCALE_ASSAULTER             = 40419,
    NPC_CHARSCALE_ASSAULTER_H           = 40420,
    NPC_CHARSCALE_ELITE                 = 40421,
    NPC_CHARSCALE_ELITE_H               = 40422,
    NPC_CHARSCALE_COMMANDER             = 40423,
    NPC_CHARSCALE_COMMANDER_H           = 40424,

    NPC_HALION                          = 39863,
    NPC_TWILIGHT_HALION                 = 40142,
    NPC_METEOR_STRIKE_MARK              = 40029,
    NPC_COMBUSTION                      = 40001,
    NPC_LIVING_INFERNO                  = 40681,
    NPC_LIVING_EMBER                    = 40683,
    NPC_ORB_CARRIER                     = 40081,
    NPC_CONSUMPTION                     = 40135
};

enum GameObjectIdsRS
{
    GO_HALION_PORTAL_1                  = 202794,
    GO_HALION_PORTAL_2                  = 202795,
    GO_HALION_PORTAL_3                  = 193988,
    GO_HALION_PORTAL_4                  = 202796
};

enum SpellIdsRS
{

    SPELL_ENERVATING_BRAND              = 74502,

    SPELL_CONFLAGRATION                 = 74452,
    SPELL_FLAME_BEACON                  = 74453,

    RS_SPELL_TAUNT_WARRIOR              = 355,
    RS_SPELL_TAUNT_PALADIN              = 62124,
    RS_SPELL_TAUNT_DK                   = 56222,
    RS_SPELL_TAUNT_DRUID                = 6795,
    RS_SPELL_PAIN_SUPPRESION            = 69910,
    RS_SPELL_MAGIC_BARRIER              = 38112,

    SPELL_FLAME_BREATH                  = 74525,
    SPELL_FLAME_BREATH_ALT1             = 68970,
    SPELL_FLAME_BREATH_ALT2             = 74403,
    SPELL_FLAME_BREATH_ALT3             = 74404,
    SPELL_FLAME_BREATH_ALT4             = 74526,
    SPELL_FLAME_BREATH_ALT5             = 74527,
    SPELL_FLAME_BREATH_ALT6             = 74528,
    SPELL_CLEAVE                        = 74524,
    SPELL_TAIL_LASH                     = 74531,

    SPELL_FIERY_COMBUSTION              = 74562,
    SPELL_MARK_OF_COMBUSTION            = 74567,

    SPELL_METEOR_STRIKE_TARGETING       = 74638,
    SPELL_METEOR_STRIKE                 = 74637,

    SPELL_TWILIGHT_PHASING              = 74808,

    SPELL_TWILIGHT_REALM                = 74807,
    SPELL_DARK_BREATH                   = 74806,
    SPELL_DARK_BREATH_25N               = 75954,
    SPELL_DARK_BREATH_10H               = 75955,
    SPELL_DARK_BREATH_25H               = 75956,
    SPELL_SOUL_CONSUMPTION              = 74792,
    SPELL_MARK_OF_CONSUMPTION           = 74795,
    SPELL_TWILIGHT_CUTTER               = 74768,
    SPELL_TWILIGHT_PULSE_PERIODIC       = 78861,

    RS_SPELL_NITRO_BOOSTS               = 54861,
    RS_SPELL_EMPOWERED_BLOOD            = 70227
};

class RsBaltharusBrandTrigger : public Trigger
{
public:
    RsBaltharusBrandTrigger(PlayerbotAI* botAI) : Trigger(botAI, "rs baltharus brand") {}
    bool IsActive() override;
};

class RsBaltharusTankPositionTrigger : public Trigger
{
public:
    RsBaltharusTankPositionTrigger(PlayerbotAI* botAI) : Trigger(botAI, "rs baltharus tank position") {}
    bool IsActive() override;
};

class RsBaltharusHealerPositionTrigger : public Trigger
{
public:
    RsBaltharusHealerPositionTrigger(PlayerbotAI* botAI) : Trigger(botAI, "rs baltharus healer position") {}
    bool IsActive() override;
};

class RsBaltharusAvoidFrontTrigger : public Trigger
{
public:
    RsBaltharusAvoidFrontTrigger(PlayerbotAI* botAI) : Trigger(botAI, "rs baltharus avoid front") {}
    bool IsActive() override;
};

class RsSavianaConflagrationTrigger : public Trigger
{
public:
    RsSavianaConflagrationTrigger(PlayerbotAI* botAI) : Trigger(botAI, "rs saviana conflagration") {}
    bool IsActive() override;
};

class RsSavianaTankPositionTrigger : public Trigger
{
public:
    RsSavianaTankPositionTrigger(PlayerbotAI* botAI) : Trigger(botAI, "rs saviana tank position") {}
    bool IsActive() override;
};

class RsSavianaAvoidFrontTrigger : public Trigger
{
public:
    RsSavianaAvoidFrontTrigger(PlayerbotAI* botAI) : Trigger(botAI, "rs saviana avoid front") {}
    bool IsActive() override;
};

class RsSavianaMeleeSpreadTrigger : public Trigger
{
public:
    RsSavianaMeleeSpreadTrigger(PlayerbotAI* botAI) : Trigger(botAI, "rs saviana melee spread") {}
    bool IsActive() override;
};

class RsZarithrianAddsTrigger : public Trigger
{
public:
    RsZarithrianAddsTrigger(PlayerbotAI* botAI) : Trigger(botAI, "rs zarithrian adds") {}
    bool IsActive() override;
};

class RsZarithrianTankTrigger : public Trigger
{
public:
    RsZarithrianTankTrigger(PlayerbotAI* botAI) : Trigger(botAI, "rs zarithrian tank") {}
    bool IsActive() override;
};

class RsHalionTankPositionTrigger : public Trigger
{
public:
    RsHalionTankPositionTrigger(PlayerbotAI* botAI) : Trigger(botAI, "rs halion tank position") {}
    bool IsActive() override;
};

class RsHalionAvoidConesTrigger : public Trigger
{
public:
    RsHalionAvoidConesTrigger(PlayerbotAI* botAI) : Trigger(botAI, "rs halion avoid cones") {}
    bool IsActive() override;
};

class RsHalionCombustionTrigger : public Trigger
{
public:
    RsHalionCombustionTrigger(PlayerbotAI* botAI) : Trigger(botAI, "rs halion combustion") {}
    bool IsActive() override;
};

class RsHalionMeteorTrigger : public Trigger
{
public:
    RsHalionMeteorTrigger(PlayerbotAI* botAI) : Trigger(botAI, "rs halion meteor") {}
    bool IsActive() override;
};

class RsHalionAddsTrigger : public Trigger
{
public:
    RsHalionAddsTrigger(PlayerbotAI* botAI) : Trigger(botAI, "rs halion adds") {}
    bool IsActive() override;
};

class RsHalionAddTankTrigger : public Trigger
{
public:
    RsHalionAddTankTrigger(PlayerbotAI* botAI) : Trigger(botAI, "rs halion add tank") {}
    bool IsActive() override;
};

class RsHalionStartPositionTrigger : public Trigger
{
public:
    RsHalionStartPositionTrigger(PlayerbotAI* botAI) : Trigger(botAI, "rs halion start position") {}
    bool IsActive() override;
};

class RsHalionEnterPortalTrigger : public Trigger
{
public:
    RsHalionEnterPortalTrigger(PlayerbotAI* botAI) : Trigger(botAI, "rs halion enter portal") {}
    bool IsActive() override;
};

class RsHalionP2TankPositionTrigger : public Trigger
{
public:
    RsHalionP2TankPositionTrigger(PlayerbotAI* botAI) : Trigger(botAI, "rs halion p2 tank position") {}
    bool IsActive() override;
};

class RsHalionP2AvoidConesTrigger : public Trigger
{
public:
    RsHalionP2AvoidConesTrigger(PlayerbotAI* botAI) : Trigger(botAI, "rs halion p2 avoid cones") {}
    bool IsActive() override;
};

class RsHalionConsumptionTrigger : public Trigger
{
public:
    RsHalionConsumptionTrigger(PlayerbotAI* botAI) : Trigger(botAI, "rs halion consumption") {}
    bool IsActive() override;
};

class RsHalionCutterTrigger : public Trigger
{
public:
    RsHalionCutterTrigger(PlayerbotAI* botAI) : Trigger(botAI, "rs halion cutter") {}
    bool IsActive() override;
};

class RsHalionHealConsumptionTrigger : public Trigger
{
public:
    RsHalionHealConsumptionTrigger(PlayerbotAI* botAI) : Trigger(botAI, "rs halion heal consumption") {}
    bool IsActive() override;
};

class RsTrashAddsTrigger : public Trigger
{
public:
    RsTrashAddsTrigger(PlayerbotAI* botAI) : Trigger(botAI, "rs trash adds") {}
    bool IsActive() override;
};

class RsTrashMainTankTrigger : public Trigger
{
public:
    RsTrashMainTankTrigger(PlayerbotAI* botAI) : Trigger(botAI, "rs trash main tank") {}
    bool IsActive() override;
};

class RsTrashAssistTankTrigger : public Trigger
{
public:
    RsTrashAssistTankTrigger(PlayerbotAI* botAI) : Trigger(botAI, "rs trash assist tank") {}
    bool IsActive() override;
};

class RsTrashRangedTrigger : public Trigger
{
public:
    RsTrashRangedTrigger(PlayerbotAI* botAI) : Trigger(botAI, "rs trash ranged") {}
    bool IsActive() override;
};

class RsTrashMeleeFlankTrigger : public Trigger
{
public:
    RsTrashMeleeFlankTrigger(PlayerbotAI* botAI) : Trigger(botAI, "rs trash melee flank") {}
    bool IsActive() override;
};

#endif

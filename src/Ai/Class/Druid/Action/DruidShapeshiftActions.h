/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_DRUIDSHAPESHIFTACTIONS_H
#define _PLAYERBOT_DRUIDSHAPESHIFTACTIONS_H

#include "GenericSpellActions.h"

class PlayerbotAI;

class CastBearFormAction : public CastBuffSpellAction
{
public:
    CastBearFormAction(PlayerbotAI* botAI) : CastBuffSpellAction(botAI, "bear form") {}

    bool isUseful() override;
    bool isPossible() override;
};

class CastDireBearFormAction : public CastBuffSpellAction
{
public:
    CastDireBearFormAction(PlayerbotAI* botAI) : CastBuffSpellAction(botAI, "dire bear form") {}

    std::vector<NextAction> getAlternatives() override;
};

class CastCatFormAction : public CastBuffSpellAction
{
public:
    CastCatFormAction(PlayerbotAI* botAI) : CastBuffSpellAction(botAI, "cat form") {}
};

class CastTreeFormAction : public CastBuffSpellAction
{
public:
    CastTreeFormAction(PlayerbotAI* botAI) : CastBuffSpellAction(botAI, "tree of life") {}

    bool isUseful() override;
};

class CastMoonkinFormAction : public CastBuffSpellAction
{
public:
    CastMoonkinFormAction(PlayerbotAI* botAI) : CastBuffSpellAction(botAI, "moonkin form") {}
};

class CastAquaticFormAction : public CastBuffSpellAction
{
public:
    CastAquaticFormAction(PlayerbotAI* botAI) : CastBuffSpellAction(botAI, "aquatic form") {}
};

class CastTravelFormAction : public CastBuffSpellAction
{
public:
    CastTravelFormAction(PlayerbotAI* botAI) : CastBuffSpellAction(botAI, "travel form") {}

    bool isUseful() override;
};

class CastCasterFormAction : public CastBuffSpellAction
{
public:
    CastCasterFormAction(PlayerbotAI* botAI) : CastBuffSpellAction(botAI, "caster form") {}

    bool Execute(Event event) override;
    bool isUseful() override;
    bool isPossible() override { return true; }
};

class CastCancelDruidAction : public CastBuffSpellAction
{
public:
    CastCancelDruidAction(PlayerbotAI* botAI, std::string const& actionName, std::string const& auraName, uint32 auraId)
        : CastBuffSpellAction(botAI, actionName), auraName(auraName), auraId(auraId)
    {
    }

    bool Execute(Event event) override;
    bool isUseful() override;
    bool isPossible() override { return true; }

private:
    std::string auraName;
    uint32 auraId;
};

class CastCancelTreeFormAction : public CastCancelDruidAction
{
public:
    CastCancelTreeFormAction(PlayerbotAI* botAI)
        : CastCancelDruidAction(botAI, "cancel tree form", "tree of life", 33891)
    {
    }
};

class CastCancelTravelFormAction : public CastCancelDruidAction
{
public:
    CastCancelTravelFormAction(PlayerbotAI* botAI)
        : CastCancelDruidAction(botAI, "cancel travel form", "travel form", 783)
    {
    }
};

class CastCancelBearFormAction : public CastCancelDruidAction
{
public:
    CastCancelBearFormAction(PlayerbotAI* botAI) : CastCancelDruidAction(botAI, "cancel bear form", "bear form", 5487) {}
};

class CastCancelDireBearFormAction : public CastCancelDruidAction
{
public:
    CastCancelDireBearFormAction(PlayerbotAI* botAI)
        : CastCancelDruidAction(botAI, "cancel dire bear form", "dire bear form", 9634)
    {
    }
};

class CastCancelCatFormAction : public CastCancelDruidAction
{
public:
    CastCancelCatFormAction(PlayerbotAI* botAI) : CastCancelDruidAction(botAI, "cancel cat form", "cat form", 768) {}
};

class CastCancelMoonkinFormAction : public CastCancelDruidAction
{
public:
    CastCancelMoonkinFormAction(PlayerbotAI* botAI)
        : CastCancelDruidAction(botAI, "cancel moonkin form", "moonkin form", 24858)
    {
    }
};

class CastCancelAquaticFormAction : public CastCancelDruidAction
{
public:
    CastCancelAquaticFormAction(PlayerbotAI* botAI)
        : CastCancelDruidAction(botAI, "cancel aquatic form", "aquatic form", 1066)
    {
    }
};

#endif

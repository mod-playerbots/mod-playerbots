/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_ICCMULTIPLIERS_H
#define PLAYERBOTS_ICCMULTIPLIERS_H

#include "Multiplier.h"

//Lady Deathwhisper
class IccLadyDeathwhisperMultiplier : public Multiplier
{
public:
    IccLadyDeathwhisperMultiplier(PlayerbotAI* ai) : Multiplier(ai, "icc lady deathwhisper") {}
    float GetValue(Action* action) override;
};

//DBS
class IccAddsDbsMultiplier : public Multiplier
{
public:
    IccAddsDbsMultiplier(PlayerbotAI* ai) : Multiplier(ai, "icc adds dbs") {}
    float GetValue(Action* action) override;
};

//DOGS

class IccDogsMultiplier : public Multiplier
{
public:
    IccDogsMultiplier(PlayerbotAI* ai) : Multiplier(ai, "icc dogs") {}
    float GetValue(Action* action) override;
};

//FESTERGUT
class IccFestergutMultiplier : public Multiplier
{
public:
    IccFestergutMultiplier(PlayerbotAI* ai) : Multiplier(ai, "icc festergut") {}
    float GetValue(Action* action) override;
};

//ROTFACE
class IccRotfaceMultiplier : public Multiplier
{
public:
    IccRotfaceMultiplier(PlayerbotAI* ai) : Multiplier(ai, "icc rotface") {}
    float GetValue(Action* action) override;
};

/*class IccRotfaceGroupPositionMultiplier : public Multiplier
{
public:
    IccRotfaceGroupPositionMultiplier(PlayerbotAI* ai) : Multiplier(ai, "icc rotface group position") {}
    float GetValue(Action* action) override;
};*/

//PP
class IccAddsPutricideMultiplier : public Multiplier
{
public:
    IccAddsPutricideMultiplier(PlayerbotAI* ai) : Multiplier(ai, "icc adds putricide") {}
    float GetValue(Action* action) override;
};

//BPC
class IccBpcAssistMultiplier : public Multiplier
{
public:
    IccBpcAssistMultiplier(PlayerbotAI* botAI) : Multiplier(botAI, "icc bpc assist") {}
    float GetValue(Action* action) override;
};

//BQL
class IccBqlMultiplier : public Multiplier
{
public:
    IccBqlMultiplier(PlayerbotAI* botAI) : Multiplier(botAI, "icc bql multiplier") {}
    float GetValue(Action* action) override;
};

//VDW
class IccValithriaDreamCloudMultiplier : public Multiplier
{
public:
    IccValithriaDreamCloudMultiplier(PlayerbotAI* ai) : Multiplier(ai, "icc valithria dream cloud") {}
    float GetValue(Action* action) override;
};

//SINDRAGOSA
class IccSindragosaMultiplier : public Multiplier
{
public:
    IccSindragosaMultiplier(PlayerbotAI* ai) : Multiplier(ai, "icc sindragosa") {}
    float GetValue(Action* action) override;
};

//LK
class IccLichKingAddsMultiplier : public Multiplier
{
public:
    IccLichKingAddsMultiplier(PlayerbotAI* ai) : Multiplier(ai, "icc lich king adds") {}
    float GetValue(Action* action) override;
};

class IccLichKingSpiritBombMultiplier : public Multiplier
{
public:
    IccLichKingSpiritBombMultiplier(PlayerbotAI* ai) : Multiplier(ai, "icc lich king spirit bomb") {}
    float GetValue(Action* action) override;
};

//GUNSHIP
class IccGunshipMultiplier : public Multiplier
{
public:
    IccGunshipMultiplier(PlayerbotAI* ai) : Multiplier(ai, "icc gunship") {}
    float GetValue(Action* action) override;
};

#endif

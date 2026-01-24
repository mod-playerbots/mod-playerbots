/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#include "BotRepositoryAdapter.h"

#include "PlayerbotRepository.h"

void BotRepositoryAdapter::Save(PlayerbotAI* botAI)
{
    sPlayerbotRepository->Save(botAI);
}

void BotRepositoryAdapter::Load(PlayerbotAI* botAI)
{
    sPlayerbotRepository->Load(botAI);
}

void BotRepositoryAdapter::Reset(PlayerbotAI* botAI)
{
    sPlayerbotRepository->Reset(botAI);
}

bool BotRepositoryAdapter::HasSavedData(uint32 /*guid*/)
{
    // PlayerbotRepository doesn't expose this directly
    // This would need to be added to PlayerbotRepository in a future refactor
    return false;
}

std::string BotRepositoryAdapter::GetSavedValue(uint32 /*guid*/, std::string const& /*key*/)
{
    // PlayerbotRepository doesn't expose individual value access
    // This would need to be added to PlayerbotRepository in a future refactor
    return "";
}

void BotRepositoryAdapter::SetSavedValue(uint32 /*guid*/, std::string const& /*key*/, std::string const& /*value*/)
{
    // PlayerbotRepository doesn't expose individual value access
    // This would need to be added to PlayerbotRepository in a future refactor
}

void BotRepositoryAdapter::DeleteSavedData(uint32 /*guid*/)
{
    // PlayerbotRepository doesn't expose this directly
    // This would need to be added to PlayerbotRepository in a future refactor
}

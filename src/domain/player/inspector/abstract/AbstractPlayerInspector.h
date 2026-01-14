#pragma once

#include <cstdint>

#include "ObjectGuid.h"
#include "ObjectAccessor.h"
#include "SharedDefines.h"
#include "Player.h"

#include "AbstractInspector.h"

class AbstractPlayerInspector : public AbstractInspector
{
public:
    AbstractPlayerInspector(
		uint64_t playerGUID
	) :
	playerGUID(playerGUID)
	{}

	AbstractPlayerInspector& operator=(AbstractPlayerInspector const&) = delete;

	Player* getCurrentPlayer() const
	{
		const ObjectGuid playerFullGUID = ObjectGuid::Create<HighGuid::Player>(this->playerGUID);
		Player* const player = ObjectAccessor::FindPlayer(playerFullGUID);

		return player;
	}

	bool isInspectable() const override
	{
		return true;
	}

protected:
	const uint64_t playerGUID;
};

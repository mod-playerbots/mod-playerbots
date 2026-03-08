/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#include "TaxiAction.h"

#include "Event.h"
#include "LastMovementValue.h"
#include "PlayerbotAIConfig.h"
#include "PlayerbotAI.h"
#include "AiObjectContext.h"

// @TODO: Refactor this to a more maintainable structure
bool TaxiAction::Execute(Event event)
{
    this->botAI->RemoveShapeshift();

    Value<LastMovementValue>* lastMovementValue = this->context->GetValue<LastMovementValue>("last taxi");

    if (lastMovementValue == nullptr)
    {
        this->botAI->TellError("I don't know where to fly");

        return false;
    }

    LastMovement& movement = lastMovementValue->Get();

    WorldPacket& packet = event.getPacket();
    const std::string& param = event.getParam();

    if ((!packet.empty() && (packet.GetOpcode() == CMSG_TAXICLEARALLNODES || packet.GetOpcode() == CMSG_TAXICLEARNODE)) || param == "clear")
    {
        movement.taxiNodes.clear();
        movement.Set(nullptr);

        this->botAI->TellMaster("I am ready for the next flight");

        return true;
    }

    Value<GuidVector>* nearestNpcsValue = this->context->GetValue<GuidVector>("nearest npcs");

    if (nearestNpcsValue == nullptr)
    {
        this->botAI->TellError("I don't see any NPCs around");

        return false;
    }

    GuidVector units = nearestNpcsValue->Get();

    const PlayerbotAIConfig& configuration = PlayerbotAIConfig::instance();

    for (const ObjectGuid guid : units)
    {
        Creature* const npc = ObjectAccessor::GetCreature(*this->bot, guid);

        if (npc == nullptr || !npc->IsAlive())
        {
            continue;
        }

        if (!(npc->GetNpcFlags() & UNIT_NPC_FLAG_FLIGHTMASTER))
        {
            continue;
        }

        if (this->bot->GetDistance(npc) > configuration.farDistance)
        {
            continue;
        }

        ObjectMgr* const objectMgr = ObjectMgr::instance();

        if (objectMgr == nullptr)
        {
            this->botAI->TellError("Cannot access ObjectMgr");

            return false;
        }

        const uint32_t curloc = objectMgr->GetNearestTaxiNode(
            npc->GetPositionX(),
            npc->GetPositionY(),
            npc->GetPositionZ(),
        npc->GetMapId(),
        this->bot->GetTeamId()
    );

        std::vector<uint32_t> nodes{};

        for (uint32_t i = 0; i < sTaxiPathStore.GetNumRows(); ++i)
        {
            const TaxiPathEntry* entry = sTaxiPathStore.LookupEntry(i);

            if (entry == nullptr)
            {
                continue;
            }

            if (entry->from != curloc)
            {
                continue;
            }

            const uint8_t field = uint8_t((i - 1) / 32);

            if (field >= TaxiMaskSize)
            {
                continue;
            }

            nodes.push_back(i);
        }

        // Only for follower bots
        if (this->botAI->HasRealPlayerMaster())
        {
            const uint32_t index = this->botAI->GetGroupSlotIndex(this->bot);
            uint32_t delay = configuration.botTaxiDelayMin + index * configuration.botTaxiGapMs + urand(0, configuration.botTaxiGapJitterMs);

            delay = std::min(delay, configuration.botTaxiDelayMax);

            // Store the NPC's GUID so we can re-acquire the pointer later
            const ObjectGuid npcGuid = npc->GetGUID();

            // schedule the take-off
            this->botAI->AddTimedEvent(
                [bot = bot, &movement, npcGuid]() -> void
                {
                    Creature* const npcPtr = ObjectAccessor::GetCreature(*bot, npcGuid);

                    if (npcPtr == nullptr)
                    {
                        return;
                    }

                    if (movement.taxiNodes.empty())
                    {
                        return;
                    }

                    bot->ActivateTaxiPathTo(movement.taxiNodes, npcPtr, 0);
                },
                delay
            );

            this->botAI->SetNextCheckDelay(delay + 50);

            return true;
        }

        if (param == "?")
        {
            this->botAI->TellMasterNoFacing("=== Taxi ===");

            uint32_t index = 1;

            for (uint32_t node : nodes)
            {
                const TaxiPathEntry* const entry = sTaxiPathStore.LookupEntry(node);

                if (entry == nullptr)
                {
                    continue;
                }

                const TaxiNodesEntry* const dest = sTaxiNodesStore.LookupEntry(entry->to);

                if (dest == nullptr)
                {
                    continue;
                }

                std::ostringstream out{};
                out << index << ": " << dest->name[0];

                this->botAI->TellMasterNoFacing(out.str());

                ++index;
            }

            return true;
        }

        const uint32_t selected = atoi(param.c_str());

        if (selected != 0)
        {
            const uint32_t path = nodes[selected - 1];
            const TaxiPathEntry* const entry = sTaxiPathStore.LookupEntry(path);

            if (entry == nullptr)
            {
                return false;
            }

            return this->bot->ActivateTaxiPathTo({entry->from, entry->to}, npc, 0);
        }

        if (!movement.taxiNodes.empty() && !this->bot->ActivateTaxiPathTo(movement.taxiNodes, npc, 0))
        {
            movement.taxiNodes.clear();
            movement.Set(nullptr);
            this->botAI->TellError("I can't fly with you");

            return false;
        }

        return true;
    }

    this->botAI->TellError("Cannot find any flightmaster to talk");

    return false;
}

/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#include "ReactionEngine.h"

#include "Action.h"
#include "Event.h"
#include "PlayerbotAI.h"
#include "PlayerbotAIConfig.h"
#include "Playerbots.h"
#include "Queue.h"
#include "Strategy.h"

void Reaction::SetAction(Action* inAction)
{
    if (inAction)
    {
        SetDuration(inAction->GetDuration());
        action = inAction;
    }
}

bool Reaction::Update(uint32 elapsed)
{
    duration = duration > elapsed ? duration - elapsed : 0;
    return !IsActive();
}

ReactionEngine::ReactionEngine(PlayerbotAI* botAI, AiObjectContext* factory)
    : Engine(botAI, factory), aiReactionUpdateDelay(0U)
{
}

void ReactionEngine::Init()
{
    Reset();

    hasTargetExclusions = false;
    for (auto& pair : strategies)
    {
        Strategy* strategy = pair.second;
        strategyTypeMask |= strategy->GetType();
        hasTargetExclusions |= strategy->HasTargetExclusions();
        strategy->InitReactionMultipliers(multipliers);
        strategy->InitReactionTriggers(triggers);
        for (auto& iter : strategy->actionNodeFactories.creators)
        {
            actionNodeFactories.creators[iter.first] = iter.second;
        }
    }
}

bool ReactionEngine::FindReaction(bool isStunned)
{
    if (!IsReacting())
    {
        aiObjectContext->Update();

        botAI->HandleCommands();

        ProcessTriggers(false);

        ActionBasket* reactionItem = nullptr;

        int iterations = 0;
        int iterationsPerTick = queue.Size() * sPlayerbotAIConfig.iterationsPerTick;
        do
        {
            reactionItem = queue.Peek();
            if (reactionItem)
            {
                bool const skipReactionPrerequisites = reactionItem->isSkipPrerequisites();
                float reactionRelevance = reactionItem->getRelevance();
                Event const reactionEvent = reactionItem->getEvent();

                ActionNode* reactionNode = queue.Pop(reactionItem);
                if (reactionNode)
                {
                    Action* reaction = InitializeAction(reactionNode);
                    if (reaction)
                    {
                        reaction->setRelevance(reactionRelevance);

                        if (reaction->isUseful() && (!isStunned || reaction->isUsefulWhenStunned()))
                        {
                            for (Multiplier* multiplier : multipliers)
                            {
                                reactionRelevance *= multiplier->GetValue(reaction);
                                reaction->setRelevance(reactionRelevance);
                                if (reactionRelevance <= 0.0f)
                                    break;
                            }

                            if (!skipReactionPrerequisites)
                            {
                                if (MultiplyAndPush(reactionNode->getPrerequisites(), reactionRelevance + 0.02f,
                                                    false, reactionEvent, "prereq"))
                                {
                                    PushAgain(reactionNode, reactionRelevance + 0.01f, reactionEvent);
                                    continue;
                                }
                            }

                            if ((reactionRelevance > 0.0f) && reaction->isPossible())
                            {
                                incomingReaction.SetAction(reaction);
                                incomingReaction.SetEvent(reactionEvent);
                                delete reactionNode;
                                break;
                            }
                            else
                            {
                                MultiplyAndPush(reactionNode->getAlternatives(), reactionRelevance + 0.03f,
                                                false, reactionEvent, "alt");
                            }
                        }
                    }

                    delete reactionNode;
                }
            }
        } while (reactionItem && ++iterations <= iterationsPerTick);

        queue.RemoveExpired();

        return incomingReaction.IsValid();
    }

    return false;
}

bool ReactionEngine::StartReaction()
{
    bool reactionExecuted = false;
    if (incomingReaction.IsValid())
    {
        reactionExecuted = ListenAndExecute(incomingReaction.GetAction(), incomingReaction.GetEvent());
        if (reactionExecuted)
            ongoingReaction = incomingReaction;

        incomingReaction.Reset();
    }

    return reactionExecuted;
}

void ReactionEngine::StopReaction()
{
    ongoingReaction.Reset();
    aiReactionUpdateDelay = 0U;
}

bool ReactionEngine::Update(uint32 elapsed, bool minimal, bool isStunned, bool& reactionFound)
{
    aiReactionUpdateDelay = aiReactionUpdateDelay > elapsed ? aiReactionUpdateDelay - elapsed : 0U;

    reactionFound = false;
    bool reactionFinished = false;

    if (CanUpdateAIReaction())
    {
        if (IsReacting())
        {
            if (ongoingReaction.Update(elapsed))
            {
                StopReaction();
                reactionFinished = true;
            }
        }
        else
            reactionFinished = true;

        if (reactionFinished)
        {
            if (HasIncomingReaction())
            {
                StartReaction();
            }
            else
            {
                if (FindReaction(isStunned))
                    reactionFound = true;
            }
        }

        if (!HasIncomingReaction() && !IsReacting())
        {
            if (aiReactionUpdateDelay < sPlayerbotAIConfig.reactDelay)
                aiReactionUpdateDelay = minimal ? sPlayerbotAIConfig.reactDelay * 10 : sPlayerbotAIConfig.reactDelay;
        }
    }

    return HasIncomingReaction() || IsReacting();
}

bool ReactionEngine::ListenAndExecute(Action* action, Event event)
{
    bool actionExecuted = action->Execute(event);
    if (actionExecuted)
    {
        if (!incomingReaction.GetAction())
            incomingReaction.SetAction(action);

        botAI->SetActionDuration(action);
    }

    return actionExecuted;
}

Action* ReactionEngine::InitializeAction(ActionNode* actionNode)
{
    Action* action = actionNode->getAction();
    if (!action)
    {
        action = aiObjectContext->GetAction(actionNode->getName());
        actionNode->setAction(action);
    }

    if (action)
    {
        action->SetReaction(true);
        // Clear any duration left over from a previous execution; the real duration is
        // applied after Execute via SetActionDuration -> SetReactionDuration.
        action->ResetDuration();
    }

    return action;
}

void ReactionEngine::SetReactionDuration(Action const* action)
{
    if (action && (IsReacting() || HasIncomingReaction()))
    {
        if (ongoingReaction.GetAction() == action)
            ongoingReaction.SetDuration(action->GetDuration());
        else if (incomingReaction.GetAction() == action)
            incomingReaction.SetDuration(action->GetDuration());
    }
}

void ReactionEngine::ResetReactions()
{
    ongoingReaction.Reset();
    incomingReaction.Reset();
    aiReactionUpdateDelay = 0U;
}

bool ReactionEngine::CanUpdateAIReaction() const
{
    Player* bot = botAI->GetBot();
    return (aiReactionUpdateDelay < 100U) &&
           bot->IsInWorld() &&
           !bot->IsBeingTeleported();
}

Reaction const* ReactionEngine::GetReaction() const
{
    if (ongoingReaction.IsValid())
        return &ongoingReaction;

    if (incomingReaction.IsValid())
        return &incomingReaction;

    return nullptr;
}

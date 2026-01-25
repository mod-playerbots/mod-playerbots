/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "mocks/MockBotServices.h"

using ::testing::Return;
using ::testing::_;

/**
 * @brief Tests for BotChatService
 *
 * These tests verify the chat service functionality
 * for bot communication operations.
 */
class ChatServiceTest : public ::testing::Test
{
protected:
    MockChatService mockChatService;
};

TEST_F(ChatServiceTest, CanMockTellMaster)
{
    EXPECT_CALL(mockChatService, TellMaster(std::string("Hello, master!"), _))
        .WillOnce(Return(true));

    EXPECT_TRUE(mockChatService.TellMaster("Hello, master!"));
}

TEST_F(ChatServiceTest, CanMockTellMasterNoFacing)
{
    EXPECT_CALL(mockChatService, TellMasterNoFacing(std::string("Status update"), _))
        .WillOnce(Return(true));

    EXPECT_TRUE(mockChatService.TellMasterNoFacing("Status update"));
}

TEST_F(ChatServiceTest, CanMockTellError)
{
    EXPECT_CALL(mockChatService, TellError(std::string("Error occurred"), _))
        .WillOnce(Return(false));

    EXPECT_FALSE(mockChatService.TellError("Error occurred"));
}

TEST_F(ChatServiceTest, CanMockSayToGuild)
{
    EXPECT_CALL(mockChatService, SayToGuild(std::string("Guild message")))
        .WillOnce(Return(true));

    EXPECT_TRUE(mockChatService.SayToGuild("Guild message"));
}

TEST_F(ChatServiceTest, CanMockSayToWorld)
{
    EXPECT_CALL(mockChatService, SayToWorld(std::string("World message")))
        .WillOnce(Return(true));

    EXPECT_TRUE(mockChatService.SayToWorld("World message"));
}

TEST_F(ChatServiceTest, CanMockSayToChannel)
{
    EXPECT_CALL(mockChatService, SayToChannel(std::string("Channel message"), 1))
        .WillOnce(Return(true));

    EXPECT_TRUE(mockChatService.SayToChannel("Channel message", 1));
}

TEST_F(ChatServiceTest, CanMockSayToParty)
{
    EXPECT_CALL(mockChatService, SayToParty(std::string("Party message")))
        .WillOnce(Return(true));

    EXPECT_TRUE(mockChatService.SayToParty("Party message"));
}

TEST_F(ChatServiceTest, CanMockSayToRaid)
{
    EXPECT_CALL(mockChatService, SayToRaid(std::string("Raid message")))
        .WillOnce(Return(true));

    EXPECT_TRUE(mockChatService.SayToRaid("Raid message"));
}

TEST_F(ChatServiceTest, CanMockSay)
{
    EXPECT_CALL(mockChatService, Say(std::string("Hello")))
        .WillOnce(Return(true));

    EXPECT_TRUE(mockChatService.Say("Hello"));
}

TEST_F(ChatServiceTest, CanMockYell)
{
    EXPECT_CALL(mockChatService, Yell(std::string("Help!")))
        .WillOnce(Return(true));

    EXPECT_TRUE(mockChatService.Yell("Help!"));
}

TEST_F(ChatServiceTest, CanMockWhisper)
{
    EXPECT_CALL(mockChatService, Whisper(std::string("Secret message"), std::string("PlayerName")))
        .WillOnce(Return(true));

    EXPECT_TRUE(mockChatService.Whisper("Secret message", "PlayerName"));
}

TEST_F(ChatServiceTest, CanMockPlaySound)
{
    EXPECT_CALL(mockChatService, PlaySound(12345))
        .WillOnce(Return(true));

    EXPECT_TRUE(mockChatService.PlaySound(12345));
}

TEST_F(ChatServiceTest, CanMockPlayEmote)
{
    EXPECT_CALL(mockChatService, PlayEmote(67890))
        .WillOnce(Return(true));

    EXPECT_TRUE(mockChatService.PlayEmote(67890));
}

/**
 * @brief Tests for chat behavior patterns
 */
class ChatBehaviorPatternTest : public ::testing::Test
{
protected:
    MockChatService mockChatService;

    bool NotifyMasterAboutCombat(MockChatService& chatService, bool inCombat)
    {
        if (inCombat)
        {
            return chatService.TellMasterNoFacing("Entering combat!");
        }
        return chatService.TellMasterNoFacing("Combat ended.");
    }

    bool AnnounceToParty(MockChatService& chatService, std::string const& message)
    {
        return chatService.SayToParty(message);
    }
};

TEST_F(ChatBehaviorPatternTest, NotifyMasterEnteringCombat)
{
    EXPECT_CALL(mockChatService, TellMasterNoFacing(std::string("Entering combat!"), _))
        .WillOnce(Return(true));

    EXPECT_TRUE(NotifyMasterAboutCombat(mockChatService, true));
}

TEST_F(ChatBehaviorPatternTest, NotifyMasterCombatEnded)
{
    EXPECT_CALL(mockChatService, TellMasterNoFacing(std::string("Combat ended."), _))
        .WillOnce(Return(true));

    EXPECT_TRUE(NotifyMasterAboutCombat(mockChatService, false));
}

TEST_F(ChatBehaviorPatternTest, PartyAnnouncement)
{
    EXPECT_CALL(mockChatService, SayToParty(std::string("Ready to go!")))
        .WillOnce(Return(true));

    EXPECT_TRUE(AnnounceToParty(mockChatService, "Ready to go!"));
}

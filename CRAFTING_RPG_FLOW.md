# Crafting RPG flow

This file visualizes the currently live crafting-related behavior in `mod-playerbots`.

It covers two connected systems:

- **Public crafting reply flow**: a player asks in `General` or `Trade` for a crafted item and one eligible bot whispers back.
- **New RPG crafting flow**: random bots can now enter a dedicated `RPG_DO_CRAFT` state and perform profession work on their own.

## Mermaid overview

```mermaid
flowchart TB
    subgraph CHAT[Public crafting request flow]
        C1[Player posts linked item in General or Trade] --> C2[Chat hook receives channel message]
        C2 --> C3[RandomPlayerbotMgr HandleCommand]
        C3 --> C4{Crafting replies enabled}
        C4 -- no --> C99[Normal chat handling]
        C4 -- yes --> C5{Message contains linked item plus craft or make keyword}
        C5 -- no --> C99
        C5 -- yes --> C6[Scan online random bots in same channel]
        C6 --> C7[Reject bots outside resolved General or Trade source]
        C7 --> C8[Evaluate recipes professions and skill requirements]
        C8 --> C9[Compute owned and missing reagents]
        C9 --> C10[Choose one best-fit bot]
        C10 --> C11{Cooldown active for player and item}
        C11 -- yes --> C98[Suppress duplicate whisper]
        C11 -- no --> C12[Format reply with fee and mats status]
        C12 --> C13[Bot whispers player]
        C13 --> C14[Update crafting reply cooldown]
        C5 --> C15[Generic chatter path sees valid craft request]
        C15 --> C16[Suppress normal social reply]
    end

    subgraph RPG[New RPG crafting flow]
        R1[New RPG status update] --> R2{Current status is idle}
        R2 -- yes --> R3[Evaluate available statuses]
        R3 --> R4[Check DoCraft availability]
        R4 --> R5{Has recipe item craftable spell enchant target or disenchant target}
        R5 -- no --> R6[Do not include DoCraft]
        R5 -- yes --> R7[Add RPG_DO_CRAFT candidate]
        R7 --> R8[Apply RpgStatusProbWeight.DoCraft]
        R8 --> R9{DoCraft selected}
        R9 -- yes --> R10[ChangeToDoCraft]
        R10 --> R11[Run new rpg do craft action]

        R11 --> R12{Quest giver accept or reward needed}
        R12 -- yes --> R13[Do quest interaction first]
        R12 -- no --> R14{Bot is moving}
        R14 -- yes --> R15[Wait until stationary]
        R14 -- no --> R16[Try use random recipe]
        R16 --> R17{Recipe learned}
        R17 -- yes --> R11
        R17 -- no --> R18[Try craft random item]
        R18 --> R19{Craft succeeded}
        R19 -- yes --> R20[Increment crafted count]
        R20 --> R11
        R19 -- no --> R21[Try enchant random item]
        R21 --> R22{Enchant succeeded}
        R22 -- yes --> R20
        R22 -- no --> R23[Try disenchant random item]
        R23 --> R24{Disenchant succeeded}
        R24 -- yes --> R20
        R24 -- no --> R25[No useful crafting work left]
        R25 --> R26[Change status to idle]
    end

    subgraph SHARED[Shared craft evaluation]
        S1[SetCraftAction helpers]
        S2[Recipe profession and skill validation]
        S3[Reagent owned and missing calculation]
        S4[Formatted crafting reply text]
    end

    C8 --> S1 --> S2 --> S3 --> S4
    R5 --> S2

    subgraph NOTES[Important behavior]
        N1[Legacy Rpg craft remains legacy and separate]
        N2[New RPG DoCraft is now live]
        N3[Public chat reply crafting is also live]
        N4[One best-fit bot replies in chat path]
        N5[DoCraft exits back to idle when no work remains or timeout expires]
    end

    C13 --> N3
    R26 --> N2
    C10 --> N4
    R25 --> N5
    N1 -.-> R11
```

## Reading guide

- The **left side** shows the player-facing service flow.
- The **right side** shows the autonomous bot-side crafting loop in the new RPG system.
- The **shared block** highlights the common validation logic reused by the chat crafting features.

## Config knobs

- `AiPlayerbot.EnableCraftingReplies`
- `AiPlayerbot.CraftingReplyCooldown`
- `AiPlayerbot.RpgStatusProbWeight.DoCraft`

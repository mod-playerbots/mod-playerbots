# AI engine

The Strategy-Trigger-Action engine under `src/Bot/Engine/`, used by everything in `src/Ai/`.
Read this before touching a strategy, action, trigger, value, multiplier, or `AiFactory.cpp`.

## How a bot ticks

- `Player::Update` calls the `OnPlayerAfterUpdate` hook, which drives `PlayerbotAI::UpdateAI` on
  the map thread. Bots on different maps run in parallel.
- Each bot owns three engines: `BOT_STATE_COMBAT`, `BOT_STATE_NON_COMBAT`, `BOT_STATE_DEAD`.
  State transitions happen only in `PlayerbotAI::DoNextAction()`.
- Several strategies are active at once. Each contributes triggers, actions, and multipliers into
  one relevance-ordered action queue; the highest-relevance possible and useful action runs.
- Values (`CalculatedValue`) cache with a `checkInterval`; triggers read cached values. Match the
  interval to how often the answer can change, not to how often it is read.

## Relevance bands

Declared in `src/Bot/Engine/Strategy/Strategy.h`:

| Band | Value | Use |
|---|---|---|
| `ACTION_BG` | 1 | battleground movement and objectives |
| `ACTION_DEFAULT` | 5 | default actions; must stay in this band |
| `ACTION_NORMAL` | 10 | rotation |
| `ACTION_HIGH` | 20 | rotation priorities |
| `ACTION_MOVE` | 30 | movement |
| `ACTION_INTERRUPT` | 40 | interrupts |
| `ACTION_DISPEL` | 50 | dispels |
| `ACTION_RAID` | 60 | boss mechanics |
| `ACTION_EMERGENCY` | 90 | survival |
| `ACTION_LIGHT_HEAL` / `MEDIUM` / `CRITICAL` | 10 / 20 / 30 | heals |
| pull sequence | 105–107 | reserved (`pull action`, `pull start`, `pull end`) |

Pick a named band plus a small offset (emergency offsets run up to 99). Never a bare float chosen
by racing existing actions, and nothing at or above the pull sequence.

## Multipliers and strategy groups

- Multipliers are multiplicative: `relevance *= GetValue()`. Returning 0 kills the action and a
  negative value is a bug. Whitelist heals and emergency actions before suppressing anything, and
  never zero out every action a state can take.
- Sibling strategy groups (`NamedObjectContext<Strategy>(shared, true)`, e.g. follow / stay /
  guard / flee) are mutually exclusive. A strategy outside the group must not queue the group's
  actions.
- Shared code must not branch on strategy names. `botAI->HasStrategy("x")` belongs in the
  strategy's own files or in command handlers, not in base actions, shared values, or engine
  helpers.
- One action, one job. An `Execute()` that cascades through unrelated fallbacks re-implements the
  relevance queue; split it into separate actions.
- Boss phase triggers are mutually exclusive, and every default-action list ends in a terminal
  fallback so the queue cannot spin.

## Wiring checklist

Unregistered names compile but never run. Before finishing any strategy, action, or trigger
change, confirm:

- New action names are registered in `src/Ai/Base/ActionContext.h` (`creators["name"]`) or the
  class's `src/Ai/Class/<Class>/<Class>AiObjectContext.cpp`.
- New trigger names are registered in `src/Ai/Base/TriggerContext.h` or the class context; new
  strategy names in `src/Ai/Base/StrategyContext.h` or the class context. `getName()` is unique.
- Chat-driven features are registered in `src/Ai/Base/ChatActionContext.h` and
  `ChatTriggerContext.h` and handled in `ChatCommandHandlerStrategy.cpp`.
- Every `NextAction("...")`, `TriggerNode("...")`, and `engine->addStrategy("...")` matches a
  registered creator exactly. Default strategy sets live in `src/Bot/Factory/AiFactory.cpp`;
  instance strategies are added and removed in balance by `ApplyInstanceStrategies()`.
- A strategy that overrides `InitTriggers()` chains the parent's, or intentionally replaces it and
  says so.
- Removed features leave no orphaned `creators[]` entries behind, and a new primitive is not added
  next to an existing one (registered but unused) that already covers the same ground.
- A change that needs core edits belongs in a companion PR to
  `mod-playerbots/azerothcore-wotlk` and the module PR says so.

Check without a build: grep the creator tables for each new name, and grep `NextAction("`,
`TriggerNode("`, and `addStrategy("` for each consumer.

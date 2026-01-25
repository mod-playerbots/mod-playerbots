# mod-playerbots Service Architecture Refactoring

## Document Overview

This document describes the architectural refactoring of the mod-playerbots module from a monolithic design to a service-oriented architecture. This refactoring was implemented in the `refactor/total-architecture` branch.

---

## 1. Purpose

### 1.1 Problem Statement

The mod-playerbots codebase had grown organically into a **monolithic architecture** centered around a single "god object" - the `PlayerbotAI` class. This class contained:

- ~2,500+ methods handling all bot functionality
- 22 static role-checking methods (IsTank, IsHeal, IsDps, etc.)
- Direct coupling to global singletons (TravelMgr, RandomPlayerbotMgr, etc.)
- Tight dependencies between AI logic and implementation details
- **No unit testing infrastructure** - the codebase was effectively untestable

### 1.2 Goals

1. **Enable Unit Testing** - Create interfaces that can be mocked for isolated testing
2. **Reduce Coupling** - Separate concerns into distinct, focused services
3. **Improve Maintainability** - Make the codebase easier to understand and modify
4. **Support Gradual Migration** - Allow incremental adoption without breaking existing code
5. **Establish Clean Architecture** - Define clear boundaries between components

### 1.3 Non-Goals

- Complete rewrite of PlayerbotAI (too risky)
- Performance optimization (focus is on architecture)
- Adding new features (refactoring only)

---

## 2. Scope

### 2.1 Summary Statistics

| Metric | Value |
|--------|-------|
| Files Changed | 316 |
| Insertions | +15,227 |
| Deletions | -2,411 |
| Net Change | +12,816 lines |
| Commits | 18 |

### 2.2 Components Changed

**New Infrastructure (Created):**
- 6 service interface files (`src/Bot/Interface/`) - *actively used*
- 6 service implementation files (`src/Bot/Service/`) - *actively used*
- 34 test files (`test/`)

**Manager Infrastructure (Now Active):**
- 3 manager interface files (ITravelManager, IRandomBotManager, IBotRepository)
- 3 adapter files (TravelManagerAdapter, RandomBotManagerAdapter, BotRepositoryAdapter)
- ManagerRegistry (`src/Bot/Core/ManagerRegistry.h`) - initialized at startup
- ~125 call sites migrated to use `sManagerRegistry` instead of direct singleton access

**AI Components Updated (Modified):**
- 130+ Action files updated to use services
- 50+ Trigger files updated
- 40+ Value files updated
- All class-specific AI (Paladin, Priest, Mage, etc.)
- All raid AI (ICC, Ulduar, Karazhan)
- All dungeon AI (Oculus, FoS, ToC)

**Core Files Refactored:**
- `PlayerbotAI.cpp` - Major restructuring (~1,054 lines changed)
- `PlayerbotAI.h` - Removed 22 static methods, added service container

### 2.3 What Was NOT Changed

- Core game logic (spell effects, combat calculations)
- Database schema
- Configuration file format
- External interfaces (commands, chat handlers)
- PlayerbotMgr class
- Engine execution model

---

## 3. Design

### 3.1 Architecture Overview

**What's Actually Active:**
```
┌─────────────────────────────────────────────────────────────────┐
│                        PlayerbotAI                              │
│  ┌─────────────────────────────────────────────────────────┐   │
│  │              BotServiceContainer (DI)                    │   │
│  │  ┌──────────┬──────────┬──────────┬──────────┬────────┐ │   │
│  │  │ IBotCtx  │ISpellSvc │IChatSvc  │IRoleSvc  │IItemSvc│ │   │
│  │  └────┬─────┴────┬─────┴────┬─────┴────┬─────┴───┬────┘ │   │
│  └───────┼──────────┼──────────┼──────────┼─────────┼──────┘   │
│          ▼          ▼          ▼          ▼         ▼          │
│     BotContext BotSpellSvc BotChatSvc BotRoleSvc BotItemSvc    │
│          │          │          │          │         │          │
│          │    (standalone) (standalone) (standalone)(standalone)│
│          │          │          │          │         │          │
│          └──────────┴──────────┴──────────┴─────────┘          │
│                              │                                  │
│              All services fully extracted with static methods   │
└─────────────────────────────────────────────────────────────────┘
```

**Global Manager Abstraction (Now Active):**
```
┌─────────────────────────────────────────────────────────────────┐
│                    ManagerRegistry (ACTIVE)                     │
│  ┌────────────────┬─────────────────┬──────────────────┐       │
│  │ITravelManager  │IRandomBotManager│IBotRepository    │       │
│  └───────┬────────┴────────┬────────┴─────────┬────────┘       │
│          ▼                 ▼                  ▼                 │
│   TravelMgrAdapter  RandomBotMgrAdapter  BotRepoAdapter        │
│          │                 │                  │                 │
│          ▼                 ▼                  ▼                 │
│      sTravelMgr    sRandomPlayerbotMgr  sPlayerbotRepository   │
│                    (underlying singletons)                      │
└─────────────────────────────────────────────────────────────────┘

Initialized at startup in Playerbots.cpp:OnBeforeWorldInitialized()
~125 call sites migrated to use sManagerRegistry.Get*() methods
```

### 3.2 Design Patterns Used

| Pattern | Usage | Location |
|---------|-------|----------|
| **Dependency Injection** | BotServiceContainer holds all services | `Bot/Core/BotServiceContainer.h` |
| **Interface Segregation** | Each service has a focused interface | `Bot/Interface/*.h` |
| **Static Factory** | Services provide static methods for direct access | `Bot/Service/Bot*.cpp` |

**Global Manager Patterns (Now Active):**
- **Service Locator** (ManagerRegistry) - initialized at startup, provides mockable access to global managers
- **Adapter Pattern** (*Adapter files) - wrap existing singletons behind interfaces for testability

### 3.3 Service Interfaces

All interfaces are pure virtual classes in `src/Bot/Interface/`:

**IBotContext** (~50 methods) - Bot state and object access:
- `GetBot()`, `GetMaster()` - Player pointers
- `GetState()`, `IsInCombat()` - State queries
- `IsOpposing()`, `IsFriendly()` - Faction checks

**ISpellService** (~15 methods) - Spell operations:
- `CanCastSpell()`, `CastSpell()` - Spell casting
- `HasAura()`, `GetAuraCount()` - Aura queries
- `InterruptSpell()`, `IsInterruptableSpellCasting()` - Interrupts

**IChatService** (~10 methods) - Communication:
- `TellMaster()`, `TellError()` - Master messages
- `Say()`, `Whisper()`, `Yell()` - Chat functions
- `SayToGuild()`, `SayToParty()` - Group messaging

**IRoleService** (~15 methods) - Role detection:
- `IsTank()`, `IsHeal()`, `IsDps()` - Primary roles
- `IsRanged()`, `IsMelee()`, `IsCaster()` - Combat style
- `GetGroupTankNum()`, `IsMainTank()` - Group positions

**IItemService** (~10 methods) - Inventory:
- `FindPoison()`, `FindAmmo()` - Class items
- `GetInventoryAndEquippedItems()` - Item queries
- `HasItemInInventory()`, `GetItemCount()` - Item checks

**IConfigProvider** (~40 methods) - Configuration:
- `GetSightDistance()`, `GetGlobalCooldown()` - Numeric configs
- `IsEnabled()`, `ShouldAutoEquip()` - Boolean configs

### 3.4 Service Implementations

All four services now use the **Full Implementation** pattern with static methods for direct access and instance methods that delegate to static methods. This provides both backward compatibility and full testability.

**Pattern: Static Methods + Instance Delegation**
```cpp
class BotRoleService : public IRoleService {
    // Static methods for direct access (no instance needed)
    static bool IsTankStatic(Player* player, bool bySpec = false);

    // Interface implementation delegates to static
    bool IsTank(Player* player, bool bySpec = false) const override {
        return IsTankStatic(player_, bySpec);
    }
};
```

**Service Extraction Summary:**

| Service | Lines | Static Methods | Context Struct |
|---------|-------|----------------|----------------|
| BotRoleService | ~850 | 22 methods | N/A |
| BotChatService | ~480 | 15 methods | ChatContext |
| BotItemService | ~400 | 17 methods | N/A |
| BotSpellService | ~590 | 12 methods | SpellContext |

**Context Structs for Complex Dependencies:**

Some services require context from PlayerbotAI for their static methods. Context structs bundle these dependencies:

```cpp
// ChatContext bundles chat-related dependencies
struct ChatContext {
    Player* bot;
    std::function<Player*()> getMaster;
    PlayerbotSecurity* security;
    std::map<std::string, time_t>* whispers;
    std::pair<ChatMsg, time_t>* currentChat;
    std::function<bool(std::string const&, int)> hasStrategy;
    std::function<bool()> hasRealPlayerMaster;
};

// SpellContext bundles spell-related dependencies
struct SpellContext {
    Player* bot;
    AiObjectContext* aiObjectContext;
    ChatHelper* chatHelper;
    std::function<void(uint32)> setNextCheckDelay;
    std::function<bool(std::string const&, int)> hasStrategy;
    std::function<bool()> hasRealPlayerMaster;
};
```

**Benefits of Full Extraction:**
- No dependency on PlayerbotAI at runtime
- Fully standalone and testable
- Static methods callable without service instance
- Instance methods provide interface compliance for mocking

### 3.5 BotServiceContainer

Central dependency injection container (`Bot/Core/BotServiceContainer.h`):

```cpp
class BotServiceContainer {
    std::unique_ptr<IBotContext> context_;
    std::unique_ptr<ISpellService> spellService_;
    std::unique_ptr<IChatService> chatService_;
    std::unique_ptr<IRoleService> roleService_;
    std::unique_ptr<IItemService> itemService_;
    std::unique_ptr<IConfigProvider> config_;
public:
    // Getters for production code
    IBotContext& GetContext();
    ISpellService& GetSpellService();
    // ...

    // Setters for test injection
    void SetContext(std::unique_ptr<IBotContext> context);
    void SetSpellService(std::unique_ptr<ISpellService> service);
    // ...
};
```

**Design Decisions:**
- `unique_ptr` for single ownership
- No copying allowed (deleted copy constructor)
- Move semantics supported
- Both const and non-const accessors

### 3.6 ManagerRegistry (Now Active)

A singleton registry provides mockable access to global managers:

```cpp
class ManagerRegistry {
    static ManagerRegistry& Instance();
    ITravelManager& GetTravelManager();
    IRandomBotManager& GetRandomBotManager();
    IBotRepository& GetBotRepository();
};
#define sManagerRegistry ManagerRegistry::Instance()
```

**Initialization (Playerbots.cpp):**
```cpp
void OnBeforeWorldInitialized() override
{
    // ... config loading ...

    // Initialize ManagerRegistry with production adapters
    sManagerRegistry.SetTravelManager(std::make_shared<TravelManagerAdapter>());
    sManagerRegistry.SetRandomBotManager(std::make_shared<RandomBotManagerAdapter>());
    sManagerRegistry.SetBotRepository(std::make_shared<BotRepositoryAdapter>());
}
```

**Usage Pattern:**
```cpp
// Before (direct singleton access)
sRandomPlayerbotMgr->IsRandomBot(bot)

// After (through registry)
sManagerRegistry.GetRandomBotManager().IsRandomBot(bot)
```

**Migration Status:**
- `IRandomBotManager`: ~65 `IsRandomBot` calls + ~60 other interface-compatible calls migrated
- `IBotRepository`: All `Save`/`Load`/`Reset` calls migrated (~8 call sites)
- `ITravelManager`: Interface has limited methods; most `sTravelMgr` calls still use direct access

**Note:** Some `sRandomPlayerbotMgr` methods are not yet in `IRandomBotManager` (e.g., `BattlegroundData`, `GetPlayerBotsBegin/End`, `IsAddclassBot`). These still use direct singleton access pending interface expansion.

### 3.7 Migration Approach

The refactoring used a **direct migration** approach rather than feature flags:

1. **Services created alongside existing code** - New service classes wrap existing PlayerbotAI methods
2. **Callers updated in bulk** - Each service migration commit updated all callers at once
3. **Tests validate behavior** - Unit tests ensure services work correctly before integration

**Note:** A `RefactorFlags.h` file was created in the foundation commit as a potential mechanism for gradual rollout, but was ultimately **not used**. The direct migration approach was chosen instead because:
- Services delegate to existing PlayerbotAI methods (minimal risk)
- Comprehensive test coverage validates the changes
- Bulk migration is cleaner than maintaining two code paths

### 3.8 Call Pattern Transformation

**Before Refactoring:**
```cpp
// Direct method calls on PlayerbotAI
if (botAI->IsTank(player)) {
    botAI->CastSpell(TAUNT_SPELL_ID, target);
    botAI->TellMaster("Taunting target!");
}
```

**After Refactoring:**
```cpp
// Service-based calls via container
if (botAI->GetServices().GetRoleService().IsTank(player)) {
    botAI->GetServices().GetSpellService().CastSpell(TAUNT_SPELL_ID, target);
    botAI->GetServices().GetChatService().TellMaster("Taunting target!");
}
```

### 3.9 Testing Infrastructure

**Test Directory Structure:**
```
test/
├── CMakeLists.txt           # Build configuration
├── fixtures/                # Test utilities
│   ├── AcoreTypes.h        # AzerothCore type definitions
│   ├── TriggerTestFixture.h
│   └── PlayerbotSecurity.h
├── mocks/                   # Mock implementations
│   ├── MockBotServices.h   # All service mocks (GMock)
│   ├── MockManagers.h      # Manager mocks
│   └── MockPlayerbotAI.h
└── unit/                    # 24 test files
    ├── Bot/Core/           # Core infrastructure tests
    │   └── ManagerRegistryTest.cpp
    ├── Bot/Service/        # Service tests
    │   ├── RoleServiceTest.cpp
    │   ├── ChatServiceTest.cpp
    │   ├── ItemServiceTest.cpp
    │   └── SpellServiceTest.cpp
    ├── Ai/Action/          # Action logic tests
    ├── Ai/Combat/          # Combat behavior tests
    └── ...
```

**Mock Example:**
```cpp
class MockRoleService : public IRoleService {
    MOCK_METHOD(bool, IsTank, (Player*, bool), (const, override));
    MOCK_METHOD(bool, IsHeal, (Player*, bool), (const, override));
    // All interface methods mocked
};
```

**Test Example:**
```cpp
TEST_F(RoleServiceTest, CanMockTankRole) {
    MockRoleService mockRoleService;
    EXPECT_CALL(mockRoleService, IsTank(_, false))
        .WillOnce(Return(true));

    EXPECT_TRUE(mockRoleService.IsTank(nullptr, false));
}
```

**Test Coverage:**
- 344+ unit tests passing
- Combat behavior tests (5 suites)
- Engine tests (4 suites)
- AI integration tests
- Resource management tests
- ManagerRegistry tests (mock injection patterns)

---

## 4. Implementation Timeline

| Commit | Phase | Description |
|--------|-------|-------------|
| 1f8cc0ce | 0-2 | Foundation: interfaces, container, test infra |
| 8ff4c776 | 3 | Extract service implementations |
| 682dde0c | 3 | Extract handler implementations |
| 0a68a631 | 3 | Integrate BotServiceContainer into PlayerbotAI |
| 10fdb6db | 3 | Add manager interfaces and registry |
| 3a709117 | 3 | Add unit tests for services |
| 88174144 | 3 | Add unit tests for AI pattern logic |
| c0e2ee7e | 3 | Add comprehensive tests for all subsystems |
| 832b1ac5 | 4 | Migrate role service callers |
| 28def1a4 | 4 | Migrate spell service callers (~460 callers) |
| 51a9e807 | 4 | Migrate chat service callers (~477 callers) |
| 3d407ea5 | 4 | Migrate item service callers (~35 callers) |
| f7b5a69e | 5 | Fix test infrastructure |
| 884ff940 | 5 | Fix all unit test algorithm issues |
| 69f75c13 | 5 | Resolve build failures |
| 66e1e7ab | 5 | Add PlayerbotSecurity.h fixture |
| b6593be8 | 5 | Delete migration progress doc (cleanup) |
| 2f473996 | 5 | Merge upstream with service adaptation |

---

## 5. Key Files Reference

**Active - Core Architecture:**
- `src/Bot/Core/BotServiceContainer.h` - DI container (actively used)
- `src/Bot/PlayerbotAI.h` - Contains BotServiceContainer
- `src/Bot/PlayerbotAI.cpp` - Service initialization

**Active - Service Interfaces:**
- `src/Bot/Interface/IBotContext.h`
- `src/Bot/Interface/ISpellService.h`
- `src/Bot/Interface/IChatService.h`
- `src/Bot/Interface/IRoleService.h`
- `src/Bot/Interface/IItemService.h`
- `src/Bot/Interface/IConfigProvider.h`

**Active - Service Implementations (all fully extracted with static methods):**
- `src/Bot/Service/BotContext.h/cpp`
- `src/Bot/Service/BotRoleService.h/cpp` (~850 lines)
- `src/Bot/Service/BotSpellService.h/cpp` (~590 lines)
- `src/Bot/Service/BotChatService.h/cpp` (~480 lines)
- `src/Bot/Service/BotItemService.h/cpp` (~400 lines)
- `src/Bot/Service/ConfigProvider.h`

**Active - Manager Infrastructure:**
- `src/Bot/Core/ManagerRegistry.h` - Global manager registry (initialized at startup)
- `src/Bot/Interface/ITravelManager.h` - Travel manager interface
- `src/Bot/Interface/IRandomBotManager.h` - Random bot manager interface
- `src/Bot/Interface/IBotRepository.h` - Bot repository interface
- `src/Bot/Service/TravelManagerAdapter.h/cpp` - Wraps sTravelMgr
- `src/Bot/Service/RandomBotManagerAdapter.h/cpp` - Wraps sRandomPlayerbotMgr
- `src/Bot/Service/BotRepositoryAdapter.h/cpp` - Wraps sPlayerbotRepository

**Unused:**
- `src/RefactorFlags.h` - Feature flags (not used)

**Test Infrastructure:**
- `test/mocks/MockBotServices.h` - All service mocks
- `test/mocks/MockManagers.h` - Manager mocks (ITravelManager, IRandomBotManager, IBotRepository)
- `test/unit/Bot/Core/ManagerRegistryTest.cpp` - ManagerRegistry mock injection tests
- `test/unit/Bot/Service/RoleServiceTest.cpp` - Role service tests
- `test/unit/Bot/Service/ChatServiceTest.cpp` - Chat service tests
- `test/unit/Bot/Service/ItemServiceTest.cpp` - Item service tests
- `test/unit/Bot/Service/SpellServiceTest.cpp` - Spell service tests

---

## 6. Benefits Achieved

1. **Testability** - Services are independently testable via interfaces
2. **Loose Coupling** - Components depend on interfaces, not implementations
3. **Clean Boundaries** - Clear separation between service responsibilities
4. **Mock Support** - Full GMock infrastructure for testing
5. **Type Safety** - Template-based registry with compile-time checking
6. **Backward Compatibility** - Services delegate to existing code, minimizing risk

---

## 7. Code Quality Improvements

During the service extraction, several compiler warnings were fixed:

**Deprecated Copy Warnings (-Wdeprecated-copy-with-user-provided-copy):**
Added copy assignment operators to classes with user-provided copy constructors:
- `NextAction` in `Action.h`
- `PositionInfo` in `PositionValue.h`
- `CraftData` in `CraftValue.h`
- `UnitPosition` in `Arrow.h`

**Initialization Order Warnings (-Wreorder-ctor):**
Fixed member initialization order to match declaration order:
- `ArrowFormation` in `Arrow.h`
- `LastMovement` in `LastMovementValue.cpp`
- Various predicate classes in `PartyMember*.cpp`

**Sign Comparison Warnings (-Wsign-compare):**
Fixed integer signedness mismatches:
- `MovementActions.h` - loop counter type
- `ChatShortcutActions.cpp` - map ID comparison
- `FollowActions.cpp` - map ID comparison

**Pessimizing Move Warnings (-Wpessimizing-move):**
Removed unnecessary `std::move()` on return statements:
- `TravelNode.cpp` lines 367, 1166

**Unused Parameter Warnings (-Wunused-parameter):**
Added `[[maybe_unused]]` attribute to intentionally unused parameters in action Execute() methods.

---

## 8. Future Considerations

**Completed (Phase 2 - Services):**
- ✅ All four services fully extracted with static methods (BotRoleService, BotChatService, BotItemService, BotSpellService)
- ✅ Context structs added for complex dependencies (ChatContext, SpellContext)
- ✅ Unit tests added for all services

**Completed (Phase 3 - ManagerRegistry):**
- ✅ ManagerRegistry initialized at startup with production adapters
- ✅ ~125 call sites migrated from direct singleton access to `sManagerRegistry`
- ✅ RandomBotManagerAdapter fully implemented (all interface methods)
- ✅ BotRepositoryAdapter core methods (Save/Load/Reset) implemented
- ✅ Unit tests added for ManagerRegistry mock injection patterns

**Remaining Work:**

*Interface Expansion:*
- Extend `IRandomBotManager` to cover more methods (BattlegroundData, GetPlayerBotsBegin/End, IsAddclassBot, etc.)
- Extend `ITravelManager` to cover more TravelMgr methods (getQuestTravelDestinations, getRpgTravelDestinations, etc.)
- Migrate remaining direct singleton calls once interfaces are expanded

*Service Architecture:*
- Migrate remaining PlayerbotAI methods to services (CanCastSpell, CastSpell - the largest methods)
- Consider extracting more granular services (e.g., AuraService, MovementService)
- Add integration tests alongside unit tests
- Document API contracts for each service
- Add deprecation warnings to PlayerbotAI methods that have been extracted

*Clean Up:*
- Remove `src/RefactorFlags.h` (unused)

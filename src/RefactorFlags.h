/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_REFACTOR_FLAGS_H
#define _PLAYERBOT_REFACTOR_FLAGS_H

// Feature flags for incremental refactoring
// Set to 1 to enable new implementation, 0 to use legacy code

// Phase 1: Configuration System
#define USE_NEW_CONFIG_SYSTEM 0

// Phase 3: Service Extraction
#define USE_NEW_ROLE_SERVICE 0
#define USE_NEW_SPELL_SERVICE 0
#define USE_NEW_CHAT_SERVICE 0
#define USE_NEW_ITEM_SERVICE 0

// Phase 4: Handler Extraction
#define USE_NEW_PACKET_HANDLER 0
#define USE_NEW_CHAT_COMMAND_HANDLER 0

// Phase 6: Manager Refactoring
#define USE_NEW_MANAGER_REGISTRY 0

#endif

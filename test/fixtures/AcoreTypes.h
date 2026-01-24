/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

/**
 * @brief Minimal AzerothCore type stubs for unit testing
 *
 * This file provides the basic type definitions normally found in AzerothCore's
 * Common.h, allowing unit tests to compile without the full server dependencies.
 */

#ifndef _TEST_ACORE_TYPES_H
#define _TEST_ACORE_TYPES_H

#include <cstdint>
#include <string>

// Basic integer types from AzerothCore
using int8 = int8_t;
using int16 = int16_t;
using int32 = int32_t;
using int64 = int64_t;
using uint8 = uint8_t;
using uint16 = uint16_t;
using uint32 = uint32_t;
using uint64 = uint64_t;

// ObjectGuid stub for testing (simplified)
class ObjectGuid
{
public:
    using LowType = uint32;

    ObjectGuid() : _guid(0) {}
    explicit ObjectGuid(uint64 guid) : _guid(guid) {}

    bool IsEmpty() const { return _guid == 0; }
    uint64 GetRawValue() const { return _guid; }
    LowType GetCounter() const { return static_cast<LowType>(_guid & 0xFFFFFFFF); }

    bool operator==(ObjectGuid const& other) const { return _guid == other._guid; }
    bool operator!=(ObjectGuid const& other) const { return _guid != other._guid; }
    bool operator<(ObjectGuid const& other) const { return _guid < other._guid; }

    static ObjectGuid Empty;

private:
    uint64 _guid;
};

// WorldLocation stub for testing
struct WorldLocation
{
    uint32 mapId{0};
    float x{0.0f};
    float y{0.0f};
    float z{0.0f};
    float orientation{0.0f};

    WorldLocation() = default;
    WorldLocation(uint32 map, float px, float py, float pz, float o = 0.0f)
        : mapId(map), x(px), y(py), z(pz), orientation(o) {}
};

#endif

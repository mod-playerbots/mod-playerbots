/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "CraftRules.h"

namespace CraftRules
{
std::string EncodeRecipeBits(std::vector<bool> const& known)
{
    static constexpr char base64[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    static constexpr size_t BITS_PER_CHAR = 6;

    std::vector<uint8> values((known.size() + BITS_PER_CHAR - 1) / BITS_PER_CHAR, 0);
    for (size_t i = 0; i < known.size(); ++i)
    {
        if (known[i])
            values[i / BITS_PER_CHAR] |= uint8(1 << (i % BITS_PER_CHAR));
    }

    std::string out;
    out.reserve(values.size());
    for (uint8 value : values)
        out += base64[value];

    return out;
}
}  // namespace CraftRules

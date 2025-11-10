#ifndef UINT32_VECTOR_TO_STRING_H
#define UINT32_VECTOR_TO_STRING_H

#include <vector>
#include <string>
#include <sstream>
#include <cstdint>

inline std::string UInt32VectorToString(const std::vector<uint32_t>& vec)
{
    std::ostringstream oss;

    for (size_t i = 0; i < vec.size(); ++i)
    {
        if (i > 0)
            oss << ',';
        oss << vec[i];
    }

    return oss.str();
}

#endif

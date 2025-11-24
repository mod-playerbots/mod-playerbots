/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_HELPERS_H
#define _PLAYERBOT_HELPERS_H

#include <stdio.h>
#include <string.h>
#include <string>

#include <algorithm>
#include <cctype>
#include <functional>
#include <locale>
#include <map>
#include <sstream>
#include <vector>

#include "Common.h"

// Trimming helpers (implemented in Helpers.cpp)
std::string& ltrim(std::string& s);
std::string& rtrim(std::string& s);
std::string& trim(std::string& s);

inline void split(std::vector<std::string>& dest, std::string const str, char const* delim)
{
    char* pTempStr = strdup(str.c_str());
    char* pWord = strtok(pTempStr, delim);

    while (pWord != nullptr)
    {
        dest.push_back(pWord);
        pWord = strtok(nullptr, delim);
    }

    free(pTempStr);
}

inline std::vector<std::string>& split(std::string const s, char delim, std::vector<std::string>& elems)
{
    std::stringstream ss(s);
    std::string item;

    while (getline(ss, item, delim))
    {
        elems.push_back(item);
    }

    return elems;
}

inline std::vector<std::string> split(std::string const s, char delim)
{
    std::vector<std::string> elems;
    return split(s, delim, elems);
}

#endif

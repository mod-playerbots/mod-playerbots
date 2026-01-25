/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_HELPERS_H
#define _PLAYERBOT_HELPERS_H

#include <string>
#include <vector>

/**
 * Case-insensitive substring search.
 *
 * @param haystack The string to search in
 * @param needle   The substring to search for
 * @return Pointer to the first matching position in haystack, or nullptr if not found.
 */
char* strstri(char const* haystack, char const* needle);

/**
 * Trim whitespace from the left side of a string (in place).
 *
 * @param s The string to trim
 * @return Reference to the modified string
 */
std::string& ltrim(std::string& s);

/**
 * Trim whitespace from the right side of a string (in place).
 *
 * @param s The string to trim
 * @return Reference to the modified string
 */
std::string& rtrim(std::string& s);

/**
 * Trim whitespace from both ends of a string (in place).
 *
 * @param s The string to trim
 * @return Reference to the modified string
 */
std::string& trim(std::string& s);

/**
 * Split a string using a C-string delimiter.
 *
 * @param dest  Vector to store split tokens
 * @param str   String to split
 * @param delim C-string delimiter
 */
void split(std::vector<std::string>& dest, std::string const str, char const* delim);

/**
 * Split a string using a single character delimiter.
 *
 * @param s     String to split
 * @param delim Delimiter character
 * @param elems Vector to store split tokens
 * @return Reference to the vector containing tokens
 */
std::vector<std::string>& split(std::string const s, char delim, std::vector<std::string>& elems);

/**
 * Split a string using a single character delimiter.
 *
 * @param s     String to split
 * @param delim Delimiter character
 * @return Vector containing split tokens
 */
std::vector<std::string> split(std::string const s, char delim);

#endif

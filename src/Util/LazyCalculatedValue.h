/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_LAZYCALCULATEDVALUE_H
#define _PLAYERBOT_LAZYCALCULATEDVALUE_H

/**
 * @brief Lazy calculation helper.
 *
 * Stores a function pointer (calculator) and its owner instance, and
 * calculates the value only when it is requested for the first time.
 * The result is cached until Reset() is called.
 *
 * @tparam TValue  Type of the calculated value.
 * @tparam TOwner  Type of the owner class containing the calculator function.
 */
template <class TValue, class TOwner>
class LazyCalculatedValue
{
public:
    /**
     * @brief Type of the calculator function.
     *
     * This is a pointer to a member function of TOwner returning TValue.
     */
    typedef TValue (TOwner::*Calculator)();

public:
    /**
     * @brief Constructor.
     *
     * @param owner      Pointer to the owner object.
     * @param calculator Pointer to the member function used to calculate the value.
     */
    LazyCalculatedValue(TOwner* owner, Calculator calculator) : calculator(calculator), owner(owner) { Reset(); }

public:
    /**
     * @brief Get the cached value or calculate it if needed.
     *
     * If the value has not been calculated yet, it calls the calculator
     * on the owner and caches the result.
     *
     * @return TValue The calculated or cached value.
     */
    TValue GetValue()
    {
        if (!calculated)
        {
            value = (owner->*calculator)();
            calculated = true;
        }

        return value;
    }

    /**
     * @brief Reset the cached state.
     *
     * After calling Reset(), the next call to GetValue() will recalculate
     * the value again.
     */
    void Reset() { calculated = false; }

protected:
    Calculator calculator;  ///< Pointer to calculator member function
    TOwner* owner;          ///< Owner instance
    bool calculated;        ///< Whether value has already been calculated
    TValue value;           ///< Cached value
};

#endif

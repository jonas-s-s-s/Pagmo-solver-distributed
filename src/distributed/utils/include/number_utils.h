#pragma once
#include <cstdint>

/**
 * Safely divides two numbers, returning the value of "fallback" in case of division by zero
 * @param a first operand
 * @param b second operand
 * @param fallback value to be returned if b is zero
 * @return a / b or fallback
 */
inline double safe_divide(const uint64_t a, const uint64_t b, const double fallback = 0)
{
    if (b > 0)
        return static_cast<double>(a) / static_cast<double>(b);
    return fallback;
}

/**
 * Return the closest number bigger or equal to "input", which is divisible by "multiple"
 * @param input the input number
 * @param multiple output will be divisible by this
 * @return a number divisible by multiple
 */
inline uint64_t ceil_to_multiple(const uint64_t input, const uint64_t multiple)
{
    const uint64_t rem = input % multiple;
    if (rem == 0)
        return input;
    return input + (multiple - rem);
}

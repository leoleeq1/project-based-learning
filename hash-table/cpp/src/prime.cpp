#include <cstdint>

#include "prime.hpp"

// 1 - prime
// 0 - not prime
// -1 - undefined
int32_t isPrime(int32_t x)
{
    if (x < 2)
    {
        return -1;
    }

    if (x < 4)
    {
        return 1;
    }

    if (x % 2 == 0)
    {
        return 0;
    }

    for (size_t i = 3; i * i <= x; i += 2)
    {
        if (x % i == 0)
        {
            return 0;
        }
    }

    return 1;
}

int32_t nextPrime(int32_t x)
{
    while (isPrime(x) <= 0)
    {
        ++x;
    }

    return x;
}

#include <stdint.h>
#include <math.h>

#include "prime.h"

// 1 - prime
// 0 - not
// -1 - undefined
int is_prime(const int32_t x)
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

    for (int i = 3; i * i <= x; i += 2)
    {
        if ((x % i) == 0)
        {
            return 0;
        }
    }

    return 1;
}

int next_prime(int32_t x)
{
    while (is_prime(x) <= 0)
    {
        ++x;
    }

    return x;
}
#include "temp.h"

int temp_round(int x10)
{
    x10 = temp_clamp(x10);
    return (x10 + 5) / 10;
}

#include "temp.h"

int temp_clamp(int x10)
{
    return x10 < -2732 ? -2732 : x10;
}

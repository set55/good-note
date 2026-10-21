#include "clamp.h"

int set_volume(int v)
{
    return clamp(v, 0, 100);
}

#ifndef CLAMP_H
#define CLAMP_H

static int clamp(int v, int lo, int hi)
{
    if (v < lo)
        return lo;
    if (v > hi)
        return hi;
    return v;
}

#endif

#include "temp.h"

int temp_calls = 0;

int temp_round(int x10)
{
    temp_calls = temp_calls + 1;
    return (x10 + 5) / 10;
}

long counter = 0;          /* v2：從 int（4 byte）改成 long（8 byte） */

int bump(void)
{
    counter = counter + 1;
    return (int)counter;
}

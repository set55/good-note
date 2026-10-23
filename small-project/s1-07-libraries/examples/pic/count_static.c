static int counter = 0;

int bump(void)
{
    counter = counter + 1;
    return counter;
}

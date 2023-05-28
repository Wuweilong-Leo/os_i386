#include "print.h"

int main()
{
    for (int i = 0; i < 1000; i++)
    {
        put_int(i);
        put_char('\n');
    }
    while (1)
        ;
}

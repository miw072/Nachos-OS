#include "syscall.h"

int
main()
{
    int *p = 0;
    p--;
    *p = 10;
    return 0;
}

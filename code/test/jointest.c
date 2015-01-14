/*
 * jointest.c
 *
 * 
 */

#include "syscall.h"

int
main()
{
    int b;
    int a;
    a = Exec("../test/jointest2", 0, 0, 1);
    b=Join(a);
    Exit(b+1);
}

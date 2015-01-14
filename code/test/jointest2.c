/*
 * jointest2.c
 *
 * 
 */

#include "syscall.h"

int
main()
{
    int i;
    for(i=0;i<100000;i++);
    Exit(123);
}

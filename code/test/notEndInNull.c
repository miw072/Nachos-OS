/*
 * notEndInNull.c
 *
 * Tring to test if Exec can handle the case
 * where the string passed in doed not end
 * in a null character
 */

#include "syscall.h"

int
main()
{
    int result = 99;
    char path[16] = "../test/exittest";
    path[16] = 'w';
    result = Exec(path, 0, 0, 0);
    Exit(result);  
}


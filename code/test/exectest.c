/*
 * exectest.c
 *
 * Exec a simple program.  Return the result of Exec.
 */

#include "syscall.h"

int
main()
{
    int result = 1000;
    result = Exec("../test/shell", 0, 0, 1);
    //Join(result);
    Exit(result);
}

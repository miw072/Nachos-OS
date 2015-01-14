/*
 * exectest.c
 *
 * Exec a simple program.  Return the result of Exec.
 */

#include "syscall.h"

int
main()
{   int a=1;
    int b=2;
    int result = 1;
    char ** s;
    s[0]="a1";
    s[1]="b2";
    s[2]="c3";
    //result = Exec("../test/exresult", 3, s, 0);
    result = Exec("../test/exresult", 3, s, 0);
    Exit(123);
}

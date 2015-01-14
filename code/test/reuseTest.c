/*
 * reuseTest.c
 *
 * Tring to test if AddrSpace releases memory when a 
 * process goes away so it can be reused again
 */

#include "syscall.h"

int
main()
{
    int result = 1000;
    result = Exec("../test/reuseTest0", 0, 0, 0);
    result = Exec("../test/reuseTest2", 0, 0, 0);
    Exit(result);  
}


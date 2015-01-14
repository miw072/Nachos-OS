/*
 * badStrAddr.c
 *
 * Tring to test if system call Exec() can
 * handle the bad string address argument
 */

#include "syscall.h"

int
main()
{
    int result = 99;
    char path[16] = "../test/exittest";
    
    // Pass a wrong address of file path string
    result = Exec(path+20, 0, 0, 0);	

    Exit(result);  
}

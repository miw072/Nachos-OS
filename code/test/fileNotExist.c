/*
 * fileNotExist.c
 *
 * Tring to test if system call Exec() can
 * handle the case where the file does not
 * exist
 */

#include "syscall.h"

int
main()
{
    int result = 99;
    
    // File foo not exist
    result = Exec("../test/foo", 0, 0, 0);
    Exit(result);  
}

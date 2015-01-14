/*
 * jointest3.c
 *
 * Tring to test if join system call is okay
 */

#include "syscall.h"

int
main()
{
    SpaceId spaceid = 99;
    SpaceId spaceid2 = 100;

    /* Process is joinable */
    spaceid = Exec("../test/joinee", 0, 0, 1);
    Join(spaceid);

    /* Process is not joinable*/
    spaceid2 = Exec("../test/joinee", 0, 0, 0);
    Join(spaceid2);
    Exit(999);
}

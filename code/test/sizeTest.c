/*
 * sizeTest.c
 * Trying load a program that doesn't fit into physical memory.
 * 
 */

#include "syscall.h"

int
main()
{
    static int foo[2705]; 
}

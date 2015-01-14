#include "syscall.h"

int
main(int argc,char** argv)
{   int k=0;
    if (argv[0][0]='a') k++;
    if (argv[0][1]='c') k++;    
    Exit(k);
}

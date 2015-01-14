//forktest.c
//main process Fork() two threads to run two functions,
//one writes "ping" and the other writes "pong".
//Yield in main() is needed because Write() will wait 
//for semaphore each time it prints a single character
#include "syscall.h"

void fun(){
    while(1){
        Write("ping\n",5,ConsoleOutput);   
        Yield();
    }
    return;
   
}

void fun2(){

    while(1){
          Write("pong\n",5,ConsoleOutput);
          Yield();
    }
    return;
}

int
main()
{
    int i;
    Fork(fun);
    Fork(fun2);
    for(i=0;i<118;i++)
    {
        Yield();
    }
    Exit(123);
}

/* HelloWorld.c
 * should print the string HelloWorld to the console and then cleanly exit
 */
 
//Needed system call declaration:
//void Write(char *buffer, int size, OpenFileId id);

#include "syscall.h"

void
main()
{
	OpenFileId input = ConsoleOutput;
	
	Write("HelloWorld\n",11,ConsoleOutput);
	
    return ;
}

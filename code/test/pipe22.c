#include <syscall.h>

int
main ()
{
    char buffer[80];
    int i;
    int j;
    // read the input terminated by a newline
       i = 0;
       do {
	   Read(&buffer[i], 1, ConsoleInput);
       } while (buffer[i++] != '\n');
       for (j=0;j<(i-1);j++)
       { buffer[j]++;}
       buffer[i] = '\0';
       Write(buffer, i, ConsoleOutput);

} 


/* Name.c
 * should ask the user for her/his name and then print it backwards
 */

//Needed system call declarations:
//void Write(char *buffer, int size, OpenFileId id);
//int Read(char *buffer, int size, OpenFileId id);

#include "syscall.h"

int
main()
{
	OpenFileId input = ConsoleInput;
    OpenFileId output = ConsoleOutput;
	
	char buffer[100];    // human names tend to be much smaller than this
    int i;

	
	i = 0;
	do {	
	    Read(&buffer[i], 1, input); 

	} while ( (buffer[i++] != '\n') && (i<100 ) );  
	
	//going backwards from the last character before newline until charachter 
	//at position 0. print character by character what was read.
	for (i=i-2;i>=0;i--) {
		Write(&buffer[i], 1, output); 
	}
	Write("\n", 1, output); //print newline (to make output clearer)
}


// SynchConsole.h
// 	Data structures to export a synchronous interface to the simulate console.


#ifndef SYNCHCONSOLE_H
#define SYNCHCONSOLE_H

#include "copyright.h"
#include "synch.h"
#include "console.h"
#include "machine.h"

class SynchConsole{
public:
		SynchConsole(char *readFile, char *writeFile);
		~SynchConsole();
		void SynchWriteDone();	 
    	void SynchReadAvail();	
		void PutChar(char c);
		char GetChar();

private:
		Console * console;  		
	    Lock *lockwrite;
	    Lock *lockread;	
		Semaphore *semawrite; 		
  	    Semaphore *semaread; 
};


#endif

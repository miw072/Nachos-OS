// synchdisk.cc
//	Routines to synchronously access the console.

#include "copyright.h"
#include "SynchConsole.h"

//call back functions


static void SynchConsoleReadPoll(int c)
{
    SynchConsole *synchconsole = (SynchConsole *)c;
    synchconsole->SynchReadAvail();
}
static void SynchConsoleWriteDone(int c)
{
    SynchConsole *synchconsole = (SynchConsole *)c;
    synchconsole->SynchWriteDone();
}

SynchConsole::SynchConsole(char *readFile, char *writeFile) 
{
	lockread = new Lock("lockread");
	lockwrite = new Lock("lockwrite");
	semaread = new Semaphore("semaphoreread", 0);
	semawrite = new Semaphore("semaphorewrite", 0);
	console = new Console(readFile, writeFile, (VoidFunctionPtr) SynchConsoleReadPoll, (VoidFunctionPtr) SynchConsoleWriteDone, (int) this);
}

SynchConsole::~SynchConsole()
{
    delete console;
    delete lockread;
    delete lockwrite;
    delete semaread;
    delete semawrite;
}


//synchronous access to the console GetChar()
char 
SynchConsole::GetChar()
{
	char ch;
	lockread->Acquire();

	semaread->P();
	ch=console->GetChar();
		
	lockread->Release();
    return ch;
}


//synchronous access to the console PutChar()
void 
SynchConsole::PutChar(char ch) 
{
	lockwrite->Acquire();

	console->PutChar(ch);
	semawrite->P();
	
	lockwrite->Release();
}


//read available, signal GetChar
void 
SynchConsole::SynchReadAvail()
{
	semaread->V();
}


//write done, signal PutChar
void
SynchConsole::SynchWriteDone()
{
	semawrite->V();
}




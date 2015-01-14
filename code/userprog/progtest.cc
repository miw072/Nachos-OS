// progtest.cc
//	Test routines for demonstrating that Nachos can load
//	a user program and execute it.
//
//	Also, routines for testing the Console hardware device.
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation
// of liability and disclaimer of warranty provisions.
#include "memorymanager.h"
#include "copyright.h"
#include "system.h"
#include "console.h"
//#include "addrspace.h"
//#include "synch.h"
#include "machine.h"
#include "Table.h"
//#include "Pcb.h"
#include "SynchConsole.h"
#include "addrspace.h"
#include "list.h"
//----------------------------------------------------------------------
// StartProcess
// 	Run a user program.  Open the executable, load it into
//	memory, and jump to it.
//----------------------------------------------------------------------
MemoryManager *memorymanager;
SynchConsole *synchcon;
Table * processtable;
TranslationEntry **memownertable;
int *memownerprotable;
int processcount=0;
List *memoryList;

void
StartProcess(char *filename)
{   memoryList = new List;
    processtable = new Table(1000);
    memorymanager = new MemoryManager(NumPhysPages);
    memownertable= new TranslationEntry*[NumPhysPages];
    memownerprotable = new int[NumPhysPages];
    
    OpenFile *executable = fileSystem->Open(filename);
    AddrSpace *space;

    if (executable == NULL) {
        printf("Unable to open file %s\n", filename);
        return;
    }
    else
    {synchcon = new SynchConsole(NULL,NULL);}
    
    space = new AddrSpace;
    ASSERT(space->Initialize(executable,0,0));
		
    currentThread->space = space;
    int pid = processtable->Alloc(currentThread->space->getpcb());
    currentThread->space->getpcb()->setpid(pid); 
    currentThread->space->getpcb()->backstoreinitialize(currentThread->space->getpagetablesize(),pid);
    //delete executable;			// close file

    space->InitRegisters();		// set the initial register values
    space->RestoreState();		// load page table register
    processcount++;
    machine->Run();			// jump to the user progam
    ASSERT(FALSE);			// machine->Run never returns;
    // the address space exits
    // by doing the syscall "exit"
}

// Data structures needed for the console test.  Threads making
// I/O requests wait on a Semaphore to delay until the I/O completes.

static Console *console;
static Semaphore *readAvail;
static Semaphore *writeDone;

//----------------------------------------------------------------------
// ConsoleInterruptHandlers
// 	Wake up the thread that requested the I/O.
//----------------------------------------------------------------------

static void ReadAvail(int arg) {
    readAvail->V();
}
static void WriteDone(int arg) {
    writeDone->V();
}

//----------------------------------------------------------------------
// ConsoleTest
// 	Test the console by echoing characters typed at the input onto
//	the output.  Stop when the user types a 'q'.
//----------------------------------------------------------------------

void
ConsoleTest (char *in, char *out)
{
    char ch;

    console = new Console(in, out, ReadAvail, WriteDone, 0);
    readAvail = new Semaphore("read avail", 0);
    writeDone = new Semaphore("write done", 0);

    for (;;) {
        readAvail->P();		// wait for character to arrive
        ch = console->GetChar();
        console->PutChar(ch);	// echo it!
        writeDone->P() ;        // wait for write to finish
        if (ch == 'q') return;  // if q, quit
    }
}
 
// addrspace.h
//	Data structures to keep track of executing user programs
//	(address spaces).
//
//	For now, we don't keep any information about address spaces.
//	The user level CPU state is saved and restored in the thread
//	executing the user program (see thread.h).
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation
// of liability and disclaimer of warranty provisions.

#ifndef ADDRSPACE_H
#define ADDRSPACE_H

#include "copyright.h"
#include "filesys.h"
#include "Pcb.h"
//#include "memorymanager.h"
#define UserStackSize		1024 	// increase this as necessary!
#include "noff.h"
#define UserArgvSize                340     // 10 30bytes strings tops and 40 bytes for their head



class AddrSpace {
public:
    AddrSpace();	// Create an address space,
    					// initializing it with the program
    					// stored in the file "executable"
    ~AddrSpace();			// De-allocate an address space

    void InitRegisters();		// Initialize user-level CPU registers,
    					// before jumping to user code
    void ForkInitRegisters();
    void SaveState();			// Save/restore address space-specific
    void RestoreState();		// info on a context switch
    int Initialize(OpenFile *executable,int argc, char ** argv); //Initialize the address space
    void Clearmap();

    Pcb *getpcb(){return pcb;}
    //int WriteMemory(OpenFile *executable, Segment data_seg);
    int TestEntry(int virt_addr);
    void WriteToUser(int UserAddress, int value,int size);
    //void pageFaultIn();
    unsigned int getpagetablesize() {return numPages;};
    TranslationEntry * getpagetable(unsigned int index){return &pageTable[index];};
 //
    int LoadHelper(Segment seg, int occasion, int page_start, int page_end, int page_size, char *phys_addr);
    int LocateSegment(Segment seg, int page_start, int page_end);
    void FaultPageIn();  
//sheng kuang

private:
    TranslationEntry *pageTable;	// Assume linear page table translation
    // for now!
    Pcb *pcb;
    unsigned int numPages;		// Number of pages in the virtual
    // address space
    int regargc;
    int regargv;
    int threadNum;
    int regArray[5];
    int stackArray[5];
    NoffHeader noffH;
    //OpenFile * as_executable;  
//shengkuang
    OpenFile *executable;
};

#endif // ADDRSPACE_H

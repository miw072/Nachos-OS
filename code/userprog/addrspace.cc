// addrspace.cc
//	Routines to manage address spaces (executing user programs).
//
//	In order to run a user program, you must:
//
//	1. link with the -N -T 0 option
//	2. run coff2noff to convert the object file to Nachos format
//		(Nachos object code format is essentially just a simpler
//		version of the UNIX executable object code format)
//	3. load the NOFF file into the Nachos file system
//		(if you haven't implemented the file system yet, you
//		don't need to do this last step)
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation
// of liability and disclaimer of warranty provisions.
#include "memorymanager.h"
#include "copyright.h"
#include "system.h"
#include "addrspace.h"
//#include "noff.h"
#ifdef HOST_SPARC
#include <strings.h>
#endif

extern Statistics * stats;
extern TranslationEntry **memownertable;
extern int *memownerprotable;
extern List *memoryList;
//----------------------------------------------------------------------
// SwapHeader
// 	Do little endian to big endian conversion on the bytes in the
//	object file header, in case the file was generated on a little
//	endian machine, and we're now running on a big endian machine.
//----------------------------------------------------------------------

static void
SwapHeader (NoffHeader *noffH)
{
    noffH->noffMagic = WordToHost(noffH->noffMagic);
    noffH->code.size = WordToHost(noffH->code.size);
    noffH->code.virtualAddr = WordToHost(noffH->code.virtualAddr);
    noffH->code.inFileAddr = WordToHost(noffH->code.inFileAddr);
    noffH->initData.size = WordToHost(noffH->initData.size);
    noffH->initData.virtualAddr = WordToHost(noffH->initData.virtualAddr);
    noffH->initData.inFileAddr = WordToHost(noffH->initData.inFileAddr);
    noffH->uninitData.size = WordToHost(noffH->uninitData.size);
    noffH->uninitData.virtualAddr = WordToHost(noffH->uninitData.virtualAddr);
    noffH->uninitData.inFileAddr = WordToHost(noffH->uninitData.inFileAddr);
}

//----------------------------------------------------------------------
// AddrSpace::AddrSpace
// 	Create an address space to run a user program.
//	Load the program from a file "executable", and set everything
//	up so that we can start executing user instructions.
//
//	Assumes that the object code file is in NOFF format.
//
//	First, set up the translation from program memory to physical
//	memory.  For now, this is really simple (1:1), since we are
//	only uniprogramming, and we have a single unsegmented page table
//
//	"executable" is the file containing the object code to load into memory
//----------------------------------------------------------------------

AddrSpace::AddrSpace()


{ 
	threadNum = 0;	//thread number of current process (max to be 4 as defined)
	pcb = new Pcb;


}

//----------------------------------------------------------------------
// AddrSpace::~AddrSpace
// 	Dealloate an address space. Delete table and pcb.
//----------------------------------------------------------------------

AddrSpace::~AddrSpace()
{
    delete [] pageTable;
    delete pcb;
    //delete as_executable;
    delete executable;
}

//----------------------------------------------------------------------
// AddrSpace::Initialize
// 	Create an address space to run a user program.
//	Load the program from a file "executable", and set everything
//	up so that we can start executing user instructions.
//
//	Assumes that the object code file is in NOFF format.
//
//	First, set up the translation from program memory to physical
//	memory. Then write the memory using WriteMemory().
//
//	"executable" is the file containing the object code to load into memory
//----------------------------------------------------------------------

extern MemoryManager *memorymanager;
int // 0-ERROR 1-NO ERROR
AddrSpace::Initialize(OpenFile *executable2,int argc, char ** argv)
{
   
//    NoffHeader noffH;
    unsigned int i, size;
         executable = executable2;
         executable->ReadAt((char *)&noffH, sizeof(noffH), 0);
        if ((noffH.noffMagic != NOFFMAGIC) && (WordToHost(noffH.noffMagic) == NOFFMAGIC))
         SwapHeader(&noffH);
        ASSERT(noffH.noffMagic == NOFFMAGIC);

// how big is address space?
    size = noffH.code.size + noffH.initData.size + noffH.uninitData.size
           + UserStackSize;	// we need to increase the size
    				// to leave room for the stack
    numPages = divRoundUp(size, PageSize);
    size = numPages * PageSize;

    //ASSERT(numPages <= NumPhysPages);		// check we're not trying
    // to run anything too big --
    // at least until we have
    // virtual memory
    DEBUG('a', "code size: %d data size: %d unin data size: %d stack size: %d\n", noffH.code.size, noffH.initData.size, noffH.uninitData.size, UserStackSize);
    DEBUG('a', "PageSize: %d\n", PageSize);
    DEBUG('x', "Initializing address space, num pages %d, size %d\n",
          numPages, size);

    // first, set up the translation
    pageTable = new TranslationEntry[numPages];
    for (i = 0; i < numPages; i++) {
        pageTable[i].virtualPage = i;			// virtual page # from 0 to table size

        pageTable[i].valid = FALSE;      	
        pageTable[i].use = FALSE;
        pageTable[i].dirty = FALSE;
        pageTable[i].readOnly = FALSE;  // if the code segment was entirely on
        				// a separate page, we could set its
        				// pages to be read-only
    }

    regargc=0;
    if(argc>0){
        regargc=argc;                                                             
        DEBUG('h',"argc: %d\n", argc);
        int UserArgvAddr = numPages * PageSize-UserArgvSize-UserStackSize;             //calculate the address to store argv
        int UserArgvAddrHead = numPages * PageSize-UserArgvSize-UserStackSize;
        DEBUG('h',"addr: %d\n", UserArgvAddr);
        for (int k=0;k<argc;k++)
        { char * argvstring = argv[k];                                                 //string
          int Useraddr=UserArgvAddr;
          do{
          WriteToUser(Useraddr,int (*argvstring),1);                                   //write to the child process address space
          argvstring++;
          Useraddr++; 
          }while (*(argvstring-1)!='\0');
          UserArgvAddr+=30;                                                            //next string
          DEBUG('h',"UserArgvAddr: %d\n", UserArgvAddr);
         }
        regargv=UserArgvAddr;                                                         //head of strings (argv)
        DEBUG('h',"regargv : %d\n", regargv);
        for (int k=0;k<argc;k++)
        { 
          WriteToUser(UserArgvAddr,UserArgvAddrHead,4);                               //write the address of the head of the string
          UserArgvAddr+=4;
          UserArgvAddrHead+=30;                                                       
          DEBUG('h',"UserArgvAddr: %d\n", UserArgvAddr);
         }
    }

    return 1;
}

//---------------------------------------------------------------------- project 2 load
// AddrSpace::WriteMemory
// 	Write physical memory.
//      First check if very begining virtual address of the segment data
//	falls on boundary. If yes fill in the physical page with integer 
//	number of pages with same PageSize and then fills in the rest(less
//	than a page data, if any, separately). If the very begining virt_addr
//	doesn't fall on boundary, first fills in the rest part of physical 
//	page(if the whole data size is smaller than rest physical page, fills
//	in just enough), then the new virt_addr will fall on boundary and 
//	go through the routine for that as mentioned above.
//	For every virt_addr and updated one, check they are valid using 
//	TestEntry().
//----------------------------------------------------------------------

/*
int
AddrSpace::WriteMemory(OpenFile *executable, Segment data_seg)
{
    DEBUG('a', "data copy begin\n");
    unsigned int numFull = (unsigned) data_seg.size/PageSize;	//number of full pages of data size
    DEBUG('a',"numFull is %d\n", numFull);
    unsigned int partial = (unsigned) data_seg.size%PageSize;	//rest of the data less than a full page
    DEBUG('a',"partial is %d\n", partial);
    int file_off = data_seg.inFileAddr;				//offset address in file
    DEBUG('a',"infileaddr is %d\n", file_off);
    int virt_addr = data_seg.virtualAddr;			//very begining virtual address of data segment
    DEBUG('a',"virt_addr is %d\n",virt_addr );
    unsigned int count = (unsigned) virt_addr/PageSize;		//virtual address's corresponding page number
    DEBUG('a',"count is %d\n",count);
    unsigned int partial_pre = (unsigned) virt_addr%PageSize;	//value virtual address falls off boundary
    DEBUG('a',"partial_pre: %d\n",partial_pre);
    int datawritten;						//the actual size of data written each time

    //test valid address
    if(!TestEntry(virt_addr)){
	DEBUG('a', "virt_addr: %d error\n", virt_addr);
            return 0;
    }
        DEBUG('a', "virt_addr: %d valid\n", virt_addr);

    //handle very begining virt_addr not fall on boundary
    if(partial_pre != 0){
        DEBUG('a', "data 0 done\n");
	char *phys_addr = &machine->mainMemory[pageTable[count].physicalPage*PageSize+partial_pre];
        DEBUG('a', "data 1 done\n");
	unsigned int PageSizeLeft = PageSize - partial_pre;

        //if data whole size large enough to fill in at least rest one page
	if(PageSizeLeft<(unsigned)data_seg.size){
	    datawritten = executable->ReadAt(phys_addr, PageSize-partial_pre, file_off);
	    count++;
	}

	//if data whole size is small and won't fill in the rest one page
	else{
	    datawritten = executable->ReadAt(phys_addr, data_seg.size, file_off);
	}
	file_off += datawritten;

	//update numFull and partial
        numFull = (data_seg.size-(datawritten))/PageSize;
        partial = (data_seg.size-(datawritten))%PageSize;
    }

    //handle virt_addr falls on boundary and at least a PageSize data
    //fills in integer number of pages based on numFull
    unsigned int countorig = count; 
    while(count<countorig+numFull){
	if(!TestEntry(count*PageSize)){
            DEBUG('a', "virt_addr: %d error\n", count*PageSize);
	    return 0;
	}
	char *phys_addr = &machine->mainMemory[pageTable[count].physicalPage*PageSize];
        DEBUG('a', "code 2 done,%d,%d\n", count,file_off);
	executable->ReadAt(phys_addr, PageSize, file_off);
	file_off += PageSize;
	count++;
    }
    DEBUG('a', "code & written size is:%d ,%d\n",data_seg.size,file_off);

    //handle if there is data less than a PageSize to be written
    if(partial!=0){	
	if(!TestEntry(count*PageSize)){
            DEBUG('a', "virt_addr: %d error\n", count*PageSize);
	    return 0;
	}
	char *phys_addr = &machine->mainMemory[pageTable[count].physicalPage*PageSize];
	executable->ReadAt(phys_addr, partial, file_off);
	file_off += partial;
    }
    DEBUG('a', "code & written size is:%d ,%d\n",data_seg.size,file_off);
    
    //if no error, return 1 as success
    return 1;
}

*/

void 

AddrSpace::WriteToUser(int UserAddress, int value,int size) {                              //write to the child process address space
        unsigned int count = (unsigned) UserAddress/PageSize;
        unsigned int partial_pre = (unsigned) UserAddress%PageSize;
        int physicalAddress=pageTable[count].physicalPage*PageSize+partial_pre;            //translate           
    switch (size) {
    case 1:
        machine->mainMemory[physicalAddress] = (unsigned char) (value & 0xff);
        break;

    case 2:
        *(unsigned short *) &machine->mainMemory[physicalAddress]
            = ShortToMachine((unsigned short) (value & 0xffff));
        break;

    case 4:
        *(unsigned int *) &machine->mainMemory[physicalAddress]
            = WordToMachine((unsigned int) value);
        break;

    default:
        ASSERT(FALSE);
    }
}

//----------------------------------------------------------------------
// AddrSpace::TestEntry
// 	Test valid virtual address for alignment and VPN errors
//----------------------------------------------------------------------

int //0-ERROR 1-NO ERROR
AddrSpace::TestEntry(int virt_addr)
{
     unsigned int vpn = (unsigned) virt_addr/PageSize;

    // check for alignment errors
    if (((PageSize == 4) && (virt_addr & 0x3)) || ((PageSize == 2) && (virt_addr & 0x1))) {
        DEBUG('a', "alignment problem at %d, size %d!\n", virt_addr, PageSize);
        return 0;
    }

    // check for vpn errors
    if (vpn >= numPages) {
            DEBUG('a', "virtual page # %d too large for page table size %d!\n",
                  virt_addr, numPages);
            return 0;
    }else if(!pageTable[vpn].valid){
            DEBUG('a', "virtual page # %d too large for page table size %d!\n",
                  virt_addr, numPages);
            return 0;
    }

    unsigned int pageFrame = pageTable[vpn].physicalPage;
    // if the pageFrame is too big, there is something really wrong!
    // An invalid translation was loaded into the page table or TLB.
    if (pageFrame >= NumPhysPages) {
        DEBUG('a', "*** frame %d > %d!\n", pageFrame, NumPhysPages);
        return 0;
    }

    return 1;
}    
//----------------------------------------------------------------------
// AddrSpace::InitRegisters
// 	Set the initial values for the user-level register set.
//
// 	We write these directly into the "machine" registers, so
//	that we can immediately jump to user code.  Note that these
//	will be saved/restored into the currentThread->userRegisters
//	when this thread is context switched out.
//----------------------------------------------------------------------

void
AddrSpace::InitRegisters()
{
	currentThread->setThdNum(threadNum);    
	int i;

    for (i = 0; i < NumTotalRegs; i++)
        machine->WriteRegister(i, 0);
    if (regargc>0)                                                           //pass the parameters to main
       {machine->WriteRegister(4, regargc);
        machine->WriteRegister(5, regargv);
        DEBUG('h',"write to R5 : %d\n", regargv);
       }

    // Initial program counter -- must be location of "Start"
    machine->WriteRegister(PCReg, 0);

    // Need to also tell MIPS where next instruction is, because
    // of branch delay possibility
    machine->WriteRegister(NextPCReg, 4);

    // Set the stack register to the end of the address space, where we
    // allocated the stack; but subtract off a bit, to make sure we don't
    // accidentally reference off the end!
    machine->WriteRegister(StackReg, numPages * PageSize - 16);
    DEBUG('a', "Initializing stack register to %d\n", numPages * PageSize - 16);
}

//----------------------------------------------------------------------
// AddrSpace::ForkInitRegisters
//	Set initial values for each thread Fork()ed in a process
//	Size of 64 is assigned for thread stack
//----------------------------------------------------------------------
void
AddrSpace::ForkInitRegisters()
{
    threadNum++;				//increase thread number count
    currentThread->setThdNum(threadNum);	//assign thread count number to thread's instance thdnum
    machine->WriteRegister(StackReg, numPages * PageSize - 16 -threadNum*64);
    DEBUG('w', "Initializing stack register to %d\n", numPages * PageSize - 16 - threadNum*64);
}

//----------------------------------------------------------------------
// AddrSpace::SaveState
// 	On a context switch, save any machine state, specific
//	to this address space or thread, that needs saving.
//	regArray[5] defined to store PCReg for each thread
//	stackArray[5] defined to store StackReg for each thread
//----------------------------------------------------------------------

void AddrSpace::SaveState()
{
    regArray[currentThread->getThdNum()]=machine->ReadRegister(PCReg);
    stackArray[currentThread->getThdNum()]=machine->ReadRegister(StackReg);
    DEBUG('w',"current thread: %s ,thd num: %d, saving pcreg: %d, saving stackreg: %d\n",currentThread->getName(),currentThread->getThdNum(),regArray[currentThread->getThdNum()],stackArray[currentThread->getThdNum()]);

}

//----------------------------------------------------------------------
// AddrSpace::RestoreState
// 	On a context switch, restore the machine state so that
//	this address space can run.
//
//      tell the machine where to find the page table.
//	restore the PCReg for each thread
//	restore the StackReg for each thread
//----------------------------------------------------------------------

void AddrSpace::RestoreState()
{
    machine->pageTable = pageTable;
    machine->pageTableSize = numPages;

	int regTmp = regArray[currentThread->getThdNum()];
        int stackTmp = stackArray[currentThread->getThdNum()];
    DEBUG('w',"current thread: %s ,thd num: %d, restoring pcreg: %d, restoring stackreg: %d\n",currentThread->getName(),currentThread->getThdNum(),regTmp,stackTmp);
    if(threadNum>0)
    {
	machine->WriteRegister(StackReg, stackTmp);
    }
    machine->WriteRegister(PCReg, regTmp);
    machine->WriteRegister(NextPCReg,regTmp+4);

}

void AddrSpace::Clearmap()
{
    unsigned int i;
    if (pageTable!=NULL){
    	for (i=0;i<numPages;i++){
                                memoryList->Find(&pageTable[i],0);
                                if (pageTable[i].valid==TRUE)
                                {                               
				memorymanager->FreePage(pageTable[i].physicalPage);	
                                pageTable[i].valid=FALSE;
                                //memownertable[pageTable[i].physicalPage]= NULL;
                                //memownerprotable[pageTable[i].physicalPage]=0;
                                
                                    }	
		}	
	}
}

//----------------------------------------------------------------------
// AddrSpace::FaultPageIn()
//      On a page fault exception(page not in backstore), handler will call FaultPageIn to
//      read bad virtual address from register, clear it corresponding 
//	physical page(already assigned in handler as in Part2), call LocateSegment()
//	to specify load occasion, call LoadHelper() to read executable into
//	into physical page based on different occasions. This is first done
//  	on Code, then on initData, for uninitData and stack, no option is 
//	made except for zero the page. The mechanism is for each faulted
//	virtual page, check if the code and initData in executable need to be loaded in,
//	if yes, load it, if else, just zero the allocated physical page. 
//----------------------------------------------------------------------
void 
AddrSpace::FaultPageIn()
{
    int virt_addr = machine->ReadRegister(BadVAddrReg);	 				//read bad virtual address
    unsigned int vpn = virt_addr/PageSize;	 	 				//calculate the corresponding page number
    
    bzero(&machine->mainMemory[(pageTable[vpn].physicalPage)*PageSize],PageSize); 	//zero physical page
    
    int page_VA_start = vpn*PageSize; 		 					//page start address
    int page_VA_end = page_VA_start + PageSize;  					//page end address
    int page_size = PageSize;			 					//page size
    char *phys_addr = &machine->mainMemory[pageTable[vpn].physicalPage*PageSize];  	//get physical page address
    
    int codepagein=0;
    int datapagein=0;
    int DataWritten;				 					//store how much data written
    int case_code = LocateSegment(noffH.code, page_VA_start, page_VA_end);  		//get which case code's page(s) is located, related to faulted page
    
    if (noffH.code.size>0)								//check code has data
    {
        DataWritten = LoadHelper(noffH.code, case_code, page_VA_start, page_VA_end, page_size, phys_addr); //load executable (code) into page based on case  
        if(DataWritten!=0)
        {codepagein++;}
    }
    DEBUG('h',"DataWritten of code is %d\n",DataWritten);
    
    if(DataWritten<PageSize)  								//if code segment load doesn't fills up the page, continue for initData
    {
	DEBUG('h',"loading data\n");
	page_VA_start = page_VA_start + DataWritten;  					//update page start address
        page_size = PageSize - DataWritten;	      					//update page size left
	phys_addr = &machine->mainMemory[pageTable[vpn].physicalPage*PageSize+DataWritten]; //get new physical page address to start writting
	int case_data = LocateSegment(noffH.initData, page_VA_start, page_VA_end);  	//get which case initData's page(s) is located, related to faulted page
        if (noffH.initData.size>0)
        { 
	    DataWritten = LoadHelper(noffH.initData, case_data, page_VA_start, page_VA_end, page_size, phys_addr); //load executable (initData) into page
            if(DataWritten!=0)
            {datapagein++;}
        }
    }

    if((codepagein>0)||(datapagein>0))
        stats->numPageIns++;

    pageTable[vpn].valid = TRUE;   							//set valid bit to true so that re-run won't fault
    pageTable[vpn].dirty = FALSE;  							//set dirty to false
    memownertable[pageTable[vpn].physicalPage]= &pageTable[vpn];                        //keep the pte that owns the physicalpage
    memownerprotable[pageTable[vpn].physicalPage]=currentThread->space->getpcb()->getpid();  //keep the process that owns the physicalpage
    memoryList->Append(&pageTable[vpn]);                                                //put the pte into the List for FIFO

}

//----------------------------------------------------------------------
// AddrSpace::LoadHelper
//	load the executable into physical page based on the cases got from
//	LocateSegment(). Only case 1-4 are affecting executable loading.
//	return how much data written into physical page. 
//----------------------------------------------------------------------
int
AddrSpace::LoadHelper(Segment seg, int occasion, int page_start, int page_end, int page_size, char *phys_addr)
{
    int data_start = seg.virtualAddr;			//segment's starting address
    int data_end = seg.virtualAddr + seg.size;		//segment's ending address
    int infile = seg.inFileAddr;			//segment's in file address
    int DataWritten = 0;
    int WriteSize;
    switch(occasion)
    {
	case 1://segment starts outside page, ends inside page
	    DEBUG('h',"case 1\n");
	    WriteSize = data_end - page_start;
	    DataWritten = executable->ReadAt(phys_addr, WriteSize, infile+(seg.size-WriteSize));
            DEBUG('h',"write %d, written %d\n",WriteSize,DataWritten);
	    break;
	case 2://segment includes page
	    DEBUG('h',"case 2\n");
	    WriteSize = page_size;
	    DataWritten = executable->ReadAt(phys_addr, WriteSize, infile+(page_start-data_start));
            DEBUG('h',"write %d, written %d\n",WriteSize,DataWritten);
	    break;
	case 3://segment included in page
    	    DEBUG('h',"case 3\n");
	    WriteSize = seg.size;
	    DataWritten = executable->ReadAt(phys_addr, WriteSize, infile);
            DEBUG('h',"write %d, written %d\n",WriteSize,DataWritten);
	    break;
	case 4://segment starts inside page, ends outside page
	    DEBUG('h',"case 4\n");
	    WriteSize = page_end - data_start;
            DEBUG('h',"pageend %d, data_start %d\n",page_end,data_start);
	    DataWritten = executable->ReadAt(phys_addr, WriteSize, infile);
            DEBUG('h',"write %d, written %d\n",WriteSize,DataWritten);
	    break;
	default:
	    DEBUG('h',"no load needed\n");
	    break; 
    }
    return DataWritten;
}

//----------------------------------------------------------------------
// AddrSpace::LocateSegment
//	to see where a segment(code or initData) is located related 
//	to faulted page. 
//	0-outside the page, exclude page; 
//	1-segment starts outside page,ends inside page; 
//	2-segment starts outside page, ends outside page, includes page; 
//	3-whole segment included in page scale; 
//	4-segment starts inside page, ends outside page; 
//	5-else, exclusive
//
//----------------------------------------------------------------------
int
AddrSpace::LocateSegment(Segment seg, int page_start, int page_end)
{
    int seg_start = seg.virtualAddr;
    int seg_end = seg.virtualAddr + seg.size;
    if(seg_start<page_start && seg_end<page_start)
    {
	return 0;
    }
    else if(seg_start<page_start && seg_end>=page_start && seg_end<=page_end)
    {
	return 1;
    }
    else if(seg_start<page_start && seg_end>page_end)
    {
	return 2;
    }
    else if(seg_start>=page_start && seg_end<=page_end)
    { 
	return 3;
    }
    else if(seg_start>=page_start && seg_start<=page_end && seg_end>page_end)
    {
	DEBUG('h',"segstart is %d, page start is %d, so case 4\n",seg_start,page_start);
	return 4;
    }
    else 
    {
	return 5;
    }
}









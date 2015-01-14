// exception.cc
//	Entry point into the Nachos kernel from user programs.
//	There are two kinds of things that can cause control to
//	transfer back to here from user code:
//
//	syscall -- The user code explicitly requests to call a procedure
//	in the Nachos kernel.  Right now, the only function we support is
//	"Halt".
//
//	exceptions -- The user code does something that the CPU can't handle.
//	For instance, accessing memory that doesn't exist, arithmetic errors,
//	etc.
//
//	Interrupts (which can also cause control to transfer from user
//	code into the Nachos kernel) are handled elsewhere.
//
// For now, this only handles the Halt() system call.
// Everything else core dumps.
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation
// of liability and disclaimer of warranty provisions.

#include <stdlib.h>
#include "copyright.h"
#include "system.h"
#include "syscall.h"
#include "Table.h"
#include "SynchConsole.h"
#include "memorymanager.h"
#define MAXNAMESIZE 64
//----------------------------------------------------------------------
// ExceptionHandler
// 	Entry point into the Nachos kernel.  Called when a user program
//	is executing, and either does a syscall, or generates an addressing
//	or arithmetic exception.
//
// 	For system calls, the following is the calling convention:
//
// 	system call code -- r2
//		arg1 -- r4
//		arg2 -- r5
//		arg3 -- r6
//		arg4 -- r7
//
//	The result of the system call, if any, must be put back into r2.
//
// And don't forget to increment the pc before returning. (Or else you'll
// loop making the same system call forever!
//
//	"which" is the kind of exception.  The list of possible exceptions
//	are in machine.h.
//----------------------------------------------------------------------
class Pcb;
extern MemoryManager *memorymanager;
extern TranslationEntry **memownertable;
extern int *memownerprotable;
extern List*memoryList;
extern Statistics * stats;
extern char* algorithmMode;

void SystemCall(int type);
void SysHalt();
void SysExit();
void SysExec();
void SysRead();
void SysWrite();
void SysJoin();
void SysFork();
void SysYield();
void ForkStart(int RegAddr);
void UserToKernelByString(int UserAddress, char *KernelAddress);
void KernelToUserByString(char *KernelAddress, int UserAddress);
void UserToKernelByNum(int UserAddress, int Num, char *KernelAddress);
void KernelToUserByNum(char *KernelAddress, int Num, int UserAddress);

void ProcessStart(int para);
void PCincrease() ;
void realexit();

void PageFaultHandler();
void ReadOnlyHandler();
void BusErrorHandler();
void AddressErrorHandler();
void OverflowHandler();
void IllegalInstrHandler();

int findvictim();

void
ExceptionHandler(ExceptionType which)
{
    int type = machine->ReadRegister(2);
    
    switch (which)
    {
    case NoException:
	 printf("No exception\n");
	 break;
    case SyscallException: 
         SystemCall(type);
	 break;
    case PageFaultException:
	 PageFaultHandler();
         break;
    case ReadOnlyException:
	 ReadOnlyHandler();
	 break;
    case BusErrorException:
	 BusErrorHandler();
	 break;
    case AddressErrorException:
	 AddressErrorHandler();
	 break;
    case OverflowException:
	 OverflowHandler();
	 break;
    case IllegalInstrException:
	 IllegalInstrHandler();
	 break;
    default :
         break;
    }
}


void SystemCall(int type)
{ 
    switch(type)
    {
    case SC_Halt :
	 SysHalt();
	 break;
    case SC_Exit :
         SysExit();
         break;
    case SC_Exec :
         SysExec();
         break;
	case SC_Read:
		 SysRead();
		 break;
	case SC_Write:
		 SysWrite();
		 break;
    case SC_Join:
		 SysJoin();
		 break;
    case SC_Fork:
	     SysFork();
	     break;
    case SC_Yield:
	     SysYield();
	     break;
    default :
         break;		 
    }



}


void SysHalt()
{
   DEBUG('a',"Shutdown, initiated by user program. \n");
   DEBUG('a',"PID %d\n",currentThread->space->getpcb()->getpid());
   interrupt->Halt();
}

void SysExit()
{
	int p = machine->ReadRegister(4);
	DEBUG('x',"Parameters: %d\n", p);

	currentThread->space->getpcb()->setexitstatus(p);    //store in the pcb for join system call

	realexit();
}


extern Table * processtable;
extern int processcount;
void SysExec()
{
	int RegAddr;
	char * filename;
	OpenFile * executable;
	AddrSpace * space;
	Thread * newthread;
	SpaceId pid;
	filename = new char [MAXNAMESIZE];
        int joinpar;
    
	RegAddr = machine->ReadRegister(4);
	UserToKernelByString(RegAddr, filename);
        executable = fileSystem->Open(filename);

    
	if (executable == NULL) {
        printf("Unable to open file %s\n", filename);
        machine->WriteRegister(2, 0);
        PCincrease();        
		return;
    }
        DEBUG('w',"gogogogo!\n");

	    int argc =machine->ReadRegister(5);              //read int argc 
        int argvaddr =machine->ReadRegister(6);              //read argv (virtual address)
        char **argv;
        argv= new char* [10];                               //max 10strings and each of them 30 bytes top 
        for (int j=0;j<10;j++)
        {argv[j]=new char [30];}
        //argv[0]=filename;
        for (int k=0;k<argc;k++)                            
        { int a;
          //DEBUG('h',"argvaddr is %d \n",argvaddr);
          if (machine->ReadMem(argvaddr, 4, &a)==FALSE)
             machine->ReadMem(argvaddr, 4, &a);                //read the address of the head of one string
          DEBUG('h',"a is %d \n",a);
          UserToKernelByString (a, argv[k]);                //read one string 
          argvaddr+=4;                                      //increase argv to read next string

          DEBUG('h',"argv is %s \n",argv[k]);
         } 

	space = new AddrSpace;

    int errorstatus = space->Initialize(executable,argc,argv);    // pass argc and argv to initialize address space

    if ( errorstatus == 0) {
		machine->WriteRegister(2, 0);
		PCincrease();        
		return;
		}    
	joinpar = machine->ReadRegister(7);

    if ((joinpar==0)||(joinpar==1))
    {       
        newthread = new Thread(filename, joinpar);
      	newthread->space = space;
        pid = processtable->Alloc(newthread->space->getpcb());
     }

    else if (joinpar==2)                                            //head of a pipe
    {   newthread = new Thread(filename, 0);                     
	newthread->space = space;
        pid = processtable->Alloc(newthread->space->getpcb());
        currentThread->space->getpcb()->pipeinitialize();           //initialize a table in the parent process
        currentThread->space->getpcb()->appendpipe(pid);            //put the child pid into the table
      }
    else if (joinpar==3)                                            //middle of a pipe
    {   newthread = new Thread(filename, 0);
	newthread->space = space;
        pid = processtable->Alloc(newthread->space->getpcb());       
        newthread->space->getpcb()->setlastpipe(currentThread->space->getpcb()->findlast());  //find the pid on the top of the table
        currentThread->space->getpcb()->appendpipe(pid);            //put the child pid into the table
      }
     else if(joinpar==4)                                            //end of a pipe
    {   newthread = new Thread(filename, 0);
	      newthread->space = space;
        pid = processtable->Alloc(newthread->space->getpcb());      
        newthread->space->getpcb()->setlastpipe(currentThread->space->getpcb()->findlast());  //find the pid on the top of the table
        currentThread->space->getpcb()->freepipe();                 //delete the table

      }
     else
     { 
	   PCincrease();        
       return;
      }
	
	      newthread->space->getpcb()->setpid(pid);
              newthread->space->getpcb()->backstoreinitialize(newthread->space->getpagetablesize(),pid);
        newthread->space->getpcb()->setjoinable(joinpar);

	machine->WriteRegister(2, pid);
    //delete executable;
    
    processcount++;
    
	newthread->Fork(ProcessStart, 0);
    PCincrease();
}


/* Read "size" bytes from the console(keyboard) into "buffer".*/

extern SynchConsole *synchcon;
void SysRead()
{
    int ReadToAddr;
    int size;
    int io;
	char *buffer;
    int pipeline;
	
	
    ReadToAddr = machine->ReadRegister(4);
    size = machine->ReadRegister(5);
    io = machine->ReadRegister(6);
    buffer = new char[size];
    pipeline=currentThread->space->getpcb()->getjoinable();
  if ((pipeline==0)||(pipeline==1))
  {
	if (io==ConsoleInput){
		for (int i=0;i<size;i++){

			buffer[i] = synchcon->GetChar();		 //get char from console to buffer[]
		}
	KernelToUserByNum(buffer, size, ReadToAddr);	 //transfer from kernel to user
	}
	else {
        machine->WriteRegister(2, -1);
		printf("Error: Ilegal Use of Read");
        PCincrease();        
		return;
	}
  }

  else if(pipeline==2)                                     //head of the pipe
  {   for (int i=0;i<size;i++)
     {buffer[i] = synchcon->GetChar();                     //read the console
      DEBUG('p',"buffer %c\n",buffer[i]);
     }
      KernelToUserByNum(buffer, size, ReadToAddr);         //write to user process
  }
  else if((pipeline==3)||(pipeline==4))                    //middle or end of the pipe
  {   DEBUG('p',"pp in");
      int pipeId = currentThread->space->getpcb()->getlastpipeId();  // get pid of the last pipe
      Pcb * lastpcb = (Pcb *) processtable->Get(pipeId);
      DEBUG('p',"pipeId %d\n",pipeId);
      for (int i=0;i<size;i++)
      { 

	buffer[i] = lastpcb->getpipebuffer()->Read();      //read from the last pipe (bounded buffer)
        DEBUG('p',"buffer %c\n",buffer[i]);
       }
     KernelToUserByNum(buffer, size, ReadToAddr);          //write to user process
  }
	delete buffer;
	PCincrease();
}


/* Write "size" bytes from "buffer" to the console(display). */
void SysWrite()
{
	int WriteFromAddr;
	int size;
	int io;
	char *buffer;
    int pipeline;

	pipeline=currentThread->space->getpcb()->getjoinable();
    WriteFromAddr = machine->ReadRegister(4);
    size = machine->ReadRegister(5);
    io = machine->ReadRegister(6);
	buffer = new char[size];


	UserToKernelByNum(WriteFromAddr, size, buffer); //transfer from user to kernel
if ((pipeline==0)||(pipeline==1))
 {
	if (io==ConsoleOutput){
		for (int i=0; i<size; i++){
			synchcon->PutChar(buffer[i]);     //put buffer[] into console
		}
	}
	else {
		machine->WriteRegister(2, -1);
		printf("Error: Ilegal Use of Write");
		PCincrease();        
		return;
	}
 }
 else if ((pipeline==2)||(pipeline==3))                       //head or middle of the pipe
 {  for (int i=0; i<size; i++)
   {
    currentThread->space->getpcb()->getpipebuffer()->Write(buffer[i]);   // write to its own pipe (bounded buffer)
    DEBUG('p',"wbuffer %c\n",buffer[i]);
   }
 }
 else if (pipeline==4)                                        //end of the pipe
 {  for (int i=0; i<size; i++){
		synchcon->PutChar(buffer[i]);                 //write to console
		}
  }
	delete buffer;
	PCincrease();
}


void SysJoin()
{
	SpaceId id;

	id = machine->ReadRegister(4);
	Pcb *pcbtmp;
        pcbtmp = new Pcb();
        pcbtmp =(Pcb*)processtable->Get(id);               //track pcb according to the id
	if (pcbtmp == NULL) {                                  //check valid id
		printf("Error: invalid id\n");
		machine->WriteRegister(2, -65535);
		PCincrease();        
		return;
	}	
	if (pcbtmp->getjoinable() == 0){
		printf("Error: id:%d process is not joinable\n", id);  //check joinable
		machine->WriteRegister(2, -65535);
		PCincrease();        
		return;
	}
	else {
		pcbtmp->semaForExit->V();                    //signal the child to finish
		while(pcbtmp->getfinish() != 1){
			pcbtmp->semaForBlock->P();               //wait for child is done                   
		}
		machine->WriteRegister(2,pcbtmp->getexitstatus());
		pcbtmp->semaForStatus->V();                  //after wirte back the parameter, signal child to finish
	}	
	PCincrease();
}

//for illustration purpose, define threadCount to count number of thread created
//			    define threadName[] to store the names to assign each thread
//			    for now, should create no more than 4 threads
int threadCount =0;
char *threadName[] = {"1","2","3","4","5"};
void SysFork()
{
    int RegAddr = machine->ReadRegister(4);								//read in function pointer
    DEBUG('t',"Forking new thread: %s\n",threadName[threadCount++]);
    Thread *thd = new Thread(threadName[threadCount++],0);
    thd->space = currentThread->space;									//let Fork()ed thread share the same address space with the process
    thd->Fork(ForkStart, RegAddr);
    currentThread->Yield();
    PCincrease();
}

//ForkStart() assists SysFork() to run passed user function
void ForkStart(int RegAddr)
{
    currentThread->space->ForkInitRegisters();
    machine->WriteRegister(PCReg, RegAddr);
    machine->WriteRegister(NextPCReg, RegAddr+4);
    machine->Run();
}

void SysYield()
{
    currentThread->Yield();
    PCincrease();  
}

void UserToKernelByNum(int UserAddress, int Num, char *KernelAddress) {
	int a;
	int i;
	for (i=0; i<Num; i++){
		if(machine->ReadMem(UserAddress, 1, &a)==FALSE)
                   machine->ReadMem(UserAddress, 1, &a);
        //DEBUG('a', "loop %d",i);
		 UserAddress++;
		*KernelAddress++ = (char)a;
	}
}

void KernelToUserByNum(char *KernelAddress, int Num, int UserAddress) {
	int i;	
	for (i=0; i<Num; i++){
		if (machine->WriteMem(UserAddress, 1, (int)*KernelAddress )==FALSE)
                     machine->WriteMem(UserAddress, 1, (int)*KernelAddress ); 
		KernelAddress++;
		UserAddress++;
	}
}

void UserToKernelByString (int UserAddress, char *KernelAddress) {
	int a;
    char last;
    int count = 0;
	do {
		//machine->ReadMem(UserAddress, 1, &a);
		if (machine->ReadMem(UserAddress, 1, &a) == FALSE) {
		    machine->ReadMem(UserAddress, 1, &a);
	        }
		UserAddress++;
		*KernelAddress++ = (char) a;
        last = *(KernelAddress-1);
        count++;
	} while (last != '\0'&& count<MAXNAMESIZE);
}


void realexit() {
	SpaceId pid;
	pid = currentThread->space->getpcb()->getpid();
	Pcb *pcbtmp;
        //pcbtmp = new Pcb();
        pcbtmp =(Pcb*)processtable->Get(pid);
	if (pcbtmp->getjoinable()==1)
	{
         pcbtmp->semaForExit->P();	       //wait for call join
         pcbtmp->setfinish(1);
         pcbtmp->semaForBlock->V();		   //signal parant
		 pcbtmp->semaForStatus->P();       //wait for the parameter be passed to Join
	}
	processtable->Release(pid);
    currentThread->space->Clearmap();
    currentThread->space->getpcb()->getbackstore()->killbs();
    DEBUG('x',"deleteing %d\n",currentThread->space->getpcb()->getpid());
	if (--(processcount))   
		currentThread->Finish();
	else
		interrupt->Halt(); 
}

void ProcessStart(int para){
	currentThread->space->InitRegisters();
    currentThread->space->RestoreState();
    machine->Run();
}

void PCincrease() {
	//machine->WriteRegister(PrevPCReg, machine->ReadRegister(PCReg)); 
        IntStatus oldLevel = interrupt->SetLevel(IntOff);
	machine->WriteRegister(PCReg, machine->ReadRegister(NextPCReg) ); 
	machine->WriteRegister(NextPCReg, machine->ReadRegister(NextPCReg) +4); 
        (void) interrupt->SetLevel(oldLevel);
}

void PageFaultHandler(){
	DEBUG('h',"Page fault exception: no valid translation found\n");     
        stats->numPageFaults++;
        int virtualAddr = machine->ReadRegister(BadVAddrReg);
        unsigned int virtualPage = (unsigned int)virtualAddr/PageSize;
        TranslationEntry* pte = currentThread->space->getpagetable(virtualPage);                //pte that invokes the pagefault
        int mmavb = memorymanager->AllocPage(); 
        if (mmavb!=-1){ 
        	pte->physicalPage =  mmavb;                                                     //free physical frame exits; give it to the pte
		}
          else{ int victimphysicalPage = findvictim();         //replacement                    //no free frame; find a victim
                //DEBUG('h',"vitim is %d\n",memownerprotable[victimphysicalPage]);
                Pcb * vicpcb= (Pcb *)processtable->Get(memownerprotable[victimphysicalPage]);   //find who owned the victim frame
                TranslationEntry* vicpte = memownertable[victimphysicalPage];
                vicpcb->getbackstore()->PageOut(vicpte);                                        //put the content of the frame to the victim's backstore
                pte->physicalPage = victimphysicalPage;                                         //give the frame to the new coming pte
		}


        if (currentThread->space->getpcb()->getbackstore()->Findpte(pte->virtualPage)==FALSE)                //first time page fault, no backstore
   {    DEBUG('h',"flag %d\n",currentThread->space->getpcb()->getbackstore()->Findpte(pte->virtualPage));    
        currentThread->space->FaultPageIn();                                                                 //FaultPageIn
        //stats->numPageIns++;
        //currentThread->space->pageFaultIn();
}
        else
        { DEBUG('h',"flag %d\n",currentThread->space->getpcb()->getbackstore()->Findpte(pte->virtualPage));   //page is already in backstore
          DEBUG('h',"BackStore Page In\n");  
          DEBUG('s',"process %d\n",currentThread->space->getpcb()->getpid());
          DEBUG('h',"virutual Page %d\n",pte->virtualPage);
          currentThread->space->getpcb()->getbackstore()->PageIn(pte,pte->physicalPage);                       //call backstore to get page in
          stats->numPageIns++;
         }
	//realexit();
}

void ReadOnlyHandler(){
	printf("Read only exception: attempt to \"write\" a \"read-only\"\n");
        realexit();
}

void BusErrorHandler(){
	printf("Bus error exception: translation results in invalid physical address\n");
        realexit();
}

void AddressErrorHandler(){
 	printf("Address error exception: unaligned reference or beyond end of address space\n");
        realexit();
}

void OverflowHandler(){
 	printf("Overflow exception: integer overflow in add or sub\n");
        realexit();
}

void IllegalInstrHandler(){
	printf("Illegal instruction exception: unimplemented or reserved instruction\n");
        realexit();
}

int  findvictim()
{  int f;
   TranslationEntry* vic;
   ListElement *element;
   bool flag;
   switch(*algorithmMode)
   { case '0':                                   //FIFO
vic=(TranslationEntry*)memoryList->Remove();     //FIFO list; remove the head of the list
f=vic->physicalPage;
break;
     case '1':                                   //LRU approximation :Second Chance
   flag = TRUE; 
   element = memoryList->findFirst();
   while (flag)
   {
   vic = (TranslationEntry*)element->item;
   if (vic->use==TRUE)                            //If referenced, offer a second chance
   {vic->use=FALSE;
    if (element->next==NULL)
    {element=memoryList->findFirst();
     DEBUG('u',"first out \n");}     
    else
    element=element->next;
    }
   else flag = FALSE;
   }
   memoryList->Find(vic,0);
   f=vic->physicalPage;  
   break;
   default:
   f=rand()%NumPhysPages;
   break;
  }  

return f;
}


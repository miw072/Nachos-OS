#include "copyright.h"
#include "Pcb.h"
#include "synch.h"
#include "stats.h"

extern Machine *machine;
extern TranslationEntry ** memownertable;
extern int * memownerprotable;
extern Thread * currentThread;
extern FileSystem * fileSystem;
extern List * memoryList;
extern Statistics* stats;

BackingStore:: BackingStore(unsigned int input, int value)
{  
   ptesize=input;         
   backpte = new bool[ptesize];                       //a shadow table to track which pte is in backstore
   int name = value;
   filename = new char[32];
   sprintf(filename, "%d", name);                     //use pid as the name of the backstore file
   fileSystem->Create(filename,ptesize*PageSize);
   backfile = fileSystem->Open(filename);
  for (unsigned int i=0;i<ptesize;i++)
   backpte[i]=FALSE;
}

BackingStore:: ~BackingStore()
{
 delete backfile;
 delete backpte;
}

void
BackingStore::killbs()                                //delete backstore file
{ fileSystem->Remove(filename);
}

void
BackingStore:: PageOut(TranslationEntry *pte)
{ int flag=Findpte(pte->virtualPage);
  char *phys_addr = &machine->mainMemory[pte->physicalPage*PageSize];              //calcualte the physical address
  if (flag>0)
  { if (pte->dirty == TRUE)                                                        //dirty page should copy to backstore again
    {DEBUG('h',"dirty\n");
     backfile->WriteAt(phys_addr , PageSize, PageSize*(pte->virtualPage));         //copy the page to backstore
     stats->numPageOuts++;
    }
  }
  else 
  {                                                                                //copy the page to the backstore (first time)
   backfile->WriteAt(phys_addr , PageSize, PageSize*(pte->virtualPage));
   backpte[pte->virtualPage]=TRUE;
   stats->numPageOuts++;
  }
 //memoryList->Find(pte,0);
 pte->valid=FALSE;
 pte->dirty=FALSE;
}

void
BackingStore::PageIn(TranslationEntry *pte, int physFrame)
{ //int flag=Findpte(pte->virtualPage);
 // if (flag >0)
//{
  char *phys_addr = &machine->mainMemory[physFrame*PageSize];                       //calcualte the physical address
  backfile->ReadAt(phys_addr, PageSize, (pte->virtualPage)*PageSize);               //copy to the frame from backstore
  memoryList->Append(pte);                                                          //put the memory to the list for FIFO
  memownertable[physFrame]=pte;                                                     //track which pte holds the frame
  memownerprotable[physFrame]=currentThread->space->getpcb()->getpid();             //track which process holds the frame
  pte->valid=TRUE;
  pte->dirty=FALSE;  
//}
//need handle later
}

bool
BackingStore::Findpte (int count)                                                   //track whether the pte is in backstore
{
   return backpte[count];
}

BoundedBuffer::BoundedBuffer(int n)
{
	Capacity = n;
    count = 0;
    incount = 0;
    outcount = 0;
	
	buffer = new char [Capacity];
	mutex = new Lock("lockmetex");
    cvnotfull = new Condition("cvnotfull");
    cvnotempty = new Condition("cvnotempty");
}

BoundedBuffer::~BoundedBuffer()
{
	delete mutex;
    delete cvnotfull;
    delete cvnotempty;
    delete buffer;
}

void 
BoundedBuffer::Write(char item)
{
	mutex->Acquire();
	while (count==Capacity)
    cvnotfull->Wait(mutex);

    buffer[incount] = item;
	incount = (incount+1)%Capacity;
    count++;

	cvnotempty->Signal(mutex);
    mutex->Release();
}

char 
BoundedBuffer::Read()
{       DEBUG('p',"Read\n");
	char tmp;
	mutex->Acquire();
	while (count == 0)
    cvnotempty->Wait(mutex);

	tmp = buffer[outcount];
	outcount = (outcount+1)%Capacity;
    count--;

	cvnotfull->Signal(mutex);
    mutex->Release();
    return tmp;
}

Pcb::Pcb()
{ 
	Pid = -1;
	semaForBlock = new Semaphore("semaForBlock",0);
        semaForExit = new Semaphore("semaForExit",0);
	semaForStatus = new Semaphore("semaForStatus",0);
        pipebuffer = new BoundedBuffer(64);   
        pipecount=0;
        lastpipeId=0;
}

Pcb::~Pcb()
{ 
	delete semaForBlock;
        delete semaForExit;
	delete semaForStatus;
        delete backstore;
  	delete pipebuffer;
}

SpaceId
Pcb::getpid()
{ 
	return Pid;
}

void
Pcb::setpid(int value)
{ 
	Pid=value;
}
/*
void setParent(Thread * newParent) 
{ 
	parent = newParent; 
}
*/
/*
Thread * getParent() 
{ 
	return parent; 
}
*/
void 
Pcb::setjoinable(int joinpar)
{ 
	joinable = joinpar;
}

int
Pcb::getjoinable()
{ 
	return joinable;
}

void 
Pcb::setfinish(int finishpar)
{ 
	finish = finishpar;
}

int
Pcb::getfinish()
{ 
	return finish;
}

void 
Pcb::setexitstatus(int status)
{ 
	exitstatus = status;
}

int
Pcb::getexitstatus()
{ 
	return exitstatus;
}

void

Pcb::pipeinitialize()         // initialize a table to store the pid of pipes; max size is 32
{ pipe = new int[32];
  pipecount=0;
  for (int i=0;i<32;i++)
  pipe[i]=0;
}

void 
Pcb::appendpipe(int value)   // put a pid into the pipe table
{
pipe[pipecount]=value;
pipecount++;
}


int  
Pcb::findlast()              //find the pid on the top of the table
{if (pipecount==0)
 return 0;
 else
 {DEBUG('p',"Id %d\n",pipe[pipecount-1]);
 return pipe[pipecount-1];
 }
}

void 
Pcb::freepipe()              //delete pipe table
{
 delete pipe;
}

void 
Pcb::setlastpipe(int value)      //store the pid of pipe which needs to be communicated
{DEBUG('p',"lastId %d\n",value);
 lastpipeId=value;}

void 
Pcb::backstoreinitialize(unsigned int ptetablesize, int value)
{ //unsigned int size = currentThread->space->getpagetablesize();
  DEBUG('x',"ptetablesize %d\n",ptetablesize);
  backstore = new BackingStore(ptetablesize,value); 
}


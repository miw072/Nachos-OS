// synch.cc
//	Routines for synchronizing threads.  Three kinds of
//	synchronization routines are defined here: semaphores, locks
//   	and condition variables (the implementation of the last two
//	are left to the reader).
//
// Any implementation of a synchronization routine needs some
// primitive atomic operation.  We assume Nachos is running on
// a uniprocessor, and thus atomicity can be provided by
// turning off interrupts.  While interrupts are disabled, no
// context switch can occur, and thus the current thread is guaranteed
// to hold the CPU throughout, until interrupts are reenabled.
//
// Because some of these routines might be called with interrupts
// already disabled (Semaphore::V for one), instead of turning
// on interrupts at the end of the atomic operation, we always simply
// re-set the interrupt state back to its original value (whether
// that be disabled or enabled).
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation
// of liability and disclaimer of warranty provisions.

#include "copyright.h"
#include "synch.h"
#include "system.h"

//----------------------------------------------------------------------
// Semaphore::Semaphore
// 	Initialize a semaphore, so that it can be used for synchronization.
//
//	"debugName" is an arbitrary name, useful for debugging.
//	"initialValue" is the initial value of the semaphore.
//----------------------------------------------------------------------

Semaphore::Semaphore(char* debugName, int initialValue)
{
    name = debugName;
    value = initialValue;
    queue = new List;
}

//----------------------------------------------------------------------
// Semaphore::Semaphore
// 	De-allocate semaphore, when no longer needed.  Assume no one
//	is still waiting on the semaphore!
//----------------------------------------------------------------------

Semaphore::~Semaphore()
{
    delete queue;
}

//----------------------------------------------------------------------
// Semaphore::P
// 	Wait until semaphore value > 0, then decrement.  Checking the
//	value and decrementing must be done atomically, so we
//	need to disable interrupts before checking the value.
//
//	Note that Thread::Sleep assumes that interrupts are disabled
//	when it is called.
//----------------------------------------------------------------------

void
Semaphore::P()
{
    IntStatus oldLevel = interrupt->SetLevel(IntOff);	// disable interrupts

    while (value == 0) { 			// semaphore not available
        queue->SortedInsert((void *)currentThread,-(currentThread->getPriority()));	// so go to sleep
        currentThread->Sleep();
    }
    value--; 					// semaphore available,
    // consume its value

    (void) interrupt->SetLevel(oldLevel);	// re-enable interrupts
}

//----------------------------------------------------------------------
// Semaphore::V
// 	Increment semaphore value, waking up a waiter if necessary.
//	As with P(), this operation must be atomic, so we need to disable
//	interrupts.  Scheduler::ReadyToRun() assumes that threads
//	are disabled when it is called.
//----------------------------------------------------------------------

void
Semaphore::V()
{
    Thread *thread;
    IntStatus oldLevel = interrupt->SetLevel(IntOff);

    thread = (Thread *)queue->Remove();
    if (thread != NULL)	   // make thread ready, consuming the V immediately
        scheduler->ReadyToRun(thread);
    value++;
    (void) interrupt->SetLevel(oldLevel);
}

// Dummy functions -- so we can compile our later assignments
// Note -- without a correct implementation of Condition::Wait(),
// the test case in the network assignment won't work!
//
//----------------------------------------------------------------------
// Lock::Lock
// 	Initialize a lock, so that it can be used for synchronization.
//
//	"debugName" is an arbitrary name, useful for debugging.
//----------------------------------------------------------------------

Lock::Lock(char* debugName) 
{
	name=debugName;
	queue=new List;
	lockheld=0;
        lockHoldingThread=NULL;
        oldPriority=0;
}

//----------------------------------------------------------------------
// Lock::~Lock
// 	De-allocate lock, when no longer needed.  Assume no one
//	is still waiting on the lock queue!
//----------------------------------------------------------------------

Lock::~Lock() 
{
        ASSERT( !isHeldByCurrentThread());          //check if the lock is held by current thread,if yes, can not delete lock
        ASSERT(lockheld==0);                        //check if hte lock is held by any thread
        ASSERT(queue->IsEmpty());                   //check if the queue of the lock is empty
	delete queue;
}

//----------------------------------------------------------------------
// Lock::Acquire
//       Acquire a lock and block others to sleep in the queue
//       This method also implements the priority inversion temporarily
//----------------------------------------------------------------------

void Lock::Acquire() 
{
	ASSERT(!isHeldByCurrentThread());            //check if the lock is held by current thread,if yes, can not acquire the lock twice by the same thread
        DEBUG('t',"Lock \"%s\": Thread %s waiting to acquire\n ", name, currentThread->getName());
	IntStatus oldLevel = interrupt->SetLevel(IntOff);
        while(lockheld !=0)                          // lock is not available                                   
		{
                  if (((!(lockHoldingThread==NULL))&(currentThread->getPriority()>lockHoldingThread->getPriority())))   //priority iversion temporarily
                     {
                     lockHoldingThread->setPriority(currentThread->getPriority());
                     scheduler->findFind(lockHoldingThread,lockHoldingThread->getPriority());
                     scheduler->ReadyToRun(lockHoldingThread);
                     }
		queue->SortedInsert((void *)currentThread,-(currentThread->getPriority()));     // so go to sleep queue in sorted order based on priority
		//scheduler->Print();
    currentThread->Sleep();
    //scheduler->Print();
		}
	 lockheld=1;
         lockHoldingThread = currentThread;
         oldPriority=lockHoldingThread->getPriority();                                          //store the old priority of the low priority thread
	 (void) interrupt->SetLevel(oldLevel);
         DEBUG('t',"Lock \"%s\": Acquired by thread %s \n ", name, currentThread->getName());
   
}

//--------------------------------------------------------------------------
// Lock::isHeldByCurrentThread
//       check current thread holds lock or not
//--------------------------------------------------------------------------


bool Lock::isHeldByCurrentThread()
{
	return (lockHoldingThread == currentThread);   //check current thread holds lock or not
}

//------------------------------------------------------------------------------------------------
// Lock::Release
//       Release a lock and let others to compete for the lock
//       This method also implements the priority inversion temporarily--giving back priority part
//------------------------------------------------------------------------------------------------

void Lock::Release() 
{
	Thread *thread;
	ASSERT(isHeldByCurrentThread());               //check if the lock is held by current thread
        IntStatus oldLevel = interrupt->SetLevel(IntOff);
	thread=(Thread *)queue->Remove();
	if (thread != NULL)	   
        scheduler->ReadyToRun(thread);
        DEBUG('t',"Lock \"%s\": Released by thread %s \n ", name, currentThread->getName());
        lockHoldingThread->setPriority(oldPriority);                                             //give back the low priority
        lockHoldingThread = NULL;
        lockheld=0;
	(void) interrupt->SetLevel(oldLevel);
 
}


//----------------------------------------------------------------------
// Condition::Condition
// 	Initialize a condition variabel, so that it can be used for synchronization.
//
//	"debugName" is an arbitrary name, useful for debugging.
//----------------------------------------------------------------------

Condition::Condition(char* debugName)
{
	name=debugName;
	queue=new List;
}

//----------------------------------------------------------------------
// Condition::~Condition
// 	De-allocate condition variable, when no longer needed.  Assume no one
//	is still waiting on the queue!
//----------------------------------------------------------------------

Condition::~Condition() 
{ 
        ASSERT(queue->IsEmpty());                     //check if the queue of the condition variable is empty, if yes, cannot delete it
	delete queue;
}

//-------------------------------------------------------------------------------
// Condition::Wait
//      Wait until another one signal it. Put the thread into the queue to sleep      
//-------------------------------------------------------------------------------

void Condition::Wait(Lock* conditionLock) 
{
	ASSERT(conditionLock->isHeldByCurrentThread());        //check if the lock is held by current thread
	IntStatus oldLevel = interrupt->SetLevel(IntOff);
        DEBUG('s',"Condition variable %s: Lock releasing\n ", name);
	conditionLock->Release();                              //release the lock so that other threads can use it 
	queue->SortedInsert((void *)currentThread,-(currentThread->getPriority()));       // so go to sleep queue in sorted order based on priority
	currentThread->Sleep();
	(void) interrupt->SetLevel(oldLevel);
        DEBUG('s',"Condition variable %s: Lock reacquiring\n ", name);
	conditionLock->Acquire();                              //signalled and re-acquire the lock
        DEBUG('s',"Condition variable %s: Lock reacquired\n ", name);
}

//-------------------------------------------------------------------------------
// Condition::Signal
//      Signal a thread waited in the queue     
//-------------------------------------------------------------------------------

void Condition::Signal(Lock* conditionLock) 
{
	Thread *thread;
	ASSERT(conditionLock->isHeldByCurrentThread());
	DEBUG('s',"Condition variable %s: signaling\n ", name);
        IntStatus oldLevel = interrupt->SetLevel(IntOff);
        thread=(Thread *)queue->Remove();                      
      	if (thread != NULL)
        {
        //DEBUG('t', "|debug| lock: %s is passed into signal\n",conditionLock->getName());	   
        scheduler->ReadyToRun(thread);                         //signal another thread
        }
	(void) interrupt->SetLevel(oldLevel);
}

//-------------------------------------------------------------------------------
// Condition::Broadcast
//      Broadcast all threads waited in the queue     
//-------------------------------------------------------------------------------

void Condition::Broadcast(Lock* conditionLock) 
{
	Thread *thread;
	ASSERT(conditionLock->isHeldByCurrentThread());
	IntStatus oldLevel = interrupt->SetLevel(IntOff);
	do{
		thread=(Thread *)queue->Remove();             
		if(thread != NULL)
			scheduler->ReadyToRun(thread);        //broadcast all threads
	   }while(thread != NULL);
	(void) interrupt->SetLevel(oldLevel);
}


//Our Mailbox uses one lock, maillock to protect the shared datas and use one 
//Condition Variable CVcount to match Send() and Receive(). The other two Condition
//variable CVsend and CVreceive will make sure that only when a Send() finishes writing 
//the message and a Receive() finishes receiving then others can come in.


Mailbox::Mailbox(char* debugName)
{
	name=debugName;
	maillock=new Lock(debugName);
	CVcount =new Condition(debugName);
	CVsend  =new Condition(debugName); 
	CVreceive= new Condition(debugName);
  mailboxcount=0;
 	mailflag=0;
 	value=0;                                //value stores the message from Send and Receive read it
}
Mailbox::~Mailbox() 
{ 
	delete maillock;
        delete CVcount;
        delete CVsend;
        delete CVreceive;
}

void Mailbox::Send(int message)
{
	maillock->Acquire();                        
        currentThread->Yield();
	if ((mailboxcount--)>0)                  //mailboxcount=0 :No Send or Reiceive
	{                                        //mailboxcount>0 :there is a Reiceive waiting
		CVcount->Signal(maillock);       //mailboxcount<0 :there is a Send waiting
	}
	else                                     //if there is a Reiceive waiting, signal that Receive 
	{                                        //else Send waits
		CVcount->Wait(maillock);       
	}
        currentThread->Yield();
  	while (mailflag==1)                      //mailflag=1 : value is waiting to be read by Receive
  	{                                        //mailflag=0 : valure can be writed by Send 
    	CVsend->Wait(maillock);                  //mailflag=1 : Send waits 
	}
        currentThread->Yield(); 
  	CVreceive->Signal(maillock);             //mailflag=0 : Send writes the value and changes mailflag
 	 value=message;                          //             signal Receive
        currentThread->Yield();
  	mailflag=!mailflag;
  	currentThread->Yield();
        printf("sent %d \n",message);
	maillock->Release();
}

void Mailbox::Receive(int *message)
{
	maillock->Acquire();
        currentThread->Yield();
	if ((mailboxcount++)<0)
	{                                        
		CVcount->Signal(maillock);       //if there is a Send waiting, signal that Send 
		
	}
	else                                     //else Receive waits
	{
		CVcount->Wait(maillock);
	}
        currentThread->Yield();	
 	 while (mailflag==0)                     //mailflag=0 : Receive waits 
  	{
   	 CVreceive->Wait(maillock); 
  	}
         currentThread->Yield();                 //mailflag=1 : Receive read the value and changes mailflag
   	 CVsend->Signal(maillock);               //             Signal Send
         currentThread->Yield();
   	 mailflag=!mailflag;
         currentThread->Yield();
   	 *message=value;
         currentThread->Yield();
   	 printf("received %d \n",*message);
    	 maillock->Release();
   
}

//----------------------------------------------------------------------
// Whale::Whale (char *debugName)
// 	Initialize a whale.
//
//	"debugName" is an arbitrary name, useful for debugging.
//----------------------------------------------------------------------
Whale::Whale(char *debugName)
{
    name = debugName;
    whaleLock = new Lock(debugName);
    CVmale = new Condition(debugName);
    CVfemale = new Condition(debugName);
    CVmatch = new Condition(debugName);
    maleNum = 0;
    femaleNum = 0;
    matchNum = 0;
    
}

//----------------------------------------------------------------------
// Whale::~Whale
// 	De-allocate Whale, when no longer needed. 
//----------------------------------------------------------------------

Whale::~Whale()
{
    delete whaleLock;
    delete CVmale;
    delete CVfemale;
    delete CVmatch;
}

//----------------------------------------------------------------------
// void Whale::Male()
// have the current thread wait until
// there is a waiting female and matchmaker.
//
// Note: All the "currentThread->Yield" statements in the following
// are for the testing purpose
//----------------------------------------------------------------------

void
Whale::Male()
{
    whaleLock->Acquire();
    currentThread->Yield();
    // Either female or matchmaker is not present, have male wait
    if (femaleNum == 0 || matchNum == 0){
        maleNum++;    // one more waiting male
        CVmale->Wait(whaleLock);
    }
    else{
        femaleNum--;	// one female got matched; decrease 
			// the waiting number
	currentThread->Yield();
        CVfemale->Signal(whaleLock);    // wake up a female
        currentThread->Yield();
        matchNum--;    // one matchmaker got matched; decrease
		       // the waiting number
        CVmatch->Signal(whaleLock);    // wake up a matchmaker
    }
    printf("---Male becomes a father!\n");
    whaleLock->Release();
}

//----------------------------------------------------------------------
// void Whale::Female()
// have the current thread wait until
// there is a waiting male and matchmaker.
//
// Note: All the "currentThread->Yield" statements in the following
// are for the testing purpose
//----------------------------------------------------------------------

void
Whale::Female()
{
    whaleLock->Acquire();
    currentThread->Yield();
      if (maleNum == 0 || matchNum == 0){
        femaleNum++;    // one more waiting female
        CVfemale->Wait(whaleLock);
    }
    else{
        maleNum--;    // one male got matched; decrease
		      // the waiting number
	currentThread->Yield();
        CVmale->Signal(whaleLock);    // wake up a male
        currentThread->Yield();
        matchNum--;    // one matchmaker got matched; decrease
	               // the waiting number
        CVmatch->Signal(whaleLock);    // wake up a matchmaker
    }
    printf("---Female becomes a mother!\n");
    whaleLock->Release();
}

//----------------------------------------------------------------------
// void Whale::matchmaker()
// have the current thread wait until
// there is a waiting male and female.
//
// Note: All the "currentThread->Yield" statements in the following
// are for the testing purpose
//----------------------------------------------------------------------

void
Whale::Matchmaker()
{
    whaleLock->Acquire();
    currentThread->Yield();
    if (maleNum == 0 || femaleNum == 0){
        matchNum++;    // one more waiting matchmaker
        CVmatch->Wait(whaleLock);
    }
    else{
        maleNum--;    // one male got matched; decrease
	              // the waiting number
	currentThread->Yield();
        CVmale->Signal(whaleLock);    // wake up a male
	currentThread->Yield();
        femaleNum--;    // one female got mated; decrease
	                // the waiting number
        CVfemale->Signal(whaleLock);    // wake up a matchmaker
    }
    printf("---A baby whale was born!\n");
    whaleLock->Release();
}


// threadtest.cc 
//	Test cases for the threads assignment.

#include "copyright.h"
#include "system.h"
#include "synch.h"

// testnum is set in main.cc
int testnum = 22;

//----------------------------------------------------------------------
// ThreadTest
// 	
//----------------------------------------------------------------------

//a lock and a condition variable, for later test usage
Condition *c = new Condition("test");	
Lock *l = new Lock("a");

//----------------------------------------------------------------------
//test#1-prob1(1)
//acquiring same lock twice by same & different threads
//----------------------------------------------------------------------
void
SimpleThread_1a(int para)
{
	printf("|debug| thd1 trying to acquire lock 1st time\n");
	l->Acquire();
	printf("|debug| Acquired\n");
	printf("|debug| thd1 trying to acquire lock 2nd time\n");
	l->Acquire();
	printf("|debug| Acquired\n");
}

void
SimpleThread_1b(int para)
{
	printf("|debug| thd1 trying to acquire lock 1st time\n");
	l->Acquire();
	printf("|debug| Acquired\n");
	currentThread->Yield();
	l->Release();
}


void
SimpleThread_1c(int para)
{
	printf("|debug| Different thread trying to acquire lock while held\n");
	l->Acquire();
}

void
ThreadTest_1a()
{
	Thread *t1 = new Thread("thd1");
	t1->Fork(SimpleThread_1a, 1);
}

void
ThreadTest_1b()
{
	Thread *t1 = new Thread("thd1");
	t1->Fork(SimpleThread_1b, 1);
	Thread *t2 = new Thread("thd2");
	t2->Fork(SimpleThread_1c, 1);
	printf("|debug| test thread is finishing\n");
}

//----------------------------------------------------------------------
//test#2-prob1(2)
//release a lock not held
//----------------------------------------------------------------------
void
SimpleThread_2a(int para)
{
	l->Acquire();
	printf("|debug| thd1: lock acquired\n");
	l->Release();
	printf("|debug| thd1: lock released\n");
}

void
SimpleThread_2b(int para)
{
	printf("|debug| thd2 trying to release a lock not held\n");
	l->Release();
}
void 
ThreadTest_2()
{
	Thread *t1 = new Thread("thd1");
        Thread *t2 = new Thread("thd2");
	t1->Fork(SimpleThread_2a, 1);
	t2->Fork(SimpleThread_2b, 1);
	currentThread->Yield();
	printf("|debug| test thread is finishing\n");
}

//----------------------------------------------------------------------
//test#3-prob1(3)
//delete a lock that is held
//----------------------------------------------------------------------
void
SimpleThread_3a(int para)
{
	printf("|debug| thd1 acquiring lock\n");
	l->Acquire();
	currentThread->Yield();
}

void
SimpleThread_3b(int para)
{
	printf("|debug| trying to delete a held lock\n");
	l->~Lock();
}

void
ThreadTest_3()
{
	Thread *t1 = new Thread("thd1");
	Thread *t2 = new Thread("thd2");
	t1->Fork(SimpleThread_3a, 1);
	t2->Fork(SimpleThread_3b, 1);
	currentThread->Yield();
	printf("|debug| test thread is finishing\n");
}

//----------------------------------------------------------------------
//test#4-prob1(4)
//wait a cv without on lock held
//----------------------------------------------------------------------
void 
SimpleThread_4(int para)
{
	printf("|debug| trying to wait on lock unheld\n");
	c->Wait(l);
}

void
ThreadTest_4()
{
        Thread *t = new Thread("thd1");
        t->Fork(SimpleThread_4, 1);
	currentThread->Yield();
	printf("|debug| test thread is finishing\n");
}

//----------------------------------------------------------------------
//test#5-prob1(5)
//signal a condition only wakes up one thread
//broadcast wakes all up
//----------------------------------------------------------------------
void
SimpleThread_5(int para)
{   
        l->Acquire();
	printf("|debug| a thread waits\n");
        c->Wait(l);
	l->Release();
}

void
SimpleThread_5b(int para)
{
	l->Acquire();
	printf("|debug| signal cv once\n");
	c->Signal(l);
	l->Release();
}

void
SimpleThread_5c(int para)
{
	l->Acquire();
	printf("|debug| broadcast cv once\n");
	c->Broadcast(l);
	l->Release();
}

void
ThreadTest_5()
{
	Thread *t1 = new Thread("thd1");
	Thread *t2 = new Thread("thd2");
	Thread *t3 = new Thread("thd3");
	Thread *t4 = new Thread("thd4");
	Thread *t5 = new Thread("thd5");

    
        t1->Fork(SimpleThread_5, 1);
        t2->Fork(SimpleThread_5, 1);
        t3->Fork(SimpleThread_5, 1);
        t4->Fork(SimpleThread_5b, 1);
	t5->Fork(SimpleThread_5c, 1);
	currentThread->Yield();
	printf("|debug| test thread is finishing\n");
}

//----------------------------------------------------------------------
//test#6-prob1(6)
//signal and broadcast cv with no waiter is no-op, later thread that waits 
//will just block (signal is not stored-state)
//----------------------------------------------------------------------
void
SimpleThread_6a(int para)
{
        l->Acquire();
	printf("|debug| thd1 trying to signal a cv without wait on it\n");
	c->Signal(l);
	l->Release();
}

//signal/broadcast with no waiter
void
SimpleThread_6b(int para)
{
        l->Acquire();
	printf("|debug| thd2 trying to broadcast a cv without wait on it\n");
        c->Broadcast(l);
        l->Release();
}

//future thread waits
void 
SimpleThread_6c(int para)
{
        l->Acquire();
	printf("|debug| thd3 waits on cv\n");
        c->Wait(l);
}

void
ThreadTest_6()
{
   
        Thread *t1 = new Thread("thd1");
        Thread *t2 = new Thread("thd2");
        Thread *t3 = new Thread("thd3");
        t1->Fork(SimpleThread_6a, 1);
        t2->Fork(SimpleThread_6b, 1);
        t3->Fork(SimpleThread_6c, 1);
	currentThread->Yield();
	printf("|debug| test thread is finishing, thd3 should still be sleeping\n");
}

//----------------------------------------------------------------------
//test#7-prob1(7)
//thread calling signal holds the lock passed into signal
//
//*add: DEBUG('t', "|debug| lock: %s is passed into signal\n",conditionLock->getName());
//in the signal() in synch.cc
//----------------------------------------------------------------------
void
SimpleThread_7a(int para)
{
	l->Acquire();
	c->Wait(l);
	l->Release();
}

void
SimpleThread_7b(int para)
{
	l->Acquire();
	printf("|debug| thread %s has the lock: %s\n", currentThread->getName(), l->getName());
	c->Signal(l);
	l->Release();
}

void
ThreadTest_7()
{
	Thread *t1 = new Thread("thd1");
	Thread *t2 = new Thread("thd2");
	t1->Fork(SimpleThread_7a, 1);
	t2->Fork(SimpleThread_7b, 1);
	currentThread->Yield();
	printf("|debug| test thread is finishing\n");
}

//----------------------------------------------------------------------
//test#8-prob1*(8)
//deleting a lock or cv should run with no threads waiting
//----------------------------------------------------------------------
void
SimpleThread_8a(int para)
{
	printf("|debug| current thread trying to acquire a lock\n");
	l->Acquire();
}

void
SimpleThread_8b(int para)
{
	printf("|debug| current thread trying to delete a lock\n");
	l->~Lock();
}

void
SimpleThread_8c(int para)
{
	l->Acquire();
	printf("|debug| current thread trying to wait on a lock\n");
	c->Wait(l);	
}

void
SimpleThread_8d(int para)
{
	printf("|debug| current thread deleting a cv\n");
	c->~Condition();
}

void
ThreadTest_8_lock()
{
	Thread *t1 = new Thread("thd1");
	Thread *t2 = new Thread("thd2");
	Thread *t3 = new Thread("thd3");
	t1->Fork(SimpleThread_8a, 1);
	t2->Fork(SimpleThread_8a, 1);
	t3->Fork(SimpleThread_8b, 1);
	currentThread->Yield(); 
	printf("|debug| test thread is finishing\n");
}

void
ThreadTest_8_cv()
{
        Thread *t1 = new Thread("thd1");
        Thread *t2 = new Thread("thd2");
        Thread *t3 = new Thread("thd3");
        Thread *t4 = new Thread("thd4");
        t1->Fork(SimpleThread_8c, 1);
        t2->Fork(SimpleThread_8c, 1);
        t3->Fork(SimpleThread_8c, 1);
	t4->Fork(SimpleThread_8d, 1);
	currentThread->Yield();
	printf("|debug| test thread is finishing\n");
}



//----------------------------------------------------------------------
// MailTest1-Send and Receive
//----------------------------------------------------------------------

Mailbox *mailtest1box = NULL;

void
MailThread1(int param)
{   currentThread->Yield();
    printf("Send a mail\n");
    currentThread->Yield();
    mailtest1box->Send(5);
    
}
void
MailThread2(int param)
{   int textvalue;
    currentThread->Yield();
    printf("Receive a mail\n");
    currentThread->Yield();
    mailtest1box->Receive(&textvalue);
    
    
}




void
MailTest1()
{
    //DEBUG('t', "Entering MailTest");

    mailtest1box = new Mailbox("MailTest1box");
    
    Thread *t = new Thread("one");
    t->Fork(MailThread1, 0);
    t = new Thread("two");
    t->Fork(MailThread2, 0);
}


//----------------------------------------------------------------------
// MailTest2-Send and Receive-multiple
//----------------------------------------------------------------------

Mailbox *mailtest2box = NULL;

void
MailThread3(int param)
{
    //printf("Send a mail\n");
    mailtest2box->Send(5);
 
    
}

void
MailThread4(int param)
{   int textvalue;
    //printf("Receive a mail\n");
    mailtest2box->Receive(&textvalue);

    
}

void
MailThread5(int param)
{   int textvalue;
    //printf("Receive a mail\n");
    mailtest2box->Receive(&textvalue);
    
}

void
MailThread6(int param)
{   
     int textvalue;
    //printf("Receive a mail\n");
    mailtest2box->Receive(&textvalue);
    
}

void
MailThread7(int param)
{   
     int textvalue;
    //printf("Receive a mail\n");
    mailtest2box->Receive(&textvalue);

    
}

void
MailThread8(int param)
{   
    //printf("Send a mail\n");
    mailtest2box->Send(10);

    
}

void
MailThread9(int param)
{   
    //printf("Send a mail\n");
    mailtest2box->Send(15);

    
}


void
MailTest2()
{
    //DEBUG('t', "Entering DeleteTest");

    mailtest2box = new Mailbox("MailTest1box");
    
    Thread *t = new Thread("one");
    t->Fork(MailThread3, 0);
    t = new Thread("two");
    t->Fork(MailThread4, 0);
    t = new Thread("three");
    t->Fork(MailThread5, 0);
    t = new Thread("four");
    t->Fork(MailThread6, 0);
    t = new Thread("five");
    t->Fork(MailThread7, 0);
    t = new Thread("six");
    t->Fork(MailThread8, 0);
    t = new Thread("seven");
    t->Fork(MailThread9, 0);


}


//----------------------------------------------------------------------
// MailTest3-No receive
//----------------------------------------------------------------------

void
MailThread31(int param)
{   currentThread->Yield();
    printf("Send a mail\n");
    currentThread->Yield();
    mailtest1box->Send(5);
    
}
void
MailThread32(int param)
{   currentThread->Yield();
    printf("Send a mail\n");
    currentThread->Yield();
    mailtest1box->Send(10);
    
    
}




void
MailTest3()
{
    //DEBUG('t', "Entering MailTest");

    mailtest1box = new Mailbox("MailTest1box");
    
    Thread *t = new Thread("one");
    t->Fork(MailThread31, 0);
    t = new Thread("two");
    t->Fork(MailThread32, 0);
}

//----------------------------------------------------------------------
// MailTest4-Send and Receive-multiple
//----------------------------------------------------------------------

void
MailThread43(int param)
{
    //printf("Send a mail\n");
    mailtest2box->Send(5);
 
    
}

void
MailThread44(int param)
{   //printf("Receive a mail\n");
    mailtest2box->Send(25);

    
}

void
MailThread45(int param)
{   int textvalue;
    //printf("Receive a mail\n");
    mailtest2box->Receive(&textvalue);
    
}

void
MailThread46(int param)
{   
     int textvalue;
    //printf("Receive a mail\n");
    mailtest2box->Receive(&textvalue);
    
}

void
MailThread47(int param)
{   
     int textvalue;
    //printf("Receive a mail\n");
    mailtest2box->Receive(&textvalue);

    
}

void
MailThread48(int param)
{   
    //printf("Send a mail\n");
    mailtest2box->Send(10);

    
}

void
MailThread49(int param)
{   
    //printf("Send a mail\n");
    mailtest2box->Send(15);

    
}


void
MailTest4()
{
    //DEBUG('t', "Entering DeleteTest");

    mailtest2box = new Mailbox("MailTest1box");
    
    Thread *t = new Thread("one");
    t->Fork(MailThread43, 0);
    t = new Thread("two");
    t->Fork(MailThread44, 0);
    t = new Thread("three");
    t->Fork(MailThread45, 0);
    t = new Thread("four");
    t->Fork(MailThread46, 0);
    t = new Thread("five");
    t->Fork(MailThread47, 0);
    t = new Thread("six");
    t->Fork(MailThread48, 0);
    t = new Thread("seven");
    t->Fork(MailThread49, 0);


}



//----------------------------------------------------------------------
//test#9-prob3*(1)
//a joinable thread won't be destroyed until a join is called on it
//----------------------------------------------------------------------
void
SimpleThread_prob3_1a(int para)
{
	printf("|debug| thd1 is calling finish\n");
	currentThread->Finish();
}

void
SimpleThread_prob3_1b(int para)
{
	printf("|debug| thd2 is calling finish\n");
	currentThread->Finish();
}

void
ThreadTest_prob3_1()
{
	Thread *t1 = new Thread("thd1",1);
	Thread *t2 = new Thread("thd2");
	t1->Fork(SimpleThread_prob3_1a, 1);
	t2->Fork(SimpleThread_prob3_1b, 1);
	currentThread->Yield();
	printf("|debug| test thread calls join() on thd1, thd1 should finish later\n");
	t1->Join();
	printf("|debug| test thread is about to finish\n");
}

//----------------------------------------------------------------------
//test#10-prob3*(2)
//parent calls join on a child, if child is still executing, parent waits
//----------------------------------------------------------------------
void
SimpleThread_prob3_2a(int para)
{
	currentThread->Yield();			//all the yield()s are giving main() chance
						//to run if it is allowed
	currentThread->Yield();
	printf("|debug| thd1 is finishing\n");
}
void
SimpleThread_prob3_2b(int para)
{
	currentThread->Yield();			
	currentThread->Yield();
	printf("|debug| thd2 is finishing\n");
}

void
ThreadTest_prob3_2()
{
	Thread *t1 = new Thread("thd1", 1);
	Thread *t2 = new Thread("thd2");
	t1->Fork(SimpleThread_prob3_2a, 1);
	t2->Fork(SimpleThread_prob3_2b, 1);
	currentThread->Yield();
	printf("|debug| test thread calling join on thd1, it waits and will not run until thd1 finishes\n");
	t1->Join();

	printf("|debug| test thread runs again and is about to end, \n");

}

//----------------------------------------------------------------------
//test#11-prob3*(3)
//a parent calls join on a child that has finished, it will not block
//----------------------------------------------------------------------
void
SimpleThread_prob3_3a(int para)
{	
	printf("|debug| thd1 is finishing\n");
}

void
SimpleThread_prob3_3b(int para)
{
	currentThread->Yield();
        printf("|debug| thd2 is finishing\n");
}

void 
ThreadTest_prob3_3()
{
	Thread *t1 =new Thread("thd1", 1);
	t1->Fork(SimpleThread_prob3_3a, 1);
	Thread *t2 =new Thread("thd2");
        t2->Fork(SimpleThread_prob3_3b, 1);
	currentThread->Yield();
	printf("|debug| test thread calling join on child\n");
	t1->Join();
	printf("|debug| test thread is finishing\n");
}

//----------------------------------------------------------------------
//test#12-prob3*(4)
//a thread doesn't call join on itself
//----------------------------------------------------------------------
void
SimpleThread_prob3_4(Thread *t1)
{
	printf("|debug| thread: %s is calling join on itself\n", t1->getName());
	t1->Join();
}
void
ThreadTest_prob3_4()
{
	Thread *t1 =new Thread("thd1", 1);
        t1->Fork((VoidFunctionPtr) SimpleThread_prob3_4, (int) t1);
	
}

//----------------------------------------------------------------------
//test#13-prob3*(5)
//join only invoked on joinable threads
//----------------------------------------------------------------------
void
SimpleThread_prob3_5(int para)
{
        printf("|debug| thd1 is join()ed and finishing, error\n");
}

void
ThreadTest_prob3_5()
{
	Thread *t1 =new Thread("thd1");
        t1->Fork(SimpleThread_prob3_3a, 1);
	printf("|debug| calling thread that is not joinable\n");
	t1->Join();
}

//----------------------------------------------------------------------
//test#14-prob3*(6)
//join only called on a thread that has forked
//----------------------------------------------------------------------
void
ThreadTest_prob3_6()
{
        Thread *t1 =new Thread("thd1", 1);
        printf("|debug| calling join() on thread that is not forked\n");
        t1->Join();
	printf("|debug| join on unforked thread went through, error\n");
}

//----------------------------------------------------------------------
//test#15-prob3*(7)
//join is not called more than once on a thread 
//----------------------------------------------------------------------
void
SimpleThread_prob3_7a(int para)
{
	currentThread->Yield();
	printf("|debug| thd1 is finishing\n");
}

void
ThreadTest_prob3_7()
{
	Thread *t1 = new Thread("thd1", 1);
	t1->Fork(SimpleThread_prob3_7a, 1);
	printf("|debug| test thread calls the first join() on thd1\n");
	t1->Join();
	currentThread->Yield();
	printf("|debug| test thread calls another join() on thd1\n");
	t1->Join();
}


//--------------------------------------------------------------
//Priority Test--Regular
//Test priority: three threads are created and run in priority order
//--------------------------------------------------------------

void
PriorityThread1(int param)
{ 
   //currentThread->Yield();
   int i;
   for (i=1;i<5;i++)
   {  
     printf("Thread 1 running, Priority %d\n",currentThread->getPriority());
     //currentThread->Yield();

   }    
}

void
PriorityThread2(int param)
{   
  // currentThread->Yield();
   int i;
   for (i=1;i<5;i++)
   {  
     printf("Thread 2 running, Priority %d\n",currentThread->getPriority());
   }    
}
    
    
void
PriorityThread3(int param)
{   
   int i;
   for (i=1;i<5;i++)
   {  
     printf("Thread 3 running, Priority %d\n",currentThread->getPriority());
     //currentThread->Yield();

   }    
}


void
PriorityTest1()
{
    //DEBUG('t', "Entering PriorityTest");
    Thread *high = new Thread("high", 0);
    Thread *medium = new Thread("medium", 1);
    Thread *low = new Thread("low", 1);
    
    high->setPriority(20);                     //set priority
    medium->setPriority(15);
    low->setPriority(1);
   
    high->Fork(PriorityThread1, 0);
    medium->Fork(PriorityThread2, 0);
    low->Fork(PriorityThread3, 0);
}



//----------------------------------------
//Priority inversion Test--Lock
//Test the priority inversion condition happened when a low priority thread holds the lock 
//and a high piority thread blocked by that lock. While two medium priority threads are already ready to run.
//----------------------------------------
Lock * Plock=NULL;
Semaphore *sema=NULL;

void
PriorityThreadl1(int param)
{  
   sema->P();
   sema->V();
   sema->V();
   Plock->Acquire();
   //scheduler->Print(); 
   int i;
   for (i=1;i<5;i++)
   {  
     printf("Thread 1 running, Priority %d\n",currentThread->getPriority());
     //currentThread->Yield();
   }    
   Plock->Release();

}

void
PriorityThreadl2(int param)
{  
   sema->P();
   currentThread->Yield();         //simulate the context switch condition
   int i;
   currentThread->Yield(); 
   for (i=1;i<5;i++)
   {  
     printf("Thread 2 running, Priority %d\n",currentThread->getPriority());
   }    
}
  
void
PriorityThreadl22(int param)
{  
   sema->P();
   currentThread->Yield();         //simulate the context switch condition
   int i;
   currentThread->Yield(); 
   for (i=1;i<5;i++)
   {  
     printf("Thread 22 running, Priority %d\n",currentThread->getPriority());
   }    
}  
    
void
PriorityThreadl3(int param)
{  
   Plock->Acquire(); 
   currentThread->Yield();
   sema->V();
   currentThread->Yield();
   //scheduler->Print();
   int i;
   for (i=1;i<5;i++)
   {  
     printf("Thread 3 running, Priority %d\n",currentThread->getPriority());
     //scheduler->Print();
   }    
   //scheduler->Print();
   Plock->Release();
   //scheduler->Print();
   

}



void
PriorityTest2()
{
    //DEBUG('t', "Entering PriorityTest");
    Thread *high2 = new Thread("high", 0);
    Thread *medium2 = new Thread("medium", 1);
    Thread *medium22 = new Thread("medium2", 1);
    Thread *low2 = new Thread("low", 1);
    
    Plock =new Lock("Plockh");
    sema=new Semaphore("semah",0);
    
    high2->setPriority(20);             //set priority
    medium2->setPriority(15);
    medium22->setPriority(10);
    low2->setPriority(1);
   
    high2->Fork(PriorityThreadl1, 0);
    medium2->Fork(PriorityThreadl2, 0);
    medium22->Fork(PriorityThreadl22,0);
    low2->Fork(PriorityThreadl3, 0);
    
}

//----------------------------------------
//Priority inversion Test--Lock recursive
//----------------------------------------

Lock * Plock1=NULL;
Lock * Plock2=NULL;
Lock * Plock3=NULL;
Semaphore *sema1=NULL;
Semaphore *sema2=NULL;
Semaphore *sema3=NULL;

void
PriorityThreadlr1(int param)
{  
  
   sema1->P();
   Plock3->Acquire();
    
   int i;
   for (i=1;i<5;i++)
   {  
     printf("Thread 1 running, Priority %d\n",currentThread->getPriority());
   }    
   Plock3->Release();
}

void
PriorityThreadlr2(int param)
{  
   Plock3->Acquire();
   sema2->P();
   sema1->V();
   Plock2->Acquire();
   currentThread->Yield();         //simulate the context switch condition
   int i;
   currentThread->Yield(); 
   for (i=1;i<5;i++)
   {  
     printf("Thread 2 running, Priority %d\n",currentThread->getPriority());
   }  
   Plock2->Release();
   Plock3->Release(); 
}
  
void
PriorityThreadlr22(int param)
{  
   Plock2->Acquire();
   sema3->P();
   sema2->V();
   Plock1->Acquire();
   currentThread->Yield();         //simulate the context switch condition
   int i;
   currentThread->Yield(); 
   for (i=1;i<5;i++)
   {  
     printf("Thread 22 running, Priority %d\n",currentThread->getPriority());
   }   
   Plock1->Release();
   Plock2->Release();
}  
    
void
PriorityThreadlr3(int param)
{  
   Plock1->Acquire(); 
   currentThread->Yield();
   sema3->V();
   currentThread->Yield();
   
   int i;
   for (i=1;i<5;i++)
   {  
     printf("Thread 3 running, Priority %d\n",currentThread->getPriority());
   }    
   Plock1->Release();    

}



void
PriorityTest7()
{
    //DEBUG('t', "Entering PriorityTest");
    Thread *high2 = new Thread("high", 0);
    Thread *medium2 = new Thread("medium", 1);
    Thread *medium22 = new Thread("medium2", 1);
    Thread *low2 = new Thread("low", 1);
    
    Plock1 =new Lock("Plockh1");
    Plock2 =new Lock("Plockh2");
    Plock3 =new Lock("Plockh3");
    sema1=new Semaphore("semah1",0);
    sema2=new Semaphore("semah2",0);
    sema3=new Semaphore("semah3",0);
    
    high2->setPriority(20);             //set priority
    medium2->setPriority(15);
    medium22->setPriority(10);
    low2->setPriority(1);
   
    high2->Fork(PriorityThreadlr1, 0);
    medium2->Fork(PriorityThreadlr2, 0);
    medium22->Fork(PriorityThreadlr22, 0);
    low2->Fork(PriorityThreadlr3, 0);

    
}

//----------------------------------------
//Priority inversion Test--Join
//Test the priority inversion condition happened when a low priority thread join  
//a high piority thread. While two medium priority threads are already ready to run.
//----------------------------------------


void
PriorityThreadj1(Thread* low)
{  
   printf("ready to join\n");
   low->Join();                                      //call join
   currentThread->Yield();
   int i;
   for (i=1;i<5;i++)
   {    
     printf("Thread 1 running, Priority %d\n",currentThread->getPriority());
     //currentThread->Yield();
   }    
   

}

void
PriorityThreadj2(int param)
{  
   currentThread->Yield();
   int i;
   currentThread->Yield();
   for (i=1;i<5;i++)
   {  
     printf("Thread 2 running, Priority %d\n",currentThread->getPriority());
     //currentThread->Yield();

   }    
}

void
PriorityThreadj22(int param)
{  
   
   currentThread->Yield();         //simulate the context switch condition
   int i;
   currentThread->Yield(); 
   for (i=1;i<5;i++)
   {  
     printf("Thread 22 running, Priority %d\n",currentThread->getPriority());
   }    
}   
    
void
PriorityThreadj3(int param)
{  
   currentThread->Yield();
   int i;
   for (i=1;i<5;i++)
   {  
     printf("Thread 3 running, Priority %d\n",currentThread->getPriority());
     //currentThread->Yield();

   }    
   
}



void
PriorityTest3()
{
    //DEBUG('t', "Entering PriorityTest");
    Thread *high2 = new Thread("high", 0);      //not joinable
    Thread *medium2 = new Thread("medium", 0);  //joinable
    Thread *low2 = new Thread("low", 1);        //joinable
    Thread *medium22 = new Thread("medium2", 1);
    
    high2->setPriority(20);
    medium2->setPriority(15);
    medium22->setPriority(10);
    low2->setPriority(1);
   
    low2->Fork((VoidFunctionPtr) PriorityThreadj3, 0);
    medium2->Fork((VoidFunctionPtr) PriorityThreadj2, 0);
    medium22->Fork((VoidFunctionPtr)PriorityThreadj22,0);
    high2->Fork((VoidFunctionPtr) PriorityThreadj1, (int) low2);
      
}

//----------------------------------------
//Priority inversion Test--Join recursive
//----------------------------------------


void
PriorityThreadjr1(Thread* medium2)
{  
   printf("ready to join\n");
   medium2->Join();                                      //call join
   currentThread->Yield();
   int i;
   for (i=1;i<5;i++)
   {    
     printf("Thread 1 running, Priority %d\n",currentThread->getPriority());
     //currentThread->Yield();
   }    
   

}

void
PriorityThreadjr2(Thread* medium22)
{  
   currentThread->Yield();
   medium22->Join();
   int i;
   currentThread->Yield();
   for (i=1;i<5;i++)
   {  
     printf("Thread 2 running, Priority %d\n",currentThread->getPriority());
     //currentThread->Yield();

   }    
}

void
PriorityThreadjr22(Thread* low2)
{  
   
   currentThread->Yield();         //simulate the context switch condition
   low2->Join();
   int i;
   currentThread->Yield(); 
   for (i=1;i<5;i++)
   {  
     printf("Thread 22 running, Priority %d\n",currentThread->getPriority());
   }    
}   
    
void
PriorityThreadjr3(int param)
{  
   currentThread->Yield();
   int i;
   for (i=1;i<5;i++)
   {  
     printf("Thread 3 running, Priority %d\n",currentThread->getPriority());
     //currentThread->Yield();

   }    
   
}



void
PriorityTest8()
{
    //DEBUG('t', "Entering PriorityTest");
    Thread *high2 = new Thread("high", 0);      //not joinable
    Thread *medium2 = new Thread("medium", 1);  //joinable
    Thread *low2 = new Thread("low", 1);        //joinable
    Thread *medium22 = new Thread("medium2", 1);
    
    high2->setPriority(20);
    medium2->setPriority(15);
    medium22->setPriority(10);
    low2->setPriority(1);
   
    low2->Fork((VoidFunctionPtr) PriorityThreadjr3, 0);
    medium2->Fork((VoidFunctionPtr) PriorityThreadjr2, (int) low2);
    medium22->Fork((VoidFunctionPtr)PriorityThreadjr22,(int) medium2);
    high2->Fork((VoidFunctionPtr) PriorityThreadjr1, (int) medium22);
      
}

//----------------------------------------
//Priority--Lock
//regular test of priority when threads with different priority are put in and woken up from the lock queue in priority order.
//----------------------------------------

void
PriorityThreadl4(int param)
{  
   sema->P();
   Plock->Acquire(); 
   int i;
   for (i=1;i<5;i++)
   {  
     printf("Thread 1 running, Priority %d\n",currentThread->getPriority());
     //currentThread->Yield();

   }    
   Plock->Release();

}

void
PriorityThreadl5(int param)
{  
   sema->P();
   Plock->Acquire(); 
   currentThread->Yield();         //simulate the context switch condition
   int i;
   currentThread->Yield(); 
   for (i=1;i<5;i++)
   {  
     printf("Thread 2 running, Priority %d\n",currentThread->getPriority());
   }    
   Plock->Release();

}
    
    
void
PriorityThreadl6(int param)
{  
   Plock->Acquire(); 
   currentThread->Yield();
   sema->V();
   currentThread->Yield();
   sema->V();
   currentThread->Yield();

   int i;
   for (i=1;i<5;i++)
   {  
     printf("Thread 3 running, Priority %d\n",currentThread->getPriority());
   }    
   Plock->Release();
   

}



void
PriorityTest4()
{
    //DEBUG('t', "Entering PriorityTest");
    Thread *high2 = new Thread("high", 0);
    Thread *medium2 = new Thread("medium", 1);
    Thread *low2 = new Thread("low", 1);
    Plock =new Lock("Plockh");
    sema=new Semaphore("semah",0);
    
    high2->setPriority(20);             //set priority
    medium2->setPriority(15);
    low2->setPriority(1);
   
    high2->Fork(PriorityThreadl4, 0);
    medium2->Fork(PriorityThreadl5, 0);
    low2->Fork(PriorityThreadl6, 0);

    
}

//----------------------------------------
//Priority--Condition variable
//regular test of priority when threads with different priority are put in and woken up from the condition variable queue in priority order.
//PriorityThreadl10 is used for broadcasting all other threads waiting on the condition variable queue, so it ends first.
//----------------------------------------
Condition * CVtest1=NULL;

void
PriorityThreadl7(int param)
{  
   Plock->Acquire();
   CVtest1->Wait(Plock); 
   int i;
   for (i=1;i<5;i++)
   {  
     printf("Thread 1 running, Priority %d\n",currentThread->getPriority());
     //currentThread->Yield();

   }    
   Plock->Release();

}

void
PriorityThreadl8(int param)
{  
   Plock->Acquire();
   CVtest1->Wait(Plock);  
   currentThread->Yield();         //simulate the context switch condition
   int i;
   currentThread->Yield(); 
   for (i=1;i<5;i++)
   {  
     printf("Thread 2 running, Priority %d\n",currentThread->getPriority());
   }    
   Plock->Release();

}
    
    
void
PriorityThreadl9(int param)
{  
   Plock->Acquire();
   CVtest1->Wait(Plock); 
 
   int i;
   for (i=1;i<5;i++)
   {  
     printf("Thread 3 running, Priority %d\n",currentThread->getPriority());
   }    
   Plock->Release();

}

void
PriorityThreadl10(int param)
{  
   Plock->Acquire(); 
   CVtest1->Broadcast(Plock);

   int i;
   for (i=1;i<5;i++)
   {  
     printf("Thread 4 running, Priority %d\n",currentThread->getPriority());
   }    
   Plock->Release();

}

void
PriorityTest5()
{
    //DEBUG('t', "Entering PriorityTest");
    Thread *high2 = new Thread("high", 0);
    Thread *medium2 = new Thread("medium", 1);
    Thread *low2 = new Thread("low", 1);
    Thread *lowlow = new Thread("lowlow", 1);

    Plock =new Lock("Plockh");
    CVtest1=new Condition("CVtest1");
    
    high2->setPriority(20);             //set priority
    medium2->setPriority(15);
    low2->setPriority(5);
    lowlow->setPriority(1);
   
    high2->Fork(PriorityThreadl7, 0);
    medium2->Fork(PriorityThreadl8, 0);
    low2->Fork(PriorityThreadl9, 0);
    lowlow->Fork(PriorityThreadl10, 0);

    
}

//----------------------------------------
//Priority--Semaphore
//regular test of priority when threads with different priority are put in and woken up from the Semaphore queue in priority order.
//PriorityThreadl14 is used for broadcasting all other threads waiting on the Semaphore queue.
//----------------------------------------

void
PriorityThreadl11(int param)
{  
   sema->P();
   int i;
   for (i=1;i<5;i++)
   {  
     printf("Thread 1 running, Priority %d\n",currentThread->getPriority());
     //currentThread->Yield();

   }    
   
}

void
PriorityThreadl12(int param)
{  
   sema->P();
   currentThread->Yield();         //simulate the context switch condition
   int i;
   currentThread->Yield(); 
   for (i=1;i<5;i++)
   {  
     printf("Thread 2 running, Priority %d\n",currentThread->getPriority());
   }    
   
}
    
    
void
PriorityThreadl13(int param)
{  
   sema->P();
   int i;
   for (i=1;i<5;i++)
   {  
     printf("Thread 3 running, Priority %d\n",currentThread->getPriority());
   }    
   
}

void
PriorityThreadl14(int param)
{  
   sema->V(); 
   sema->V();
   sema->V();
   currentThread->Yield(); 
   int i;
   for (i=1;i<5;i++)
   {  
     printf("Thread 4 running, Priority %d\n",currentThread->getPriority());
   }    
   
}

void
PriorityTest6()
{
    //DEBUG('t', "Entering PriorityTest");
    Thread *high2 = new Thread("high", 0);
    Thread *medium2 = new Thread("medium", 1);
    Thread *low2 = new Thread("low", 1);
    Thread *lowlow = new Thread("lowlow", 1);
    sema=new Semaphore("semah",0);

        
    high2->setPriority(20);             //set priority
    medium2->setPriority(15);
    low2->setPriority(5);
    lowlow->setPriority(1);
   
    high2->Fork(PriorityThreadl11, 0);
    medium2->Fork(PriorityThreadl12, 0);
    low2->Fork(PriorityThreadl13, 0);
    lowlow->Fork(PriorityThreadl14, 0);

    
}

//----------------------------------------------------------------------
// WhaleTest1-multiple
//      Multiple male,female and matchmakers
//----------------------------------------------------------------------

Whale *whaletest1 = NULL;

void
WhaleThread1(int param)
{
    
    whaletest1->Male();
   
}

void
WhaleThread2(int param)
{
    
    whaletest1->Female();
   
}

void
WhaleThread3(int param)
{
    
    whaletest1->Matchmaker();
   
}

void
WhaleThread4(int param)
{
    
    whaletest1->Female();
   
}

void
WhaleThread5(int param)
{
    
    whaletest1->Matchmaker();
   
}

void
WhaleThread6(int param)
{
    
    whaletest1->Matchmaker();
   
}

void
WhaleThread7(int param)
{
    
    whaletest1->Male();
   
}

void
WhaleThread8(int param)
{
    
    whaletest1->Male();
   
}

void
WhaleThread9(int param)
{
    
    whaletest1->Female();
   
}


void
WhaleTest1()
{
    //DEBUG('t', "Entering DeleteTest");

    whaletest1 = new Whale("WhaleTest1");
    
    Thread *t = new Thread("one");
    t->Fork(WhaleThread1, 0);
    t = new Thread("two");
    t->Fork(WhaleThread2, 0);
    t = new Thread("three");
    t->Fork(WhaleThread3, 0);
    t = new Thread("four");
    t->Fork(WhaleThread4, 0);
    t = new Thread("five");
    t->Fork(WhaleThread5, 0);
    t = new Thread("six");
    t->Fork(WhaleThread6, 0);
    t = new Thread("seven");
    t->Fork(WhaleThread7, 0);
    t = new Thread("eight");
    t->Fork(WhaleThread8, 0);
    t = new Thread("nine");
    t->Fork(WhaleThread9, 0);

}

//----------------------------------------------------------------------
// WhaleTest2
//      Multiple males,females and only one matchmaker
//----------------------------------------------------------------------

Whale *whaletest2 = NULL;

void
WhaleThread11(int param)
{
    
    whaletest2->Male();
   
}

void
WhaleThread12(int param)
{
    
    whaletest2->Male();
   
}

void
WhaleThread13(int param)
{
    
    whaletest2->Male();
   
}

void
WhaleThread14(int param)
{
    
    whaletest2->Female();
   
}

void
WhaleThread15(int param)
{
    
    whaletest2->Female();
   
}

void
WhaleThread16(int param)
{
    
    whaletest2->Female();
   
}

void
WhaleThread17(int param)
{
    
    whaletest2->Matchmaker();
   
}



void
WhaleTest2()
{
    //DEBUG('t', "Entering DeleteTest");

    whaletest2 = new Whale("WhaleTest2");
    
    Thread *t = new Thread("one");
    t->Fork(WhaleThread11, 0);
    t = new Thread("two");
    t->Fork(WhaleThread12, 0);
    t = new Thread("three");
    t->Fork(WhaleThread13, 0);
    t = new Thread("four");
    t->Fork(WhaleThread14, 0);
    t = new Thread("five");
    t->Fork(WhaleThread15, 0);
    t = new Thread("six");
    t->Fork(WhaleThread16, 0);
    t = new Thread("seven");
    t->Fork(WhaleThread17, 0);



}

//----------------------------------------------------------------------
// ThreadTest
// 	Invoke a test routine.
//----------------------------------------------------------------------

void
ThreadTest()
{
    switch (testnum) {
    case 1:			//same thread acquires same lock twice
        ThreadTest_1a();
        break;
    case 2:			//different threads acquire lock
        ThreadTest_1b();
        break;
    case 3:                     //release a lock not held
	ThreadTest_2();
	break;
    case 4:                     //delete a lock that is held
	ThreadTest_3();
	break;
    case 5:                     //wait a cv without on lock held
	ThreadTest_4();
	break;
    case 6:                     //signal a condition only wakes up one thread,broadcast wakes all up. Better run with -d t.
        ThreadTest_5();
        break;
    case 7:                     //signal and broadcast cv with no waiter is no-op, later thread that waits will just block
	ThreadTest_6();
	break;
    case 8:                     //thread calling signal holds the lock passed into signal,now affected debug code is commeted
	ThreadTest_7();
	break;
    case 9:			//delete lock should run with no threads waiting
	ThreadTest_8_lock();
	break;
    case 10:			//delete cv should run with no threads waiting
	ThreadTest_8_cv();
	break;
        
    case 11:                    //mailtest--couple
         MailTest1();
         break;
    case 12:                    //mailtest--multiple
         MailTest2();
         break;

    case 13:                    //a joinable thread won't be destroyed until a join is called on it. Better run with -d t
	ThreadTest_prob3_1();
	break;
    case 14:                    //parent calls join on a child, if child is still executing, parent waits. Better run with -d t
	ThreadTest_prob3_2();
	break;
    case 15:                    //a parent calls join on a child that has finished, it will not block. Better run with -d t
	ThreadTest_prob3_3();
	break;
    case 16:                    //a thread doesn't call join on itself
	ThreadTest_prob3_4();
   	break;
    case 17:                    //join only invoked on joinable threads
	ThreadTest_prob3_5();
	break;
    case 18:                    //join only called on a thread that has forked
	ThreadTest_prob3_6();
	break;
    case 19:                    //join is not called more than once on a thread 
	ThreadTest_prob3_7();
	break;
        
    case 20:                    //prioritytest-regular
         PriorityTest1();
         break;
    case 21:                    //prioritytest-inversion-lock
         PriorityTest2();
         break;
    case 22:                    //prioritytest-inversion-join
         PriorityTest3();
         break;
    case 23:                    //prioritytest-lock
         PriorityTest4();
         break;
    case 24:                    //prioritytest-condition variable
         PriorityTest5();
         break;
    case 25:                    //prioritytest-condition variable
         PriorityTest6();
         break;

    case 26:                    //whaletest--Multiple males,females,matchmakers
         WhaleTest1();
         break;
    case 27:                    //whaletest--Multiple males,females and only one matchmaker
         WhaleTest2();
	       break;    
    
    //Supplementary Mailtest          
    case 28:                    //Mailtest--No receive;
         MailTest3(); 
         break;
    case 29:                    //Mailtest--4 Send 3 receive;
         MailTest4();
         break;
    
    //Supplementary "priority inversion" test, if you meet compling error because we add a method to list.cc, you can ignore these two tests
    case 30:
         PriorityTest7();
         break;
    case 31:
         PriorityTest8();
         break;
    default:
	       printf("No test specified.\n");
	       break;
    }
}


#include "copyright.h"
#include "utility.h"
#include "syscall.h"
#include "filesys.h"
class Semaphore;
class Lock;
class Condition;
class TranslationEntry;

#ifndef PCB_H
#define PCB_H

class BackingStore
{
public:
  BackingStore(unsigned int input, int value);
  ~BackingStore();
  void PageOut(TranslationEntry *pte);
  void PageIn(TranslationEntry *pte, int physFrame);
  bool Findpte (int count);
  void killbs();
private:
  OpenFile *backfile;
  bool* backpte;
  unsigned int ptesize;
  char *filename;
};


class BoundedBuffer
{
public:
    BoundedBuffer(int n);
    ~BoundedBuffer();
    void Write(char item);
    char Read();

private:
    Lock *mutex;
    Condition *cvnotfull;
    Condition *cvnotempty;
    int count;
    int Capacity;
    char *buffer;
    int incount;
    int outcount;
};

class Pcb{
public: 
   Pcb();
   ~Pcb();
   int getpid();
   void setpid(int value);
  // void setparent(Thread* newparent);
   void setjoinable(int joinpar);
   void setfinish(int finishpar);
   int getjoinable();
   int getfinish();
   int getexitstatus();
   void setexitstatus(int status);
   void pipeinitialize();
   void appendpipe(int value);
   int findlast();
   void freepipe();
   void setlastpipe(int value); 
   //Thread *getparent();
   Semaphore *semaForBlock;
   Semaphore *semaForExit;
   Semaphore *semaForStatus;
   BoundedBuffer *getpipebuffer() {DEBUG('p',"piep buffer\n");return pipebuffer;};
   void backstoreinitialize(unsigned int ptetablesize, int value);
   BackingStore * getbackstore() {return backstore;};
   int getlastpipeId() {return lastpipeId;};
private:
   SpaceId Pid;
   BoundedBuffer * pipebuffer;
   BackingStore *backstore;
   //Thread *parent;
   int*pipe;
   int pipecount;
   int lastpipeId;
   int joinable;
   int finish;
   int exitstatus;
   };

#endif

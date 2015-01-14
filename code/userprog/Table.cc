//Table.cc
#include "copyright.h"
#include "Table.h"

Tableobject::Tableobject()
{ item=NULL;
}


Table::Table(int inputsize)
{tablelock=new Lock("tablelock"); 
 size=inputsize;
 table = new Tableobject[size];
}

Table::~Table()
{
  delete tablelock;
}

/* Allocate a table slot for "object", returning the "index" of the
   allocated entry; otherwise, return -1 if no free slots are available. */
int 
Table::Alloc(void *object)
{tablelock->Acquire();
   int i=1;
   while (i<size) 
 {
   if (table[i].item == NULL) 
   {
   table[i].item = object;
   tablelock->Release();
   return i;
   }
   i++;
 }
   tablelock->Release();
   return -1;
}

/* Retrieve the object from table slot at "index", or NULL if that
   slot has not been allocated. */
void *
Table::Get(int index) 
{ 
 return table[index].item;
}

/* Free the table slot at index. */
void 
Table::Release(int index) 
{tablelock->Acquire();
 table[index].item=NULL;
 tablelock->Release();
}

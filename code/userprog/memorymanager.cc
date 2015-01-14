//memorymanager.cc
//keep track of the physical page frame, allocate and deallocate
#include "memorymanager.h"
#include "copyright.h"


//----------------------------------------------------------------------
// MemoryManager::MemoryManager
// 	Initialize a mmorymanager with a bitmap
//----------------------------------------------------------------------

MemoryManager::MemoryManager(int numPages)
{
    mmbitmap = new BitMap(numPages);
    mmlock = new Lock("mmlock");
}

//----------------------------------------------------------------------
// MemoryManager::~MemoryManager
// 	deallocate a memorymanager
//----------------------------------------------------------------------

MemoryManager::~MemoryManager()
{
	delete mmbitmap;
	delete mmlock;
}

//----------------------------------------------------------------------
// MemoryManager::AllocPage
// 	find a page is free and allocate it
//----------------------------------------------------------------------

int
MemoryManager::AllocPage()
{
    mmlock->Acquire();
	int value;
	value = mmbitmap->Find();
        
    mmlock->Release();
    return value;
}

//----------------------------------------------------------------------
// MemoryManager::FreePage
// 	deallocate a page when it is no longer in use
//----------------------------------------------------------------------
void
MemoryManager::FreePage(int physPageNum)
{
	mmlock->Acquire();
	mmbitmap->Clear(physPageNum);
	mmlock->Release();
}

//----------------------------------------------------------------------
// MemoryManager::PageIsAllocate
// 	check if a page is allocated, true for yes, false for no
//----------------------------------------------------------------------

bool
MemoryManager::PageIsAllocate(int physPageNum)
{
	mmlock->Acquire();
    if (mmbitmap->Test(physPageNum))
        {
		mmlock->Release();
                return TRUE;
        }
    else 
	{
		mmlock->Release();
                return FALSE;
	}        
}

//commen






















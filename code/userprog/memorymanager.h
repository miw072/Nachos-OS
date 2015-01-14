//memorymanager.h
//MemoryManager is used to manipulate the bitmap. 
//Find a free bit and allocate it
//Clear the certain bit

#ifndef MEMORYMANAGER_H
#define MEMORYMANAGER_H

#include "copyright.h"
#include "synch.h"
#include "bitmap.h"
#include "system.h"

class MemoryManager{
public:
	MemoryManager(int numPages);
	~MemoryManager();
	int AllocPage();
	void FreePage(int physPageNum);
        bool PageIsAllocate(int physPageNum);

private:	
	BitMap *mmbitmap;
        Lock *mmlock;
};
#endif

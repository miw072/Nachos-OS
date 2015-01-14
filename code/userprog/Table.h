#include "copyright.h"
#include "utility.h"
#include "synch.h"
#ifndef TABLE_H
#define TABLE_H

class Tableobject {
public:
     Tableobject();
     void *item;
};

class Table{
public:
    Table(int inputsize);			
    ~Table();
    int Alloc(void *object);
    void *Get(int index);
    void Release(int index);
private:
    int size;
    Tableobject *table;
    Lock * tablelock;
};
#endif

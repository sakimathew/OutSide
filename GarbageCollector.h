#ifndef __GARBAGECOLLECTOR_H__
#define __GARBAGECOLLECTOR_H__
#include <stdarg.h>
#include <objbase.h>
#include <Unknwn.h>
#include <iostream>
#include <string>
#include <list>

const int ElementNumber = 10;

class GarbageCollector
{
public:
    SRWLOCK MainLock;
    list<CObject *> ElementList[ElementNumber];
    list<CObject *> ShouldBeDeletedList;
    int CurrentElementListInUse;
    HANDLE GarbageHandlerEvent;
    HANDLE GarbageHandlerThread;

    GarbageCollector();
    bool StartGarbageCollector(void);
public:
    static GarbageCollector *Initialize(void);
    list<CObject *> *GetVariableListInUse(void)
    {
        return &( ElementList[CurrentElementListInUse]);
    };
    bool StopGarbageCollector(void);
    friend DWORD GarbageCollectorMainRoutine(LPVOID);
};

#endif

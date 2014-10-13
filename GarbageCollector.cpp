#include "sdbg.h"
#include <functional>

using namespace std;
GarbageCollector *pg_GarbageCollector = NULL;

GarbageCollector::GarbageCollector()
{
    InitializeSRWLock( &MainLock);
    this->CurrentElementListInUse = 0;
    this->GarbageHandlerEvent = INVALID_HANDLE_VALUE;
    this->GarbageHandlerThread = INVALID_HANDLE_VALUE;
}

GarbageCollector *GarbageCollector::Initialize(void)
{
    if( NULL == pg_GarbageCollector){
        pg_GarbageCollector = new GarbageCollector();
        pg_GarbageCollector->StartGarbageCollector();
    }
    
    return pg_GarbageCollector;
}

bool GarbageCollector::StartGarbageCollector(void)
{
#ifdef GARBAGECOLLECTOR
    this->GarbageHandlerEvent = CreateEvent( 
                                                                            NULL, 
                                                                            TRUE, 
                                                                            FALSE, 
                                                                            NULL
                                                                        ); 
    this->GarbageHandlerThread = CreateThread(
                                                                                NULL,
                                                                                0,
                                                                                (LPTHREAD_START_ROUTINE)GarbageCollectorMainRoutine,
                                                                                this,
                                                                                0,
                                                                                NULL);
#endif
    return true;
}

bool GarbageCollector::StopGarbageCollector(void)
{
#ifdef GARBAGECOLLECTOR
    SetEvent( this->GarbageHandlerEvent);
    WaitForSingleObject( this->GarbageHandlerThread, INFINITE);
#endif
    return true;
}

DWORD GarbageCollectorMainRoutine(LPVOID arg)
{
    GarbageCollector *pGarbageCollector = (GarbageCollector*)arg;
    DWORD nextCurrentElementListInUse = 0;
    DWORD VariableListToClean = 0;
#ifdef GARBAGECOLLECTOR
    while(WAIT_TIMEOUT == WaitForSingleObject( pg_GarbageCollector->GarbageHandlerEvent, GARBAGECOLLECTORWAKEUPTIMER))
    {
        for(int i = 0; i < 500; i++){;}

        for(int index = 0; index < ElementNumber; index++){
            if( pGarbageCollector->ElementList[index].size() < pGarbageCollector->ElementList[nextCurrentElementListInUse].size()){
                nextCurrentElementListInUse = index;
            }
        }
        if( nextCurrentElementListInUse == pGarbageCollector->CurrentElementListInUse){
            continue;
        }

        VariableListToClean = pGarbageCollector->CurrentElementListInUse;
        pGarbageCollector->CurrentElementListInUse = nextCurrentElementListInUse;

        pGarbageCollector->ShouldBeDeletedList.remove_if( 
                                                            [](CObject *arg)->bool
                                                            {
                                                                if( arg->GetCount() == 0){ 
                                                                    delete arg; 
                                                                    return true; 
                                                                }
                                                                else{ return false; }
                                                            }
                                                            );
    }
#endif
    return 0;
}

#include "PrepocessorMacros.h"
#include "sdbg.h"
#include "System.h"

static SYSTEM_INFO sysInfo;
static bool CustomizedHeapInitialized = false;
static bool IsMultiThreaded = false;
static DWORD NumberOfThreads = 1;

class FixedSizeHeap{
    vector<LPVOID> MemoryBlock;
    DWORD sizeOfBlock;
    LPVOID nextFreeMemoryAddress;
    LPVOID lastFreeMemoryAddress;
    DWORD freeBlockIndex;
    DWORD maxBlockIndex;
    LPVOID arrayOfBlocks;
    CRITICAL_SECTION sync;
    void* SyncAllocate();
    void* NonSyncAllocate();
    void NonSyncFree( void *addr);
    void SyncFree( void *addr);

    friend HRESULT Allocate(CInt **retAddr);
    friend HRESULT Free(CInt *addr);
    friend HRESULT Allocate(CString **retAddr);
    friend HRESULT Free(CString *addr);
    friend HRESULT Allocate(Variable **retAddr);
    friend HRESULT Free(Variable *addr);
public:
    FixedSizeHeap()
    {
        freeBlockIndex = 0;
        maxBlockIndex = 0;
        arrayOfBlocks = NULL;
        InitializeCriticalSection( &( this->sync));
    }
    void SetSizeOfBlock(DWORD size){ sizeOfBlock = size; }
}MemoryManager[3];

void CustomHeapInitialize()
{
    if( CustomizedHeapInitialized == false){
        CustomizedHeapInitialized = true;
        GetSystemInfo( &sysInfo);
        for(int index = 0; index < sizeof(MemoryManager)/sizeof(MemoryManager[0]); index++){
            if( index == 0){
                MemoryManager[index].SetSizeOfBlock(sizeof(CInt));
            }
            else if( index == 1){
                MemoryManager[index].SetSizeOfBlock(sizeof(CString));
            }
            else if( index == 2){
                MemoryManager[index].SetSizeOfBlock(sizeof(Variable));
            }
        }
    }
    ExecutionSystem::RegisterForThreadChanging( [](DWORD arg)->void
                                                {
                                                    NumberOfThreads = arg;
                                                    arg == 1? IsMultiThreaded = false: IsMultiThreaded = true;
                                                });
}

void *FixedSizeHeap::SyncAllocate()
{
    void *addr = NULL;

    EnterCriticalSection( &this->sync);
    addr = this->NonSyncAllocate();
    LeaveCriticalSection( &this->sync);

    return addr;
}

void* FixedSizeHeap::NonSyncAllocate()
{
    HRESULT hr = S_OK;
    void *retAddr;

    if( NULL == arrayOfBlocks){
        MemoryBlock.push_back(VirtualAlloc( NULL, 1, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE));
        arrayOfBlocks = MemoryBlock[MemoryBlock.size() - 1];
        freeBlockIndex = 0;
        maxBlockIndex = sysInfo.dwPageSize / sizeOfBlock;
    }

    if(lastFreeMemoryAddress == nextFreeMemoryAddress){
        retAddr = (void *)((char *)arrayOfBlocks + (freeBlockIndex * sizeOfBlock));
        freeBlockIndex++;
        if( freeBlockIndex >= maxBlockIndex){
            arrayOfBlocks = NULL;
        }
    }
    else{
        retAddr = nextFreeMemoryAddress;
        memcpy( &nextFreeMemoryAddress, nextFreeMemoryAddress, sizeof(LPVOID));
    }

    return retAddr;
}

void FixedSizeHeap::SyncFree( void *addr)
{
    EnterCriticalSection( &(this->sync));
    this->NonSyncFree( addr);
    LeaveCriticalSection( &(this->sync));
}

void FixedSizeHeap::NonSyncFree( void *addr)
{
    if( nextFreeMemoryAddress == NULL){
        lastFreeMemoryAddress = nextFreeMemoryAddress = addr;
    }
    else{
        memcpy(lastFreeMemoryAddress, &addr, sizeof(LPVOID));
        lastFreeMemoryAddress = addr;
    }
}

HRESULT Allocate( CInt **retAddr)
{
    *retAddr = (CInt *)(true == IsMultiThreaded? MemoryManager[0].SyncAllocate():MemoryManager[0].NonSyncAllocate());

    return S_OK;
}

HRESULT Free( CInt *addr)
{
    true == IsMultiThreaded ? MemoryManager[0].SyncFree( addr) : MemoryManager[0].NonSyncFree( addr);
    return S_OK;
}

HRESULT Allocate( CString **retAddr)
{
    *retAddr = (CString *)(true == IsMultiThreaded? MemoryManager[1].SyncAllocate():MemoryManager[1].NonSyncAllocate());
    
    return S_OK;
}

HRESULT Free( CString *addr)
{
    true == IsMultiThreaded ? MemoryManager[1].SyncFree( addr) : MemoryManager[1].NonSyncFree( addr);

    return S_OK;
}

HRESULT Allocate( Variable **retAddr)
{
    *retAddr = (Variable*)(true == IsMultiThreaded? MemoryManager[2].SyncAllocate():MemoryManager[2].NonSyncAllocate());

    return S_OK;
}

HRESULT Free( Variable *addr)
{
    true == IsMultiThreaded ? MemoryManager[2].SyncFree( addr) : MemoryManager[2].NonSyncFree( addr);

    return S_OK;
}
#ifndef MEMORYMANAGER_H
#define MEMORYMANAGER_H
#include "sdbg_data.h"

HRESULT Allocate(CInt **retAddr);
HRESULT Free(CInt *addr);

HRESULT Allocate(CString **retAddr);
HRESULT Free(CString *addr);

HRESULT Allocate(Variable **retAddr);
HRESULT Free(Variable *addr);

void CustomHeapInitialize();

#endif
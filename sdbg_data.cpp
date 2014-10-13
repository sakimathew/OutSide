#include "sdbg.h"
#include "sdbg_data.h"
#include "System.h"
#include <string>
#include <sstream>
#include <algorithm>
#include <iterator>
#include <stack>

list<CObject*> *pg_ListOfVariables;

CObject::CObject()
{
    list<CObject *>  *tmp = NULL;
    count = 0;
    neverDeleted = false;
    AddRef(); 
#ifdef GARBAGECOLLECTOR
    tmp = pg_GarbageCollector->GetVariableListInUse();
    tmp->push_back(this);
#endif
}

void CObject::AddRef()
{
    if(this->neverDeleted != true){
        this->count++;
    }
}

void CObject::Release()
{
    if(count == 0){
        throw_on_error(E_FAIL, wstring(L"FATAL ERROR: Releasing a freed object."));
    }
    else{
        if(neverDeleted != true){
            this->count--;
        }
        if( this->count == 0){
#ifndef GARBAGECOLLECTOR
            delete this;
#else
            pg_GarbageCollector->ShouldBeDeletedList.push_back( this);
#endif
        }
    }
}
void *CObject::operator new( size_t stAllocateBlock)
{
    return HeapAlloc( g_hndlHeap, HEAP_ZERO_MEMORY | HEAP_ARG, stAllocateBlock);
}

void CObject::operator delete( void *memBlock)
{
    HeapFree( g_hndlHeap, HEAP_ARG, memBlock);
}
/************************************************Definations of CInt**************************************************/

void CInt::PutValue(CObject *value)
{
    CInt *passedValue = reinterpret_cast<CInt*>(value->GetValue());

    if( s_integer != passedValue->GetType()){
        throw_on_error(E_FAIL, wstring(L"FATAL ERROR: Assigning a non-integer expression value to a integer variable."));
    }
    this->data = passedValue->GetData( );
    passedValue->Release();
}

CObject *CInt::GetValue(void)
{
    this->AddRef();
    return this; 
}

CObject *CInt::GetCopyOf(void)
{
    return new CInt(this->data);
}

#ifdef USE_NEW_MEMORY_MODEL
void *CInt::operator new( size_t stAllocateBlock)
{
    CInt *retAddr = NULL;
    
    Allocate( &retAddr);

    return retAddr;
}

void CInt::operator delete( void *memBlock)
{
    Free((CInt *)memBlock);
}
#endif
/************************************************Definations of CIntArray*********************************************/

void CIntArray::PutValue(CObject *arg)
{
    if( s_integer == arg->GetType()){
        CInt *passedArg = reinterpret_cast<CInt*>(arg->GetValue());
        this->arrayOfInt.push_back(passedArg);
    }
    else if( s_integerarray == arg->GetType()){
        CIntArray *passedArg = reinterpret_cast<CIntArray*>( arg->GetValue());

        this->Clear();
        for(DWORD index = 0; index < passedArg->GetCount(); index++){
            this->arrayOfInt.push_back( reinterpret_cast<CInt*>(passedArg->arrayOfInt[index]->GetCopyOf()));
        }
    }
    else{
        throw_on_error(E_FAIL, wstring(L"FATAL ERROR: Assigning a non-integer expression value to a integer array variable."));
    }
}

void CIntArray::Release()
{ 
    if(this->count <= 0){ 
        this->count = 0; 
    }
    else{ 
        this->count--;
        if( this->count == 0){
            for(DWORD index = 0; index < arrayOfInt.size(); index++){
                arrayOfInt[index]->Release();
            }
            arrayOfInt.clear();
        }
    }
}

CObject *CIntArray::GetValue(void)
{
    this->AddRef();
    return this;
}

CObject *CIntArray::GetCopyOf(void)
{
    CIntArray *copy = new CIntArray();

    for(DWORD index = 0; index < this->arrayOfInt.size(); index++){
        copy->PutValue( this->arrayOfInt[index]->GetCopyOf());
    }

    return copy;
}

CObject *CIntArray::GetObject(UINT64 arg)
{
    if( arg >= (UINT64)this->arrayOfInt.size()){
        return NULL;
    }

    this->arrayOfInt[(unsigned int)arg]->AddRef();

    return this->arrayOfInt[(unsigned int)arg];
}

void CIntArray::PutValue(UINT64 arg, CObject *arg1)
{
    UINT64 index = 0;
    CInt *value = NULL;

    if( (arg > (UINT64)(this->arrayOfInt.size() + 1))){
        throw_on_error(E_FAIL, wstring(L"FATAL ERROR: Putting an integer value into an integer array which will overflow."));
    }
    value = reinterpret_cast<CInt*>(arg1->GetValue());
    if(s_integer != value->GetType()){
        throw_on_error(E_FAIL, wstring(L"FATAL ERROR: Assigning a non-integer expression value to a integer array variable."));
    }

    if( arg < this->arrayOfInt.size()){
        this->arrayOfInt[(unsigned int)arg] = value;
    }
    else{
        this->arrayOfInt.push_back(value);
    }
}

UINT64 CIntArray::GetCount(void)
{
    UINT64 count = this->arrayOfInt.size();

    return count;
}

UINT64 CIntArray::GetCurrentIndex(void)
{
    return 0;
}

void CIntArray::Reset()
{
    this->row = 0;
}

/************************************************Definations of CString*********************************************/

void CString::PutValue(CObject *value)
{
    CString *passedArg = reinterpret_cast<CString*> (value->GetValue());

    if( s_string != passedArg->GetType()){
        throw_on_error(E_FAIL, wstring(L"FATAL ERROR: Assigning a non-string expression value to a string array variable."));
    }
    this->data = passedArg->GetData();
    passedArg->Release();
}

CObject* CString::GetValue(void)
{
    this->AddRef();
    return this; 
}

CObject *CString::GetCopyOf(void)
{
    return new CString(this->data);
}

#ifdef USE_NEW_MEMORY_MODEL
void *CString::operator new( size_t stAllocateBlock)
{
    CString *retAddr = NULL;
    
    Allocate( &retAddr);

    return retAddr;
}

void CString::operator delete( void *memBlock)
{
    //this->
    Free((CString *)memBlock);
}
#endif
/************************************************Definations of CStringArray****************************************/

void CStringArray::PutValue(CObject *arg)
{
    if( s_string == arg->GetType()){
        CString *passedArg = reinterpret_cast<CString*>(arg->GetValue());

        this->arrayOfString.push_back( passedArg);
    }
    else if( s_stringarray == arg->GetType()){
        CStringArray *passedArg = reinterpret_cast<CStringArray*>(arg->GetValue());

        this->Clear();
        for(DWORD index = 0; index < passedArg->GetCount(); index++){
            this->arrayOfString.push_back( reinterpret_cast<CString*>(passedArg->arrayOfString[index]->GetCopyOf()));
        }
    }
    else{
        throw_on_error(E_FAIL, wstring(L"FATAL ERROR: Assigning a non-string expression value to a string array variable."));
    }
}

void CStringArray::Release()
{ 
    if(this->count <= 0){ 
        this->count = 0; 
    }
    else{ 
        this->count--;
        if( this->count == 0){
            for(DWORD index = 0; index < arrayOfString.size(); index++){
                arrayOfString[index]->Release();
            }
            arrayOfString.clear();
        }
    }
}

CObject *CStringArray::GetValue(void)
{
    this->AddRef();
    return this;
}

CObject *CStringArray::GetCopyOf(void)
{
    CStringArray *copy = new CStringArray();

    for(DWORD index = 0; index < this->arrayOfString.size(); index++){
        copy->PutValue( this->arrayOfString[index]->GetCopyOf());
    }

    return copy;
}

CObject *CStringArray::GetObject(UINT64 arg)
{
    UINT64 index = 0;
    CObject *retValue = NULL;
    
    if( arg >= (UINT64)this->arrayOfString.size()){
        return NULL;
    }
    this->row = arg+1;
    
    this->arrayOfString[(unsigned int)arg]->AddRef();

    return this->arrayOfString[(unsigned int)arg];
}

void CStringArray::PutValue(UINT64 arg, CObject *arg1)
{
    
    UINT64 index = 0;
    CString *value = NULL;
    bool valueInserted = false;

    if( arg > (UINT64)(this->arrayOfString.size() + 1)){
        throw_on_error(E_FAIL, wstring(L"FATAL ERROR: Putting a string value into an string array which will overflow."));
    }
    value = reinterpret_cast<CString*>(arg1->GetValue());
    if(s_string != value->GetType()){
        throw_on_error(E_FAIL, wstring(L"FATAL ERROR: Assigning a non-string expression value to a string array variable."));
    }

    if( arg < this->arrayOfString.size()){
        this->arrayOfString[(unsigned int)arg] = (CString*)value;
    }
    else{
        this->arrayOfString.push_back(value);
    }
}

UINT64 CStringArray::GetCount(void)
{
    UINT64 count = this->arrayOfString.size();

    return count;
}

UINT64 CStringArray::GetCurrentIndex(void)
{
    return this->row;
}

void CStringArray::Reset()
{
    this->row = 0;
}


/************************************************Definations of SDBGFunction******************************************/

CObject** SDBGFunction::Prologue(void)
{
    DWORD ArgPassedLength = this->ArgumentsPassed.size();
    CObject **variableStack = ExecutionSystem::GetThreadContext()->ThreadStack + ExecutionSystem::GetThreadContext()->CurrentIndex;
    CObject **prevLocalESP = NULL;

    this->instructionPointer = 0;
    ExecutionSystem::GetThreadContext()->currentExecutingFunction = this;
    if( this->typeOfBlock == function_block){
        ExecutionSystem::GetThreadContext()->retAddress = NULL;
    }
    if( ArgPassedLength > 0){
        DWORD index = 0;

        do{
            CObject *tmpValue = this->ArgumentsPassed[index];

            variableStack[index] = tmpValue->GetValue();
            index++;
        }while(index < ArgPassedLength);

        prevLocalESP = ExecutionSystem::GetThreadContext()->localESP;
        ExecutionSystem::GetThreadContext()->localESP = variableStack;
        ExecutionSystem::GetThreadContext()->CurrentIndex += index;
    }

    return prevLocalESP;
}

void SDBGFunction::Epilogue(CObject **prevLocalESP)
{
    DWORD ArgPassedLength = this->ArgumentsPassed.size();
    CObject **variableStack =ExecutionSystem::GetThreadContext()->ThreadStack +ExecutionSystem::GetThreadContext()->CurrentIndex;

    this->instructionPointer = 0;

    if( ArgPassedLength > 0){
        int index = 0;

        while(index < (int)ArgPassedLength){
            CObject *tmp = variableStack[-(index + 1)];
            tmp->Release();
            index++;
        }

        ExecutionSystem::GetThreadContext()->localESP = prevLocalESP;
        ExecutionSystem::GetThreadContext()->CurrentIndex -= index;
        throw_on_error( NULL == prevLocalESP &&ExecutionSystem::GetThreadContext()->CurrentIndex != 0);
    }
}

void SDBGFunction::ExecuteLine(DWORD index)
{
    ReleaseMemory((*(this->functionBody))[instructionPointer]->GetValue());
}

CObject *SDBGFunction::GetValue(void)
{
    CObject *retVal = NULL;

    DontPushIntoScheduler++;
    CObject **prevLocalESP = this->Prologue();

    for(DWORD ip = 0; ip < this->functionBody->size(); ){
        this->ExecuteLine( ip);

        if( NULL != ExecutionSystem::GetThreadContext()->retAddress){
            if( this->typeOfBlock == function_block){ 
                retVal = ExecutionSystem::GetThreadContext()->retAddress;
                ExecutionSystem::GetThreadContext()->retAddress = NULL;
            }
            break;
        }
        if( true == ExecutionSystem::GetThreadContext()->breakCommand || true == ExecutionSystem::GetThreadContext()->continueCommand){
            break;
        }
        if( ExecutionSystem::GetThreadContext()->changeIPOfCurrentFunction == true && ExecutionSystem::GetThreadContext()->changeIPOfThisFunction == this){
            ip = ExecutionSystem::GetThreadContext()->newIP;
            ExecutionSystem::GetThreadContext()->changeIPOfCurrentFunction = false;
            ExecutionSystem::GetThreadContext()->changeIPOfThisFunction = NULL;
        }
        ip++;
        this->instructionPointer = ip;
    }
    
    this->Epilogue(prevLocalESP);
    DontPushIntoScheduler--;

    return retVal;
}

CObject *SDBGFunction::GetCopyOf()
{
    SDBGFunction *retVal = new SDBGFunction(*(this->functionname));
    vector<CObject*>::iterator it;

    LIST_LOOP(it,(this->ArgumentsPassed)){
        retVal->ArgumentsPassed.push_back( (*it)->GetCopyOf());
    }

    retVal->functionBody = this->functionBody;
    retVal->ArgPassedCount = this->ArgPassedCount;
    retVal->ArgPushCount = 0;
    retVal->instructionPointer = this->instructionPointer;
    if( NULL != this->returnValue){
        retVal->returnValue = this->returnValue->GetCopyOf();
    }

    return (CObject*)retVal;
}

void SDBGFunction::PutValue( CObject *arg)
{
    if( NULL == arg){
        throw_on_error(E_FAIL, wstring(L"FATAL ERROR: Passing a NULL object to a SDBG function."));
    }

    this->ArgumentsPassed[ArgPushCount] = arg;
    ArgPushCount++;
}

void SDBGFunction::AddFunctionBody(CObject *arg)
{
    throw_on_error(NULL == arg, wstring(L"FATAL ERROR: Inserting a NULL object into a SDBG function body."));
    this->functionBody->push_back(arg);
}

/************************************************Definations of Variable*********************************************/
#ifdef USE_NEW_MEMORY_MODEL
void *Variable::operator new( size_t stAllocateBlock)
{
    Variable *retAddr = NULL;
    
    Allocate( &retAddr);

    return retAddr;
}

void Variable::operator delete( void *memBlock)
{
    Free((Variable *)memBlock);
}
#endif
/************************************************Definations of LocalVariiable****************************************/

void LocalVariable::PutValue(CObject *arg)
{

    if( NULL == index){
        ExecutionSystem::GetThreadContext()->localESP[positionInStack] = arg;
    }
    else{
        CInt *ptr = reinterpret_cast<CInt*>(index->GetValue( ));
        CObjectList *objList = reinterpret_cast<CObjectList*>(ExecutionSystem::GetThreadContext()->localESP[positionInStack]);

        objList->PutValue( ptr->GetData(), arg);
    }
}

CObject *LocalVariable::GetValue(void)
{
    if( NULL == index){
        return (ExecutionSystem::GetThreadContext()->localESP[positionInStack])->GetValue();
    }
    else{
        CObject *retVal = NULL;
        CInt *ptr = reinterpret_cast<CInt*>(index->GetValue());
        CObjectList *objList = reinterpret_cast<CObjectList*>(ExecutionSystem::GetThreadContext()->localESP[positionInStack]);

        retVal = objList->GetObject( ptr->GetData());
        ptr->Release();

        return retVal;
    }
}

CObject *LocalVariable::GetCopyOf(void)
{
    LocalVariable *ret = new LocalVariable(positionInStack);

    ret->index = this->index;
    
    return ret;
}

/************************************************Definations of GlobalVariable****************************************/

void GlobalVariable::PutValue(CObject *arg)
{
    throw_on_error(g_GlobalSymbolTable.size() < positionInStack, wstring(L"FATAL ERROR: Global variable is pointing to an invalid memory location."));

    if( NULL == index){
        g_GlobalSymbolTable[positionInStack] = arg;
    }
    else{
        CInt *ptr = reinterpret_cast<CInt*>(index->GetValue( ));
        CObjectList *objList = reinterpret_cast<CObjectList*>(g_GlobalSymbolTable[positionInStack]);

        objList->PutValue( ptr->GetData(), arg);
        arg->Release();
    }
}

CObject *GlobalVariable::GetValue(void)
{
    throw_on_error(g_GlobalSymbolTable.size() < positionInStack, wstring(L"FATAL ERROR: Global variable is pointing to an invalid memory location."));

    if( NULL == index){
        return g_GlobalSymbolTable[positionInStack]->GetValue();
    }
    else{
        CObject *retVal = NULL;
        CInt *ptr = reinterpret_cast<CInt*>(index->GetValue( ));
        CObjectList *objList = reinterpret_cast<CObjectList*>(g_GlobalSymbolTable[positionInStack]);

        retVal = objList->GetObject( ptr->GetData());
        ptr->Release();

        return retVal;
    }
}

CObject *GlobalVariable::GetCopyOf(void)
{
    GlobalVariable *ret = new GlobalVariable(positionInStack);

    ret->index = this->index;
    
    return ret;
}

/************************************************Definations of CDecisionTree****************************************/

void CDecisionTree::PutValue(CObject *arg){ 
    SDBGFunction *ftmp = dynamic_cast<SDBGFunction*>(arg);
    CDecisionTree *ftree = dynamic_cast<CDecisionTree*>(arg);

    throw_on_error(NULL == ftmp && NULL == ftree);

    if( NULL != ftmp){
        if( ftmp->GetBlockType() == if_block){ 
            if( NULL == this->left){ 
                this->left = ftmp; 
                ftmp->AddRef();
            }
            else if( NULL != dynamic_cast<CDecisionTree*>(this->right)){ this->right->PutValue( ftmp); }
            else{ throw_on_error(E_FAIL);}
        }
        else if( ftmp->GetBlockType() == else_block){
            throw_on_error(NULL == this->left); 
            if( NULL == this->right){ 
                this->right = ftmp; 
                ftmp->AddRef();
            }
            else if( NULL != dynamic_cast<CDecisionTree*>(this->right)){ this->right->PutValue( ftmp); }
        }
        else { throw_on_error(E_FAIL);}
    }
    else if( NULL != ftree){
        throw_on_error( NULL == this->left);
        if( NULL == this->right){ 
            this->right = ftree; 
            ftree->AddRef();
        }
        else if( NULL != dynamic_cast<SDBGFunction*>(this->right)){
            throw_on_error(E_FAIL); 
        }
        else{ 
            this->right->PutValue( ftree); 
        }
    }
}

CObject *CDecisionTree::GetValue(void)
{
    CInt *iTmp = NULL;
    DWORD boolean_value = 0;
    
    throw_on_error(NULL == this->left);

    iTmp = reinterpret_cast<CInt*>(this->value->GetValue());
    throw_on_error(NULL == iTmp);
    boolean_value = (DWORD)iTmp->GetData();
    iTmp->Release();

    if( 0 != boolean_value){
        return this->left->GetValue();
    }
    else{
        if( NULL != this->right){
            return this->right->GetValue();
        }
        else{
            return new CInt(0);
        }
    }
}
#ifndef __SDBG_DATA_H__
#define __SDBG_DATA_H__
#include <stdarg.h>
#include <objbase.h>
#include <Unknwn.h>
#include <iostream>
#include <string>
#include <list>
#include "PrepocessorMacros.h"

using namespace std;

#ifdef __cplusplus
extern "C" {
#endif

typedef enum
{
    s_integer,
    s_integerarray,
    s_string,
    s_stringarray,
    s_command,
    s_function,
    s_stackvalue,
    s_globalheapvalue,
}data_type;

class CObject
{
protected:
    DWORD count;
    bool neverDeleted;
public:
    CObject();
    virtual void AddRef();
    virtual void Release();
    DWORD GetCount(){ return count; }
    void *operator new ( size_t stAllocateBlock);
    void operator delete( void *memBlock);
    virtual ~CObject(){ ; };
    virtual data_type GetType() = 0;
    virtual void PutValue(CObject *) = 0;
    virtual CObject *GetValue(void) = 0;
    virtual CObject *GetCopyOf(void) = 0;
    virtual void NeverDelete(){ neverDeleted = true;}
};

class CObjectList : public CObject
{
    public:
        virtual CObject *GetObject(UINT64) = 0;
        virtual void PutValue(UINT64 , CObject *) = 0;
        virtual UINT64 GetCount(void) = 0;
        virtual UINT64 GetCurrentIndex(void) = 0;
        virtual void Clear() = 0;
        virtual void Reset() = 0;
};

class CInt : public CObject
{
    private:
        UINT64 data;
    public:
        CInt():data(0){ };
        CInt(UINT64 arg):data(arg){};
        virtual ~CInt()
        { 
            ;
        };
        data_type GetType(){ return s_integer; };
#ifdef USE_NEW_MEMORY_MODEL
        void *operator new ( size_t stAllocateBlock);
        void operator delete( void *memBlock);
#endif
        void PutValue(CObject *);
        CObject *GetValue(void);
        CObject *GetCopyOf(void);
        UINT64 GetData(void){ return data; };
        void PutData(UINT64 arg){ data = arg;};
};

class CIntArray : public CObjectList
{
    private:
        vector<CInt *> arrayOfInt;
        UINT64 row;
    public:
        CIntArray():row(0){ };
        virtual ~CIntArray()
        {
            arrayOfInt.clear();
        }
        data_type GetType(){ return s_integerarray; };
        void Release();
        void PutValue(CObject *);
        CObject *GetValue(void);
        CObject *GetCopyOf(void);
        CObject *GetObject(UINT64);
        void PutValue(UINT64 , CObject *);
        UINT64 GetCount(void);
        UINT64 GetCurrentIndex(void);
        void Clear()
        {
            for(DWORD index = 0; index < arrayOfInt.size(); index++){ arrayOfInt[index]->Release(); }
            arrayOfInt.clear();
        }
        void Reset(void);
};

class CString : public CObject
{
    private:
        wstring data;
        char unused;
    public:
        CString():data(L""){ };
        CString(wstring &arg):data(arg){ };
        virtual ~CString()
        {
            ;
        }
        data_type GetType(){ return s_string; };
#ifdef USE_NEW_MEMORY_MODEL
        void *operator new ( size_t stAllocateBlock);
        void operator delete( void *memBlock);
#endif
        void PutValue(CObject *);
        CObject *GetValue(void);
        CObject *GetCopyOf(void);
        wstring GetData(void){ return data; };
        void PutData(wstring &arg){ data = arg;};
};

class CStringArray : public CObjectList
{
    private:
        vector<CString *> arrayOfString;
        UINT64 row;
        char unused;
    public:
        CStringArray():row(0){ };
        ~CStringArray()
        {
            arrayOfString.clear();
        }
        data_type GetType(){ return s_stringarray; };
        void PutValue(CObject *);
        void Release();
        CObject *GetValue(void);
        CObject *GetCopyOf(void);
        CObject *GetObject(UINT64);
        void PutValue(UINT64 , CObject *);
        UINT64 GetCount(void);
        UINT64 GetCurrentIndex(void);
        void Clear()
        {  
            for( DWORD index = 0; index < arrayOfString.size(); index++){ arrayOfString[index]->Release();}
            arrayOfString.clear();
            this->Reset();
        }
        void Reset(void);
        
};

class CCommand: public CObject
{
    public:
        virtual wstring get_Syntax(void) = 0;
        virtual wstring get_OptionalSyntax(void) = 0;
        virtual wstring get_Name(void) = 0;
        virtual void put_Name(wstring &) = 0;
        virtual CObject* get_Argument(DWORD) = 0;
};

struct Variable:public CObject
{
    DWORD positionInStack;
    CObject *index, *column;
    Variable():positionInStack(0),index(NULL), column(NULL){};
    Variable(DWORD arg):positionInStack(arg),index(NULL){};
#ifdef USE_NEW_MEMORY_MODEL
    void *operator new ( size_t stAllocateBlock);
    void operator delete( void *memBlock);
#endif
};

struct LocalVariable:public Variable
{
    LocalVariable():Variable(0){ };
    LocalVariable(DWORD pos):Variable(pos){ };
    ~LocalVariable()
    { 
        if( NULL != index){ 
            index->Release();
            index = NULL;
        };
    }
    data_type GetType(){ return s_stackvalue; };
    CObject *GetValue(void);
    void PutValue(CObject *arg);
    CObject *GetCopyOf(void);
};

struct GlobalVariable:public Variable
{
    GlobalVariable():Variable(0){};
    GlobalVariable(DWORD pos):Variable(pos){};
    ~GlobalVariable(){ ;}
    data_type GetType(){ return s_globalheapvalue; };
    CObject *GetValue(void);
    void PutValue(CObject *arg);
    CObject *GetCopyOf(void);
};

class SDBGCommand: public CCommand
{
    protected:
        wstring CommandName;
        vector<CObject*> arguments;
        wstring syntax;
        wstring optional_syntax;
        DWORD argsPushed;
    public:
        SDBGCommand(vector<CObject*> *argPassed, wstring argCommandName, wstring argSyntax);
        virtual ~SDBGCommand()
        {
            for(DWORD index = 0; index < arguments.size(); index++){
                arguments[index]->Release();
            }
            arguments.clear();
        }
        data_type GetType(){ return s_command; };
        wstring ToString(void){ return CommandName; };
        void PutValue(CObject *arg)
        { 
            if(NULL != arg){ 
                argsPushed++; 
                arguments.push_back(arg); 
                arg->AddRef();
            }
        };
        wstring get_Syntax(void){ return syntax; };

        wstring get_OptionalSyntax(void) { return optional_syntax;};
        wstring get_Name(void) { return CommandName;};
        void put_Name(wstring &arg){ CommandName = arg;};
        CObject *get_RawArgument(DWORD arg){ return this->arguments[arg]; }
        virtual CObject *GetValue(void) = 0;
        virtual CObject *GetCopyOf(void) = 0;
        CObject *get_Argument( DWORD arg)
        { 
            if( arg >= arguments.size()){
                return NULL;
            }
            return ((this->arguments[arg])->GetValue()); 
        }

};

typedef enum{
    undefined_block = 1,
    function_block,
    blank_block,
    if_block,
    else_block,
    loop_block
}block_type;

class SDBGFunction: public CCommand
{
    private:
        CObject *returnValue;
        vector<CObject*> *functionBody;
        vector<CObject*> ArgumentsPassed;
        DWORD ArgPushCount;
        DWORD ArgPassedCount;
        wstring *functionname;
        block_type typeOfBlock;
        bool setExitStatus;
        bool IsFullCopy;
        DWORD instructionPointer;
    public:
        SDBGFunction(wstring arg):functionname(new wstring(arg)),returnValue(NULL),ArgPassedCount(0),ArgPushCount(0),typeOfBlock(function_block),setExitStatus(false),instructionPointer(0),IsFullCopy(false)
        { 
            functionBody = new vector<CObject*>; 
        }
        SDBGFunction(block_type arg):functionname(NULL),returnValue(NULL),ArgPassedCount(0),ArgPushCount(0),typeOfBlock(arg),setExitStatus(false),instructionPointer(0),IsFullCopy(false)
        {
            functionBody = new vector<CObject*>; 
        }
        virtual ~SDBGFunction()
        {
            this->functionname = NULL;
            for(auto it = this->ArgumentsPassed.begin(); it != this->ArgumentsPassed.end(); it++){
                (*it)->Release();
            }
            this->ArgumentsPassed.clear();
            if( this->IsFullCopy == true)
            {
                for(auto it = this->functionBody->begin(); it != this->functionBody->end(); it++){
                    (*it)->Release();
                }	
            }
            this->functionBody = NULL;
            if( NULL != returnValue){ returnValue->Release();}
        }
        data_type GetType()
        { 
            return s_function; 
        }
        void SetReturnValue(CObject *arg)
        { 
            this->returnValue = arg; 
        }
        wstring get_Name(void) 
        { 
            return *functionname;
        }
        vector<CObject*> *get_ArgumentsPassed(void)
        { 
            return &ArgumentsPassed;
        };
        block_type GetBlockType()
        { 
            return this->typeOfBlock;
        }
        bool Pause(){ return true;};
        bool Resume(){ return true;};
        DWORD GetInstructionPointer(){ return instructionPointer;};
        bool SetInstructionPointer(DWORD arg){ if( arg >= functionBody->size()){ return false;} else{ instructionPointer = arg; return true;}};
        void put_Name(wstring &arg){ return; };
        CObject* get_Argument(DWORD arg) 
        {
            if( arg >= ArgumentsPassed.size()){
                return NULL;
            }
            return ((this->ArgumentsPassed[arg])->GetValue()); 
        };
        wstring get_Syntax(void){ return L"";};
        wstring get_OptionalSyntax(void){ return L"";};
        void PutValue(CObject *);
        CObject *GetValue(void);
        CObject *GetCopyOf(void);
        CObject **Prologue(void);
        void Epilogue(CObject **);
        void ExecuteLine(DWORD);
        void AddFunctionBody(CObject *);
};

class CDecisionTree:public CObject
{
    CObject *left, *right;
    CObject *value;
public:
    CDecisionTree():left(NULL),right(NULL){}
    virtual ~CDecisionTree()
    {
        delete this;
    }
    data_type GetType(){ return s_function; };
    void Add(CObject *arg){ value = arg;}
    void PutValue(CObject *arg);
    CObject *GetLeftValue(void){ return left; }
    CObject *GetRightValue(void){ return right; }
    CObject *GetValue(void);
    CObject *GetCopyOf(void){ return this; }
    CObject *GetFullCopy();
};

class ReturnValue{
public:
    CObject *retData;
    ReturnValue( CObject *ptr){ retData = ptr->GetCopyOf(); }
};

#ifdef __cplusplus
};
#endif

#endif


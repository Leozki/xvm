//
// Created by Leozki on 2017/7/3.
//

#ifndef XVM_SCRIPT_H
#define XVM_SCRIPT_H

typedef struct _Value {
    int iType;
    union {
        int iIntLiteral;
        float fFloatLiteral;
        char *pstrStringLiteral;
        int iStackIndex;
        int iInstrIndex;
        int iFuncIndex;
        int iHostAPICallIndex;
        int iReg;
    };
    int iOffsetIndex;
} Value;


typedef struct _RuntimeStack {
    Value *pElmnts;
    int iSize;

    int iTopIndex;
    int iFrameIndex;

} RuntimeStack;


typedef struct _Func {
    int iEntryPoint;
    int iParamCount;
    int iLocalDataSize;
    int iStackFrameSize;
} Func;

typedef struct _Instr {
    int iOpcode;
    int iOpCount;
    Value *pOpList;
} Instr;

typedef struct _InstrStream {
    Instr *pInstrs;
    int iSize;
    // stream
    int iCurrInstr;
} InstrStream;


typedef struct _HostAPICallTable {
    char **ppstrCalls;
    int iSize;
} HostAPICallTable;

typedef struct _Script {
    int iIsActive;

    // Header data

    int iGlobalDataSize;
    int iIsMainFuncPresent;
    int iMainFuncIndex;

    // Runtime tracking

    int iIsRunning;
    int iIsPaused;
    int iPauseEndTime;

    // Register file

    Value _RetVal;

    // Script data

    InstrStream InstrStream;
    RuntimeStack Stack;
    Func *pFuncTable;
    HostAPICallTable HostAPICallTable;
} Script;


#endif //XVM_SCRIPT_H

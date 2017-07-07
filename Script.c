//
// Created by Leozki on 2017/7/3.
//

#include <stdio.h>
#include <malloc.h>
#include <afxres.h>
#include <conio.h>
#include "Script.h"
#include "Stack.h"


Script g_Scripts[MAX_THREAD_COUNT];
int g_iCurrThread; // 当前正在运行的线程索引
int g_iCurrThreadActiveTime; // 当前线程激活的时间

// 支持的关键字
char g_ppstrMnemonics[][12] =
        {
                "Mov",
                "Add", "Sub", "Mul", "Div", "Mod", "Exp", "Neg", "Inc", "Dec",
                "And", "Or", "XOr", "Not", "ShL", "ShR",
                "Concat", "GetChar", "SetChar",
                "Jmp", "JE", "JNE", "JG", "JL", "JGE", "JLE",
                "Push", "Pop",
                "Call", "Ret", "CallHost",
                "Pause", "Exit"
        };


void ResetScript(int iThreadIndex) {
    int iMainFuncIndex = g_Scripts[iThreadIndex].iMainFuncIndex;
    if (g_Scripts[iThreadIndex].pFuncTable) {
        if (g_Scripts[iThreadIndex].iIsMainFuncPresent)
            g_Scripts[iThreadIndex].InstrStream.iCurrInstr = g_Scripts[iThreadIndex].pFuncTable[iMainFuncIndex].iEntryPoint;
    }


    g_Scripts[iThreadIndex].Stack.iTopIndex = 0;
    g_Scripts[iThreadIndex].Stack.iFrameIndex = 0;

    for (int iCurrElmntIndex = 0; iCurrElmntIndex < g_Scripts[iThreadIndex].Stack.iSize; ++iCurrElmntIndex)
        g_Scripts[iThreadIndex].Stack.pElmnts[iCurrElmntIndex].iType = OP_TYPE_NULL;


    g_Scripts[iThreadIndex].iIsPaused = FALSE;


    PushFrame(iThreadIndex, g_Scripts[iThreadIndex].iGlobalDataSize);

    PushFrame(iThreadIndex, g_Scripts[iThreadIndex].pFuncTable[iMainFuncIndex].iStackFrameSize + 1);
}

int LoadScript(char *pstrFilename, int *iThdIndex) {

    int iThreadIndex = 0;
    int iFreeThreadFound = FALSE;
    for (int iCurrThreadIndex = 0; iCurrThreadIndex < MAX_THREAD_COUNT; ++iCurrThreadIndex) {

        if (!g_Scripts[iCurrThreadIndex].iIsActive) {
            iThreadIndex = iCurrThreadIndex;
            iFreeThreadFound = TRUE;
            break;
        }
    }

    if (!iFreeThreadFound)
        return LOAD_ERROR_OUT_OF_THREADS;

    (*iThdIndex) = iThreadIndex;

    FILE *pScriptFile;
    if (!(pScriptFile = fopen(pstrFilename, "rb")))
        return LOAD_ERROR_FILE_IO;

    char *pstrIDString;
    if (!(pstrIDString = (char *) malloc(5)))
        return LOAD_ERROR_OUT_OF_MEMORY;

    fread(pstrIDString, 4, 1, pScriptFile);
    pstrIDString[strlen(XSE_ID_STRING)] = '\0';

    if (strcmp(pstrIDString, XSE_ID_STRING) != 0)
        return LOAD_ERROR_INVALID_XSE;

    free(pstrIDString);

    int iMajorVersion = 0,
            iMinorVersion = 0;

    fread(&iMajorVersion, 1, 1, pScriptFile);
    fread(&iMinorVersion, 1, 1, pScriptFile);

    if (iMajorVersion != 0 || iMinorVersion != 4)
        return LOAD_ERROR_UNSUPPORTED_VERS;

    fread(&g_Scripts[iThreadIndex].Stack.iSize, 4, 1, pScriptFile);


    if (g_Scripts[iThreadIndex].Stack.iSize == 0)
        g_Scripts[iThreadIndex].Stack.iSize = DEF_STACK_SIZE;

    int iStackSize = g_Scripts[iThreadIndex].Stack.iSize;
    if (!(g_Scripts[iThreadIndex].Stack.pElmnts = (Value *) malloc(iStackSize * sizeof(Value))))
        return LOAD_ERROR_OUT_OF_MEMORY;

    fread(&g_Scripts[iThreadIndex].iGlobalDataSize, 4, 1, pScriptFile);

    fread(&g_Scripts[iThreadIndex].iIsMainFuncPresent, 1, 1, pScriptFile);


    fread(&g_Scripts[iThreadIndex].iMainFuncIndex, 4, 1, pScriptFile);

    fread(&g_Scripts[iThreadIndex].InstrStream.iSize, 4, 1, pScriptFile);

    if (!(g_Scripts[iThreadIndex].InstrStream.pInstrs = (Instr *) malloc(
            g_Scripts[iThreadIndex].InstrStream.iSize * sizeof(Instr))))
        return LOAD_ERROR_OUT_OF_MEMORY;

    for (int iCurrInstrIndex = 0; iCurrInstrIndex < g_Scripts[iThreadIndex].InstrStream.iSize; ++iCurrInstrIndex) {

        g_Scripts[iThreadIndex].InstrStream.pInstrs[iCurrInstrIndex].iOpcode = 0;
        fread(&g_Scripts[iThreadIndex].InstrStream.pInstrs[iCurrInstrIndex].iOpcode, 2, 1, pScriptFile);

        g_Scripts[iThreadIndex].InstrStream.pInstrs[iCurrInstrIndex].iOpCount = 0;
        fread(&g_Scripts[iThreadIndex].InstrStream.pInstrs[iCurrInstrIndex].iOpCount, 1, 1, pScriptFile);

        int iOpCount = g_Scripts[iThreadIndex].InstrStream.pInstrs[iCurrInstrIndex].iOpCount;

        Value *pOpList;
        if (!(pOpList = (Value *) malloc(iOpCount * sizeof(Value))))
            return LOAD_ERROR_OUT_OF_MEMORY;

        for (int iCurrOpIndex = 0; iCurrOpIndex < iOpCount; ++iCurrOpIndex) {

            pOpList[iCurrOpIndex].iType = 0;
            fread(&pOpList[iCurrOpIndex].iType, 1, 1, pScriptFile);


            switch (pOpList[iCurrOpIndex].iType) {

                case OP_TYPE_INT:
                    fread(&pOpList[iCurrOpIndex].iIntLiteral, sizeof(int), 1, pScriptFile);
                    break;

                case OP_TYPE_FLOAT:
                    fread(&pOpList[iCurrOpIndex].fFloatLiteral, sizeof(float), 1, pScriptFile);
                    break;

                case OP_TYPE_STRING:

                    fread(&pOpList[iCurrOpIndex].iIntLiteral, sizeof(int), 1, pScriptFile);
                    pOpList[iCurrOpIndex].iType = OP_TYPE_STRING;
                    break;

                case OP_TYPE_INSTR_INDEX:
                    fread(&pOpList[iCurrOpIndex].iInstrIndex, sizeof(int), 1, pScriptFile);
                    break;

                case OP_TYPE_ABS_STACK_INDEX:
                    fread(&pOpList[iCurrOpIndex].iStackIndex, sizeof(int), 1, pScriptFile);
                    break;

                case OP_TYPE_REL_STACK_INDEX:
                    fread(&pOpList[iCurrOpIndex].iStackIndex, sizeof(int), 1, pScriptFile);
                    fread(&pOpList[iCurrOpIndex].iOffsetIndex, sizeof(int), 1, pScriptFile);
                    break;

                case OP_TYPE_FUNC_INDEX:
                    fread(&pOpList[iCurrOpIndex].iFuncIndex, sizeof(int), 1, pScriptFile);
                    break;

                case OP_TYPE_HOST_API_CALL_INDEX:
                    fread(&pOpList[iCurrOpIndex].iHostAPICallIndex, sizeof(int), 1, pScriptFile);
                    break;

                case OP_TYPE_REG:
                    fread(&pOpList[iCurrOpIndex].iReg, sizeof(int), 1, pScriptFile);
                    break;
                default:
                    break;
            }
        }

        g_Scripts[iThreadIndex].InstrStream.pInstrs[iCurrInstrIndex].pOpList = pOpList;
    }

    int iStringTableSize;
    fread(&iStringTableSize, 4, 1, pScriptFile);

    if (iStringTableSize) {
        char **ppstrStringTable;
        if (!(ppstrStringTable = (char **) malloc(iStringTableSize * sizeof(char *))))
            return LOAD_ERROR_OUT_OF_MEMORY;
        for (int iCurrStringIndex = 0; iCurrStringIndex < iStringTableSize; ++iCurrStringIndex) {

            size_t iStringSize;
            fread(&iStringSize, 4, 1, pScriptFile);

            char *pstrCurrString;
            if (!(pstrCurrString = (char *) malloc(iStringSize + 1)))
                return LOAD_ERROR_OUT_OF_MEMORY;


            fread(pstrCurrString, iStringSize, 1, pScriptFile);
            pstrCurrString[iStringSize] = '\0';


            ppstrStringTable[iCurrStringIndex] = pstrCurrString;
        }

        for (int iCurrInstrIndex = 0; iCurrInstrIndex < g_Scripts[iThreadIndex].InstrStream.iSize; ++iCurrInstrIndex) {

            int iOpCount = g_Scripts[iThreadIndex].InstrStream.pInstrs[iCurrInstrIndex].iOpCount;
            Value *pOpList = g_Scripts[iThreadIndex].InstrStream.pInstrs[iCurrInstrIndex].pOpList;

            for (int iCurrOpIndex = 0; iCurrOpIndex < iOpCount; ++iCurrOpIndex) {

                if (pOpList[iCurrOpIndex].iType == OP_TYPE_STRING) {

                    int iStringIndex = pOpList[iCurrOpIndex].iIntLiteral;

                    char *pstrStringCopy;
                    if (!(pstrStringCopy = (char *) malloc(strlen(ppstrStringTable[iStringIndex]) + 1)))
                        return LOAD_ERROR_OUT_OF_MEMORY;
                    strcpy(pstrStringCopy, ppstrStringTable[iStringIndex]);

                    pOpList[iCurrOpIndex].pstrStringLiteral = pstrStringCopy;
                }
            }
        }

        for (int iCurrStringIndex = 0; iCurrStringIndex < iStringTableSize; ++iCurrStringIndex)
            free(ppstrStringTable[iCurrStringIndex]);


        free(ppstrStringTable);
    }

    int iFuncTableSize;
    fread(&iFuncTableSize, 4, 1, pScriptFile);

    if (!(g_Scripts[iThreadIndex].pFuncTable = (Func *) malloc(iFuncTableSize * sizeof(Func))))
        return LOAD_ERROR_OUT_OF_MEMORY;


    for (int iCurrFuncIndex = 0; iCurrFuncIndex < iFuncTableSize; ++iCurrFuncIndex) {
        int iEntryPoint;
        fread(&iEntryPoint, 4, 1, pScriptFile);


        int iParamCount = 0;
        fread(&iParamCount, 1, 1, pScriptFile);

        int iLocalDataSize;
        fread(&iLocalDataSize, 4, 1, pScriptFile);

        int iStackFrameSize = iParamCount + 1 + iLocalDataSize;

        g_Scripts[iThreadIndex].pFuncTable[iCurrFuncIndex].iEntryPoint = iEntryPoint;
        g_Scripts[iThreadIndex].pFuncTable[iCurrFuncIndex].iParamCount = iParamCount;
        g_Scripts[iThreadIndex].pFuncTable[iCurrFuncIndex].iLocalDataSize = iLocalDataSize;
        g_Scripts[iThreadIndex].pFuncTable[iCurrFuncIndex].iStackFrameSize = iStackFrameSize;
    }


    fread(&g_Scripts[iThreadIndex].HostAPICallTable.iSize, 4, 1, pScriptFile);

    if (!(g_Scripts[iThreadIndex].HostAPICallTable.ppstrCalls = (char **) malloc(
            g_Scripts[iThreadIndex].HostAPICallTable.iSize * sizeof(char *))))
        return LOAD_ERROR_OUT_OF_MEMORY;


    for (int iCurrCallIndex = 0; iCurrCallIndex < g_Scripts[iThreadIndex].HostAPICallTable.iSize; ++iCurrCallIndex) {
        size_t iCallLength = 0;
        fread(&iCallLength, 1, 1, pScriptFile);
        char *pstrCurrCall;
        if (!(pstrCurrCall = (char *) malloc(iCallLength + 1)))
            return LOAD_ERROR_OUT_OF_MEMORY;
        fread(pstrCurrCall, iCallLength, 1, pScriptFile);
        pstrCurrCall[iCallLength] = '\0';
        g_Scripts[iThreadIndex].HostAPICallTable.ppstrCalls[iCurrCallIndex] = pstrCurrCall;
    }

    fclose(pScriptFile);

    g_Scripts[iThreadIndex].iIsActive = TRUE;
    g_Scripts[iThreadIndex].iIsRunning = TRUE;
    ResetScript(iThreadIndex);
    return LOAD_OK;
}


int GetCurrTime() {
    return (int) GetTickCount();
}

void RunScripts() {
    int iExitExecLoop = FALSE;

    int iCurrInstr;
    g_iCurrThreadActiveTime = GetCurrTime();
    int iCurrTime;

    while (!kbhit()) {

        iCurrTime = GetCurrTime();
        if (iCurrTime > g_iCurrThreadActiveTime + THREAD_TIMESLICE_DUR ||
            !g_Scripts[g_iCurrThread].iIsRunning) {

            while (TRUE) {
                ++g_iCurrThread;
                if (g_iCurrThread >= MAX_THREAD_COUNT)
                    g_iCurrThread = 0;
                if (g_Scripts[g_iCurrThread].iIsActive && g_Scripts[g_iCurrThread].iIsRunning)
                    break;
            }
            g_iCurrThreadActiveTime = iCurrTime;
        }


        if (g_Scripts[g_iCurrThread].iIsPaused) {
            if (iCurrTime >= g_Scripts[g_iCurrThread].iPauseEndTime) {
                g_Scripts[g_iCurrThread].iIsPaused = FALSE;
            } else {
                continue;
            }
        }


        iCurrInstr = g_Scripts[g_iCurrThread].InstrStream.iCurrInstr;

        int iOpcode = g_Scripts[g_iCurrThread].InstrStream.pInstrs[iCurrInstr].iOpcode;

        ++iCurrInstr;
        printf("\t%d", g_iCurrThread);
        printf("\t");
        if (iOpcode < 10)
            printf(" %d", iOpcode);
        else
            printf("%d", iOpcode);
        printf(" %s ", g_ppstrMnemonics[iOpcode]);

        // 处理OpCode

        switch (iOpcode) {
            default:
                break;
        }

        printf("\n");

        if (iExitExecLoop)
            break;
        g_Scripts[g_iCurrThread].InstrStream.iCurrInstr = iCurrInstr;
    }
}
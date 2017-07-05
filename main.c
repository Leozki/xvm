#include <stdio.h>
#include <malloc.h>
#include <string.h>
#include "Script.h"
#include "Constants.h"


Script g_Scripts[MAX_THREAD_COUNT];

int g_iCurrThread;                                // 当前正在运行的线程索引


void Init() {
    // 给每个线程分配一个 script对象
    for (int iCurrScriptIndex = 0; iCurrScriptIndex < MAX_THREAD_COUNT; ++iCurrScriptIndex) {
        g_Scripts[iCurrScriptIndex].iIsActive = FALSE;

        g_Scripts[iCurrScriptIndex].iIsRunning = FALSE;
        g_Scripts[iCurrScriptIndex].iIsMainFuncPresent = FALSE;
        g_Scripts[iCurrScriptIndex].iIsPaused = FALSE;

        g_Scripts[iCurrScriptIndex].InstrStream.pInstrs = NULL;
        g_Scripts[iCurrScriptIndex].Stack.pElmnts = NULL;
        g_Scripts[iCurrScriptIndex].pFuncTable = NULL;
        g_Scripts[iCurrScriptIndex].HostAPICallTable.ppstrCalls = NULL;
    }

    g_iCurrThread = 0;
}

int LoadScript(char *pstrFilename, int iThreadIndex) {

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
    // ResetScript(iThreadIndex);
    return LOAD_OK;
}


int main(int argc, char *argv[]) {
    // 允许加载多个 .xse 文件同时运行
    if (argc < 2) {
        printf("Usage:\tXVM Script.XSE\n");
        return 0;
    }
    char **pstrScriptFilenames = (char **) malloc((argc - 1) * sizeof(char *));

    for (int iCurrFilename = 1; iCurrFilename < argc; ++iCurrFilename) {
        pstrScriptFilenames[iCurrFilename - 1] = (char *) malloc((strlen(argv[iCurrFilename]) + 5));
        strcpy(pstrScriptFilenames[iCurrFilename - 1], argv[iCurrFilename]);
        strupr(pstrScriptFilenames[iCurrFilename - 1]);

        // 如果没有输入后缀,则自动补上
        if (!strstr(pstrScriptFilenames[iCurrFilename - 1], EXEC_FILE_EXT))
            strcat(pstrScriptFilenames[iCurrFilename - 1], EXEC_FILE_EXT);
        printf("FileName = %s", pstrScriptFilenames[iCurrFilename - 1]);
    }

    Init();

    return 0;
}
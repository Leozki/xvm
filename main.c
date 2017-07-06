#include <stdio.h>
#include <malloc.h>
#include <string.h>
#include "Script.h"




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

    // 加载每个源文件,每个文件单独分配一个线程执行
    for (int iCurrScript = 0; iCurrScript < argc - 1; ++iCurrScript) {
        int iThreadIndex;
        int iErrorCode = LoadScript(pstrScriptFilenames[iCurrScript], &iThreadIndex);
        if (iErrorCode == LOAD_OK) {
            printf(" - Script \"%s\" has been loaded into thread %d.\n", pstrScriptFilenames[iCurrScript],
                   iThreadIndex);
        } else {
            printf("Error: ");
            switch (iErrorCode) {
                case LOAD_ERROR_FILE_IO:
                    printf("File I/O error");
                    break;
                case LOAD_ERROR_INVALID_XSE:
                    printf(".XSE structure invalid");
                    break;
                case LOAD_ERROR_UNSUPPORTED_VERS:
                    printf("Unsupported .XSE format version");
                    break;
                case LOAD_ERROR_OUT_OF_MEMORY:
                    printf("Out of memory");
                    break;
                case LOAD_ERROR_OUT_OF_THREADS:
                    printf("Out of threads");
                    break;
                default:
                    break;
            }
            printf(".\n");
            return 0;
        }
        free(pstrScriptFilenames[iCurrScript]);
    }
    free(pstrScriptFilenames);



    return 0;
}
#include <stdio.h>
#include <malloc.h>
#include <string.h>
#include "Script.h"


#ifndef FALSE
#define FALSE               0
#endif

#ifndef TRUE
#define TRUE                1
#endif

#define EXEC_FILE_EXT ".XSE"

#define MAX_THREAD_COUNT            1024 // 最大线程数目

Script g_Scripts [ MAX_THREAD_COUNT ];

int g_iCurrThread;								// 当前正在运行的线程索引


void Init() {
    // 给每个线程分配一个 script对象
    for ( int iCurrScriptIndex = 0; iCurrScriptIndex < MAX_THREAD_COUNT; ++ iCurrScriptIndex )
    {
        g_Scripts [ iCurrScriptIndex ].iIsActive = FALSE;

        g_Scripts [ iCurrScriptIndex ].iIsRunning = FALSE;
        g_Scripts [ iCurrScriptIndex ].iIsMainFuncPresent = FALSE;
        g_Scripts [ iCurrScriptIndex ].iIsPaused = FALSE;

        g_Scripts [ iCurrScriptIndex ].InstrStream.pInstrs = NULL;
        g_Scripts [ iCurrScriptIndex ].Stack.pElmnts = NULL;
        g_Scripts [ iCurrScriptIndex ].pFuncTable = NULL;
        g_Scripts [ iCurrScriptIndex ].HostAPICallTable.ppstrCalls = NULL;
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

    return 0;
}
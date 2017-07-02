#include <stdio.h>
#include <malloc.h>
#include <string.h>

#define EXEC_FILE_EXT ".XSE"

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

    return 0;
}
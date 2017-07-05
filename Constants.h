//
// Created by Leozki on 2017/7/5.
//

#ifndef XVM_CONSTANTS_H
#define XVM_CONSTANTS_H

#ifndef TRUE
#define TRUE                    1           // True
#endif

#ifndef FALSE
#define FALSE                   0           // False
#endif


#define EXEC_FILE_EXT                ".XSE"

#define XSE_ID_STRING               "XSE0"

#define LOAD_OK                        0
#define LOAD_ERROR_FILE_IO        1

#define LOAD_ERROR_INVALID_XSE        2
#define LOAD_ERROR_UNSUPPORTED_VERS    3
#define LOAD_ERROR_OUT_OF_MEMORY    4
#define LOAD_ERROR_OUT_OF_THREADS    5


#define MAX_THREAD_COUNT            1024 // 最大线程数目

#define DEF_STACK_SIZE                1024 // 默认栈大小


#define OP_TYPE_NULL                -1
#define OP_TYPE_INT                 0
#define OP_TYPE_FLOAT               1
#define OP_TYPE_STRING              2
#define OP_TYPE_ABS_STACK_INDEX     3
#define OP_TYPE_REL_STACK_INDEX     4
#define OP_TYPE_INSTR_INDEX         5
#define OP_TYPE_FUNC_INDEX          6
#define OP_TYPE_HOST_API_CALL_INDEX 7
#define OP_TYPE_REG                 8


#endif //XVM_CONSTANTS_H

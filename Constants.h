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

#define THREAD_TIMESLICE_DUR 20 // 每个线程的时间片,20毫秒

#define INSTR_MOV                   0

#define INSTR_ADD                   1
#define INSTR_SUB                   2
#define INSTR_MUL                   3
#define INSTR_DIV                   4
#define INSTR_MOD                   5
#define INSTR_EXP                   6
#define INSTR_NEG                   7
#define INSTR_INC                   8
#define INSTR_DEC                   9

#define INSTR_AND                   10
#define INSTR_OR                    11
#define INSTR_XOR                   12
#define INSTR_NOT                   13
#define INSTR_SHL                   14
#define INSTR_SHR                   15

#define INSTR_CONCAT                16
#define INSTR_GETCHAR               17
#define INSTR_SETCHAR               18

#define INSTR_JMP                   19
#define INSTR_JE                    20
#define INSTR_JNE                   21
#define INSTR_JG                    22
#define INSTR_JL                    23
#define INSTR_JGE                   24
#define INSTR_JLE                   25

#define INSTR_PUSH                  26
#define INSTR_POP                   27

#define INSTR_CALL                  28
#define INSTR_RET                   29
#define INSTR_CALLHOST              30

#define INSTR_PAUSE                 31
#define INSTR_EXIT                  32

#endif //XVM_CONSTANTS_H

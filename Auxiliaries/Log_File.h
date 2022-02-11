#ifndef LOG_FILE_H_INCLUDED
#define LOG_FILE_H_INCLUDED

#include <stdio.h>
#include <stdlib.h>

#define OPEN_LOG_FILE               \
do                                  \
{                                   \
    if (Open_Log_File () == ERROR)  \
        return EXIT_FAILURE;        \
}                                   \
while (0)

#define MY_ASSERT(condition, var, err_num, error)                               \
do                                                                              \
{                                                                               \
    if (!(condition))                                                           \
    {                                                                           \
        My_Assert (false, __FILE__, __LINE__, __FUNCTION__, var, err_num);      \
        return error;                                                           \
    }                                                                           \
}                                                                               \
while (0)

int My_Assert (bool condition, const char *const file, const int line,
               const char *const fun_name, const char *const var_name, const int err_name);
int  Open_Log_File  (void);
void Close_Log_File (void);

enum ERRORS
{
    NO_ERRORS = -101,
    ERROR,
};

enum ERROR_TYPES
{
    UNDEF_BEH = 0,
    FUNC_ERROR,

    LOG_NOT_OPEN,
    OPEN_ERR,
    CLOSE_ERR,
    EMP_FILE,
    INCORR_FILE,
    FILE_WRT,
    FSEEK_ERR,
    FTELL_ERR,
    STAT_ERR,

    POS_VAL,
    UNEXP_VAL,
    UNEXP_SYMB,
    INT_EXPECTED,
    NULL_PTR,
    NOT_NULL_PTR,
    NE_MEM,
    NULL_POW_NULL,
    DIV_BY_NULL,

    INCORR_LBL,
    EMP_LBL,
    EQUAL_LBLS,
    TF_LBLS,
    UNKNOWN_JMP,
    POS_LBL,
    END_LBL,

    UNDEF_CMD,
    INCORR_ARG,
    WRONG_DECIMAL,
    TF_ARGS,
    TM_ARGS,
    LINE_ERR,
    SCND_BRACE,
    TWO_BRACK,

    CTOR_ERR,
    PUSH_ERR,
    POP_ERR,
    DTOR_ERR,
    UNINIT_STACK,
    ZERO_POP,
    MULT_CTOR,
    BAD_RESIZE,
    RSZ_ERR,
    L_S_CANARY_CG,
    R_S_CANARY_CG,
    L_D_CANARY_CG,
    R_D_CANARY_CG,
    HASH_CHANGE,

    ZERO_DIV,

    END_I_ERR,
    BEG_I_ERR,
    LIST_UF,
    LIST_OF,
    INV_POS,
    INV_DEL,
    FREE_INSERT,
    FREE_DEL,
    BEFORE_ZERO,
    UNINIT_LIST,
    NO_NUM,
    PREV_HEAD,
    NEXT_TAIL,
    LIST_DAM,
    FREE_DAM,

    HAS_L_SON,
    HAS_R_SON
};

#endif // LOG_FILE_H_INCLUDED

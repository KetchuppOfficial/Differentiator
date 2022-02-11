#ifndef CANARY_PROTECTION_H_INCLUDED
#define CANARY_PROTECTION_H_INCLUDED

#define CHECK_CANARY(condition)                \
{                                              \
    if (Check_Canary ((condition)) == ERROR)   \
        return ERROR;                          \
}

typedef unsigned long canary_t;

const canary_t STACK_L_CANARY = 0xDEDBEDA;
const canary_t STACK_R_CANARY = 0xDEDBAD;
const canary_t DATA_L_CANARY  = 0xBADEDA;
const canary_t DATA_R_CANARY  = 0xBE3BAB;

const size_t CANARY_SZ = sizeof (canary_t);

#endif // CANARY_PROTECTION_H_INCLUDED

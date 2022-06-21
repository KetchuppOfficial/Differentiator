#ifndef STACK_H_INCLUDED
#define STACK_H_INCLUDED

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>

#define CHAR      0     // char
#define INT       1     // int
#define U_INT     2     // unsigned int
#define L_INT     3     // long int
#define U_L_INT   4     // unsigned long int
#define L_L_INT   5     // long long int
#define U_L_L_INT 6     // unsigned long long int
#define FLOAT     7     // float
#define DOUBLE    8     // double
#define L_DOUBLE  9     // long double
#define PTR       10    // void *

#define STACK_TYPE PTR

#if STACK_TYPE == CHAR
    #define ELEM_T char
    #define STACK_FMT "c"
#elif STACK_TYPE == INT
    #define ELEM_T int
    #define STACK_FMT "d"
#elif STACK_TYPE == U_INT
    #define ELEM_T unsigned int
    #define STACK_FMT "u"
#elif STACK_TYPE == L_INT
    #define ELEM_T long
    #define STACK_FMT "ld"
#elif STACK_TYPE == U_L_INT
    #define ELEM_T unsigned long
    #define STACK_FMT "lu"
#elif STACK_TYPE == L_L_INT
    #define ELEM_T long long
    #define STACK_FMT "lld"
#elif STACK_TYPE == U_L_L_INT
    #define ELEM_T unsigned long long
    #define STACK_FMT "llu"
#elif STACK_TYPE == FLOAT
    #define ELEM_T float
    #define STACK_FMT "g"
#elif STACK_TYPE == DOUBLE
    #define ELEM_T double
    #define STACK_FMT "g"
#elif STACK_TYPE == L_DOUBLE
    #define ELEM_T long double
    #define STACK_FMT "lg"
#elif STACK_TYPE == PTR
    #define ELEM_T void*
    #define STACK_FMT "p"
#endif

#define ELEM_SZ sizeof (ELEM_T)

#define SECURITY_LEVEL 0

struct Stack;

struct Stack *Stack_Ctor     (void);
int           Stack_Dtor     (struct Stack *stack_ptr);
int           Stack_Push     (struct Stack *stack_ptr, const ELEM_T value);
int           Stack_Pop      (struct Stack *stack_ptr, ELEM_T *value_ptr);
int           Stack_Dump     (struct Stack *stack_ptr, FILE *output);
long          Get_Stack_Size (const struct Stack *stack_ptr);
ELEM_T        Stack_Top_Elem (const struct Stack *stack_ptr);

#endif

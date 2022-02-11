/** \file
 * \brief Header file of stack
 *
 * The header contains:\n
 * 1) Including of standard headers.\n
 * 2) Block of macros that defines basic stack types of data.\n
 * 3) Block that is responsible for conditional compilation. You can choose a security level and your program will work on it.\n
 * 4) Declaration of stack constants and stack sructure.\n
 * 5) Prototypes of 4 stack functions that are allowed to use in main function.
 *
 * \details Stack works with 10 standard types: char, int, unsigned int, long int, unsigned long int,
 *          long long int, unsigned long long int, float, double, long double.
 * \details Stack works on 3 security levels.\n
 *          On level 0 there will be only error reports in the log file.\n
 *          On level 1 there also will be canaries before and after both stack structure and data array.\n
 *          On level 2 there also will be hash protection.\n
 */

#ifndef STACK_H_INCLUDED
#define STACK_H_INCLUDED

//-----------------------------------------------
//Including headers
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>
#include <typeinfo>
//-----------------------------------------------

//-----------------------------------------------
//Defining type
#define CHAR      0     ///< Adds "char" as valid type of stack
#define INT       1     ///< Adds "int" as valid type of stack
#define U_INT     2     ///< Adds "unsigned int" as valid type of stack
#define L_INT     3     ///< Adds "long int" as valid type of stack
#define U_L_INT   4     ///< Adds "unsigned long int" as valid type of stack
#define L_L_INT   5     ///< Adds "long long int" as valid type of stack
#define U_L_L_INT 6     ///< Adds "unsigned long long int" as valid type of stack
#define FLOAT     7     ///< Adds "float" as valid type of stack
#define DOUBLE    8     ///< Adds "double" as valid type of stack
#define L_DOUBLE  9     ///< Adds "long doubleS" as valid type of stack
#define PTR       10    ///< Adds "void *" as valid type of stack

#define STACK_TYPE PTR

#if STACK_TYPE == CHAR
    #define ELEM_T char
    #define STACK_FMT "c"
#endif

#if STACK_TYPE == INT
    #define ELEM_T int
    #define STACK_FMT "d"
#endif

#if STACK_TYPE == U_INT
    #define ELEM_T unsigned int
    #define STACK_FMT "u"
#endif

#if STACK_TYPE == L_INT
    #define ELEM_T long
    #define STACK_FMT "ld"
#endif

#if STACK_TYPE == U_L_INT
    #define ELEM_T unsigned long
    #define STACK_FMT "lu"
#endif

#if STACK_TYPE == L_L_INT
    #define ELEM_T long long
    #define STACK_FMT "lld"
#endif

#if STACK_TYPE == U_L_L_INT
    #define ELEM_T unsigned long long
    #define STACK_FMT "llu"
#endif

#if STACK_TYPE == FLOAT
    #define ELEM_T float
    #define STACK_FMT "g"
#endif

#if STACK_TYPE == DOUBLE
    #define ELEM_T double
    #define STACK_FMT "g"
#endif

#if STACK_TYPE == L_DOUBLE
    #define ELEM_T long double
    #define STACK_FMT "lg"
#endif

#if STACK_TYPE == PTR
    #define ELEM_T void*
    #define STACK_FMT "p"
#endif

#define DATA_OUTPUT(index, data_src, output) fprintf (output, "\tdata[%d] = %" STACK_FMT "\n", (index), (data_src))
//-----------------------------------------------

//-----------------------------------------------
//Defining security
#define SECURITY_LEVEL 2

#if SECURITY_LEVEL == 1 || SECURITY_LEVEL == 2
    #include "Canary_Protection.h"
#endif

#if SECURITY_LEVEL == 2
    #include "Hash_Protection.h"
#elif SECURITY_LEVEL == 0
    #define CANARY_SZ 0
#endif
//-----------------------------------------------

const long          MIN_CAPACITY = 8;               ///< Minimal capacity of stack
const long          MULTIPLIER   = 2;               ///< Coefficient that shows how many times the capacity will be increased/decreased after stack resizing
const size_t        ELEM_SZ      = sizeof (ELEM_T); ///< Size of one element of stack in bytes
const unsigned long STK_POISON       = 792647;          ///< Constant that becomes the value of each element of stack.data after deconstruction

struct Stack
{
    #if SECURITY_LEVEL == 1 || SECURITY_LEVEL == 2
    canary_t l_canary;
    #endif

    ELEM_T   *data;
    long     size;
    long     capacity;
    bool     initialized;

    #if SECURITY_LEVEL == 2
    hash_t   hash;
    #endif

    #if SECURITY_LEVEL == 1 || SECURITY_LEVEL == 2
    canary_t r_canary;
    #endif
};

int Stack_Ctor (struct Stack *stack_ptr);
int Stack_Dtor (struct Stack *stack_ptr);
int Stack_Push (struct Stack *stack_ptr, const ELEM_T value);
int Stack_Pop  (struct Stack *stack_ptr, ELEM_T *value_ptr);
int Stack_Dump (struct Stack *stack_ptr, FILE *output);

#endif // STACK_H_INCLUDED

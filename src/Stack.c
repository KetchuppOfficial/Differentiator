#include "../include/Stack.h"
#include "My_Lib.h"

#if SECURITY_LEVEL == 2
typedef unsigned long hash_t;
#endif

#if SECURITY_LEVEL == 1 || SECURITY_LEVEL == 2
typedef unsigned long canary_t;
#endif

extern FILE *LOG_FILE_;

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

#if SECURITY_LEVEL == 0
#define CANARY_SZ 0
#endif

#if SECURITY_LEVEL == 1 || SECURITY_LEVEL == 2
#define CHECK_CANARY(condition)                \
{                                              \
    if (Check_Canary ((condition)) == ERROR)   \
        return ERROR;                          \
}

static const canary_t STACK_L_CANARY = 0xDEDBEDA;
static const canary_t STACK_R_CANARY = 0xDEDBAD;
static const canary_t DATA_L_CANARY  = 0xBADEDA;
static const canary_t DATA_R_CANARY  = 0xBE3BAB;

static const size_t CANARY_SZ = sizeof (canary_t);

static int Check_Canary (struct Stack *stack_ptr)
{
    MY_ASSERT (stack_ptr,              "struct Stack *stack_ptr", NULL_PTR,  ERROR);
    MY_ASSERT (stack_ptr->initialized, "stack_ptr->initialized",  UNEXP_VAL, ERROR);

    if (stack_ptr->l_canary != STACK_L_CANARY)
    {
        Stack_Dump (stack_ptr, LOG_FILE_);
        MY_ASSERT (false, "L_STACK_CANARY", UNEXP_VAL, ERROR);
    }

    if (stack_ptr->r_canary != STACK_R_CANARY)
    {
        Stack_Dump (stack_ptr, LOG_FILE_);
        MY_ASSERT (false, "R_STACK_CANARY", UNEXP_VAL, ERROR);
    }

    if (*(canary_t *)((char *)(stack_ptr->data) - CANARY_SZ) != DATA_L_CANARY)
    {
        Stack_Dump (stack_ptr, LOG_FILE_);
        MY_ASSERT (false, "L_DATA_CANARY", UNEXP_VAL, ERROR);
    }

    if (*(canary_t *)((char *)(stack_ptr->data) + stack_ptr->capacity * ELEM_SZ) != DATA_R_CANARY)
    {
        Stack_Dump (stack_ptr, LOG_FILE_);
        MY_ASSERT (false, "R_DATA_CANARY", UNEXP_VAL, ERROR);
    }

    return NO_ERRORS;
}
#endif

#if SECURITY_LEVEL == 2

#define CHECK_HASH(condition)               \
{                                           \
    if (Check_Hash ((condition)) == ERROR)  \
        return ERROR;                       \
}

#define HASH_VAR(hash, var)                 \
{                                           \
    hash += (hash_t)(var);                  \
    hash += (hash << 10);                   \
    hash ^= (hash >> 6);                    \
}

static hash_t Calc_Hash (const struct Stack *stack_ptr)
{
    MY_ASSERT (stack_ptr,              "struct Stack *stack_ptr", NULL_PTR,  (hash_t)ERROR);
    MY_ASSERT (stack_ptr->initialized, "stack_ptr->initialized",  UNEXP_VAL, (hash_t)ERROR);

    hash_t hash = 0;

    for (int index = 0; index < stack_ptr->capacity; index++)
        HASH_VAR (hash, stack_ptr->data[index]);

    HASH_VAR (hash, stack_ptr->size);
    HASH_VAR (hash, stack_ptr->capacity);
    HASH_VAR (hash, *((char *)(stack_ptr->data)));
    HASH_VAR (hash, stack_ptr->initialized);

    hash += (hash << 3);
    hash ^= (hash >> 11);
    hash += (hash << 15);

    return hash;
}

static int Check_Hash (struct Stack *stack_ptr)
{
    MY_ASSERT (stack_ptr,              "struct Stack *stack_ptr", NULL_PTR,  ERROR);
    MY_ASSERT (stack_ptr->initialized, "stack_ptr->initialized",  UNEXP_VAL, ERROR);

    hash_t old_hash = stack_ptr->hash;

    hash_t new_hash = Calc_Hash (stack_ptr);

    if (old_hash != new_hash)
    {
        Stack_Dump (stack_ptr, LOG_FILE_);
        MY_ASSERT (false, "stack_ptr->hash", UNEXP_VAL, ERROR);
    }

    return NO_ERRORS;
}
#undef HASH_VAR
#endif

const long          MIN_CAPACITY = 8;
const long          MULTIPLIER   = 2;
const unsigned long STK_POISON   = 792647;

// =================================== Constructor =================================== //

static struct Stack *Start_Initialization (void)
{
    struct Stack *stack_ptr = (struct Stack *)calloc (1, sizeof (struct Stack));
    MY_ASSERT (stack_ptr, "struct Stack *stack_ptr", NE_MEM, NULL);

    #if SECURITY_LEVEL == 1 || SECURITY_LEVEL == 2
    stack_ptr->l_canary = STACK_L_CANARY;
    stack_ptr->r_canary = STACK_R_CANARY;
    #endif

    stack_ptr->data = NULL;

    stack_ptr->size = 0;

    stack_ptr->capacity = MIN_CAPACITY;

    #if SECURITY_LEVEL == 2
    stack_ptr->hash = 0;
    #endif

    return stack_ptr;
}

struct Stack *Stack_Ctor (void)
{
    struct Stack *stack_ptr = Start_Initialization ();

    char *raw_ptr = (char *)calloc (2 * CANARY_SZ + MIN_CAPACITY * ELEM_SZ, sizeof (char));
    MY_ASSERT (raw_ptr, "char *raw_ptr", NE_MEM, NULL);

    #if SECURITY_LEVEL == 1 || SECURITY_LEVEL == 2
    *((canary_t *)raw_ptr) = DATA_L_CANARY;
    *((canary_t *)(raw_ptr + CANARY_SZ + MIN_CAPACITY * ELEM_SZ)) = DATA_R_CANARY;
    #endif

    stack_ptr->data = (ELEM_T *)(raw_ptr + CANARY_SZ);

    stack_ptr->initialized = true;

    #if SECURITY_LEVEL == 2
    stack_ptr->hash = Calc_Hash (stack_ptr);
    #endif

    return stack_ptr;
}

// =================================== Destructor =================================== //

int Stack_Dtor (struct Stack *stack_ptr)
{
    MY_ASSERT (stack_ptr,              "struct Stack *stack_ptr", NULL_PTR,  ERROR);
    MY_ASSERT (stack_ptr->initialized, "stack_ptr->initialized",  UNEXP_VAL, ERROR);

    #if SECURITY_LEVEL == 2
    CHECK_HASH (stack_ptr);
    #endif

    #if SECURITY_LEVEL == 1 || SECURITY_LEVEL == 2
    stack_ptr->l_canary = STK_POISON;
    stack_ptr->r_canary = STK_POISON;
    *((canary_t *)((char *)(stack_ptr->data) - CANARY_SZ)) = STK_POISON;
    *((canary_t *)(stack_ptr->data + stack_ptr->capacity)) = STK_POISON;
    #endif

    for (int index = 0; index < stack_ptr->capacity; index++)
    {
        #if STACK_TYPE != PTR
        stack_ptr->data[index] = STK_POISON;
        #else
        stack_ptr->data[index] = NULL;
        #endif
    }

    free ((char *)(stack_ptr->data) - CANARY_SZ);

    stack_ptr->size = 0;
    stack_ptr->capacity = 0;

    stack_ptr->initialized = false;

    free (stack_ptr);

    return NO_ERRORS;
}

// =================================== Push =================================== //

static int Stack_Resize_Up (struct Stack *stack_ptr)
{
    MY_ASSERT (stack_ptr,              "struct Stack *stack_ptr", NULL_PTR,  ERROR);
    MY_ASSERT (stack_ptr->initialized, "stack_ptr->initialized",  UNEXP_VAL, ERROR);

    #if SECURITY_LEVEL == 2
    CHECK_HASH (stack_ptr);
    #endif

    #if SECURITY_LEVEL == 1 || SECURITY_LEVEL == 2
    CHECK_CANARY (stack_ptr);
    #endif

    char *raw_ptr = (char *)realloc ((char *)stack_ptr->data - CANARY_SZ, stack_ptr->capacity * ELEM_SZ * MULTIPLIER + 2 * CANARY_SZ);
    MY_ASSERT (raw_ptr, "void *raw_ptr", NE_MEM, ERROR);

    #if SECURITY_LEVEL == 1 || SECURITY_LEVEL == 2
    char *r_canary_ptr = raw_ptr + CANARY_SZ + stack_ptr->capacity * ELEM_SZ;

    for (size_t counter = 0; counter < CANARY_SZ + stack_ptr->capacity * ELEM_SZ * (MULTIPLIER - 1); counter++)
        *(r_canary_ptr + counter) = 0;

    *((canary_t *)(r_canary_ptr + stack_ptr->capacity * ELEM_SZ * (MULTIPLIER - 1))) = DATA_R_CANARY;
    #endif

    stack_ptr->data = (ELEM_T *)(raw_ptr + CANARY_SZ);

    stack_ptr->capacity *= MULTIPLIER;

    #if SECURITY_LEVEL == 2
    stack_ptr->hash = Calc_Hash (stack_ptr);
    #endif

    return NO_ERRORS;
}

int Stack_Push (struct Stack *stack_ptr, const ELEM_T value)
{
    MY_ASSERT (stack_ptr,              "struct Stack *stack_ptr", NULL_PTR,  ERROR);
    MY_ASSERT (stack_ptr->initialized, "stack_ptr->initialized",  UNEXP_VAL, ERROR);

    #if SECURITY_LEVEL == 2
    CHECK_HASH (stack_ptr);
    #endif

    #if SECURITY_LEVEL == 1 || SECURITY_LEVEL == 2
    CHECK_CANARY (stack_ptr);
    #endif

    if (stack_ptr->size >= stack_ptr->capacity)
        MY_ASSERT (Stack_Resize_Up (stack_ptr), "Stack_Resize_Up ()", FUNC_ERROR, ERROR);

    if (stack_ptr->size < stack_ptr->capacity)
        stack_ptr->data[stack_ptr->size++] = (ELEM_T)value;

    #if SECURITY_LEVEL == 2
    stack_ptr->hash = Calc_Hash (stack_ptr);
    #endif

    return NO_ERRORS;
}

// =================================== Pop =================================== //

static int Stack_Resize_Down (struct Stack *stack_ptr)
{
    MY_ASSERT (stack_ptr,                          "struct Stack *stack_ptr", NULL_PTR,  ERROR);
    MY_ASSERT (stack_ptr->initialized,             "stack_ptr->initialized",  UNEXP_VAL, ERROR);
    MY_ASSERT (stack_ptr->capacity > MIN_CAPACITY, "stack_ptr->capacity",     UNEXP_VAL, ERROR);

    #if SECURITY_LEVEL == 2
    CHECK_HASH (stack_ptr);
    #endif

    #if SECURITY_LEVEL == 1 || SECURITY_LEVEL == 2
    CHECK_CANARY (stack_ptr);
    #endif

    if (stack_ptr->capacity > MIN_CAPACITY)
    {
        stack_ptr->capacity /= MULTIPLIER;

        char *raw_ptr = (char *)realloc ((char *)stack_ptr->data - CANARY_SZ, stack_ptr->capacity * ELEM_SZ + 2 * CANARY_SZ);
        MY_ASSERT (raw_ptr, "char *raw_ptr", NE_MEM, ERROR);

        stack_ptr->data = (ELEM_T *)(raw_ptr + CANARY_SZ);

        #if SECURITY_LEVEL == 1 || SECURITY_LEVEL == 2
        *(canary_t *)(raw_ptr + CANARY_SZ + stack_ptr->capacity * ELEM_SZ) = DATA_R_CANARY;
        *(canary_t *)raw_ptr = DATA_L_CANARY;
        #endif
    }

    #if SECURITY_LEVEL == 2
    stack_ptr->hash = Calc_Hash (stack_ptr);
    #endif

    return NO_ERRORS;
}

int Stack_Pop (struct Stack *stack_ptr, ELEM_T *value_ptr)
{
    MY_ASSERT (stack_ptr,              "struct Stack *stack_ptr", NULL_PTR,  ERROR);
    MY_ASSERT (stack_ptr->initialized, "stack_ptr->initialized",  UNEXP_VAL, ERROR);
    MY_ASSERT (stack_ptr->size > 0,    "stack_ptr->size",         POS_VAL,   ERROR);

    #if SECURITY_LEVEL == 2
    CHECK_HASH (stack_ptr);
    #endif

    #if SECURITY_LEVEL == 1 || SECURITY_LEVEL == 2
    CHECK_CANARY (stack_ptr);
    #endif

    ELEM_T value = 0;

    value = stack_ptr->data[stack_ptr->size - 1];
    stack_ptr->data[stack_ptr->size - 1] = 0;
    stack_ptr->size--;

    if (stack_ptr->size < stack_ptr->capacity / MULTIPLIER - MIN_CAPACITY + 1)
        MY_ASSERT (Stack_Resize_Down (stack_ptr), "Stack_Resize_Down ()", FUNC_ERROR, ERROR);

    if (value_ptr)
        *value_ptr = value;

    #if SECURITY_LEVEL == 2
    stack_ptr->hash = Calc_Hash (stack_ptr);
    #endif

    return NO_ERRORS;
}

// =================================== Dump =================================== //

#define DATA_OUTPUT(index, data_src, output) fprintf (output, "\tdata[%d] = %" STACK_FMT "\n", (index), (data_src))

int Stack_Dump (struct Stack *stack_ptr, FILE *output)
{
    MY_ASSERT (stack_ptr, "struct Stack *stack_ptr", NULL_PTR,  ERROR);
    MY_ASSERT (output,    "FILE *output",            NULL_PTR,  ERROR);

    fprintf (output, "*************************************\n");
    fprintf (output, "ALL INFORMATION ABOUT STACK\n\n");
    fprintf (output, "POINTER ON STACK: %p\n", stack_ptr);

    #if SECURITY_LEVEL == 1 || SECURITY_LEVEL == 2
    fprintf (output, "LEFT STACK CANARY: %lX\n", stack_ptr->l_canary);
    #endif

    fprintf (output, "DATA:\n");
    #if SECURITY_LEVEL == 1 || SECURITY_LEVEL == 2
    fprintf (output, "LEFT DATA CANARY: %lX\n", *((canary_t *)((char *)stack_ptr->data - CANARY_SZ)));
    #endif
    for (int index = 0; index < (stack_ptr->capacity); index++)
        DATA_OUTPUT (index, (stack_ptr->data)[index], output);
    #if SECURITY_LEVEL == 1 || SECURITY_LEVEL == 2
    fprintf (output, "RIGHT DATA CANARY: %lX\n", *((canary_t *)((char*)stack_ptr->data + stack_ptr->capacity * ELEM_SZ)));
    #endif
    fprintf (output, "SIZE: %ld\n",     stack_ptr->size);
    fprintf (output, "CAPACITY: %ld\n", stack_ptr->capacity);

    #if SECURITY_LEVEL == 2
    fprintf (output, "SAVED HASH: %lu\n",   stack_ptr->hash);
    hash_t curr_hash = Calc_Hash (stack_ptr);
    fprintf (output, "CURRENT HASH: %lu\n", curr_hash);
    #endif

    #if SECURITY_LEVEL == 1 || SECURITY_LEVEL == 2
    fprintf (output, "RIGHT STACK CANARY: %lX\n", stack_ptr->r_canary);
    #endif
    fprintf (output, "*************************************\n\n");

    return NO_ERRORS;
}

long Get_Stack_Size (const struct Stack *stack_ptr)
{
    MY_ASSERT (stack_ptr, "const struct Stack *stack_ptr", NULL_PTR, (long)ERROR);
    
    return stack_ptr->size;
}

ELEM_T Stack_Top_Elem (const struct Stack *stack_ptr)
{
    return stack_ptr->data[stack_ptr->size - 1];
}

#undef DATA_OUTPUT

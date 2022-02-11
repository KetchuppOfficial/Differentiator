#include "Stack.h"
#include "../Log_File.h"

static int    Start_Initialization (struct Stack *stack_ptr);
static int    Stack_Resize_Up      (struct Stack *stack_ptr);
static int    Stack_Resize_Down    (struct Stack *stack_ptr);
#if SECURITY_LEVEL == 1 || SECURITY_LEVEL == 2
static int    Check_Canary         (struct Stack *stack_ptr);
#endif
#if SECURITY_LEVEL == 2
static int    Check_Hash           (struct Stack *stack_ptr);
static hash_t Calc_Hash            (struct Stack *stack_ptr);
#endif
extern FILE *LOG_FILE;

/** \brief Constructs stack
 *
 *  Calls Stack_Initialization ()
 *  Dynamically allocates memory for array of MIN_CAPACITY elements of type ELEM_T.
 *  If enough memory can not be allocated, the function returns ERROR and puts error report into log file
 *  If SECURITY_LEVEL == 1 or 2, the function also allocates memory for to canaries and puts them in the beginning and in the end of the array
 *  If SECURITY_LEVEL == 2, the function also calculates hash of the stack as the last action
 *
 * \param stack_ptr [out] Pointer on stack
 * \return If the work is completed successfully, returns NO_ERRORS; else returns ERROR.
 *
 * \warning If the pointer on the stack is NULL, function returns ERROR and puts error report into log file.
 * \warning If the stack is initialized, the function returns ERROR and puts error report into log file.
 *          It's forbidden to construct stack more than once.
 * \warning If you change the value of stack.initialized into false and try to call Stack_Ctor (),
 *          function returns ERROR and puts error report into log file.
 *          It's forbidden to construct the same stack again.
 */

int Stack_Ctor (struct Stack *stack_ptr)
{
    MY_ASSERT (stack_ptr,               "struct Stack *stack_ptr", NULL_PTR,  ERROR);
    MY_ASSERT (!stack_ptr->initialized, "stack_ptr->initialized",  MULT_CTOR, ERROR);

    Start_Initialization (stack_ptr);

    void *raw_ptr = calloc (2 * CANARY_SZ + MIN_CAPACITY * ELEM_SZ, sizeof (char));
    MY_ASSERT (raw_ptr, "void *raw_ptr", NE_MEM, ERROR);

    #if SECURITY_LEVEL == 1 || SECURITY_LEVEL == 2
    *((canary_t *)raw_ptr) = DATA_L_CANARY;
    *((canary_t *)((char *)raw_ptr + CANARY_SZ + MIN_CAPACITY * ELEM_SZ)) = DATA_R_CANARY;
    #endif

    stack_ptr->data = (ELEM_T *)((char *)raw_ptr + CANARY_SZ);

    stack_ptr->initialized = true;

    #if SECURITY_LEVEL == 2
    stack_ptr->hash = Calc_Hash (stack_ptr);
    #endif

    return NO_ERRORS;
}

/** \brief Initializes stack
 *
 *  Variables stack.data, stack.size, stack.capacity and stack.hash are initialized with 0.
 *  Variables stack.l_canary and stack.r_canary are initialized with their specific values.
 *
 * \param stack_ptr [out] Pointer on stack
 * \return If the work is completed successfully, returns NO_ERRORS; else returns ERROR.
 *
 * \warning This function is static. Consequently, you can not use it in other files.
 * \warning If the pointer on the stack is NULL, function returns ERROR and puts error report into log file.
 * \warning If the stack is initialized, function returns ERROR and puts error report into log file.
 *          It's forbidden to construct stack more than once.
 */

static int Start_Initialization (struct Stack *stack_ptr)
{
    MY_ASSERT (stack_ptr, "struct Stack *stack_ptr", NULL_PTR, ERROR);

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

    return NO_ERRORS;
}

/** \brief Destructs stack
 *
 * Changes value of each element of stack.data into POISON and calls free to deallocate memory.
 * Changes value of stack.size and stack.capacity into 0.
 * Changes value of stack.initialized into false.
 * If SECURITY_LEVEL == 1 or 2, the function also changes value of stack canaries into POISON.
 * If SECURITY_LEVEL == 2, the function also checks if the value of hash had been changed before the function was called.
 *
 * \param stack_ptr [out] Pointer on stack
 * \return If the work is completed successfully, returns NO_ERRORS; else returns ERROR.
 *
 * \warning If the pointer on the stack is NULL, function returns ERROR and puts error report into log file.
 * \warning If the stack is not initialized, function returns ERROR and puts error report into log file.
 *          It's forbidden to destruct stack that has not been constructed.
 */

int Stack_Dtor (struct Stack *stack_ptr)
{
    MY_ASSERT (stack_ptr,              "struct Stack *stack_ptr", NULL_PTR,     ERROR);
    MY_ASSERT (stack_ptr->initialized, "stack_ptr->initialized",  UNINIT_STACK, ERROR);

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

    return NO_ERRORS;
}

/** \brief Adds an element to stack
 *
 * Firstly, the function checks if there is enough space to contain the new element.
 * If there is not enough space, the function calls Stack_Resize_Up ().
 * If there is enough space, the function pushes the new element into stack.
 * If SECURITY_LEVEL == 1 or 2, the function also checks if the value of canaries had been changed before the function was called.
 * If SECURITY_LEVEL == 2, the function also checks if the value of hash had been changed before the function was called.
 * And it calculates hash of the stack as the last action.
 *
 * \param stack_ptr [out] Pointer on stack
 * \param value [in] Element that you add to stack
 * \return If the work is completed successfully, returns NO_ERRORS; else returns ERROR.
 *
 * \warning If the pointer on the stack is NULL, function returns ERROR and puts error report into log file.
 * \warning If the stack is not initialized, function returns ERROR and puts error report into log file.
 *          It's forbidden to destruct stack that has not been constructed.
 */
int Stack_Push (struct Stack *stack_ptr, const ELEM_T value)
{
    MY_ASSERT (stack_ptr,              "struct Stack *stack_ptr", NULL_PTR,     ERROR);
    MY_ASSERT (stack_ptr->initialized, "stack_ptr->initialized",  UNINIT_STACK, ERROR);

    #if SECURITY_LEVEL == 2
    CHECK_HASH (stack_ptr);
    #endif

    #if SECURITY_LEVEL == 1 || SECURITY_LEVEL == 2
    CHECK_CANARY (stack_ptr);
    #endif

    if (stack_ptr->size >= stack_ptr->capacity)
        MY_ASSERT (Stack_Resize_Up (stack_ptr), "Stack_Resize_Up ()", RSZ_ERR, ERROR);

    if (stack_ptr->size < stack_ptr->capacity)
        stack_ptr->data[stack_ptr->size++] = (ELEM_T)value;

    #if SECURITY_LEVEL == 2
    stack_ptr->hash = Calc_Hash (stack_ptr);
    #endif

    return NO_ERRORS;
}

/** \brief Increases the capacity of stack
 *
 * The function increases the stack capacity exponentially. The base can be changed.
 * If SECURITY_LEVEL == 1 or 2, the function also checks if the value of canaries had been changed before the function was called.
 * If SECURITY_LEVEL == 2, the function also checks if the value of hash had been changed before the function was called.
 * And it calculates hash of the stack as the last action.
 *
 * \param stack_ptr [out] Pointer on stack
 * \return If the work is completed successfully, returns NO_ERRORS; else returns ERROR.
 *
 * \warning This function is static. Consequently, you can not use it in other files.
 * \warning If the pointer on the stack is NULL, function returns ERROR and puts error report into log file.
 * \warning If the stack is not initialized, function returns ERROR and puts error report into log file.
 *          It's forbidden to destruct stack that has not been constructed.
 */
static int Stack_Resize_Up (struct Stack *stack_ptr)
{
    MY_ASSERT (stack_ptr,              "struct Stack *stack_ptr", NULL_PTR,     ERROR);
    MY_ASSERT (stack_ptr->initialized, "stack_ptr->initialized",  UNINIT_STACK, ERROR);

    #if SECURITY_LEVEL == 2
    CHECK_HASH (stack_ptr);
    #endif

    #if SECURITY_LEVEL == 1 || SECURITY_LEVEL == 2
    CHECK_CANARY (stack_ptr);
    #endif

    void *raw_ptr = realloc ((char *)(stack_ptr->data) - CANARY_SZ, stack_ptr->capacity * ELEM_SZ * MULTIPLIER + 2 * CANARY_SZ);
    MY_ASSERT (raw_ptr, "void *raw_ptr", NE_MEM, ERROR);

    char *r_canary_ptr = (char *)raw_ptr + CANARY_SZ + stack_ptr->capacity * ELEM_SZ;

    for (size_t counter = 0; counter < CANARY_SZ + stack_ptr->capacity * ELEM_SZ * (MULTIPLIER - 1); counter++)
        *(r_canary_ptr + counter) = 0;

    #if SECURITY_LEVEL == 1 || SECURITY_LEVEL == 2
    *((canary_t *)(r_canary_ptr + stack_ptr->capacity * ELEM_SZ * (MULTIPLIER - 1))) = DATA_R_CANARY;
    #endif

    (stack_ptr->data) = (ELEM_T *)((char *)raw_ptr + CANARY_SZ);

    (stack_ptr->capacity) *= MULTIPLIER;

    #if SECURITY_LEVEL == 2
    stack_ptr->hash = Calc_Hash (stack_ptr);
    #endif

    return NO_ERRORS;
}

/** \brief Removes the most recent added element of the stack that was not yet removed
 *
 * Firstly, the function removes an element from the stack.
 * If the stack capacity has the certain value, the function calls Stack_Resize_Down ().
 * If SECURITY_LEVEL == 1 or 2, the function also checks if the value of canaries had been changed before the function was called.
 * If SECURITY_LEVEL == 2, the function also checks if the value of hash had been changed before the function was called.
 * And it calculates hash of the stack as the last action.
 *
 * \param stack_ptr [out] Pointer on stack
 * \param elem_ptr [out] Pointer on element that you remove from stack
 * \return If the work is completed successfully, returns NO_ERRORS; else returns ERROR.
 *
 * \warning If the pointer on the stack is NULL, function returns ERROR and puts error report into log file.
 * \warning If the pointer on the variable to contain removed element is NULL, function returns ERROR and puts error report into log file.
 * \warning If the stack is not initialized, function returns ERROR and puts error report into log file.
 *          It's forbidden to destruct stack that has not been constructed.
 * \warning If the stack size is less than or equal to zero, function returns ERROR and puts error report into log file.
 *          It's forbidden to remove elements from the empty stack.
 */
int Stack_Pop (struct Stack *stack_ptr, ELEM_T *value_ptr)
{
    MY_ASSERT (stack_ptr,              "struct Stack *stack_ptr", NULL_PTR,      ERROR);
    MY_ASSERT (stack_ptr->initialized, "stack_ptr->initialized",  UNINIT_STACK,  ERROR);
    MY_ASSERT (stack_ptr->size > 0,    "stack_ptr->size",         ZERO_POP,      ERROR);

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
        MY_ASSERT (Stack_Resize_Down (stack_ptr), "Stack_Resize_Down ()", RSZ_ERR, ERROR);

    if (value_ptr)
        *value_ptr = value;

    #if SECURITY_LEVEL == 2
    stack_ptr->hash = Calc_Hash (stack_ptr);
    #endif

    return NO_ERRORS;
}

/** \brief Decreases the capacity of stack
 *
 * The function decreases the stack capacity exponentially. The base can be changed.
 * If the stack capacity is less than or equal to MIN_CAPACITY, the function will not decrease the capacity.
 * If SECURITY_LEVEL == 1 or 2, the function also checks if the value of canaries had been changed before the function was called.
 * If SECURITY_LEVEL == 2, the function also checks if the value of hash had been changed before the function was called.
 * And it calculates hash of the stack as the last action.
 *
 * \param stack_ptr [out] Pointer on stack
 * \return If the work is completed successfully, returns NO_ERRORS; else returns ERROR.
 *
 * \warning This function is static. Consequently, you can not use it in other files.
 * \warning If the pointer on the stack is NULL, function returns ERROR and puts error report into log file.
 * \warning If the stack is not initialized, function returns ERROR and puts error report into log file.
 *          It's forbidden to destruct stack that has not been constructed.
 */
static int Stack_Resize_Down (struct Stack *stack_ptr)
{
    MY_ASSERT (stack_ptr,                          "struct Stack *stack_ptr", NULL_PTR,      ERROR);
    MY_ASSERT (stack_ptr->initialized,             "stack_ptr->initialized",  UNINIT_STACK,  ERROR);
    MY_ASSERT (stack_ptr->capacity > MIN_CAPACITY, "stack_ptr->capacity",     BAD_RESIZE,    ERROR);

    #if SECURITY_LEVEL == 2
    CHECK_HASH (stack_ptr);
    #endif

    #if SECURITY_LEVEL == 1 || SECURITY_LEVEL == 2
    CHECK_CANARY (stack_ptr);
    #endif

    if (stack_ptr->capacity > MIN_CAPACITY)
    {
        stack_ptr->capacity /= MULTIPLIER;

        void *raw_ptr = realloc ((char *)(stack_ptr->data) - CANARY_SZ, stack_ptr->capacity * ELEM_SZ + 2 * CANARY_SZ);
        MY_ASSERT (raw_ptr, "void *raw_ptr", NE_MEM, ERROR);

        #if SECURITY_LEVEL == 1 || SECURITY_LEVEL == 2
        *((canary_t *)((char *)raw_ptr + CANARY_SZ + stack_ptr->capacity * ELEM_SZ)) = DATA_R_CANARY;
        #endif
    }

    #if SECURITY_LEVEL == 2
    stack_ptr->hash = Calc_Hash (stack_ptr);

    #endif

    return NO_ERRORS;
}

/** \brief Prints the most important information about stack
 *
 * The function prints the pointer on the stack, the type of containing data, the value of size and capacity.
 * If SECURITY_LEVEL == 1 or 2, the function also prints the value of stack canaries.
 * If SECURITY_LEVEL == 2, the function also prints the value of hash that was saved in stack.
 * Than the function calls Calc_Hash to recalculate hash and print it as well.
 *
 * \param stack_ptr [out] Pointer on stack
 * \param output [out] pointer on a file to print information into
 * \return If the work is completed successfully, returns NO_ERRORS; else returns ERROR.
 *
 * \warning This function is static. Consequently, you can not use it in other files.
 * \warning If the pointer on the stack is NULL, function returns ERROR and puts error report into log file.
 * \warning If the pointer on the file is NULL, function returns ERROR and puts error report into log file.
 */
int Stack_Dump (struct Stack *stack_ptr, FILE *output)
{
    MY_ASSERT (stack_ptr, "struct Stack *stack_ptr", NULL_PTR,  ERROR);
    MY_ASSERT (output,    "FILE *output",            NULL_PTR,  ERROR);

    fprintf (output, "*************************************\n");
    fprintf (output, "ALL INFORMATION ABOUT STACK\n\n");
    fprintf (output, "POINTER ON STACK: %p\n", stack_ptr);

    fprintf (output, "TYPE OF DATA: %s\n", typeid(stack_ptr->data[0]).name());
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
    fprintf (output, "RIGHT DATA CANARY: %lX\n", *((canary_t *)((char*)stack_ptr->data + stack_ptr->capacity)));
    #endif
    fprintf (output, "SIZE: %ld\n",     stack_ptr->size);
    fprintf (output, "CAPACITY: %ld\n", stack_ptr->capacity);

    #if SECURITY_LEVEL == 2
    fprintf (output, "SAVED HASH: %lld\n",   stack_ptr->hash);
    hash_t curr_hash = Calc_Hash (stack_ptr);
    fprintf (output, "CURRENT HASH: %lld\n", curr_hash);
    #endif

    #if SECURITY_LEVEL == 1 || SECURITY_LEVEL == 2
    fprintf (output, "RIGHT STACK CANARY: %lX\n", stack_ptr->r_canary);
    #endif
    fprintf (output, "*************************************\n\n");

    return NO_ERRORS;
}
#if SECURITY_LEVEL == 1 || SECURITY_LEVEL == 2
static int Check_Canary (struct Stack *stack_ptr)
{
    MY_ASSERT (stack_ptr,              "struct Stack *stack_ptr", NULL_PTR,      ERROR);
    MY_ASSERT (stack_ptr->initialized, "stack_ptr->initialized",  UNINIT_STACK,  ERROR);

    if (stack_ptr->l_canary != STACK_L_CANARY)
    {
        Stack_Dump (stack_ptr, LOG_FILE);
        MY_ASSERT (false, "l_STACK_CANARY", L_S_CANARY_CG, ERROR);
    }

    if (stack_ptr->r_canary != STACK_R_CANARY)
    {
        Stack_Dump (stack_ptr, LOG_FILE);
        MY_ASSERT (false, "R_STACK_CANARY", R_S_CANARY_CG, ERROR);
    }

    if (*((canary_t *)((char *)(stack_ptr->data) - CANARY_SZ)) != DATA_L_CANARY)
    {
        Stack_Dump (stack_ptr, LOG_FILE);
        MY_ASSERT (false, "L_DATA_CANARY",  L_D_CANARY_CG, ERROR);
    }

    if (*((canary_t *)((char *)(stack_ptr->data) + (stack_ptr->capacity) * ELEM_SZ)) != DATA_R_CANARY)
    {
        Stack_Dump (stack_ptr, LOG_FILE);
        MY_ASSERT (false, "R_DATA_CANARY",  R_D_CANARY_CG, ERROR);
    }

    return NO_ERRORS;
}
#endif

#if SECURITY_LEVEL == 2
static int Check_Hash (struct Stack *stack_ptr)
{
    MY_ASSERT (stack_ptr,              "struct Stack *stack_ptr", NULL_PTR,     ERROR);
    MY_ASSERT (stack_ptr->initialized, "stack_ptr->initialized",  UNINIT_STACK, ERROR);

    hash_t old_hash = stack_ptr->hash;

    hash_t new_hash = Calc_Hash (stack_ptr);

    if (old_hash != new_hash)
    {
        Stack_Dump (stack_ptr, LOG_FILE);
        MY_ASSERT (false, "stack_ptr->hash", HASH_CHANGE, ERROR);
    }

    return NO_ERRORS;
}

static hash_t Calc_Hash (struct Stack *stack_ptr)
{
    MY_ASSERT (stack_ptr,              "struct Stack *stack_ptr", NULL_PTR,     (hash_t)ERROR);
    MY_ASSERT (stack_ptr->initialized, "stack_ptr->initialized",  UNINIT_STACK, (hash_t)ERROR);

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
#undef HASH_VAR
#endif

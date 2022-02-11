#include "Log_File.h"

FILE *LOG_FILE = NULL;

static const char *PREAMBLE = "This is log file of the program \"Differentiator\"\n"
                              "This file will help you to find and fix all the errors.\n\n";

static const char *ERROR_DESCRIPTIONS[] =
{
    "Undefined behavior",
    "Error in called function",

    "Impossible to open log file",
    "Impossible to open file",
    "Impossible to close file",
    "Empty file",
    "Incorrect file",
    "The number of written symbols differs from the expected one",
    "Error in work of function fseek ()",
    "Error in work of function ftell ()",
    "Error in work of function _stat()",

    "Expected positive value",
    "Unexpected value",
    "Unexpected symbol",
    "Expected integer number",
    "Pointer is NULL",
    "Pointer is not NULL",
    "Impossible to allocate enough memory",
    "0^0 is undefined",
    "Dividing by 0 is undefined",

    "Incorrect label",
    "Empty label",
    "Equal labels have been found",
    "The number of jumps cannot be greater than the number of labels",
    "There is no label for a jump",
    "Label should be a positive integer number",
    "Label cannot be the last instruction",

    "Undefined command",
    "Incorrect argument",
    "No digits after decimal point",
    "Too few arguments",
    "Too many arguments",
    "Incomplete line",
    "Second brace expected",
    "Two brackets in a row",

    "Constructing error",
    "Pushing error",
    "Popping error",
    "Destructing error",
    "It's forbidden to use uninitialized stack",
    "It's forbidden to pop stack which size is 0",
    "It's forbidden to initialize stack more than once",
    "It's forbidden to resize down stack which capacity is minimal or less than minimal",
    "Resizing error",
    "Left stack canary changed its value",
    "Right stack canary changed its value",
    "Left data canary changed its value",
    "Right data canary changed its value",
    "Hash changed its value unexpectedly",

    "It's forbidden to divide by zero",

    "The next pointer of the last node should be marked",
    "The prev pointer of the first node should be marked",
    "List underflow",
    "List overflow",
    "It's forbidden to add element after position that is not between 0 and current size",
    "It's forbidden to delete element after position that is not between 0 and current size",
    "It's forbidden to insert element after free node",
    "It's forbidden to delete element after free node",
    "It's forbidden to add elements before something in empty list",
    "It's forbidden to use uninitialized list",
    "There is no such logical number in list",
    "The \"prev\" of head has to be equal to 0",
    "The \"next\" of tail has to be equal to 0",
    "The structure of list has been damaged. The order of elements is incorrect",
    "The structure of list of free nodes is damaged",

    "This node already has left son",
    "This node already has right son"
};

int Open_Log_File (void)
{
    LOG_FILE = fopen ("log_file.log", "wb");

    if (!LOG_FILE)
    {
        LOG_FILE = stderr;
        MY_ASSERT (false, "LOG_FILE", LOG_NOT_OPEN, LOG_NOT_OPEN);
    }

    fprintf (LOG_FILE, "%s", PREAMBLE);

    atexit (Close_Log_File);

    return NO_ERRORS;
}

void Close_Log_File (void)
{
    if (fclose (LOG_FILE) == 0)
        printf ("Log file is closed successfully\n");
    else
        printf ("Log file is NOT closed successfully\n");
}

int My_Assert (bool condition, const char *const file, const int line,
               const char *const fun_name, const char *const var_name, const int err_name)
{
    if (!LOG_FILE || !fun_name || !var_name || err_name < 0 || line <= 0)
        return ERROR;

    if (!condition)
    {
        fprintf (LOG_FILE, "File: %s\n",                                   file);
        fprintf (LOG_FILE, "Line: %d\n",                                   line);
        fprintf (LOG_FILE, "Function with error: %s\n",                    fun_name);
        fprintf (LOG_FILE, "Variable or function that caused error: %s\n", var_name);
        fprintf (LOG_FILE, "Error description: %s\n\n",                    ERROR_DESCRIPTIONS[err_name]);

        return 1;
    }

    return 0;
}

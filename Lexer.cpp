#include "Differentiator.h"
#include "Functions.h"
#include "Reading_File.h"
#include "Log_File.h"

#define DEBUG 0
#define EVERY_SYMB_DUMB 0
#define EXPRESSION 0

#define PRINT(beg_end, function_name)                                       \
do                                                                          \
{                                                                           \
    printf ("%s of %s arg->str[arg->symb_i] = ", beg_end, function_name);   \
    if (arg->str[arg->symb_i] == '\0')                                      \
        printf ("NULL\n");                                                  \
    else                                                                    \
        printf ("%c\n", arg->str[arg->symb_i]);                             \
}                                                                           \
while (0)                                                                   

struct Token *Read_Function (const char *file_name, int *n_tokens)
{
    FILE *tree_file = Open_File (file_name, "rb");
    MY_ASSERT (tree_file, "Open_File ()", FUNC_ERROR, NULL);

    long n_symbs = Define_File_Size (tree_file);
    MY_ASSERT (n_symbs != ERROR, "Define_File_Size ()", FUNC_ERROR, NULL);

    char *buffer = Make_Buffer (tree_file, n_symbs);
    MY_ASSERT (buffer, "Make_Buffer ()", FUNC_ERROR, NULL);

    struct Token *token_arr = (struct Token *)calloc (n_symbs, sizeof (struct Token));
    MY_ASSERT (token_arr, "struct Token *token_arr", NE_MEM, NULL);

    *n_tokens = Lexer (buffer, n_symbs, token_arr);
    if (*n_tokens == ERROR)
        MY_ASSERT (false, "Lexer ()", FUNC_ERROR, NULL);

    #if EVERY_SYMB_DUMB == 1
    for (int i = 0; 1 <= token_arr[i].type && token_arr[i].type <= 6; i++)
    {
        switch (token_arr[i].type)
        {
            case NUMBER:      printf ("number: %f\n",      token_arr[i].value.number);  break;
            case MATH_SIGN:   printf ("math sign: %c\n",   token_arr[i].value.symbol);  break;
            case MATH_CONST:
                if (strcmp (token_arr[i].value.math_const, "e") == 0)
                    printf ("math const: e\n");
                else
                    printf ("math const: pi\n");
                break;
            case VARIABLE:    printf ("variable: %c\n",    token_arr[i].value.symbol);   break;
            case FUNCTION:    printf ("function: %s\n",    token_arr[i].value.function); break;
            case PARANTHESIS: printf ("paranthesis: %c\n", token_arr[i].value.symbol);   break;
            default: printf ("Some shit happened\n");
        }
    }
    #endif

    #if EXPRESSION == 1
    for (int i = 0; 1 <= token_arr[i].type && token_arr[i].type <= 6; i++)
    {
        switch (token_arr[i].type)
        {
            case NUMBER:      printf ("%.2f", token_arr[i].value.number);  break;
            case MATH_SIGN:   printf ("%c",   token_arr[i].value.symbol);  break;
            case MATH_CONST:
                if (strcmp (token_arr[i].value.math_const, "e") == 0)
                    printf ("e");
                else
                    printf ("pi");
                break;
            case VARIABLE:    printf ("%c", token_arr[i].value.symbol);    break;
            case FUNCTION:    printf ("%s", token_arr[i].value.function);  break;
            case PARANTHESIS: printf ("%c", token_arr[i].value.symbol);    break;
            default: printf ("Some shit happened\n");
        }
    }
    printf ("\n");
    #endif

    if (Close_File (tree_file, file_name) == ERROR)
        MY_ASSERT (false, "Close_File ()", FUNC_ERROR, NULL);

    free (buffer);

    return token_arr;
}

int Lexer (const char *str, const long n_symbs, struct Token *token_arr)
{
    MY_ASSERT (str,         "const char *str",         NULL_PTR, ERROR);
    MY_ASSERT (token_arr,   "struct Token *token_arr", NULL_PTR, ERROR);
    MY_ASSERT (n_symbs > 0, "const long n_symbs",      POS_VAL,  ERROR);

    struct Argument arg = {str, 0L, token_arr, 0L};

    if (Get_General (&arg) == ERROR)
        MY_ASSERT (false, "Get_General ()", FUNC_ERROR, ERROR);

    int n_tokens = arg.token_i;

    for (int i = 0; i < n_tokens - 1; i++)
    {
        if (token_arr[i].type == MATH_SIGN && token_arr[i + 1].type == MATH_SIGN)
            return ERROR;

        if (token_arr[i].type == FUNCTION && token_arr[i + 1].type != PARANTHESIS)
            return ERROR;
    }

    return n_tokens;
}

#define Require(symb)                                                   \
do                                                                      \
{                                                                       \
    if (arg->str[arg->symb_i] == symb)                                  \
        arg->symb_i++;                                                  \
    else                                                                \
        MY_ASSERT (false, "Buffer with function", UNEXP_SYMB, ERROR);   \
}                                                                       \
while (0)

#define Add_Token(tp, value_t, val)                         \
{                                                           \
    arg->token_arr[arg->token_i].type = tp;                 \
    arg->token_arr[arg->token_i].value.value_t = val;       \
    arg->token_i++;                                         \
}

int Get_General (struct Argument *arg)
{
    MY_ASSERT (arg, "struct Argument *arg", NULL_PTR, ERROR);

    #if DEBUG == 1
    PRINT ("beginning", "Get_Plus_Minus ");
    #endif

    if (Get_Plus_Minus (arg) == ERROR)
        MY_ASSERT (false, "Get_Plus_Minus ()", FUNC_ERROR, ERROR);

    #if DEBUG == 1
    PRINT ("ending   ", "Get_Plus_Minus ");
    #endif

    Require ('\0');

    return NO_ERRORS;
}

int Get_Plus_Minus (struct Argument *arg)
{
    MY_ASSERT (arg, "struct Argument *arg", NULL_PTR, ERROR);

    #if DEBUG == 1
    PRINT ("beginning", "Get_Mult_Div   ");
    #endif

    if (Get_Mult_Div (arg) == ERROR)
        MY_ASSERT (false, "Get_Mult_Div ()", FUNC_ERROR, ERROR);

    #if DEBUG == 1
    PRINT ("ending   ", "Get_Mult_Div   ");
    #endif

    while (arg->str[arg->symb_i] == '+' || arg->str[arg->symb_i] == '-')
    {
        Add_Token (MATH_SIGN, symbol, arg->str[arg->symb_i]);
        #if DEBUG == 1
            printf ("Token added\n");
        #endif
        arg->symb_i++;
        if (Skip_Spaces (arg) == ERROR)
            MY_ASSERT (false, "Skip_Spaces ()", FUNC_ERROR, ERROR);

        #if DEBUG == 1
        PRINT ("beginning", "Get_Mult_Div   ");
        #endif

        if (Get_Mult_Div (arg) == ERROR)
            MY_ASSERT (false, "Get_Mult_Div ()", FUNC_ERROR, ERROR);

        #if DEBUG == 1
        PRINT ("ending   ", "Get_Mult_Div   ");
        #endif
    }

    return NO_ERRORS;
}

int Get_Mult_Div (struct Argument *arg)
{
    MY_ASSERT (arg, "struct Argument *arg", NULL_PTR, ERROR);

    #if DEBUG == 1
    PRINT ("beginning", "Get_Pow        ");
    #endif

    if (Get_Pow (arg) == ERROR)
        MY_ASSERT (false, "Get_Pow ()", FUNC_ERROR, ERROR);

    #if DEBUG == 1
    PRINT ("ending   ", "Get_Pow        ");
    #endif

    while (arg->str[arg->symb_i] == '*' || arg->str[arg->symb_i] == '/')
    {
        Add_Token (MATH_SIGN, symbol, arg->str[arg->symb_i]);
        #if DEBUG == 1
            printf ("Token added\n");
        #endif

        arg->symb_i++;
        if (Skip_Spaces (arg) == ERROR)
            MY_ASSERT (false, "Skip_Spaces ()", FUNC_ERROR, ERROR);

        #if DEBUG == 1
        PRINT ("beginning", "Get_Pow        ");
        #endif

        if (Get_Pow (arg) == ERROR)
            MY_ASSERT (false, "Get_Pow ()", FUNC_ERROR, ERROR);

        #if DEBUG == 1
        PRINT ("ending   ", "Get_Pow        ");
        #endif
    }

    return NO_ERRORS;
}

int Get_Pow (struct Argument *arg)
{
    MY_ASSERT (arg, "struct Argument *arg", NULL_PTR, ERROR);

    #if DEBUG == 1
    PRINT ("beginning", "Get_Parenthesis");
    #endif

    if (Get_Parenthesis (arg) == ERROR)
        MY_ASSERT (false, "Get_Parenthesis ()", FUNC_ERROR, ERROR);

    #if DEBUG == 1
    PRINT ("ending   ", "Get_Parenthesis");
    #endif

    while (arg->str[arg->symb_i] == '^')
    {
        Add_Token (MATH_SIGN, symbol, '^');
        #if DEBUG == 1
            printf ("Token added\n");
        #endif

        arg->symb_i++;
        if (Skip_Spaces (arg) == ERROR)
            MY_ASSERT (false, "Skip_Spaces ()", FUNC_ERROR, ERROR);

        #if DEBUG == 1
        PRINT ("beginning", "Get_Parenthesis");
        #endif

        if (Get_Parenthesis (arg) == ERROR)
            MY_ASSERT (false, "Get_Parenthesis ()", FUNC_ERROR, ERROR);

        #if DEBUG == 1
        PRINT ("ending   ", "Get_Parenthesis");
        #endif
    }

    return NO_ERRORS;
}

int Get_Parenthesis (struct Argument *arg)
{
    MY_ASSERT (arg, "struct Argument *arg", NULL_PTR, ERROR);

    if (arg->str[arg->symb_i] == '(')
    {
        Add_Token (PARANTHESIS, symbol, '(');
        #if DEBUG == 1
            printf ("Token added\n");
        #endif

        arg->symb_i++;
        if (Skip_Spaces (arg) == ERROR)
            MY_ASSERT (false, "Skip_Spaces ()", FUNC_ERROR, ERROR);

        #if DEBUG == 1
        PRINT ("beginning", "Get_Plus_Minus ");
        #endif

        if (Get_Plus_Minus (arg) == ERROR)
            MY_ASSERT (false, "Get_Plus_Minus ()", FUNC_ERROR, ERROR);

        #if DEBUG == 1
        PRINT ("ending   ", "Get_Plus_Minus ");
        #endif

        Require (')');

        Add_Token (PARANTHESIS, symbol, ')')
        #if DEBUG == 1
            printf ("Token added\n");
        #endif

        if (Skip_Spaces (arg) == ERROR)
            MY_ASSERT (false, "Skip_Spaces ()", FUNC_ERROR, ERROR);

        return NO_ERRORS;
    }
    else if (isdigit (arg->str[arg->symb_i]))
    {
        #if DEBUG == 1
        PRINT ("beginning", "Get_Number     ");
        #endif

        if (Get_Number (arg) == ERROR)
            MY_ASSERT (false, "Get_Number ()", FUNC_ERROR, ERROR);

        #if DEBUG == 1
        PRINT ("ending   ", "Get_Number     ");
        #endif
    }
    else if (isalpha (arg->str[arg->symb_i]))
    {
        if (isalpha (arg->str[arg->symb_i + 1]))
        {
            #if DEBUG == 1
            PRINT ("beginning", "Get_Function   ");
            #endif

            if (Get_Function (arg) == ERROR)
                MY_ASSERT (false, "Get_Function ()", FUNC_ERROR, ERROR);

            #if DEBUG == 1
            PRINT ("ending   ", "Get_Function   ");
            #endif

            #if DEBUG == 1
            PRINT ("beginning", "Get_Parenthesis");
            #endif

            if (Get_Parenthesis (arg) == ERROR)
                MY_ASSERT (false, "Get_Parenthesis ()", FUNC_ERROR, ERROR);

            #if DEBUG == 1
            PRINT ("ending   ", "Get_Parenthesis");
            #endif
        }
        else
        {
            #if DEBUG == 1
            PRINT ("beginning", "Get_Var        ");
            #endif

            if (Get_Var (arg) == ERROR)
                MY_ASSERT (false, "Get_Var ()", FUNC_ERROR, ERROR);

            #if DEBUG == 1
            PRINT ("ending   ", "Get_Var        ");
            #endif
        }
    }

    return NO_ERRORS;
}

int Get_Number (struct Argument *arg)
{
    MY_ASSERT (arg, "struct Argument *arg", NULL_PTR, ERROR);

    double num = 0.0;

    while ('0' <= arg->str[arg->symb_i] && arg->str[arg->symb_i] <= '9')
    {
        num = num * 10 + (arg->str[arg->symb_i] - '0');
        arg->symb_i++;
    }

    if (arg->str[arg->symb_i] == '.')
    {
        arg->symb_i++;
        const char *old_str = arg->str + arg->symb_i;

        int degree = -1;

        while ('0' <= arg->str[arg->symb_i] && arg->str[arg->symb_i] <= '9')
        {
            num += (arg->str[arg->symb_i] - '0') * pow (10, degree);
            degree--;
            arg->symb_i++;
        }

        if (old_str == arg->str + arg->symb_i)
            return ERROR;
    }

    Add_Token (NUMBER, number, num)
    #if DEBUG == 1
        printf ("Token added\n");
    #endif

    if (Skip_Spaces (arg) == ERROR)
        MY_ASSERT (false, "Skip_Spaces ()", FUNC_ERROR, ERROR);

    return NO_ERRORS;
}

int Get_Var (struct Argument *arg)
{
    MY_ASSERT (arg, "struct Argument *arg", NULL_PTR, ERROR);

    if (arg->str[arg->symb_i] == 'e')
    {
        arg->token_arr[arg->token_i].type = MATH_CONST;
        memmove (arg->token_arr[arg->token_i].value.math_const, "e", 2 * sizeof (char));
        arg->token_i++;
    }
    else
        Add_Token (VARIABLE, symbol, arg->str[arg->symb_i])

    #if DEBUG == 1
        printf ("Token added\n");
    #endif

    arg->symb_i++;

    if (Skip_Spaces (arg) == ERROR)
        MY_ASSERT (false, "Skip_Spaces ()", FUNC_ERROR, ERROR);

    return NO_ERRORS;
}

int Get_Function (struct Argument *arg)
{
    MY_ASSERT (arg, "struct Argument *arg", NULL_PTR, ERROR);

    char *name_arr = (char *)calloc (MAX_F_SIZE, sizeof (char));
    MY_ASSERT (name_arr, "char *name_arr", NE_MEM, ERROR);

    for (int i = 0; isalpha (arg->str[arg->symb_i]); i++)
    {
        name_arr[i] = arg->str[arg->symb_i];
        arg->symb_i++;
    }

    if (Check_Function (name_arr) == ERROR)
    {
        if (strcmp (name_arr, "pi") == 0)
        {
            arg->token_arr[arg->token_i].type = MATH_CONST;
            memmove (arg->token_arr[arg->token_i].value.math_const, "pi", 3 * sizeof (char));
            arg->token_i++;
        }
        else
            MY_ASSERT (false, "Check_Function ()", FUNC_ERROR, ERROR);
    }
    else
    {
        arg->token_arr[arg->token_i].type = FUNCTION;
        memmove (arg->token_arr[arg->token_i].value.function, name_arr, MAX_F_SIZE);
        arg->token_i++;
    }

    #if DEBUG == 1
        printf ("Token added\n");
    #endif

    if (Skip_Spaces (arg) == ERROR)
        MY_ASSERT (false, "Skip_Spaces ()", FUNC_ERROR, ERROR);

    free (name_arr);

    return NO_ERRORS;
}

int Check_Function (const char *func_name)
{
    MY_ASSERT (func_name, "const char *func_name", NULL_PTR, ERROR);

    int found = 0;

    for (int i = 0; i < N_FUNCTIONS; i++)
    {
        if (strcmp (Functions_Data_Base[i], func_name) == 0)
            found++;
    }

    if (found == 0)
        return ERROR;

    return NO_ERRORS;
}

int Skip_Spaces (struct Argument *arg)
{
    MY_ASSERT (arg, "struct Argument *arg", NULL_PTR, ERROR);

    while (arg->str[arg->symb_i] != '\n' && arg->str[arg->symb_i] != '\r' && isspace (arg->str[arg->symb_i]))
        arg->symb_i++;

    if (arg->str[arg->symb_i] == '\n' || arg->str[arg->symb_i] == '\r')
        return ERROR;

    return NO_ERRORS;
}

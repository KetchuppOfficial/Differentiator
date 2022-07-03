#include "../include/Differentiator.h"
#include "My_Lib.h"

// ==================================== STATIC FUNCTIONS ==================================== //

static int  Lexer_         (const char *str, const long n_symbs, struct Token *token_arr);
static int  Get_Number     (const char *str, int *symb_i, double *num_ptr);
static void Show_Error     (const char *str, const int symb_i);
static int  Check_Function (const char *func_name);

#ifdef LEXER_DUMP
static int Lexer_Dump (const struct Token *token_arr, const int n_tokens);
#endif

// ========================================================================================== //

struct Token *Lexer (const char *buffer, const long n_symbs, int *n_tokens)
{
    MY_ASSERT (buffer,   "const char *buffer", NULL_PTR, NULL);
    MY_ASSERT (n_tokens, "int *n_tokens",      NULL_PTR, NULL);

    struct Token *token_arr = (struct Token *)calloc (n_symbs, sizeof (struct Token));
    MY_ASSERT (token_arr, "struct Token *token_arr", NE_MEM, NULL);

    *n_tokens = Lexer_ (buffer, n_symbs, token_arr);
    if (*n_tokens == ERROR)
        MY_ASSERT (false, "Lexer_ ()", FUNC_ERROR, NULL);

    free (buffer);

    #ifdef LEXER_DUMP
    Lexer_Dump (token_arr, *n_tokens);
    #endif

    return token_arr;
}

static int Lexer_ (const char *str, const long n_symbs, struct Token *token_arr)
{
    char name_buff[MAX_NAME_SIZE] = "";
    int letter_i = 0;
    
    int symb_i = 0, token_i = 0;
    while (symb_i < n_symbs)
    {
        switch (str[symb_i])
        {
            case '(':
                token_arr[token_i++].type = L_PARANTHESIS;
                symb_i++;
                break;

            case ')':
                token_arr[token_i++].type = R_PARANTHESIS;
                symb_i++;
                break;

            case '+':
                token_arr[token_i++].type = PLUS;
                symb_i++;
                break;

            case '-':
                token_arr[token_i++].type = MINUS;
                symb_i++;
                break;

            case '*':
                token_arr[token_i++].type = MULT;
                symb_i++;
                break;

            case '/':
                token_arr[token_i++].type = DIV;
                symb_i++;
                break;

            case '^':
                token_arr[token_i++].type = POW;
                symb_i++;
                break;

            case '0':
            case '1':
            case '2':
            case '3':
            case '4':
            case '5':                
            case '6':
            case '7':                
            case '8':                
            case '9':
            {
                if (token_i > 0 && (token_arr[token_i - 1].type == PI || token_arr[token_i - 1].type == E_NUM || token_arr[token_i - 1].type == VARIABLE))
                {
                    Show_Error (str, symb_i);
                    MY_ASSERT (false, "str[symb_i]", UNEXP_SYMB, ERROR);
                }

                token_arr[token_i].type = NUMBER;

                double num = NAN;
                MY_ASSERT (Get_Number (str, &symb_i, &num) != ERROR, "Get_Number ()", FUNC_ERROR, ERROR);

                token_arr[token_i++].value.num = num;
                break;
            }
            
            case ' ':
            case '\t':
            case '\n':
            case '\r':
                symb_i++;
                break;

            default:
                if (('a' <= str[symb_i] && str[symb_i] <= 'z') || ('A' <= str[symb_i] && str[symb_i] <= 'Z') || str[symb_i] == '_')
                {
                    do
                    {
                        name_buff[letter_i++] = str[symb_i++];
                    }
                    while (('a' <= str[symb_i] && str[symb_i] <= 'z') || ('A' <= str[symb_i] && str[symb_i] <= 'Z') || str[symb_i] == '_');
                }
                else
                {
                    Show_Error (str, symb_i);
                    MY_ASSERT (false, "str[symb_i]", UNEXP_SYMB, ERROR);
                }
                break;
        }

        if (letter_i > 0)
        {
            int func_num = Check_Function (name_buff);

            if (func_num != ERROR)
            {
                token_arr[token_i++].type = func_num;
            }
            else if (strcmp (name_buff, "e") == 0)
            {
                token_arr[token_i++].type = E_NUM;
            }
            else if (strcmp (name_buff, "pi") == 0)
            {
                token_arr[token_i++].type = PI;
            }
            else
            {
                token_arr[token_i].type = VARIABLE;
                memcpy (token_arr[token_i++].value.str, name_buff, MAX_NAME_SIZE);
            }

            memset (name_buff, 0, MAX_NAME_SIZE);
            letter_i = 0;
        }
    }

    return token_i;
}

static void Show_Error (const char *str, const int symb_i)
{
    printf ("%s\n", str);
    
    for (int i = 0; i < symb_i; i++)
        printf (" ");

    printf ("^~~~~~~~~~ Incorrect symbol\n");
}

static int Get_Number (const char *str, int *symb_i, double *num_ptr)
{
    MY_ASSERT (str,    "const char *str", NULL_PTR, ERROR);
    MY_ASSERT (symb_i, "int *symb_i",     NULL_PTR, ERROR);

    double num = 0.0;

    while ('0' <= str[*symb_i] && str[*symb_i] <= '9')
    {
        num = num * 10 + (str[*symb_i] - '0');
        (*symb_i)++;
    }

    if (str[*symb_i] == '.')
    {
        (*symb_i)++;
        const char *old_str = str + *symb_i;

        int degree = -1;

        while ('0' <= str[*symb_i] && str[*symb_i] <= '9')
        {
            num += (str[*symb_i] - '0') * pow (10, degree);
            degree--;
            (*symb_i)++;
        }

        if (old_str == str + *symb_i)
        {
            Show_Error (str, *symb_i);
            return ERROR;
        }
    }

    *num_ptr = num;

    return NO_ERRORS;
}

static int Check_Function (const char *func_name)
{
    MY_ASSERT (func_name, "const char *func_name", NULL_PTR, ERROR);

    for (int func_i = 0; Functions_Data_Base[func_i].name[0] != '\0'; func_i++)
    {
        if (strcmp (Functions_Data_Base[func_i].name, func_name) == 0)
            return Functions_Data_Base[func_i].num;
    }

    return ERROR;
}

#ifdef LEXER_DUMP
static int Lexer_Dump (const struct Token *token_arr, const int n_tokens)
{
    for (int token_i = 0; token_i < n_tokens; token_i++)
    {
        switch (token_arr[token_i].type)
        {
            case NUMBER:
                printf ("number: %f\n", token_arr[token_i].value.num);
                break;

            case PI:
                printf ("pi\n");
                break;

            case E_NUM:
                printf ("e\n");
                break;

            case VARIABLE:
                printf ("variable: %s\n", token_arr[token_i].value.str);
                break;

            case PLUS:
                printf ("+\n");
                break;

            case MINUS:
                printf ("-\n");
                    break;

            case MULT:
                printf ("*\n");
                break;

            case DIV:
                printf ("/\n");
                break;

            case POW:
                printf ("^");
                break;

            case SQRT:
                printf ("sqrt\n");
                break;

            case LN:
                printf ("ln\n");
                break;

            case SIN:
                printf ("sin\n");
                break;

            case COS:
                printf ("cos\n");
                break;

            case TAN:
                printf ("tan\n");
                break;

            case COT:
                printf ("cot\n");
                break;

            case ARCSIN:
                printf ("arcsin\n");
                break;

            case ARCCOS:
                printf ("arccos\n");
                break;

            case ARCTAN:
                printf ("arctan\n");
                break;

            case ARCCOT:
                printf ("arccot\n");
                break;

            case SINH:
                printf ("sing\n");
                break;
                
            case COSH:
                printf ("cosh\n");
                break;
                
            case TANH:
                printf ("tanh\n");
                break;

            case COTH:
                printf ("coth\n");
                break;
                
            case L_PARANTHESIS:
                printf ("(\n");
                break;

            case R_PARANTHESIS:
                printf (")\n");
                break;

            default:
                MY_ASSERT (false, "token_arr[token_i].type", UNEXP_VAL, ERROR);
        }
    }

    return NO_ERRORS;
}
#endif

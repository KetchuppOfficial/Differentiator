#include "../include/Differentiator.h"
#include "../include/Stack.h"
#include "My_Lib.h"

static struct Token *Lexer  (const char *buffer, const long n_symbs, int *n_tokens); 
struct Node         *Parser (const struct Token *token_arr, const int n_tokens);

struct Node *Plant_Tree (const char *buffer, const long n_symbs)
{
    MY_ASSERT (buffer, "const char *buffer", NULL_PTR, NULL);
    
    int n_tokens = 0;
    struct Token *token_arr = Lexer (buffer, n_symbs, &n_tokens);
    MY_ASSERT (token_arr, "Lexer ()", FUNC_ERROR, NULL);

    struct Node *root = Parser (token_arr, n_tokens);
    MY_ASSERT (root, "Parser ()", FUNC_ERROR, NULL);

    free (token_arr);

    return root;
}

// ========================================= LEXER ========================================= //

static int  Lexer_         (const char *str, const long n_symbs, struct Token *token_arr);
static int  Get_Number     (const char *str, int *symb_i, double *num_ptr);
static void Show_Error     (const char *str, const int symb_i);
static int  Check_Function (const char *func_name);

#ifdef LEXER_DUMP
static int Lexer_Dump (const struct Token *token_arr, const int n_tokens);
#endif

// ========================================================================================= //

struct Token
{
    enum Types  type;
    union Value value;
};

static struct Token *Lexer (const char *buffer, const long n_symbs, int *n_tokens)
{
    MY_ASSERT (buffer,   "const char *buffer", NULL_PTR, NULL);
    MY_ASSERT (n_tokens, "int *n_tokens",      NULL_PTR, NULL);

    struct Token *token_arr = (struct Token *)calloc (n_symbs, sizeof (struct Token));
    MY_ASSERT (token_arr, "struct Token *token_arr", NE_MEM, NULL);

    *n_tokens = Lexer_ (buffer, n_symbs, token_arr);
    if (*n_tokens == ERROR)
        MY_ASSERT (false, "Lexer_ ()", FUNC_ERROR, NULL);

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
    while (symb_i < n_symbs && str[symb_i] != '\n')
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

struct Parser
{
    struct Stack *operator_stack;
    struct Stack *node_stack;
    const struct Token *token_arr;
    int n_tokens;
    int token_i;
};

// ================================================= PARSER ================================================ //

static struct Parser *Parser_Ctor (const struct Token *token_arr, const int n_tokens);
static int            Parser_Dtor (struct Parser *parser);

static struct Node *Parser_  (struct Parser *parser);
static struct Node *Get_Root (struct Parser *parser);

static int Add_PM_Node          (struct Parser *parser);
static int Handle_Plus_Minus    (struct Parser *parser);
static int Add_MD_Node          (struct Parser *parser);
static int Handle_Mult_Div      (struct Parser *parser);
static int Handle_Pow           (struct Parser *parser);
static int Handle_Other         (struct Parser *parser);
static int Handle_Parenthesis   (struct Parser *parser);
static int Handle_Function      (struct Parser *parser);
static int Handle_Num_Var_Const (struct Parser *parser);

static struct Token *Pop_Operator  (struct Stack *operator_stack);
static struct Node  *Add_Node      (const struct Token *token_ptr, struct Node *l_son, struct Node *r_son);
static int           Gather_Two_Nodes (struct Stack *node_stack, struct Stack *operator_stack);

// ========================================================================================================= //

// =========================================== GENERAL FUNCTIONS =========================================== //

struct Node *Parser (const struct Token *token_arr, const int n_tokens)
{
    MY_ASSERT (token_arr,    "const struct Token *token_arr", NULL_PTR, NULL);
    MY_ASSERT (n_tokens > 0, "int n_tokens",                  POS_VAL,  NULL);
    
    struct Parser *parser = Parser_Ctor (token_arr, n_tokens);
    MY_ASSERT (parser, "Parser_Ctor ()", FUNC_ERROR, NULL);
    
    struct Node *root = Parser_ (parser);
    MY_ASSERT (root, "Parser_", FUNC_ERROR, NULL);

    MY_ASSERT (Parser_Dtor (parser) != ERROR, "Parser_Dtor ()", FUNC_ERROR, NULL);

    return root;
}

static struct Parser *Parser_Ctor (const struct Token *token_arr, const int n_tokens)
{
    MY_ASSERT (token_arr,    "const struct Token *token_arr", NULL_PTR, NULL);
    MY_ASSERT (n_tokens > 0, "int n_tokens",                  POS_VAL,  NULL);
    
    struct Parser *parser = (struct Parser *)calloc (1, sizeof (struct Parser));
    MY_ASSERT (parser, "struct Parser *parser", NE_MEM, NULL);

    parser->token_arr = token_arr;
    parser->n_tokens  = n_tokens;
    parser->token_i   = 0;
    
    parser->operator_stack = Stack_Ctor ();
    MY_ASSERT (parser->operator_stack, "Stack_Ctor ()", FUNC_ERROR, NULL);

    parser->node_stack = Stack_Ctor ();
    MY_ASSERT (parser->node_stack, "Stack_Ctor ()", FUNC_ERROR, NULL);

    return parser;
}

static int Parser_Dtor (struct Parser *parser)
{
    MY_ASSERT (parser, "struct Parser *parser", NULL_PTR, ERROR);

    parser->token_arr = NULL;
    parser->n_tokens  = 0;
    parser->token_i   = 0;

    if (Stack_Dtor (parser->operator_stack) == ERROR)
        MY_ASSERT (false, "Stack_Dtor (parser->operator_stack)", FUNC_ERROR, ERROR);

    if (Stack_Dtor (parser->node_stack) == ERROR)
        MY_ASSERT (false, "Stack_Dtor (parser->node_stack)", FUNC_ERROR, ERROR);

    free (parser);

    return NO_ERRORS;
}

int Tree_Destructor (struct Node *node_ptr)
{
    MY_ASSERT (node_ptr, "struct Node *node_ptr", NULL_PTR, ERROR);

    if (node_ptr->left_son)
        Tree_Destructor (node_ptr->left_son);

    if (node_ptr->right_son)
        Tree_Destructor (node_ptr->right_son);

    node_ptr->type = 0;

    node_ptr->left_son  = NULL;
    node_ptr->right_son = NULL;

    free (node_ptr);

    return NO_ERRORS;
}

// ========================================================================================================= //

// =========================================== RECURSIVE DESCENT =========================================== //

#define CURR_TOKEN parser->token_arr[parser->token_i]
#define OP_STACK_TOP ((struct Token *)Stack_Top_Elem (parser->operator_stack))
#define OP_STACK parser->operator_stack
#define ND_STACK parser->node_stack

static struct Node *Parser_ (struct Parser *parser)
{
    MY_ASSERT (parser, "struct Parser *parser", NULL_PTR, NULL);

    if (Handle_Plus_Minus (parser) == ERROR)
        MY_ASSERT (false, "Handle_Plus_Minus ()", FUNC_ERROR, NULL);

    struct Node *root = Get_Root (parser);
    MY_ASSERT (root, "Get_Root ()", FUNC_ERROR, NULL);

    return root;
}

static struct Node *Get_Root (struct Parser *parser)
{
    MY_ASSERT (parser, "struct Parser *parser", NULL_PTR, NULL);

    if (parser->n_tokens > 1)
    {
        while (Get_Stack_Size (ND_STACK) > 1 && Get_Stack_Size (OP_STACK) > 0)
            Gather_Two_Nodes (ND_STACK, OP_STACK);
    }
    
    struct Node *root = NULL;
    Stack_Pop (ND_STACK, (void **)&root);

    return root;
}

static struct Token *Pop_Operator (struct Stack *operator_stack)
{
    MY_ASSERT (operator_stack, "struct Stack *operator_stack", NULL_PTR, NULL);

    struct Token *ptr = NULL;
    Stack_Pop (operator_stack, (void **)&ptr);

    return ptr;
}

static struct Node *Add_Node (const struct Token *token_ptr, struct Node *l_son, struct Node *r_son)
{
    MY_ASSERT (token_ptr, "struct Token *token_ptr", NULL_PTR, NULL);

    struct Node *node_ptr = (struct Node *)calloc (1, sizeof (struct Node));
    MY_ASSERT (node_ptr, "struct Node *node_ptr", NE_MEM, NULL);

    node_ptr->left_son  = l_son;
    node_ptr->right_son = r_son;
    node_ptr->parent    = NULL;
    node_ptr->type      = token_ptr->type;
    node_ptr->value     = token_ptr->value;

    return node_ptr;
}

static int Add_PM_Node (struct Parser *parser)
{
    MY_ASSERT (parser, "struct Parser *parser", NULL_PTR, ERROR);
    
    if (Get_Stack_Size (OP_STACK) == 0 || OP_STACK_TOP->type == L_PARANTHESIS)
    {
        Stack_Push (OP_STACK, parser->token_arr + parser->token_i);
    }
    else
    {
        while (Get_Stack_Size (OP_STACK) > 0 && 
               (OP_STACK_TOP->type == PLUS || OP_STACK_TOP->type == MINUS ||
                OP_STACK_TOP->type == MULT || OP_STACK_TOP->type == DIV   || OP_STACK_TOP->type == POW))
        {
            MY_ASSERT (Gather_Two_Nodes (ND_STACK, OP_STACK) != ERROR, "Gather_Two_Nodes ()", FUNC_ERROR, ERROR);
        }

        Stack_Push (OP_STACK, parser->token_arr + parser->token_i);
    }

    return NO_ERRORS;
}

static int Handle_Plus_Minus (struct Parser *parser)
{
    MY_ASSERT (parser, "struct Parser *parser", NULL_PTR, ERROR);

    if (Handle_Mult_Div (parser) == ERROR)
        MY_ASSERT (false, "Handle_Mult_Div ()", FUNC_ERROR, ERROR);

    while (parser->token_i < parser->n_tokens && 
           (CURR_TOKEN.type == PLUS || CURR_TOKEN.type == MINUS))
    {
        MY_ASSERT (Add_PM_Node (parser), "Add_PM_Node ()", FUNC_ERROR, ERROR);
        
        parser->token_i++;

        if (Handle_Mult_Div (parser) == ERROR)
            MY_ASSERT (false, "Handle_Mult_Div ()", FUNC_ERROR, ERROR);
    }

    return NO_ERRORS;
}

int Add_MD_Node (struct Parser *parser)
{
    MY_ASSERT (parser, "struct Parser *parser", NULL_PTR, ERROR);
    
    if (Get_Stack_Size (OP_STACK) == 0 || OP_STACK_TOP->type == L_PARANTHESIS)
    {
        Stack_Push (OP_STACK, parser->token_arr + parser->token_i);
    }
    else
    {
        if (OP_STACK_TOP->type != POW)
            Stack_Push (OP_STACK, parser->token_arr + parser->token_i);
        else
        {
            MY_ASSERT (Gather_Two_Nodes (ND_STACK, OP_STACK), "Gather_Two_Nodes ()", FUNC_ERROR, ERROR);

            Stack_Push (OP_STACK, parser->token_arr + parser->token_i);
        }
        
    }

    return NO_ERRORS;
}

static int Handle_Mult_Div (struct Parser *parser)
{
    MY_ASSERT (parser, "struct Parser *parser", NULL_PTR, ERROR);

    if (Handle_Pow (parser) == ERROR)
        MY_ASSERT (false, "Handle_Pow ()", FUNC_ERROR, ERROR);

    while (parser->token_i < parser->n_tokens && 
           (CURR_TOKEN.type == MULT || CURR_TOKEN.type == DIV))
    {
        MY_ASSERT (Add_MD_Node (parser), "Add_MD_Node ()", FUNC_ERROR, ERROR);
        
        parser->token_i++;

        if (Handle_Pow (parser) == ERROR)
            MY_ASSERT (false, "Handle_Pow ()", FUNC_ERROR, ERROR);
    }

    return NO_ERRORS;
}

static int Handle_Pow (struct Parser *parser)
{
    MY_ASSERT (parser, "struct Parser *parser", NULL_PTR, ERROR);

    if (Handle_Other (parser) == ERROR)
        MY_ASSERT (false, "Handle_Parenthesis ()", FUNC_ERROR, ERROR);

    while (parser->token_i < parser->n_tokens && CURR_TOKEN.type == POW)
    {
        Stack_Push (OP_STACK, parser->token_arr + parser->token_i);

        parser->token_i++;

        if (Handle_Other (parser) == ERROR)
            MY_ASSERT (false, "Handle_Parenthesis ()", FUNC_ERROR, ERROR);
    }

    return NO_ERRORS;
}

static int Handle_Other (struct Parser *parser)
{
    MY_ASSERT (parser, "struct Parser *parser", NULL_PTR, ERROR);

    switch (CURR_TOKEN.type)
    {
        case L_PARANTHESIS:
            MY_ASSERT (Handle_Parenthesis (parser) != ERROR, "Handle_Parenthesis ()", FUNC_ERROR, ERROR);
            break;

        case SQRT:
        case LN:
        case SIN:
        case COS:
        case TAN:
        case COT:
        case ARCSIN:
        case ARCCOS:
        case ARCTAN:
        case ARCCOT:
        case SINH:
        case COSH:
        case TANH:
        case COTH:
            MY_ASSERT (Handle_Function (parser) != ERROR, "Handle_Function ()", FUNC_ERROR, ERROR);
            break;

        case VARIABLE:
        case NUMBER:
        case PI:
        case E_NUM:
            MY_ASSERT (Handle_Num_Var_Const (parser) != ERROR, "Handle_Num_Var_Const ()", FUNC_ERROR, ERROR);
            break;

        default:
            MY_ASSERT (false, "parser->token_arr[parser->torkn_i]", UNEXP_VAL, ERROR);
    }

    return NO_ERRORS;
}

static int Handle_Parenthesis (struct Parser *parser)
{
    MY_ASSERT (parser, "struct Parser *parser", NULL_PTR, ERROR);
    
    Stack_Push (OP_STACK, parser->token_arr + parser->token_i);

    parser->token_i++;

    if (Handle_Plus_Minus (parser) == ERROR)
        MY_ASSERT (false, "Handle_Plus_Minus ()", FUNC_ERROR, ERROR);

    if (CURR_TOKEN.type == R_PARANTHESIS)
    {
        while (OP_STACK_TOP->type != L_PARANTHESIS)
            MY_ASSERT (Gather_Two_Nodes (ND_STACK, OP_STACK), "Gather_Two_Nodes ()", FUNC_ERROR, ERROR);

        Stack_Pop (OP_STACK, NULL); 

        if (Get_Stack_Size (OP_STACK) > 0)
        {
            switch (OP_STACK_TOP->type)
            {
                case SQRT:
                case LN:
                case SIN:
                case COS:
                case TAN:
                case COT:
                case ARCSIN:
                case ARCCOS:
                case ARCTAN:
                case ARCCOT:
                case SINH:
                case COSH:
                case TANH:
                case COTH:
                    MY_ASSERT (Gather_Two_Nodes (ND_STACK, OP_STACK), "Gather_Two_Nodes ()", FUNC_ERROR, ERROR);
                    break;

                default:
                    break;
            }
        }
        
        parser->token_i++;
    }
    else
        MY_ASSERT (false, "parser->token_arr[parser->token_i]", UNEXP_VAL, ERROR);

    return NO_ERRORS;
}

static int Handle_Function (struct Parser *parser)
{
    MY_ASSERT (parser, "struct Parser *parser", NULL_PTR, ERROR);

    Stack_Push (OP_STACK, parser->token_arr + parser->token_i);

    parser->token_i++;

    if (Handle_Other (parser) == ERROR)
        MY_ASSERT (false, "Handle_Parenthesis ()", FUNC_ERROR, ERROR);

    return NO_ERRORS;
}

static int Handle_Num_Var_Const (struct Parser *parser)
{
    MY_ASSERT (parser, "struct Parser *parser", NULL_PTR, ERROR);

    struct Node *new_node = Add_Node (parser->token_arr + parser->token_i, NULL, NULL);
    MY_ASSERT (new_node, "Add_Node", FUNC_ERROR, ERROR);

    Stack_Push (ND_STACK, new_node);

    parser->token_i++;
        
    return NO_ERRORS;
}

static int Gather_Two_Nodes (struct Stack *node_stack, struct Stack *operator_stack)
{
    MY_ASSERT (node_stack,     "struct Stack *node_stack",     NULL_PTR, ERROR);
    MY_ASSERT (operator_stack, "struct Stack *operator_stack", NULL_PTR, ERROR);

    struct Token *token_ptr = Pop_Operator (operator_stack);
    MY_ASSERT (token_ptr, "Pop_Operator ()", FUNC_ERROR, ERROR);
    
    struct Node *node_1 = NULL;
    
    Stack_Pop (node_stack, (void **)&node_1);
    MY_ASSERT (node_1, "struct Node *node_1", NULL_PTR, ERROR);

    struct Node *curr_root = NULL;
    switch (token_ptr->type)
    {
        case SQRT:
        case LN:
        case SIN:
        case COS:
        case TAN:
        case COT:
        case ARCSIN:
        case ARCCOS:
        case ARCTAN:
        case ARCCOT:
        case SINH:
        case COSH:
        case TANH:
        case COTH:
            curr_root = Add_Node (token_ptr, node_1, NULL);
            MY_ASSERT (curr_root, "Add_Node ()", FUNC_ERROR, ERROR);
            break;

        default:
        {
            struct Node *node_2 = NULL;
        
            Stack_Pop (node_stack, (void **)&node_2);
            MY_ASSERT (node_2, "struct Node *node_2", NULL_PTR, ERROR);

            curr_root = Add_Node (token_ptr, node_2, node_1);
            MY_ASSERT (curr_root, "Add_Node ()", FUNC_ERROR, ERROR);

            node_2->parent = curr_root;
            break;
        }
    }

    node_1->parent = curr_root;

    Stack_Push (node_stack, curr_root);

    return NO_ERRORS;
}

// ========================================================================================================= //


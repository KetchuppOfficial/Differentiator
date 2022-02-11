#include "Differentiator.h"
#include "Functions.h"
#include "./Auxiliaries/Reading_File.h"
#include "./Auxiliaries/Log_File.h"

extern const char *TEX_FILE;

//*****************************************************************************************************************
int Diff_Every_Var (struct Node *root_ptr)
{
    int n_vars = 0;
    char *vars_arr = Find_Vars (root_ptr, &n_vars);

    struct Node **tree_arr = (struct Node **)calloc (n_vars, sizeof (struct Node *));
    MY_ASSERT (tree_arr, "struct Node **tree_arr", NE_MEM, ERROR);

    for (int i = 0; i < n_vars; i++)
    {
        tree_arr[i] = (struct Node *)calloc (1, sizeof  (struct Node));
        MY_ASSERT (tree_arr[i], "tree_arr[i]", NE_MEM, ERROR);
    }

    for (int i = 0; i < n_vars; i++)
        Simplify_Function (&root_ptr, vars_arr[i]);

    for (int i = 0; i < n_vars; i++)
    {
        if (Diff_One_Var (root_ptr, vars_arr[i], tree_arr[i]) == ERROR)
            return ERROR;

        Simplify_Function (tree_arr + i, vars_arr[i]);

        char buffer_1[BUFF_SIZE] = "";
        char buffer_2[BUFF_SIZE] = "";
        if (n_vars > 1)
        {
            sprintf (buffer_1, "Partial_Derivative_Of_%c.dot", toupper (vars_arr[i]));
            sprintf (buffer_2, "Partial_Derivative_Of_%c.png", toupper (vars_arr[i]));
        }
        else
        {
            sprintf (buffer_1, "./Output/Derivative.dot");
            sprintf (buffer_2, "./Output/Derivative.png");
        }

        Tree_Dump (tree_arr[i], buffer_1, buffer_2, vars_arr[i]);
    }

    Dump_In_Tex (root_ptr, tree_arr, vars_arr, n_vars, TEX_FILE);

    for (int i = 0; i < n_vars; i++)
        Tree_Destructor (tree_arr[i]);

    free (tree_arr);
    free (vars_arr);

    return NO_ERRORS;
}

int Simplify_Function (struct Node **root_ptr, const char var)
{
    int change_i = 0;
    for (;;)
    {
        int n_changes = Check_Fractions (*root_ptr);
        n_changes += Simplify (root_ptr, var);
        if (change_i == n_changes)
            break;
        else
            change_i = n_changes;
    }

    return NO_ERRORS;
}

char *Find_Vars (struct Node *node_ptr, int *n_vars)
{
    MY_ASSERT (node_ptr, "struct Node *node_ptr", NULL_PTR, NULL);
    MY_ASSERT (n_vars,   "int *n_vars",           NULL_PTR, NULL);

    char *vars_arr = (char *)calloc (51, sizeof (char));
    MY_ASSERT (vars_arr, "char *vars_arr", NE_MEM, NULL);

    int var_i = 0;
    Add_Vars (node_ptr, vars_arr, &var_i);

    *n_vars = var_i;

    return vars_arr;
}

int Add_Vars (struct Node* node_ptr, char *vars_arr, int *var_i)
{
    MY_ASSERT (node_ptr, "struct Node *node_ptr", NULL_PTR, ERROR);
    MY_ASSERT (vars_arr, "char *vars_arr",        NULL_PTR, ERROR);
    MY_ASSERT (var_i,    "int *var_i",            NULL_PTR, ERROR);

    if (node_ptr->type == VARIABLE && !strchr (vars_arr, node_ptr->value.symbol))
    {
        vars_arr[*var_i] = node_ptr->value.symbol;
        (*var_i)++;
    }

    if (node_ptr->left_son)
        Add_Vars (node_ptr->left_son, vars_arr, var_i);

    if (node_ptr->right_son)
        Add_Vars (node_ptr->right_son, vars_arr, var_i);

    return NO_ERRORS;
}

#define Diff_Sin(node_ptr, var, new_node_ptr)    Diff_Sin_Sinh_Cosh (node_ptr, var, new_node_ptr, Sin)
#define Diff_Sinh(node_ptr, var, new_node_ptr)   Diff_Sin_Sinh_Cosh (node_ptr, var, new_node_ptr, Sinh)
#define Diff_Cosh(node_ptr, var, new_node_ptr)   Diff_Sin_Sinh_Cosh (node_ptr, var, new_node_ptr, Cosh)
#define Diff_Tan(node_ptr, var, new_node_ptr)    Diff_Tan_Tanh      (node_ptr, var, new_node_ptr, Tan)
#define Diff_Tanh(node_ptr, var, new_node_ptr)   Diff_Tan_Tanh      (node_ptr, var, new_node_ptr, Tanh)
#define Diff_Cot(node_ptr, var, new_node_ptr)    Diff_Cot_Coth      (node_ptr, var, new_node_ptr, Cot)
#define Diff_Coth(node_ptr, var, new_node_ptr)   Diff_Cot_Coth      (node_ptr, var, new_node_ptr, Coth)
#define Diff_Arcsin(node_ptr, var, new_node_ptr) Diff_Arcsin_Arccos (node_ptr, var, new_node_ptr, Arcsin)
#define Diff_Arccos(node_ptr, var, new_node_ptr) Diff_Arcsin_Arccos (node_ptr, var, new_node_ptr, Arccos)
#define Diff_Arctan(node_ptr, var, new_node_ptr) Diff_Arctan_Arccot (node_ptr, var, new_node_ptr, Arctan)
#define Diff_Arccot(node_ptr, var, new_node_ptr) Diff_Arctan_Arccot (node_ptr, var, new_node_ptr, Arccot)

#define Differentiate_Inside(what_diff, where_put, parent_ptr)      \
do                                                                  \
{                                                                   \
    where_put = (struct Node *)calloc (1, sizeof (struct Node));    \
    where_put->parent = parent_ptr;                                 \
                                                                    \
    Diff_One_Var (what_diff, var, where_put);                       \
}                                                                   \
while (0)

#define Create_Node(node_type, val, parent) Create_Node_ (node_type, parent, 1, val)

#define VAL_FUNC node_ptr->value.function

#define Code_Generate(func)                         \
if (!strcmp (VAL_FUNC, Functions_Data_Base[func]))  \
    Diff_##func (node_ptr, var, new_node_ptr);      \
                                                    \
else

int Diff_One_Var (const struct Node *node_ptr, const char var, struct Node *new_node_ptr)
{
    MY_ASSERT (node_ptr,     "const struct Node *node_ptr", NULL_PTR, ERROR);
    MY_ASSERT (new_node_ptr, "struct Node *new_node_ptr",   NULL_PTR, ERROR);

    switch (node_ptr->type)
    {
        case FUNCTION:
            #include "Insert_Functions.h"
            MY_ASSERT (false, "node_ptr->value.function", UNEXP_VAL, ERROR);
            break;

        case VARIABLE:
            new_node_ptr->type = NUMBER;
            if (node_ptr->value.symbol == var)
                new_node_ptr->value.number = 1.0;
            else
                new_node_ptr->value.number = 0.0;

            new_node_ptr->left_son  = NULL;
            new_node_ptr->right_son = NULL;
            break;

        case NUMBER:
        case MATH_CONST:
            new_node_ptr->type = NUMBER;
            new_node_ptr->value.number = 0.0;
            new_node_ptr->left_son  = NULL;
            new_node_ptr->right_son = NULL;
            break;

        case MATH_SIGN:
            switch (node_ptr->value.symbol)
            {
                case '+':
                case '-':
                    Diff_Sum_Sub (node_ptr, var, new_node_ptr);
                    break;

                case '*':
                    Diff_Mult (node_ptr, var, new_node_ptr, '+');
                    break;

                case '/':
                    Diff_Div (node_ptr, var, new_node_ptr);
                    break;

                case '^':
                    switch (Check_Operands (node_ptr, var))
                    {
                        case L_FUNC_R_NUM:
                        case L_FUNC_R_CONST:
                            Diff_Pow (node_ptr, var, new_node_ptr);
                            break;

                        case L_NUM_R_FUNC:
                        case L_CONST_R_FUNC:
                            Diff_Exp (node_ptr, var, new_node_ptr);
                            break;

                        case L_FUNC_R_FUNC:
                            Diff_Pow_Exp (node_ptr, var, new_node_ptr);
                            break;

                        case L_CONST_R_CONST:
                        case L_CONST_R_NUM:
                        case L_NUM_R_CONST:
                            new_node_ptr->type = NUMBER;
                            new_node_ptr->value.number = 0.0;

                            new_node_ptr->left_son  = NULL;
                            new_node_ptr->right_son = NULL;
                            break;

                        default: MY_ASSERT (false, "Check_Operands ()", FUNC_ERROR, ERROR);
                    }
                    break;

                default: MY_ASSERT (false, "node_ptr->value.symbol", UNEXP_VAL, ERROR);
            }
            break;

        default: MY_ASSERT (false, "node_ptr->type", UNEXP_VAL, ERROR);
    }

    return NO_ERRORS;
}
#undef VAL_FUNC

int Diff_Sum_Sub (const struct Node *node_ptr, const char var, struct Node *new_node_ptr)
{
    MY_ASSERT (node_ptr,     "const struct Node *node_ptr", NULL_PTR, ERROR);
    MY_ASSERT (new_node_ptr, "struct Node *new_node_ptr",   NULL_PTR, ERROR);

    new_node_ptr->type = MATH_SIGN;
    new_node_ptr->value.symbol = node_ptr->value.symbol;

    Differentiate_Inside (node_ptr->left_son,  new_node_ptr->left_son,  new_node_ptr);

    Differentiate_Inside (node_ptr->right_son, new_node_ptr->right_son, new_node_ptr);

    return NO_ERRORS;
}

int Diff_Mult (const struct Node *node_ptr, const char var, struct Node *new_node_ptr, const char sum_sub)
{
    MY_ASSERT (node_ptr,     "const struct Node *node_ptr", NULL_PTR, ERROR);
    MY_ASSERT (new_node_ptr, "struct Node *new_node_ptr",   NULL_PTR, ERROR);

    if (sum_sub != '+' && sum_sub != '-')
        return ERROR;

    new_node_ptr->type = MATH_SIGN;
    new_node_ptr->value.symbol = sum_sub;

    new_node_ptr->left_son = Create_Node (MATH_SIGN, '*', new_node_ptr);
    new_node_ptr->left_son->right_son = Copy_Tree (node_ptr->right_son, new_node_ptr->left_son);

    Differentiate_Inside (node_ptr->left_son, new_node_ptr->left_son->left_son, new_node_ptr->left_son);

    new_node_ptr->right_son = Create_Node (MATH_SIGN, '*', new_node_ptr);
    new_node_ptr->right_son->left_son = Copy_Tree (node_ptr->left_son, new_node_ptr->right_son);

    Differentiate_Inside (node_ptr->right_son, new_node_ptr->right_son->right_son, new_node_ptr->right_son);

    return NO_ERRORS;
}

int Diff_Div (const struct Node *node_ptr, const char var, struct Node *new_node_ptr)
{
    MY_ASSERT (node_ptr,     "const struct Node *node_ptr", NULL_PTR, ERROR);
    MY_ASSERT (new_node_ptr, "struct Node *new_node_ptr",   NULL_PTR, ERROR);

    new_node_ptr->type = MATH_SIGN;
    new_node_ptr->value.symbol = '/';

    new_node_ptr->left_son = Create_Node (MATH_SIGN, '-', new_node_ptr);
    Diff_Mult (node_ptr, var, new_node_ptr->left_son, '-');

    new_node_ptr->right_son = Create_Node (MATH_SIGN, '^', new_node_ptr);

    new_node_ptr->right_son->left_son  = Copy_Tree (node_ptr->right_son, new_node_ptr->right_son);
    new_node_ptr->right_son->right_son = Create_Node (NUMBER, 2.0, new_node_ptr->right_son);


    return NO_ERRORS;
}

int Diff_Sqrt (const struct Node *node_ptr, const char var, struct Node *new_node_ptr)
{
    MY_ASSERT (node_ptr,     "const struct Node *node_ptr", NULL_PTR, ERROR);
    MY_ASSERT (new_node_ptr, "struct Node *new_node_ptr",   NULL_PTR, ERROR);

    new_node_ptr->type = MATH_SIGN;
    new_node_ptr->value.symbol = '*';

    new_node_ptr->left_son  = Create_Node (NUMBER, 0.5, new_node_ptr);
    new_node_ptr->right_son = Create_Node (MATH_SIGN, '*', new_node_ptr);

    new_node_ptr->right_son->left_son = Create_Node (MATH_SIGN, '^', new_node_ptr->right_son);

    Differentiate_Inside (node_ptr->left_son, new_node_ptr->right_son->right_son, new_node_ptr->right_son);

    new_node_ptr->right_son->left_son->left_son  = Copy_Tree (node_ptr->left_son, new_node_ptr->right_son->left_son);
    new_node_ptr->right_son->left_son->right_son = Create_Node (NUMBER, -0.5, new_node_ptr->right_son->left_son);

    return NO_ERRORS;
}

int Diff_Ln (const struct Node *node_ptr, const char var, struct Node *new_node_ptr)
{
    MY_ASSERT (node_ptr,     "const struct Node *node_ptr", NULL_PTR, ERROR);
    MY_ASSERT (new_node_ptr, "struct Node *new_node_ptr",   NULL_PTR, ERROR);

    new_node_ptr->type = MATH_SIGN;
    new_node_ptr->value.symbol = '/';

    Differentiate_Inside (node_ptr->left_son, new_node_ptr->left_son, new_node_ptr);

    new_node_ptr->right_son = Copy_Tree (node_ptr->left_son, new_node_ptr);

    return NO_ERRORS;
}

int Diff_Sin_Sinh_Cosh (const struct Node *node_ptr, const char var, struct Node *new_node_ptr, Functions_Enum mode)
{
    MY_ASSERT (node_ptr,     "const struct Node *node_ptr", NULL_PTR, ERROR);
    MY_ASSERT (new_node_ptr, "struct Node *new_node_ptr",   NULL_PTR, ERROR);

    new_node_ptr->type = MATH_SIGN;
    new_node_ptr->value.symbol = '*';

    int func = 0;
    switch (mode)
    {
        case Sin:
            func = Cos;
            break;
        case Sinh:
            func = Cosh;
            break;
        case Cosh:
            func = Sinh;
            break;
        default: MY_ASSERT (false, "Functions_Enum mode", UNEXP_VAL, ERROR);
    }

    new_node_ptr->left_son = Create_Node (FUNCTION, Functions_Data_Base[func], new_node_ptr);

    new_node_ptr->left_son->left_son = Copy_Tree (node_ptr->left_son, new_node_ptr->left_son);

    Differentiate_Inside (node_ptr->left_son, new_node_ptr->right_son, new_node_ptr);

    return NO_ERRORS;
}

int Diff_Cos (const struct Node *node_ptr, const char var, struct Node *new_node_ptr)
{
    MY_ASSERT (node_ptr,     "const struct Node *node_ptr", NULL_PTR, ERROR);
    MY_ASSERT (new_node_ptr, "struct Node *new_node_ptr",   NULL_PTR, ERROR);

    new_node_ptr->type = MATH_SIGN;
    new_node_ptr->value.symbol = '*';

    new_node_ptr->left_son  = Create_Node (NUMBER, -1.0, new_node_ptr);
    new_node_ptr->right_son = Create_Node (MATH_SIGN, '*', new_node_ptr);

    new_node_ptr->right_son->left_son = Create_Node (FUNCTION, Functions_Data_Base[Sin], new_node_ptr->right_son);

    new_node_ptr->right_son->left_son->left_son = Copy_Tree (node_ptr->left_son, new_node_ptr->right_son->left_son);

    Differentiate_Inside (node_ptr->left_son, new_node_ptr->right_son->right_son, new_node_ptr->right_son);

    return NO_ERRORS;
}

int Diff_Tan_Tanh (const struct Node *node_ptr, const char var, struct Node *new_node_ptr, Functions_Enum mode)
{
    MY_ASSERT (node_ptr,                    "const struct Node *node_ptr", NULL_PTR,  ERROR);
    MY_ASSERT (new_node_ptr,                "struct Node *new_node_ptr",   NULL_PTR,  ERROR);
    MY_ASSERT (mode == Tan || mode == Tanh, "Functions_Enum mode",         UNEXP_VAL, ERROR);

    new_node_ptr->type = MATH_SIGN;
    new_node_ptr->value.symbol = '/';

    Differentiate_Inside (node_ptr->left_son, new_node_ptr->left_son, new_node_ptr);

    new_node_ptr->right_son = Create_Node (MATH_SIGN, '^', new_node_ptr);

    int func = (mode == Tan) ? Cos : Cosh;
    new_node_ptr->right_son->left_son  = Create_Node (FUNCTION, Functions_Data_Base[func], new_node_ptr->right_son);
    new_node_ptr->right_son->right_son = Create_Node (NUMBER, 2.0, new_node_ptr->right_son);

    new_node_ptr->right_son->left_son->left_son = Copy_Tree (node_ptr->left_son, new_node_ptr->right_son->left_son);

    return NO_ERRORS;
}

int Diff_Cot_Coth (const struct Node *node_ptr, const char var, struct Node *new_node_ptr, Functions_Enum mode)
{
    MY_ASSERT (node_ptr,                    "const struct Node *node_ptr", NULL_PTR,  ERROR);
    MY_ASSERT (new_node_ptr,                "struct Node *new_node_ptr",   NULL_PTR,  ERROR);
    MY_ASSERT (mode == Cot || mode == Coth, "Functions_Enum mode",         UNEXP_VAL, ERROR);

    new_node_ptr->type = MATH_SIGN;
    new_node_ptr->value.symbol = '/';

    new_node_ptr->left_son = Create_Node (MATH_SIGN, '*', new_node_ptr);

    new_node_ptr->left_son->left_son = Create_Node (NUMBER, -1.0, new_node_ptr->left_son);

    Differentiate_Inside (node_ptr->left_son, new_node_ptr->left_son->right_son, new_node_ptr->left_son);

    new_node_ptr->right_son = Create_Node (MATH_SIGN, '^', new_node_ptr);

    int func = (mode == Cot) ? Sin : Sinh;
    new_node_ptr->right_son->left_son  = Create_Node (FUNCTION, Functions_Data_Base[func], new_node_ptr->right_son);
    new_node_ptr->right_son->right_son = Create_Node (NUMBER, 2.0, new_node_ptr->right_son);

    new_node_ptr->right_son->left_son->left_son = Copy_Tree (node_ptr->left_son, new_node_ptr->right_son->left_son);

    return NO_ERRORS;
}

int Diff_Pow (const struct Node *node_ptr, const char var, struct Node *new_node_ptr)
{
    MY_ASSERT (node_ptr,     "const struct Node *node_ptr", NULL_PTR, ERROR);
    MY_ASSERT (new_node_ptr, "struct Node *new_node_ptr",   NULL_PTR, ERROR);

    new_node_ptr->type = MATH_SIGN;
    new_node_ptr->value.symbol = '*';

    new_node_ptr->left_son = Create_Node (MATH_SIGN, '*', new_node_ptr);

    Differentiate_Inside (node_ptr->left_son, new_node_ptr->right_son, new_node_ptr);

    new_node_ptr->left_son->left_son = Copy_Tree (node_ptr->right_son, new_node_ptr->left_son);

    new_node_ptr->left_son->right_son = Create_Node (MATH_SIGN, '^', new_node_ptr->left_son);

    new_node_ptr->left_son->right_son->left_son = Copy_Tree (node_ptr->left_son, new_node_ptr->left_son->right_son);

    new_node_ptr->left_son->right_son->right_son = Create_Node (MATH_SIGN, '-', new_node_ptr->left_son->right_son);

    new_node_ptr->left_son->right_son->right_son->right_son = Create_Node (NUMBER, 1.0, new_node_ptr->left_son->right_son->right_son);

    new_node_ptr->left_son->right_son->right_son->left_son = Copy_Tree (node_ptr->right_son, new_node_ptr->left_son->right_son->right_son);

    return NO_ERRORS;
}

int Diff_Exp (const struct Node *node_ptr, const char var, struct Node *new_node_ptr)
{
    MY_ASSERT (node_ptr,     "const struct Node *node_ptr", NULL_PTR, ERROR);
    MY_ASSERT (new_node_ptr, "struct Node *new_node_ptr",   NULL_PTR, ERROR);

    new_node_ptr->type = MATH_SIGN;
    new_node_ptr->value.symbol = '*';

    Differentiate_Inside (node_ptr->right_son, new_node_ptr->right_son, new_node_ptr);

    new_node_ptr->left_son = Create_Node (MATH_SIGN, '*', new_node_ptr);

    new_node_ptr->left_son->right_son = Create_Node (FUNCTION, Functions_Data_Base[Ln], new_node_ptr->left_son);

    new_node_ptr->left_son->right_son->left_son = Copy_Tree (node_ptr->left_son, new_node_ptr->left_son->right_son);

    new_node_ptr->left_son->left_son = Create_Node (MATH_SIGN, '^', new_node_ptr->left_son);

    new_node_ptr->left_son->left_son->left_son = Copy_Tree (node_ptr->left_son, new_node_ptr->left_son->left_son);

    new_node_ptr->left_son->left_son->right_son = Copy_Tree (node_ptr->right_son, new_node_ptr->left_son->left_son);

    return NO_ERRORS;
}

int Diff_Pow_Exp (const struct Node *node_ptr, const char var, struct Node *new_node_ptr)
{
    MY_ASSERT (node_ptr,     "const struct Node *node_ptr", NULL_PTR, ERROR);
    MY_ASSERT (new_node_ptr, "struct Node *new_node_ptr",   NULL_PTR, ERROR);

    new_node_ptr->type = MATH_SIGN;
    new_node_ptr->value.symbol = '*';

    new_node_ptr->left_son = Create_Node (MATH_SIGN, '^', new_node_ptr);

    new_node_ptr->left_son->left_son  = Copy_Tree (node_ptr->left_son, new_node_ptr->left_son);
    new_node_ptr->left_son->right_son = Copy_Tree (node_ptr->right_son, new_node_ptr->left_son);

    new_node_ptr->right_son = Create_Node (MATH_SIGN, '+', new_node_ptr);

    new_node_ptr->right_son->left_son = Create_Node (MATH_SIGN, '*', new_node_ptr->right_son);
    new_node_ptr->right_son->right_son = Create_Node (MATH_SIGN, '*', new_node_ptr->right_son);

    Differentiate_Inside (node_ptr->right_son, new_node_ptr->right_son->left_son->left_son, new_node_ptr->right_son->left_son);

    new_node_ptr->right_son->left_son->right_son = Create_Node (FUNCTION, Functions_Data_Base[Ln], new_node_ptr->right_son->left_son);

    new_node_ptr->right_son->left_son->right_son->left_son = Copy_Tree (node_ptr->left_son, new_node_ptr->right_son->left_son->right_son);

    new_node_ptr->right_son->right_son->left_son  = Copy_Tree (node_ptr->right_son, new_node_ptr->right_son->right_son);
    new_node_ptr->right_son->right_son->right_son = Create_Node (MATH_SIGN, '/', new_node_ptr->right_son->right_son);

    new_node_ptr->right_son->right_son->right_son->right_son = Copy_Tree (node_ptr->left_son, new_node_ptr->right_son->right_son->right_son);

    Differentiate_Inside (node_ptr->left_son, new_node_ptr->right_son->right_son->right_son->left_son, new_node_ptr->right_son->right_son->right_son);

    return NO_ERRORS;
}

int Diff_Arcsin_Arccos (const struct Node *node_ptr, const char var, struct Node *new_node_ptr, Functions_Enum mode)
{
    MY_ASSERT (node_ptr,                         "const struct Node *node_ptr", NULL_PTR,  ERROR);
    MY_ASSERT (new_node_ptr,                     "struct Node *new_node_ptr",   NULL_PTR,  ERROR);
    MY_ASSERT (mode == Arcsin || mode == Arccos, "Functions_Enum mode",         UNEXP_VAL, ERROR);

    new_node_ptr->type = MATH_SIGN;
    new_node_ptr->value.symbol = '*';

    Differentiate_Inside (node_ptr->left_son, new_node_ptr->right_son, new_node_ptr);

    new_node_ptr->left_son = Create_Node (MATH_SIGN, '/', new_node_ptr);

    double temp = (mode == Arcsin) ? 1.0 : -1.0;
    new_node_ptr->left_son->left_son  = Create_Node (NUMBER, temp, new_node_ptr->left_son);
    new_node_ptr->left_son->right_son = Create_Node (FUNCTION, Functions_Data_Base[Sqrt], new_node_ptr->left_son);

    new_node_ptr->left_son->right_son->left_son = Create_Node (MATH_SIGN, '-', new_node_ptr->left_son->right_son);

    new_node_ptr->left_son->right_son->left_son->left_son  = Create_Node (NUMBER, 1.0, new_node_ptr->left_son->right_son->left_son);
    new_node_ptr->left_son->right_son->left_son->right_son = Create_Node (MATH_SIGN, '^', new_node_ptr->left_son->right_son->left_son);

    new_node_ptr->left_son->right_son->left_son->right_son->left_son  = Copy_Tree (node_ptr->left_son, new_node_ptr->left_son->right_son->left_son->right_son);
    new_node_ptr->left_son->right_son->left_son->right_son->right_son = Create_Node (NUMBER, 2.0, new_node_ptr->left_son->right_son->left_son->right_son);

    return NO_ERRORS;
}

int Diff_Arctan_Arccot (const struct Node *node_ptr, const char var, struct Node *new_node_ptr, Functions_Enum mode)
{
    MY_ASSERT (node_ptr,                         "const struct Node *node_ptr", NULL_PTR,  ERROR);
    MY_ASSERT (new_node_ptr,                     "struct Node *new_node_ptr",   NULL_PTR,  ERROR);
    MY_ASSERT (mode == Arctan || mode == Arccot, "Functions_Enum mode",         UNEXP_VAL, ERROR);

    new_node_ptr->type = MATH_SIGN;
    new_node_ptr->value.symbol = '*';

    Differentiate_Inside (node_ptr->left_son, new_node_ptr->right_son, new_node_ptr);

    new_node_ptr->left_son = Create_Node (MATH_SIGN, '/', new_node_ptr);

    double temp = (mode == Arctan) ? 1.0 : -1.0;
    new_node_ptr->left_son->left_son  = Create_Node (NUMBER, temp, new_node_ptr->left_son);
    new_node_ptr->left_son->right_son = Create_Node (MATH_SIGN, '+', new_node_ptr->left_son);

    new_node_ptr->left_son->right_son->left_son  = Create_Node (NUMBER, 1.0, new_node_ptr->left_son->right_son);
    new_node_ptr->left_son->right_son->right_son = Create_Node (MATH_SIGN, '^', new_node_ptr->left_son->right_son);

    new_node_ptr->left_son->right_son->right_son->left_son  = Copy_Tree (node_ptr->left_son, new_node_ptr->left_son->right_son->right_son);
    new_node_ptr->left_son->right_son->right_son->right_son = Create_Node (NUMBER, 2.0, new_node_ptr->left_son->right_son->right_son);

    return NO_ERRORS;
}

struct Node *Create_Node_ (Types node_type, struct Node *parent, int parmN, ...)
{
    MY_ASSERT (parent, "struct Node *parent", NULL_PTR, NULL);

    struct Node *node_ptr = (struct Node *)calloc (1, sizeof (struct Node));
    MY_ASSERT (node_ptr, "struct Node *node_ptr", NE_MEM, NULL);

    node_ptr->type       = node_type;
    node_ptr->left_son   = NULL;
    node_ptr->right_son  = NULL;
    if (parent)
        node_ptr->parent = parent;

    va_list ap;
    va_start (ap, parmN);

    switch (node_type)
    {
        case NUMBER:
            node_ptr->value.number = va_arg (ap, double);
            break;
        case MATH_SIGN:
            node_ptr->value.symbol = va_arg (ap, int);
            break;
        case MATH_CONST:
            memmove (node_ptr->value.math_const, va_arg (ap, const char *), 2 * sizeof (char));
            break;
        case FUNCTION:
            memmove (node_ptr->value.function, va_arg (ap, const char *), MAX_F_SIZE);
            break;
        default:
            MY_ASSERT (false, "Types node_type", UNEXP_VAL, NULL);
    }

    va_end (ap);

    return node_ptr;
}

#define Left_T node_ptr->left_son->type
#define Right_T node_ptr->right_son->type
#define Left_Val node_ptr->left_son->value.symbol
#define Right_Val node_ptr->right_son->value.symbol
int Check_Operands (const struct Node *node_ptr, const char var)
{
    MY_ASSERT (node_ptr, "const struct Node *node_ptr", NULL_PTR, ERROR);

    if (!node_ptr->left_son || !node_ptr->right_son)
        return NO_ERRORS;

    if (Left_T == NUMBER && Right_T == NUMBER)
        return L_NUM_R_NUM;

    else if (Left_T == NUMBER && (Right_T == MATH_CONST || (Right_T == VARIABLE && Right_Val != var)))
        return L_NUM_R_CONST;

    else if ((Left_T == MATH_CONST || (Left_T == VARIABLE && Left_Val != var)) && Right_T == NUMBER)
        return L_CONST_R_NUM;

    else if ((Left_T  == MATH_CONST || (Left_T  == VARIABLE && Left_Val  != var)) &&
             (Right_T == MATH_CONST || (Right_T == VARIABLE && Right_Val != var)))
        return L_CONST_R_CONST;

    else if (Left_T == NUMBER && (Right_T == MATH_SIGN || Right_T == FUNCTION ||(Right_T == VARIABLE && Right_Val == var)))
        return L_NUM_R_FUNC;

    else if ((Left_T == MATH_SIGN || Left_T == FUNCTION || (Left_T == VARIABLE && Left_Val == var)) && Right_T == NUMBER)
        return L_FUNC_R_NUM;

    else if ((Left_T  == MATH_CONST || (Left_T == VARIABLE && Left_Val != var)) &&
             (Right_T == MATH_SIGN || Right_T == FUNCTION ||(Right_T == VARIABLE && Right_Val == var)))
        return L_CONST_R_FUNC;

    else if ((Left_T == MATH_SIGN || Left_T == FUNCTION || (Left_T == VARIABLE && Left_Val == var)) &&
             (Right_T == MATH_CONST || (Right_T == VARIABLE && Right_Val != var)))
        return L_FUNC_R_CONST;

    else if ((Left_T  == MATH_SIGN || Left_T  == FUNCTION || (Left_T  == VARIABLE && Left_Val  == var)) &&
             (Right_T == MATH_SIGN || Right_T == FUNCTION || (Right_T == VARIABLE && Right_Val == var)))
        return L_FUNC_R_FUNC;

    else
        return NO_ERRORS;
}
#undef Left_T
#undef Right_T
#undef Left_Val
#undef Right_Val

struct Node *Copy_Tree (const struct Node *node_ptr, struct Node *parent)
{
    MY_ASSERT (node_ptr, "const struct Node *node_ptr", NULL_PTR, NULL);
    MY_ASSERT (parent,   "struct Node *parent",         NULL_PTR, NULL);

    struct Node *new_node_ptr = (struct Node *)calloc (1, sizeof (struct Node));
    MY_ASSERT (new_node_ptr, "struct Node *new_node_ptr", NE_MEM, NULL);

    memmove (new_node_ptr, node_ptr, sizeof (struct Node));
    new_node_ptr->parent = parent;

    if (node_ptr->left_son)
        new_node_ptr->left_son = Copy_Tree (node_ptr->left_son, new_node_ptr);

    if (node_ptr->right_son)
        new_node_ptr->right_son = Copy_Tree (node_ptr->right_son, new_node_ptr);

    return new_node_ptr;
}

//*****************************************************************************************************************
//RELATED TO OPTIMIZATION OF A TREE
#define Change_Into_Num(num)                    \
do                                              \
{                                               \
    node_ptr->type = NUMBER;                    \
    node_ptr->value.number = num;               \
                                                \
    if (node_ptr->left_son)                     \
        Tree_Destructor (node_ptr->left_son);   \
                                                \
    if (node_ptr->right_son)                    \
        Tree_Destructor (node_ptr->right_son);  \
                                                \
    node_ptr->left_son  = NULL;                 \
    node_ptr->right_son = NULL;                 \
                                                \
    change_i++;                                 \
}                                               \
while (0)

#define Delete_Neutral_Elem                             \
do                                                      \
{                                                       \
    if (node_ptr->parent)                               \
    {                                                   \
        recursion->parent = node_ptr->parent;           \
                                                        \
        if (node_ptr->parent->left_son == node_ptr)     \
            node_ptr->parent->left_son  = recursion;    \
        else                                            \
            node_ptr->parent->right_son = recursion;    \
    }                                                   \
    else                                                \
        recursion->parent = NULL;                       \
                                                        \
    node_ptr->parent = NULL;                            \
                                                        \
    Tree_Destructor (node_ptr);                         \
                                                        \
    *node_ptr_ptr = recursion;                          \
                                                        \
    Simplify (node_ptr_ptr, var);                       \
                                                        \
    change_i++;                                         \
}                                                       \
while (0)

#define Fraction_Reduction(left_node, right_node)           \
do                                                          \
{                                                           \
    struct Node *l_node = Copy_Tree (left_node, node_ptr);  \
    struct Node *r_node = Copy_Tree (right_node, node_ptr); \
                                                            \
    Tree_Destructor (node_ptr->left_son);                   \
    Tree_Destructor (node_ptr->right_son);                  \
                                                            \
    node_ptr->left_son  = l_node;                           \
    node_ptr->right_son = r_node;                           \
}                                                           \
while (0)                                                   \

#define TP          node_ptr->type
#define VAL_S       node_ptr->value.symbol
#define VAL_N       node_ptr->value.number

#define LEFT        node_ptr->left_son
#define RIGHT       node_ptr->right_son
#define LEFT_TP     node_ptr->left_son->type
#define RIGHT_TP    node_ptr->right_son->type

#define LEFT_VAL_S  node_ptr->left_son->value.symbol
#define RIGHT_VAL_S node_ptr->right_son->value.symbol
#define LEFT_VAL_N  node_ptr->left_son->value.number
#define RIGHT_VAL_N node_ptr->right_son->value.number

#define LEFT_LEFT   node_ptr->left_son->left_son
#define LEFT_RIGHT  node_ptr->left_son->right_son
#define RIGHT_LEFT  node_ptr->right_son->left_son
#define RIGHT_RIGHT node_ptr->right_son->right_son

int Check_Fractions (struct Node *node_ptr)
{
    static int change_i = 0;

    if (TP == MATH_SIGN && VAL_S == '/')
    {
        if (LEFT_TP  == MATH_SIGN && LEFT_VAL_S  == '*' && RIGHT_TP == MATH_SIGN && RIGHT_VAL_S == '*')
        {
            change_i++;
            if      (Compare_Trees (LEFT_LEFT,  RIGHT_LEFT))
                Fraction_Reduction (LEFT_RIGHT, RIGHT_RIGHT);

            else if (Compare_Trees (LEFT_LEFT,  RIGHT_RIGHT))
                Fraction_Reduction (LEFT_RIGHT, RIGHT_LEFT);

            else if (Compare_Trees (LEFT_RIGHT, RIGHT_LEFT))
                Fraction_Reduction (LEFT_LEFT,  RIGHT_RIGHT);

            else if (Compare_Trees (LEFT_RIGHT, RIGHT_RIGHT))
                Fraction_Reduction (LEFT_LEFT,  RIGHT_LEFT);

            else
                change_i--;
        }
        else if (LEFT_TP  == MATH_SIGN && LEFT_VAL_S  == '/' && RIGHT_TP == MATH_SIGN && RIGHT_VAL_S == '/')
        {
            change_i++;
            if      (Compare_Trees (LEFT_RIGHT,  RIGHT_RIGHT))
                Fraction_Reduction (LEFT_LEFT,   RIGHT_LEFT);

            else if (Compare_Trees (LEFT_LEFT,   RIGHT_LEFT))
                Fraction_Reduction (RIGHT_RIGHT, LEFT_RIGHT);

            else
                change_i--;
        }
    }

    if (LEFT)
        Check_Fractions (LEFT);

    if (RIGHT)
        Check_Fractions (RIGHT);

    return NO_ERRORS;
}
#undef LEFT_LEFT
#undef LEFT_RIGHT
#undef RIGHT_LEFT
#undef RIGHT_RIGHT

int Simplify (struct Node **node_ptr_ptr, const char var)
{
    MY_ASSERT (node_ptr_ptr, "struct Node", NULL_PTR, ERROR);

    struct Node *node_ptr = *node_ptr_ptr;

    static int change_i = 0;
    int status = Check_Operands (node_ptr, var);

    if (TP == MATH_SIGN && status == L_NUM_R_NUM)
    {
        double temp = Calc_Expression (node_ptr);

        TP = NUMBER;
        VAL_N = temp;

        Tree_Destructor (LEFT);
        Tree_Destructor (RIGHT);

        LEFT  = NULL;
        RIGHT = NULL;

        change_i++;
    }
    else if ((TP == MATH_SIGN && VAL_S == '*' && (status == L_NUM_R_CONST || status == L_NUM_R_FUNC) && LEFT_VAL_N == 1.0) ||
             (TP == MATH_SIGN && VAL_S == '+' && (status == L_NUM_R_CONST || status == L_NUM_R_FUNC) && LEFT_VAL_N == 0.0))
    {
        struct Node *recursion = RIGHT;
        RIGHT = NULL;
        Delete_Neutral_Elem;
    }
    else if ((TP == MATH_SIGN && (VAL_S == '*' || VAL_S == '/') && (status == L_CONST_R_NUM || status == L_FUNC_R_NUM) && RIGHT_VAL_N == 1.0) ||
             (TP == MATH_SIGN && (VAL_S == '+' || VAL_S == '-') && (status == L_CONST_R_NUM || status == L_FUNC_R_NUM) && RIGHT_VAL_N == 0.0) ||
             (TP == MATH_SIGN &&  VAL_S == '^'                  && (status == L_CONST_R_NUM || status == L_FUNC_R_NUM) && RIGHT_VAL_N == 1.0))
    {
        struct Node *recursion = LEFT;
        LEFT = NULL;
        Delete_Neutral_Elem;
    }
    else if ((TP == MATH_SIGN && VAL_S == '*' && (((status == L_CONST_R_NUM || status == L_FUNC_R_NUM) && RIGHT_VAL_N == 0.0) ||
                                                  ((status == L_NUM_R_CONST || status == L_NUM_R_FUNC) && LEFT_VAL_N  == 0.0))) ||
             (TP == MATH_SIGN && VAL_S == '/' &&   (status == L_NUM_R_CONST || status == L_NUM_R_FUNC) && LEFT_VAL_N  == 0.0))
        Change_Into_Num (0.0);

    else if (TP == MATH_SIGN && VAL_S == '/' && (status == L_CONST_R_NUM || status == L_FUNC_R_NUM) && RIGHT_VAL_N == 0.0)
        MY_ASSERT (false, "Right operand of expression", DIV_BY_NULL, ERROR);

    else if ((TP == MATH_SIGN && VAL_S == '^' && ((status == L_NUM_R_CONST || status == L_NUM_R_FUNC) && LEFT_VAL_N  == 1.0)) ||
             (TP == MATH_SIGN && VAL_S == '^' && ((status == L_CONST_R_NUM || status == L_FUNC_R_NUM) && RIGHT_VAL_N == 0.0)))
    {
        if (LEFT_VAL_N == 0.0 && RIGHT_VAL_N == 0.0)
            MY_ASSERT (false, "Operands of expression", NULL_POW_NULL, ERROR);
        else
            Change_Into_Num (1.0);
    }

    else if (TP == FUNCTION && strcmp (node_ptr->value.function, Functions_Data_Base[Ln]) == 0 &&
             LEFT_TP == MATH_CONST && strcmp (node_ptr->left_son->value.math_const, "e") == 0)
        Change_Into_Num (1.0);

    else
    {
        if (LEFT)
            Simplify (&LEFT, var);

        if (RIGHT)
            Simplify (&RIGHT, var);
    }

    return change_i;
}

double Calc_Expression (const struct Node *node_ptr)
{
    switch (VAL_S)
    {
        case '+':
            return (LEFT_VAL_N + RIGHT_VAL_N);

        case '-':
            return (LEFT_VAL_N - RIGHT_VAL_N);

        case '*':
            return (LEFT_VAL_N * RIGHT_VAL_N);

        case '/':
            if (RIGHT_VAL_N == 0.0)
                MY_ASSERT (false, "Right operand of expression", DIV_BY_NULL, (double)ERROR);

            return (LEFT_VAL_N / RIGHT_VAL_N);

        case '^':
            if (LEFT_VAL_N == 0.0 && RIGHT_VAL_N == 0.0)
                MY_ASSERT (false, "Operands of expression", NULL_POW_NULL, (double)ERROR);

            return (pow (LEFT_VAL_N, RIGHT_VAL_N));

        default: MY_ASSERT (false, "node_ptr->value.sumbol", UNEXP_VAL, (double)ERROR);
    }
}

int Compare_Trees (const struct Node *tree_1, const struct Node *tree_2)
{
    if (tree_1->type != tree_2->type)
        return 0;

    switch (tree_1->type)
    {
        case NUMBER:
            if (tree_1->value.number != tree_2->value.number)
                return 0;
            break;
        case VARIABLE:
        case MATH_SIGN:
            if (tree_1->value.symbol != tree_2->value.symbol)
                return 0;
            break;
        case MATH_CONST:
            if (strcmp (tree_1->value.math_const, tree_2->value.math_const) != 0)
                return 0;
            break;
        case FUNCTION:
            if (strcmp (tree_1->value.function, tree_2->value.function) != 0)
                return 0;
            break;
        default: MY_ASSERT (false, "tree_1->type", UNEXP_VAL, ERROR);
    }

    if (tree_1->left_son && tree_2->left_son)
        if (Compare_Trees (tree_1->left_son, tree_2->left_son) == 0)
            return 0;

    if (tree_1->right_son && tree_2->right_son)
        if (Compare_Trees (tree_1->right_son, tree_2->right_son) == 0)
            return 0;

    return 1;
}
//*****************************************************************************************************************

//*****************************************************************************************************************
//RELATED OT DUMP IN TEX

int Dump_In_Tex (const struct Node *orig_tree, struct Node **tree_arr, const char *vars_arr, const int n_vars, const char *text_file_name)
{
    MY_ASSERT (tree_arr,       "const struct Node **tree_arr", NULL_PTR, ERROR);
    MY_ASSERT (vars_arr,       "const char *vars_arr",         NULL_PTR, ERROR);
    MY_ASSERT (text_file_name, "const char *text_file_name",   NULL_PTR, ERROR);

    FILE *tex_file = Open_File (text_file_name, "wb");

    fprintf (tex_file, "\\(f(");
    for (int i = 0; i < n_vars - 1; i++)
        fprintf (tex_file, "%c, ", vars_arr[i]);
    fprintf (tex_file, "%c) = ", vars_arr[n_vars - 1]);

    Formula_Dump (orig_tree, tex_file);
    fprintf (tex_file, "\\)");
    fprintf (tex_file, "\\\\\n\\\\\n");

    for (int i = 0; i < n_vars; i++)
    {
        if (n_vars == 1)
            fprintf (tex_file, "\\(f'(%c) = ", vars_arr[i]);
        else
            fprintf (tex_file, "\\( \\frac{\\partial f}{\\partial %c} = ", vars_arr[i]);

        Formula_Dump (tree_arr[i], tex_file);
        fprintf (tex_file, "\\)");
        fprintf (tex_file, "\\\\\n\\\\\n");
    }

    Close_File (tex_file, text_file_name);

    return NO_ERRORS;
}

#define PRINT(l_brace, side, r_brace)       \
do                                          \
{                                           \
    fprintf (tex_file, "%s", l_brace);      \
    Formula_Dump (side, tex_file);          \
    fprintf (tex_file, "%s ", r_brace);     \
}                                           \
while (0)

#define PRINT_MULT_OPERAND(side, side_tp, side_val_n, side_val_s)                                       \
do                                                                                                      \
{                                                                                                       \
    switch (side_tp)                                                                                    \
    {                                                                                                   \
        case NUMBER:                                                                                    \
            if (side_val_n < 0)                                                                         \
                PRINT ("(", side, ")");                                                                 \
            else                                                                                        \
                Formula_Dump (side, tex_file);                                                          \
            break;                                                                                      \
        case MATH_CONST: case VARIABLE: case FUNCTION:                                                  \
            Formula_Dump (side, tex_file);                                                              \
            break;                                                                                      \
        default:                                                                                        \
            if (side_tp == MATH_SIGN && (side_val_s == '*' || side_val_s == '^' || side_val_s == '/'))  \
                Formula_Dump (side, tex_file);                                                          \
            else                                                                                        \
                PRINT ("(", side, ")");                                                                 \
    }                                                                                                   \
}                                                                                                       \
while (0)

#define PRINT_POW_OPERAND(side, side_tp, side_val_n, other_side_val_s)  \
do                                                                      \
{                                                                       \
    fprintf (tex_file, "{");                                            \
    switch (side_tp)                                                    \
    {                                                                   \
        case NUMBER:                                                    \
            if (side_val_n < 0)                                         \
                PRINT ("(", side, ")");                                 \
            else                                                        \
                Formula_Dump (side, tex_file);                          \
            break;                                                      \
        case MATH_CONST: case VARIABLE: case FUNCTION:                  \
            Formula_Dump (side, tex_file);                              \
            break;                                                      \
        default:                                                        \
            if (side_tp == MATH_SIGN && other_side_val_s == '^')        \
                Formula_Dump (side, tex_file);                          \
            else                                                        \
                PRINT ("(", side, ")");                                 \
    }                                                                   \
    fprintf (tex_file, "}");                                            \
}                                                                       \
while (0)

int Formula_Dump (const struct Node *node_ptr, FILE *tex_file)
{
    switch (TP)
    {
        case NUMBER:
            if (VAL_N == 0.5)
                fprintf (tex_file, "\\frac{1}{2}");
            else if (VAL_N == -0.5)
                fprintf (tex_file, "\\frac{-1}{2}");
            else
                fprintf (tex_file, "%.f", VAL_N);
            break;

        case MATH_CONST:
            if (strcmp (node_ptr->value.math_const, "e") == 0)
                fprintf (tex_file, "e");
            else
                fprintf (tex_file, "\\pi");
            break;

        case VARIABLE:
            fprintf (tex_file, "%c", VAL_S);
            break;

        case FUNCTION:
            if (!strcmp (node_ptr->value.function, Functions_Data_Base[Sqrt]))
            {
                fprintf (tex_file, "\\sqrt");
                PRINT ("{", LEFT, "}");
            }
            else
            {
                fprintf (tex_file, "%s ", node_ptr->value.function);
                PRINT ("(", LEFT, ")");
            }

            break;

        case MATH_SIGN:
            switch (VAL_S)
            {
                case '+':
                case '-':
                    Formula_Dump (LEFT, tex_file);
                    fprintf (tex_file, " %c ", VAL_S);
                    Formula_Dump (RIGHT, tex_file);
                    break;

                case '*':
                    PRINT_MULT_OPERAND (LEFT, LEFT_TP, LEFT_VAL_N, LEFT_VAL_S);
                    fprintf (tex_file, "\\cdot ");
                    PRINT_MULT_OPERAND (RIGHT, RIGHT_TP, RIGHT_VAL_N, RIGHT_VAL_S);
                    break;

                case '^':
                    PRINT_POW_OPERAND (LEFT, LEFT_TP, LEFT_VAL_N, RIGHT_VAL_S);
                    fprintf (tex_file, "^");
                    PRINT_POW_OPERAND (RIGHT, RIGHT_TP, RIGHT_VAL_N, LEFT_VAL_S);
                    break;

                case '/':
                    fprintf (tex_file, "\\frac");
                    PRINT ("{", LEFT,  "}");
                    PRINT ("{", RIGHT, "}");
                    break;

                default: MY_ASSERT (false, "node_ptr->value.symbol", UNEXP_VAL, ERROR);
            }
            break;

        default: MY_ASSERT (false, "node_ptr->type", UNEXP_VAL, ERROR);
    }

    return NO_ERRORS;
}

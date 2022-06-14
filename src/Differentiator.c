#include "../include/Differentiator.h"
#include "My_Lib.h"

#include <assert.h>
#define OPTIMIZATION

// ========================================== STATIC PROTOTYPES ========================================== //

static struct Node **Forest_Ctor   (const int n_vars);
static int           Forest_Dtor   (struct Node **forest, const int n_vars);
static int           Dump_One_Tree (struct Node *root, const char *var, const int n_vars);
static char        **Find_Vars     (struct Node *node_ptr, int *n_vars);
static void          Count_Vars    (struct Node* node_ptr, int *n_vars);
static void          Add_Vars      (struct Node* node_ptr, char **vars_arr, int *var_i);
static bool          Find_Name     (char **vars_arr, const char *var, const int n_vars);

static int Diff_One_Var (const struct Node *node_ptr, const char *var, struct Node *new_node_ptr);

static struct Node *Differentiate_Inside (struct Node *what_diff, struct Node *parent, const char *var);
static struct Node *Create_Node_         (enum Types node_type, struct Node *parent, int parmN, ...);
static struct Node *Copy_Tree            (const struct Node *node_ptr, struct Node *parent);
static int          Check_Operands       (const struct Node *node_ptr, const char *var);

static int Diff_Sum_Sub (const struct Node *node_ptr, const char *var, struct Node *new_node_ptr, const enum Types func_i);
static int Diff_Mult    (const struct Node *node_ptr, const char *var, struct Node *new_node_ptr);
static int Diff_Div     (const struct Node *node_ptr, const char *var, struct Node *new_node_ptr);
static int Diff_Sqrt    (const struct Node *node_ptr, const char *var, struct Node *new_node_ptr);
static int Diff_Pow     (const struct Node *node_ptr, const char *var, struct Node *new_node_ptr);
static int Diff_Exp     (const struct Node *node_ptr, const char *var, struct Node *new_node_ptr);
static int Diff_Pow_Exp (const struct Node *node_ptr, const char *var, struct Node *new_node_ptr);
static int Diff_Ln      (const struct Node *node_ptr, const char *var, struct Node *new_node_ptr);

static int Diff_Cos           (const struct Node *node_ptr, const char *var, struct Node *new_node_ptr);
static int Diff_Sin_Sinh_Cosh (const struct Node *node_ptr, const char *var, struct Node *new_node_ptr, const enum Types func_i);
static int Diff_Tan_Tanh      (const struct Node *node_ptr, const char *var, struct Node *new_node_ptr, const enum Types func_i);
static int Diff_Cot_Coth      (const struct Node *node_ptr, const char *var, struct Node *new_node_ptr, const enum Types func_i);
static int Diff_Arcsin_Arccos (const struct Node *node_ptr, const char *var, struct Node *new_node_ptr, const enum Types func_i);
static int Diff_Arctan_Arccot (const struct Node *node_ptr, const char *var, struct Node *new_node_ptr, const enum Types func_i);

#ifdef OPTIMIZATION
static int Simplify_Function  (struct Node **root, const char *var);
static int Check_Fractions    (struct Node *node_ptr);
static int Fraction_Reduction (struct Node *root, struct Node *left_node, struct Node *right_node);
static int Simplify           (struct Node **node_ptr_ptr, const char *var);
static double Calc_Expression (const struct Node *node_ptr);
static int Compare_Trees      (const struct Node *tree_1, const struct Node *tree_2);
static int Compare_Double     (const double first, const double second);
#endif

// ======================================================================================================= //

// ========================================== GENERAL FUNCTIONS ========================================== //

int Differentiator (struct Node *root)
{
    int n_vars = 0;
    char **vars_arr = Find_Vars (root, &n_vars);

    struct Node **forest = Forest_Ctor (n_vars);

    #ifdef OPTIMIZATION
    for (int var_i = 0; var_i < n_vars; var_i++)
    {
        int SF_status = Simplify_Function (&root, vars_arr[var_i]);
        MY_ASSERT (SF_status != ERROR, "Simplify_Function ()", FUNC_ERROR, ERROR);
    }
    #endif

    for (int var_i = 0; var_i < n_vars; var_i++)
    {
        int DOV_status = Diff_One_Var (root, vars_arr[var_i], forest[var_i]);
        MY_ASSERT (DOV_status != ERROR, "Diff_One_Var ()", FUNC_ERROR, ERROR);

        #ifdef OPTIMIZATION
        int SF_status = Simplify_Function (&root, vars_arr[var_i]);
        MY_ASSERT (SF_status != ERROR, "Simplify_Function ()", FUNC_ERROR, ERROR);
        #endif

        int DOT_status = Dump_One_Tree (forest[var_i], vars_arr[var_i], n_vars);
        MY_ASSERT (DOT_status != ERROR, "Dump_One_Tree ()", FUNC_ERROR, ERROR);
    }

    #if 0
    int DIT_status = Dump_In_Tex (root, forest, vars_arr, n_vars);
    MY_ASSERT (DIT_status != ERROR, "Dump_In_Tex ()", FUNC_ERROR, ERROR);
    #endif

    free (vars_arr);
    Forest_Dtor (forest, n_vars);

    return NO_ERRORS;
}

static struct Node **Forest_Ctor (const int n_vars)
{
    MY_ASSERT (n_vars > 0, "const int n_vars", POS_VAL, NULL);
    
    struct Node **forest = (struct Node **)calloc (n_vars, sizeof (struct Node *));
    MY_ASSERT (forest, "struct Node **forest", NE_MEM, NULL);

    for (int var_i = 0; var_i < n_vars; var_i++)
    {
        forest[var_i] = (struct Node *)calloc (1, sizeof  (struct Node));
        MY_ASSERT (forest[var_i], "tree_arr[var_i]", NE_MEM, NULL);
    }

    return forest;
}

static int Forest_Dtor (struct Node **forest, const int n_vars)
{
    MY_ASSERT (forest,     "struct Node **forest", NULL_PTR, ERROR);
    MY_ASSERT (n_vars > 0, "const int n_vars",     POS_VAL,  ERROR);
    
    for (int i = 0; i < n_vars; i++)
    {
        MY_ASSERT (Tree_Destructor (forest[i]), "Tree_Destructor ()", FUNC_ERROR, ERROR);
    } 

    free (forest);

    return NO_ERRORS;
}

static int Dump_One_Tree (struct Node *root, const char *var, const int n_vars)
{
    MY_ASSERT (root,       "struct Node *root", NULL_PTR, ERROR);
    MY_ASSERT (var,        "const char *var",   NULL_PTR, ERROR);
    MY_ASSERT (n_vars > 0, "const int n_vars",  POS_VAL,  ERROR);
    
    const size_t buff_size = sizeof ("./Output/Partial_Derivative_Of_") + strlen (var) + sizeof (".dot");

    char *buffer_1 = (char *)calloc (buff_size, sizeof (char));
    char *buffer_2 = (char *)calloc (buff_size, sizeof (char));

    if (n_vars > 1)
    {
        sprintf (buffer_1, "./Output/Partial_Derivative_Of_%s.dot", var);
        sprintf (buffer_2, "./Output/Partial_Derivative_Of_%s.png", var);
    }
    else
    {
        sprintf (buffer_1, "./Output/Derivative.dot");
        sprintf (buffer_2, "./Output/Derivative.png");
    }

    Tree_Dump (root, buffer_1, buffer_2, var);
    free (buffer_1);
    free (buffer_2);

    return NO_ERRORS;
}

static char **Find_Vars (struct Node *node_ptr, int *n_vars)
{
    MY_ASSERT (node_ptr, "struct Node *node_ptr", NULL_PTR, NULL);
    MY_ASSERT (n_vars,   "int *n_vars",           NULL_PTR, NULL);

    Count_Vars (node_ptr, n_vars);
    
    char **vars_arr = (char **)calloc (*n_vars, sizeof (char *));
    MY_ASSERT (vars_arr, "char *vars_arr", NE_MEM, NULL);

    int var_i = 0;
    Add_Vars (node_ptr, vars_arr, &var_i);

    return vars_arr;
}

static void Count_Vars (struct Node* node_ptr, int *n_vars)
{
    if (node_ptr->type == VARIABLE)
        (*n_vars)++;

    if (node_ptr->left_son)
        Count_Vars (node_ptr->left_son, n_vars);

    if (node_ptr->right_son)
        Count_Vars (node_ptr->right_son, n_vars);
}

static void Add_Vars (struct Node* node_ptr, char **vars_arr, int *var_i)
{
    if (node_ptr->type == VARIABLE && Find_Name (vars_arr, node_ptr->value.str, *var_i) == false)
    {
        vars_arr[*var_i] = node_ptr->value.str;
        (*var_i)++;
    }

    if (node_ptr->left_son)
        Add_Vars (node_ptr->left_son, vars_arr, var_i);

    if (node_ptr->right_son)
        Add_Vars (node_ptr->right_son, vars_arr, var_i);
}

static bool Find_Name (char **vars_arr, const char *var, const int n_vars)
{   
    for (int var_i = 0; var_i < n_vars; var_i++)
        if (strcmp (vars_arr[var_i], var) == 0)
            return true;

    return false;
}
// ======================================================================================================= //

// =========================================== DIFFERENTIATION =========================================== //

enum Operands
{
    L_NUM_R_NUM,
    L_NUM_R_CONST,
    L_CONST_R_NUM,
    L_CONST_R_CONST,
    L_NUM_R_FUNC,
    L_FUNC_R_NUM,
    L_CONST_R_FUNC,
    L_FUNC_R_CONST,
    L_FUNC_R_FUNC
};

static int Diff_One_Var (const struct Node *node_ptr, const char *var, struct Node *new_node_ptr)
{
    MY_ASSERT (node_ptr,     "const struct Node *node_ptr", NULL_PTR, ERROR);
    MY_ASSERT (var,          "cosnt char *var",             NULL_PTR, ERROR);
    MY_ASSERT (new_node_ptr, "struct Node *new_node_ptr",   NULL_PTR, ERROR);

    switch (node_ptr->type)
    {
        case SQRT:
            Diff_Sqrt (node_ptr, var, new_node_ptr);
            break;

        case LN:
            Diff_Ln (node_ptr, var, new_node_ptr);
            break;

        case SIN:
        case SINH:
        case COSH:
            Diff_Sin_Sinh_Cosh (node_ptr, var, new_node_ptr, node_ptr->type);
            break;

        case COS:
            Diff_Cos (node_ptr, var, new_node_ptr);
            break;  

        case TAN:
        case TANH:
            Diff_Tan_Tanh (node_ptr, var, new_node_ptr, node_ptr->type);
            break;

        case COT:
        case COTH:
            Diff_Cot_Coth (node_ptr, var, new_node_ptr, node_ptr->type);
            break;

        case ARCSIN:
        case ARCCOS:
            Diff_Arcsin_Arccos (node_ptr, var, new_node_ptr, node_ptr->type);
            break;

        case ARCTAN:
        case ARCCOT:
            Diff_Arctan_Arccot (node_ptr, var, new_node_ptr, node_ptr->type);
            break;

        case VARIABLE:

            new_node_ptr->type = NUMBER;

            if (strcmp (node_ptr->value.str, var) == 0)
                new_node_ptr->value.num = 1.0;
            else
                new_node_ptr->value.num = 0.0;

            new_node_ptr->left_son  = NULL;
            new_node_ptr->right_son = NULL;

            break;

        case NUMBER:
        case PI:
        case E_NUM:
            new_node_ptr->type      = NUMBER;
            new_node_ptr->value.num = 0.0;
            new_node_ptr->left_son  = NULL;
            new_node_ptr->right_son = NULL;

            break;

        case PLUS:
        case MINUS:
            Diff_Sum_Sub (node_ptr, var, new_node_ptr, node_ptr->type);
            break;

        case MULT:
            Diff_Mult (node_ptr, var, new_node_ptr);
            break;

        case DIV:
            Diff_Div (node_ptr, var, new_node_ptr);
            break;

        case POW:
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
                    new_node_ptr->value.num = 0.0;

                    new_node_ptr->left_son  = NULL;
                    new_node_ptr->right_son = NULL;
                    break;

                default:
                    MY_ASSERT (false, "Check_Operands ()", FUNC_ERROR, ERROR);
            }
            break;

        default: 
            MY_ASSERT (false, "node_ptr->type", UNEXP_VAL, ERROR);
    }

    return NO_ERRORS;
}

static int Diff_Sum_Sub (const struct Node *node_ptr, const char *var, struct Node *new_node_ptr, const enum Types func_i)
{
    MY_ASSERT (node_ptr,     "const struct Node *node_ptr", NULL_PTR, ERROR);
    MY_ASSERT (new_node_ptr, "struct Node *new_node_ptr",   NULL_PTR, ERROR);

    switch (func_i)
    {
        case PLUS:
        case MINUS:
            break;
        default: 
            MY_ASSERT (false, "enum Types func_i", UNEXP_VAL, ERROR);
    }
    
    new_node_ptr->type = func_i;

    new_node_ptr->left_son = Differentiate_Inside (node_ptr->left_son, new_node_ptr, var);
    MY_ASSERT (new_node_ptr->left_son, "Differentiate_Inside ()", FUNC_ERROR, ERROR);

    new_node_ptr->right_son = Differentiate_Inside (node_ptr->right_son, new_node_ptr, var);
    MY_ASSERT (new_node_ptr->right_son, "Differentiate_Inside ()", FUNC_ERROR, ERROR);

    return NO_ERRORS;
}

#define Create_Num_Node(val, parent) Create_Node_ (NUMBER, parent, 1, val)
#define Create_Node(node_type, parent) Create_Node_ (node_type, parent, 0)

static int Diff_Mult (const struct Node *node_ptr, const char *var, struct Node *new_node_ptr)
{
    MY_ASSERT (node_ptr,     "const struct Node *node_ptr", NULL_PTR, ERROR);
    MY_ASSERT (new_node_ptr, "struct Node *new_node_ptr",   NULL_PTR, ERROR);

    new_node_ptr->type = PLUS;

    new_node_ptr->left_son = Create_Node (MULT, new_node_ptr);
    MY_ASSERT (new_node_ptr->left_son, "Create_Node ()", FUNC_ERROR, ERROR);

    new_node_ptr->left_son->right_son = Copy_Tree (node_ptr->right_son, new_node_ptr->left_son);
    MY_ASSERT (new_node_ptr->left_son->right_son, "Copy_Tree ()", FUNC_ERROR, ERROR);

    new_node_ptr->left_son->left_son = Differentiate_Inside (node_ptr->left_son, new_node_ptr->left_son, var);
    MY_ASSERT (new_node_ptr->left_son->left_son, "Differentiate_Inside ()", FUNC_ERROR, ERROR);

    new_node_ptr->right_son = Create_Node (MULT, new_node_ptr);
    MY_ASSERT (new_node_ptr->right_son, "Create_Node ()", FUNC_ERROR, ERROR);

    new_node_ptr->right_son->left_son = Copy_Tree (node_ptr->left_son, new_node_ptr->right_son);
    MY_ASSERT (new_node_ptr->right_son->left_son, "Copy_Tree ()", FUNC_ERROR, ERROR);

    new_node_ptr->right_son->right_son = Differentiate_Inside (node_ptr->right_son, new_node_ptr->right_son, var);
    MY_ASSERT (new_node_ptr->right_son->right_son, "Differentiate_Inside ()", FUNC_ERROR, ERROR);

    return NO_ERRORS;
}

static int Diff_Div (const struct Node *node_ptr, const char *var, struct Node *new_node_ptr)
{
    MY_ASSERT (node_ptr,     "const struct Node *node_ptr", NULL_PTR, ERROR);
    MY_ASSERT (new_node_ptr, "struct Node *new_node_ptr",   NULL_PTR, ERROR);

    new_node_ptr->type = DIV;

    new_node_ptr->left_son = Create_Node (MINUS, new_node_ptr);
    MY_ASSERT (new_node_ptr->left_son, "Create_Node ()", FUNC_ERROR, ERROR);

    new_node_ptr->left_son->left_son = Create_Node (MULT, new_node_ptr->left_son);
    MY_ASSERT (new_node_ptr->left_son->left_son, "Create_Node ()", FUNC_ERROR, ERROR);

    new_node_ptr->left_son->left_son->right_son = Copy_Tree (node_ptr->right_son, new_node_ptr->left_son->left_son);
    MY_ASSERT (new_node_ptr->left_son->left_son->right_son, "Copy_Tree ()", FUNC_ERROR, ERROR);

    new_node_ptr->left_son->left_son->left_son  = Differentiate_Inside (node_ptr->left_son, new_node_ptr->left_son->left_son, var);
    MY_ASSERT (new_node_ptr->left_son->left_son->left_son, "Differentiate_Inside ()", FUNC_ERROR, ERROR);

    new_node_ptr->left_son->right_son = Create_Node (MULT, new_node_ptr->left_son);
    MY_ASSERT (new_node_ptr->left_son->right_son, "Create_Node ()", FUNC_ERROR, ERROR);

    new_node_ptr->left_son->right_son->left_son = Copy_Tree (node_ptr->left_son, new_node_ptr->left_son->right_son);
    MY_ASSERT (new_node_ptr->left_son->right_son->left_son, "Copy_Tree ()", FUNC_ERROR, ERROR);

    new_node_ptr->left_son->right_son->right_son = Differentiate_Inside (node_ptr->right_son, new_node_ptr->left_son->right_son, var);
    MY_ASSERT (new_node_ptr->left_son->right_son->right_son, "Differentiate_Inside ()", FUNC_ERROR, ERROR);

    new_node_ptr->right_son = Create_Node (POW, new_node_ptr);
    MY_ASSERT (new_node_ptr->right_son, "Create_Node ()", FUNC_ERROR, ERROR);

    new_node_ptr->right_son->left_son  = Copy_Tree (node_ptr->right_son, new_node_ptr->right_son);
    MY_ASSERT (new_node_ptr->right_son->left_son, "Copy_Tree ()", FUNC_ERROR, ERROR);

    new_node_ptr->right_son->right_son = Create_Num_Node (2.0, new_node_ptr->right_son);
    MY_ASSERT (new_node_ptr->right_son->right_son, "Create_Num_Node ()", FUNC_ERROR, ERROR);

    return NO_ERRORS;
}

static int Diff_Sqrt (const struct Node *node_ptr, const char *var, struct Node *new_node_ptr)
{
    MY_ASSERT (node_ptr,     "const struct Node *node_ptr", NULL_PTR, ERROR);
    MY_ASSERT (new_node_ptr, "struct Node *new_node_ptr",   NULL_PTR, ERROR);

    new_node_ptr->type = DIV;

    new_node_ptr->left_son = Differentiate_Inside (node_ptr->left_son, new_node_ptr, var);
    MY_ASSERT (new_node_ptr->left_son, "Differentiate_Inside ()", FUNC_ERROR, ERROR);

    new_node_ptr->right_son = Create_Node (MULT, new_node_ptr);
    MY_ASSERT (new_node_ptr->right_son, "Create_Node ()", FUNC_ERROR, ERROR);
    
    new_node_ptr->right_son->left_son = Create_Num_Node (2.0, new_node_ptr->right_son);
    MY_ASSERT (new_node_ptr->right_son->left_son, "Create_Num_Node ()", FUNC_ERROR, ERROR);

    new_node_ptr->right_son->right_son = Copy_Tree (node_ptr, new_node_ptr->right_son);
    MY_ASSERT (new_node_ptr->right_son->right_son, "Copy_Tree ()", FUNC_ERROR, ERROR);

    return NO_ERRORS;
}

static int Diff_Pow (const struct Node *node_ptr, const char *var, struct Node *new_node_ptr)
{
    MY_ASSERT (node_ptr,     "const struct Node *node_ptr", NULL_PTR, ERROR);
    MY_ASSERT (new_node_ptr, "struct Node *new_node_ptr",   NULL_PTR, ERROR);

    new_node_ptr->type = MULT;

    new_node_ptr->left_son = Create_Node (MULT, new_node_ptr);
    MY_ASSERT (new_node_ptr->left_son, "Create_Node ()", FUNC_ERROR, ERROR);

    new_node_ptr->left_son->left_son = Copy_Tree (node_ptr->right_son, new_node_ptr->left_son);
    MY_ASSERT (new_node_ptr->left_son->left_son, "Copy_Tree ()", FUNC_ERROR, ERROR);

    new_node_ptr->left_son->right_son = Create_Node (POW, new_node_ptr->left_son);
    MY_ASSERT (new_node_ptr->left_son->right_son, "Create_Node ()", FUNC_ERROR, ERROR);

    new_node_ptr->left_son->right_son->left_son = Copy_Tree (node_ptr->left_son, new_node_ptr->left_son->right_son);
    MY_ASSERT (new_node_ptr->left_son->right_son->left_son, "Copy_Tree ()", FUNC_ERROR, ERROR);

    new_node_ptr->left_son->right_son->right_son = Create_Node (MINUS, new_node_ptr->left_son->right_son);
    MY_ASSERT (new_node_ptr->left_son->right_son->right_son, "Create_Node ()", FUNC_ERROR, ERROR);

    new_node_ptr->left_son->right_son->right_son->left_son = Copy_Tree (node_ptr->right_son, new_node_ptr->left_son->right_son->right_son);
    MY_ASSERT (new_node_ptr->left_son->right_son->right_son->left_son, "Copy_Tree ()", FUNC_ERROR, ERROR);
    
    new_node_ptr->left_son->right_son->right_son->right_son = Create_Num_Node (1.0, new_node_ptr->left_son->right_son->right_son);
    MY_ASSERT (new_node_ptr->left_son->right_son->right_son->right_son, "Create_Num_Node ()", FUNC_ERROR, ERROR);

    new_node_ptr->right_son = Differentiate_Inside (node_ptr->left_son, new_node_ptr, var);
    MY_ASSERT (new_node_ptr->right_son, "Differentiate_Inside ()", FUNC_ERROR, ERROR);

    return NO_ERRORS;
}

static int Diff_Exp (const struct Node *node_ptr, const char *var, struct Node *new_node_ptr)
{
    MY_ASSERT (node_ptr,     "const struct Node *node_ptr", NULL_PTR, ERROR);
    MY_ASSERT (new_node_ptr, "struct Node *new_node_ptr",   NULL_PTR, ERROR);

    new_node_ptr->type = MULT;

    new_node_ptr->right_son = Differentiate_Inside (node_ptr->right_son, new_node_ptr, var);
    MY_ASSERT (new_node_ptr->right_son, "Differentiate_Inside ()", FUNC_ERROR, ERROR);

    new_node_ptr->left_son = Create_Node (MULT, new_node_ptr);
    MY_ASSERT (new_node_ptr->left_son, "Create_Node ()", FUNC_ERROR, ERROR);

    new_node_ptr->left_son->right_son = Create_Node (LN, new_node_ptr->left_son);
    MY_ASSERT (new_node_ptr->left_son->right_son, "Create_Node ()", FUNC_ERROR, ERROR);

    new_node_ptr->left_son->right_son->left_son = Copy_Tree (node_ptr->left_son, new_node_ptr->left_son->right_son);
    MY_ASSERT (new_node_ptr->left_son->right_son->left_son, "Copy_Tree ()", FUNC_ERROR, ERROR);

    new_node_ptr->left_son->left_son = Create_Node (POW, new_node_ptr->left_son);
    MY_ASSERT (new_node_ptr->left_son->left_son, "Create_Node ()", FUNC_ERROR, ERROR);

    new_node_ptr->left_son->left_son->left_son = Copy_Tree (node_ptr->left_son, new_node_ptr->left_son->left_son);
    MY_ASSERT (new_node_ptr->left_son->left_son->left_son, "Copy_Tree ()", FUNC_ERROR, ERROR);

    new_node_ptr->left_son->left_son->right_son = Copy_Tree (node_ptr->right_son, new_node_ptr->left_son->left_son);
    MY_ASSERT (new_node_ptr->left_son->left_son->right_son, "Copy_Tree ()", FUNC_ERROR, ERROR);

    return NO_ERRORS;
}

static int Diff_Pow_Exp (const struct Node *node_ptr, const char *var, struct Node *new_node_ptr)
{
    MY_ASSERT (node_ptr,     "const struct Node *node_ptr", NULL_PTR, ERROR);
    MY_ASSERT (new_node_ptr, "struct Node *new_node_ptr",   NULL_PTR, ERROR);

    new_node_ptr->type = MULT;

    new_node_ptr->left_son = Create_Node (POW, new_node_ptr);
    MY_ASSERT (new_node_ptr->left_son, "Create_Node ()", FUNC_ERROR, ERROR);

    new_node_ptr->left_son->left_son  = Copy_Tree (node_ptr->left_son, new_node_ptr->left_son);
    MY_ASSERT (new_node_ptr->left_son->left_son, "Copy_Tree ()", FUNC_ERROR, ERROR);

    new_node_ptr->left_son->right_son = Copy_Tree (node_ptr->right_son, new_node_ptr->left_son);
    MY_ASSERT (new_node_ptr->left_son->right_son, "Copy_Tree ()", FUNC_ERROR, ERROR);

    new_node_ptr->right_son = Create_Node (PLUS, new_node_ptr);
    MY_ASSERT (new_node_ptr->right_son, "Create_Node ()", FUNC_ERROR, ERROR);

    new_node_ptr->right_son->left_son = Create_Node (MULT, new_node_ptr->right_son);
    MY_ASSERT (new_node_ptr->right_son->left_son, "Create_Node ()", FUNC_ERROR, ERROR);

    new_node_ptr->right_son->right_son = Create_Node (MULT, new_node_ptr->right_son);
    MY_ASSERT (new_node_ptr->right_son->right_son, "Create_Node ()", FUNC_ERROR, ERROR);

    new_node_ptr->right_son->left_son->left_son = Differentiate_Inside (node_ptr->right_son, new_node_ptr->right_son->left_son, var);
    MY_ASSERT (new_node_ptr->left_son->left_son, "Differentiate_Inside ()", FUNC_ERROR, ERROR);

    new_node_ptr->right_son->left_son->right_son = Create_Node (LN, new_node_ptr->right_son->left_son);
    MY_ASSERT (new_node_ptr->right_son->left_son->right_son, "Create_Node ()", FUNC_ERROR, ERROR);

    new_node_ptr->right_son->left_son->right_son->left_son = Copy_Tree (node_ptr->left_son, new_node_ptr->right_son->left_son->right_son);
    MY_ASSERT (new_node_ptr->right_son->left_son->right_son, "Copy_Tree ()", FUNC_ERROR, ERROR);

    new_node_ptr->right_son->right_son->left_son  = Copy_Tree (node_ptr->right_son, new_node_ptr->right_son->right_son);
    MY_ASSERT (new_node_ptr->right_son->right_son->left_son, "Copy_Tree ()", FUNC_ERROR, ERROR);

    new_node_ptr->right_son->right_son->right_son = Create_Node (DIV, new_node_ptr->right_son->right_son);
    MY_ASSERT (new_node_ptr->right_son->right_son->right_son, "Create_Node ()", FUNC_ERROR, ERROR);

    new_node_ptr->right_son->right_son->right_son->right_son = Copy_Tree (node_ptr->left_son, new_node_ptr->right_son->right_son->right_son);
    MY_ASSERT (new_node_ptr->right_son->right_son->right_son->right_son, "Copy_Tree ()", FUNC_ERROR, ERROR);

    new_node_ptr->right_son->right_son->right_son->left_son = Differentiate_Inside (node_ptr->left_son, new_node_ptr->right_son->right_son->right_son, var);
    MY_ASSERT (new_node_ptr->right_son->right_son->right_son->left_son, "Differentiate_Inside ()", FUNC_ERROR, ERROR);

    return NO_ERRORS;
}

static int Diff_Ln (const struct Node *node_ptr, const char *var, struct Node *new_node_ptr)
{
    MY_ASSERT (node_ptr,     "const struct Node *node_ptr", NULL_PTR, ERROR);
    MY_ASSERT (new_node_ptr, "struct Node *new_node_ptr",   NULL_PTR, ERROR);

    new_node_ptr->type = DIV;

    new_node_ptr->left_son = Differentiate_Inside (node_ptr->left_son, new_node_ptr, var);
    MY_ASSERT (new_node_ptr->right_son, "Differentiate_Inside ()", FUNC_ERROR, ERROR);

    new_node_ptr->right_son = Copy_Tree (node_ptr->left_son, new_node_ptr);
    MY_ASSERT (new_node_ptr->right_son, "Copy_Tree ()", FUNC_ERROR, ERROR);

    return NO_ERRORS;
}

static int Diff_Sin_Sinh_Cosh (const struct Node *node_ptr, const char *var, struct Node *new_node_ptr, const enum Types func_i)
{
    MY_ASSERT (node_ptr,     "const struct Node *node_ptr", NULL_PTR, ERROR);
    MY_ASSERT (new_node_ptr, "struct Node *new_node_ptr",   NULL_PTR, ERROR);

    enum Types diff_func = 0;
    switch (func_i)
    {
        case SIN:
            diff_func = COS;
            break;
        case SINH:
            diff_func = COSH;
            break;
        case COSH:
            diff_func = SINH;
            break;
        default: 
            MY_ASSERT (false, "enum Types func_i", UNEXP_VAL, ERROR);
    }

    new_node_ptr->type = MULT;

    new_node_ptr->left_son = Create_Node (diff_func, new_node_ptr);
    MY_ASSERT (new_node_ptr->left_son, "Create_Node ()", FUNC_ERROR, ERROR);

    new_node_ptr->left_son->left_son = Copy_Tree (node_ptr->left_son, new_node_ptr->left_son);
    MY_ASSERT (new_node_ptr->left_son->left_son, "Copy_Tree ()", FUNC_ERROR, ERROR);

    new_node_ptr->right_son = Differentiate_Inside (node_ptr->left_son, new_node_ptr, var);
    MY_ASSERT (new_node_ptr->right_son, "Differentiate_Inside ()", FUNC_ERROR, ERROR);

    return NO_ERRORS;
}

static int Diff_Cos (const struct Node *node_ptr, const char *var, struct Node *new_node_ptr)
{
    MY_ASSERT (node_ptr,     "const struct Node *node_ptr", NULL_PTR, ERROR);
    MY_ASSERT (new_node_ptr, "struct Node *new_node_ptr",   NULL_PTR, ERROR);

    new_node_ptr->type = MULT;

    new_node_ptr->left_son  = Create_Num_Node (-1.0, new_node_ptr);
    MY_ASSERT (new_node_ptr->left_son, "Create_Num_Node ()", FUNC_ERROR, ERROR);

    new_node_ptr->right_son = Create_Node (MULT, new_node_ptr);
    MY_ASSERT (new_node_ptr->right_son, "Create_Node ()", FUNC_ERROR, ERROR);

    new_node_ptr->right_son->left_son = Create_Node (SIN, new_node_ptr->right_son);
    MY_ASSERT (new_node_ptr->right_son->left_son, "Create_Node ()", FUNC_ERROR, ERROR);

    new_node_ptr->right_son->left_son->left_son = Copy_Tree (node_ptr->left_son, new_node_ptr->right_son->left_son);
    MY_ASSERT (new_node_ptr->right_son->left_son->left_son, "Create_Node ()", FUNC_ERROR, ERROR);

    new_node_ptr->right_son->right_son = Differentiate_Inside (node_ptr->left_son, new_node_ptr->right_son, var);
    MY_ASSERT (new_node_ptr->right_son->right_son, "Differentiate_Inside ()", FUNC_ERROR, ERROR);

    return NO_ERRORS;
}

static int Diff_Tan_Tanh (const struct Node *node_ptr, const char *var, struct Node *new_node_ptr, const enum Types func_i)
{
    MY_ASSERT (node_ptr,     "const struct Node *node_ptr", NULL_PTR,  ERROR);
    MY_ASSERT (new_node_ptr, "struct Node *new_node_ptr",   NULL_PTR,  ERROR);

    enum Types diff_func = 0;
    switch (func_i)
    {
        case TAN:
            diff_func = COS;
            break;
        case TANH:
            diff_func = COSH;
            break;
        default: 
            MY_ASSERT (false, "enum Types func_i", UNEXP_VAL, ERROR);
    }

    new_node_ptr->type = DIV;

    new_node_ptr->left_son = Differentiate_Inside (node_ptr->left_son, new_node_ptr, var);
    MY_ASSERT (new_node_ptr->left_son, "Differentiate_Inside ()", FUNC_ERROR, ERROR);

    new_node_ptr->right_son = Create_Node (POW, new_node_ptr);
    MY_ASSERT (new_node_ptr->right_son, "Create_Node ()", FUNC_ERROR, ERROR);

    new_node_ptr->right_son->left_son  = Create_Node (diff_func, new_node_ptr->right_son);
    MY_ASSERT (new_node_ptr->right_son->left_son, "Create_Node ()", FUNC_ERROR, ERROR);
    
    new_node_ptr->right_son->right_son = Create_Num_Node (2.0, new_node_ptr->right_son);
    MY_ASSERT (new_node_ptr->right_son->right_son, "Create_Num_Node ()", FUNC_ERROR, ERROR);

    new_node_ptr->right_son->left_son->left_son = Copy_Tree (node_ptr->left_son, new_node_ptr->right_son->left_son);
    MY_ASSERT (new_node_ptr->right_son->left_son->left_son, "Copy_Tree ()", FUNC_ERROR, ERROR);

    return NO_ERRORS;
}

static int Diff_Cot_Coth (const struct Node *node_ptr, const char *var, struct Node *new_node_ptr, const enum Types func_i)
{
    MY_ASSERT (node_ptr,     "const struct Node *node_ptr", NULL_PTR,  ERROR);
    MY_ASSERT (new_node_ptr, "struct Node *new_node_ptr",   NULL_PTR,  ERROR);

    enum Types diff_func = 0;
    switch (func_i)
    {
        case COT:
            diff_func = SIN;
            break;
        case COTH:
            diff_func = SINH;
            break;
        default: 
            MY_ASSERT (false, "enum Types func_i", UNEXP_VAL, ERROR);
    }

    new_node_ptr->type = DIV;

    new_node_ptr->left_son = Create_Node (MULT, new_node_ptr);
    MY_ASSERT (new_node_ptr->left_son, "Create_Node ()", FUNC_ERROR, ERROR);

    new_node_ptr->left_son->left_son = Create_Num_Node (-1.0, new_node_ptr->left_son);
    MY_ASSERT (new_node_ptr->left_son->left_son, "Create_Node ()", FUNC_ERROR, ERROR);

    new_node_ptr->left_son->right_son = Differentiate_Inside (node_ptr->left_son, new_node_ptr->left_son, var);
    MY_ASSERT (new_node_ptr->left_son->right_son, "Differentiate_Inside ()", FUNC_ERROR, ERROR);

    new_node_ptr->right_son = Create_Node (POW, new_node_ptr);
    MY_ASSERT (new_node_ptr->right_son, "Create_Node ()", FUNC_ERROR, ERROR);

    new_node_ptr->right_son->left_son  = Create_Node (diff_func, new_node_ptr->right_son);
    MY_ASSERT (new_node_ptr->right_son->left_son, "Create_Node ()", FUNC_ERROR, ERROR);

    new_node_ptr->right_son->right_son = Create_Num_Node (2.0, new_node_ptr->right_son);
    MY_ASSERT (new_node_ptr->right_son->right_son, "Create_Num_Node ()", FUNC_ERROR, ERROR);

    new_node_ptr->right_son->left_son->left_son = Copy_Tree (node_ptr->left_son, new_node_ptr->right_son->left_son);
    MY_ASSERT (new_node_ptr->right_son->left_son->left_son, "Copy_Tree ()", FUNC_ERROR, ERROR);

    return NO_ERRORS;
}

static int Diff_Arcsin_Arccos (const struct Node *node_ptr, const char *var, struct Node *new_node_ptr, const enum Types func_i)
{
    MY_ASSERT (node_ptr,     "const struct Node *node_ptr", NULL_PTR,  ERROR);
    MY_ASSERT (new_node_ptr, "struct Node *new_node_ptr",   NULL_PTR,  ERROR);

    double multipilier = NAN;
    switch (func_i)
    {
        case ARCSIN:
            multipilier = 1.0;
            break;
        case ARCCOS:
            multipilier = -1.0;
            break;
        default: 
            MY_ASSERT (false, "enum Types func_i", UNEXP_VAL, ERROR);
    }

    new_node_ptr->type = MULT;

    new_node_ptr->right_son = Differentiate_Inside (node_ptr->left_son, new_node_ptr, var);
    MY_ASSERT (new_node_ptr->right_son, "Differentiate_Inside ()", FUNC_ERROR, ERROR);

    new_node_ptr->left_son = Create_Node (DIV, new_node_ptr);
    MY_ASSERT (new_node_ptr->left_son, "Create_Node ()", FUNC_ERROR, ERROR);

    new_node_ptr->left_son->left_son  = Create_Num_Node (multipilier, new_node_ptr->left_son);
    MY_ASSERT (new_node_ptr->left_son->left_son, "Create_Num_Node ()", FUNC_ERROR, ERROR);

    new_node_ptr->left_son->right_son = Create_Node (SQRT, new_node_ptr->left_son);
    MY_ASSERT (new_node_ptr->left_son->right_son, "Create_Node ()", FUNC_ERROR, ERROR);

    new_node_ptr->left_son->right_son->left_son = Create_Node (MINUS, new_node_ptr->left_son->right_son);
    MY_ASSERT (new_node_ptr->left_son->right_son->left_son, "Create_Node ()", FUNC_ERROR, ERROR);

    new_node_ptr->left_son->right_son->left_son->left_son  = Create_Num_Node (1.0, new_node_ptr->left_son->right_son->left_son);
    MY_ASSERT (new_node_ptr->left_son->right_son->left_son->left_son, "Create_Num_Node ()", FUNC_ERROR, ERROR);

    new_node_ptr->left_son->right_son->left_son->right_son = Create_Node (POW, new_node_ptr->left_son->right_son->left_son);
    MY_ASSERT (new_node_ptr->left_son->right_son->left_son->right_son, "Create_Node ()", FUNC_ERROR, ERROR);

    new_node_ptr->left_son->right_son->left_son->right_son->left_son  = Copy_Tree (node_ptr->left_son, new_node_ptr->left_son->right_son->left_son->right_son);
    MY_ASSERT (new_node_ptr->left_son->right_son->left_son->right_son->left_son, "Copy_Tree ()", FUNC_ERROR, ERROR);

    new_node_ptr->left_son->right_son->left_son->right_son->right_son = Create_Num_Node (2.0, new_node_ptr->left_son->right_son->left_son->right_son);
    MY_ASSERT (new_node_ptr->left_son->right_son->left_son->right_son->right_son, "Create_Num_Node ()", FUNC_ERROR, ERROR);

    return NO_ERRORS;
}

static int Diff_Arctan_Arccot (const struct Node *node_ptr, const char *var, struct Node *new_node_ptr, const enum Types func_i)
{
    MY_ASSERT (node_ptr,     "const struct Node *node_ptr", NULL_PTR,  ERROR);
    MY_ASSERT (new_node_ptr, "struct Node *new_node_ptr",   NULL_PTR,  ERROR);

    double multipilier = NAN;
    switch (func_i)
    {
        case ARCTAN:
            multipilier = 1.0;
            break;
        case ARCCOT:
            multipilier = -1.0;
            break;
        default: 
            MY_ASSERT (false, "enum Types func_i", UNEXP_VAL, ERROR);
    }

    new_node_ptr->type = MULT;

    new_node_ptr->right_son = Differentiate_Inside (node_ptr->left_son, new_node_ptr, var);
    MY_ASSERT (new_node_ptr->right_son, "Differentiate_Inside ()", FUNC_ERROR, ERROR);

    new_node_ptr->left_son = Create_Node (DIV, new_node_ptr);
    MY_ASSERT (new_node_ptr->left_son, "Create_Node ()", FUNC_ERROR, ERROR);

    new_node_ptr->left_son->left_son  = Create_Num_Node (multipilier, new_node_ptr->left_son);
    MY_ASSERT (new_node_ptr->left_son->left_son, "Create_Num_Node ()", FUNC_ERROR, ERROR);

    new_node_ptr->left_son->right_son = Create_Node (PLUS, new_node_ptr->left_son);
    MY_ASSERT (new_node_ptr->left_son->right_son, "Create_Node ()", FUNC_ERROR, ERROR);

    new_node_ptr->left_son->right_son->left_son  = Create_Num_Node (1.0, new_node_ptr->left_son->right_son);
    MY_ASSERT (new_node_ptr->left_son->right_son->left_son, "Create_Num_Node ()", FUNC_ERROR, ERROR);

    new_node_ptr->left_son->right_son->right_son = Create_Node (POW, new_node_ptr->left_son->right_son);
    MY_ASSERT (new_node_ptr->left_son->right_son->right_son, "Create_Node ()", FUNC_ERROR, ERROR);

    new_node_ptr->left_son->right_son->right_son->left_son  = Copy_Tree (node_ptr->left_son, new_node_ptr->left_son->right_son->right_son);
    MY_ASSERT (new_node_ptr->left_son->right_son->right_son->left_son, "Copy_Tree ()", FUNC_ERROR, ERROR);

    new_node_ptr->left_son->right_son->right_son->right_son = Create_Num_Node (2.0, new_node_ptr->left_son->right_son->right_son);
    MY_ASSERT (new_node_ptr->left_son->right_son->right_son->right_son, "Create_Num_Node ()", FUNC_ERROR, ERROR);

    return NO_ERRORS;
}

static struct Node *Differentiate_Inside (struct Node *what_diff, struct Node *parent, const char *var)
{
    struct Node *where_put = (struct Node *)calloc (1, sizeof (struct Node));
    MY_ASSERT (where_put, "where_put", NE_MEM, NULL);

    where_put->parent = parent;

    int DOV_status = Diff_One_Var (what_diff, var, where_put);
    MY_ASSERT (DOV_status != ERROR, "Diff_One_Var ()", FUNC_ERROR, NULL);

    return where_put;
}

static struct Node *Create_Node_ (enum Types node_type, struct Node *parent, int parmN, ...)
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
        case PLUS:
        case MINUS:
        case MULT:
        case DIV:
        case POW:
        case PI:
        case E_NUM:
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
            break;

        case NUMBER:
            node_ptr->value.num = va_arg (ap, double);
            break;

        default:
            MY_ASSERT (false, "Types node_type", UNEXP_VAL, NULL);
    }

    va_end (ap);

    return node_ptr;
}

static struct Node *Copy_Tree (const struct Node *node_ptr, struct Node *parent)
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

#define LT node_ptr->left_son->type
#define RT node_ptr->right_son->type

static int Check_Operands (const struct Node *node_ptr, const char *var)
{
    MY_ASSERT (node_ptr, "const struct Node *node_ptr", NULL_PTR, ERROR);
    MY_ASSERT (var,      "const char *var",             NULL_PTR, ERROR);

    MY_ASSERT (node_ptr->left_son,  "node_ptr->left_son",  NULL_PTR, ERROR);
    MY_ASSERT (node_ptr->right_son, "node_ptr->right_son", NULL_PTR, ERROR);

    if (LT == NUMBER && RT == NUMBER)
        return L_NUM_R_NUM;

    else if (LT == NUMBER && (RT == E_NUM || RT == PI))
        return L_NUM_R_CONST;

    else if ((LT == E_NUM || LT == PI) && RT == NUMBER)
        return L_CONST_R_NUM;

    else if ((LT == E_NUM || LT == PI) && (RT == E_NUM || RT == PI))
        return L_CONST_R_CONST;

    else if (LT == NUMBER && ((PLUS <= RT && RT <= POW) || (SQRT <= RT && RT <= COTH) || RT == VARIABLE))
        return L_NUM_R_FUNC;

    else if (((PLUS <= LT && LT <= POW) || (SQRT <= LT && LT <= COTH) || LT == VARIABLE) && RT == NUMBER)
        return L_FUNC_R_NUM;

    else if ((LT == E_NUM || LT == PI) && ((PLUS <= RT && RT <= POW) || (SQRT <= RT && RT <= COTH) || RT == VARIABLE))
        return L_CONST_R_FUNC;

    else if (((PLUS <= LT && LT <= POW) || (SQRT <= LT && LT <= COTH) || LT == VARIABLE) && (RT == E_NUM || RT == PI))
        return L_FUNC_R_CONST;

    else if (((PLUS <= LT && LT <= POW) || (SQRT <= LT && LT <= COTH) || LT == VARIABLE) && 
             ((PLUS <= RT && RT <= POW) || (SQRT <= RT && RT <= COTH) || RT == VARIABLE))
        return L_FUNC_R_FUNC;

    else
        return NO_ERRORS;
}
#undef LT
#undef RT

// ======================================================================================================= //

#ifdef OPTIMIZATION
// ============================================ OPTIMIZATIONS ============================================ //

#define TP    node_ptr->type
#define VAL_S node_ptr->value.str
#define NUM   node_ptr->value.num

#define LEFT  node_ptr->left_son
#define RIGHT node_ptr->right_son

#define L_TP  node_ptr->left_son->type
#define R_TP  node_ptr->right_son->type
#define L_NUM node_ptr->left_son->value.num
#define R_NUM node_ptr->right_son->value.num

#define LL node_ptr->left_son->left_son
#define LR node_ptr->left_son->right_son
#define RL node_ptr->right_son->left_son
#define RR node_ptr->right_son->right_son

static int Simplify_Function (struct Node **root, const char *var)
{
    int change_i = 0;
    for (;;)
    {
        int n_changes = Check_Fractions (*root);
        MY_ASSERT (n_changes != ERROR, "Check_Fractions ()", FUNC_ERROR, ERROR);

        int S_status = Simplify (root, var);
        MY_ASSERT (S_status != ERROR, "Simplify ()", FUNC_ERROR, ERROR);
        n_changes += S_status;

        if (change_i == n_changes)
            break;
        else
            change_i = n_changes;
    }

    return NO_ERRORS;
}

static int Check_Fractions (struct Node *node_ptr)
{
    static int change_i = 0;

    if (TP == DIV)
    {
        if (L_TP == MULT && R_TP == MULT)
        {
            change_i++;
            if      (Compare_Trees (LL, RL))
                Fraction_Reduction (node_ptr, LR, RR);

            else if (Compare_Trees (LL, RR))
                Fraction_Reduction (node_ptr, LR, RL);

            else if (Compare_Trees (LR, RL))
                Fraction_Reduction (node_ptr, LL, RR);

            else if (Compare_Trees (LR, RR))
                Fraction_Reduction (node_ptr, LL, RL);

            else
                change_i--;
        }
        else if (L_TP == DIV && R_TP == DIV)
        {
            change_i++;
            if      (Compare_Trees (LR, RR))
                Fraction_Reduction (node_ptr, LL, RL);

            else if (Compare_Trees (LL, RL))
                Fraction_Reduction (node_ptr, RR, LR);

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

static int Fraction_Reduction (struct Node *root, struct Node *left_node, struct Node *right_node)
{
    MY_ASSERT (root,       "struct Node *root",       NULL_PTR, ERROR);
    MY_ASSERT (left_node,  "struct Node *left_node",  NULL_PTR, ERROR);
    MY_ASSERT (right_node, "struct Node *right_node", NULL_PTR, ERROR);
    
    assert (root);
    struct Node *l_node = Copy_Tree (left_node, root);
    MY_ASSERT (l_node, "struct Node *l_node", NULL_PTR, ERROR);

    struct Node *r_node = Copy_Tree (right_node, root);
    MY_ASSERT (r_node, "struct Node *r_node", NULL_PTR, ERROR);

    Tree_Destructor (root->left_son);
    Tree_Destructor (root->right_son);

    root->left_son  = l_node;
    root->right_son = r_node;

    return NO_ERRORS;
}

#undef LL
#undef LR
#undef RL
#undef RR

#define Change_Into_Num(number)                 \
do                                              \
{                                               \
    node_ptr->type = NUMBER;                    \
    node_ptr->value.num = number;               \
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

enum Comparison
{
    LESS = -1,
    EQUAL,
    GREATER
};

static const double EPSILON = 1E-6;

static int Compare_Double (const double first, const double second)
{
    double absolute_value = fabs (first - second);

    if (absolute_value > EPSILON)
        return (first > second) ? GREATER : LESS;
    else
        return EQUAL;
}

static int Simplify (struct Node **node_ptr_ptr, const char *var)
{
    MY_ASSERT (node_ptr_ptr, "struct Node", NULL_PTR, ERROR);

    struct Node *node_ptr = *node_ptr_ptr;
    static int change_i = 0;

    if (LEFT && !RIGHT)
    {
        Simplify (&LEFT, var);
        return change_i;
    }
    else if (!LEFT && RIGHT)
    {
        Simplify (&RIGHT, var);
        return change_i;
    }
    else if (LEFT && RIGHT)
    {
        int status = Check_Operands (node_ptr, var);
        MY_ASSERT (status != ERROR, "Check_Operands ()", FUNC_ERROR, ERROR);

        if (status == L_NUM_R_FUNC)
            printf ("%f\n", LEFT->value.num);

        printf ("Check_Operands () = %d\n", status);

        if (PLUS <= TP && TP <= POW && status == L_NUM_R_NUM)
        {
            double temp = Calc_Expression (node_ptr);

            printf ("this\n");

            TP = NUMBER;
            NUM = temp;

            Tree_Destructor (LEFT);
            Tree_Destructor (RIGHT);

            LEFT  = NULL;
            RIGHT = NULL;

            change_i++;
        }
        else if ((TP == MULT && (status == L_NUM_R_CONST || status == L_NUM_R_FUNC) && Compare_Double (L_NUM, 1.0) == EQUAL) ||
                 (TP == PLUS && (status == L_NUM_R_CONST || status == L_NUM_R_FUNC) && Compare_Double (L_NUM, 0.0) == EQUAL))
        {
            struct Node *recursion = RIGHT;
            RIGHT = NULL;
            Delete_Neutral_Elem;
            printf ("here\n");
        }
        else if (((TP == MULT || TP == DIV)   && (status == L_CONST_R_NUM || status == L_FUNC_R_NUM) && Compare_Double (R_NUM, 1.0) == EQUAL) ||
                 ((TP == PLUS || TP == MINUS) && (status == L_CONST_R_NUM || status == L_FUNC_R_NUM) && Compare_Double (R_NUM, 0.0) == EQUAL) ||
                  (TP == POW                  && (status == L_CONST_R_NUM || status == L_FUNC_R_NUM) && Compare_Double (R_NUM, 1.0) == EQUAL))
        {
            struct Node *recursion = LEFT;
            LEFT = NULL;
            Delete_Neutral_Elem;
            printf ("there\n");
        }
        else if ((TP == MULT && (((status == L_CONST_R_NUM || status == L_FUNC_R_NUM) && Compare_Double (R_NUM, 0.0) == EQUAL)   ||
                                 ((status == L_NUM_R_CONST || status == L_NUM_R_FUNC) && Compare_Double (L_NUM, 0.0) == EQUAL))) ||
                 (TP == DIV  &&   (status == L_NUM_R_CONST || status == L_NUM_R_FUNC) && Compare_Double (L_NUM, 0.0) == EQUAL))
        {
            Change_Into_Num (0.0);
            printf ("hello\n");
        }

        else if (TP == DIV && (status == L_CONST_R_NUM || status == L_FUNC_R_NUM) && Compare_Double (R_NUM, 0.0) == EQUAL)
            MY_ASSERT (false, "Right operand of expression", UNEXP_ZERO, ERROR);

        else if ((TP == POW && ((status == L_NUM_R_CONST || status == L_NUM_R_FUNC) && Compare_Double (L_NUM, 1.0) == EQUAL)) ||
                 (TP == POW && ((status == L_CONST_R_NUM || status == L_FUNC_R_NUM) && Compare_Double (R_NUM, 0.0) == EQUAL)))
        {
            if (Compare_Double (L_NUM,  0.0) == EQUAL && Compare_Double (R_NUM, 0.0) == EQUAL)
                MY_ASSERT (false, "Operands of expression", UNEXP_ZERO, ERROR);
            else
                Change_Into_Num (1.0);

            printf ("hi\n");
        }

        else if (TP == LN && L_TP == E_NUM)
            Change_Into_Num (1.0);

        Simplify (&LEFT, var);

        Simplify (&RIGHT, var);
    }
    else
        return change_i;
    
    return change_i;
}

static double Calc_Expression (const struct Node *node_ptr)
{
    switch (TP)
    {
        case PLUS:
            return (L_NUM + R_NUM);

        case MINUS:
            return (L_NUM - R_NUM);

        case MULT:
            return (L_NUM * R_NUM);

        case DIV:
            if (Compare_Double (R_NUM, 0.0) == EQUAL)
                MY_ASSERT (false, "Right operand of expression", UNEXP_ZERO, (double)ERROR);

            return (L_NUM / R_NUM);

        case POW:
            if (Compare_Double (L_NUM, 0.0) == EQUAL && Compare_Double (R_NUM, 0.0) == EQUAL)
                MY_ASSERT (false, "Operands of expression", UNEXP_ZERO, (double)ERROR);

            return pow (L_NUM, R_NUM);

        default: 
            MY_ASSERT (false, "node_ptr->value.sumbol", UNEXP_VAL, (double)ERROR);
    }
}

static int Compare_Trees (const struct Node *tree_1, const struct Node *tree_2)
{
    if (tree_1->type != tree_2->type)
        return 0;

    switch (tree_1->type)
    {
        case NUMBER:
            if (Compare_Double (tree_1->value.num, tree_2->value.num) != EQUAL)
                return 0;
            break;
            
        default:
            MY_ASSERT (false, "tree_1->type", UNEXP_VAL, ERROR);
    }

    if (tree_1->left_son && tree_2->left_son)
        if (Compare_Trees (tree_1->left_son, tree_2->left_son) == 0)
            return 0;

    if (tree_1->right_son && tree_2->right_son)
        if (Compare_Trees (tree_1->right_son, tree_2->right_son) == 0)
            return 0;

    return 1;
}
// ======================================================================================================= //
#endif

#if 0
// ============================================= DUMP IN TEX ============================================= //

int Dump_In_Tex (const struct Node *orig_tree, struct Node **tree_arr, char **vars_arr, const int n_vars)
{
    MY_ASSERT (tree_arr,       "const struct Node **tree_arr", NULL_PTR, ERROR);
    MY_ASSERT (vars_arr,       "const char **vars_arr",        NULL_PTR, ERROR);

    FILE *tex_file = Open_File ("./Output/Tex_File.txt", "wb");

    fprintf (tex_file, "\\(f(");
    for (int i = 0; i < n_vars - 1; i++)
        fprintf (tex_file, "%s, ", vars_arr[i]);
    fprintf (tex_file, "%s) = ", vars_arr[n_vars - 1]);

    Formula_Dump (orig_tree, tex_file);
    fprintf (tex_file, "\\)");
    fprintf (tex_file, "\\\\\n\\\\\n");

    for (int i = 0; i < n_vars; i++)
    {
        if (n_vars == 1)
            fprintf (tex_file, "\\(f'(%s) = ", vars_arr[i]);
        else
            fprintf (tex_file, "\\( \\frac{\\partial f}{\\partial %s} = ", vars_arr[i]);

        Formula_Dump (tree_arr[i], tex_file);
        fprintf (tex_file, "\\)");
        fprintf (tex_file, "\\\\\n\\\\\n");
    }

    Close_File (tex_file, "./Output/Tex_File.txt");

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

#define PRINT_MULT_OPERAND(side, side_tp, side_val_n)                   \
do                                                                      \
{                                                                       \
    switch (side_tp)                                                    \
    {                                                                   \
        case NUMBER:                                                    \
            if (side_val_n < 0)                                         \
                PRINT ("(", side, ")");                                 \
            else                                                        \
                Formula_Dump (side, tex_file);                          \
            break;                                                      \
        case MATH_CONST:                                                \
        case VARIABLE:                                                  \
        case FUNCTION:                                                  \
            Formula_Dump (side, tex_file);                              \
            break;                                                      \
        default:                                                        \
            if (side_tp == MULT || side_tp == POW || side_tp == DIV)    \
                Formula_Dump (side, tex_file);                          \
            else                                                        \
                PRINT ("(", side, ")");                                 \
    }                                                                   \
}                                                                       \
while (0)

#define PRINT_POW_OPERAND(side, side_tp, side_val_n, other_side_tp)             \
do                                                                              \
{                                                                               \
    fprintf (tex_file, "{");                                                    \
    switch (side_tp)                                                            \
    {                                                                           \
        case NUMBER:                                                            \
            if (side_val_n < 0)                                                 \
                PRINT ("(", side, ")");                                         \
            else                                                                \
                Formula_Dump (side, tex_file);                                  \
            break;                                                              \
        case MATH_CONST:                                                        \
        case VARIABLE:                                                          \
        case FUNCTION:                                                          \
            Formula_Dump (side, tex_file);                                      \
            break;                                                              \
        default:                                                                \
            if (PLUS <= side_tp && side_tp <= POW && other_side_tp == POW)      \
                Formula_Dump (side, tex_file);                                  \
            else                                                                \
                PRINT ("(", side, ")");                                         \
    }                                                                           \
    fprintf (tex_file, "}");                                                    \
}                                                                               \
while (0)

int Formula_Dump (const struct Node *node_ptr, FILE *tex_file)
{
    switch (TP)
    {
        case NUMBER:
            if (Compare_Double (NUM, 0.5) == EQUAL)
                fprintf (tex_file, "\\frac{1}{2}");
            else if (Compare_Double (NUM, -0.5) == EQUAL)
                fprintf (tex_file, "\\frac{-1}{2}");
            else
                fprintf (tex_file, "%.f", NUM);
            break;

        case MATH_CONST:
            if (strcmp (node_ptr->value.math_const, "e") == 0)
                fprintf (tex_file, "e");
            else
                fprintf (tex_file, "\\pi");
            break;

        case VARIABLE:
            fprintf (tex_file, "%s", VAL_S);
            break;

        case FUNCTION:
            if (!strcmp (node_ptr->value.str, Functions_Data_Base[Sqrt]))
            {
                fprintf (tex_file, "\\sqrt");
                PRINT ("{", LEFT, "}");
            }
            else
            {
                fprintf (tex_file, "%s ", node_ptr->value.str);
                PRINT ("(", LEFT, ")");
            }

            break;

        case PLUS:
        case MINUS:
            Formula_Dump (LEFT, tex_file);
            fprintf (tex_file, " %s ", VAL_S);
            Formula_Dump (RIGHT, tex_file);
            break;

        case MULT:
            PRINT_MULT_OPERAND (LEFT, L_TP, L_NUM);
            fprintf (tex_file, "\\cdot ");
            PRINT_MULT_OPERAND (RIGHT, R_TP, R_NUM);
            break;

        case POW:
            PRINT_POW_OPERAND (LEFT, L_TP, L_NUM, R_TP);
            fprintf (tex_file, "^");
            PRINT_POW_OPERAND (RIGHT, R_TP, R_NUM, L_TP);
            break;

        case DIV:
            fprintf (tex_file, "\\frac");
            PRINT ("{", LEFT,  "}");
            PRINT ("{", RIGHT, "}");
            break;

        default: MY_ASSERT (false, "node_ptr->type", UNEXP_VAL, ERROR);
    }

    return NO_ERRORS;
}

// ======================================================================================================= //
#endif

#include "../include/Differentiator.h"
#include "My_Lib.h"

// ========================================== STATIC PROTOTYPES ========================================== //

static struct Node **Forest_Ctor   (const int n_vars);
static int           Forest_Dtor   (struct Node **forest, const int n_vars);
static int           Dump_One_Tree (const struct Node *func_root, const struct Node *der_root, const char *var, const int n_vars);
static char        **Find_Vars     (const struct Node *node_ptr, int *n_vars);
static int           Add_Vars      (const struct Node* node_ptr, char **vars_arr);
static bool          Find_Name     (char **vars_arr, const char *var, const int n_vars);

static int Diff_One_Var (const struct Node *node_ptr, const char *var, struct Node *new_node_ptr);

static struct Node *Differentiate_Inside (struct Node *what_diff, struct Node *parent, const char *var);
static struct Node *Create_Node_         (enum Types node_type, struct Node *parent, int parmN, ...);
static struct Node *Copy_Tree            (const struct Node *node_ptr, struct Node *parent);
static int          Check_Operands       (const struct Node *node_ptr, const char *var);
static bool         Check_If_Function    (const int node_type);
static bool         Check_If_Const       (const int node_type);

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

static int    Optimizer           (struct Node **root);
static int    Check_Fractions     (struct Node *node_ptr);
static int    Compare_Trees       (const struct Node *tree_1, const struct Node *tree_2);
static int    Fraction_Reduction  (struct Node *root, struct Node *left_node, struct Node *right_node);
static int    Simplify            (struct Node **node_ptr_ptr);
static void   Turn_Into_Num       (struct Node *node_ptr, const double num);
static void   Delete_Neutral_Elem (struct Node **node_ptr_ptr, struct Node *not_neutral);
static double Calc_Expression     (const struct Node *node_ptr);
static bool   Check_If_Math_Sign  (const int node_type);
static int    Compare_Double      (const double first, const double second);

static int Dump_In_Tex        (const struct Node *orig_tree, struct Node **forest, char **vars_arr, const int n_vars);
static int Formula_Dump       (const struct Node *node_ptr, FILE *tex_file);
static int Print_Mult_Operand (const struct Node *node_ptr, FILE *tex_file);
static int Print_Pow_Operand  (const struct Node *node_ptr, FILE *tex_file);

// ======================================================================================================= //

// ========================================== GENERAL FUNCTIONS ========================================== //

int Differentiator (const struct Node *root)
{
    MY_ASSERT (root, "struct Node *root", NULL_PTR, ERROR);

    struct Node *root_copy = Copy_Tree (root, NULL);
    MY_ASSERT (root_copy, "Copy_Tree ()", FUNC_ERROR, ERROR);
    
    int O_status = Optimizer (&root_copy);
    MY_ASSERT (O_status != ERROR, "Optimizer ()", FUNC_ERROR, ERROR);
    
    int n_vars = 0;
    char **vars_arr = Find_Vars (root_copy, &n_vars);
    MY_ASSERT (vars_arr, "Find_Vars ()", FUNC_ERROR, ERROR);

    struct Node **forest = Forest_Ctor (n_vars);
    MY_ASSERT (forest, "Forest_Ctor ()", FUNC_ERROR, ERROR);

    for (int var_i = 0; var_i < n_vars; var_i++)
    {
        int DOV_status = Diff_One_Var (root_copy, vars_arr[var_i], forest[var_i]);
        MY_ASSERT (DOV_status != ERROR, "Diff_One_Var ()", FUNC_ERROR, ERROR);

        O_status = Optimizer (forest + var_i);
        MY_ASSERT (O_status != ERROR, "Optimizer ()", FUNC_ERROR, ERROR);

        int DOT_status = Dump_One_Tree (root_copy, forest[var_i], vars_arr[var_i], n_vars);
        MY_ASSERT (DOT_status != ERROR, "Dump_One_Tree ()", FUNC_ERROR, ERROR);
    }

    int DIT_status = Dump_In_Tex (root_copy, forest, vars_arr, n_vars);
    MY_ASSERT (DIT_status != ERROR, "Dump_In_Tex ()", FUNC_ERROR, ERROR);

    for (int var_i = 0; var_i < n_vars; var_i++)
        free (vars_arr[var_i]);       
        
    free (vars_arr);

    int TD_status = Tree_Destructor (root_copy);
    MY_ASSERT (TD_status != ERROR, "Tree_Destructor ()", FUNC_ERROR, ERROR);

    int FD_status = Forest_Dtor (forest, n_vars);
    MY_ASSERT (FD_status , "Forest_Dtor ()", FUNC_ERROR, ERROR);

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
        int TD_status = Tree_Destructor (forest[i]);
        MY_ASSERT (TD_status != ERROR, "Tree_Destructor ()", FUNC_ERROR, ERROR);
    } 

    free (forest);

    return NO_ERRORS;
}

static int Dump_One_Tree (const struct Node *func_root, const struct Node *der_root, const char *var, const int n_vars)
{
    MY_ASSERT (func_root,  "const struct Node *func_root", NULL_PTR, ERROR);
    MY_ASSERT (der_root,   "const struct Node *der_root",  NULL_PTR, ERROR);
    MY_ASSERT (var,        "const char *var",              NULL_PTR, ERROR);
    MY_ASSERT (n_vars > 0, "const int n_vars",             POS_VAL,  ERROR);
    
    const size_t var_len = strlen (var);
    const size_t func_buff_size = sizeof ("./output/Function_Of_") + var_len + sizeof (".dot");
    const size_t der_buff_size = sizeof ("./output/Partial_Derivative_Of_") + var_len + sizeof (".dot");

    char *func_buffer_1 = (char *)calloc (func_buff_size, sizeof (char));
    MY_ASSERT (func_buffer_1, "char *func_buffer_1", NE_MEM, ERROR);

    char *func_buffer_2 = (char *)calloc (func_buff_size, sizeof (char));
    MY_ASSERT (func_buffer_1, "char *func_buffer_2", NE_MEM, ERROR);

    char *der_buffer_1 = (char *)calloc (der_buff_size, sizeof (char));
    MY_ASSERT (der_buffer_1, "char *buffer_1", NE_MEM, ERROR);

    char *der_buffer_2 = (char *)calloc (der_buff_size, sizeof (char));
    MY_ASSERT (der_buffer_2, "char *buffer_2", NE_MEM, ERROR);

    system ("mkdir -p ./output");

    if (n_vars > 1)
    {
        sprintf (func_buffer_1, "./output/Function_Of_%s.dot", var);
        sprintf (func_buffer_2, "./output/Function_Of_%s.png", var);
        sprintf (der_buffer_1,  "./output/Partial_Derivative_Of_%s.dot", var);
        sprintf (der_buffer_2,  "./output/Partial_Derivative_Of_%s.png", var);
    }
    else
    {
        sprintf (func_buffer_1, "./output/Function.dot");
        sprintf (func_buffer_2, "./output/Function.png");
        sprintf (der_buffer_1,  "./output/Derivative.dot");
        sprintf (der_buffer_2,  "./output/Derivative.png");
    }

    int TD_status = Tree_Dump (func_root, func_buffer_1, func_buffer_2, var);
    MY_ASSERT (TD_status != ERROR, "Tree_Dump ()", FUNC_ERROR, ERROR);

    TD_status = Tree_Dump (der_root, der_buffer_1, der_buffer_2, var);
    MY_ASSERT (TD_status != ERROR, "Tree_Dump ()", FUNC_ERROR, ERROR);

    free (func_buffer_1);
    free (func_buffer_2);
    free (der_buffer_1);
    free (der_buffer_2);

    return NO_ERRORS;
}

#define MAX_N_VARS 50

static char **Find_Vars (const struct Node *node_ptr, int *n_vars)
{
    MY_ASSERT (node_ptr, "struct Node *node_ptr", NULL_PTR, NULL);
    MY_ASSERT (n_vars,   "int *n_vars",           NULL_PTR, NULL);
    
    char **vars_arr = (char **)calloc (MAX_N_VARS, sizeof (char *));
    MY_ASSERT (vars_arr, "char *vars_arr", NE_MEM, NULL);

    *n_vars = Add_Vars (node_ptr, vars_arr);
    MY_ASSERT (*n_vars != ERROR, "Add_Vars ()", FUNC_ERROR, NULL);

    return vars_arr;
}

static int Add_Vars (const struct Node* node_ptr, char **vars_arr)
{
    MY_ASSERT (node_ptr, "struct Node *node_ptr", NULL_PTR, ERROR);
    MY_ASSERT (vars_arr, "char **vars_arr",       NULL_PTR, ERROR);
    
    static int var_i = 0;
    
    if (node_ptr->type == VARIABLE && Find_Name (vars_arr, node_ptr->value.str, var_i) == false)
    {
        size_t str_len = strlen (node_ptr->value.str);
        vars_arr[var_i] = (char *)calloc (str_len, sizeof (char));
        MY_ASSERT (vars_arr[var_i], "vars_arr[var_i]", NE_MEM, ERROR);

        memcpy (vars_arr[var_i], node_ptr->value.str, str_len);
        var_i++;
    }

    if (node_ptr->left_son)
        Add_Vars (node_ptr->left_son, vars_arr);

    if (node_ptr->right_son)
        Add_Vars (node_ptr->right_son, vars_arr);

    return var_i;
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
        {
            int DS_status = Diff_Sqrt (node_ptr, var, new_node_ptr);
            MY_ASSERT (DS_status != ERROR, "Diff_Sqrt ()", FUNC_ERROR, ERROR);
            break;
        }
            
        case LN:
        {
            int DL_status = Diff_Ln (node_ptr, var, new_node_ptr);
            MY_ASSERT (DL_status != ERROR, "Diff_Ln ()", FUNC_ERROR, ERROR);
            break;
        }

        case SIN:
        case SINH:
        case COSH:
        {
            int DSSC_status = Diff_Sin_Sinh_Cosh (node_ptr, var, new_node_ptr, node_ptr->type);
            MY_ASSERT (DSSC_status != ERROR, "Diff_Sin_Sinh_Cosh ()", FUNC_ERROR, ERROR);
            break;
        }
            
        case COS:
        {
            int DC_status = Diff_Cos (node_ptr, var, new_node_ptr);
            MY_ASSERT (DC_status != ERROR, "Diff_Cos ()", FUNC_ERROR, ERROR);
            break;
        }

        case TAN:
        case TANH:
        {
            int DTT_status = Diff_Tan_Tanh (node_ptr, var, new_node_ptr, node_ptr->type);
            MY_ASSERT (DTT_status != ERROR, "Diff_Tan_Tanh ()", FUNC_ERROR, ERROR);
            break;
        }

        case COT:
        case COTH:
        {
            int DCC_status = Diff_Cot_Coth (node_ptr, var, new_node_ptr, node_ptr->type);
            MY_ASSERT (DCC_status != ERROR, "Diff_Cot_Coth ()", FUNC_ERROR, ERROR);
            break;
        }
            

        case ARCSIN:
        case ARCCOS:
        {
            int DAA_status = Diff_Arcsin_Arccos (node_ptr, var, new_node_ptr, node_ptr->type);
            MY_ASSERT (DAA_status != ERROR, "Diff_Arcsin_Arccos ()", FUNC_ERROR, ERROR);
            break;
        }
            

        case ARCTAN:
        case ARCCOT:
        {
            int DAA_status = Diff_Arctan_Arccot (node_ptr, var, new_node_ptr, node_ptr->type);
            MY_ASSERT (DAA_status != ERROR, "Diff_Arctan_Arccot ()", FUNC_ERROR, ERROR);
            break;
        }

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
        {
            int DSS_status = Diff_Sum_Sub (node_ptr, var, new_node_ptr, node_ptr->type);
            MY_ASSERT (DSS_status != ERROR, "Diff_Sum_Sub ()", FUNC_ERROR, ERROR);
            break;
        }
            
        case MULT:
        {
            int DM_status = Diff_Mult (node_ptr, var, new_node_ptr);
            MY_ASSERT (DM_status != ERROR, "Diff_Mult ()", FUNC_ERROR, ERROR);
            break;
        }

        case DIV:
        {
            int DD_status = Diff_Div (node_ptr, var, new_node_ptr);
            MY_ASSERT (DD_status != ERROR, "Diff_Div ()", FUNC_ERROR, ERROR);
            break;
        }

        case POW:
        {
            int CO_status = Check_Operands (node_ptr, var);
            MY_ASSERT (CO_status != ERROR, "Check_Operands ()", FUNC_ERROR, ERROR);
            
            switch (CO_status)
            {
                case L_FUNC_R_NUM:
                case L_FUNC_R_CONST:
                {
                    int DP_status = Diff_Pow (node_ptr, var, new_node_ptr);
                    MY_ASSERT (DP_status != ERROR, "Diff_Pow ()", FUNC_ERROR, ERROR);
                    break;
                }

                case L_NUM_R_FUNC:
                case L_CONST_R_FUNC:
                {
                    int DE_status = Diff_Exp (node_ptr, var, new_node_ptr);
                    MY_ASSERT (DE_status != ERROR, "Diff_Exp ()", FUNC_ERROR, ERROR);
                    break;
                }

                case L_FUNC_R_FUNC:
                {
                    int DPE_status = Diff_Pow_Exp (node_ptr, var, new_node_ptr);
                    MY_ASSERT (DPE_status != ERROR, "Diff_Pow_Exp ()", FUNC_ERROR, ERROR);
                    break;
                }

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
        }

        default: 
            MY_ASSERT (false, "node_ptr->type", UNEXP_VAL, ERROR);
    }

    return NO_ERRORS;
}

#define L left_son
#define R right_son

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

    new_node_ptr->L = Differentiate_Inside (node_ptr->L, new_node_ptr, var);
    MY_ASSERT (new_node_ptr->L, "Differentiate_Inside ()", FUNC_ERROR, ERROR);

    new_node_ptr->R = Differentiate_Inside (node_ptr->R, new_node_ptr, var);
    MY_ASSERT (new_node_ptr->R, "Differentiate_Inside ()", FUNC_ERROR, ERROR);

    return NO_ERRORS;
}

#define Create_Num_Node(val, parent) Create_Node_ (NUMBER, parent, 1, val)
#define Create_Node(node_type, parent) Create_Node_ (node_type, parent, 0)

static int Diff_Mult (const struct Node *node_ptr, const char *var, struct Node *new_node_ptr)
{
    MY_ASSERT (node_ptr,     "const struct Node *node_ptr", NULL_PTR, ERROR);
    MY_ASSERT (new_node_ptr, "struct Node *new_node_ptr",   NULL_PTR, ERROR);

    new_node_ptr->type = PLUS;

    new_node_ptr->L = Create_Node (MULT, new_node_ptr);
    MY_ASSERT (new_node_ptr->L, "Create_Node ()", FUNC_ERROR, ERROR);

    new_node_ptr->L->R = Copy_Tree (node_ptr->R, new_node_ptr->L);
    MY_ASSERT (new_node_ptr->L->R, "Copy_Tree ()", FUNC_ERROR, ERROR);

    new_node_ptr->L->L = Differentiate_Inside (node_ptr->L, new_node_ptr->L, var);
    MY_ASSERT (new_node_ptr->L->L, "Differentiate_Inside ()", FUNC_ERROR, ERROR);

    new_node_ptr->R = Create_Node (MULT, new_node_ptr);
    MY_ASSERT (new_node_ptr->R, "Create_Node ()", FUNC_ERROR, ERROR);

    new_node_ptr->R->L = Copy_Tree (node_ptr->L, new_node_ptr->R);
    MY_ASSERT (new_node_ptr->R->L, "Copy_Tree ()", FUNC_ERROR, ERROR);

    new_node_ptr->R->R = Differentiate_Inside (node_ptr->R, new_node_ptr->R, var);
    MY_ASSERT (new_node_ptr->R->R, "Differentiate_Inside ()", FUNC_ERROR, ERROR);

    return NO_ERRORS;
}

static int Diff_Div (const struct Node *node_ptr, const char *var, struct Node *new_node_ptr)
{
    MY_ASSERT (node_ptr,     "const struct Node *node_ptr", NULL_PTR, ERROR);
    MY_ASSERT (new_node_ptr, "struct Node *new_node_ptr",   NULL_PTR, ERROR);

    new_node_ptr->type = DIV;

    new_node_ptr->L = Create_Node (MINUS, new_node_ptr);
    MY_ASSERT (new_node_ptr->L, "Create_Node ()", FUNC_ERROR, ERROR);

    new_node_ptr->L->L = Create_Node (MULT, new_node_ptr->L);
    MY_ASSERT (new_node_ptr->L->L, "Create_Node ()", FUNC_ERROR, ERROR);

    new_node_ptr->L->L->R = Copy_Tree (node_ptr->R, new_node_ptr->L->L);
    MY_ASSERT (new_node_ptr->L->L->R, "Copy_Tree ()", FUNC_ERROR, ERROR);

    new_node_ptr->L->L->L  = Differentiate_Inside (node_ptr->L, new_node_ptr->L->L, var);
    MY_ASSERT (new_node_ptr->L->L->L, "Differentiate_Inside ()", FUNC_ERROR, ERROR);

    new_node_ptr->L->R = Create_Node (MULT, new_node_ptr->L);
    MY_ASSERT (new_node_ptr->L->R, "Create_Node ()", FUNC_ERROR, ERROR);

    new_node_ptr->L->R->L = Copy_Tree (node_ptr->L, new_node_ptr->L->R);
    MY_ASSERT (new_node_ptr->L->R->L, "Copy_Tree ()", FUNC_ERROR, ERROR);

    new_node_ptr->L->R->R = Differentiate_Inside (node_ptr->R, new_node_ptr->L->R, var);
    MY_ASSERT (new_node_ptr->L->R->R, "Differentiate_Inside ()", FUNC_ERROR, ERROR);

    new_node_ptr->R = Create_Node (POW, new_node_ptr);
    MY_ASSERT (new_node_ptr->R, "Create_Node ()", FUNC_ERROR, ERROR);

    new_node_ptr->R->L  = Copy_Tree (node_ptr->R, new_node_ptr->R);
    MY_ASSERT (new_node_ptr->R->L, "Copy_Tree ()", FUNC_ERROR, ERROR);

    new_node_ptr->R->R = Create_Num_Node (2.0, new_node_ptr->R);
    MY_ASSERT (new_node_ptr->R->R, "Create_Num_Node ()", FUNC_ERROR, ERROR);

    return NO_ERRORS;
}

static int Diff_Sqrt (const struct Node *node_ptr, const char *var, struct Node *new_node_ptr)
{
    MY_ASSERT (node_ptr,     "const struct Node *node_ptr", NULL_PTR, ERROR);
    MY_ASSERT (new_node_ptr, "struct Node *new_node_ptr",   NULL_PTR, ERROR);

    new_node_ptr->type = DIV;

    new_node_ptr->L = Differentiate_Inside (node_ptr->L, new_node_ptr, var);
    MY_ASSERT (new_node_ptr->L, "Differentiate_Inside ()", FUNC_ERROR, ERROR);

    new_node_ptr->R = Create_Node (MULT, new_node_ptr);
    MY_ASSERT (new_node_ptr->R, "Create_Node ()", FUNC_ERROR, ERROR);
    
    new_node_ptr->R->L = Create_Num_Node (2.0, new_node_ptr->R);
    MY_ASSERT (new_node_ptr->R->L, "Create_Num_Node ()", FUNC_ERROR, ERROR);

    new_node_ptr->R->R = Copy_Tree (node_ptr, new_node_ptr->R);
    MY_ASSERT (new_node_ptr->R->R, "Copy_Tree ()", FUNC_ERROR, ERROR);

    return NO_ERRORS;
}

static int Diff_Pow (const struct Node *node_ptr, const char *var, struct Node *new_node_ptr)
{
    MY_ASSERT (node_ptr,     "const struct Node *node_ptr", NULL_PTR, ERROR);
    MY_ASSERT (new_node_ptr, "struct Node *new_node_ptr",   NULL_PTR, ERROR);

    new_node_ptr->type = MULT;

    new_node_ptr->L = Create_Node (MULT, new_node_ptr);
    MY_ASSERT (new_node_ptr->L, "Create_Node ()", FUNC_ERROR, ERROR);

    new_node_ptr->L->L = Copy_Tree (node_ptr->R, new_node_ptr->L);
    MY_ASSERT (new_node_ptr->L->L, "Copy_Tree ()", FUNC_ERROR, ERROR);

    new_node_ptr->L->R = Create_Node (POW, new_node_ptr->L);
    MY_ASSERT (new_node_ptr->L->R, "Create_Node ()", FUNC_ERROR, ERROR);

    new_node_ptr->L->R->L = Copy_Tree (node_ptr->L, new_node_ptr->L->R);
    MY_ASSERT (new_node_ptr->L->R->L, "Copy_Tree ()", FUNC_ERROR, ERROR);

    new_node_ptr->L->R->R = Create_Node (MINUS, new_node_ptr->L->R);
    MY_ASSERT (new_node_ptr->L->R->R, "Create_Node ()", FUNC_ERROR, ERROR);

    new_node_ptr->L->R->R->L = Copy_Tree (node_ptr->R, new_node_ptr->L->R->R);
    MY_ASSERT (new_node_ptr->L->R->R->L, "Copy_Tree ()", FUNC_ERROR, ERROR);
    
    new_node_ptr->L->R->R->R = Create_Num_Node (1.0, new_node_ptr->L->R->R);
    MY_ASSERT (new_node_ptr->L->R->R->R, "Create_Num_Node ()", FUNC_ERROR, ERROR);

    new_node_ptr->R = Differentiate_Inside (node_ptr->L, new_node_ptr, var);
    MY_ASSERT (new_node_ptr->R, "Differentiate_Inside ()", FUNC_ERROR, ERROR);

    return NO_ERRORS;
}

static int Diff_Exp (const struct Node *node_ptr, const char *var, struct Node *new_node_ptr)
{
    MY_ASSERT (node_ptr,     "const struct Node *node_ptr", NULL_PTR, ERROR);
    MY_ASSERT (new_node_ptr, "struct Node *new_node_ptr",   NULL_PTR, ERROR);

    new_node_ptr->type = MULT;

    new_node_ptr->R = Differentiate_Inside (node_ptr->R, new_node_ptr, var);
    MY_ASSERT (new_node_ptr->R, "Differentiate_Inside ()", FUNC_ERROR, ERROR);

    new_node_ptr->L = Create_Node (MULT, new_node_ptr);
    MY_ASSERT (new_node_ptr->L, "Create_Node ()", FUNC_ERROR, ERROR);

    new_node_ptr->L->R = Create_Node (LN, new_node_ptr->L);
    MY_ASSERT (new_node_ptr->L->R, "Create_Node ()", FUNC_ERROR, ERROR);

    new_node_ptr->L->R->L = Copy_Tree (node_ptr->left_son, new_node_ptr->L->R);
    MY_ASSERT (new_node_ptr->L->R->L, "Copy_Tree ()", FUNC_ERROR, ERROR);

    new_node_ptr->L->L = Create_Node (POW, new_node_ptr->L);
    MY_ASSERT (new_node_ptr->L->L, "Create_Node ()", FUNC_ERROR, ERROR);

    new_node_ptr->L->L->L = Copy_Tree (node_ptr->L, new_node_ptr->L->L);
    MY_ASSERT (new_node_ptr->L->L->L, "Copy_Tree ()", FUNC_ERROR, ERROR);

    new_node_ptr->L->L->R = Copy_Tree (node_ptr->R, new_node_ptr->L->L);
    MY_ASSERT (new_node_ptr->L->L->R, "Copy_Tree ()", FUNC_ERROR, ERROR);

    return NO_ERRORS;
}

static int Diff_Pow_Exp (const struct Node *node_ptr, const char *var, struct Node *new_node_ptr)
{
    MY_ASSERT (node_ptr,     "const struct Node *node_ptr", NULL_PTR, ERROR);
    MY_ASSERT (new_node_ptr, "struct Node *new_node_ptr",   NULL_PTR, ERROR);

    new_node_ptr->type = MULT;

    new_node_ptr->L = Create_Node (POW, new_node_ptr);
    MY_ASSERT (new_node_ptr->L, "Create_Node ()", FUNC_ERROR, ERROR);

    new_node_ptr->L->L = Copy_Tree (node_ptr->L, new_node_ptr->L);
    MY_ASSERT (new_node_ptr->L->L, "Copy_Tree ()", FUNC_ERROR, ERROR);

    new_node_ptr->L->R = Copy_Tree (node_ptr->R, new_node_ptr->L);
    MY_ASSERT (new_node_ptr->L->R, "Copy_Tree ()", FUNC_ERROR, ERROR);

    new_node_ptr->R = Create_Node (PLUS, new_node_ptr);
    MY_ASSERT (new_node_ptr->R, "Create_Node ()", FUNC_ERROR, ERROR);

    new_node_ptr->R->L = Create_Node (MULT, new_node_ptr->R);
    MY_ASSERT (new_node_ptr->R->L, "Create_Node ()", FUNC_ERROR, ERROR);

    new_node_ptr->R->R = Create_Node (MULT, new_node_ptr->R);
    MY_ASSERT (new_node_ptr->R->R, "Create_Node ()", FUNC_ERROR, ERROR);

    new_node_ptr->R->L->L = Differentiate_Inside (node_ptr->R, new_node_ptr->R->L, var);
    MY_ASSERT (new_node_ptr->R->L->L, "Differentiate_Inside ()", FUNC_ERROR, ERROR);

    new_node_ptr->R->L->R = Create_Node (LN, new_node_ptr->R->L);
    MY_ASSERT (new_node_ptr->R->L->right_son, "Create_Node ()", FUNC_ERROR, ERROR);

    new_node_ptr->R->L->R->L = Copy_Tree (node_ptr->L, new_node_ptr->R->L->R);
    MY_ASSERT (new_node_ptr->R->L->R->L, "Copy_Tree ()", FUNC_ERROR, ERROR);

    new_node_ptr->R->R->L  = Copy_Tree (node_ptr->R, new_node_ptr->R->R);
    MY_ASSERT (new_node_ptr->R->R->L, "Copy_Tree ()", FUNC_ERROR, ERROR);

    new_node_ptr->R->R->R = Create_Node (DIV, new_node_ptr->R->R);
    MY_ASSERT (new_node_ptr->R->R->R, "Create_Node ()", FUNC_ERROR, ERROR);

    new_node_ptr->R->R->R->R = Copy_Tree (node_ptr->L, new_node_ptr->R->R->R);
    MY_ASSERT (new_node_ptr->R->R->R->R, "Copy_Tree ()", FUNC_ERROR, ERROR);

    new_node_ptr->R->R->R->L = Differentiate_Inside (node_ptr->L, new_node_ptr->R->R->R, var);
    MY_ASSERT (new_node_ptr->R->R->R->L, "Differentiate_Inside ()", FUNC_ERROR, ERROR);

    return NO_ERRORS;
}

static int Diff_Ln (const struct Node *node_ptr, const char *var, struct Node *new_node_ptr)
{
    MY_ASSERT (node_ptr,     "const struct Node *node_ptr", NULL_PTR, ERROR);
    MY_ASSERT (new_node_ptr, "struct Node *new_node_ptr",   NULL_PTR, ERROR);

    new_node_ptr->type = DIV;

    new_node_ptr->L = Differentiate_Inside (node_ptr->L, new_node_ptr, var);
    MY_ASSERT (new_node_ptr->right_son, "Differentiate_Inside ()", FUNC_ERROR, ERROR);

    new_node_ptr->R = Copy_Tree (node_ptr->L, new_node_ptr);
    MY_ASSERT (new_node_ptr->R, "Copy_Tree ()", FUNC_ERROR, ERROR);

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

    new_node_ptr->L = Create_Node (diff_func, new_node_ptr);
    MY_ASSERT (new_node_ptr->L, "Create_Node ()", FUNC_ERROR, ERROR);

    new_node_ptr->L->L = Copy_Tree (node_ptr->left_son, new_node_ptr->L);
    MY_ASSERT (new_node_ptr->L->L, "Copy_Tree ()", FUNC_ERROR, ERROR);

    new_node_ptr->R = Differentiate_Inside (node_ptr->L, new_node_ptr, var);
    MY_ASSERT (new_node_ptr->R, "Differentiate_Inside ()", FUNC_ERROR, ERROR);

    return NO_ERRORS;
}

static int Diff_Cos (const struct Node *node_ptr, const char *var, struct Node *new_node_ptr)
{
    MY_ASSERT (node_ptr,     "const struct Node *node_ptr", NULL_PTR, ERROR);
    MY_ASSERT (new_node_ptr, "struct Node *new_node_ptr",   NULL_PTR, ERROR);

    new_node_ptr->type = MULT;

    new_node_ptr->L  = Create_Num_Node (-1.0, new_node_ptr);
    MY_ASSERT (new_node_ptr->L, "Create_Num_Node ()", FUNC_ERROR, ERROR);

    new_node_ptr->R = Create_Node (MULT, new_node_ptr);
    MY_ASSERT (new_node_ptr->R, "Create_Node ()", FUNC_ERROR, ERROR);

    new_node_ptr->R->L = Create_Node (SIN, new_node_ptr->R);
    MY_ASSERT (new_node_ptr->R->L, "Create_Node ()", FUNC_ERROR, ERROR);

    new_node_ptr->R->L->L = Copy_Tree (node_ptr->L, new_node_ptr->R->L);
    MY_ASSERT (new_node_ptr->R->L->L, "Create_Node ()", FUNC_ERROR, ERROR);

    new_node_ptr->R->R = Differentiate_Inside (node_ptr->L, new_node_ptr->R, var);
    MY_ASSERT (new_node_ptr->R->R, "Differentiate_Inside ()", FUNC_ERROR, ERROR);

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

    new_node_ptr->L = Differentiate_Inside (node_ptr->L, new_node_ptr, var);
    MY_ASSERT (new_node_ptr->L, "Differentiate_Inside ()", FUNC_ERROR, ERROR);

    new_node_ptr->R = Create_Node (POW, new_node_ptr);
    MY_ASSERT (new_node_ptr->R, "Create_Node ()", FUNC_ERROR, ERROR);

    new_node_ptr->R->L = Create_Node (diff_func, new_node_ptr->R);
    MY_ASSERT (new_node_ptr->R->L, "Create_Node ()", FUNC_ERROR, ERROR);
    
    new_node_ptr->R->R = Create_Num_Node (2.0, new_node_ptr->R);
    MY_ASSERT (new_node_ptr->R->R, "Create_Num_Node ()", FUNC_ERROR, ERROR);

    new_node_ptr->R->L->L = Copy_Tree (node_ptr->L, new_node_ptr->R->L);
    MY_ASSERT (new_node_ptr->R->L->L, "Copy_Tree ()", FUNC_ERROR, ERROR);

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

    new_node_ptr->L = Create_Node (MULT, new_node_ptr);
    MY_ASSERT (new_node_ptr->L, "Create_Node ()", FUNC_ERROR, ERROR);

    new_node_ptr->L->L = Create_Num_Node (-1.0, new_node_ptr->L);
    MY_ASSERT (new_node_ptr->L->L, "Create_Node ()", FUNC_ERROR, ERROR);

    new_node_ptr->L->R = Differentiate_Inside (node_ptr->left_son, new_node_ptr->L, var);
    MY_ASSERT (new_node_ptr->L->R, "Differentiate_Inside ()", FUNC_ERROR, ERROR);

    new_node_ptr->R = Create_Node (POW, new_node_ptr);
    MY_ASSERT (new_node_ptr->R, "Create_Node ()", FUNC_ERROR, ERROR);

    new_node_ptr->R->L  = Create_Node (diff_func, new_node_ptr->R);
    MY_ASSERT (new_node_ptr->R->L, "Create_Node ()", FUNC_ERROR, ERROR);

    new_node_ptr->R->R = Create_Num_Node (2.0, new_node_ptr->R);
    MY_ASSERT (new_node_ptr->R->R, "Create_Num_Node ()", FUNC_ERROR, ERROR);

    new_node_ptr->R->L->L = Copy_Tree (node_ptr->L, new_node_ptr->R->L);
    MY_ASSERT (new_node_ptr->R->L->L, "Copy_Tree ()", FUNC_ERROR, ERROR);

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

    new_node_ptr->R = Differentiate_Inside (node_ptr->L, new_node_ptr, var);
    MY_ASSERT (new_node_ptr->R, "Differentiate_Inside ()", FUNC_ERROR, ERROR);

    new_node_ptr->L = Create_Node (DIV, new_node_ptr);
    MY_ASSERT (new_node_ptr->L, "Create_Node ()", FUNC_ERROR, ERROR);

    new_node_ptr->L->L  = Create_Num_Node (multipilier, new_node_ptr->L);
    MY_ASSERT (new_node_ptr->L->L, "Create_Num_Node ()", FUNC_ERROR, ERROR);

    new_node_ptr->L->R = Create_Node (SQRT, new_node_ptr->L);
    MY_ASSERT (new_node_ptr->L->R, "Create_Node ()", FUNC_ERROR, ERROR);

    new_node_ptr->L->R->L = Create_Node (MINUS, new_node_ptr->L->R);
    MY_ASSERT (new_node_ptr->L->R->L, "Create_Node ()", FUNC_ERROR, ERROR);

    new_node_ptr->L->R->L->L = Create_Num_Node (1.0, new_node_ptr->L->R->L);
    MY_ASSERT (new_node_ptr->L->R->L->L, "Create_Num_Node ()", FUNC_ERROR, ERROR);

    new_node_ptr->L->R->L->R = Create_Node (POW, new_node_ptr->L->R->L);
    MY_ASSERT (new_node_ptr->L->R->L->R, "Create_Node ()", FUNC_ERROR, ERROR);

    new_node_ptr->L->R->L->R->L  = Copy_Tree (node_ptr->L, new_node_ptr->L->R->L->R);
    MY_ASSERT (new_node_ptr->L->R->L->R->L, "Copy_Tree ()", FUNC_ERROR, ERROR);

    new_node_ptr->L->R->L->R->R = Create_Num_Node (2.0, new_node_ptr->L->R->L->R);
    MY_ASSERT (new_node_ptr->L->R->L->R->R, "Create_Num_Node ()", FUNC_ERROR, ERROR);

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

    new_node_ptr->R = Differentiate_Inside (node_ptr->L, new_node_ptr, var);
    MY_ASSERT (new_node_ptr->R, "Differentiate_Inside ()", FUNC_ERROR, ERROR);

    new_node_ptr->L = Create_Node (DIV, new_node_ptr);
    MY_ASSERT (new_node_ptr->L, "Create_Node ()", FUNC_ERROR, ERROR);

    new_node_ptr->L->L = Create_Num_Node (multipilier, new_node_ptr->L);
    MY_ASSERT (new_node_ptr->L->L, "Create_Num_Node ()", FUNC_ERROR, ERROR);

    new_node_ptr->L->R = Create_Node (PLUS, new_node_ptr->L);
    MY_ASSERT (new_node_ptr->L->R, "Create_Node ()", FUNC_ERROR, ERROR);

    new_node_ptr->L->R->L = Create_Num_Node (1.0, new_node_ptr->L->R);
    MY_ASSERT (new_node_ptr->L->R->L, "Create_Num_Node ()", FUNC_ERROR, ERROR);

    new_node_ptr->L->R->R = Create_Node (POW, new_node_ptr->L->R);
    MY_ASSERT (new_node_ptr->L->R->R, "Create_Node ()", FUNC_ERROR, ERROR);

    new_node_ptr->L->R->R->L  = Copy_Tree (node_ptr->L, new_node_ptr->L->R->R);
    MY_ASSERT (new_node_ptr->L->R->R->L, "Copy_Tree ()", FUNC_ERROR, ERROR);

    new_node_ptr->L->R->R->R = Create_Num_Node (2.0, new_node_ptr->L->R->R);
    MY_ASSERT (new_node_ptr->L->R->R->R, "Create_Num_Node ()", FUNC_ERROR, ERROR);

    return NO_ERRORS;
}

#undef L
#undef R

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

    else if (LT == NUMBER && Check_If_Const (RT))
        return L_NUM_R_CONST;

    else if (Check_If_Const (LT) && RT == NUMBER)
        return L_CONST_R_NUM;

    else if (Check_If_Const (LT) && Check_If_Const (RT))
        return L_CONST_R_CONST;

    else if (LT == NUMBER && Check_If_Function (RT))
        return L_NUM_R_FUNC;

    else if (Check_If_Function (LT) && RT == NUMBER)
        return L_FUNC_R_NUM;

    else if (Check_If_Const (LT) && Check_If_Function (RT))
        return L_CONST_R_FUNC;

    else if (Check_If_Function (LT) && Check_If_Const (RT))
        return L_FUNC_R_CONST;

    else if (Check_If_Function (LT) && Check_If_Function (RT))
        return L_FUNC_R_FUNC;

    else
        return NO_ERRORS;
}

static bool Check_If_Function (const int node_type)
{
    switch (node_type)
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

        case PLUS:
        case MINUS:
        case MULT:
        case DIV:
        case POW:

        case VARIABLE:

            return true;

        default:
            return false;
    }
}

static bool Check_If_Const (const int node_type)
{
    switch (node_type)
    {
        case PI:
        case E_NUM:
            return true;

        default:
            return false;
    }
}

// ======================================================================================================= //

// ============================================ OPTIMIZATIONS ============================================ //

#define TP  node_ptr->type
#define VAL node_ptr->value

#define L node_ptr->left_son
#define R node_ptr->right_son

#define L_VAL node_ptr->left_son->value
#define R_VAL node_ptr->right_son->value

#define LL node_ptr->left_son->left_son
#define LR node_ptr->left_son->right_son
#define RL node_ptr->right_son->left_son
#define RR node_ptr->right_son->right_son

#define LLT node_ptr->left_son->left_son->type
#define LRT node_ptr->left_son->right_son->type
#define RLT node_ptr->right_son->left_son->type
#define RRT node_ptr->right_son->right_son->type

#define LL_VAL node_ptr->left_son->left_son->value
#define LR_VAL node_ptr->left_son->right_son->value
#define RL_VAL node_ptr->right_son->left_son->value
#define RR_VAL node_ptr->right_son->right_son->value

enum Comparison
{
    LESS = -1,
    EQUAL,
    GREATER
};

static int Optimizer (struct Node **root)
{
    MY_ASSERT (root, "struct Node **root", NULL_PTR, ERROR);

    int n_changes = 0;
    int old_n_changes = 0;
    
    for (;;)
    {
        int CF_status = Check_Fractions (*root);
        MY_ASSERT (CF_status != ERROR, "Check_Fractions ()", FUNC_ERROR, ERROR);
        n_changes += CF_status;

        int S_status = Simplify (root);
        MY_ASSERT (S_status != ERROR, "Simplify ()", FUNC_ERROR, ERROR);
        n_changes = S_status;


        if (n_changes == old_n_changes)
            return NO_ERRORS;
        else
            old_n_changes = n_changes;
    }

    return NO_ERRORS;
}

static int Check_Fractions (struct Node *node_ptr)
{
    static int change_i = 0;

    if (TP == DIV)
    {
        if (LT == MULT && RT == MULT)
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
        else if (LT == DIV && RT == DIV)
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
    else if (TP == MULT)
    {
        if (LT == VARIABLE && RT == DIV && RLT == NUMBER && RRT == VARIABLE)   // x * (num / x) = num
        {
            if (strcmp (L_VAL.str, RR_VAL.str) == 0)
            {
                Turn_Into_Num (node_ptr, RL_VAL.num);
                change_i++;
            }
        }
        else if (RT == VARIABLE && LT == DIV && LLT == NUMBER && LRT == VARIABLE)   // (num / x) * x = num
        {
            if (strcmp (R_VAL.str, LR_VAL.str) == 0)
            {
                Turn_Into_Num (node_ptr, LL_VAL.num);
                change_i++;
            }
        }
    }

    if (L)
        Check_Fractions (L);

    if (R)
        Check_Fractions (R);

    return NO_ERRORS;
}

static int Compare_Trees (const struct Node *tree_1, const struct Node *tree_2)
{
    if (tree_1->type != tree_2->type)
        return 0;
    else if (Compare_Double (tree_1->value.num, tree_2->value.num) != EQUAL)
        return 0;

    if (tree_1->left_son && tree_2->left_son)
    {
        if (Compare_Trees (tree_1->left_son, tree_2->left_son) == 0)
            return 0;
    }
    else if (tree_1->left_son || tree_1->left_son)
        return 0;

    if (tree_1->right_son && tree_2->right_son)
    {
        if (Compare_Trees (tree_1->right_son, tree_2->right_son) == 0)
            return 0;
    }   
    else if (tree_1->right_son || tree_1->right_son)
        return 0;

    return 1;
}

static int Fraction_Reduction (struct Node *root, struct Node *new_left_node, struct Node *new_right_node)
{
    MY_ASSERT (root,           "struct Node *root",       NULL_PTR, ERROR);
    MY_ASSERT (new_left_node,  "struct Node *left_node",  NULL_PTR, ERROR);
    MY_ASSERT (new_right_node, "struct Node *right_node", NULL_PTR, ERROR);
    
    struct Node *left_node = Copy_Tree (new_left_node, root);
    MY_ASSERT (left_node, "struct Node *l_node", NULL_PTR, ERROR);

    struct Node *right_node = Copy_Tree (new_right_node, root);
    MY_ASSERT (right_node, "struct Node *r_node", NULL_PTR, ERROR);

    Tree_Destructor (root->left_son);
    Tree_Destructor (root->right_son);

    root->left_son  = left_node;
    root->right_son = right_node;

    return NO_ERRORS;
}

#undef LL
#undef LR
#undef RL
#undef RR

static int Simplify (struct Node **node_ptr_ptr)
{
    MY_ASSERT (node_ptr_ptr, "struct Node **node_ptr_ptr", NULL_PTR, ERROR);

    struct Node *node_ptr = *node_ptr_ptr;
    MY_ASSERT (node_ptr, "struct Node *node_ptr", NULL_PTR, ERROR);

    static int change_i = 0;

    if (!L && R)
    {
        MY_ASSERT (false, "node_ptr->left_son", NULL_PTR, ERROR);
    }
    else if (L && !R)
    {
        switch (TP)
        {
            case LN:
                if (LT == E_NUM)
                {
                    Turn_Into_Num (node_ptr, 1.0);
                    change_i++;
                }
                break;

            case SIN:
            case TAN:
                if (LT == PI)
                {
                    Turn_Into_Num (node_ptr, 0.0);
                    change_i++;
                }       
                break;
            
            case COS:
                if (LT == PI)
                {
                    Turn_Into_Num (node_ptr, -1.0);
                    change_i++;
                }    
                break;
            
            case COT:
                if (LT == PI)
                    MY_ASSERT (false, "node_ptr->type", UNEXP_VAL, ERROR);
                break;

            default:
                break;
        }

        if (L)
            Simplify (&L);
    }
    else if (L && R)
    {
        if (Check_If_Math_Sign (TP) && (LT == NUMBER && RT == NUMBER))
        {
            double temp = Calc_Expression (node_ptr);

            TP = NUMBER;
            VAL.num = temp;

            Tree_Destructor (L);
            Tree_Destructor (R);

            L = NULL;
            R = NULL;

            change_i++;
        }
        else if (LT == NUMBER && ((TP == MULT && Compare_Double (L_VAL.num, 1.0) == EQUAL) ||   // 1 * f(x) = f(x)
                                  (TP == PLUS && Compare_Double (L_VAL.num, 0.0) == EQUAL)))    // 0 + f(x) = f(x)
        {
            Delete_Neutral_Elem (node_ptr_ptr, R);
            node_ptr->right_son = NULL;
            Tree_Destructor (node_ptr);
            node_ptr = *node_ptr_ptr;
            change_i++;
        }
        else if (RT == NUMBER && (((TP == MULT || TP == DIV)   && Compare_Double (R_VAL.num, 1.0) == EQUAL) ||  // f(x) * 1 = f(x) or f(x) / 1 = f(x) 
                                  ((TP == PLUS || TP == MINUS) && Compare_Double (R_VAL.num, 0.0) == EQUAL) ||  // f(x) + 0 = f(x) or f(x) - 0 = f(x)
                                  ( TP == POW                  && Compare_Double (R_VAL.num, 1.0) == EQUAL)))   // f(x) ^ 1 = f(x)
        {
            Delete_Neutral_Elem (node_ptr_ptr, L);
            node_ptr->left_son = NULL;
            Tree_Destructor (node_ptr);
            node_ptr = *node_ptr_ptr;
            change_i++;
        }
        else if ((LT == NUMBER && (TP == MULT || TP == DIV) && Compare_Double (L_VAL.num, 0.0) == EQUAL) || // 0 * f(x) = 0 or 0 / f(x) = 0
                 (RT == NUMBER &&  TP == MULT               && Compare_Double (R_VAL.num, 0.0) == EQUAL))   // f(x) * 0 = 0
        {
            Turn_Into_Num (node_ptr, 0.0);
            change_i++;
        }
        else if (RT == NUMBER && TP == DIV && Compare_Double (R_VAL.num, 0.0) == EQUAL)     // f(x) / 0 = ERROR
        {
            MY_ASSERT (false, "Right operand of expression", UNEXP_ZERO, ERROR);
        }
        else if ((LT == NUMBER && TP == POW && Compare_Double (L_VAL.num, 1.0) == EQUAL) || // 1 ^ f(x) = 1
                 (RT == NUMBER && TP == POW && Compare_Double (R_VAL.num, 0.0) == EQUAL))   // f(x) ^ 0 = 1
        {
            Turn_Into_Num (node_ptr, 1.0);
            change_i++;
        }

        if (L)
            Simplify (&L);
            
        if (R)
            Simplify (&R);
    }
    
    return change_i;
}

static void Turn_Into_Num (struct Node *node_ptr, const double num)
{   
    node_ptr->type = NUMBER;
    node_ptr->value.num = num;

    if (node_ptr->left_son)
        Tree_Destructor (node_ptr->left_son);

    if (node_ptr->right_son)
        Tree_Destructor (node_ptr->right_son);

    node_ptr->left_son  = NULL;
    node_ptr->right_son = NULL;
}

static void Delete_Neutral_Elem (struct Node **node_ptr_ptr, struct Node *not_neutral)
{
    struct Node *node_ptr = *node_ptr_ptr;
    
    if (node_ptr->parent)
    {
        not_neutral->parent = node_ptr->parent;

        if (node_ptr->parent->left_son == node_ptr)
            (*node_ptr_ptr)->parent->left_son  = not_neutral;
        else
            (*node_ptr_ptr)->parent->right_son = not_neutral;
    }
    else
    {
        not_neutral->parent = NULL;
        *node_ptr_ptr = not_neutral;
    }
}

static double Calc_Expression (const struct Node *node_ptr)
{
    switch (TP)
    {
        case PLUS:
            return (L_VAL.num + R_VAL.num);

        case MINUS:
            return (L_VAL.num - R_VAL.num);

        case MULT:
            return (L_VAL.num * R_VAL.num);

        case DIV:
            if (Compare_Double (R_VAL.num, 0.0) == EQUAL)
                MY_ASSERT (false, "Right operand of expression", UNEXP_ZERO, (double)ERROR);

            return (L_VAL.num / R_VAL.num);

        case POW:
            if (Compare_Double (L_VAL.num, 0.0) == EQUAL && Compare_Double (R_VAL.num, 0.0) == EQUAL)
                MY_ASSERT (false, "Operands of expression", UNEXP_ZERO, (double)ERROR);

            return pow (L_VAL.num, R_VAL.num);

        default: 
            MY_ASSERT (false, "node_ptr->value.sumbol", UNEXP_VAL, (double)ERROR);
    }
}

static bool Check_If_Math_Sign (const int node_type)
{
    switch (node_type)
    {
        case PLUS:
        case MINUS:
        case MULT:
        case DIV:
        case POW:
            return true;

        default:
            return false;
    }
}

static const double EPSILON = 1E-6;

static int Compare_Double (const double first, const double second)
{
    double absolute_value = fabs (first - second);

    if (absolute_value > EPSILON)
        return (first > second) ? GREATER : LESS;
    else
        return EQUAL;
}

// ======================================================================================================= //

// ============================================== TeX DUMP =============================================== //

static int Dump_In_Tex (const struct Node *orig_tree, struct Node **forest, char **vars_arr, const int n_vars)
{
    MY_ASSERT (orig_tree,  "const struct Node *orig_tree", NULL_PTR, ERROR);
    MY_ASSERT (forest,     "const struct Node **forest",   NULL_PTR, ERROR);
    MY_ASSERT (vars_arr,   "const char **vars_arr",        NULL_PTR, ERROR);
    MY_ASSERT (n_vars > 0, "const int n_vars",             POS_VAL,  ERROR);

    system ("mkdir -p output");
    FILE *tex_file = Open_File ("./output/formulas.tex", "wb");

    fprintf (tex_file, "\\documentclass{article}\n"
                       "\\usepackage[utf8]{inputenc}\n\n"
                       "\\begin{document}\n\n");

    fprintf (tex_file, "\t\\[f(");
    for (int i = 0; i < n_vars - 1; i++)
        fprintf (tex_file, "%s, ", vars_arr[i]);
    fprintf (tex_file, "%s) = ", vars_arr[n_vars - 1]);

    Formula_Dump (orig_tree, tex_file);

    fprintf (tex_file, "\\]\n\n");

    for (int i = 0; i < n_vars; i++)
    {
        if (n_vars == 1)
            fprintf (tex_file, "\t\\[f'(%s) = ", vars_arr[i]);
        else
            fprintf (tex_file, "\t\\[\\frac{\\partial f}{\\partial %s} = ", vars_arr[i]);

        Formula_Dump (forest[i], tex_file);

        fprintf (tex_file, "\\]\n");
    }

    fprintf (tex_file, "\n\\end{document}\n");

    Close_File (tex_file, "./output/formulas.txt");

    system ("pdflatex -output-directory=output ./output/formulas.tex");
    system ("xdg-open ./output/formulas.pdf");

    return NO_ERRORS;
}

static int Formula_Dump (const struct Node *node_ptr, FILE *tex_file)
{
    MY_ASSERT (node_ptr, "const struct Node *node_ptr", NULL_PTR, ERROR);
    MY_ASSERT (tex_file, "FILE *tex_file",              NULL_PTR, ERROR);
    
    switch (node_ptr->type)
    {
        case NUMBER:
            if (Compare_Double (node_ptr->value.num, 0.5) == EQUAL)
                fprintf (tex_file, "\\frac{1}{2}");
            else if (Compare_Double (node_ptr->value.num, -0.5) == EQUAL)
                fprintf (tex_file, "\\frac{-1}{2}");
            else
                fprintf (tex_file, "%.f", node_ptr->value.num);
            break;

        case PI:
            fprintf (tex_file, "\\pi");
            break;

        case E_NUM:
            fprintf (tex_file, "e");
            break;

        case VARIABLE:
            fprintf (tex_file, "%s", node_ptr->value.str);
            break;

        case SQRT:
            fprintf (tex_file, "\\%s", Functions_Data_Base[node_ptr->type].name);

            fprintf (tex_file, "{");
            Formula_Dump (node_ptr->left_son, tex_file);
            fprintf (tex_file, "}");

            break;

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

            fprintf (tex_file, "\\%s", Functions_Data_Base[node_ptr->type].name);

            fprintf (tex_file, "(");
            Formula_Dump (node_ptr->left_son, tex_file);
            fprintf (tex_file, ")");

            break;

        case PLUS:

            Formula_Dump (node_ptr->left_son, tex_file);
            fprintf (tex_file, " + ");
            Formula_Dump (node_ptr->right_son, tex_file);

            break;

        case MINUS:

            Formula_Dump (node_ptr->left_son, tex_file);
            fprintf (tex_file, " - ");
            Formula_Dump (node_ptr->right_son, tex_file);

            break;

        case MULT:
            Print_Mult_Operand (node_ptr->left_son, tex_file);
            fprintf (tex_file, " \\cdot ");
            Print_Mult_Operand (node_ptr->right_son, tex_file);
            break;

        case POW:

            Print_Pow_Operand (node_ptr->left_son, tex_file);
            fprintf (tex_file, "^");
            Print_Pow_Operand (node_ptr->right_son, tex_file);

            break;

        case DIV:

            fprintf (tex_file, "\\frac");

            fprintf (tex_file, "{");
            Formula_Dump (node_ptr->left_son, tex_file);
            fprintf (tex_file, "}");

            fprintf (tex_file, "{");
            Formula_Dump (node_ptr->right_son, tex_file);
            fprintf (tex_file, "}");

            break;

        default: MY_ASSERT (false, "node_ptr->type", UNEXP_VAL, ERROR);
    }

    return NO_ERRORS;
}

static int Print_Mult_Operand (const struct Node *node_ptr, FILE *tex_file)
{
    MY_ASSERT (node_ptr, "const struct Node *node_ptr", NULL_PTR, ERROR);
    MY_ASSERT (tex_file, "FILE *tex_file",              NULL_PTR, ERROR);

    switch (node_ptr->type)
    {
        case NUMBER:

            if (node_ptr->value.num < 0)
            {
                fprintf (tex_file, "\\left(");
                Formula_Dump (node_ptr, tex_file);
                fprintf (tex_file, "\\right)");
            }
            else
                Formula_Dump (node_ptr, tex_file);

            break;

        case PI:
        case E_NUM:

        case VARIABLE:

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

        case MULT:
        case DIV:
        case POW:

            Formula_Dump (node_ptr, tex_file);
            break;

        case PLUS:
        case MINUS:

            fprintf (tex_file, "\\left(");
            Formula_Dump (node_ptr, tex_file);
            fprintf (tex_file, "\\right)");

            break;
            
        default:
            MY_ASSERT (false, "node_ptr->type", UNEXP_VAL, ERROR);
    }

    return NO_ERRORS;
}

static int Print_Pow_Operand (const struct Node *node_ptr, FILE *tex_file)
{
    MY_ASSERT (node_ptr, "const struct Node *node_ptr", NULL_PTR, ERROR);
    MY_ASSERT (tex_file, "FILE *tex_file",              NULL_PTR, ERROR);
    
    fprintf (tex_file, "{");

    switch (node_ptr->type)
    {
        case NUMBER:
            if (node_ptr->value.num < 0)
            {
                fprintf (tex_file, "\\left(");
                Formula_Dump (node_ptr, tex_file);
                fprintf (tex_file, "\\right)");
            }
            else
                Formula_Dump (node_ptr, tex_file);
                
            break;

        case PI:
        case E_NUM:

        case VARIABLE:

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
        case PLUS:
        case MINUS:
        case MULT:
        case DIV:
        case POW:
            Formula_Dump (node_ptr, tex_file);
            break;
            
        default:
            MY_ASSERT (false, "node_ptr->type", UNEXP_VAL, ERROR);
    }

    fprintf (tex_file, "}");

    return NO_ERRORS;
}

// ======================================================================================================= //
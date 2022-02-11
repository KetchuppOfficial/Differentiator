#ifndef DIFFERENTIATOR_H_INCLUDED
#define DIFFERENTIATOR_H_INCLUDED

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <ctype.h>
#include <math.h>

const double EPSILON = 10e-4;
const int BUFF_SIZE = 1024;

const void *const DEAD_PTR = NULL;
const int POISON = 2004;

const int MAX_F_SIZE = 10;

enum Types
{
    NUMBER,
    MATH_CONST,
    VARIABLE,
    MATH_SIGN,
    FUNCTION,
    PARANTHESIS
};

union Value
{
    double number;
    char   symbol;
    char   math_const[3];
    char   function[MAX_F_SIZE];
};

struct Token
{
    Types       type;
    union Value value;
};

//***************************************************************************************
//RELATED TO LEXER

struct Argument
{
    const char *const str;
    long              symb_i;
    struct Token      *token_arr;
    int               token_i;
};

int Lexer           (const char *str, const long n_symbs, struct Token *token_arr);
int Get_General     (struct Argument *arg);
int Get_Plus_Minus  (struct Argument *arg);
int Get_Mult_Div    (struct Argument *arg);
int Get_Pow         (struct Argument *arg);
int Get_Parenthesis (struct Argument *arg);
int Get_Number      (struct Argument *arg);
int Get_Var         (struct Argument *arg);
int Get_Function    (struct Argument *arg);
int Check_Function  (const char *func_name);
int Skip_Spaces     (struct Argument *arg);

//***************************************************************************************
//RELATED TO PARSER

struct Node
{
    Types  type;
    union Value value;
    struct Node *left_son;
    struct Node *right_son;
    struct Node *parent;
};

enum Pow_Modes
{
    NUM,
    CONST
};

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

enum Functions_Enum
{
    Sqrt,

    Ln,

    Sin,
    Cos,
    Tan,
    Cot,

    Arcsin,
    Arccos,
    Arctan,
    Arccot,

    Sinh,
    Cosh,
    Tanh,
    Coth
};
//*******************************************************************************************************************
//FUNCTION PROTOTYPES
int Tree_Destructor  (struct Node *node_ptr);

struct Token *Read_Function      (const char *file_name, int *n_tokens);

struct Node  *Plant_Tree    (struct Token *token_arr, const int n_tokens);
struct Node  *Add_Node      (struct Token *token_ptr, struct Node *left_son, struct Node *right_son);
struct Node  *Push_Node_Ptr (struct Stack *node_stack, struct Stack *operator_stack);
struct Token *Pop_Operator  (struct Stack *operator_stack);
int          Check_Priority (const char symb_1, const char symb_2);
int          Add_Function   (struct Argument_2 *arg);

char       *Find_Vars      (struct Node* node_ptr, int *n_vars);
int         Add_Vars       (struct Node* node_ptr, char *vars_arr, int *var_i);
int         Diff_Every_Var (struct Node *root_ptr);
int         Diff_One_Var   (const struct Node *node_ptr, const char var, struct Node *new_node_ptr);
int         Check_Operands (const struct Node *node_ptr, const char var);
struct Node *Create_Node_  (Types node_type, struct Node *parent, int parmN, ...);
struct Node *Copy_Tree     (const struct Node *node_ptr, struct Node *parent);

int Diff_Sum_Sub (const struct Node *node_ptr, const char var, struct Node *new_node_ptr);
int Diff_Mult    (const struct Node *node_ptr, const char var, struct Node *new_node_ptr, const char sum_sub);
int Diff_Div     (const struct Node *node_ptr, const char var, struct Node *new_node_ptr);

int Diff_Sqrt          (const struct Node *node_ptr, const char var, struct Node *new_node_ptr);
int Diff_Pow           (const struct Node *node_ptr, const char var, struct Node *new_node_ptr);
int Diff_Exp           (const struct Node *node_ptr, const char var, struct Node *new_node_ptr);
int Diff_Pow_Exp       (const struct Node *node_ptr, const char var, struct Node *new_node_ptr);
int Diff_Ln            (const struct Node *node_ptr, const char var, struct Node *new_node_ptr);

int Diff_Sin_Sinh_Cosh (const struct Node *node_ptr, const char var, struct Node *new_node_ptr, Functions_Enum mode);
int Diff_Cos           (const struct Node *node_ptr, const char var, struct Node *new_node_ptr);
int Diff_Tan_Tanh      (const struct Node *node_ptr, const char var, struct Node *new_node_ptr, Functions_Enum mode);
int Diff_Cot_Coth      (const struct Node *node_ptr, const char var, struct Node *new_node_ptr, Functions_Enum mode);
int Diff_Arcsin_Arccos (const struct Node *node_ptr, const char var, struct Node *new_node_ptr, Functions_Enum mode);
int Diff_Arctan_Arccot (const struct Node *node_ptr, const char var, struct Node *new_node_ptr, Functions_Enum mode);

int    Check_Fractions    (struct Node *node_ptr);
int    Simplify           (struct Node **node_ptr, const char var);
int    Simplify_Function  (struct Node **root_ptr, const char var);
int    Compare_Trees      (const struct Node *tree_1, const struct Node *tree_2);
double Calc_Expression    (const struct Node *node_ptr);

int Tree_Dump   (struct Node *root_ptr, const char *text_file_name, const char *image_file_name, const char var);
int Node_Dump   (struct Node *node_ptr, FILE *graph_file, const char var);
int Arrows_Dump (struct Node *node_ptr, FILE *graph_file);
int Print_Dump  (const char *text_file_name, const char *image_file_name);

int Dump_In_Tex  (const struct Node *orig_tree, struct Node **tree_arr, const char *vars_arr,
                  const int n_vars, const char *text_file_name);
int Formula_Dump (const struct Node *node_ptr, FILE *tex_file);
//*******************************************************************************************************************

#endif // DIFFERENTIATOR_H_INCLUDED

#ifndef DIFFERENTIATOR_H_INCLUDED
#define DIFFERENTIATOR_H_INCLUDED

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <ctype.h>
#include <math.h>

#include "../include/Stack.h"

#define MAX_NAME_SIZE 10

enum Types
{  
    SQRT,
    LN,
    SIN,
    COS,
    TAN,
    COT,
    ARCSIN,
    ARCCOS,
    ARCTAN,
    ARCCOT,
    SINH,
    COSH,
    TANH,
    COTH,

    NUMBER,
    VARIABLE,

    PI,
    E_NUM,

    PLUS,
    MINUS,
    MULT,
    DIV,
    POW,

    L_PARANTHESIS,
    R_PARANTHESIS,
};

union Value
{
    double num;
    char   str[MAX_NAME_SIZE];
};

struct Token
{
    enum Types  type;
    union Value value;
};

struct Node
{
    enum   Types type;
    union  Value value;
    struct Node *left_son;
    struct Node *right_son;
    struct Node *parent;
};

struct Function
{
    enum Types num;
    const char *name;
};

static const struct Function Functions_Data_Base[] =
{
    {SQRT, "sqrt"},

    {LN, "ln"},

    {SIN, "sin"},
    {COS, "cos"},
    {TAN, "tan"},
    {COT, "cot"},

    {ARCSIN, "arcsin"},
    {ARCCOS, "arccos"},
    {ARCTAN, "arctan"},
    {ARCCOT, "arccot"},

    {SINH, "sinh"},
    {COSH, "cosh"},
    {TANH, "tanh"},
    {COTH, "coth"},
    {-1, ""}
};

struct Token *Lexer (const char *file_name, int *n_tokens);
struct Node *Parser (const struct Token *token_arr, const int n_tokens);
int Tree_Destructor (struct Node *node_ptr);
int Differentiator (struct Node *root);
int Tree_Dump   (struct Node *root_ptr, const char *text_file_name, const char *image_file_name, const char *var);

//*******************************************************************************************************************
//FUNCTION PROTOTYPES
#if 0

int    Check_Fractions    (struct Node *node_ptr);
int    Simplify           (struct Node **node_ptr, const char *var);
int    Simplify_Function  (struct Node **root_ptr, const char *var);
int    Compare_Trees      (const struct Node *tree_1, const struct Node *tree_2);
double Calc_Expression    (const struct Node *node_ptr);

int Dump_In_Tex  (const struct Node *orig_tree, struct Node **tree_arr, char **vars_arr,
                  const int n_vars, const char *text_file_name);
int Formula_Dump (const struct Node *node_ptr, FILE *tex_file);
#endif
//*******************************************************************************************************************

#endif // DIFFERENTIATOR_H_INCLUDED

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

struct Token *Lexer           (const char *buffer, const long n_symbs, int *n_tokens); 
struct Node  *Parser          (const struct Token *token_arr, const int n_tokens);
int           Tree_Destructor (struct Node *node_ptr);
struct Node **Differentiator  (const struct Node *root, int *n_vars);
int           Forest_Dtor     (struct Node **forest, const int n_vars);
int           Tree_Dump       (const struct Node *root_ptr, const char *text_file_name, const char *image_file_name, const char *var);

#endif

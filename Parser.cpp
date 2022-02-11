#ifndef PARSER_H_INCLUDED
#define PARSER_H_INCLUDED

#include "Differentiator.h"
#include "./Stack/Stack.h"
#include "Reading_File.h"
#include "Log_File.h"

#define Universal_If(function)              \
do                                          \
{                                           \
    if (node_1 == NULL)                     \
        function (node_1, token_arr, i);    \
    else                                    \
        function (node_2, token_arr, i);    \
}                                           \
while (0)

#define TOP_TOKEN_IN_STACK ((struct Token *)(operator_stack.data[operator_stack.size - 1]))

//*****************************************************************************************************************
//RELATED TO WORK WITH TREE OF EXPRESSION
int Tree_Destructor (struct Node *node_ptr)
{
    MY_ASSERT (node_ptr, "struct Node *node_ptr", NULL_PTR, ERROR);

    if (node_ptr->left_son)
        Tree_Destructor (node_ptr->left_son);

    if (node_ptr->right_son)
        Tree_Destructor (node_ptr->right_son);

    node_ptr->type = (Types)POISON;

    node_ptr->left_son  = (struct Node *)DEAD_PTR;
    node_ptr->right_son = (struct Node *)DEAD_PTR;

    free (node_ptr);

    return NO_ERRORS;
}

struct Node *Plant_Tree (struct Token *token_arr, const int n_tokens)
{
    struct Stack operator_stack = {};
    struct Stack node_stack = {};

    Stack_Ctor (&operator_stack);
    Stack_Ctor (&node_stack);

    struct Node *curr_root = NULL;

    for (int i = 0; i < n_tokens; i++)
    {
        switch (token_arr[i].type)
        {
            case NUMBER:
            case VARIABLE:
            case MATH_CONST:
                Stack_Push (&node_stack, Add_Node (token_arr + i, NULL, NULL));
                break;

            case FUNCTION:
                Stack_Push (&operator_stack, token_arr + i);
                break;

            case MATH_SIGN:
                if (operator_stack.size == 0 || Check_Priority (token_arr[i].value.symbol, TOP_TOKEN_IN_STACK->value.symbol) > 0)
                {
                    Stack_Push (&operator_stack, token_arr + i);
                }
                else
                {
                    while (operator_stack.size > 0 && TOP_TOKEN_IN_STACK->value.symbol != ')' && TOP_TOKEN_IN_STACK->type != FUNCTION &&
                           Check_Priority (token_arr[i].value.symbol, TOP_TOKEN_IN_STACK->value.symbol) <= 0)
                    {
                        curr_root = Push_Node_Ptr (&node_stack, &operator_stack);
                    }

                    Stack_Push (&operator_stack, token_arr + i);
                }
                break;

            case PARANTHESIS:
                switch (token_arr[i].value.symbol)
                {
                    case '(':
                        Stack_Push (&operator_stack, token_arr + i);
                        break;
                    case ')':
                        while (TOP_TOKEN_IN_STACK->value.symbol != '(')
                            curr_root = Push_Node_Ptr (&node_stack, &operator_stack);

                        Stack_Pop (&operator_stack, NULL);

                        if (operator_stack.size > 0 && TOP_TOKEN_IN_STACK->type == FUNCTION)
                            curr_root = Push_Node_Ptr (&node_stack, &operator_stack);

                        break;

                    default: MY_ASSERT (false, "token_arr[i].value.symbol", UNEXP_VAL, NULL);
                }
                break;

            default: MY_ASSERT (false, "token_arr[i].type", UNEXP_VAL, NULL);
        }
    }

    if (n_tokens > 1)
    {
        while (node_stack.size != 0 && operator_stack.size != 0)
            curr_root = Push_Node_Ptr (&node_stack, &operator_stack);
    }
    else
    {
        void *temp = NULL;
        Stack_Pop (&node_stack, &temp);
        curr_root = (struct Node *)temp;
    }

    Stack_Dtor (&operator_stack);

    Stack_Dtor (&node_stack);

    return curr_root;
}

struct Node *Push_Node_Ptr (struct Stack *node_stack, struct Stack *operator_stack)
{
    MY_ASSERT (node_stack,     "struct Stack *node_stack",     NULL_PTR, NULL);
    MY_ASSERT (operator_stack, "struct Stack *operator_stack", NULL_PTR, NULL);

    void *node_1 = NULL, *node_2 = NULL, *curr_root = NULL;
    struct Token *token_ptr = NULL;

    Stack_Pop (node_stack, &node_1);

    token_ptr = Pop_Operator (operator_stack);

    if (token_ptr->type != FUNCTION)
    {
        Stack_Pop (node_stack, &node_2);

        curr_root = Add_Node (token_ptr, (struct Node *)node_2, (struct Node *)node_1);

        ((struct Node *)node_2)->parent = (struct Node *)curr_root;
    }
    else
        curr_root = Add_Node (token_ptr, (struct Node *)node_1, NULL);

    ((struct Node *)node_1)->parent = (struct Node *)curr_root;

    Stack_Push (node_stack, (struct Node *)curr_root);

    return (struct Node *)curr_root;
}

struct Token *Pop_Operator (struct Stack *operator_stack)
{
    MY_ASSERT (operator_stack, "struct Stack *operator_stack", NULL_PTR, NULL);

    void *ptr = NULL;

    Stack_Pop (operator_stack, &ptr);

    return (struct Token *)ptr;
}

struct Node *Add_Node (struct Token *token_ptr, struct Node *left_son, struct Node *right_son)
{
    MY_ASSERT (token_ptr, "struct Token *token_ptr", NULL_PTR, NULL);

    struct Node *node_ptr = (struct Node *)calloc (1, sizeof (struct Node));

    node_ptr->left_son  = left_son;
    node_ptr->right_son = right_son;
    node_ptr->parent    = NULL;
    node_ptr->type      = token_ptr->type;
    node_ptr->value     = token_ptr->value;

    return node_ptr;
}

#define EQ_Switch(symb, equivalent) \
do                                  \
{                                   \
    switch (symb)                   \
    {                               \
        case '^':                   \
            equivalent = 3;         \
            break;                  \
        case '*':                   \
        case '/':                   \
            equivalent = 2;         \
            break;                  \
        case '+':                   \
        case '-':                   \
            equivalent = 1;         \
            break;                  \
    }                               \
}                                   \
while (0)
int Check_Priority (const char symb_1, const char symb_2)
{
    int equivalent_1 = 0, equivalent_2 = 0;

    EQ_Switch (symb_1, equivalent_1);
    EQ_Switch (symb_2, equivalent_2);

    return (equivalent_1 - equivalent_2);
}

//*****************************************************************************************************************
//RELATED TO GRAPHIC DUMP
int Tree_Dump (struct Node *root_ptr, const char *text_file_name, const char *image_file_name, const char var)
{
    MY_ASSERT (root_ptr,        "struct Node *root_ptr",       NULL_PTR, ERROR);
    MY_ASSERT (text_file_name,  "const char *text_file_name",  NULL_PTR, ERROR);
    MY_ASSERT (image_file_name, "const char *image_file_name", NULL_PTR, ERROR);

    FILE *graph_file = Open_File (text_file_name, "wb");

    fprintf (graph_file, "digraph Tree\n"
                         "{\n"
                         "\trankdir = TB;\n"
                         "\tnode [style = rounded];\n\n");

    Node_Dump (root_ptr, graph_file, var);

    Arrows_Dump (root_ptr, graph_file);

    fprintf (graph_file, "}\n");

    Close_File (graph_file, text_file_name);

    Print_Dump (text_file_name, image_file_name);

    return NO_ERRORS;
}

int Node_Dump (struct Node *node_ptr, FILE *graph_file, const char var)
{
    MY_ASSERT (node_ptr,   "struct Node *node_ptr", NULL_PTR, ERROR);
    MY_ASSERT (graph_file, "FILE *graph_file",      NULL_PTR, ERROR);

    switch (node_ptr->type)
    {
        case NUMBER:
            fprintf (graph_file, "\tnode%p\t[style = filled, fillcolor = limegreen, label = \"%.2f\"];\n\n", node_ptr, node_ptr->value.number);
            break;

        case MATH_SIGN:
            fprintf (graph_file, "\tnode%p\t[style = filled, fillcolor = deepskyblue, label = \"%c\"];\n\n", node_ptr, node_ptr->value.symbol);
            break;

        case VARIABLE:
            if (node_ptr->value.symbol == var)
                fprintf (graph_file, "\tnode%p\t[style = filled, fillcolor = deeppink, label = \"%c\"];\n\n", node_ptr, node_ptr->value.symbol);
            else
                fprintf (graph_file, "\tnode%p\t[style = filled, fillcolor = gold1, label = \"%c\"];\n\n", node_ptr, node_ptr->value.symbol);

            break;

        case FUNCTION:
            fprintf (graph_file, "\tnode%p\t[style = filled, fillcolor = coral, label = \"%s\"];\n\n", node_ptr, node_ptr->value.function);
            break;

        case MATH_CONST:
            if (strcmp (node_ptr->value.math_const, "e") == 0)
                fprintf (graph_file, "\tnode%p\t[style = filled, fillcolor = grey, label = \"%s\"];\n\n", node_ptr, "e");
            else if (strcmp (node_ptr->value.math_const, "pi") == 0)
                fprintf (graph_file, "\tnode%p\t[style = filled, fillcolor = grey, label = \"%s\"];\n\n", node_ptr, "pi");
            break;

        default: MY_ASSERT (false, "node_ptr->type", UNEXP_VAL, ERROR);
    }

    if (node_ptr->left_son)
        Node_Dump (node_ptr->left_son, graph_file, var);

    if (node_ptr->right_son)
        Node_Dump (node_ptr->right_son, graph_file, var);

    return NO_ERRORS;
}

int Arrows_Dump (struct Node *node_ptr, FILE *graph_file)
{
    MY_ASSERT (node_ptr,   "struct Node *node_ptr", NULL_PTR, ERROR);
    MY_ASSERT (graph_file, "FILE *graph_file",      NULL_PTR, ERROR);

    if (node_ptr->left_son)
    {
        fprintf (graph_file, "node%p -> node%p [color = \"blue\"];\n", node_ptr, node_ptr->left_son);
        Arrows_Dump (node_ptr->left_son, graph_file);
    }

    if (node_ptr->right_son)
    {
        fprintf (graph_file, "node%p -> node%p [color = \"gold\"];\n", node_ptr, node_ptr->right_son);
        Arrows_Dump (node_ptr->right_son, graph_file);
    }

    if (node_ptr->parent)
    {
        fprintf (graph_file, "node%p -> node%p [color = \"dimgray\"];\n", node_ptr, node_ptr->parent);
    }

    return NO_ERRORS;
}

int Print_Dump (const char *text_file_name, const char *image_file_name)
{
    MY_ASSERT (text_file_name,  "const char *text_file_name",  NULL_PTR, ERROR);
    MY_ASSERT (image_file_name, "const char *image_file_name", NULL_PTR, ERROR);

    char print_dump[BUFF_SIZE] = "";
    sprintf (print_dump, "dot -Tpng ./%s -o ./%s", text_file_name, image_file_name);
    system (print_dump);

    return NO_ERRORS;
}
//*****************************************************************************************************************

#endif // PARSER_H_INCLUDED

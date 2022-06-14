#include "../include/Differentiator.h"
#include "My_Lib.h"

int Check_Argc (const int argc, const int expected)
{
    return (argc == expected) ? 0 : 1;
}

int main (int argc, char *argv[])
{
    Open_Log_File ("Differentiator");

    MY_ASSERT (Check_Argc (argc, 2) == 0, "Check_Args ()", FUNC_ERROR, ERROR);

    int n_tokens = 0;
    struct Token *token_arr = Lexer (argv[1], &n_tokens);
    MY_ASSERT (token_arr, "Lexer ()", FUNC_ERROR, ERROR);

    struct Node *root = Parser (token_arr, n_tokens);
    MY_ASSERT (root, "Parser ()", FUNC_ERROR, ERROR);

    free (token_arr);

    Tree_Dump (root, "./Output/Expression_Text.dot", "./Output/Function.png", "x");

    Differentiator (root);

    Tree_Destructor (root);

    return 0;
}

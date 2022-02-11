#include "Differentiator.h"
#include "Log_File.h"

const char *EXPRESSION_FILE = "Expression_Text.dot";
const char *TEX_FILE        = "Tex_File.txt";

int main (int argc, char *argv[])
{
    OPEN_LOG_FILE;

    int n_tokens = 0;
    struct Token *token_arr = Read_Function (argv[1], &n_tokens);

    MY_ASSERT (token_arr, "Read_Function ()", FUNC_ERROR, ERROR);

    struct Node *root_ptr = Plant_Tree (token_arr, n_tokens);

    Tree_Dump (root_ptr, EXPRESSION_FILE, "Function.png", 0);

    Diff_Every_Var (root_ptr);

    Tree_Destructor (root_ptr);

    free (token_arr);

    return 0;
}

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

    long n_symbs = 0L;
    char *buffer = Make_File_Buffer (argv[1], &n_symbs);
    MY_ASSERT (buffer, "Make_File_Buffer ()", FUNC_ERROR, ERROR);

    struct Node *root = Plant_Tree (buffer, n_symbs);

    free (buffer);

    int n_vars = 0;
    struct Node **forest = Differentiator (root, &n_vars);
    MY_ASSERT (forest, "Differentiator ()", FUNC_ERROR, ERROR);

    int FD_status = Forest_Dtor (forest, n_vars);
    MY_ASSERT (FD_status , "Forest_Dtor ()", FUNC_ERROR, ERROR);

    Tree_Destructor (root);

    return 0;
}

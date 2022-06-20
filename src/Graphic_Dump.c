#include "../include/Differentiator.h"
#include "My_Lib.h"

static int Node_Dump   (const struct Node *node_ptr, FILE *graph_file, const char *var);
static int Arrows_Dump (const struct Node *node_ptr, FILE *graph_file);
static int Print_Dump  (const char *text_file_name, const char *image_file_name);

int Tree_Dump (const struct Node *root_ptr, const char *text_file_name, const char *image_file_name, const char *var)
{
    MY_ASSERT (root_ptr,        "struct Node *root_ptr",       NULL_PTR, ERROR);
    MY_ASSERT (text_file_name,  "const char *text_file_name",  NULL_PTR, ERROR);
    MY_ASSERT (image_file_name, "const char *image_file_name", NULL_PTR, ERROR);
    MY_ASSERT (var,             "constr char *var",            NULL_PTR, ERROR);

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

static int Node_Dump (const struct Node *node_ptr, FILE *graph_file, const char *var)
{
    MY_ASSERT (node_ptr,   "struct Node *node_ptr", NULL_PTR, ERROR);
    MY_ASSERT (graph_file, "FILE *graph_file",      NULL_PTR, ERROR);
    MY_ASSERT (var,        "constr char *var",      NULL_PTR, ERROR);

    switch (node_ptr->type)
    {
        case NUMBER:
            fprintf (graph_file, "\tnode%p\t[style = filled, fillcolor = limegreen, label = \"%.2f\"];\n\n", node_ptr, node_ptr->value.num);
            break;

        case PLUS:
            fprintf (graph_file, "\tnode%p\t[style = filled, fillcolor = deepskyblue, label = \"+\"];\n\n", node_ptr);
            break;
        
        case MINUS:
            fprintf (graph_file, "\tnode%p\t[style = filled, fillcolor = deepskyblue, label = \"-\"];\n\n", node_ptr);
            break;

        case MULT:
            fprintf (graph_file, "\tnode%p\t[style = filled, fillcolor = deepskyblue, label = \"*\"];\n\n", node_ptr);
            break;

        case DIV:
            fprintf (graph_file, "\tnode%p\t[style = filled, fillcolor = deepskyblue, label = \"/\"];\n\n", node_ptr);
            break;

        case POW:
            fprintf (graph_file, "\tnode%p\t[style = filled, fillcolor = deepskyblue, label = \"^\"];\n\n", node_ptr);
            break;

        case VARIABLE:
            if (strcmp (node_ptr->value.str, var) == 0)
                fprintf (graph_file, "\tnode%p\t[style = filled, fillcolor = deeppink, label = \"%s\"];\n\n", node_ptr, node_ptr->value.str);
            else
                fprintf (graph_file, "\tnode%p\t[style = filled, fillcolor = gold1, label = \"%s\"];\n\n", node_ptr, node_ptr->value.str);

            break;

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
            fprintf (graph_file, "\tnode%p\t[style = filled, fillcolor = coral, label = \"%s\"];\n\n", node_ptr, Functions_Data_Base[node_ptr->type].name);
            break;

        case PI:
            fprintf (graph_file, "\tnode%p\t[style = filled, fillcolor = grey, label = \"pi\"];\n\n", node_ptr);
            break;

        case E_NUM:
            fprintf (graph_file, "\tnode%p\t[style = filled, fillcolor = grey, label = \"e\"];\n\n", node_ptr);
            break;

        default: 
            printf ("node_ptr->type = %d\n", node_ptr->type);
            MY_ASSERT (false, "node_ptr->type", UNEXP_VAL, ERROR);
    }

    if (node_ptr->left_son)
        Node_Dump (node_ptr->left_son, graph_file, var);

    if (node_ptr->right_son)
        Node_Dump (node_ptr->right_son, graph_file, var);

    return NO_ERRORS;
}

static int Arrows_Dump (const struct Node *node_ptr, FILE *graph_file)
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

    size_t str_size = sizeof ("dot -Tpng ./") + strlen (text_file_name) + sizeof (" -o ./") + strlen (image_file_name);

    char *print_dump = (char *)calloc (str_size, sizeof (char));
    sprintf (print_dump, "dot -Tpng ./%s -o ./%s", text_file_name, image_file_name);
    system (print_dump);

    free (print_dump);

    return NO_ERRORS;
}

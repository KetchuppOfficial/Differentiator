#include "Reading_File.h"
#include "Log_File.h"

FILE *Open_File (const char *const file_name, const char *const mode)
{
    MY_ASSERT (file_name, "const char *const file_name", NULL_PTR, NULL);
    MY_ASSERT (mode,      "const char *const mode",      NULL_PTR, NULL);

    FILE *file_ptr = fopen (file_name, mode);

    MY_ASSERT (file_ptr, "FILE *file_ptr", OPEN_ERR, NULL);

    return file_ptr;
}

int Close_File (FILE *file_ptr, const char *const file_name)
{
    MY_ASSERT (file_ptr,  "FILE *file_ptr",        NULL_PTR, ERROR);
    MY_ASSERT (file_name, "const char *file_name", NULL_PTR, ERROR);

    MY_ASSERT (fclose (file_ptr) == 0, "fclose ()", CLOSE_ERR, ERROR);

    return NO_ERRORS;
}

long Define_File_Size (FILE *file_ptr)
{
    MY_ASSERT (file_ptr, "FILE *file_ptr", NULL_PTR, ERROR);

    long start_pos = ftell (file_ptr);

    if (fseek (file_ptr, 0L, SEEK_END))
        MY_ASSERT (0, "fseek ()", FSEEK_ERR, ERROR);

    long n_symbs = ftell (file_ptr);
    MY_ASSERT (n_symbs != -1L, "long n_symbs", FTELL_ERR, ERROR);
    MY_ASSERT (n_symbs > 0L,   "long n_symbs", EMP_FILE, ERROR);

    if (fseek (file_ptr, start_pos, SEEK_SET))
        MY_ASSERT (false, "fseek ()", FSEEK_ERR, ERROR);

    return n_symbs;
}

char *Make_Buffer (FILE *file_ptr, const long n_symbs)
{
    MY_ASSERT (file_ptr,     "FILE *file_ptr",     NULL_PTR, NULL);
    MY_ASSERT (n_symbs > 0L, "const long n_symbs", POS_VAL,  NULL);

    char *buffer = (char *)calloc (n_symbs + 1L, sizeof (char));
    MY_ASSERT (buffer, "char *buffer", NE_MEM, NULL);

    size_t read_symbs = fread (buffer, sizeof (char), n_symbs, file_ptr);

    MY_ASSERT (read_symbs == (size_t)n_symbs, "size_t read_symbs", FILE_WRT, NULL);

    return buffer;
}

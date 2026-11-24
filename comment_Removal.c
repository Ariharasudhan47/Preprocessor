#include "header.h"

/* Removes block and line comments from one line.
   - string and char literals are respected ("//" inside a string stays)
   - the newline is always kept so line structure is preserved
   - processed_line must hold at least strlen(line)+2 bytes             */
void remove_comments(const char *line, char *processed_line, int *in_multi_comment)
{
    int p = 0, i = 0;
    char quote = 0;               /* 0, '"' or '\'' when inside a literal */

    while (line[i] != '\0')
    {
        if (*in_multi_comment)
        {
            if (line[i] == '*' && line[i + 1] == '/')
            {
                *in_multi_comment = 0;
                processed_line[p++] = ' ';   /* comment acts as one space */
                i += 2;
            }
            else
                i++;
        }
        else if (quote)
        {
            processed_line[p++] = line[i];
            if (line[i] == '\\' && line[i + 1] != '\0')
                processed_line[p++] = line[++i];
            else if (line[i] == quote)
                quote = 0;
            i++;
        }
        else
        {
            if (line[i] == '"' || line[i] == '\'')
            {
                quote = line[i];
                processed_line[p++] = line[i++];
            }
            else if (line[i] == '/' && line[i + 1] == '*')
            {
                *in_multi_comment = 1;
                i += 2;
            }
            else if (line[i] == '/' && line[i + 1] == '/')
                break;
            else
                processed_line[p++] = line[i++];
        }
    }

    /* keep the line break even if a comment swallowed it */
    {
        size_t len = strlen(line);
        if (len > 0 && line[len - 1] == '\n' && (p == 0 || processed_line[p - 1] != '\n'))
            processed_line[p++] = '\n';
    }
    processed_line[p] = '\0';
}
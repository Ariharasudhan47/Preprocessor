#include "header.h"


void remove_comments(const char *line, char *processed_line, int *in_multi_comment)
{
    int p = 0, i = 0;
    char quote = 0;              

    while (line[i] != '\0')
    {
        if (*in_multi_comment)
        {
            if (line[i] == '*' && line[i + 1] == '/')
            {
                *in_multi_comment = 0;
                processed_line[p++] = ' ';  
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

   
    {
        size_t len = strlen(line);
        if (len > 0 && line[len - 1] == '\n' && (p == 0 || processed_line[p - 1] != '\n'))
            processed_line[p++] = '\n';
    }
    processed_line[p] = '\0';
}

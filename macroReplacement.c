#include "header.h"

#define MAX       256    
#define ARG       16      
#define NAME      64    
#define ARG_SIZE  512     
#define MAX_DEPTH 16      

static char macro[MAX][NAME], value[MAX][SIZE];
static char param[MAX][ARG][NAME];
static int  type[MAX], nargs[MAX], defined_flag[MAX], expanding[MAX];
static int  count = 0;

static int is_id_start(int c) { return isalpha((unsigned char)c) || c == '_'; }
static int is_id_char(int c)  { return isalnum((unsigned char)c) || c == '_'; }


static void append(char *dst, const char *s)
{
    size_t l = strlen(dst), n = strlen(s);
    if (l >= SIZE - 1) return;
    if (l + n > SIZE - 1) n = SIZE - 1 - l;
    memcpy(dst + l, s, n);
    dst[l + n] = '\0';
}
static void append_char(char *dst, char c)
{
    char t[2];
    t[0] = c; t[1] = '\0';
    append(dst, t);
}
static void trim(char *s)
{
    size_t n = strlen(s), st = 0;
    while (n > 0 && isspace((unsigned char)s[n - 1])) s[--n] = '\0';
    while (s[st] && isspace((unsigned char)s[st])) st++;
    if (st) memmove(s, s + st, strlen(s + st) + 1);
}


static int search(const char *word)
{
    int i;
    for (i = 0; i < count; i++)
        if (defined_flag[i] && strcmp(word, macro[i]) == 0)
            return i;
    return -1;
}
int macro_defined(const char *word) { return search(word) != -1; }
void macro_undef(const char *word)
{
    int i = search(word);
    if (i != -1) defined_flag[i] = 0;
}


static void define_macro(const char *line)
{
    char word[NAME];
    int i = 0, j = 0, index;

    while (isspace((unsigned char)line[i])) i++;
    while (is_id_char(line[i]) && j < NAME - 1) word[j++] = line[i++];
    word[j] = '\0';
    if (j == 0) return;

    index = search(word);
    if (index == -1)
    {
        for (index = 0; index < count; index++)    
            if (!defined_flag[index]) break;
        if (index == count)
        {
            if (count >= MAX) { fprintf(stderr, "Too many macros\n"); return; }
            count++;
        }
    }
    strcpy(macro[index], word);
    defined_flag[index] = 1;
    type[index] = 0;
    nargs[index] = 0;

    if (line[i] == '(')                
    {
        type[index] = 1;
        i++;
        while (line[i] != '\0' && line[i] != ')')
        {
            while (isspace((unsigned char)line[i])) i++;
            j = 0;
            while (is_id_char(line[i]) && j < NAME - 1)
                param[index][nargs[index]][j++] = line[i++];
            param[index][nargs[index]][j] = '\0';
            if (j > 0 && nargs[index] < ARG) nargs[index]++;
            while (isspace((unsigned char)line[i])) i++;
            if (line[i] == ',') i++;
            else if (line[i] != ')' && line[i] != '\0') i++;   
        }
        if (line[i] == ')') i++;
    }
    while (isspace((unsigned char)line[i])) i++;
    strncpy(value[index], line + i, SIZE - 1);
    value[index][SIZE - 1] = '\0';
    trim(value[index]);                
}


static void expand(const char *line, char *output, int depth)
{
    int i = 0;

    while (line[i] != '\0')
    {
        char c = line[i];

        if (c == '"' || c == '\'')                 
        {
            char q = c, d;
            append_char(output, line[i++]);
            while (line[i] != '\0')
            {
                d = line[i++];
                append_char(output, d);
                if (d == '\\' && line[i] != '\0') append_char(output, line[i++]);
                else if (d == q) break;
            }
        }
        else if (isdigit((unsigned char)c))       
        {
            while (is_id_char(line[i]) || line[i] == '.')
                append_char(output, line[i++]);
        }
        else if (is_id_start(c))
        {
            char word[256], body[SIZE];
            int j = 0, index, k;

            while (is_id_char(line[i]) && j < 255) word[j++] = line[i++];
            word[j] = '\0';

            index = search(word);
            if (index == -1 || expanding[index] || depth >= MAX_DEPTH)
            {
                append(output, word);
                continue;
            }

            strcpy(body, value[index]);

            if (type[index])
            {
                char actual[ARG][ARG_SIZE];
                int n = 0, d = 0, len = 0, a;

                k = i;
                while (isspace((unsigned char)line[k])) k++;
                if (line[k] != '(') { append(output, word); continue; }
                k++;

                actual[0][0] = '\0';
                for (; line[k] != '\0'; k++)      
                {
                    char ch = line[k];
                    if (ch == '(') d++;
                    else if (ch == ')')
                    {
                        if (d == 0) break;
                        d--;
                    }
                    else if (ch == ',' && d == 0 && n < nargs[index] - 1)
                    {
                        actual[n][len] = '\0';
                        n++; len = 0;
                        continue;
                    }
                    if (len < ARG_SIZE - 1) actual[n][len++] = ch;
                }
                actual[n][len] = '\0';

                if (line[k] != ')') { append(output, word); continue; }
                if (nargs[index] == 0)
                {
                    trim(actual[0]);
                    if (actual[0][0] != '\0') { append(output, word); continue; }
                }
                else if (n != nargs[index] - 1) { append(output, word); continue; }

                
                for (a = 0; a < nargs[index]; a++)
                {
                    char tmp[SIZE];
                    trim(actual[a]);
                    tmp[0] = '\0';
                    expand(actual[a], tmp, depth + 1);
                    strncpy(actual[a], tmp, ARG_SIZE - 1);
                    actual[a][ARG_SIZE - 1] = '\0';
                }

             
                {
                    char result[SIZE], token[256];
                    int x = 0, t;
                    result[0] = '\0';
                    while (body[x] != '\0')
                    {
                        if (body[x] == '"' || body[x] == '\'')
                        {
                            char q = body[x], dch;
                            append_char(result, body[x++]);
                            while (body[x] != '\0')
                            {
                                dch = body[x++];
                                append_char(result, dch);
                                if (dch == '\\' && body[x] != '\0') append_char(result, body[x++]);
                                else if (dch == q) break;
                            }
                        }
                        else if (is_id_start(body[x]))
                        {
                            t = 0;
                            while (is_id_char(body[x]) && t < 255) token[t++] = body[x++];
                            token[t] = '\0';
                            for (a = 0; a < nargs[index]; a++)
                                if (strcmp(token, param[index][a]) == 0) break;
                            if (a < nargs[index]) append(result, actual[a]);
                            else append(result, token);
                        }
                        else
                            append_char(result, body[x++]);
                    }
                    strcpy(body, result);
                }
                i = k + 1;                       
            }

           
            {
                char sub[SIZE];
                sub[0] = '\0';
                expanding[index] = 1;               
                expand(body, sub, depth + 1);
                expanding[index] = 0;
                append(output, sub);
            }
        }
        else
            append_char(output, line[i++]);
    }
}

void macro_replace(char *line, FILE *fs)
{
    char output[SIZE];
    int i = 0;

    while (line[i] == ' ' || line[i] == '\t') i++;
    if (line[i] == '#')
    {
        int k = i + 1;
        while (line[k] == ' ' || line[k] == '\t') k++;
        if (strncmp(line + k, "define", 6) == 0 && isspace((unsigned char)line[k + 6]))
        {
            define_macro(line + k + 6);
            return;                                  
        }
    }

    output[0] = '\0';
    expand(line, output, 0);
    fputs(output, fs);
}

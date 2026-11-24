#include "header.h"

#define MAX_COND 64

typedef struct {
    int parent;   /* was the enclosing region active?            */
    int taken;    /* has some branch of this #if already been used? */
} Cond;

static int include_depth = 0;

/* Reads the directive name (define, include, ifdef...) of a line that
   starts with '#'. Returns pointer just after the name, name copied to kw. */
static const char *get_directive(const char *line, char *kw)
{
    int i = 0, j = 0;
    while (line[i] == ' ' || line[i] == '\t') i++;
    if (line[i] != '#') { kw[0] = '\0'; return NULL; }
    i++;
    while (line[i] == ' ' || line[i] == '\t') i++;
    while (isalpha((unsigned char)line[i]) && j < 31) kw[j++] = line[i++];
    kw[j] = '\0';
    return line + i;
}

/* reads an identifier from s into name */
static void get_name(const char *s, char *name, int cap)
{
    int j = 0;
    while (isspace((unsigned char)*s)) s++;
    while ((isalnum((unsigned char)*s) || *s == '_') && j < cap - 1)
        name[j++] = *s++;
    name[j] = '\0';
}

/* extracts "file" or <file> into filename; returns 1 on success */
static int get_include_name(const char *s, char *filename, int cap)
{
    char close;
    int j = 0;
    while (isspace((unsigned char)*s)) s++;
    if (*s == '"') close = '"';
    else if (*s == '<') close = '>';
    else return 0;
    s++;
    while (*s && *s != close && j < cap - 1) filename[j++] = *s++;
    filename[j] = '\0';
    return *s == close && j > 0;
}

void process_file(FILE *in, FILE *out)
{
    char line[SIZE];
    char clean[SIZE + 2];
    int multi_line = 0;
    Cond cond[MAX_COND];
    int sp = 0, active = 1;

    while (fgets(line, sizeof(line), in))
    {
        char kw[32], name[256];
        const char *rest;

        remove_comments(line, clean, &multi_line);
        rest = get_directive(clean, kw);

        if (rest && (strcmp(kw, "ifdef") == 0 || strcmp(kw, "ifndef") == 0))
        {
            int val;
            get_name(rest, name, sizeof(name));
            val = macro_defined(name);
            if (kw[2] == 'n') val = !val;
            if (sp < MAX_COND)
            {
                cond[sp].parent = active;
                cond[sp].taken = active && val;
                active = cond[sp].taken;
                sp++;
            }
            continue;
        }
        if (rest && strcmp(kw, "else") == 0)
        {
            if (sp > 0)
            {
                active = cond[sp - 1].parent && !cond[sp - 1].taken;
                cond[sp - 1].taken = 1;
            }
            continue;
        }
        if (rest && strcmp(kw, "endif") == 0)
        {
            if (sp > 0) { sp--; active = cond[sp].parent; }
            continue;
        }

        if (!active)             /* inside a skipped #ifdef/#else block */
            continue;

        if (rest && strcmp(kw, "include") == 0)
        {
            char filename[256];
            if (get_include_name(rest, filename, sizeof(filename)))
            {
                if (include_depth >= MAX_INCLUDE_DEPTH)
                    fprintf(stderr, "Warning: include nesting too deep at '%s'\n", filename);
                else
                {
                    FILE *inc = fopen(filename, "r");
                    if (inc)
                    {
                        include_depth++;
                        process_file(inc, out);
                        include_depth--;
                        fclose(inc);
                    }
                    else
                        fprintf(stderr, "Warning: cannot open include file '%s'\n", filename);
                }
                continue;
            }
        }

        if (rest && strcmp(kw, "undef") == 0)
        {
            get_name(rest, name, sizeof(name));
            macro_undef(name);
            continue;
        }

        macro_replace(clean, out);   /* handles #define and normal lines */
    }

    if (sp != 0)
        fprintf(stderr, "Warning: unterminated #if/#ifdef block\n");
}
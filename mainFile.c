#include "header.h"

int main(int argc, char **argv)
{
    FILE *fp, *fs;

    if (argc != 3)
    {
        fprintf(stderr, "Usage: %s inputfilename outputfilename\n", argv[0]);
        return 1;
    }
    fp = fopen(argv[1], "r");
    if (fp == NULL)
    {
        fprintf(stderr, "File not present: %s\n", argv[1]);
        return 1;
    }
    fs = fopen(argv[2], "w");
    if (fs == NULL)
    {
        fprintf(stderr, "Cannot create output file: %s\n", argv[2]);
        fclose(fp);
        return 1;
    }

    process_file(fp, fs);

    fclose(fp);
    fclose(fs);
    return 0;
}
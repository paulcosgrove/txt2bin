/* 
 * txt2bin -- A program to convert an ASCII text file to a binary file
 *
 * For usage information type:
 *
 *    txtbin -h
 * 
 * Copyright 2014, Paul Cosgrove
 *
 */
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>
#include <math.h>

#define MAX_LINE_LENGTH     4096
#define NIBBLES_PER_BYTE    2
#define BYTE_MASK           0xff
#define BITS_PER_BYTE       8

/* The name of the program, set from agv0 */
const char *program_name;

static void print_usage(FILE *stream);
static void print_bytes(unsigned int value, int nbytes);
static char *skip_whitespace(const char *s);
static char *skip_non_whitespace(const char *s);
static char *skip_line(const char *s);
static bool string_has_prefix(const char *s, const char *prefix);
static bool hex_string_to_int(const char *s, unsigned int *value, int *width);

static void convert_file(FILE *file)
{
    char buffer[MAX_LINE_LENGTH];

    while (fgets(buffer, sizeof(buffer), file))
    {
        const char *next = buffer;
        unsigned int value;
        int width;

        while (*next != '\0')
        {
            /* Skip leading whitespace */
            next = skip_whitespace(next);

            /* Skip comments */
            if (*next == '#')
            {
                next = skip_line(next);
                continue;
            }

            /* Try to parse a hexadecimal value */
            if (hex_string_to_int(next, &value, &width))
            {
                /* Output the bytes on stdout */
                int byte_count = (int) ceil((double) width / NIBBLES_PER_BYTE);
                print_bytes(value, byte_count);
            }
            next = skip_non_whitespace(next);
        }
    }
}

int main(int argc, char **argv)
{
    FILE *input_file = NULL;
    int c = -1;

    /* Store the name of this executable */
    program_name = argv[0];

    /* Parse command line options */
    while ((c = getopt(argc, argv, "h")) != -1)
    {
        switch (c)
        {
            case 'h':
                /* Display help */
                print_usage(stdout);
                exit(EXIT_SUCCESS);
            case '?':
                /* Unknown option */
                print_usage(stderr);
                exit(EXIT_FAILURE);
            default:
                abort();
        }
    }

    /* Parse remaining arguments */

    if (argc - optind > 1)
    {
        fprintf(stderr, 
                "%s: too many arguments\n",
                program_name);
        exit(EXIT_FAILURE);
    }

    input_file = stdin;
    if (optind < argc)
    {
        const char *input_filename = argv[optind];
        if (strcmp(input_filename, "-") != 0)
        {
            input_file = fopen(input_filename, "r");
            if (input_file == NULL)
            {
                fprintf(stderr, 
                        "unable to open file '%s' for reading\n",
                        input_filename);
                exit(EXIT_SUCCESS);
            }
        }
    }

    convert_file(input_file);

    fclose(input_file);

    return EXIT_SUCCESS;
}

static void print_usage(FILE *stream)
{
    fprintf(stream, "Usage: %s [OPTIONS] [FILE]\n", program_name);
    fprintf(stream, "Options:\n");
    fprintf(stream, "    -h    Display help\n");
    fprintf(stream, "\n");
    fprintf(stream, "If FILE is missing or is '-', the program will read from stdin.\n");
}

static void print_bytes(unsigned int value, int count)
{
    int i;
    for (i = 0; i < count; i++)
    {
        putchar(value>>(i * BITS_PER_BYTE) & BYTE_MASK);
    }
}

static char *skip_whitespace(const char *s)
{
    while (isspace(*s)) s++;
    return (char *) s;
}
    
static char *skip_non_whitespace(const char *s)
{
    while (*s != '\0' && !isspace(*s)) s++;
    return (char *) s;
}

static char *skip_line(const char *s)
{
    while (*s != '\0') s++;
    return (char *) s;
}

static bool string_has_prefix(const char *s, const char *prefix)
{
    return strncmp(s, prefix, strlen(prefix));
}

static bool hex_string_to_int(const char *s, unsigned int *value, int *width)
{
    int items_scanned = sscanf(s, "%x%n", value, width); 
    if (string_has_prefix(s, "0x"))
    {
        width -= 2;
    }
    return items_scanned == 1;
}


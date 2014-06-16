/* 
 * txt2bin -- A program to create binary files using ASCII
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
#include <assert.h>

#define NIBBLES_PER_BYTE        2
#define BITS_PER_BYTE           8
#define BYTE_MASK               0xff

#define MAX_LINE_LENGTH         4096
#define COMMENT_CHAR            '#'
#define QUOTE_CHAR              '"'
#define BIG_ENDIAN_CHAR         '^'
#define LITTLE_ENDIAN_CHAR      '_'
#define BIG_ENDIAN_KEYWORD      "BIG_ENDIAN"
#define LITTLE_ENDIAN_KEYWORD   "LITTLE_ENDIAN"

enum byte_order 
{
    BIG_ENDIAN_BYTE_ORDER, 
    LITTLE_ENDIAN_BYTE_ORDER
}; 

/* The name of this program */
const char *program_name;

static void print_usage(FILE *stream);
static void print_bytes(unsigned int value, int count, enum byte_order byte_order);
static int  is_word_end(char c);
static bool keyword(const char *s, const char *keyword, const char **rest);
static bool quoted_text(const char *s, char quote_char, const char **rest);
static char *skip_whitespace(const char *s);
static char *skip_non_whitespace(const char *s);
static char *skip_line(const char *s);

static void convert_file(FILE *file)
{
    char buffer[MAX_LINE_LENGTH];
    enum byte_order default_byte_order = LITTLE_ENDIAN_BYTE_ORDER;

    while (fgets(buffer, sizeof(buffer), file))
    {
        const char *look = buffer;
        const char *rest = NULL;

        while (*look != '\0')
        {
            if (isspace(*look))
            {
                /* 
                 * Skip whitespace.
                 */
                look = skip_whitespace(look);
            }
            else if (*look == COMMENT_CHAR)
            {
                /* 
                 * Skip comments.
                 */
                look = skip_line(look);
            }
            else if (quoted_text(look, QUOTE_CHAR, &rest))
            {
                /* 
                 * Output quoted text after adjusting the results to exclude 
                 * the quotes.
                 */
                printf("%.*s", (rest - 1) - (look + 1), look + 1); 
                look = rest;
            }
            else if (keyword(look, BIG_ENDIAN_KEYWORD, &rest))
            {
                /* 
                 * Set the default endianness to big-endian 
                 */
                default_byte_order = BIG_ENDIAN_BYTE_ORDER;
                look = rest;
            }
            else if (keyword(look, LITTLE_ENDIAN_KEYWORD, &rest))
            {
                /* 
                 * Set the default endianness to little-endian
                 */
                default_byte_order = LITTLE_ENDIAN_BYTE_ORDER;
                look = rest;
            }
            else
            {
                /* 
                 * Finally, try to parse a hex string.
                 */
                enum byte_order byte_order = default_byte_order;
                unsigned int hex_string_value;
                int hex_string_width;

                /* 
                 * Check for a byte order prefix on the hex value. If
                 * found, override the default endianness for this value only.
                 */
                if (*look == BIG_ENDIAN_CHAR)
                {
                    byte_order = BIG_ENDIAN_BYTE_ORDER;
                    look++;
                }
                else if (*look == LITTLE_ENDIAN_CHAR)
                {
                    byte_order = LITTLE_ENDIAN_BYTE_ORDER;
                    look++;
                }

                /* 
                 * Skip optional '0x' hex prefix to prevent it from being 
                 * included in the result of sscanf's "%n" conversion specifier. 
                 */  
                if (strncmp("0x", look, 2) == 0)
                {
                    look += 2;
                }

                /* Parse hex string */
                if (sscanf(look, "%x%n", &hex_string_value, &hex_string_width) == 1)
                {
                    /* 
                     * Determine the number of bytes to output from the number 
                     * of hex digits in the hex string. 
                     */
                    int byte_count = (int) ceil((double) hex_string_width / 
                            NIBBLES_PER_BYTE);

                    print_bytes(hex_string_value, byte_count, byte_order);
                }
                look = skip_non_whitespace(look);
            }
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

static void print_bytes(unsigned int value, int count, enum byte_order byte_order)
{
    int i;
    if (byte_order == LITTLE_ENDIAN_BYTE_ORDER)
    {
        for (i = 0; i < count; i++)
        {
            putchar((value >> (i * BITS_PER_BYTE)) & BYTE_MASK);
        }
    }
    else
    {
        for (i = count-1; i >= 0; i--)
        {
            putchar((value >> (i * BITS_PER_BYTE)) & BYTE_MASK);
        }
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

static int is_word_end(char c)
{
    return c == '\0' || !isgraph(c);
}

static bool keyword(const char *s, const char *keyword, const char **rest)
{
    size_t keyword_length = strlen(keyword);

    if (strncmp(s, keyword, keyword_length) == 0)
    {
        s += keyword_length;
        if (is_word_end(*s))
        {
            if (rest) *rest = s;
            return true;
        }
    }
    return false;
}

static bool quoted_text(const char *s, char quote_char, const char **rest)
{
    if (*s == quote_char)
    {
        const char *end_quote = strchr(s + 1, quote_char);
        if (end_quote)
        {
            if (rest) *rest = end_quote + 1;
            return true;
        }
    }
    return false;
}


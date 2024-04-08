/*
 * Character Set Validator for C/C++ Code.
 *
 * Copyright (C) 2024 Julian Kraemer
 *
 * Distributed under MIT license.
 * See LICENSE file for details or copy at https://opensource.org/licenses/MIT
 */

#include "cargs.h"

#include <locale.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define PROGRAM_NAME    "cvc"
#define VERSION         "0.1.0-alpha"

#define CHUNK_SIZE      (16U * 1024U)
#define MAX_VALID_CHAR  (126 + 1)

#define CHAR_CODE_HT        (9)
#define CHAR_CODE_LF        (10)
#define CHAR_CODE_VT        (11)
#define CHAR_CODE_FF        (12)
#define CHAR_CODE_CR        (13)
#define CHAR_CODE_DOLLAR    (36)
#define CHAR_CODE_AT        (64)
#define CHAR_CODE_BACKTICK  (96)

typedef enum
{
    EOL_AUTO_NA, /* Not available, placeholder. */
    EOL_CR,  /* CR = Carriage Return = \r, "Mac" */
    EOL_LF,  /* LF = Line Feed = \n, "Unix/Linux" */
    EOL_CRLF /* Sequence of CR and LF, "Windows" */
} eol_t;

enum
{
    RETURN_VALID = 0,
    RETURN_INVALID,
    RETURN_ERROR_EOL,
    RETURN_ERROR_UNSPECIFIC,
    RETURN_ERROR_INPUT,
    RETURN_ERROR_OPTIONS
};

enum
{
    ARG_ID_FILE,
    ARG_ID_EOL,
    ARG_ID_FF,
    ARG_ID_VT,
    ARG_ID_APA,
    ARG_ID_NOHT,
    ARG_ID_VERBOSE,
    ARG_ID_VERSION,
    ARG_ID_HELP
};

const struct cag_option options[] =
{
    {
        .identifier = ARG_ID_FILE,
        .access_letters = "f",
        .access_name = "file",
        .value_name = "FILE",
        .description = "Specify a file (default: n/a)"
    },
    {
        .identifier = ARG_ID_EOL,
        .access_letters = "e",
        .access_name = "eol",
        .value_name = "LF/CRLF/CR/AUTO",
        .description = "End-of-line indicator (default: AUTO)"
    },
    {
        .identifier = ARG_ID_NOHT,
        .access_letters = NULL,
        .access_name = "noht",
        .value_name = NULL,
        .description = "Forbid horizontal tab character"
    },
    {
        .identifier = ARG_ID_FF,
        .access_letters = NULL,
        .access_name = "ff",
        .value_name = NULL,
        .description = "Permit form feed character"
    },
    {
        .identifier = ARG_ID_VT,
        .access_letters = NULL,
        .access_name = "vt",
        .value_name = NULL,
        .description = "Permit vertical tab character"
    },
    {
        .identifier = ARG_ID_APA,
        .access_letters = "a",
        .access_name = "all",
        .value_name = NULL,
        .description = "Permit all printable ASCII characters"
    },
    {
        .identifier = ARG_ID_VERBOSE,
        .access_letters = "v",
        .access_name = "verbose",
        .value_name = NULL,
        .description = "Enable verbose output"
    },
    {
        .identifier = ARG_ID_HELP,
        .access_letters = "h",
        .access_name = "help",
        .value_name = NULL,
        .description = "Show the command help"
    },
    {
        .identifier = ARG_ID_VERSION,
        .access_letters = NULL,
        .access_name = "version",
        .value_name = NULL,
        .description = "Show the program version"
    }
};

static eol_t
determine_eol(const char* p)
{
    eol_t eol = EOL_AUTO_NA;

    for (; *p != '\0'; p++)
    {
        if (*p == '\n')
        {
            if (eol == EOL_CR)
            {
                eol = EOL_CRLF;
                break;
            }
            else
            {
                return EOL_LF;
            }
        }
        else if (*p == '\r')
        {
            eol = EOL_CR;
        }
        else
        {
            if (eol == EOL_CR)
            {
                break;
            }
        }
    }

    return eol;
}

static unsigned int
validate_eol(const char* buf, eol_t eol)
{
    unsigned int line = 1U;
    char eol_chars[2];
    char eol_invalid;
    int eol_len;

    switch (eol)
    {
        case EOL_CR:
            eol_chars[0] = '\r';
            eol_invalid = '\n';
            eol_len = 1;
            break;
        case EOL_LF:
            eol_chars[0] = '\n';
            eol_invalid = '\r';
            eol_len = 1;
            break;
        case EOL_CRLF:
            eol_chars[0] = '\r';
            eol_chars[1] = '\n';
            eol_invalid = '\n';
            eol_len = 2;
            break;
        default: /* EOL_NA, no end-of-line indicator in string */
            return 0U;
    }

    const char* p = buf;
    while (*p != '\0')
    {
        if (*p == eol_chars[0])
        {
            if (eol_len == 1)
            {
                line++;
                p++;
            }
            else /* len == 2 */
            {
                if (*(p + 1) == eol_chars[1])
                {
                    line++;
                    p += eol_len;
                }
                else
                {
                    return line;
                }
            }
        }
        else if (*p == eol_invalid)
        {
            return line;
        }
        else
        {
            p++;
        }
    }

    return 0U;
}

static inline bool
is_eol(const char* p, eol_t eol)
{
    switch (eol)
    {
        case EOL_CR:
            if (*p == '\r')
            {
                return true;
            }
            break;
        case EOL_LF:
            if (*p == '\n')
            {
                return true;
            }
            break;
        case EOL_CRLF:
            if ((*p == '\r') && (*(p + 1) == '\n'))
            {
                return true;
            }
            break;
        default:
            return false;
    }

    return false;
}

static void
show_usage(void)
{
    printf("Usage: "PROGRAM_NAME" [OPTION]...\n\n");
    cag_option_print(options, CAG_ARRAY_SIZE(options), stdout);
}

static void
show_help(void)
{
    show_usage();
    printf("\nCharacter Set Validator for C/C++ Source Code\n\n\
"PROGRAM_NAME" reads from standard output if no file given.\n\
"PROGRAM_NAME" determines EOL indicator if no eol specified (EOL NA).\n\
"PROGRAM_NAME" checks for consistent EOL prior validation.\n\n\
Exit codes:\n");
    printf("%d = input passed validation\n", RETURN_VALID);
    printf("%d = input failed validation \n", RETURN_INVALID);
    printf("%d = EOL indicator mismatch\n", RETURN_ERROR_EOL);
    printf("%d = unspecific error\n", RETURN_ERROR_UNSPECIFIC);
    printf("%d = input error, e.g., file could not be read\n", RETURN_ERROR_INPUT);
    printf("%d = invalid option\n", RETURN_ERROR_OPTIONS);
}

static void
show_version(void)
{
    printf(PROGRAM_NAME" "VERSION"\n\n\
Copyright (C) 2024 Julian Kraemer\n\
MIT license <https://github.com/piscilus/cvc/blob/main/LICENSE>\n\n\
Get latest version of "PROGRAM_NAME" from <https://github.com/piscilus/cvc>\n");
}

int
main(int argc, char** argv)
{
    setlocale(LC_ALL, "");

    eol_t eol = EOL_AUTO_NA;
    bool verbose = false;
    bool valid_chars[MAX_VALID_CHAR] = {0};

    /* preset range of printable ASCII characters */
    for (size_t i = 0x20U; i < MAX_VALID_CHAR; i++)
    {
        valid_chars[i] = true;
    }
    /* preset range of control characters */
    valid_chars[CHAR_CODE_HT] = true;
    valid_chars[CHAR_CODE_LF] = true;
    valid_chars[CHAR_CODE_VT] = false;
    valid_chars[CHAR_CODE_FF] = false;
    valid_chars[CHAR_CODE_CR] = true;
    valid_chars[CHAR_CODE_DOLLAR] = false;
    valid_chars[CHAR_CODE_AT] = false;
    valid_chars[CHAR_CODE_BACKTICK] = false;

    cag_option_context context;
    const char* file = NULL;

    cag_option_prepare(&context, options, CAG_ARRAY_SIZE(options), argc, argv);
    while (cag_option_fetch(&context))
    {
        char identifier = cag_option_get(&context);
        switch (identifier)
        {
            case ARG_ID_FILE:
                file = cag_option_get_value(&context);
                break;
            case ARG_ID_FF:
                valid_chars[CHAR_CODE_FF] = true;
                break;
            case ARG_ID_VT:
                valid_chars[CHAR_CODE_VT] = true;
                break;
            case ARG_ID_NOHT:
                valid_chars[CHAR_CODE_HT] = false;
                break;
            case ARG_ID_APA:
                valid_chars[CHAR_CODE_DOLLAR] = true;
                valid_chars[CHAR_CODE_AT] = true;
                valid_chars[CHAR_CODE_BACKTICK] = true;
                break;
            case ARG_ID_VERBOSE:
                verbose = true;
                break;
            case ARG_ID_EOL:
            {
                const char* eol_opt = cag_option_get_value(&context);
                if (eol_opt != NULL)
                {
                    if (strcmp(eol_opt, "LF") == 0)
                    {
                        eol = EOL_LF;
                        break;
                    }
                    else if (strcmp(eol_opt, "CRLF") == 0)
                    {
                        eol = EOL_CRLF;
                        break;
                    }
                    else if (strcmp(eol_opt, "CR") == 0)
                    {
                        eol = EOL_CRLF;
                        break;
                    }
                    else if (strcmp(eol_opt, "AUTO") == 0)
                    {
                        eol = EOL_AUTO_NA;
                        break;
                    }
                }
                fprintf(stderr, "Error: EOL not supported!\n");
                show_usage();
                exit(RETURN_ERROR_OPTIONS);
            }
            case ARG_ID_VERSION:
                show_version();
                exit(EXIT_SUCCESS);
            case ARG_ID_HELP:
                show_help();
                exit(EXIT_SUCCESS);
            default:
                fprintf(stderr, "Error: invalid option!\n");
                show_usage();
                exit(RETURN_ERROR_OPTIONS);
        }
    }

    FILE* in_stream;
    if (file != NULL)
    {
        if ((in_stream = fopen(file, "rb")) == NULL)
        {
            fprintf(stderr, "Error: Failed to open file '%s'!\n", file);
            exit(RETURN_ERROR_INPUT);
        }
        else
        {
            if (verbose)
            {
                printf("file %s:\n", file);
            }
        }
    }
    else
    {
        in_stream = stdin;
    }

    char* data = NULL;
    size_t total_size = 0U;
    size_t bytes_read;
    char buf[CHUNK_SIZE + 1U];
    while ((bytes_read = fread(buf, sizeof(char), CHUNK_SIZE, in_stream)) > 0U)
    {
        buf[bytes_read] = '\0';
        data = realloc(data, total_size + bytes_read + 1U);
        if (data == NULL)
        {
            fprintf(stderr, "Error: Memory allocation failed!\n");
            if (file != NULL)
            {
                fclose(in_stream);
            }
            exit(RETURN_ERROR_UNSPECIFIC);
        }
        memcpy(data + total_size, buf, bytes_read);
        total_size += bytes_read;
    }

    if (file != NULL)
    {
        fclose(in_stream);
    }

    if (total_size > 0U)
    {
        data[total_size] = '\0';
    }
    else
    {
        if (verbose)
        {
            printf("Empty input/file.\n");
        }
        exit(EXIT_SUCCESS);
    }

    /* If NA is given, the EOL shall be determined automatically */
    eol_t e = eol;
    if (eol == EOL_AUTO_NA)
    {
        e = determine_eol(data);
    }

    unsigned int result = validate_eol(data, e);
    if (result != 0U)
    {
        if (verbose)
        {
            fprintf(stderr,
                    "Error: Unexpected end-of-line indicator in line %u!\n",
                    result);
        }
        free(data);
        exit(RETURN_ERROR_EOL);
    }

    unsigned int line = 1U;
    unsigned int last_line = 0U;
    unsigned int errors = 0U;
    for (char* p = data; *p != '\0'; p++)
    {
        if (is_eol(p, e))
        {
            if (e == EOL_CRLF)
            {
                p++; /* skip the second EOL character */
            }
            if (last_line == line)
            {
                putchar('\n');
            }
            line++;
        }
        else if ((*p < 0) || (*p > 126) || (!valid_chars[(int)*p]))
        {
            if (verbose)
            {
                if (line != last_line)
                {
                    printf("line %u:", line);
                    last_line = line;
                }
                printf(" 0x%02X (%c)", (unsigned char)*p, *p);
            }
            errors++;
        }
    }

    free(data);

    printf("%u\n", errors);

    return (errors == 0U) ? RETURN_VALID : RETURN_INVALID;
}

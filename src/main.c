/*
 * Character Set Validator for C/C++ Code.
 *
 * (C) Copyright 2024 "piscilus" Julian Kraemer
 *
 * Distributed under MIT license.
 * See LICENSE file for details or copy at https://opensource.org/licenses/MIT
 */

#include "cargs.h"

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define VERSION ("0.1.0-alpha")

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
    EOL_NA,  /* Not available, placeholder. */
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
    RETURN_ERROR_PARAMETER
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
        .value_name = "LF/CRLF/CR/NA",
        .description = "End-of-line indicator (default: NA)"
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
        .access_letters = NULL,
        .access_name = "apa",
        .value_name = NULL,
        .description = "Permit all printable ASCII characters"
    },
    {
        .identifier = ARG_ID_NOHT,
        .access_letters = NULL,
        .access_name = "noht",
        .value_name = NULL,
        .description = "Forbid horizontal tab character"
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
determine_eol(const char* buf)
{
    eol_t eol = EOL_NA;

    for (; *buf != '\0'; buf++)
    {
        if (*buf == '\n')
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
        else if (*buf == '\r')
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
is_eol(const char* buf, eol_t eol)
{
    switch (eol)
    {
        case EOL_CR:
            if (*buf == '\r')
            {
                return true;
            }
            break;
        case EOL_LF:
            if (*buf == '\n')
            {
                return true;
            }
            break;
        case EOL_CRLF:
            if ((*buf == '\r') && (*(buf + 1) == '\n'))
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
    printf("Usage: cvc [OPTION]...\n");
    printf("Character Set Validator for C/C++ Source Code.\n\n");
    cag_option_print(options, CAG_ARRAY_SIZE(options), stdout);
    printf("\nGet latest version from: https://github.com/piscilus/cvc\n");
    printf("\nWith no FILE, read standard input.\n");
    printf("\n_Exit Codes_\n");
    printf("%d: valid\n", RETURN_VALID);
    printf("%d: validation \n", RETURN_INVALID);
    printf("%d: EOL indicator mismatch\n", RETURN_ERROR_EOL);
    printf("%d: unspecific error\n", RETURN_ERROR_UNSPECIFIC);
    printf("%d: input error, e.g., file could not be read\n", RETURN_ERROR_INPUT);
    printf("%d: invalid parameter\n", RETURN_ERROR_PARAMETER);
}

int
main(int argc, char** argv)
{
    eol_t eol = EOL_NA;
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
                if (strcmp(eol_opt, "LF") == 0)
                {
                    eol = EOL_LF;
                }
                else if (strcmp(eol_opt, "CRLF") == 0)
                {
                    eol = EOL_CRLF;
                }
                else if (strcmp(eol_opt, "CR") == 0)
                {
                    eol = EOL_CRLF;
                }
                else if (strcmp(eol_opt, "NA") == 0)
                {
                    eol = EOL_NA;
                }
                else
                {
                    fprintf(stderr, "Error: EOL '%s' not supported!\n", eol_opt);
                    return RETURN_ERROR_PARAMETER;
                }
                break;
            }
            case ARG_ID_VERSION:
                printf("%s\n", VERSION);
                return EXIT_SUCCESS;
            case ARG_ID_HELP:
                show_usage();
                return EXIT_SUCCESS;
            default:
                return RETURN_ERROR_PARAMETER;
        }
    }

    FILE* in_stream;
    if (file != NULL)
    {
        if ((in_stream = fopen(file, "rb")) == NULL)
        {
            fprintf(stderr, "Failed to open file %s!\n", file);
            return RETURN_ERROR_INPUT;
        }
        else
        {
            if (verbose)
            {
                printf("file: %s\n", file);
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
            fprintf(stderr, "Memory allocation failed!\n");
            if (file != NULL)
            {
                fclose(in_stream);
            }
            return RETURN_ERROR_UNSPECIFIC;
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
        return EXIT_SUCCESS;
    }

    /* If NA is given, the EOL shall be determined automatically */
    eol_t e = eol;
    if (eol == EOL_NA)
    {
        e = determine_eol(data);
    }

    unsigned int result = validate_eol(data, e);
    if (result != 0U)
    {
        if (verbose)
        {
            fprintf(stderr,
                    "Unexpected end-of-line indicator in line %u!\n",
                    result);
        }
        free(data);
        return RETURN_ERROR_EOL;
    }

    unsigned int line = 1U;
    unsigned int last_line = 0U;
    unsigned int errors = 0U;
    for (char* s = data; *s != '\0'; s++)
    {
        if (is_eol(s, e))
        {
            if (e == EOL_CRLF)
            {
                s++; /* skip the second EOL character */
            }
            if (last_line == line)
            {
                putchar('\n');
            }
            line++;
        }
        else if ((*s < 0) || (*s > 126) || (!valid_chars[(int)*s]))
        {
            if (verbose)
            {
                if (line != last_line)
                {
                    printf("line %u:", line);
                    last_line = line;
                }
                printf(" 0x%02X (%c)", (unsigned char)*s, *s);
            }
            errors++;
        }
    }

    free(data);

    printf("%u\n", errors);

    return (errors == 0U) ? RETURN_VALID : RETURN_INVALID;
}

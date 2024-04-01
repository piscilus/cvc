# Character Set Validator for C/C++ Code

The **cvc** tool can be used to check whether C/C++ source files only contain
characters from the *basic character set* as defined in the C standard.

## Motivation

The source code should consist of the basic source character set primarily for
reasons of portability, compatibility and security.

Compilers are designed to interpret and process code written using the basic
source character set. Deviating from this set may lead to unexpected behavior or
errors during compilation. By sticking to the standard, you minimize the risk of
encountering compatibility issues with compilers.

Various development tools, such as code editors, IDEs, and static analysis tools
are built to understand and assist with code written according to the C
standard. Conforming to the basic source character set ensures better support
from these tools, enhancing the development process.

Unicode defines various control characters that are invisible or have
non-printable representations, such as Left-to-Right Override or Right-to-Left
Isolate, zero-width joiners. These characters might not be visible when viewing
the source code in a regular text editor, making their presence non-obvious.

Depending on the context and the specific characters used, these invisible
characters can alter the meaning of the code. For example, they might introduce
syntax errors, change variable names, or create conditions that are not apparent
to the programmer but are interpreted by the compiler or interpreter in
unintended ways. Malicious actors can potentially exploit this by inserting
trojan code into a program's source code. This code might perform actions that
the programmer did not intend, such as granting unauthorized access, leaking
sensitive information, or altering the behavior of the program in unexpected
ways.

Further reading: <https://trojansource.codes/>

### Example

This example shows a shortened result for cargs.h. The file contains 35 invalid
characters in total (validation with default settings). With the verbose output
enabled, the file name and a list of lines with the invalid characters are
printed to stdout.

```console
$> cvc -f lib/cargs/cargs.h --verbose
file lib/cargs/cargs.h:
line 97: 0x40 (@)
line 103: 0x40 (@)
...
line 161: 0x40 (@)
line 166: 0x40 (@)
line 169: 0x60 (`) 0x60 (`)
line 172: 0x40 (@)
line 173: 0x40 (@)
line 178: 0x40 (@)
line 182: 0x60 (`) 0x60 (`)
line 184: 0x40 (@)
...
line 209: 0x40 (@)
line 210: 0x40 (@)
35
```

## Encoding

**cvc** assumes that all characters are encoded as single bytes. However, the
C23 standard (upcoming ISO/IEC 9899:2023) permits multibyte characters to
represent members of the character set. However, this is locale-specific and
therefore not within the scope of this program.

## Basic source character set

hex    | dec     | char | remark
------ | ------- | ---- | -----
9      | 9       | HT   | horizontal tab, see --noht option
0A     | 10      | LF   | line feed, EOL indicator, see -e/--eol option
0B     | 11      | VT   | vertical tab, see --vt option
0C     | 12      | FF   | form feed, see --ff option
0D     | 13      | CR   | carriage return, EOL indicator, see -e/--eol option
..     | ..      | ..   | n/a
20     | 32      | SP   | space
21     | 33      | !    | -
22     | 34      | "    | -
23     | 35      | #    | -
24     | 36      | $    | C23, see -a/--all option
25     | 37      | %    | -
26     | 38      | &    | -
27     | 39      | '    | -
28     | 40      | (    | -
29     | 41      | )    | -
2A     | 42      | *    | -
2B     | 43      | +    | -
2C     | 44      | ,    | -
2D     | 45      | -    | -
2E     | 46      | .    | -
2F     | 47      | /    | -
30..39 | 48..57  | 0-9  | -
3A     | 58      | :    | -
3B     | 59      | ;    | -
3C     | 60      | <    | -
3D     | 61      | =    | -
3E     | 62      | >    | -
3F     | 63      | ?    | -
40     | 64      | @    | C23, see -a/--all option
41..5A | 65..90  | A-Z  | -
5B     | 91      | [    | -
5C     | 92      | \    | -
5D     | 93      | ]    | -
5E     | 94      | ^    | -
5F     | 95      | _    | -
60     | 96      | `    | C23, see -a/--all option
61..7A | 97..122 | a-z  | -
7B     | 123     | {    | -
7C     | 124     | \|   | -
7D     | 125     | }    | -
7E     | 126     | ~    | -

## Validation

**cvc** checks the EOL first. By default, EOL is determined automatically
based on the first occurrence of an EOL indicator. However, an expected EOL
indicator can be specified with the -e/--eol option. The EOL indicator must be
used consistently throughout the file. Otherwise, the validation stops
at the first erroneous EOL indicator.

## Usage

There are three ways to pass input data to the program:

1. specify a source file
2. pipe the data to cvc
3. type input it manually

```console
$> cvc -f main.c --noht --verbose
file: main.c
0
$> cat main.c | cvc --noht --verbose
0
$> cvc -e CRLF --noht --verbose
hello
world(<CTRL> + <D>)
Unexpected end-of-line indicator in line 1!
```

### Cooperation with other tools

**cvc** is designed with UNIX philosophy in mind and therefore intentionally to
be used in cooperation with other command-line tools. The following examples
show the basic idea:

Find all *.c/*.cpp/.*h files starting from current directory recursively and
forward them to cvc for validation:

```console
find . -regex '.*/.*\.\(c\|cpp\|h\)$' -exec cvc -f {} --verbose \;
```

Validate all *.c/*.h files starting from current directory recursively and
add up the results:

```console
find . -regex '.*/.*\.\(c\|h\)$' -exec cvc -f {} \; | paste -sd+ - | bc
```

Show exit code:

```console
cvc -f main.c
echo $?
```

### Defaults

If **cvc** is used with default settings, the following applies:

- horizontal tabs are allowed
- form feed and vertical tabs are not allowed
- EOL indicator will be detected automatically
- characters $, @, ` are not allowed

## Open Source Library

This program utilizes the
[libcargs]([link-to-library](https://github.com/likle/cargs)) version 1.1.0,
which is licensed under the MIT License.

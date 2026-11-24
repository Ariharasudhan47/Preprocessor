# Preprocessor
# Mini C Preprocessor

A small C preprocessor written in C. It reads a C source file, removes comments,
expands `#include` files, handles `#define` macros (simple and function-like),
supports basic conditional compilation, and writes the processed code to an
output file.

It was built as a learning project to understand the first stage of C
compilation: what happens to your code *before* the compiler sees it.

---

## Project Description

When you compile a C program, the preprocessor runs first. It turns your source
file into a "clean" version that the compiler can read directly. This project
re-implements the most important parts of that stage:

1. **Comment removal** - strips `// line` and `/* block */` comments
2. **File inclusion** - replaces `#include "file.h"` with the contents of the file
3. **Macro expansion** - replaces macro names with their definitions
4. **Conditional compilation** - keeps or skips code using `#ifdef` / `#ifndef`

The program processes the input line by line, so it can handle large files
without loading everything into memory.

---

## Features

| Feature | Details |
|---|---|
| Comment removal | `//` and `/* */`, including multi-line comments |
| String safety | `//` or `/*` inside `"strings"` and `'chars'` are not treated as comments |
| `#include` | `#include "file"` and `#include <file>` (searched in the current folder), nested includes supported |
| Include safety | Depth limit stops infinite include loops |
| Object-like macros | `#define PI 3.14` |
| Function-like macros | `#define SQ(x) ((x)*(x))`, multiple parameters, zero parameters, nested parentheses in arguments |
| Nested macros | A macro can use other macros; self-recursion is blocked |
| Redefinition | Redefining a macro replaces the old definition |
| `#undef` | Removes a macro |
| Conditionals | `#ifdef`, `#ifndef`, `#else`, `#endif`, nested |
| Literal protection | Macros are not replaced inside string/char literals |
| Line preservation | Line breaks are kept so the output keeps the same line structure |

---

## Project Structure

```
.
├── header.h              Shared header: includes, constants, function prototypes
├── mainFile.c            Entry point: opens files, calls process_file()
├── file_processing.c     Reads the file line by line, handles #include and #ifdef
├── comment_Removal.c     Removes comments from a line
├── macroReplacement.c    Stores #define macros and expands them
└── README.md
```

### What each file does

- **mainFile.c** - checks command-line arguments, opens the input and output
  files, starts processing, and closes the files.
- **file_processing.c** - the "driver". For each line it removes comments,
  checks for directives (`#include`, `#ifdef`, `#else`, `#endif`, `#undef`),
  and passes everything else to the macro stage. Skips lines inside inactive
  `#ifdef` blocks.
- **comment_Removal.c** - `remove_comments()` walks through the line character
  by character, tracking whether it is inside a string or a block comment.
- **macroReplacement.c** - keeps a table of macros (name, parameters, body).
  `#define` lines are stored; all other lines are scanned and macro names are
  replaced.
- **header.h** - function prototypes and constants such as `SIZE` (maximum line
  length) and `MAX_INCLUDE_DEPTH`.

---

## How It Works

```
 input file
     |
     v
 read one line  ---->  remove_comments()
                              |
                              v
               is it a directive? (#include, #ifdef, #else, #endif, #undef)
                   |                              |
                  yes                             no
                   |                              |
      handle it (open the file,          is the region active?
      change conditional state)                   |
                                                  v
                                           macro_replace()
                                          /               \
                                   #define line        normal line
                                   store macro          expand macros
                                   (no output)          write to output
```

### Macro expansion steps

1. Find an identifier in the line.
2. If it is a known macro, collect its arguments (if function-like), respecting
   nested parentheses.
3. Fully expand each argument first.
4. Substitute the parameters into the macro body in a single pass.
5. Rescan the result, so macros inside macros are expanded too.

---

## Build

Requires `gcc` (or any C compiler).

```bash
gcc -Wall -Wextra -o pp mainFile.c file_processing.c macroReplacement.c comment_Removal.c
```

On Windows (MinGW) the output will be `pp.exe`.

## Usage

```bash
./pp inputfile outputfile
```

Example:

```bash
./pp test.c output.c
```

If the arguments are wrong or the input file does not exist, an error message is
printed and the program exits with code 1.

---

## Example

**input.c**
```c
#include "inc.h"
#define PI 3.14          // value of pi
#define SQ(x) ((x)*(x))
#define TWO_PI (2*PI)

/* area of a circle */
float area = PI * SQ(5);
float c = TWO_PI;
char *s = "PI // not a comment";

#ifdef DEBUG
int debug_on;
#else
int debug_off;
#endif
```

**inc.h**
```c
#define DEBUG
int from_header;
```

**output.c**
```c
int from_header;


float area = 3.14 * ((5)*(5));
float c = (2*3.14);
char *s = "PI // not a comment";

int debug_on;
```

---

## Testing

Test one feature at a time with small input files:

1. Wrong or missing arguments
2. Comments (line, block, multi-line, inside strings)
3. Simple and function-like macros
4. Nested macros
5. `#include`, including a missing file
6. `#ifdef` / `#ifndef` / `#else` / `#undef`
7. Compare with the real preprocessor: `gcc -E -P file.c`

Tip: compile with `-fsanitize=address -g` on Linux/WSL to catch memory errors.

---

## Limitations

- `#if`, `#elif`, and `defined()` are not supported (only `#ifdef` / `#ifndef`)
- Stringify (`#x`) and token pasting (`##`) are not supported
- Variadic macros (`...`, `__VA_ARGS__`) are not supported
- Predefined macros (`__FILE__`, `__LINE__`, ...) are not supported
- `#define` lines continued with `\` are not supported
- `#include` only searches the current directory, no include paths
- Maximum line length is 4096 characters (`SIZE` in `header.h`)
- Limits: 256 macros, 16 parameters per macro, 512 characters per macro argument

## Possible Improvements

- Add `#if` / `#elif` with expression evaluation
- Add `#` and `##` operators
- Add `-I` option for include directories
- Add `__FILE__` and `__LINE__`
- Replace fixed-size buffers with dynamic memory

---

## Concepts Used

File handling (`fopen`, `fgets`, `fputs`), string processing, recursion
(nested includes and nested macros), static tables, a small state machine
(comment/string tracking), and defensive buffer handling.

# 📚 The Complete C Standard Library Header Reference

> A deep-dive guide to every standard C header (C89 → C23), with definitions, function signatures, format tables, and copy-paste-ready examples.
>
> Compile note for Linux/macOS users: anything from `<math.h>` needs `-lm` (e.g. `gcc main.c -o main -lm`). On Windows (MSVC / MinGW) it links automatically.
>
> **New to C?** Read [section 1](#preliminaries) fully, then jump straight to [`<stdio.h>`](#stdioh) and [`<stdlib.h>`](#stdlibh). Ignore everything from section 15 onward until you finish loops, functions, arrays and pointers.

---

## Table of Contents

1. [Preliminaries: what a "header" actually is](#preliminaries)
2. [`<stdio.h>` — Input/Output](#stdioh)
3. [`<stdlib.h>` — General utilities & memory](#stdlibh)
4. [`<string.h>` — Strings & memory blocks](#stringh)
5. [`<ctype.h>` — Character classification](#ctypeh)
6. [`<math.h>` — Mathematics](#mathh)
7. [`<limits.h>` & `<float.h>` — Type limits](#limits-float)
8. [`<stdint.h>` & `<inttypes.h>` — Fixed-width integers](#stdint-inttypes)
9. [`<stdbool.h>` — Booleans](#stdboolh)
10. [`<stddef.h>` — Common definitions](#stddefh)
11. [`<time.h>` — Date, time & timing](#timeh)
12. [`<assert.h>` — Assertions](#asserth)
13. [`<errno.h>` — Error reporting](#errnoh)
14. [`<stdarg.h>` — Variable arguments](#stdargh)
15. [`<setjmp.h>` — Non-local jumps](#setjmph)
16. [`<signal.h>` — Signal handling](#signalh)
17. [`<locale.h>` — Localization](#localeh)
18. [Wide characters: `<wchar.h>`, `<wctype.h>`, `<uchar.h>`](#wide-chars)
19. [`<complex.h>` & `<fenv.h>` — Advanced numerics](#complex-fenv)
20. [`<iso646.h>`, `<stdalign.h>`, `<stdnoreturn.h>` — Keyword macros](#keyword-macros)
21. [`<threads.h>` & `<stdatomic.h>` — Concurrency (C11)](#concurrency)
22. [C23 additions: `<stdbit.h>`, `<stdckdint.h>`](#c23)
23. [Learning roadmap](#roadmap)
24. [Practice playground — mini projects per topic](#practice)
25. [Compile & run quickstart (Windows)](#compile-run)

---

<a id="preliminaries"></a>
## 1. Preliminaries: What Is a "Header" Actually?

A header file (`.h`) is **not** compiled code. It contains:

- **Function *declarations*** (prototypes) — tells the compiler "this function exists, here's its signature". The actual machine code lives in the **standard library** (`libc`), which the linker attaches automatically.
- **Macro definitions** — e.g. `NULL`, `INT_MAX`, `EOF`.
- **Type definitions** — e.g. `size_t`, `FILE`, `time_t`, `struct tm`.

```c
#include <stdio.h>   // angle brackets = search system include dirs
#include "mylib.h"   // quotes       = search your project dir first
```

That's why you can call `printf()` without writing it yourself — the compiler reads its prototype from `<stdio.h>`, and the linker finds its code in libc.

### 1.1 How to read this guide

- **Don't read it cover-to-cover.** Use it like a dictionary: hit a problem,
  look up the header, copy the example, experiment.
- Symbols used throughout:
  - ⚠️ = a trap that WILL bite beginners
  - ✅ = the recommended/safe way
  - ❌ = code that compiles but is wrong
- Every example is standalone — paste it into a `.c` file inside `main()`
  (add the shown `#include`s) and run it.

### 1.2 The Big Five beginner bugs (memorize these!)

**1. Missing `&` in `scanf`**
```c
int age;
scanf("%d", age);    // ❌ passes the VALUE — crashes or corrupts memory
scanf("%d", &age);   // ✅ passes the ADDRESS where to store it
```

**2. `=` (assign) vs `==` (compare)**
```c
if (x = 5) { ... }   // ❌ assigns 5 to x, always true! (compiler warns)
if (x == 5) { ... }  // ✅ comparison
```

**3. Array out-of-bounds**
```c
int arr[5];                 // valid indexes: 0,1,2,3,4 — NOT 5!
for (int i = 0; i <= 5; i++) arr[i] = i;   // ❌ writes arr[5] = UB
for (int i = 0; i < 5; i++)  arr[i] = i;   // ✅
```
C does **not** check bounds — going past the end silently corrupts memory.

**4. Using an uninitialized variable**
```c
int total;
for (int i = 0; i < n; i++) total += arr[i];  // ❌ total starts as garbage
int total = 0;                                // ✅ always initialize
```

**5. Comparing strings or floats with `==`**
```c
if (name == "Dev") { ... }              // ❌ compares POINTERS, never equal
if (strcmp(name, "Dev") == 0) { ... }   // ✅ compares contents

if (a == b) { ... }                     // ❌ floats: rounding makes this flaky
if (fabs(a - b) < 1e-9) { ... }         // ✅ "close enough" comparison
```

### 1.3 Error message decoder

When your build fails, **fix only the FIRST error**, recompile, repeat — later
errors are usually just fallout from the first one.

| Compiler says | What it actually means | Fix |
|---|---|---|
| `undefined reference to 'sqrt'` | Linker can't find math code | Linux/macOS: add `-lm`. Windows: should auto-link |
| `implicit declaration of function 'printf'` | You forgot the `#include` | Add the right header |
| `'total' undeclared (first use in this function)` | Typo, or variable used outside its `{ }` block | Declare it, check spelling & scope |
| `expected ';' before '}' token` | Missing semicolon — often on the line ABOVE | Look one line earlier than reported |
| `control reaches end of non-void function` | Function promises to return a value but might not | Add a `return` at the end |
| `format '%d' expects argument of type 'int', but argument has type 'double'` | Wrong specifier | `%f` for double, `%d` for int |
| `subscripted value is neither array nor pointer` | You used `[ ]` on a non-array | Check the variable type |
| *(no compile error)* **Segmentation fault** | Bad pointer or out-of-bounds array access at runtime | Check `&` in scanf, array limits, NULL pointers |
| *(no compile error)* Program prints garbage | Uninitialized variable, or `%s` given a non-string | Initialize; verify types |

💡 Turn warnings UP — they catch most of these before runtime:
```
gcc -Wall -Wextra -o prog prog.c
```

### 1.4 Danger zone: unsafe functions → safe replacements

These classic functions still appear in old books (like *Let Us C*), but avoid
them in your own code:

| ❌ Avoid | Why dangerous | ✅ Use instead |
|---|---|---|
| `gets(s)` | Cannot limit input — famous hack vector, removed in C11 | `fgets(s, sizeof s, stdin)` |
| `scanf("%s", buf)` | No length limit → buffer overflow | `scanf("%19s", buf)` or `fgets` |
| `strcpy(dst, src)` | Overflow if src too big | `snprintf(dst, sizeof dst, "%s", src)` |
| `sprintf(buf, ...)` | Same overflow risk | `snprintf(buf, sizeof buf, ...)` |
| `strcat(dst, src)` | Overflow if dst fills up | `snprintf(dst, sizeof dst, "%s%s", dst, src)` |
| `atoi(s)` | Silent failure (returns 0), no overflow check | `strtol(s, &end, 10)` + check `errno` |
| `rand()` for passwords/keys | Predictable sequence | (out of scope — needs OS crypto APIs) |

---

<a id="stdioh"></a>
## 2. `<stdio.h>` — Standard Input/Output

The most-used header in all of C. Provides **st**andar**d** **i**nput/**o**utput.

### Key types & constants

| Name | Meaning |
|---|---|
| `FILE` | Opaque struct representing an open file stream |
| `stdin`, `stdout`, `stderr` | Pre-opened streams: keyboard, screen, error channel |
| `EOF` | End-of-file marker, usually `-1` |
| `NULL` | Null pointer constant |
| `BUFSIZ` | Default buffer size (commonly 8192) |
| `FILENAME_MAX`, `L_tmpnam` | Max path length, temp-name length |

### 2.1 Formatted output — `printf` family

```c
int printf (const char *format, ...);              // to stdout
int fprintf(FILE *stream, const char *format, ...);// to any stream/file
int sprintf(char *buf,    const char *format, ...);// into a char array ⚠️ no bounds check!
int snprintf(char *buf, size_t n, const char *format, ...); // SAFE version (C99)
```

Return value: number of characters printed (negative on error).

#### Format specifier anatomy

```
%[flags][width][.precision][length]conversion
   │      │        │         │        └─ d i u f e g s c p x o %
   │      │        │         └─ hh h l ll z j t L
   │      │        └─ minimum digits (ints) / decimals (floats) / max chars (strings)
   │      └─ minimum field width (padded with spaces)
   └─ - left-align, + force sign, 0 zero-pad, space, # alternate form
```

#### Conversion specifiers

| Specifier | Converts | Example | Output |
|---|---|---|---|
| `%d` / `%i` | signed int | `printf("%d", -42)` | `-42` |
| `%u` | unsigned int | `printf("%u", 42u)` | `42` |
| `%o` | octal | `printf("%o", 64)` | `100` |
| `%x` / `%X` | hex (lower/upper) | `printf("%x", 255)` | `ff` |
| `%f` | double (fixed) | `printf("%f", 3.14)` | `3.140000` |
| `%e` / `%E` | scientific | `printf("%e", 31400.0)` | `3.140000e+04` |
| `%g` / `%G` | shorter of %f/%e | `printf("%g", 3.14)` | `3.14` |
| `%a` / `%A` | hex float (C99) | `printf("%a", 1.0)` | `0x1p+0` |
| `%c` | single char | `printf("%c", 'A')` | `A` |
| `%s` | string (NUL-terminated) | `printf("%s", "hi")` | `hi` |
| `%p` | pointer address | `printf("%p", ptr)` | `0x7ffd...` |
| `%%` | literal percent | `printf("100%%")` | `100%` |

#### Length modifiers

| Modifier | For type | Example |
|---|---|---|
| `hh` | `char` / `signed char` | `%hhd` |
| `h` | `short` | `%hd` |
| `l` | `long`, `wchar_t` | `%ld` |
| `ll` | `long long` | `%lld` |
| `z` | `size_t` | `%zu` ← use this for sizeof results! |
| `j` | `intmax_t` | `%jd` |
| `t` | `ptrdiff_t` | `%td` |
| `L` | `long double` | `%Lf` |

#### Width, precision, flags in action

```c
printf("[%5d]", 42);      // [   42]   width 5, right-aligned
printf("[%-5d]", 42);     // [42   ]   left-aligned
printf("[%05d]", 42);     // [00042]   zero-padded
printf("[%+.2f]", 3.14159);// [+3.14]  forced sign, 2 decimals
printf("[%10.3s]", "hello");// [     hel] width 10, precision truncates string
printf("[%*d]", 6, 42);   // [    42]   width taken from argument
```

### 2.2 Formatted input — `scanf` family

```c
int scanf (const char *format, ...);
int fscanf(FILE *stream, const char *format, ...);
int sscanf(const char *buf, const char *format, ...); // parse FROM a string
```

⚠️ **Critical rules:**
1. Arguments must be **pointers**: `scanf("%d", &num);` — forgetting `&` is bug #1 for beginners.
2. `%s` has **no bounds checking** — buffer overflow risk. Prefer width: `scanf("%19s", buf);` for a 20-byte buffer.
3. Returns the **number of items successfully read** — always check it!

```c
int age;
if (scanf("%d", &age) != 1) {
    fprintf(stderr, "Invalid input\n");
}
```

Useful input tricks:

```c
scanf(" %c", &ch);        // leading SPACE skips whitespace/newlines before %c
scanf("%d,%d", &a, &b);   // literal chars in format must match input exactly
scanf("%*[^\n]");         // * = assignment suppression: read & discard
scanf("%19[^\n]", buf);   // scanset: read up to newline (reads spaces too!)
```

### 2.3 Character & line I/O

```c
int getchar(void);            // read ONE char (returns int! so EOF fits)
int putchar(int c);

char *fgets(char *buf, int n, FILE *stream);  // SAFE line read (keeps '\n')
int fputs(const char *s, FILE *stream);       // write string (no auto newline)
int puts(const char *s);                      // write string + newline
```

⚠️ Never use `gets()` — removed from the language in C11 because it cannot be made safe.

```c
char line[100];
if (fgets(line, sizeof line, stdin) != NULL) {
    line[strcspn(line, "\n")] = '\0';   // strip trailing newline (classic idiom)
}
```

### 2.4 File I/O

```c
FILE *fopen (const char *path, const char *mode);
int   fclose(FILE *stream);
```

| Mode | Meaning | If file exists | If file missing |
|---|---|---|---|
| `"r"` | read | ✓ opens | ✗ fails (NULL) |
| `"w"` | write | ✗ **truncated to 0!** | creates |
| `"a"` | append | ✓ writes at end | creates |
| `"r+"` | read + write | ✓ opens | ✗ fails |
| `"w+"` | read + write | ✗ truncated | creates |
| `"a+"` | read + append | ✓ appends | creates |
| *(add `b`)* | binary mode, e.g. `"rb"`, `"wb"` | — | — |

Random access & other file ops:

```c
long ftell(FILE *stream);                       // current byte offset
int  fseek(FILE *stream, long off, int origin); // SEEK_SET(0) SEEK_CUR(1) SEEK_END(2)
void rewind(FILE *stream);                      // back to start
size_t fread (void *buf, size_t sz, size_t n, FILE *stream);  // binary read
size_t fwrite(const void *buf, size_t sz, size_t n, FILE *stream); // binary write
int  feof(FILE *stream);                        // hit end-of-file?
int  ferror(FILE *stream);                      // error occurred?
void perror(const char *msg);                   // print msg + errno description
int  remove(const char *path);                  // delete file
int  rename(const char *old, const char *new);  // rename/move file
int  fflush(FILE *stream);                      // flush output buffer
```

#### Full worked example

```c
#include <stdio.h>

int main(void) {
    FILE *fp = fopen("data.txt", "w");
    if (fp == NULL) {                    // ALWAYS check fopen!
        perror("fopen");
        return 1;
    }
    fprintf(fp, "Name: %s, Score: %d\n", "Devashish", 95);
    fclose(fp);

    fp = fopen("data.txt", "r");
    if (!fp) { perror("reopen"); return 1; }

    char line[128];
    while (fgets(line, sizeof line, fp)) // read until EOF
        fputs(line, stdout);

    fclose(fp);
    return 0;
}
```

---

<a id="stdlibh"></a>
## 3. `<stdlib.h>` — General Utilities ("standard library")

The grab-bag header: dynamic memory, conversions, randomness, sorting, program control.

### 3.1 Dynamic memory management

```c
void *malloc (size_t size);                  // allocate; contents UNINITIALIZED (garbage)
void *calloc (size_t count, size_t size);    // allocate AND zero-initialize
void *realloc(void *ptr, size_t new_size);   // resize (may move the block!)
void  free   (void *ptr);                    // release memory
```

Rules every beginner must internalize:

1. Every `malloc`/`calloc`/`realloc` needs exactly one matching `free`.
2. Check for `NULL` return (allocation can fail).
3. After `free`, the pointer is dangling — don't use it again.
4. Freeing the same pointer twice = undefined behavior.
5. Memory is NOT automatically freed when the pointer goes out of scope (that's the leak).

```c
#include <stdlib.h>
#include <stdio.h>

int main(void) {
    int n = 5;

    /* malloc: array of n ints */
    int *arr = malloc(n * sizeof *arr);          // idiomatic: sizeof *arr
    if (!arr) { perror("malloc"); return 1; }
    arr[0] = 42;                                  // garbage elsewhere!

    /* calloc: zeroed array */
    int *zeroed = calloc(n, sizeof *zeroed);      // all elements == 0

    /* grow the array */
    int *bigger = realloc(arr, 10 * sizeof *arr);
    if (bigger) arr = bigger;                     // keep old ptr if realloc failed

    free(arr);
    free(zeroed);
    return 0;
}                                                 // forgot free(bigger)? leak!
```

### 3.2 String ↔ number conversion

| Function | Signature | Parses | Notes |
|---|---|---|---|
| `atoi` | `int atoi(const char *s)` | int | ⚠️ No error detection; returns 0 on failure |
| `atol`, `atof` | similar | long, double | Same caveat |
| `strtol` | `long strtol(const char *s, char **end, int base)` | long | ✅ Robust: reports where parsing stopped |
| `strtoll` | C99 | long long | base 2–36, or 0 = auto-detect (0x→hex, 0→octal) |
| `strtod` | `double strtod(const char *s, char **end)` | double | Handles `1.5e3`, `inf`, hex floats |

```c
const char *s = "1234abc";
char *end;
long val = strtol(s, &end, 10);   // val = 1234, end points at 'a'

if (end == s)          printf("No digits found\n");
else if (*end != '\0') printf("Parsed %ld, stopped at '%c'\n", val, *end);
else                   printf("Fully parsed %ld\n", val);
```

### 3.3 Random numbers

```c
int  rand(void);                 // pseudo-random int in [0, RAND_MAX]
void srand(unsigned seed);       // seed the generator
```

```c
#include <stdlib.h>
#include <time.h>

srand((unsigned) time(NULL));           // seed ONCE at program start
int dice = rand() % 6 + 1;              // 1..6 (modulo bias exists but fine for games)
int r    = rand() % 100;                // 0..99
```

⚠️ `rand()%N` is slightly biased and low-quality; fine for learning, not crypto/statistics.

### 3.4 Sorting & searching — `qsort` / `bsearch`

```c
void qsort(void *base, size_t nmemb, size_t size,
           int (*compar)(const void *, const void *));
void *bsearch(const void *key, void *base, size_t nmemb, size_t size,
              int (*compar)(const void *, const void *));  // requires SORTED data
```

```c
#include <stdlib.h>
#include <stdio.h>

int cmp_int(const void *a, const void *b) {          // comparator MUST take const void*
    int x = *(const int *)a;
    int y = *(const int *)b;
    return (x > y) - (x < y);                        // safe: avoids overflow of x-y
}

int main(void) {
    int arr[] = {5, 1, 9, 3, 7};
    qsort(arr, 5, sizeof arr[0], cmp_int);           // {1,3,5,7,9}

    int key = 7;
    int *found = bsearch(&key, arr, 5, sizeof arr[0], cmp_int);
    if (found) printf("Found %d\n", *found);
}
```

Comparator convention: return **negative** if a<b, **zero** if equal, **positive** if a>b.

### 3.5 Program control & environment

```c
void exit(int status);        // normal termination (flushes streams, runs atexit fns)
void _Exit(int status);       // immediate termination, no cleanup
void abort(void);             // abnormal termination (SIGABRT)
int  atexit(void (*func)(void));   // register cleanup fn to run at exit
int  system(const char *cmd);      // run OS command, e.g. system("cls")
char *getenv(const char *name);    // environment variable, e.g. getenv("PATH")
int abs(int x); labs(long); llabs(long long);   // absolute values
div_t div(int num, int den);       // quotient AND remainder in one call
EXIT_SUCCESS / EXIT_FAILURE        // portable exit codes
```

---

<a id="stringh"></a>
## 4. `<string.h>` — String Handling & Memory Blocks

Remember: a C "string" is just a `char` array ending with the `'\0'` terminator. These functions trust that convention completely.

### 4.1 Length & copy

| Function | Signature | Does |
|---|---|---|
| `strlen` | `size_t strlen(const char *s)` | Length **excluding** `'\0'`. O(n)! Don't call in loop conditions on huge strings. |
| `strcpy` | `char *strcpy(char *dst, const char *src)` | Copy. ⚠️ No bounds check — dst must be big enough |
| `strncpy` | `char *strncpy(char *dst, const char *src, size_t n)` | Copy ≤ n bytes. ⚠️ May NOT null-terminate if src is longer than n! |
| `strdup` | `char *strdup(const char *s)` (POSIX/C23) | malloc + copy — makes an owned heap copy |
| `memcpy` | `void *memcpy(void *dst, const void *src, size_t n)` | Raw byte copy (regions must not overlap) |
| `memmove` | `void *memmove(void *dst, const void *src, size_t n)` | Byte copy that handles overlap correctly |

```c
char src[] = "hello";
char dst[6];
strcpy(dst, src);                          // dst now "hello"

/* Safe copy pattern with strncpy: */
char safe[6];
strncpy(safe, "hello world", sizeof safe - 1);
safe[sizeof safe - 1] = '\0';              // manual termination — always do this
```

### 4.2 Concatenation & comparison

```c
char *strcat(char *dst, const char *src);               // append src to dst
char *strncat(char *dst, const char *src, size_t n);    // append ≤ n chars (DOES terminate)

int strcmp (const char *a, const char *b);   // <0, 0, >0 — case SENSITIVE
int strncmp(const char *a, const char *b, size_t n);    // first n chars only
int memcmp (const void *a, const void *b, size_t n);    // raw bytes
```

⚠️ `if (strcmp(a,b) == 0)` means strings are EQUAL. Beginners often write `== 1`.
Never compare strings with `==` — that compares pointers, not contents!

### 4.3 Searching

| Function | Finds | Returns |
|---|---|---|
| `strchr(s, c)` | first occurrence of char `c` | pointer, or NULL |
| `strrchr(s, c)` | last occurrence of char | pointer, or NULL |
| `strstr(hay, needle)` | first substring match | pointer, or NULL |
| `strspn(s, accept)` | length of initial segment made ONLY of accept chars | size_t |
| `strcspn(s, reject)` | length of initial segment containing NO reject chars | size_t |
| `strpbrk(s, accept)` | first char that is ANY of accept | pointer, or NULL |
| `strtok(s, delim)` | next token split by delimiters | token, or NULL |

```c
/* Idiom: strip trailing newline (from fgets) */
line[strcspn(line, "\n")] = '\0';
// strcspn finds position of '\n'; we overwrite it with terminator

/* strtok: split a CSV-ish string */
char str[] = "apple,banana;cherry";
char *tok = strtok(str, ",;");
while (tok) {
    printf("%s\n", tok);      // apple \n banana \n cherry
    tok = strtok(NULL, ",;"); // pass NULL to continue
}
```

⚠️ `strtok` modifies the original string (inserts `'\0'`) and keeps hidden state — not thread-safe. Use `strtok_r` (POSIX) in threaded code.

### 4.4 Memory block utilities

```c
void *memset(void *dst, int ch, size_t n);   // fill n bytes with ch
```

```c
memset(buf, 0, sizeof buf);                  // classic "clear the buffer"
memset(buf, 'A', 10);                        // buf = "AAAAAAAAAA..."
```

---

<a id="ctypeh"></a>
## 5. `<ctype.h>` — Character Classification & Conversion

Each takes an `int` (the char) and returns nonzero (true) or 0 (false).

### Classification

| Function | True when character is… |
|---|---|
| `isalpha(c)` | a letter (A–Z, a–z) |
| `isdigit(c)` | a digit 0–9 |
| `isalnum(c)` | letter OR digit |
| `isspace(c)` | whitespace: space, `\t \n \v \f \r` |
| `isupper(c)` / `islower(c)` | uppercase / lowercase letter |
| `ispunct(c)` | printable, but not alphanumeric/space |
| `isprint(c)` | printable (incl. space) |
| `isgraph(c)` | printable excluding space |
| `iscntrl(c)` | control character (e.g. tab, ESC) |
| `isxdigit(c)` | hex digit: 0–9, a–f, A–F |

### Conversion

```c
int toupper(int c);   // 'a' -> 'A' (unchanged if not lowercase)
int tolower(int c);   // 'A' -> 'a'
```

### Worked example — counting vowels & digits

```c
#include <stdio.h>
#include <ctype.h>
#include <string.h>

int main(void) {
    char text[] = "Hello World 123!";
    int letters = 0, digits = 0, spaces = 0, others = 0;

    for (size_t i = 0; i < strlen(text); i++) {
        if (isalpha(text[i]))      letters++;
        else if (isdigit(text[i])) digits++;
        else if (isspace(text[i])) spaces++;
        else                       others++;
    }
    printf("letters=%d digits=%d spaces=%d others=%d\n",
           letters, digits, spaces, others);

    char shout[50];
    for (size_t i = 0; i <= strlen(text); i++)
        shout[i] = (char)toupper(text[i]);
    printf("%s\n", shout);                    // HELLO WORLD 123!
}
```

⚠️ Pass the char as `unsigned char` cast when looping over arbitrary data:
`isalpha((unsigned char)text[i])` — passing negative values is UB.

---

<a id="mathh"></a>
## 6. `<math.h>` — Mathematics

All trig functions work in **radians**, not degrees!

### Power, roots, absolute value

| Function | Meaning |
|---|---|
| `sqrt(x)` | square root (√x), domain: x ≥ 0 |
| `cbrt(x)` | cube root ∛x (works for negatives, C99) |
| `pow(x, y)` | xʸ (returns double) |
| `hypot(x, y)` | √(x²+y²) without overflow — great for distance |
| `fabs(x)` | absolute value of double (NOT `abs` — that's integer!) |
| `fmod(x, y)` | floating-point remainder of x/y |
| `remainder(x, y)` | IEEE remainder (C99) |

### Rounding

| Function | Behavior | `2.5` → | `-2.5` → |
|---|---|---|---|
| `floor(x)` | round down | 2.0 | −3.0 |
| `ceil(x)` | round up | 3.0 | −2.0 |
| `round(x)` | round half away from zero | 3.0 | −3.0 |
| `trunc(x)` | chop decimal part | 2.0 | −2.0 |
| `nearbyint(x)` / `rint(x)` | round per current mode | 2.0 | −2.0 |

### Exponentials & logs

```c
exp(x)      // e^x
log(x)      // natural log ln(x), domain x > 0
log10(x)    // log₁₀(x)
log2(x)     // log₂(x)  (C99)
expm1(x)    // e^x - 1, accurate for tiny x
log1p(x)    // ln(1+x), accurate for tiny x
```

### Trigonometry

```c
sin(x) cos(x) tan(x)          // x in radians
asin(x) acos(x) atan(x)       // inverse; asin/acos domain [-1, 1]
atan2(y, x)                   // angle of point (x,y): full-circle aware, (-π, π]
sinh cosh tanh                // hyperbolic
M_PI                          // π — technically non-standard but nearly universal
```

Degrees ↔ radians conversion:

```c
#define PI 3.14159265358979323846
double deg2rad(double d) { return d * PI / 180.0; }
```

### Floating-point helpers (C99)

```c
frexp(x, &exp)   // split into mantissa × 2^exp
ldexp(x, exp)    // x × 2^exp
modf(x, &iptr)   // split into fractional + integer parts
fmin(a,b) fmax(a,b) fdim(a,b)   // min, max, positive difference
isnan(x) isinf(x) isfinite(x)   // classify special values
NAN, INFINITY                   // constants
```

### Example — quadratic equation solver

```c
#include <stdio.h>
#include <math.h>

int main(void) {
    double a = 1, b = -3, c = 2;                 // x² - 3x + 2 = 0
    double disc = b*b - 4*a*c;

    if (disc < 0)
        printf("Complex roots\n");
    else {
        double sq = sqrt(disc);
        printf("x1 = %.2f, x2 = %.2f\n", (-b + sq)/(2*a), (-b - sq)/(2*a));
    }                                            // x1 = 2.00, x2 = 1.00
}
```

---

<a id="limits-float"></a>
## 7. `<limits.h>` & `<float.h>` — Type Limits

### `<limits.h>` — integer ranges (typical 64-bit Windows/Linux values)

| Macro | Meaning | Typical value |
|---|---|---|
| `CHAR_BIT` | bits per byte | 8 |
| `CHAR_MIN` / `CHAR_MAX` | char range | −128 / 127 |
| `SCHAR_MIN` / `SCHAR_MAX` | signed char | −128 / 127 |
| `UCHAR_MAX` | unsigned char | 255 |
| `SHRT_MIN` / `SHRT_MAX` | short | −32,768 / 32,767 |
| `INT_MIN` / `INT_MAX` | int | −2³¹ / 2³¹−1 (≈ ±2.1 billion) |
| `UINT_MAX` | unsigned int | 2³²−1 (≈ 4.29 billion) |
| `LONG_MIN` / `LONG_MAX` | long | 64-bit Linux: ±2⁶³; **Windows: same as int!** |
| `LLONG_MIN` / `LLONG_MAX` | long long | ±2⁶³ everywhere |
| `ULLONG_MAX` | unsigned long long | 2⁶⁴−1 |

Classic use — finding the maximum safely:

```c
int max = INT_MIN;                 // smallest possible int
for (int i = 0; i < n; i++)
    if (arr[i] > max) max = arr[i];
```

Overflow warning: `INT_MAX + 1` is **undefined behavior** for signed ints.

### `<float.h>` — floating-point characteristics

| Macro | Meaning |
|---|---|
| `FLT_MAX` / `DBL_MAX` | largest finite float / double (≈3.4e38 / ≈1.8e308) |
| `FLT_MIN` / `DBL_MIN` | smallest **positive normalized** value (≈1.2e-38 / 2.2e-308) |
| `FLT_EPSILON` / `DBL_EPSILON` | smallest x where 1.0+x ≠ 1.0 (≈1.19e-7 / 2.22e-16) |
| `FLT_DIG` / `DBL_DIG` | guaranteed correct decimal digits (6 / 15) |
| `FLT_MANT_DIG` | bits of mantissa precision |

Epsilon comparison — never compare floats with `==`:

```c
#include <math.h>
#include <float.h>
if (fabs(a - b) <= DBL_EPSILON * fabs(a))   // "close enough"
```

---

<a id="stdint-inttypes"></a>
## 8. `<stdint.h>` & `<inttypes.h>` — Fixed-Width Integers (C99)

`int` size varies by platform (usually 32-bit, but 16-bit on old systems). When you need EXACT widths (file formats, protocols, embedded):

### Exact-width types

| Signed | Unsigned | Bits | Range |
|---|---|---|---|
| `int8_t` | `uint8_t` | 8 | −128..127 / 0..255 |
| `int16_t` | `uint16_t` | 16 | ±32,767 / 0..65,535 |
| `int32_t` | `uint32_t` | 32 | ±2.1e9 / 0..4.29e9 |
| `int64_t` | `uint64_t` | 64 | ±9.2e18 / 0..1.8e19 |

### Other useful types

| Type | Meaning |
|---|---|
| `intptr_t`, `uintptr_t` | Integer big enough to hold a pointer |
| `intmax_t`, `uintmax_t` | Widest supported integer |
| `int_least8_t` … | Smallest type of ≥ N bits |
| `int_fast8_t` … | Fastest type of ≥ N bits |

### `<inttypes.h>` — printing them portably

You can't use `%d` for `int32_t` reliably (it might be `long` somewhere).
So:

```c
#include <inttypes.h>
#include <stdio.h>

int64_t big = 9000000000;
printf("%" PRId64 "\n", big);    // PRId64 expands to correct specifier
printf("%" PRIu32 "\n", (uint32_t)big);
scanf("%" SCNd32, &x);           // SCNd* for scanf
```

Common macros: `PRId8/16/32/64`, `PRIu8/16/32/64`, `PRIx32` (hex), `SCNd32` etc.

---

<a id="stdboolh"></a>
## 9. `<stdbool.h>` — Booleans (C99)

Before C99, C had no boolean type — people used `int` with 0/1, or homemade
macros like `#define TRUE 1`.

```c
#include <stdbool.h>

bool flag = true;      // true == 1, false == 0
if (flag) { ... }
```

It literally defines just three macros:

```c
#define bool  _Bool      // _Bool is the real built-in type
#define true  1
#define false 0
```

Notes:
- Any nonzero value converts to `true`; this is why `if (5)` works.
- In **C23**, `bool`, `true`, `false` became real keywords and the header is empty/deprecated.
- ⚠️ Your earlier code used `True` (capital T) — that's Python style, not C. It's `true`.

---

<a id="stddefh"></a>
## 10. `<stddef.h>` — Common Definitions

Small but essential — included implicitly by many others.

| Name | Meaning |
|---|---|
| `size_t` | Unsigned type for sizes/count — result type of `sizeof`. Print with `%zu` |
| `ptrdiff_t` | Signed type for pointer differences. Print with `%td` |
| `NULL` | Null pointer constant (usually `((void*)0)` or `0`) |
| `offsetof(type, member)` | Byte offset of a struct member |

```c
#include <stddef.h>
#include <stdio.h>

struct Packet { char id; int payload; char tag; };

int main(void) {
    printf("sizeof(size_t) = %zu\n", sizeof(size_t));
    printf("offset of payload = %zu\n", offsetof(struct Packet, payload));
    // Often 4 due to PADDING — the compiler aligns members for fast access!
}
```

---

<a id="timeh"></a>
## 11. `<time.h>` — Date, Time & Timing

### Core types

| Type | Meaning |
|---|---|
| `time_t` | Calendar time (usually seconds since Jan 1 1970 UTC — the "Unix epoch") |
| `clock_t` | Processor time ticks |
| `struct tm` | Broken-down calendar time (see below) |
| `CLOCKS_PER_SEC` | Ticks per second for `clock()` (usually 1,000,000) |

### `struct tm` — every field explained

```c
struct tm {
    int tm_sec;    // seconds after minute : 0–60 (60 = leap second!)
    int tm_min;    // minutes after hour   : 0–59
    int tm_hour;   // hours since midnight : 0–23
    int tm_mday;   // day of month         : 1–31   ← NOT 0-based!
    int tm_mon;    // months since January : 0–11   ← 0-based! Add 1 to display
    int tm_year;   // years since 1900     ← add 1900 to display!
    int tm_wday;   // days since Sunday    : 0–6
    int tm_yday;   // days since Jan 1     : 0–365
    int tm_isdst;  // daylight saving flag : >0 yes, 0 no, <0 unknown
};
```

⚠️ Two famous gotchas: `tm_mon` starts at 0, `tm_year` counts from 1900.

### The conversion pipeline

```
   time() ──────────────► time_t (raw seconds)
      │
      ├── localtime(&t) ──► struct tm*  (YOUR timezone)
      ├── gmtime(&t) ─────► struct tm*  (UTC)
      │
      ├── ctime(&t) ───────► "Tue Aug 25 14:30:00 2026\n"  (fixed format)
      │
      ├── strftime(buf, sz, "format", tmp) ──► CUSTOM formatted string
      │
 mktime(&tm) ◄── build your own struct tm ──► time_t (reverse direction!)
```

### Function reference

```c
time_t time(time_t *t);                       // current calendar time
clock_t clock(void);                          // CPU time since program start
double difftime(time_t end, time_t start);    // difference in seconds

struct tm *localtime(const time_t *t);        // → local-time struct tm
struct tm *gmtime(const time_t *t);           // → UTC struct tm
time_t mktime(struct tm *tmp);                // struct tm → time_t (normalizes fields!)

char *ctime(const time_t *t);                 // quick human-readable string
char *asctime(const struct tm *tmp);          // same, from struct tm
size_t strftime(char *buf, size_t max, const char *fmt, const struct tm *tmp);
```

### `strftime` format specifiers — the full table

| Specifier | Meaning | Example |
|---|---|---|
| `%Y` | 4-digit year | `2026` |
| `%y` | 2-digit year | `26` |
| `%C` | century | `20` |
| `%m` | month 01–12 | `08` |
| `%b` / `%h` | abbreviated month | `Aug` |
| `%B` | full month name | `August` |
| `%d` | day of month 01–31 | `25` |
| `%e` | day, space-padded | ` 25` |
| `%a` | abbreviated weekday | `Tue` |
| `%A` | full weekday | `Tuesday` |
| `%j` | day of year 001–366 | `237` |
| `%H` | hour 00–23 (24h) | `14` |
| `%I` | hour 01–12 (12h) | `02` |
| `%p` | AM/PM | `PM` |
| `%M` | minutes 00–59 | `30` |
| `%S` | seconds 00–60 | `07` |
| `%R` | `%H:%M` | `14:30` |
| `%T` | `%H:%M:%S` | `14:30:07` |
| `%r` | 12-hour full time | `02:30:07 PM` |
| `%D` | `%m/%d/%y` | `08/25/26` |
| `%F` | `%Y-%m-%d` (ISO date) | `2026-08-25` |
| `%c` | locale date+time | `Tue Aug 25 14:30:07 2026` |
| `%x` | locale date | `08/25/26` |
| `%X` | locale time | `14:30:07` |
| `%Z` | timezone name | `IST` |
| `%z` | UTC offset | `+0530` |
| `%U` | week of year (Sun-first) | `34` |
| `%W` | week of year (Mon-first) | `34` |
| `%%` | literal % | `%` |

### Worked examples

```c
#include <stdio.h>
#include <time.h>

int main(void) {
    /* --- 1. Current date/time, custom format --- */
    time_t now = time(NULL);
    struct tm *lt = localtime(&now);

    char buf[64];
    strftime(buf, sizeof buf, "%A, %d %B %Y — %I:%M:%S %p", lt);
    printf("Now: %s\n", buf);
    // Now: Tuesday, 25 August 2026 — 02:30:07 PM

    /* --- 2. Quick-and-dirty --- */
    printf("ctime says: %s", ctime(&now));

    /* --- 3. Building a specific date with mktime --- */
    struct tm birthday = {0};
    birthday.tm_year = 105;      // 1900 + 105 = 2005
    birthday.tm_mon  = 6;        // month index 6 = JULY (0-based!)
    birthday.tm_mday = 15;       // 15th
    birthday.tm_hour = 12;
    time_t btime = mktime(&birthday);   // fields get normalized/filled in
    printf("Birthday weekday: %s\n", ctime(&btime));

    /* --- 4. Measuring elapsed wall-clock seconds --- */
    time_t start = time(NULL);
    /* ... slow work ... */
    printf("Took %.0f seconds\n", difftime(time(NULL), start));

    /* --- 5. Measuring CPU time precisely --- */
    clock_t c0 = clock();
    double s = 0;
    for (volatile long i = 0; i < 50000000L; i++) s += i * 0.5;
    clock_t c1 = clock();
    printf("CPU time: %.3f sec\n", (double)(c1 - c0) / CLOCKS_PER_SEC);
}
```

`clock()` measures **processor** time (may ignore sleep/wait); `time()` measures
wall-clock time. For sub-millisecond wall timing, use platform APIs
(`QueryPerformanceCounter` on Windows, `clock_gettime` POSIX).

---

<a id="asserth"></a>
## 12. `<assert.h>` — Assertions

```c
void assert(int expression);
```

If the expression is false (0), the program prints the failing expression, file, and line to stderr, then aborts. It's a debugging tripwire, not error handling.

```c
#include <assert.h>

void divide(int *out, int a, int b) {
    assert(out != NULL);       // programmer contract: crash loudly during dev
    assert(b != 0);
    *out = a / b;
}
```

On failure it prints something like:

```
Assertion failed: b != 0, file math.c, line 12
```

Disable ALL assertions in release builds by defining NDEBUG **before** including:

```c
#define NDEBUG
#include <assert.h>     // assert() now compiles to nothing
```

C11 also added compile-time checks:

```c
static_assert(sizeof(int) >= 4, "need 32-bit int");
```

Rule of thumb: assert for *bugs that should be impossible*; use real `if` +
error returns for *runtime failures the user can trigger* (bad files, bad input).

---

<a id="errnoh"></a>
## 13. `<errno.h>` — Error Reporting

Library functions report failures through the global `errno` (error number).

```c
errno          // int macro — last error code (thread-local in modern libs)
EDOM           // domain error, e.g. sqrt(-1)
ERANGE         // range error/overflow, e.g. strtol result too big
EILSEQ         // illegal multibyte sequence
```

Pattern:

```c
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

errno = 0;                       // clear it first — errno is NEVER reset to 0 for you!
long v = strtol("999999999999999999999", NULL, 10);
if (errno == ERANGE)
    fprintf(stderr, "Overflow: %s\n", strerror(errno));
    // strerror(errno) turns code into message like "Numerical result out of range"
```

Or the shortcut:

```c
FILE *fp = fopen("missing.txt", "r");
if (!fp) perror("opening file");
// prints: opening file: No such file or directory
```

⚠️ Only inspect `errno` **after** a call actually failed — success doesn't clear it.

---

<a id="stdargh"></a>
## 14. `<stdarg.h>` — Variable Arguments

How does `printf` accept any number of arguments? With these tools you can
write your own variadic functions.

```c
va_list ap;                    // the argument "cursor"
va_start(ap, last_fixed_arg);  // begin after the last named parameter
va_arg(ap, type);              // fetch next arg AS type (no type checking!)
va_end(ap);                    // clean up (required)
va_copy(dst, src);             // duplicate a cursor mid-scan (C99)
```

### Example — sum of any number of ints

```c
#include <stdarg.h>
#include <stdio.h>

/* Convention needed: caller must say how many args follow */
int sum(int count, ...) {
    va_list ap;
    va_start(ap, count);           // 'count' is the last named parameter
    int total = 0;
    for (int i = 0; i < count; i++)
        total += va_arg(ap, int);  // YOU must ask for the right type
    va_end(ap);
    return total;
}

int main(void) {
    printf("%d\n", sum(3, 10, 20, 30));   // 60
    printf("%d\n", sum(5, 1, 2, 3, 4, 5));// 15
}
```

Rules & dangers:
- At least one **named** parameter required (that's why `printf(fmt, ...)`).
- `va_arg` performs **default argument promotions**: `char`/`short` arrive as `int`, `float` arrives as `double`. Ask for `int`/`double`, never `char`/`float`.
- There is NO way to know arg count or types at runtime — you need a convention (like printf's format string) or you'll read garbage.

---

<a id="setjmph"></a>
## 15. `<setjmp.h>` — Non-Local Jumps

Lets you jump from a deeply nested function straight back to an earlier point —
like a manual exception mechanism.

```c
int  setjmp(jmp_buf env);            // marks a point; returns 0 directly, or nonzero via longjmp
void longjmp(jmp_buf env, int val);  // "teleport" back to the setjmp point
```

```c
#include <setjmp.h>
#include <stdio.h>

jmp_buf recover_point;

void deep_function(int depth) {
    printf("depth %d\n", depth);
    if (depth == 3)
        longjmp(recover_point, 42);   // abort everything, jump back!
    else
        deep_function(depth + 1);
}

int main(void) {
    int jumped = setjmp(recover_point);   // FIRST call returns 0
    if (jumped) {
        printf("Recovered with value %d\n", jumped);  // 42
        return 0;
    }
    deep_function(0);   // prints depth 0..3 then vanishes via longjmp
}
```

Warnings:
- Variables modified between `setjmp` and `longjmp` should be `volatile` (or the behavior is undefined for non-volatile locals that changed).
- After `longjmp`, stack frames in between are abandoned — RAII-style cleanup doesn't happen.
- Rarely used today except in interpreters, coroutines, and some error-handling libraries.

---

<a id="signalh"></a>
## 16. `<signal.h>` — Signal Handling

Signals are asynchronous notifications from the OS: Ctrl+C, crashes, timers.

```c
void (*signal(int sig, void (*handler)(int)))(int);  // install handler
int  raise(int sig);                                 // send signal to yourself
```

Standard signals:

| Signal | Typical cause |
|---|---|
| `SIGINT` | Ctrl+C interrupt |
| `SIGTERM` | polite termination request (kill default) |
| `SIGSEGV` | invalid memory access (segfault!) |
| `SIGABRT` | `abort()` called / failed assertion |
| `SIGFPE` | arithmetic error (e.g. divide by zero) |
| `SIGILL` | illegal instruction |
| `SIGKILL`, `SIGSTOP` | uncatchable kill/pause (can't be handled) |

Handlers:

```c
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>

volatile sig_atomic_t running = 1;   // sig_atomic_t = safe async access type

void on_interrupt(int sig) {
    running = 0;                     // keep handlers MINIMAL — set a flag, return
}

int main(void) {
    signal(SIGINT, on_interrupt);    // now Ctrl+C won't kill us instantly
    while (running) {
        printf("Working... press Ctrl+C to stop gracefully\n");
        /* busy loop */
    }
    printf("Clean shutdown!\n");
}
```

Special handlers: `SIG_DFL` (restore default), `SIG_IGN` (ignore signal),
e.g. `signal(SIGINT, SIG_IGN);` makes Ctrl+C do nothing.

---

<a id="localeh"></a>
## 17. `<locale.h>` — Localization

Adapts formatting to regional conventions (decimal comma, currency symbols, month names in other languages).

```c
char *setlocale(int category, const char *name);
```

Categories: `LC_ALL` (everything), `LC_NUMERIC` (decimal point), `LC_TIME`
(date names), `LC_MONETARY`, `LC_COLLATE` (sort order), `LC_CTYPE` (chars).

```c
#include <locale.h>
#include <stdio.h>
#include <time.h>
#include <langinfo.h>   // POSIX only

int main(void) {
    setlocale(LC_ALL, "");        // "" = adopt user's environment locale
    // setlocale(LC_ALL, "de_DE.UTF-8");  // force German

    printf("%.2f\n", 1234.5);     // German locale may print: 1234,50

    time_t t = time(NULL);
    struct tm *lt = localtime(&t);
    char buf[64];
    strftime(buf, sizeof buf, "%A %d %B", lt);   // weekday/month in local language
    printf("%s\n", buf);
}
```

Default state is the minimal `"C"` locale — pure ASCII English. Without calling `setlocale`, nothing ever changes.

---

<a id="wide-chars"></a>
## 18. Wide Characters: `<wchar.h>`, `<wctype.h>`, `<uchar.h>`

### First: what IS a wide character?

A plain `char` is (almost always) **one byte** — it can hold 256 different
values. That's fine for ASCII/English, but Chinese, Hindi (देवनागरी), emoji, etc.
have thousands of characters. Two strategies exist:

1. **Multibyte encoding (UTF-8):** keep using `char` arrays, but some characters
   occupy 2–4 consecutive bytes. `"हि"` is several bytes. This is what modern
   programs mostly do — ordinary `char*` strings, UTF-8 encoded.
2. **Wide characters:** use a bigger fixed-size element type:

```c
wchar_t w;        // wide char: 2 bytes on Windows (UTF-16), 4 bytes on Linux (UTF-32)
wchar_t name[] = L"देवशीष";    // note the L prefix for wide string literals
```

Think of `wchar_t` vs `char` like `double` vs `char`: a wider container per element.

### `<wchar.h>` — wide versions of everything you know

| Narrow (`string.h`/`stdio.h`) | Wide equivalent |
|---|---|
| `printf` / `scanf` | `wprintf` / `wscanf` (use `%ls` for wide strings) |
| `fopen` | `_wfopen` (Windows) / `fopen` + `wcs` paths vary |
| `strlen` | `wcslen` |
| `strcpy` / `strcat` | `wcscpy` / `wcscat` |
| `strcmp` / `strncmp` | `wcscmp` / `wcsncmp` |
| `strstr` / `strchr` | `wcsstr` / `wcschr` |
| `strtok` | `wcstok` |
| `snprintf` | `swprintf` |
| `atoi` / `strtol` | `wtoi`(ext) / `wcstol` |

```c
#include <wchar.h>
#include <locale.h>

int main(void) {
    setlocale(LC_ALL, "");
    wchar_t msg[] = L"Wide string!";
    wprintf(L"Length: %zu, text: %ls\n", wcslen(msg), msg);
}
```

### `<wctype.h>` — wide char classification

Same idea as `<ctype.h>`, prefixed with `w`:

```c
iswalpha(wc) iswdigit(wc) iswspace(wc) iswupper(wc) ...
towupper(wc) towlower(wc)
```

### `<uchar.h>` (C11) — explicit Unicode types

```c
char16_t u16 = u'अ';          // UTF-16 code unit  (u'' prefix)
char32_t u32 = U'अ';          // UTF-32 code unit  (U'' prefix)
char8_t  (C23)                // UTF-8             (u8'' prefix)

size_t mbrtoc16(char16_t *pc16, const char *s, size_t n, mbstate_t *ps);
// converts multibyte (UTF-8) sequence → UTF-16, and c16rtomb goes back
```

Practical advice for learners: stick with plain `char` + UTF-8 for now; reach for `wchar_t` mainly on Windows where the OS API (`CreateFileW` etc.) uses UTF-16.

---

<a id="complex-fenv"></a>
## 19. `<complex.h>` & `<fenv.h>` — Advanced Numerics (C99)

### `<complex.h>` — complex numbers

```c
#include <complex.h>

double complex z = 3.0 + 4.0 * I;   // I = imaginary unit
printf("|z| = %f\n", cabs(z));      // 5.0
printf("real=%f imag=%f\n", creal(z), cimag(z));

double complex w = cexp(I * 3.14159);  // e^{iπ} ≈ -1 (Euler!)
```

Functions mirror `<math.h>` with a `c` prefix: `csqrt`, `cpow`, `csin`, `clog`, `cabs`, `carg` (angle). Types: `float complex`, `double complex`, `long double complex`.

### `<fenv.h>` — floating-point environment

Controls rounding mode and detects FP exceptions (overflow, divide-by-zero) without trapping:

```c
#include <fenv.h>

feclearexcept(FE_ALL_EXCEPT);
double x = 1.0 / 3.0;
if (fetestexcept(FE_INEXACT))  printf("Result was rounded\n");

fesetround(FE_UPWARD);        // FE_TONEAREST (default), FE_DOWNWARD, FE_TOWARDZERO
```

Used mainly in numerical/scientific computing. Most beginners never need it.

---

<a id="keyword-macros"></a>
## 20. `<iso646.h>`, `<stdalign.h>`, `<stdnoreturn.h>` — Keyword Macros

### `<iso646.h>` (C95) — word operators

For keyboards/environments lacking symbols:

| Macro | Means | Macro | Means |
|---|---|---|---|
| `and` | `&&` | `bitand` | `&` |
| `or` | `\|\|` | `bitor` | `\|` |
| `not` | `!` | `xor` | `^` |
| `not_eq` | `!=` | `compl` | `~` |
| `and_eq` | `&=` | `or_eq` | `\|=` |
| `xor_eq` | `^=` | | |

```c
if (a > 0 and b > 0) { ... }   // legal C95+, rarely seen in practice
```

### `<stdalign.h>` (C11)

```c
alignas(16) char buffer[64];        // force 16-byte alignment (SIMD, caches)
size_t a = alignof(double);         // alignment requirement of a type (= _Alignof)
```

### `<stdnoreturn.h>` (C11)

```c
#include <stdnoreturn.h>
noreturn void fatal_error(const char *msg) {   // promises: never comes back
    fprintf(stderr, "%s\n", msg);
    exit(1);
}
```

Helps compilers optimize/warn. All three headers are obsolete in **C23** (their macros became real keywords).

---

<a id="concurrency"></a>
## 21. `<threads.h>` & `<stdatomic.h>` — Concurrency (C11)

⚠️ MSVC still lacks `<threads.h>`; MinGW-w64 and glibc support it.

### `<threads.h>` — native threads

```c
#include <threads.h>
#include <stdio.h>

int worker(void *arg) {
    int id = *(int *)arg;
    printf("Thread %d running\n", id);
    return id;
}

int main(void) {
    thrd_t t;
    int id = 7;
    if (thrd_create(&t, worker, &id) == thrd_success) {
        int result;
        thrd_join(t, &result);          // wait for completion
        printf("Thread returned %d\n", result);
    }
}
```

Key functions: `thrd_create`, `thrd_join`, `thrd_detach`, `thrd_sleep`, `mtx_init/lock/unlock/destroy` (mutexes), `cnd_init/wait/signal/broadcast`(condition variables), `tss_create/get/set` (thread-local storage).

### `<stdatomic.h>` — lock-free atomic operations

```c
#include <stdatomic.h>

atomic_int counter = 0;

counter++;                              // thread-safe increment, no mutex needed!
atomic_fetch_add(&counter, 5);          // explicit version
int v = atomic_load(&counter);
atomic_store(&counter, 10);
atomic_compare_exchange_strong(&counter, &expected, newval);  // CAS
```

Memory ordering parameters (`memory_order_relaxed`, `acquire`, `release`, `seq_cst` default) control how visible changes are across threads — a deep topic; use the defaults until you study concurrency formally.

---

<a id="c23"></a>
## 22. C23 Additions: `<stdbit.h>`, `<stdckdint.h>`

### `<stdbit.h>` — bit manipulation

Generic (type-generic macro) bit utilities:

```c
stdc_count_ones(x)        // population count: number of 1-bits
stdc_count_zeros(x)       // number of 0-bits
stdc_leading_zeros(x)     // leading zero count
stdc_trailing_zeros(x)    // trailing zeros
stdc_bit_width(x)         // bits needed to represent x
stdc_has_single_bit(x)    // is x a power of two?
stdc_bit_floor(x)         // largest power of two ≤ x
stdc_bit_ceil(x)          // smallest power of two ≥ x
```

### `<stdckdint.h>` — checked integer arithmetic

Detects overflow instead of invoking undefined behavior:

```c
#include <stdckdint.h>

int a = 2000000000, b = 2000000000, result;
if (ckd_add(&result, a, b))
    printf("Addition overflowed!\n");       // fires: 4e9 > INT_MAX
if (ckd_mul(&result, a, 3))
    printf("Multiplication overflowed!\n");
```

Also `ckd_sub`. Works on any integer type — invaluable for parsers, allocators, and security-sensitive code.

---

<a id="roadmap"></a>
## 23. Learning Roadmap (in order)

| Stage | Headers | Why |
|---|---|---|
| **Beginner** (now) | `stdio.h`, `stdlib.h`, `ctype.h`, `string.h` (basics), `stdbool.h`, `math.h` | Everything in Basics + Conditionals + Loops exercises |
| **Intermediate** | `string.h` (deep), `limits.h`, `float.h`, `assert.h`, `errno.h`, `time.h`, `stddef.h`, `stdint.h` | Writing robust programs, debugging, measuring |
| **Advanced** | `stdlib.h` (dynamic memory mastery), `stdarg.h`, `signal.h`, `setjmp.h`, `inttypes.h` | Data structures, libraries, systems code |
| **Specialist** | `threads.h`, `stdatomic.h`, `wchar.h`, `complex.h`, `fenv.h`, `locale.h`, C23 stuff | Concurrency, Unicode, scientific computing |

### Cheat-sheet: which header when?

```
Printing/reading/files ............ stdio.h
malloc/free/conversions/qsort ..... stdlib.h
strlen/strcmp/copy/search ......... string.h
isalpha/toupper ................... ctype.h
sqrt/pow/sin ...................... math.h
INT_MAX, DBL_EPSILON .............. limits.h / float.h
exact-width ints .................. stdint.h (+ inttypes.h to print)
bool/true/false ................... stdbool.h
size_t/NULL/offsetof .............. stddef.h
dates, timing ..................... time.h
debugging contracts ............... assert.h
error codes ....................... errno.h
your own printf-like fn ........... stdarg.h
Ctrl+C handling ................... signal.h
threads/atomics ................... threads.h / stdatomic.h
```

---

<a id="practice"></a>
## 24. Practice Playground — Mini Projects Per Topic

Reading ≠ learning. After each chapter, build one of these (each is 20–60 min):

| Topic | Mini project | Headers you'll practice |
|---|---|---|
| printf formatting | Print a neat multiplication table (aligned columns with `%4d`) | `stdio.h` |
| scanf + conditionals | Temperature converter (C ↔ F), with invalid-input handling | `stdio.h` |
| Conditionals | Leap-year checker; grade calculator (marks → letter grade) | `stdio.h` |
| Loops | FizzBuzz; sum of digits; reverse a number; pyramid patterns | `stdio.h` |
| `ctype.h` | Vowel/digit/punctuation counter for a sentence | `ctype.h`, `stdio.h` |
| `string.h` | Palindrome checker; count words in a sentence; uppercase converter | `string.h`, `ctype.h` |
| `math.h` | Quadratic solver (from §6); hypotenuse calculator | `math.h` |
| `stdlib.h` | Dice-roll simulator (`srand`/`rand`); number guessing game | `stdlib.h`, `time.h` |
| `limits.h` | Find max/min in an array starting from `INT_MIN`/`INT_MAX` | `limits.h` |
| File I/O | Save a to-do list to `todo.txt`, reload on next run | `stdio.h` |
| `time.h` | Stopwatch: measure how long the user takes to answer a quiz | `time.h` |
| `assert.h` | Add assertions to an older exercise, then break them on purpose | `assert.h` |
| **Capstone** | Contact book: menu (add/list/search/sort) using arrays + `qsort` + file save/load | `stdio.h`, `string.h`, `stdlib.h` |

Rule of thumb: when a project feels easy, add ONE feature (input validation,
a second file, sorting) — that's where real learning happens.

---

<a id="compile-run"></a>
## 25. Compile & Run Quickstart (Windows + gcc/MinGW)

```powershell
cd "C:\Users\Devashish\Desktop\VS Code\C_Language_learning\01_Basics"
gcc -Wall -Wextra 01_Printing.c -o 01_Printing.exe   # compile (warnings ON!)
.\01_Printing.exe                                    # run
```

- `-Wall -Wextra` — show all warnings. **Never skip these while learning**;
  warnings are free bug reports.
- `-g` — include debug info so VS Code's debugger can step through your code.
- `-o name` — output filename (otherwise `a.exe`).
- One command chain: `gcc -Wall prog.c -o prog.exe ; .\prog.exe`
- If `gcc` isn't recognized, install [MinGW-w64](https://www.mingw-w64.org/)
  or MSYS2 and add its `bin` folder to PATH.

Debugging workflow when something misbehaves:
1. Read the FIRST warning/error, fix, recompile.
2. Still broken? Add temporary `printf("x=%d\\n", x);` lines to trace values.
3. Still broken? Set a breakpoint in VS Code (F5 with a `launch.json` using
   the C/C++ extension) and inspect variables step by step.

---
*Generated as a personal study reference — pair each section with small practice programs in your `01_Basics/` and `02_Conditionals/` folders.*

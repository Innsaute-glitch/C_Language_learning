# 🛠️ C: From Scratch to Advanced — The Missing Manual

> This is the "everything else" guide: how C actually gets built, how memory really
> works, what undefined behavior is, how to debug and test, how to build real data
> structures, and how to write C that doesn't fall over.
>
> Self-contained. Start at section 1 if you've never compiled a program. Skip to any
> section once you know your way around.
>
> Every code block is runnable. Where a snippet needs `#include`s to compile on its
> own, they're shown.

---

## Table of Contents

**Part I — The Machine Under You**
1. [What actually happens when you compile](#build-pipeline)
2. [Setting up a real toolchain (Windows first)](#toolchain)
3. [Compiler flags that matter](#flags)
4. [The preprocessor, properly](#preprocessor)
5. [Multi-file projects, headers, and linkage](#multifile)
6. [Build automation: Make, then CMake](#build-automation)

**Part II — The Language, Deeply**
7. [How to read any C declaration](#declarations)
8. [The memory model: stack, heap, static, and lifetimes](#memory-model)
9. [Pointers, for real this time](#pointers)
10. [Arrays, decay, and multidimensional data](#arrays)
11. [Structs, unions, enums, padding, and alignment](#structs)
12. [Strings and ownership](#strings)
13. [Integer conversions and the promotion traps](#integers)
14. [Floating point: what the hardware actually does](#floats)
15. [Bit manipulation](#bits)
16. [Undefined behavior: the catalogue](#ub)

**Part III — Working Like an Engineer**
17. [Debugging: gdb, sanitizers, valgrind](#debugging)
18. [Testing C without a framework, then with one](#testing)
19. [Error handling patterns](#errors)
20. [Command-line programs: argc, argv, exit codes, pipes](#cli)
21. [Security: the C-specific OWASP](#security)
22. [Performance: measure, then optimize](#performance)
23. [Portability and platform detection](#portability)

**Part IV — Building Things**
24. [Dynamic array (vector)](#vector)
25. [Linked list](#linked-list)
26. [Stack and queue](#stack-queue)
27. [Binary search tree](#bst)
28. [Hash table](#hashtable)
29. [Generic code with `void *` and function pointers](#generics)
30. [API design: opaque types and modules](#api-design)

**Part V — Advanced and Modern**
31. [Recursion, and when to kill it](#recursion)
32. [Concurrency: threads, races, mutexes, atomics](#concurrency)
33. [C89 → C23: what changed and what to use](#standards)
34. [Style guide](#style)

**Part VI — Reference**
35. [Learning roadmap with projects](#roadmap)
36. [Cheat sheets](#cheatsheets)
37. [Glossary](#glossary)

---

<a id="build-pipeline"></a>
# Part I — The Machine Under You

## 1. What Actually Happens When You Compile

`gcc main.c -o main` looks like one step. It's four. Knowing which stage produced
your error cuts debugging time enormously.

```
main.c
  │
  │  ① PREPROCESSOR  (cpp)
  │     - strips comments
  │     - pastes in every #include, recursively
  │     - expands every #define macro
  │     - resolves #if / #ifdef conditionals
  ▼
main.i          ← still C source, just enormous (often 30,000+ lines)
  │
  │  ② COMPILER  (cc1)
  │     - parses C, type-checks
  │     - optimizes
  │     - emits assembly for your CPU
  ▼
main.s          ← human-readable assembly
  │
  │  ③ ASSEMBLER  (as)
  │     - assembly → machine code
  │     - leaves "holes" where external symbols go
  ▼
main.o          ← object file: real machine code, incomplete
  │
  │  ④ LINKER  (ld)
  │     - combines all .o files
  │     - fills the holes (resolves printf, sqrt, your functions)
  │     - attaches the C standard library
  │     - adds startup code that calls main()
  ▼
main.exe        ← executable
```

### Run each stage yourself

Do this once. It demystifies everything.

```bash
gcc -E main.c -o main.i      # ① preprocess only
gcc -S main.c -o main.s      # ② compile to assembly
gcc -c main.c -o main.o      # ③ assemble to object file
gcc main.o -o main           # ④ link
```

Try it on the smallest possible program:

```c
#include <stdio.h>

int main(void) {
    printf("hi\n");
    return 0;
}
```

Then `wc -l main.i` (or open it). You'll see thousands of lines — that's
`<stdio.h>` and everything it includes. **This is why `#include` in a header
matters:** it multiplies.

### Which stage produced my error?

| Error text | Stage | Meaning |
|---|---|---|
| `No such file or directory: 'foo.h'` | ① Preprocessor | Include path wrong |
| `unterminated #ifdef` | ① Preprocessor | Missing `#endif` |
| `expected ';' before '}'` | ② Compiler | Syntax |
| `'x' undeclared` | ② Compiler | Name not visible here |
| `implicit declaration of function 'foo'` | ② Compiler | Missing prototype / `#include` |
| `incompatible pointer types` | ② Compiler | Type mismatch |
| `undefined reference to 'foo'` | ④ **Linker** | Declared but never *defined*, or you forgot to compile/link the `.c` file |
| `multiple definition of 'foo'` | ④ **Linker** | Same symbol defined in two `.o` files |
| `cannot find -lm` | ④ Linker | Library not installed/found |
| *Segmentation fault* | **Runtime** | Bad memory access — the compiler is done, this is your program misbehaving |

> 🔑 **The single most useful diagnostic skill:** "undefined reference" is *always*
> a linker problem. The compiler believed you when you declared the function. The
> linker went looking for the body and found nothing. Either you didn't write it,
> or you didn't add that `.c` file to the build command.

```bash
# ❌ undefined reference to 'helper'
gcc main.c -o app

# ✅ tell the linker about the other file
gcc main.c helper.c -o app
```

### What `main` really looks like

```c
int main(void);                       // ✅ takes no arguments
int main(int argc, char *argv[]);     // ✅ takes command-line arguments
void main(void);                      // ❌ non-standard, don't
```

`main` returns `int` to the operating system. `0` means success. Non-zero means
failure. If you omit `return 0;` in C99 or later, the compiler inserts it for
`main` only — but write it anyway for clarity.

---

<a id="toolchain"></a>
## 2. Setting Up a Real Toolchain (Windows First)

You're on Windows, so here are your three real options.

### Option A — MSYS2 + GCC (recommended for learning)

Gives you genuine GCC, gdb, and make, matching every tutorial and textbook.

1. Install from [msys2.org](https://www.msys2.org/).
2. Open the **MSYS2 UCRT64** terminal and run:

```bash
pacman -Syu
pacman -S mingw-w64-ucrt-x86_64-gcc mingw-w64-ucrt-x86_64-gdb make
```

3. Add `C:\msys64\ucrt64\bin` to your Windows `PATH`.
4. Verify in a normal PowerShell:

```powershell
gcc --version
gdb --version
```

### Option B — MSVC (Microsoft's compiler)

Install "Visual Studio Build Tools" with the C++ workload. Use the
**Developer Command Prompt**:

```bat
cl /W4 /Zi main.c
```

MSVC is excellent but its flags and diagnostics differ from GCC/Clang, and its C
support historically trailed. Fine for Windows-native work; more friction while
learning from books.

### Option C — WSL2 (Linux inside Windows)

```powershell
wsl --install
```

Then inside Ubuntu:

```bash
sudo apt update
sudo apt install build-essential gdb valgrind
```

This gets you the full Linux toolchain including **valgrind** and the best
sanitizer support. If you plan to go deep, install this eventually.

### VS Code configuration

Create `.vscode/tasks.json` to build with one keystroke (`Ctrl+Shift+B`):

```json
{
  "version": "2.0.0",
  "tasks": [
    {
      "label": "build active file",
      "type": "shell",
      "command": "gcc",
      "args": [
        "-std=c17",
        "-Wall",
        "-Wextra",
        "-g",
        "${file}",
        "-o",
        "${fileDirname}/${fileBasenameNoExtension}.exe"
      ],
      "group": { "kind": "build", "isDefault": true },
      "problemMatcher": ["$gcc"]
    }
  ]
}
```

And `.vscode/launch.json` to debug with `F5`:

```json
{
  "version": "0.2.0",
  "configurations": [
    {
      "name": "Debug active file",
      "type": "cppdbg",
      "request": "launch",
      "program": "${fileDirname}/${fileBasenameNoExtension}.exe",
      "args": [],
      "stopAtEntry": false,
      "cwd": "${fileDirname}",
      "externalConsole": false,
      "MIMode": "gdb",
      "miDebuggerPath": "C:/msys64/ucrt64/bin/gdb.exe",
      "preLaunchTask": "build active file"
    }
  ]
}
```

The `"problemMatcher": ["$gcc"]` line is the one people miss — it's what makes
compiler errors clickable in the Problems panel.

---

<a id="flags"></a>
## 3. Compiler Flags That Matter

### The set you should always use while learning

```bash
gcc -std=c17 -Wall -Wextra -g -fsanitize=address,undefined main.c -o main
```

| Flag | Does |
|---|---|
| `-std=c17` | Pin the language version. Without it you get a compiler-specific default |
| `-Wall` | Enable common warnings (misleading name — it is *not* all warnings) |
| `-Wextra` | Enable more warnings that `-Wall` skips |
| `-g` | Include debug info so gdb can show variable names and line numbers |
| `-fsanitize=address` | Catch buffer overflows, use-after-free, leaks **at runtime** |
| `-fsanitize=undefined` | Catch signed overflow, bad shifts, null deref, misalignment |
| `-O0` | No optimization (default) — fastest builds, most faithful debugging |

### Warnings worth adding

```bash
-Wshadow            # a local variable hides an outer one
-Wconversion        # implicit conversions that may lose data
-Wstrict-prototypes # f() with no parameter list
-Wpedantic          # strictly enforce the standard
-Werror             # turn every warning into a hard error
```

`-Werror` feels harsh. Turn it on anyway. Warnings you don't fix become bugs you
ship. C's type system is loose enough that warnings are your real safety net.

### Optimization levels

| Level | Use for |
|---|---|
| `-O0` | Debugging. Code maps 1:1 to source |
| `-O1` | Light optimization, still debuggable |
| `-O2` | **The normal release choice.** Aggressive but safe |
| `-O3` | More inlining/vectorizing. Sometimes slower than `-O2`. Measure |
| `-Os` | Optimize for binary size |
| `-Og` | Optimize but keep debugging usable |

> ⚠️ **A program that works at `-O0` but breaks at `-O2` almost always contains
> undefined behavior.** The optimizer is allowed to assume UB never happens, so it
> deletes or reorders code around your bug. Don't blame the compiler — run the
> sanitizers. See [section 16](#ub).

### Linking libraries

```bash
gcc main.c -o main -lm          # math library (Linux/macOS; auto on Windows)
gcc main.c -o main -lpthread    # POSIX threads
gcc main.c -o main -L./lib -lmylib   # -L adds a search dir, -l names the library
```

Note: `-l` flags go **after** your source files. The linker processes left to
right and only pulls in what's still unresolved.

---

<a id="preprocessor"></a>
## 4. The Preprocessor, Properly

The preprocessor is a **text substitution engine** that runs before C is parsed.
It doesn't know types, scope, or expressions. That's the source of every macro bug.

### 4.1 Object-like macros

```c
#define MAX_USERS 100
#define GREETING  "hello"
#define PI 3.14159265358979323846
```

Prefer `const` or `enum` for typed constants in modern C:

```c
enum { MAX_USERS = 100 };            // integer constant, visible to debugger
static const double PI = 3.14159265358979323846;
```

Macros survive because they work in contexts requiring compile-time constants
(array sizes in C89, `#if` conditions, `static` initializers).

### 4.2 Function-like macros and the parenthesis rule

```c
#define SQUARE(x) x * x              // ❌ broken
```

Watch it break:

```c
SQUARE(2 + 3)    →  2 + 3 * 2 + 3   →  11    // expected 25
10 / SQUARE(2)   →  10 / 2 * 2      →  10    // expected 2.5
```

**Rule: parenthesize every parameter AND the whole body.**

```c
#define SQUARE(x) ((x) * (x))        // ✅
```

Now `SQUARE(2 + 3)` → `((2 + 3) * (2 + 3))` → 25. Correct.

### 4.3 Macros are not functions: double evaluation

```c
#define MAX(a, b) ((a) > (b) ? (a) : (b))

int i = 5;
int m = MAX(i++, 3);     // ❌ i++ evaluated TWICE — i becomes 7, m is garbage
```

Any argument with a side effect (`i++`, `getchar()`, a function call) is a bug
waiting to happen. In C11+ you can write a real inline function instead:

```c
static inline int max_int(int a, int b) { return a > b ? a : b; }
```

Type-generic version with `_Generic` (C11):

```c
#define MAX(a, b) _Generic((a), \
    int:    max_int,            \
    long:   max_long,           \
    double: max_double          \
)(a, b)
```

### 4.4 Multi-statement macros

```c
#define SWAP(a, b) do {          \
    int tmp_ = (a);              \
    (a) = (b);                   \
    (b) = tmp_;                  \
} while (0)
```

The `do { } while (0)` wrapper makes the macro behave as a single statement so
this works:

```c
if (x > y) SWAP(x, y);      // ✅ needs the do-while form to be correct
```

The trailing `_` on `tmp_` reduces the chance of shadowing a caller's variable.

### 4.5 Conditional compilation

```c
#ifdef DEBUG
    printf("x = %d\n", x);
#endif

#ifndef NDEBUG
    /* assertions active */
#endif

#if defined(_WIN32)
    #include <windows.h>
#elif defined(__linux__)
    #include <unistd.h>
#else
    #error "Unsupported platform"
#endif

#if __STDC_VERSION__ >= 201112L
    /* C11 or later available */
#endif
```

A debug-print macro that costs nothing in release builds:

```c
#ifdef DEBUG
  #define DBG(fmt, ...) fprintf(stderr, "[%s:%d] " fmt "\n", \
                                __FILE__, __LINE__, __VA_ARGS__)
#else
  #define DBG(fmt, ...) ((void)0)
#endif

DBG("count=%d name=%s", count, name);
```

Compile with `-DDEBUG` to enable.

### 4.6 Stringify `#` and paste `##`

```c
#define STR(x)  #x               // turn token into string literal
#define XSTR(x) STR(x)           // expand macro first, THEN stringify
#define CONCAT(a, b) a##b        // glue tokens together

STR(hello)       →  "hello"
STR(MAX_USERS)   →  "MAX_USERS"      // not "100"!
XSTR(MAX_USERS)  →  "100"            // the double-expansion idiom
CONCAT(my, var)  →  myvar
```

The `XSTR` trick is worth memorizing — single-level stringify does *not* expand
its argument.

### 4.7 Predefined macros

| Macro | Value |
|---|---|
| `__FILE__` | current source filename |
| `__LINE__` | current line number |
| `__func__` | current function name (C99) |
| `__DATE__`, `__TIME__` | compile date/time |
| `__STDC_VERSION__` | `199901L`, `201112L`, `201710L`, `202311L` |
| `_WIN32`, `__linux__`, `__APPLE__` | platform |
| `__GNUC__`, `_MSC_VER`, `__clang__` | compiler |

### 4.8 A minimal assert-with-message

```c
#define REQUIRE(cond, msg)                                        \
    do {                                                          \
        if (!(cond)) {                                            \
            fprintf(stderr, "%s:%d: %s (failed: %s)\n",           \
                    __FILE__, __LINE__, msg, #cond);              \
            abort();                                              \
        }                                                         \
    } while (0)
```

### 4.9 When you should debug a macro

```bash
gcc -E -P main.c | less        # see exactly what the compiler will see
```

`-P` suppresses line markers, making output readable. If a macro is misbehaving,
stop guessing and look at the expansion.

---

<a id="multifile"></a>
## 5. Multi-File Projects, Headers, and Linkage

A **translation unit** = one `.c` file plus everything it `#include`s, after
preprocessing. The compiler handles one translation unit at a time and knows
nothing about the others. The linker joins them.

### 5.1 The header/source split

The rule: **headers declare, sources define.**

`geometry.h` — the interface, what callers need:

```c
#ifndef GEOMETRY_H
#define GEOMETRY_H

typedef struct {
    double x, y;
} Point;

/* Straight-line distance between two points. */
double distance(Point a, Point b);

/* Area of the triangle formed by three points; 0 if collinear. */
double triangle_area(Point a, Point b, Point c);

#endif /* GEOMETRY_H */
```

`geometry.c` — the implementation:

```c
#include "geometry.h"
#include <math.h>

double distance(Point a, Point b) {
    return hypot(a.x - b.x, a.y - b.y);
}

double triangle_area(Point a, Point b, Point c) {
    double cross = (b.x - a.x) * (c.y - a.y) - (c.x - a.x) * (b.y - a.y);
    return fabs(cross) / 2.0;
}
```

`main.c` — a consumer:

```c
#include <stdio.h>
#include "geometry.h"

int main(void) {
    Point a = {0, 0}, b = {3, 0}, c = {0, 4};
    printf("distance a-b : %.2f\n", distance(a, b));
    printf("triangle area: %.2f\n", triangle_area(a, b, c));
    return 0;
}
```

Build:

```bash
gcc -std=c17 -Wall -Wextra -g main.c geometry.c -o app
```

Note `geometry.c` includes its own header. That's deliberate — it makes the
compiler verify the definitions match the declarations.

### 5.2 Include guards

Without a guard, including a header twice redefines its types:

```c
#ifndef GEOMETRY_H       // if not yet defined...
#define GEOMETRY_H       // ...define it...
   /* contents */
#endif                   // ...so the second include skips everything
```

`#pragma once` does the same in one line and is supported by every mainstream
compiler, though it isn't in the standard:

```c
#pragma once
```

Either is fine. Be consistent.

### 5.3 `#include "..."` vs `#include <...>`

```c
#include <stdio.h>       // search system include directories
#include "geometry.h"    // search the current file's directory first, then system
```

Use quotes for your own headers, angle brackets for library headers.

### 5.4 Never `#include` a `.c` file

```c
#include "helper.c"      // ❌ this is why you get "multiple definition" errors
```

You'd paste the function bodies into every including file, and the linker then
sees the same symbol many times. Include the `.h`, compile the `.c` separately.

### 5.5 Linkage: `static` and `extern`

| Keyword | At file scope | Inside a function |
|---|---|---|
| *(nothing)* | **External linkage** — visible to other files | Automatic (local) variable |
| `static` | **Internal linkage** — private to this file | Variable persists across calls |
| `extern` | Declares something defined elsewhere | Same |

```c
/* counter.c */
static int call_count = 0;          // private to counter.c
int total_calls = 0;                // visible everywhere

static int helper(void) {            // private helper function
    return ++call_count;
}

int record_call(void) {              // public API
    total_calls++;
    return helper();
}
```

```c
/* main.c */
extern int total_calls;              // "it exists, defined in another file"
int record_call(void);
```

Better: put `extern int total_calls;` in `counter.h` so everyone agrees.

> 🔑 **Default to `static` for anything not in your header.** It keeps your
> namespace clean, prevents accidental symbol collisions, and lets the compiler
> optimize more aggressively because it can see every call site.

### 5.6 Global variables done right

Globals in headers are a classic mistake:

```c
/* config.h */
int verbose = 0;        // ❌ DEFINITION in a header → multiple definition error
```

```c
/* config.h */
extern int verbose;     // ✅ declaration only
```

```c
/* config.c */
int verbose = 0;        // ✅ exactly one definition
```

### 5.7 Function-local `static`

```c
int next_id(void) {
    static int id = 0;       // initialized once, survives between calls
    return ++id;
}
```

Handy, but it's hidden mutable state — not thread-safe and hard to test. Use
sparingly.

### 5.8 The One Definition Rule, in practice

| Thing | Where it goes |
|---|---|
| `typedef`, `struct` definition | Header |
| `enum` definition | Header |
| Function *prototype* | Header |
| Function *body* | Source file |
| `extern` variable declaration | Header |
| Variable *definition* | Source file (exactly one) |
| `static inline` function | Header is fine (each TU gets its own copy) |
| Macro | Header |

### 5.9 Recommended project layout

```
project/
  include/          public headers
    geometry.h
  src/              implementation
    main.c
    geometry.c
    geometry_internal.h    (private header, not in include/)
  tests/
    test_geometry.c
  Makefile
```

Compile with `-Iinclude` so `#include "geometry.h"` resolves:

```bash
gcc -std=c17 -Wall -Wextra -Iinclude src/*.c -o app
```

---

<a id="build-automation"></a>
## 6. Build Automation: Make, then CMake

Typing the full gcc command every time is fine for one file and miserable for ten.

### 6.1 Make

`make` rebuilds only what changed, based on file timestamps.

> ⚠️ **Makefile recipe lines must begin with a literal TAB, not spaces.** This is
> the number one Makefile error. In VS Code, check that the file isn't set to
> convert tabs to spaces.

```make
CC       := gcc
CFLAGS   := -std=c17 -Wall -Wextra -g -Iinclude
LDFLAGS  := -lm

SRC      := $(wildcard src/*.c)
OBJ      := $(SRC:.c=.o)
TARGET   := app

# First target is the default, so `make` alone builds the app.
$(TARGET): $(OBJ)
    $(CC) $(OBJ) -o $@ $(LDFLAGS)

# Pattern rule: how to make any .o from its .c
%.o: %.c
    $(CC) $(CFLAGS) -c $< -o $@

run: $(TARGET)
    ./$(TARGET)

clean:
    rm -f $(OBJ) $(TARGET)

.PHONY: run clean
```

Automatic variables you'll see constantly:

| Variable | Means |
|---|---|
| `$@` | the target being built |
| `$<` | the first prerequisite |
| `$^` | all prerequisites, deduplicated |
| `$?` | prerequisites newer than the target |

On Windows without MSYS2, replace `rm -f` with `del /Q` and `./$(TARGET)` with
`$(TARGET)`.

`.PHONY` tells make that `clean` and `run` are commands, not files to produce.
Without it, a file literally named `clean` would break the rule.

### 6.2 Header dependencies

The Makefile above has a flaw: edit `geometry.h` and nothing rebuilds, because
make only sees `.c` files. Fix it by having the compiler generate dependency info:

```make
CFLAGS += -MMD -MP
DEPS := $(OBJ:.o=.d)

-include $(DEPS)          # the leading - suppresses "no such file" on first run
```

Add `$(DEPS)` to your `clean` rule. Now header edits trigger correct rebuilds.

### 6.3 CMake

CMake generates build files for whatever toolchain you have (Make, Ninja, MSVC,
Xcode). It's what most real projects use.

`CMakeLists.txt`:

```cmake
cmake_minimum_required(VERSION 3.16)
project(app C)

set(CMAKE_C_STANDARD 17)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)   # gives clangd/VS Code perfect IntelliSense

add_executable(app
    src/main.c
    src/geometry.c
)

target_include_directories(app PRIVATE include)

if(MSVC)
    target_compile_options(app PRIVATE /W4)
else()
    target_compile_options(app PRIVATE -Wall -Wextra)
    target_link_libraries(app PRIVATE m)
endif()
```

Build:

```bash
cmake -S . -B build
cmake --build build
```

`CMAKE_EXPORT_COMPILE_COMMANDS` writes `build/compile_commands.json`. Point your
editor at it and IntelliSense stops guessing about your include paths.

---

<a id="declarations"></a>
# Part II — The Language, Deeply

## 7. How to Read Any C Declaration

C's declaration syntax is genuinely confusing. There's a mechanical rule that
always works.

### The spiral / right-left rule

1. Find the **identifier** (the name).
2. Read **right** if the next symbol is `[` or `(`.
3. Read **left** for `*`.
4. Keep alternating outward; parentheses group.
5. The base type comes last.

Worked examples:

```c
int x;
// x is an int

int *p;
// p is a pointer to int

int arr[10];
// arr is an array of 10 int

int *pa[10];
// pa is an array of 10 pointers to int
//    (go right first: [10], then left: *)

int (*ap)[10];
// ap is a pointer to an array of 10 int
//    (parens force * to bind first)

int f(void);
// f is a function taking void, returning int

int *f(void);
// f is a function returning a pointer to int

int (*fp)(void);
// fp is a pointer to a function taking void, returning int

int (*fpa[5])(void);
// fpa is an array of 5 pointers to functions returning int

char *argv[];
// argv is an array of pointers to char   (i.e. an array of strings)

char **argv;
// argv is a pointer to a pointer to char  (equivalent as a parameter)

const char *s;
// s is a pointer to a const char   → can't modify the characters
//                                  → CAN point somewhere else

char * const s;
// s is a const pointer to char     → CAN modify characters
//                                  → can't repoint

const char * const s;
// both locked

int *(*f)(int, char *);
// f is a pointer to a function taking (int, char*), returning pointer to int
```

### `const` reading trick

Read `const` as applying to whatever is **immediately to its left**; if there's
nothing to its left, it applies to the right.

```c
const int *p;     // == int const *p;   → const int, pointer to it
int *const p;     // const pointer
```

### Make it readable with `typedef`

```c
typedef int (*IntPredicate)(int);        // name the ugly type once

IntPredicate is_even;                    // now trivially readable
int count_matching(const int *a, size_t n, IntPredicate pred);
```

```c
#include <stdio.h>
#include <stddef.h>

typedef int (*IntPredicate)(int);

static int is_even(int v) { return v % 2 == 0; }
static int is_positive(int v) { return v > 0; }

static size_t count_matching(const int *a, size_t n, IntPredicate pred) {
    size_t count = 0;
    for (size_t i = 0; i < n; i++)
        if (pred(a[i])) count++;
    return count;
}

int main(void) {
    int data[] = {-3, -2, 0, 1, 4, 7, 8};
    size_t n = sizeof data / sizeof data[0];

    printf("even     : %zu\n", count_matching(data, n, is_even));
    printf("positive : %zu\n", count_matching(data, n, is_positive));
    return 0;
}
```

That last example is your first taste of **callbacks** — passing behavior as data.
See [section 29](#generics).

---

<a id="memory-model"></a>
## 8. The Memory Model: Stack, Heap, Static, and Lifetimes

Every variable lives somewhere, for some duration. Get this model straight and
most pointer bugs become obvious.

```
HIGH addresses
┌─────────────────────────────┐
│  Command-line args & env    │
├─────────────────────────────┤
│  STACK                      │  local variables, function parameters,
│    │ grows downward ↓       │  return addresses. Automatic lifetime.
│    ▼                        │  Fast. Small (typically 1–8 MB).
│                             │
│         (free space)        │
│                             │
│    ▲ grows upward ↑         │
│  HEAP                       │  malloc/calloc/realloc. Manual lifetime.
├─────────────────────────────┤  Large. You must free().
│  BSS                        │  uninitialized globals & statics → zeroed
├─────────────────────────────┤
│  DATA                       │  initialized globals & statics
├─────────────────────────────┤
│  TEXT (code) + RODATA       │  machine code, string literals.
└─────────────────────────────┘  Read-only — writing here crashes.
LOW addresses
```

### 8.1 Storage duration — the four kinds

| Duration | Created by | Lives until | Initialized? |
|---|---|---|---|
| **Automatic** | local variable | end of its enclosing block | ❌ garbage unless you initialize |
| **Static** | global, or `static` local | program exit | ✅ zeroed automatically |
| **Allocated** | `malloc`/`calloc`/`realloc` | you call `free` | `malloc` no, `calloc` yes |
| **Thread** | `_Thread_local` (C11) | thread exit | ✅ zeroed |

```c
#include <stdio.h>
#include <stdlib.h>

int global_counter;                 // static duration, BSS, auto-zeroed → 0

void demo(void) {
    int automatic;                  // ❌ GARBAGE — not zero
    static int persistent;          // ✅ 0 on first call, keeps value after
    int *heap = malloc(sizeof *heap);  // ❌ contents garbage
    int *zeroed = calloc(1, sizeof *zeroed);  // ✅ contents 0

    persistent++;
    printf("persistent = %d\n", persistent);

    free(heap);
    free(zeroed);
}

int main(void) {
    demo();   // persistent = 1
    demo();   // persistent = 2
    demo();   // persistent = 3
    return 0;
}
```

> 🔑 **Globals and statics are zero-initialized. Locals and `malloc` are not.**
> This single asymmetry causes an enormous number of "works sometimes" bugs.

### 8.2 Stack frames

Each function call pushes a frame containing its parameters, locals, and the
return address. On return, the frame is popped — instantly, no cleanup code.

```c
void inner(int b) {          // ← frame 3
    int local = b * 2;
}

void outer(int a) {          // ← frame 2
    inner(a + 1);
}

int main(void) {             // ← frame 1
    outer(5);
    return 0;
}
```

This is why returning a pointer to a local is catastrophic:

```c
int *broken(void) {
    int value = 42;
    return &value;        // ❌ frame is destroyed on return — dangling pointer
}

int *fine(void) {
    int *value = malloc(sizeof *value);
    if (!value) return NULL;
    *value = 42;
    return value;         // ✅ heap survives; caller must free()
}

const char *also_fine(void) {
    return "literal";     // ✅ string literals live in RODATA, whole program
}

int *static_ok(void) {
    static int value = 42;
    return &value;        // ✅ static duration — but shared by all callers!
}
```

`-Wall` catches the first case with *"function returns address of local
variable"*. Listen to it.

### 8.3 Stack overflow

The stack is finite. Two ways to blow it:

```c
void infinite(int n) {
    char padding[1024];
    infinite(n + 1);          // ❌ no base case → stack overflow
}

void too_big(void) {
    int huge[10 * 1000 * 1000];   // ❌ ~40 MB on the stack → crash
}
```

Large data goes on the heap:

```c
int *huge = malloc(10000000 * sizeof *huge);
if (!huge) { /* handle */ }
```

### 8.4 The heap contract

```c
void *malloc(size_t size);                 // uninitialized
void *calloc(size_t count, size_t size);   // zeroed, checks count*size overflow
void *realloc(void *ptr, size_t new_size); // resize; MAY MOVE the block
void  free(void *ptr);                     // release; free(NULL) is safe
```

Five rules:

1. **Every allocation gets exactly one `free`.**
2. **Check for `NULL`** — allocation can fail.
3. **After `free`, the pointer is dangling.** Don't read, write, or free it again.
4. **`realloc` may return a different address.** All old pointers into the block
   are invalid.
5. **Nothing is automatic.** Leaving scope doesn't free heap memory.

Correct `realloc` usage — this is the one people get wrong:

```c
/* ❌ if realloc fails, you overwrote your only pointer and leaked the block */
buf = realloc(buf, new_size);

/* ✅ use a temporary */
int *tmp = realloc(buf, new_size * sizeof *tmp);
if (!tmp) {
    free(buf);              // or keep using the old buf and report failure
    return -1;
}
buf = tmp;
```

The `sizeof *tmp` idiom (rather than `sizeof(int)`) means the size automatically
stays correct if you change the type. Use it everywhere.

### 8.5 The four heap bugs, and what they look like

```c
/* 1. LEAK — allocated, never freed */
void leak(void) {
    char *p = malloc(100);
    return;                     // memory unreachable, never reclaimed
}

/* 2. USE AFTER FREE */
char *p = malloc(100);
free(p);
p[0] = 'x';                     // ❌ writing to freed memory

/* 3. DOUBLE FREE */
free(p);
free(p);                        // ❌ corrupts the allocator's bookkeeping

/* 4. BUFFER OVERFLOW */
char *p = malloc(10);
strcpy(p, "this string is far too long");   // ❌ writes past the block
```

A habit that prevents #2 and #3:

```c
free(p);
p = NULL;        // now accidental reuse is a clean NULL deref, and free(NULL) is a no-op
```

Or make it a macro:

```c
#define FREE(p) do { free(p); (p) = NULL; } while (0)
```

All four are caught instantly by AddressSanitizer. See [section 17](#debugging).

### 8.6 `const`, `volatile`, `restrict`

```c
const int limit = 100;           // "I promise not to modify this"
```

`const` is for the *compiler and the reader*, and it's the cheapest documentation
you can write. Mark pointer parameters `const` whenever the function only reads:

```c
size_t count_char(const char *s, char target);   // clearly read-only
void   to_upper(char *s);                        // clearly modifies
```

```c
volatile int flag;    // "may change outside this program's control —
                      //  reload it from memory every time, don't optimize away"
```

Use for memory-mapped hardware registers and variables modified by signal
handlers. **`volatile` is not a threading tool** — it provides no atomicity or
memory ordering. Use `<stdatomic.h>` for that.

```c
void add(int n, const int *restrict a, const int *restrict b, int *restrict out);
```

`restrict` (C99) promises the pointers don't overlap, letting the compiler
vectorize. If you lie, you get UB. Use rarely and deliberately.

---

<a id="pointers"></a>
## 9. Pointers, For Real This Time

A pointer is a variable whose value is a memory address. That's all.

```c
#include <stdio.h>

int main(void) {
    int  x = 42;
    int *p = &x;        // & = "address of"

    printf("value of x   : %d\n", x);
    printf("address of x : %p\n", (void *)&x);
    printf("value of p   : %p\n", (void *)p);      // same address
    printf("value at p   : %d\n", *p);             // * = "dereference"

    *p = 99;                                       // write through the pointer
    printf("x is now     : %d\n", x);              // 99

    return 0;
}
```

Cast to `void *` when printing with `%p` — that's what the standard requires.

### 9.1 Why pointers exist: output parameters

C passes everything **by value**. A function gets a copy.

```c
void broken_double(int n) { n *= 2; }         // modifies a copy, caller unaffected
void works_double(int *n) { *n *= 2; }        // modifies the caller's variable

int x = 5;
broken_double(x);   // x still 5
works_double(&x);   // x now 10
```

This is the *entire reason* `scanf` needs `&`:

```c
int age;
scanf("%d", &age);    // scanf needs the ADDRESS to write into
```

Returning multiple values:

```c
#include <stdio.h>

/* Returns 0 on success, -1 on divide by zero. */
int divmod(int a, int b, int *quotient, int *remainder) {
    if (b == 0) return -1;
    if (quotient)  *quotient  = a / b;      // NULL check = caller may opt out
    if (remainder) *remainder = a % b;
    return 0;
}

int main(void) {
    int q, r;
    if (divmod(17, 5, &q, &r) == 0)
        printf("17 / 5 = %d remainder %d\n", q, r);   // 3 remainder 2

    int only_q;
    divmod(17, 5, &only_q, NULL);            // don't care about remainder
    printf("q = %d\n", only_q);
    return 0;
}
```

### 9.2 Pointer arithmetic

Pointer arithmetic is scaled by the pointed-to type's size.

```c
#include <stdio.h>

int main(void) {
    int arr[5] = {10, 20, 30, 40, 50};
    int *p = arr;

    printf("%d\n", *p);         // 10
    printf("%d\n", *(p + 1));   // 20  — advances by sizeof(int) bytes, not 1
    printf("%d\n", *(p + 3));   // 40

    p++;                        // now points at arr[1]
    printf("%d\n", *p);         // 20

    int *end = arr + 5;         // one-past-the-end: legal to form, illegal to deref
    printf("elements: %td\n", end - arr);     // 5, use %td for ptrdiff_t

    /* Idiomatic pointer walk */
    for (int *it = arr; it != end; ++it)
        printf("%d ", *it);
    putchar('\n');

    return 0;
}
```

Rules:
- `p + n` advances `n * sizeof(*p)` bytes.
- `p - q` (same array) gives the element count, type `ptrdiff_t`, print with `%td`.
- One-past-the-end may be **formed and compared** but never **dereferenced**.
- Anything beyond one-past-the-end is undefined behavior.
- `a[i]` is *defined as* `*(a + i)`. Which means `i[a]` also compiles. Never do that.

### 9.3 `void *` — the generic pointer

```c
void *anything;         // can hold any object pointer; cannot be dereferenced
```

```c
int x = 5;
void *v = &x;
/* printf("%d", *v);  ❌ error: dereferencing void pointer — no size known */
printf("%d\n", *(int *)v);   // ✅ cast back to the real type first
```

`malloc` returns `void *`, which is why it works for every type. No cast needed
in C (unlike C++):

```c
int *p = malloc(10 * sizeof *p);       // ✅ implicit void* → int* is fine in C
```

### 9.4 Pointer to pointer

Needed when a function must change the caller's *pointer*, not just what it
points to.

```c
#include <stdio.h>
#include <stdlib.h>

/* Grows the array; must update the caller's pointer, hence int** */
int grow(int **arr, size_t *cap) {
    size_t new_cap = (*cap == 0) ? 4 : *cap * 2;
    int *tmp = realloc(*arr, new_cap * sizeof *tmp);
    if (!tmp) return -1;
    *arr = tmp;
    *cap = new_cap;
    return 0;
}

int main(void) {
    int *data = NULL;
    size_t cap = 0;

    if (grow(&data, &cap) != 0) return 1;
    data[0] = 7;
    printf("cap=%zu data[0]=%d\n", cap, data[0]);

    free(data);
    return 0;
}
```

### 9.5 Function pointers

```c
#include <stdio.h>

static int add(int a, int b) { return a + b; }
static int sub(int a, int b) { return a - b; }
static int mul(int a, int b) { return a * b; }

int main(void) {
    int (*op)(int, int);        // pointer to function (int,int)->int

    op = add;                   // function name decays to its address
    printf("%d\n", op(4, 3));   // 7  — no need to write (*op)(4,3)

    /* A dispatch table — replaces a long switch */
    struct { char symbol; int (*fn)(int, int); } table[] = {
        {'+', add},
        {'-', sub},
        {'*', mul},
    };

    for (size_t i = 0; i < sizeof table / sizeof table[0]; i++)
        printf("4 %c 3 = %d\n", table[i].symbol, table[i].fn(4, 3));

    return 0;
}
```

### 9.6 Pointer bug checklist

| Bug | Symptom | Guard |
|---|---|---|
| Dereferencing `NULL` | segfault | check before use |
| Uninitialized pointer | random crash | initialize to `NULL` |
| Dangling (freed or out-of-scope) | works then breaks | set to `NULL` after free; never return `&local` |
| Off-by-one | corrupt neighbors, silent | `<` not `<=` in loops |
| Wrong type cast | garbage values | avoid casts; trust types |
| Leak | growing memory use | one `free` per allocation |
| Losing the only pointer | leak | keep the original until success |

---

<a id="arrays"></a>
## 10. Arrays, Decay, and Multidimensional Data

### 10.1 Arrays are not pointers (but they decay into them)

```c
int arr[5];
```

`arr` is an object of type `int[5]`, occupying 20 bytes. But in **almost every
expression** it converts ("decays") to `int *` pointing at `arr[0]`.

The three places decay does **not** happen:

```c
sizeof arr        // 20 — the array's real size
&arr              // type int(*)[5], not int**
char s[] = "hi";  // initialization from a string literal
```

This is why `sizeof` works locally and fails across function calls:

```c
#include <stdio.h>

void takes_array(int arr[10]) {      // ⚠️ the "[10]" is a LIE — it's really int*
    printf("inside : %zu\n", sizeof arr);   // 8 — size of a pointer!
}

int main(void) {
    int arr[10];
    printf("outside: %zu\n", sizeof arr);   // 40 — the real array
    takes_array(arr);
    return 0;
}
```

> 🔑 **Always pass the length alongside the array.** There is no other way for the
> function to know it.

```c
void process(const int *arr, size_t n);
process(arr, sizeof arr / sizeof arr[0]);
```

The `sizeof a / sizeof a[0]` idiom only works where the array's real type is
visible — i.e. the same scope it was declared in.

### 10.2 Array initialization

```c
int a[5] = {1, 2, 3, 4, 5};
int b[5] = {1, 2};              // rest zero-filled → {1,2,0,0,0}
int c[5] = {0};                 // all zeros — common idiom
int d[]  = {1, 2, 3};           // size inferred: 3
int e[5] = {[4] = 9, [0] = 1};  // designated initializers (C99) → {1,0,0,0,9}

char s1[] = "hello";            // 6 bytes: 'h','e','l','l','o','\0'
char s2[6] = "hello";           // same
char s3[5] = "hello";           // ⚠️ no room for '\0' — not a valid string!
```

### 10.3 2D arrays

```c
#include <stdio.h>

int main(void) {
    int grid[3][4] = {
        {1,  2,  3,  4},
        {5,  6,  7,  8},
        {9, 10, 11, 12},
    };

    for (size_t r = 0; r < 3; r++) {
        for (size_t c = 0; c < 4; c++)
            printf("%3d", grid[r][c]);
        putchar('\n');
    }

    printf("sizeof grid    = %zu\n", sizeof grid);       // 48
    printf("sizeof grid[0] = %zu\n", sizeof grid[0]);    // 16 (one row)

    return 0;
}
```

Memory layout is **row-major** — contiguous, row after row:

```
grid[0][0..3]  grid[1][0..3]  grid[2][0..3]
[1 2 3 4]      [5 6 7 8]      [9 10 11 12]
```

`grid[r][c]` is at offset `r * 4 + c`. Iterating rows-then-columns is
cache-friendly; columns-then-rows is not. That's a real, measurable difference on
large arrays.

### 10.4 Passing 2D arrays

The column count must be known:

```c
void print_grid(int rows, int grid[][4]) {      /* ✅ columns required */
    for (int r = 0; r < rows; r++)
        for (int c = 0; c < 4; c++)
            printf("%d ", grid[r][c]);
}

/* C99 variably-modified type — cleanest when supported */
void print_vla(int rows, int cols, int grid[rows][cols]) {
    for (int r = 0; r < rows; r++)
        for (int c = 0; c < cols; c++)
            printf("%d ", grid[r][c]);
}
```

MSVC does not support VLA parameters. For portable code, flatten:

```c
void print_flat(const int *grid, int rows, int cols) {
    for (int r = 0; r < rows; r++)
        for (int c = 0; c < cols; c++)
            printf("%d ", grid[r * cols + c]);
}
```

Flattening (`index = row * cols + col`) is what serious C code usually does. It's
portable, one allocation, and cache-friendly.

### 10.5 Dynamic 2D allocation

**Option A — single flat block (recommended):**

```c
int rows = 3, cols = 4;
int *grid = malloc((size_t)rows * cols * sizeof *grid);
if (!grid) return 1;

grid[1 * cols + 2] = 7;        // grid[1][2]

free(grid);                    // one free
```

**Option B — array of row pointers (allows ragged rows):**

```c
int **grid = malloc(rows * sizeof *grid);
if (!grid) return 1;
for (int r = 0; r < rows; r++) {
    grid[r] = malloc(cols * sizeof *grid[r]);
    if (!grid[r]) { /* free earlier rows, then grid */ }
}

grid[1][2] = 7;                // natural syntax

for (int r = 0; r < rows; r++) free(grid[r]);
free(grid);
```

Option A is faster (one allocation, contiguous memory, fewer cache misses).
Option B gives nicer syntax and variable-length rows. Know both; default to A.

### 10.6 Variable-length arrays (VLAs)

```c
void demo(int n) {
    int arr[n];                 // C99 VLA — size decided at runtime, on the STACK
    /* ... */
}
```

Caveats: no bounds checking, blows the stack if `n` is large, made *optional* in
C11, unsupported by MSVC. Prefer `malloc` for runtime sizes.

---

<a id="structs"></a>
## 11. Structs, Unions, Enums, Padding, and Alignment

### 11.1 Structs

```c
#include <stdio.h>
#include <string.h>

struct Point { int x, y; };              // "struct Point" is the type name

typedef struct {                          // typedef → just "Vec3"
    double x, y, z;
} Vec3;

typedef struct Employee {                 // both names available
    char   name[64];
    int    id;
    double salary;
} Employee;

int main(void) {
    struct Point p = {3, 4};
    Vec3 v = {1.0, 2.0, 3.0};

    /* Designated initializers (C99) — order-independent, self-documenting */
    Employee e = {
        .id     = 101,
        .salary = 55000.0,
        .name   = "Devashish",
    };

    printf("%s (#%d) earns %.2f\n", e.name, e.id, e.salary);
    printf("p = (%d, %d)\n", p.x, p.y);
    printf("v.z = %.1f\n", v.z);

    /* Struct assignment copies all members (shallow copy) */
    Employee copy = e;
    strcpy(copy.name, "Someone Else");
    printf("original still: %s\n", e.name);

    return 0;
}
```

Access: `.` for a struct value, `->` for a pointer to struct.

```c
Employee *ep = &e;
printf("%d\n", ep->id);        // shorthand for (*ep).id
```

### 11.2 Structs and functions

```c
/* By value — copies the whole struct. Fine for small ones. */
double magnitude(Vec3 v);

/* By const pointer — no copy, clearly read-only. Prefer for large structs. */
double magnitude_fast(const Vec3 *v);

/* By pointer — can modify */
void normalize(Vec3 *v);
```

Rule of thumb: pass by value up to ~2–3 machine words; by `const *` beyond that.

### 11.3 Padding and alignment — why `sizeof` surprises you

```c
#include <stdio.h>
#include <stddef.h>

struct Bad  { char a; int b; char c; };    // likely 12 bytes
struct Good { int b; char a; char c; };    // likely 8 bytes

int main(void) {
    printf("Bad  = %zu\n", sizeof(struct Bad));
    printf("Good = %zu\n", sizeof(struct Good));

    printf("offsetof(Bad, a) = %zu\n", offsetof(struct Bad, a));
    printf("offsetof(Bad, b) = %zu\n", offsetof(struct Bad, b));
    printf("offsetof(Bad, c) = %zu\n", offsetof(struct Bad, c));
    return 0;
}
```

Why: the CPU wants an `int` at an address divisible by 4. The compiler inserts
invisible padding.

```
struct Bad:
  offset 0: char a       (1 byte)
  offset 1: ---padding--- (3 bytes)   ← so b lands on a 4-byte boundary
  offset 4: int  b       (4 bytes)
  offset 8: char c       (1 byte)
  offset 9: ---padding--- (3 bytes)   ← so the struct's size is a multiple of 4
  total: 12

struct Good:
  offset 0: int  b       (4 bytes)
  offset 4: char a       (1 byte)
  offset 5: char c       (1 byte)
  offset 6: ---padding--- (2 bytes)
  total: 8
```

> 🔑 **Declare members largest-to-smallest** to minimize padding. On big arrays of
> structs this is real memory and real cache performance.

Consequences of padding:

```c
/* ❌ never memcmp structs — padding bytes are indeterminate */
if (memcmp(&a, &b, sizeof a) == 0) { }

/* ✅ compare fields */
if (a.x == b.x && a.y == b.y) { }

/* ❌ never fwrite a struct and expect another machine to read it —
      padding, endianness, and type sizes all differ. Serialize field by field. */
```

C11 gives you introspection and control:

```c
#include <stdalign.h>
printf("%zu\n", alignof(double));       // typically 8
alignas(16) char buffer[64];            // 16-byte aligned (for SIMD, etc.)
```

### 11.4 Nested and self-referential structs

```c
typedef struct {
    char street[64];
    char city[32];
} Address;

typedef struct {
    char    name[64];
    Address home;                 // nested by value
} Person;

Person p = { .name = "Dev", .home = { .city = "Pune" } };
printf("%s\n", p.home.city);
```

A struct can't contain itself, but it can contain a **pointer** to itself — the
foundation of every linked data structure:

```c
typedef struct Node {
    int          value;
    struct Node *next;        // must use the struct tag; the typedef isn't ready yet
} Node;
```

### 11.5 Unions — one memory location, multiple interpretations

```c
#include <stdio.h>
#include <stdint.h>

union Value {
    int32_t i;
    float   f;
    char    bytes[4];
};

int main(void) {
    union Value v;
    v.i = 1;

    printf("sizeof = %zu\n", sizeof v);          // 4 — size of the LARGEST member
    printf("as int   : %d\n", v.i);
    printf("bytes    : %d %d %d %d\n",
           v.bytes[0], v.bytes[1], v.bytes[2], v.bytes[3]);
    return 0;
}
```

Only **one member is valid at a time** — writing `f` invalidates `i`. Reading a
member you didn't write is type punning; C permits it through unions but the
result is implementation-defined.

The standard use is a **tagged union**:

```c
#include <stdio.h>

typedef enum { TYPE_INT, TYPE_FLOAT, TYPE_STRING } ValueType;

typedef struct {
    ValueType type;                 // the tag says which member is live
    union {
        int    as_int;
        double as_double;
        char  *as_string;
    } data;
} Variant;

void print_variant(const Variant *v) {
    switch (v->type) {
        case TYPE_INT:    printf("int %d\n", v->data.as_int);       break;
        case TYPE_FLOAT:  printf("float %.2f\n", v->data.as_double); break;
        case TYPE_STRING: printf("string %s\n", v->data.as_string); break;
    }
}

int main(void) {
    Variant a = { .type = TYPE_INT,    .data.as_int = 42 };
    Variant b = { .type = TYPE_STRING, .data.as_string = "hi" };
    print_variant(&a);
    print_variant(&b);
    return 0;
}
```

### 11.6 Enums

```c
enum Color { RED, GREEN, BLUE };            // 0, 1, 2
enum Status { OK = 200, NOT_FOUND = 404, ERROR = 500 };
enum Flags { F_READ = 1, F_WRITE = 2, F_EXEC = 4 };   // powers of 2 for bit flags

typedef enum { MON, TUE, WED, THU, FRI, SAT, SUN, DAY_COUNT } Day;
```

The trailing `DAY_COUNT` trick gives you the count for free — it's `7`.

Enums are `int`-compatible, so they're not type-safe, but they're better than
magic numbers: named, visible in the debugger, and `switch` coverage can be
checked by the compiler (`-Wswitch`).

### 11.7 Bit fields

```c
#include <stdio.h>

struct Flags {
    unsigned int is_active   : 1;      // 1 bit
    unsigned int permissions : 3;      // 3 bits (0–7)
    unsigned int priority    : 4;      // 4 bits (0–15)
};

int main(void) {
    struct Flags f = { .is_active = 1, .permissions = 5, .priority = 12 };
    printf("sizeof = %zu\n", sizeof f);    // typically 4, not 3 bits worth
    printf("perm = %u\n", f.permissions);
    return 0;
}
```

Compact, but layout and packing order are implementation-defined. Don't use bit
fields for wire formats — use explicit shifts and masks ([section 15](#bits)).

### 11.8 Flexible array member (C99)

A struct whose last member is an unsized array, so the header and data live in
one allocation:

```c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    size_t len;
    char   data[];        // must be LAST, and the struct needs another member
} Buffer;

Buffer *buffer_create(const char *text) {
    size_t len = strlen(text);
    Buffer *b = malloc(sizeof *b + len + 1);   // header + payload, one malloc
    if (!b) return NULL;
    b->len = len;
    memcpy(b->data, text, len + 1);
    return b;
}

int main(void) {
    Buffer *b = buffer_create("hello");
    if (!b) return 1;
    printf("%zu: %s\n", b->len, b->data);
    free(b);                                    // one free
    return 0;
}
```

---

<a id="strings"></a>
## 12. Strings and Ownership

C has no string type. A "string" is a `char` array whose end is marked by `'\0'`.
Every string bug traces back to one of: missing terminator, insufficient space, or
unclear ownership.

### 12.1 The five ways to hold a string

```c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(void) {
    /* 1. String literal — read-only, static duration */
    const char *lit = "hello";
    /* lit[0] = 'H';  ❌ UB — literals may live in read-only memory */

    /* 2. Array initialized from a literal — a modifiable COPY */
    char arr[] = "hello";        // 6 bytes on the stack
    arr[0] = 'H';                // ✅ fine
    printf("%s\n", arr);

    /* 3. Fixed buffer, filled later */
    char buf[64];
    snprintf(buf, sizeof buf, "%s world", arr);
    printf("%s\n", buf);

    /* 4. Heap-allocated, you own it */
    char *heap = malloc(64);
    if (heap) {
        strcpy(heap, "heap string");
        printf("%s\n", heap);
        free(heap);
    }

    /* 5. Heap copy of an existing string */
    size_t n = strlen(arr) + 1;
    char *copy = malloc(n);
    if (copy) {
        memcpy(copy, arr, n);
        printf("%s\n", copy);
        free(copy);
    }
    return 0;
}
```

> 🔑 **`char *s = "literal";` gives a pointer to read-only memory. Writing through
> it is undefined behavior.** Declare literals as `const char *` so the compiler
> catches the mistake.

### 12.2 Safe input

```c
#include <stdio.h>
#include <string.h>

int main(void) {
    char name[64];

    printf("Name: ");
    if (!fgets(name, sizeof name, stdin)) {
        fprintf(stderr, "read failed\n");
        return 1;
    }
    name[strcspn(name, "\n")] = '\0';       // strip the trailing newline

    printf("Hello, %s! (%zu chars)\n", name, strlen(name));
    return 0;
}
```

`name[strcspn(name, "\n")] = '\0';` is the canonical newline-strip. `strcspn`
returns the index of the first `\n`, or the string length if there is none —
either way the assignment is correct.

### 12.3 The dangerous functions and their replacements

| ❌ Avoid | Why | ✅ Use |
|---|---|---|
| `gets(s)` | Cannot bound input. **Removed in C11** | `fgets(s, sizeof s, stdin)` |
| `scanf("%s", b)` | Unbounded | `scanf("%63s", b)` or `fgets` |
| `strcpy(d, s)` | Unbounded | `snprintf(d, sizeof d, "%s", s)` |
| `strcat(d, s)` | Unbounded | `snprintf` building the whole string |
| `sprintf(b, ...)` | Unbounded | `snprintf(b, sizeof b, ...)` |
| `strncpy(d, s, n)` | May not terminate | see below |
| `atoi(s)` | No error reporting | `strtol` |

`strncpy` is a trap — it's a *fixed-width field* function, not a safe copy:

```c
char dst[6];
strncpy(dst, "hello world", sizeof dst);
/* dst is now "hello" with NO terminator — printf("%s") runs off the end */

/* If you must use it: */
strncpy(dst, src, sizeof dst - 1);
dst[sizeof dst - 1] = '\0';

/* Better: */
snprintf(dst, sizeof dst, "%s", src);      // always terminates, always bounded
```

`snprintf` returns the length it *wanted* to write, so you can detect truncation:

```c
int written = snprintf(buf, sizeof buf, "%s/%s", dir, file);
if (written < 0 || (size_t)written >= sizeof buf) {
    fprintf(stderr, "path truncated\n");
    return -1;
}
```

### 12.4 Ownership: the question you must always answer

For every `char *` crossing a function boundary, decide and **document**:

```c
/* Caller owns the buffer; we only read. */
size_t count_words(const char *text);

/* Caller owns the buffer; we write into it, bounded by cap. */
int format_name(char *out, size_t cap, const char *first, const char *last);

/* WE allocate; CALLER must free(). Document this loudly. */
char *read_whole_file(const char *path);

/* We take ownership and will free it. */
void queue_take_message(Queue *q, char *owned_message);
```

Conventions that scale:
- Return `const char *` for borrowed strings, `char *` for owned ones.
- Name allocating functions `*_create`, `*_dup`, `*_read`, and always pair with a
  matching `*_destroy`/`free`.
- Prefer caller-provided buffers (`out`, `cap`) — no allocation, no ownership
  question at all.

### 12.5 Writing your own string helpers

```c
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

/* Heap copy. Returns NULL on failure. Caller must free. */
char *str_dup(const char *s) {
    size_t n = strlen(s) + 1;
    char *copy = malloc(n);
    if (copy) memcpy(copy, s, n);
    return copy;
}

/* Uppercase in place. */
void str_upper(char *s) {
    for (; *s; s++)
        *s = (char)toupper((unsigned char)*s);
}

/* Trim leading and trailing whitespace in place; returns s. */
char *str_trim(char *s) {
    char *start = s;
    while (*start && isspace((unsigned char)*start)) start++;

    if (start != s) memmove(s, start, strlen(start) + 1);

    char *end = s + strlen(s);
    while (end > s && isspace((unsigned char)end[-1])) end--;
    *end = '\0';
    return s;
}

/* Does s begin with prefix? */
int str_starts_with(const char *s, const char *prefix) {
    return strncmp(s, prefix, strlen(prefix)) == 0;
}
```

Note `(unsigned char)` casts on `ctype.h` calls. Passing a negative `char` to
`isspace`/`toupper` is undefined behavior, and plain `char` is signed on most
platforms. This is a real bug with non-ASCII input.

### 12.6 Reading a whole file

```c
#include <stdio.h>
#include <stdlib.h>

/* Reads the entire file into a NUL-terminated heap buffer.
   Returns NULL on failure. Caller must free the result.
   If out_len is non-NULL, receives the byte count. */
char *read_whole_file(const char *path, size_t *out_len) {
    FILE *fp = fopen(path, "rb");
    if (!fp) return NULL;

    if (fseek(fp, 0, SEEK_END) != 0) { fclose(fp); return NULL; }
    long size = ftell(fp);
    if (size < 0) { fclose(fp); return NULL; }
    rewind(fp);

    char *buf = malloc((size_t)size + 1);
    if (!buf) { fclose(fp); return NULL; }

    size_t read = fread(buf, 1, (size_t)size, fp);
    fclose(fp);

    buf[read] = '\0';
    if (out_len) *out_len = read;
    return buf;
}

int main(void) {
    size_t len;
    char *text = read_whole_file("input.txt", &len);
    if (!text) {
        perror("read_whole_file");
        return 1;
    }
    printf("read %zu bytes\n", len);
    free(text);
    return 0;
}
```

Every failure path closes the file and frees what it allocated. That discipline is
the whole game in C.

---

<a id="integers"></a>
## 13. Integer Conversions and the Promotion Traps

This section prevents a class of bug that is invisible until it isn't.

### 13.1 Integer promotion

Anything smaller than `int` is promoted to `int` before arithmetic.

```c
#include <stdio.h>

int main(void) {
    char a = 100, b = 100;
    char c = a + b;                   // a+b computed as int (200), then truncated
    printf("%d\n", c);                // -56 on signed-char platforms

    int correct = a + b;
    printf("%d\n", correct);          // 200
    return 0;
}
```

### 13.2 The signed/unsigned trap

When you mix signed and unsigned of the same rank, **the signed value converts to
unsigned.**

```c
#include <stdio.h>

int main(void) {
    unsigned int u = 1;
    int          s = -2;

    if (u + s > 0)
        printf("positive\n");         // ← this prints!
    else
        printf("negative\n");

    /* -2 becomes 4294967294; +1 = 4294967295, which is > 0 */
    printf("%u\n", u + s);
    return 0;
}
```

The most common real-world instance:

```c
#include <stdio.h>
#include <string.h>

int main(void) {
    const char *s = "hi";

    /* ❌ strlen returns size_t (unsigned). If it's 0, 0-1 wraps to a huge number. */
    for (int i = 0; i < strlen(s) - 1; i++) { }

    /* ❌ This never runs: sizeof is unsigned, so -1 converts to SIZE_MAX */
    if (sizeof(int) > -1) printf("never\n");

    /* ✅ Compare like with like */
    size_t len = strlen(s);
    for (size_t i = 0; i + 1 < len; i++)
        printf("%c", s[i]);
    putchar('\n');
    return 0;
}
```

> 🔑 **Never write `unsigned_value - something` without checking it can't go
> negative.** Unsigned arithmetic wraps silently to a gigantic number. Rewrite
> `i < len - 1` as `i + 1 < len`.

Enable `-Wsign-compare` (included in `-Wextra`) to catch these.

### 13.3 Signed overflow is undefined; unsigned wraps

```c
#include <stdio.h>
#include <limits.h>

int main(void) {
    int          s = INT_MAX;
    unsigned int u = UINT_MAX;

    /* printf("%d\n", s + 1);  ❌ UNDEFINED BEHAVIOR — anything may happen */
    printf("%u\n", u + 1);     // ✅ defined: wraps to 0
    return 0;
}
```

Because signed overflow is UB, the optimizer assumes it never happens. This is
why `-O2` can delete your overflow check:

```c
/* ❌ the compiler may assume a+b doesn't overflow and remove this entirely */
if (a + b < a) { /* overflow! */ }

/* ✅ check BEFORE the operation */
#include <limits.h>
if (b > 0 && a > INT_MAX - b) { /* would overflow */ }
if (b < 0 && a < INT_MIN - b) { /* would underflow */ }
```

C23 adds checked arithmetic in `<stdckdint.h>`:

```c
#include <stdckdint.h>
int result;
if (ckd_add(&result, a, b)) { /* overflow occurred */ }
```

GCC/Clang have had builtins for years:

```c
int result;
if (__builtin_add_overflow(a, b, &result)) { /* overflow */ }
```

### 13.4 Allocation size overflow — a real security bug

```c
/* ❌ count * size can overflow, allocating far less than you think,
      then your writes run off the end */
void *buf = malloc(count * sizeof(Item));

/* ✅ check first */
if (count > SIZE_MAX / sizeof(Item)) return NULL;
void *buf = malloc(count * sizeof(Item));

/* ✅ or let calloc do the check for you */
void *buf = calloc(count, sizeof(Item));
```

### 13.5 Truncation and sign extension

```c
#include <stdio.h>
#include <stdint.h>

int main(void) {
    int32_t big = 300;
    int8_t  small = (int8_t)big;        // truncated: 300 & 0xFF = 44
    printf("%d\n", small);              // 44

    int8_t   neg = -1;
    uint32_t wide = (uint32_t)neg;      // sign-extended to 0xFFFFFFFF
    printf("%u\n", wide);               // 4294967295
    return 0;
}
```

### 13.6 Division and modulo

```c
printf("%d\n",  7 /  2);        //  3   integer division truncates toward zero
printf("%d\n", -7 /  2);        // -3   (C99 guarantees truncation)
printf("%d\n", -7 %  2);        // -1   sign follows the DIVIDEND
printf("%f\n", 7 / 2.0);        // 3.5  one operand is double → double division

int a = 7, b = 2;
printf("%f\n", (double)a / b);  // 3.5  cast BEFORE dividing
printf("%f\n", (double)(a / b));// 3.0  ❌ too late, already truncated
```

Negative modulo bites when implementing wrap-around:

```c
/* ❌ -1 % 5 is -1, not 4 */
int index = (i - 1) % n;

/* ✅ */
int index = ((i - 1) % n + n) % n;
```

### 13.7 Which integer type to use

| Need | Use |
|---|---|
| General counting, loop index over `int` range | `int` |
| Sizes, array lengths, `sizeof` results | `size_t` |
| Pointer differences | `ptrdiff_t` |
| Exact width (file formats, protocols, hardware) | `int32_t`, `uint8_t`, … |
| Maximum range | `long long` / `int64_t` |
| Boolean | `bool` from `<stdbool.h>` |
| A single byte of raw data | `unsigned char` |

Print them correctly:

```c
size_t    n = 10;   printf("%zu\n", n);
ptrdiff_t d = 3;    printf("%td\n", d);
int64_t   v = 1;    printf("%" PRId64 "\n", v);   // needs <inttypes.h>
long long L = 1;    printf("%lld\n", L);
```

Passing the wrong specifier to `printf` is undefined behavior, not a rounding
issue. `-Wformat` (in `-Wall`) catches most of it.

---

<a id="floats"></a>
## 14. Floating Point: What the Hardware Actually Does

### 14.1 Why 0.1 + 0.2 != 0.3

```c
#include <stdio.h>

int main(void) {
    double a = 0.1, b = 0.2;
    printf("%.20f\n", a + b);          // 0.30000000000000004441
    printf("%.20f\n", 0.3);            // 0.29999999999999998890
    printf("%s\n", (a + b == 0.3) ? "equal" : "NOT equal");   // NOT equal
    return 0;
}
```

`double` stores numbers as `sign × mantissa × 2^exponent`. Only fractions whose
denominator is a power of two are exact. `0.1` in binary is `0.0001100110011...`
repeating forever — it gets rounded, and the error accumulates.

### 14.2 Never compare floats with `==`

```c
#include <math.h>
#include <float.h>
#include <stdbool.h>

/* Absolute tolerance — fine when you know the magnitude range */
bool nearly_equal_abs(double a, double b, double eps) {
    return fabs(a - b) < eps;
}

/* Relative tolerance — better across magnitudes */
bool nearly_equal(double a, double b) {
    double diff = fabs(a - b);
    double scale = fmax(fabs(a), fabs(b));
    if (scale < DBL_MIN) return diff < DBL_EPSILON;   // both near zero
    return diff / scale < 1e-9;
}
```

The exception: comparing against a value you *assigned* literally
(`if (x == 0.0)`) is fine, since no arithmetic introduced error. Comparing a
*computed* value is not.

### 14.3 Types

| Type | Bits | Decimal digits | Range | `printf` |
|---|---|---|---|---|
| `float` | 32 | ~7 | ±3.4e38 | `%f` (promoted to double) |
| `double` | 64 | ~15 | ±1.8e308 | `%f`, `%e`, `%g` |
| `long double` | 80/128 | 18+ | platform | `%Lf` |

**Default to `double`.** `float` saves memory in large arrays but its 7 digits of
precision run out fast. Note that `float` arguments to variadic functions like
`printf` are automatically promoted to `double`, which is why `%f` works for both.

### 14.4 Special values

```c
#include <stdio.h>
#include <math.h>

int main(void) {
    double inf = 1.0 / 0.0;         // +infinity (or use INFINITY)
    double nan = 0.0 / 0.0;         // Not-a-Number (or NAN)

    printf("%f %f\n", inf, nan);
    printf("isinf: %d, isnan: %d\n", isinf(inf), isnan(nan));

    printf("nan == nan : %d\n", nan == nan);   // 0 — NaN compares unequal to itself!
    printf("isnan(nan) : %d\n", isnan(nan));   // 1 — the correct test

    /* Both zeros exist and compare equal */
    printf("%d\n", 0.0 == -0.0);               // 1
    return 0;
}
```

`x != x` being true is the historical NaN test. Use `isnan(x)` for clarity.

### 14.5 Accumulation error

```c
#include <stdio.h>

int main(void) {
    double sum = 0.0;
    for (int i = 0; i < 10; i++) sum += 0.1;
    printf("%.20f\n", sum);        // 0.99999999999999988898, not 1.0
    return 0;
}
```

Mitigations:
- **Count with integers, convert once**: `sum = count * 0.1;`
- **Use fixed-point for money**: store cents as `long long`. Never use `double`
  for currency.
- **Kahan summation** for long float sums:

```c
double kahan_sum(const double *a, size_t n) {
    double sum = 0.0, comp = 0.0;
    for (size_t i = 0; i < n; i++) {
        double y = a[i] - comp;
        double t = sum + y;
        comp = (t - sum) - y;        // recover the lost low-order bits
        sum = t;
    }
    return sum;
}
```

### 14.6 Practical rules

1. Use `double` unless you have a measured reason not to.
2. Never `==`. Use a tolerance.
3. Never store money in a float. Use integer minor units.
4. Trig functions take **radians**. `sin(90)` is not 1.
5. `abs()` is for `int`; `fabs()` is for `double`. Mixing them silently truncates.
6. Check domains: `sqrt(-1)`, `log(0)`, `asin(2)` produce NaN or -inf.
7. On Linux/macOS, `<math.h>` needs `-lm` at link time.

---

<a id="bits"></a>
## 15. Bit Manipulation

### 15.1 The operators

| Op | Name | Example | Result |
|---|---|---|---|
| `&` | AND | `0b1100 & 0b1010` | `0b1000` |
| `\|` | OR | `0b1100 \| 0b1010` | `0b1110` |
| `^` | XOR | `0b1100 ^ 0b1010` | `0b0110` |
| `~` | NOT | `~0b1100` | `0b...0011` (all bits flipped) |
| `<<` | left shift | `0b0011 << 2` | `0b1100` |
| `>>` | right shift | `0b1100 >> 2` | `0b0011` |

Don't confuse them with `&&`, `||`, `!` — those are logical, produce 0 or 1, and
short-circuit.

### 15.2 The five core idioms

```c
#define BIT(n)              (1u << (n))

#define SET_BIT(x, n)       ((x) |=  BIT(n))
#define CLEAR_BIT(x, n)     ((x) &= ~BIT(n))
#define TOGGLE_BIT(x, n)    ((x) ^=  BIT(n))
#define TEST_BIT(x, n)      (((x) >> (n)) & 1u)
#define ASSIGN_BIT(x, n, v) ((v) ? SET_BIT(x, n) : CLEAR_BIT(x, n))
```

```c
#include <stdio.h>

int main(void) {
    unsigned int flags = 0;

    SET_BIT(flags, 0);           // 0b0001
    SET_BIT(flags, 3);           // 0b1001
    printf("%u\n", flags);       // 9

    printf("%d\n", TEST_BIT(flags, 3));   // 1
    CLEAR_BIT(flags, 3);
    printf("%d\n", TEST_BIT(flags, 3));   // 0
    return 0;
}
```

### 15.3 Flag sets — the standard pattern

```c
#include <stdio.h>

typedef enum {
    PERM_READ    = 1u << 0,     // 1
    PERM_WRITE   = 1u << 1,     // 2
    PERM_EXECUTE = 1u << 2,     // 4
    PERM_DELETE  = 1u << 3,     // 8
} Permission;

int main(void) {
    unsigned perms = PERM_READ | PERM_WRITE;      // combine with OR

    if (perms & PERM_READ)   printf("can read\n");
    if (perms & PERM_DELETE) printf("can delete\n");   // not printed

    perms |=  PERM_EXECUTE;                       // add
    perms &= ~PERM_WRITE;                         // remove
    perms ^=  PERM_READ;                          // toggle

    /* test for ALL of a set */
    unsigned need = PERM_READ | PERM_EXECUTE;
    if ((perms & need) == need) printf("has both\n");

    /* test for ANY of a set */
    if (perms & need) printf("has at least one\n");

    return 0;
}
```

One `unsigned int` holds 32 independent booleans, and each test is a single CPU
instruction. This is how file permissions, event masks, and feature flags are
represented everywhere.

### 15.4 Shifting: the rules and the traps

```c
unsigned x = 1;
x << 3        // 8      — x * 2^3
x >> 1        // 0      — integer division, remainder lost

/* ⚠️ Shifting by >= the type's width is UNDEFINED BEHAVIOR */
unsigned y = 1u << 32;         // ❌ UB on 32-bit unsigned int
unsigned long long z = 1ull << 32;   // ✅ note the ull suffix

/* ⚠️ Right-shifting a NEGATIVE signed value is implementation-defined
      (in practice arithmetic shift, sign-extending) */
int neg = -8;
neg >> 1;     // usually -4, but not guaranteed portable

/* ⚠️ Left-shifting into the sign bit of a signed int is UB */
int bad = 1 << 31;            // ❌
unsigned ok = 1u << 31;       // ✅
```

**Always shift unsigned values, and never by more than `width - 1`.**

### 15.5 Useful bit tricks

```c
/* Test even/odd — faster and clearer than % 2 for unsigned */
if (x & 1u) /* odd */

/* Is x a power of two? (x > 0 required) */
int is_pow2(unsigned x) { return x && !(x & (x - 1)); }

/* Clear the lowest set bit */
x &= x - 1;

/* Isolate the lowest set bit */
unsigned lowest = x & (~x + 1);     // or x & -x for unsigned

/* Round up to the next power of two (32-bit) */
unsigned next_pow2(unsigned x) {
    if (x == 0) return 1;
    x--;
    x |= x >> 1; x |= x >> 2; x |= x >> 4; x |= x >> 8; x |= x >> 16;
    return x + 1;
}

/* Count set bits (Kernighan's algorithm — loops once per SET bit) */
int popcount(unsigned x) {
    int n = 0;
    while (x) { x &= x - 1; n++; }
    return n;
}

/* Swap without a temporary — a party trick, NOT faster, and broken if a == b */
a ^= b; b ^= a; a ^= b;

/* Extract a bit field: `len` bits starting at `pos` */
unsigned extract(unsigned x, int pos, int len) {
    return (x >> pos) & ((1u << len) - 1u);
}

/* Fast multiply/divide by powers of two — the compiler already does this
   for you; write the arithmetic and let it optimize. */
```

C23 standardizes many of these in `<stdbit.h>`: `stdc_popcount`,
`stdc_bit_width`, `stdc_leading_zeros`, `stdc_first_leading_one`, and friends.
GCC/Clang have had `__builtin_popcount`, `__builtin_clz`, `__builtin_ctz` for
years and they compile to single instructions.

### 15.6 Printing binary (there's no `%b` before C23)

```c
#include <stdio.h>

void print_bits(unsigned x, int width) {
    for (int i = width - 1; i >= 0; i--) {
        putchar((x >> i) & 1u ? '1' : '0');
        if (i % 4 == 0 && i) putchar('_');     // readability separator
    }
    putchar('\n');
}

int main(void) {
    print_bits(0xA5, 8);       // 1010_0101
    print_bits(12, 8);         // 0000_1100
    return 0;
}
```

C23 adds `%b` to `printf` and binary literals `0b1010`. GCC and Clang have
supported `0b` literals as an extension for a long time.

### 15.7 Endianness

Multi-byte values can be stored low-byte-first (little-endian: x86, ARM by
default) or high-byte-first (big-endian: network byte order, some embedded).

```c
#include <stdio.h>

int main(void) {
    unsigned int x = 0x12345678;
    unsigned char *b = (unsigned char *)&x;

    printf("%02X %02X %02X %02X\n", b[0], b[1], b[2], b[3]);
    // little-endian: 78 56 34 12
    // big-endian:    12 34 56 78

    printf("%s\n", b[0] == 0x78 ? "little-endian" : "big-endian");
    return 0;
}
```

This matters the moment you write binary files or network packets that another
machine reads. The portable fix is to serialize byte-by-byte with explicit shifts
rather than `fwrite`-ing a struct:

```c
/* Write a 32-bit value in big-endian order, portable everywhere */
void put_u32_be(unsigned char *p, uint32_t v) {
    p[0] = (unsigned char)(v >> 24);
    p[1] = (unsigned char)(v >> 16);
    p[2] = (unsigned char)(v >>  8);
    p[3] = (unsigned char)(v);
}

uint32_t get_u32_be(const unsigned char *p) {
    return ((uint32_t)p[0] << 24) | ((uint32_t)p[1] << 16)
         | ((uint32_t)p[2] <<  8) |  (uint32_t)p[3];
}
```

These functions work identically on every platform — no `#ifdef` needed. That's
the right way to handle binary formats.

---

<a id="ub"></a>
## 16. Undefined Behavior: The Catalogue

This is the single most important section in this document.

**Undefined behavior (UB)** means the standard places *no requirements at all* on
what happens. The program may crash, produce wrong output, work perfectly on your
machine and fail on another, or work in debug and fail in release. The compiler is
permitted to assume UB never occurs, and it uses that assumption to optimize —
which is how UB deletes your code.

### 16.1 The three categories people confuse

| Kind | Meaning | Example |
|---|---|---|
| **Undefined** | Anything may happen. No guarantees. | Signed overflow, out-of-bounds access |
| **Unspecified** | Several behaviors allowed, compiler needn't document which | Order of evaluation of function arguments |
| **Implementation-defined** | Compiler picks, and **must document** it | Whether `char` is signed; `sizeof(int)` |

Unspecified and implementation-defined are portability concerns. **Undefined is a
correctness catastrophe.**

### 16.2 How UB deletes your code

```c
int f(int a) {
    int b = a + 100;
    if (b < a) return -1;      // "overflow check"
    return b;
}
```

The compiler reasons: signed overflow is UB, so `a + 100` never overflows, so
`b > a` always, so `b < a` is always false, so **delete the check.** Compile with
`-O2` and inspect the assembly — the branch is gone. Your safety net evaporated
because it was built on UB.

```c
if (!p) return;      // null check
*p = 5;
/* ...vs... */
*p = 5;
if (!p) return;      // ❌ compiler deletes this: you already dereferenced p,
                     //    so p "cannot" be null
```

### 16.3 The catalogue

**Memory**

```c
int a[5];
a[5] = 1;                    // ❌ out of bounds (valid: 0–4)
a[-1] = 1;                   // ❌

int *p;
*p = 5;                      // ❌ uninitialized pointer

int *q = NULL;
*q = 5;                      // ❌ null dereference

free(p); *p = 1;             // ❌ use after free
free(p); free(p);            // ❌ double free
free(p + 1);                 // ❌ must free exactly what malloc returned

char *s = "hi"; s[0] = 'H';  // ❌ modifying a string literal

int *r = malloc(4);
printf("%d\n", *r);          // ❌ reading uninitialized heap memory

int x;
printf("%d\n", x);           // ❌ reading an uninitialized local

char buf[4];
strcpy(buf, "hello");        // ❌ buffer overflow
```

**Arithmetic**

```c
int i = INT_MAX; i++;        // ❌ signed overflow
int d = 1 / 0;               // ❌ integer division by zero
int m = INT_MIN % -1;        // ❌ overflow
int s = 1 << 32;             // ❌ shift >= width
int t = 1 << 31;             // ❌ shift into sign bit of signed int
int n = -1 << 2;             // ❌ left shift of a negative value
```

**Pointers**

```c
int a[5], b[5];
ptrdiff_t d = a - b;         // ❌ pointers into different arrays
int *p = a + 6;              // ❌ beyond one-past-the-end
int *q = a + 5;              // ✅ one past the end is LEGAL
*q;                          // ❌ but dereferencing it is not

/* Strict aliasing: accessing an object through an incompatible type */
float f = 1.0f;
int i = *(int *)&f;          // ❌ UB — use memcpy instead
memcpy(&i, &f, sizeof i);    // ✅ the correct type-punning idiom
```

**Sequencing**

```c
int i = 0;
a[i] = i++;                  // ❌ unsequenced read and modify
i = i++ + 1;                 // ❌
printf("%d %d\n", i++, i++); // ❌ order of argument evaluation unspecified,
                             //    and two modifications are unsequenced
f(i++, i++);                 // ❌ same
```

C11 formalized this as "unsequenced side effects." The rule of thumb: **a variable
you modify in an expression must not appear elsewhere in that same expression.**

Note that `i++ + i++` is UB but `i++, i++` (comma operator) is fine — the comma
operator is a sequence point.

**Functions and lifetime**

```c
char *f(void) { char buf[10]; return buf; }   // ❌ returns a dead pointer

int g(void) { }                               // ❌ falling off a non-void function
                                              //    and the caller uses the value

printf("%d\n", 3.14);                         // ❌ wrong format specifier
printf("%s\n", 42);                           // ❌ catastrophic — treats 42 as an address
printf("%d %d\n", 1);                         // ❌ too few arguments
```

Format-string mismatches are UB and `-Wformat` (in `-Wall`) catches nearly all of
them. This is a big part of why `-Wall` is non-negotiable.

**Library misuse**

```c
memcpy(dst, src, n);         // ❌ if dst and src overlap — use memmove
strcpy(dst, src);            // ❌ if they overlap, or dst is too small
free(stack_array);           // ❌ only free what came from malloc family
fclose(fp); fread(..., fp);  // ❌ use after close
isalpha(negative_char);      // ❌ must be a valid unsigned char value or EOF
```

### 16.4 Finding UB: the sanitizers

This is not optional knowledge. Sanitizers turn silent corruption into a precise
error report with a stack trace.

```bash
gcc -std=c17 -Wall -Wextra -g -fsanitize=address,undefined main.c -o main
./main
```

**AddressSanitizer (ASan)** catches buffer overflow, use-after-free, double
free, memory leaks, stack overflow, use-after-return.

**UndefinedBehaviorSanitizer (UBSan)** catches signed overflow, bad shifts, null
dereference, misaligned access, division by zero, invalid enum/bool values.

What a real report looks like:

```
=================================================================
==12345==ERROR: AddressSanitizer: heap-buffer-overflow on address 0x602000000018
WRITE of size 4 at 0x602000000018 thread T0
    #0 0x401234 in main /home/dev/main.c:8
0x602000000018 is located 0 bytes to the right of 4-byte region
allocated by thread T0 here:
    #0 0x7f... in malloc
    #1 0x401200 in main /home/dev/main.c:6
```

It tells you the exact line of the bad write **and** the exact line where that
memory was allocated. That's usually enough to fix the bug in under a minute.

MSVC supports ASan too:

```bat
cl /fsanitize=address /Zi main.c
```

> 🔑 **Sanitizers slow the program ~2× and use more memory. That's a fine trade
> while developing.** Build with them on for all your practice programs. Turn them
> off for release builds. Do not use `-fsanitize=address` together with valgrind.

### 16.5 Related tools

```bash
valgrind --leak-check=full ./main     # Linux/WSL — no recompile needed
gcc -fanalyzer -c main.c              # GCC static analysis, finds bugs without running
clang --analyze main.c                # Clang static analyzer
cppcheck --enable=all main.c          # independent static checker
```

Static analysis catches different bugs than sanitizers do — it explores paths your
tests never hit, but reports false positives. Use both.

### 16.6 The UB defense checklist

- Compile with `-Wall -Wextra -Werror`.
- Develop with `-fsanitize=address,undefined`.
- Initialize every variable at declaration.
- Always check `malloc` for `NULL`.
- Pass array lengths alongside array pointers, and validate indices.
- Use `snprintf` / `fgets`, never `strcpy` / `gets` / unbounded `scanf("%s")`.
- Use unsigned types for bit operations; never shift by `>= width`.
- Use `memcpy` for type punning, never a pointer cast.
- Set pointers to `NULL` after `free`.
- Don't modify a variable twice in one expression.
- If it works at `-O0` but not `-O2`, look for UB — not a compiler bug.

---

<a id="debugging"></a>
# Part III — Working Like an Engineer

## 17. Debugging: gdb, Sanitizers, Valgrind

### 17.1 The mindset

Debugging is a search for the first moment your program's actual state diverges
from your expected state. Everything below is a tool for narrowing that search.

The systematic loop:

1. **Reproduce reliably.** An intermittent bug you can't trigger is unfixable.
   Find the minimal input that fails.
2. **Form one specific hypothesis.** "The index is off by one at line 40," not
   "something's wrong with the loop."
3. **Test it with an observation** — a print, a breakpoint, an assertion.
4. **Narrow.** Bisect the program's execution: is the state good halfway through?
5. **Fix the cause, not the symptom.** If you don't understand *why* the fix
   works, you haven't fixed it.

> 🔑 If a change makes the bug "go away" and you can't explain the mechanism, you
> most likely moved undefined behavior somewhere less visible.

### 17.2 printf debugging (legitimately useful)

Fast, always available, and the right tool for understanding control flow.

```c
#include <stdio.h>

#define TRACE(fmt, ...) \
    fprintf(stderr, "%s:%d %s: " fmt "\n", \
            __FILE__, __LINE__, __func__, __VA_ARGS__)

/* usage */
TRACE("i=%d len=%zu val=%d", i, len, arr[i]);
```

Two rules that make it work well:

- **Print to `stderr`**, not `stdout`. `stderr` is unbuffered, so it appears
  immediately even if the program crashes right after. `stdout` is line-buffered
  and can lose your last messages on a segfault.
- **Print the variable name with the value.** `TRACE("i=%d", i)` beats
  `printf("%d\n", i)` when you have twelve of them.

If you must use `printf` to `stdout` while chasing a crash, call
`fflush(stdout)` after each one, or `setbuf(stdout, NULL)` once at startup.

### 17.3 assert

```c
#include <assert.h>

void process(const int *arr, size_t n, size_t index) {
    assert(arr != NULL);
    assert(index < n);            // documents AND checks the precondition
    /* ... */
}
```

A failed assertion prints the expression, file, and line, then aborts. Asserts
document your assumptions in a machine-checked way.

Compile with `-DNDEBUG` to remove them all from release builds:

```bash
gcc -DNDEBUG -O2 main.c -o main
```

> ⚠️ Because asserts vanish under `NDEBUG`, **never put a side effect inside one**:
> ```c
> assert(pop(&stack) == 5);      // ❌ the pop disappears in release!
> int v = pop(&stack);
> assert(v == 5);                // ✅
> ```

Use `assert` for programmer errors (bugs — "this can't happen"). Use real error
handling for runtime conditions (bad user input, missing file, allocation
failure). Never assert on something the user controls.

C11 adds compile-time assertions:

```c
_Static_assert(sizeof(int) == 4, "this code assumes 32-bit int");
static_assert(sizeof(void *) == 8, "64-bit only");   // C23 spelling
```

These cost nothing at runtime and catch platform assumptions at build time.

### 17.4 gdb — the essential commands

```bash
gcc -g -O0 main.c -o main       # -g for symbols, -O0 so code matches source
gdb ./main
```

| Command | Short | Does |
|---|---|---|
| `run [args]` | `r` | Start the program |
| `break main.c:42` | `b` | Breakpoint at a line |
| `break my_func` | `b` | Breakpoint at a function |
| `break foo if x > 100` | | **Conditional** breakpoint |
| `next` | `n` | Step one line, over function calls |
| `step` | `s` | Step one line, into function calls |
| `finish` | `fin` | Run until the current function returns |
| `continue` | `c` | Resume until the next breakpoint |
| `print expr` | `p` | Evaluate and show a C expression |
| `print arr[3]@5` | | Show 5 elements starting at `arr[3]` |
| `display x` | | Auto-print `x` at every stop |
| `watch x` | | Break when `x` **changes value** |
| `backtrace` | `bt` | The call stack — where am I? |
| `frame 2` | `f 2` | Switch to a caller's frame |
| `info locals` | | All local variables |
| `info args` | | Function arguments |
| `list` | `l` | Show source around the current line |
| `ptype x` | | Show the type of `x` |
| `set var x = 5` | | Change a variable mid-run |
| `layout src` | | Split-screen source view (TUI) |
| `quit` | `q` | Exit |

**The 30-second crash triage.** This alone justifies learning gdb:

```
$ gdb ./main
(gdb) run
Program received signal SIGSEGV, Segmentation fault.
0x0000555555555185 in process (arr=0x0, n=5) at main.c:12
12          return arr[0];
(gdb) bt
#0  process (arr=0x0, n=5) at main.c:12
#1  0x00005555555551b2 in main () at main.c:20
(gdb) info args
arr = (int *) 0x0
n = 5
```

`arr = 0x0` — it's NULL, passed from line 20. Bug located without a single
`printf`.

`watch` is the underrated one. When a variable is being corrupted and you have no
idea where:

```
(gdb) watch total
Hardware watchpoint 2: total
(gdb) continue
Hardware watchpoint 2: total
Old value = 15
New value = 32767
some_function (...) at other.c:88
```

It stops at the exact instruction that changed it.

For a crashing program, you can also debug the corpse. On Linux:

```bash
ulimit -c unlimited        # enable core dumps
./main                     # crashes, writes `core`
gdb ./main core
(gdb) bt
```

### 17.5 Where the bug usually is

| Symptom | Look at |
|---|---|
| Segfault on the first iteration | Uninitialized or NULL pointer |
| Segfault on the *last* iteration | Off-by-one — `<=` where you needed `<` |
| Wrong result, no crash | Uninitialized accumulator, integer division, wrong operator precedence |
| Corrupted unrelated variable | Buffer overflow into a neighbor |
| Crash inside `malloc`/`free` | Heap corruption from an *earlier* overflow |
| Works alone, breaks when called twice | Function-local `static`, or unfreed state |
| Hangs forever | Loop counter not advancing; unsigned `>= 0`; blocked on input |
| Skipped an input | Leftover `'\n'` from `scanf` |
| Differs between `-O0` and `-O2` | Undefined behavior |
| Differs between runs | Uninitialized memory, or a race |

### 17.6 Bisecting

When you can't reason about it, bisect mechanically.

**In the code:** comment out half the work. Does the bug persist? Keep halving.

**In the input:** cut the input in half. Which half triggers it?

**In history:**

```bash
git bisect start
git bisect bad                # current commit fails
git bisect good v1.0          # this old one worked
# git checks out the midpoint; you test and say `git bisect good` or `bad`
# repeat ~log2(n) times
git bisect reset
```

For 1000 commits that's 10 tests to find the exact breaking commit.

---

<a id="testing"></a>
## 18. Testing C Without a Framework, Then With One

### 18.1 The minimal harness (60 lines, no dependencies)

You do not need a library to start testing. Put this in `test.h`:

```c
#ifndef TEST_H
#define TEST_H

#include <stdio.h>
#include <string.h>
#include <math.h>

static int tests_run = 0;
static int tests_failed = 0;

#define CHECK(cond)                                                    \
    do {                                                               \
        tests_run++;                                                   \
        if (!(cond)) {                                                 \
            tests_failed++;                                            \
            printf("  FAIL %s:%d: %s\n", __FILE__, __LINE__, #cond);   \
        }                                                              \
    } while (0)

#define CHECK_INT(actual, expected)                                    \
    do {                                                               \
        tests_run++;                                                   \
        long long a_ = (actual), e_ = (expected);                      \
        if (a_ != e_) {                                                \
            tests_failed++;                                            \
            printf("  FAIL %s:%d: %s == %lld, expected %lld\n",        \
                   __FILE__, __LINE__, #actual, a_, e_);               \
        }                                                              \
    } while (0)

#define CHECK_STR(actual, expected)                                    \
    do {                                                               \
        tests_run++;                                                   \
        const char *a_ = (actual), *e_ = (expected);                   \
        if (strcmp(a_, e_) != 0) {                                     \
            tests_failed++;                                            \
            printf("  FAIL %s:%d: %s == \"%s\", expected \"%s\"\n",    \
                   __FILE__, __LINE__, #actual, a_, e_);               \
        }                                                              \
    } while (0)

#define CHECK_NEAR(actual, expected, eps)                              \
    do {                                                               \
        tests_run++;                                                   \
        double a_ = (actual), e_ = (expected);                         \
        if (fabs(a_ - e_) > (eps)) {                                   \
            tests_failed++;                                            \
            printf("  FAIL %s:%d: %s == %g, expected %g\n",            \
                   __FILE__, __LINE__, #actual, a_, e_);               \
        }                                                              \
    } while (0)

#define RUN_TEST(fn)                                                   \
    do { printf("%s\n", #fn); fn(); } while (0)

#define TEST_REPORT()                                                  \
    (printf("\n%d checks, %d failed\n", tests_run, tests_failed),       \
     tests_failed != 0)

#endif
```

Using it:

```c
/* test_geometry.c */
#include "test.h"
#include "geometry.h"

static void test_distance(void) {
    Point a = {0, 0}, b = {3, 4};
    CHECK_NEAR(distance(a, b), 5.0, 1e-9);       // 3-4-5 triangle
    CHECK_NEAR(distance(a, a), 0.0, 1e-9);       // identity
    CHECK_NEAR(distance(b, a), 5.0, 1e-9);       // symmetry
}

static void test_triangle_area(void) {
    Point a = {0, 0}, b = {4, 0}, c = {0, 3};
    CHECK_NEAR(triangle_area(a, b, c), 6.0, 1e-9);

    Point d = {1, 1}, e = {2, 2}, f = {3, 3};
    CHECK_NEAR(triangle_area(d, e, f), 0.0, 1e-9);   // collinear → zero
}

int main(void) {
    RUN_TEST(test_distance);
    RUN_TEST(test_triangle_area);
    return TEST_REPORT();          // nonzero exit code if anything failed
}
```

```bash
gcc -std=c17 -Wall -Wextra -g -fsanitize=address,undefined \
    test_geometry.c geometry.c -o test -lm
./test
```

Returning `tests_failed != 0` from `main` is what lets `make test` and CI detect
failure. Always propagate the exit code.

### 18.2 What to test

For every function, cover four categories:

| Category | Examples |
|---|---|
| **Typical case** | The thing the function is for |
| **Boundaries** | 0, 1, n-1, n, empty, single element, full capacity |
| **Invalid input** | NULL, negative length, out-of-range index |
| **Known-tricky** | Overflow limits, duplicate keys, self-assignment, reallocation |

```c
static void test_vector(void) {
    Vec v = {0};

    /* empty */
    CHECK_INT(vec_len(&v), 0);

    /* single */
    CHECK(vec_push(&v, 10));
    CHECK_INT(vec_len(&v), 1);
    CHECK_INT(v.data[0], 10);

    /* forces several reallocations — catches growth bugs */
    for (int i = 0; i < 1000; i++) CHECK(vec_push(&v, i));
    CHECK_INT(vec_len(&v), 1001);
    CHECK_INT(v.data[1000], 999);

    /* cleanup must not leak — ASan verifies this */
    vec_free(&v);
    CHECK(v.data == NULL);
}
```

That "1000 pushes" test is the one that finds capacity-doubling off-by-ones, and
running it under ASan simultaneously verifies there are no leaks or overflows.

### 18.3 Make integration

```make
CC      := gcc
CFLAGS  := -std=c17 -Wall -Wextra -g -Iinclude
SAN     := -fsanitize=address,undefined

test: tests/test_geometry.c src/geometry.c
    $(CC) $(CFLAGS) $(SAN) $^ -o build/test -lm
    ./build/test

.PHONY: test
```

Now `make test` builds and runs everything with sanitizers on.

### 18.4 Real frameworks

When the harness above stops being enough:

| Framework | Style | Notes |
|---|---|---|
| **Unity** | Single `.c` + `.h`, drop into your repo | Tiny, embedded-friendly, very popular |
| **greatest** | Single header | Zero setup, good output |
| **µnit / munit** | Single file | Parameterized tests, timing |
| **Check** | Library, forks per test | Isolates crashes so one segfault doesn't kill the run |
| **CMocka** | Library | Real mocking and function stubbing |

Unity in practice:

```c
#include "unity.h"
#include "geometry.h"

void setUp(void)    { }        // runs before each test
void tearDown(void) { }        // runs after each test

void test_distance_345(void) {
    Point a = {0, 0}, b = {3, 4};
    TEST_ASSERT_DOUBLE_WITHIN(1e-9, 5.0, distance(a, b));
}

int main(void) {
    UNITY_BEGIN();
    RUN_TEST(test_distance_345);
    return UNITY_END();
}
```

The `Check` framework's process isolation is genuinely valuable in C: a test that
segfaults gets reported as a failure instead of terminating the whole test binary.

### 18.5 Coverage

Which lines did your tests actually execute?

```bash
gcc --coverage -O0 test_geometry.c geometry.c -o test -lm
./test
gcov geometry.c
# → geometry.c.gcov shows an execution count per line; ##### marks never-run lines

# Nicer HTML report:
lcov --capture --directory . --output-file coverage.info
genhtml coverage.info --output-directory out
```

Coverage tells you what you *haven't* tested. It does not tell you your tests are
good — 100% coverage with no assertions proves nothing. Use it to find blind
spots, not as a score to maximize.

### 18.6 Fuzzing

Feed random input and see what breaks. Astonishingly effective for parsers.

```c
/* fuzz_target.c */
#include <stddef.h>
#include <stdint.h>

int parse_config(const char *text, size_t len);   // your function

int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size) {
    parse_config((const char *)data, size);
    return 0;
}
```

```bash
clang -g -fsanitize=fuzzer,address,undefined fuzz_target.c parser.c -o fuzz
./fuzz                      # runs until it finds a crash, then saves the input
```

libFuzzer mutates inputs intelligently, guided by code coverage. Point it at any
function that parses untrusted bytes and let it run for a few minutes — it will
find edge cases you never imagined.

---

<a id="errors"></a>
## 19. Error Handling Patterns

C has no exceptions. Every error must be represented in a return value or an
out-parameter, and **checked at the call site**. Discipline here is what separates
robust C from fragile C.

### 19.1 Pattern 1 — return a status code, results via out-parameters

The most common and most flexible approach.

```c
typedef enum {
    OK = 0,
    ERR_NULL_ARG,
    ERR_OUT_OF_MEMORY,
    ERR_NOT_FOUND,
    ERR_INVALID_INPUT,
    ERR_IO,
} Status;

const char *status_str(Status s) {
    switch (s) {
        case OK:                return "ok";
        case ERR_NULL_ARG:      return "null argument";
        case ERR_OUT_OF_MEMORY: return "out of memory";
        case ERR_NOT_FOUND:     return "not found";
        case ERR_INVALID_INPUT: return "invalid input";
        case ERR_IO:            return "I/O error";
    }
    return "unknown error";
}

Status parse_port(const char *s, int *out) {
    if (!s || !out) return ERR_NULL_ARG;

    char *end;
    errno = 0;
    long v = strtol(s, &end, 10);

    if (end == s || *end != '\0')      return ERR_INVALID_INPUT;
    if (errno == ERANGE)               return ERR_INVALID_INPUT;
    if (v < 1 || v > 65535)            return ERR_INVALID_INPUT;

    *out = (int)v;
    return OK;
}
```

```c
int port;
Status st = parse_port(argv[1], &port);
if (st != OK) {
    fprintf(stderr, "bad port: %s\n", status_str(st));
    return EXIT_FAILURE;
}
```

Note the `switch` with no `default:` in `status_str` — add a new enum member and
`-Wswitch` reminds you to handle it.

### 19.2 Pattern 2 — sentinel return values

Works when one value can't be a legitimate result.

```c
void *malloc(size_t);              // NULL on failure
FILE *fopen(const char *, const char *);   // NULL on failure
char *strchr(const char *, int);   // NULL if not found
int   getchar(void);               // EOF on end/error
long  ftell(FILE *);               // -1L on error
```

Simple, but it can't distinguish *why* it failed — which is exactly why `errno`
exists alongside it.

### 19.3 Pattern 3 — errno

Standard library functions set the global `errno` on failure.

```c
#include <errno.h>
#include <string.h>
#include <stdio.h>

FILE *fp = fopen("missing.txt", "r");
if (!fp) {
    fprintf(stderr, "fopen failed: %s\n", strerror(errno));
    perror("fopen");            // shorter: prints "fopen: No such file or directory"
    return EXIT_FAILURE;
}
```

Rules for using `errno` correctly:

- **Set `errno = 0` before the call** if you intend to inspect it, because
  functions only ever set it on failure — a stale value from an earlier call will
  mislead you.
- Only check `errno` **after** you've confirmed failure by the return value. A
  successful call may modify `errno` arbitrarily.
- `strtol` is the classic case: it returns 0 both for "the string was 0" and for
  "no digits found," so you must check the `end` pointer *and* `errno == ERANGE`.

```c
errno = 0;
long v = strtol(s, &end, 10);
if (end == s)                 { /* no digits */ }
else if (errno == ERANGE)     { /* overflow: v is LONG_MAX or LONG_MIN */ }
else if (*end != '\0')        { /* trailing garbage */ }
else                          { /* clean parse */ }
```

`errno` is thread-local in C11 and POSIX, so it's safe in threaded code.

### 19.4 Pattern 4 — the cleanup ladder (`goto`)

This is the one place `goto` is idiomatic C, and it's used heavily in the Linux
kernel. It solves the "acquire N resources, release them correctly on any failure"
problem without deep nesting.

```c
#include <stdio.h>
#include <stdlib.h>

int process_file(const char *path) {
    FILE   *fp    = NULL;
    char   *buf   = NULL;
    int    *table = NULL;
    int     rc    = -1;                 // pessimistic default

    fp = fopen(path, "r");
    if (!fp) goto cleanup;

    buf = malloc(4096);
    if (!buf) goto cleanup;

    table = calloc(256, sizeof *table);
    if (!table) goto cleanup;

    /* ... real work; any failure can `goto cleanup` ... */

    rc = 0;                             // success

cleanup:
    free(table);                        // free(NULL) is safe, so no checks needed
    free(buf);
    if (fp) fclose(fp);                 // fclose(NULL) is NOT safe — check this one
    return rc;
}
```

Why this beats nested `if`s: there is exactly **one** exit path, so you cannot
forget a `free` on some rare error branch. That's the single most common source of
leaks in C.

Note the asymmetry: `free(NULL)` is explicitly legal, `fclose(NULL)` is not.

Multiple labels when cleanup must be partial and ordered:

```c
    a = acquire_a(); if (!a) goto fail;
    b = acquire_b(); if (!b) goto fail_a;
    c = acquire_c(); if (!c) goto fail_b;

    /* work */
    release_c(c);
fail_b: release_b(b);
fail_a: release_a(a);
fail:   return rc;
```

### 19.5 Pattern 5 — a result struct

Bundles the value and the status so nothing can be read without the status
present.

```c
typedef struct {
    Status status;
    int    value;
} IntResult;

IntResult parse_int(const char *s) {
    IntResult r = { .status = ERR_INVALID_INPUT, .value = 0 };
    /* ... */
    return r;
}

IntResult r = parse_int(s);
if (r.status == OK) use(r.value);
```

Clean for small types. Less appealing for large ones since it copies.

### 19.6 What NOT to do

```c
/* ❌ Ignoring return values */
malloc(100);                   // leak, and you never knew if it worked
scanf("%d", &x);               // did it actually read anything?
fclose(fp);                    // can fail — buffered data may not have flushed

/* ❌ exit() from library code — the caller loses all control */
void my_lib_fn(void) {
    if (bad) { printf("error\n"); exit(1); }   // never do this in a library
}

/* ❌ Printing errors from deep inside a library */
/*    Return the error; let the APPLICATION decide how to report it. */

/* ❌ Using errno without checking the return value first */
strtol(s, &end, 10);
if (errno) { ... }             // errno may be stale from something unrelated
```

`fclose` really can fail — that's when buffered writes get flushed to disk. For
programs where data integrity matters, check it.

### 19.7 Documenting the contract

Every non-trivial function should state its contract in a comment. This is the
closest thing C has to a type system for errors and ownership:

```c
/* Reads at most `cap - 1` bytes from `path` into `dst`, NUL-terminating.
 *
 * Returns OK on success.
 *         ERR_NULL_ARG    if path or dst is NULL, or cap == 0.
 *         ERR_IO          if the file cannot be opened or read.
 *
 * On failure, the contents of `dst` are unspecified.
 * Does not take ownership of any argument. Does not allocate. */
Status read_text_file(const char *path, char *dst, size_t cap);
```

State: what it returns, what each error means, what state things are left in on
failure, and who owns what memory. Those four facts prevent most misuse.

---

<a id="cli"></a>
## 20. Command-Line Programs: argc, argv, Exit Codes, Pipes

### 20.1 argc and argv

```c
#include <stdio.h>

int main(int argc, char *argv[]) {
    printf("program name: %s\n", argv[0]);
    printf("argument count: %d\n", argc);       // includes argv[0]

    for (int i = 1; i < argc; i++)
        printf("argv[%d] = %s\n", i, argv[i]);

    return 0;
}
```

```
$ ./prog hello 42 "two words"
program name: ./prog
argument count: 4
argv[1] = hello
argv[2] = 42
argv[3] = two words
```

Facts worth knowing:

- `argv[argc]` is guaranteed to be `NULL`, so you can iterate `while (*++argv)`.
- Every argument is a **string**, even numbers — you must parse them.
- The shell handles quoting and glob expansion before your program starts. `*.txt`
  arrives already expanded into many arguments.
- `argv[0]` is conventionally the program name but is not guaranteed to be
  meaningful; don't rely on it for logic.

### 20.2 Parsing numeric arguments safely

```c
#include <errno.h>
#include <limits.h>
#include <stdlib.h>
#include <stdio.h>

static int parse_int_arg(const char *s, int *out) {
    char *end;
    errno = 0;
    long v = strtol(s, &end, 10);

    if (end == s || *end != '\0') return 0;      // not a clean number
    if (errno == ERANGE || v < INT_MIN || v > INT_MAX) return 0;

    *out = (int)v;
    return 1;
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "usage: %s <count>\n", argv[0]);
        return EXIT_FAILURE;
    }

    int count;
    if (!parse_int_arg(argv[1], &count)) {
        fprintf(stderr, "%s: '%s' is not a valid integer\n", argv[0], argv[1]);
        return EXIT_FAILURE;
    }

    printf("count = %d\n", count);
    return EXIT_SUCCESS;
}
```

Use this rather than `atoi`. `atoi("abc")` returns 0 with no way to detect the
error, and `atoi` has undefined behavior on overflow.

### 20.3 A hand-rolled option parser

`getopt` is POSIX (available in MSYS2/WSL, not MSVC). Portable manual parsing:

```c
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

typedef struct {
    int         verbose;
    int         count;
    const char *output;
    const char *input;
} Options;

static void usage(const char *prog, FILE *out) {
    fprintf(out,
        "usage: %s [options] <input>\n"
        "\n"
        "options:\n"
        "  -h, --help           show this help\n"
        "  -v, --verbose        verbose output\n"
        "  -n, --count N        repeat N times (default 1)\n"
        "  -o, --output FILE    write to FILE\n",
        prog);
}

int main(int argc, char *argv[]) {
    Options opt = { .count = 1 };

    int i = 1;
    for (; i < argc; i++) {
        const char *a = argv[i];

        if (a[0] != '-' || strcmp(a, "-") == 0) break;   // first non-option
        if (strcmp(a, "--") == 0) { i++; break; }        // end of options

        if (!strcmp(a, "-h") || !strcmp(a, "--help")) {
            usage(argv[0], stdout);
            return EXIT_SUCCESS;
        } else if (!strcmp(a, "-v") || !strcmp(a, "--verbose")) {
            opt.verbose = 1;
        } else if (!strcmp(a, "-n") || !strcmp(a, "--count")) {
            if (++i >= argc) { fprintf(stderr, "%s: -n needs a value\n", argv[0]); return 2; }
            opt.count = atoi(argv[i]);
        } else if (!strcmp(a, "-o") || !strcmp(a, "--output")) {
            if (++i >= argc) { fprintf(stderr, "%s: -o needs a value\n", argv[0]); return 2; }
            opt.output = argv[i];
        } else {
            fprintf(stderr, "%s: unknown option '%s'\n", argv[0], a);
            usage(argv[0], stderr);
            return 2;
        }
    }

    if (i >= argc) {
        fprintf(stderr, "%s: missing input file\n", argv[0]);
        usage(argv[0], stderr);
        return 2;
    }
    opt.input = argv[i];

    if (opt.verbose)
        fprintf(stderr, "input=%s output=%s count=%d\n",
                opt.input, opt.output ? opt.output : "<stdout>", opt.count);

    return EXIT_SUCCESS;
}
```

Conventions this follows, which users expect:

- `-h/--help` prints usage to **stdout** and exits 0 (it's a successful request).
- Errors print usage to **stderr** and exit non-zero.
- `--` terminates options so you can pass a filename that starts with `-`.
- Unknown options are an error, not silently ignored.

### 20.4 Exit codes

```c
return 0;               // or EXIT_SUCCESS — success
return 1;               // or EXIT_FAILURE — generic failure
return 2;               // conventionally: usage/CLI error
```

Only the low 8 bits are visible to the shell, so keep codes in 0–125. The shell
reads it as:

```bash
./prog; echo $?          # bash / MSYS2
./prog; echo $LASTEXITCODE   # PowerShell
```

Exit codes are how `make`, CI systems, and shell `&&` chains detect failure. A
program that always returns 0 cannot be scripted.

`exit()` vs `return` from `main`: they're equivalent in `main`. Elsewhere, `exit()`
terminates the whole program — flushing streams and running `atexit` handlers.
`_Exit()` skips all cleanup.

```c
#include <stdlib.h>

static void cleanup(void) { printf("goodbye\n"); }

int main(void) {
    atexit(cleanup);           // runs on exit() or return from main
    /* ... */
    return 0;
}
```

### 20.5 stdin, stdout, stderr — the three streams

```c
printf("result\n");                    // → stdout: the program's actual OUTPUT
fprintf(stderr, "warning\n");          // → stderr: diagnostics, progress, errors
fgets(buf, sizeof buf, stdin);         // ← stdin: input
```

The separation is what makes Unix pipelines work:

```bash
./prog < input.txt > output.txt 2> errors.log
./prog data.txt | sort | uniq -c | head
./prog 2>&1 | less                 # merge stderr into stdout
```

> 🔑 **Data goes to stdout; everything else goes to stderr.** If you print progress
> messages to stdout, you corrupt the data stream and break every pipeline your
> program appears in.

### 20.6 A filter: read stdin, write stdout

The classic Unix program shape. It works with files *and* pipes for free.

```c
#include <stdio.h>
#include <ctype.h>

int main(int argc, char *argv[]) {
    FILE *in = stdin;

    if (argc > 1) {
        in = fopen(argv[1], "r");
        if (!in) { perror(argv[1]); return 1; }
    }

    int ch;
    while ((ch = fgetc(in)) != EOF)
        putchar(toupper((unsigned char)ch));

    if (ferror(in)) { perror("read"); }
    if (in != stdin) fclose(in);
    return 0;
}
```

```bash
./upper file.txt          # from a file
cat file.txt | ./upper    # from a pipe
echo hi | ./upper         # from an echo
```

Defaulting to `stdin` when no file is given is a small touch that makes your
program composable.

### 20.7 Detecting a terminal vs a pipe

Useful for deciding whether to print colors or progress bars.

```c
#ifdef _WIN32
  #include <io.h>
  #define ISATTY(fd) _isatty(fd)
  #define FILENO(f)  _fileno(f)
#else
  #include <unistd.h>
  #define ISATTY(fd) isatty(fd)
  #define FILENO(f)  fileno(f)
#endif

int interactive = ISATTY(FILENO(stdout));
if (interactive) printf("\033[32mgreen text\033[0m\n");
else             printf("plain text\n");
```

Not standard C — this is a platform capability, hence the `#ifdef`.

### 20.8 Environment variables

```c
#include <stdlib.h>

const char *home = getenv("HOME");          // NULL if not set
const char *path = getenv("PATH");
if (!home) home = getenv("USERPROFILE");    // Windows fallback
```

Never dereference the result without a NULL check. Setting variables (`setenv`,
`_putenv_s`) is platform-specific.

---

<a id="security"></a>
## 21. Security: The C-Specific OWASP

C gives you direct memory access and no runtime checks. Most historically severe
vulnerabilities — Heartbleed, Shellshock, countless kernel exploits — are C memory
bugs. These are the categories and their fixes.

### 21.1 Buffer overflow

The archetypal C vulnerability. Writing past a buffer can overwrite adjacent
variables, saved return addresses, and function pointers — turning a bug into
arbitrary code execution.

```c
/* ❌ CATASTROPHIC — no bound at all */
char name[8];
gets(name);                            // removed from C11 for this reason
scanf("%s", name);                     // unbounded
strcpy(name, untrusted);               // unbounded
sprintf(name, "%s", untrusted);        // unbounded

/* ✅ Bounded equivalents */
fgets(name, sizeof name, stdin);
scanf("%7s", name);                    // sizeof - 1
snprintf(name, sizeof name, "%s", untrusted);
```

```c
/* ❌ Off-by-one — the classic "fencepost" bug */
char buf[10];
for (int i = 0; i <= 10; i++) buf[i] = 'x';    // writes buf[10]

/* ✅ */
for (int i = 0; i < 10; i++) buf[i] = 'x';
```

Always compute bounds with `sizeof` on the array itself, never a repeated literal:

```c
char buf[64];
fgets(buf, 64, stdin);           // ⚠️ breaks silently if you resize buf later
fgets(buf, sizeof buf, stdin);   // ✅ always correct
```

### 21.2 Integer overflow leading to a small allocation

A subtle and very real attack path:

```c
/* ❌ count * size can wrap, allocating far less than requested,
   after which the loop writes way past the end. */
void *buf = malloc(count * sizeof(Item));
for (size_t i = 0; i < count; i++) ((Item*)buf)[i] = items[i];
```

```c
/* ✅ Check before multiplying */
if (count > SIZE_MAX / sizeof(Item)) return NULL;
void *buf = malloc(count * sizeof(Item));

/* ✅ Or use calloc, which is required to detect the overflow itself */
void *buf = calloc(count, sizeof(Item));
```

Prefer `calloc` for array allocations specifically because of this guarantee.

### 21.3 Format string vulnerability

```c
char user_input[128];
fgets(user_input, sizeof user_input, stdin);

printf(user_input);              // ❌ VULNERABILITY
printf("%s", user_input);        // ✅
```

If input contains `%x %x %x`, `printf` reads values off the stack and leaks them.
`%n` can **write** to memory. Never pass untrusted data as a format string.

`-Wformat-security` (add it explicitly) catches non-literal format strings.

### 21.4 Use-after-free and double-free

Both are exploitable, not merely crashy — an attacker who controls what gets
allocated into the freed slot can hijack execution.

```c
free(ptr);
ptr = NULL;            // ✅ makes subsequent use an immediate, harmless crash
```

Also beware the dangling alias — setting one pointer to NULL doesn't help if
another copy exists:

```c
char *a = malloc(10);
char *b = a;
free(a);
a = NULL;
b[0] = 'x';            // ❌ still a use-after-free
```

Single, clear ownership is the only real defense. Document who frees what.

### 21.5 Uninitialized memory disclosure

Heartbleed's family. Sending a buffer you didn't fully initialize leaks whatever
was in that memory before — potentially keys or other users' data.

```c
/* ❌ Padding bytes and unwritten fields contain old heap contents */
struct Packet p;
p.type = 1;
p.len  = 4;
send(fd, &p, sizeof p);        // leaks padding + uninitialized fields

/* ✅ Zero everything first */
struct Packet p = {0};         // or memset(&p, 0, sizeof p);
p.type = 1;
p.len  = 4;
send(fd, &p, sizeof p);
```

### 21.6 Validate at the boundary

Trust nothing that comes from outside your program: user input, files, network
data, environment variables, command-line arguments.

```c
Status handle_request(const char *raw, size_t len) {
    if (!raw)                 return ERR_NULL_ARG;
    if (len == 0)             return ERR_INVALID_INPUT;
    if (len > MAX_REQUEST)    return ERR_INVALID_INPUT;   // reject, don't truncate

    size_t index;
    if (!parse_index(raw, &index))   return ERR_INVALID_INPUT;
    if (index >= table_len)          return ERR_INVALID_INPUT;   // ← bounds check

    /* From here on, the data is known-good. Internal code needn't re-check. */
    return process(table[index]);
}
```

Validate **once, at the edge**, then trust internally. Sprinkling defensive checks
through every internal function is noise; a single hard boundary is verifiable.

### 21.7 Path traversal

```c
/* ❌ user_file could be "../../etc/passwd" */
char path[256];
snprintf(path, sizeof path, "/var/data/%s", user_file);
```

Reject anything containing `..`, absolute paths, or path separators, and prefer
resolving with `realpath` (POSIX) / `GetFullPathName` (Windows) then verifying the
result is still inside your intended directory.

```c
if (strstr(user_file, "..") || strchr(user_file, '/') || strchr(user_file, '\\'))
    return ERR_INVALID_INPUT;
```

### 21.8 Command injection

```c
/* ❌ user_input = "x; rm -rf /" */
char cmd[256];
snprintf(cmd, sizeof cmd, "convert %s out.png", user_input);
system(cmd);
```

`system()` runs a shell, which interprets `;`, `|`, `&&`, backticks, and `$()`.
Avoid `system()` with any untrusted data. Use `exec`-family calls (POSIX) or
`CreateProcess` (Windows) that take an argument **array**, so no shell parsing
occurs. If you truly must use a shell, whitelist the allowed characters rather
than trying to blacklist dangerous ones.

### 21.9 Weak randomness

```c
srand(time(NULL));
int token = rand();          // ❌ predictable — never for passwords, tokens, keys
```

`rand()` is a small, fully predictable PRNG. Seeded with the current time, an
attacker can reproduce your entire sequence. For anything security-relevant use
the OS CSPRNG:

- Windows: `BCryptGenRandom`
- Linux/macOS: `getrandom()` or read `/dev/urandom`
- C11 optional: `arc4random_buf` (BSD/glibc 2.36+)

`rand()` is fine for games, shuffling, and simulations.

### 21.10 The security build

```bash
gcc -std=c17 -O2 \
    -Wall -Wextra -Werror \
    -Wformat=2 -Wformat-security \
    -Wconversion -Wsign-conversion \
    -Wshadow -Wcast-qual -Wstrict-prototypes \
    -D_FORTIFY_SOURCE=2 \
    -fstack-protector-strong \
    -fPIE -pie \
    -Wl,-z,relro,-z,now \
    main.c -o main
```

| Flag | Protection |
|---|---|
| `-D_FORTIFY_SOURCE=2` | Runtime checks on `memcpy`, `strcpy`, `sprintf`, etc. |
| `-fstack-protector-strong` | Detects stack buffer overflows via canary values |
| `-fPIE -pie` | Address space layout randomization for the executable |
| `-Wl,-z,relro,-z,now` | Read-only relocations; blocks GOT overwrite attacks |

These cost almost nothing at runtime and stop entire exploit classes. Ship with
them on.

### 21.11 Checklist

- [ ] No `gets`, `strcpy`, `strcat`, `sprintf`, unbounded `scanf("%s")`
- [ ] Every buffer bound uses `sizeof`, not a repeated literal
- [ ] Every `malloc`/`calloc`/`realloc` return value checked for NULL
- [ ] Multiplications in allocation sizes checked for overflow (or use `calloc`)
- [ ] All array indices bounds-checked against the actual length
- [ ] Format strings are always literals
- [ ] Pointers set to NULL after `free`; single clear owner per allocation
- [ ] Structs sent over the wire are zero-initialized
- [ ] All external input validated at the boundary
- [ ] No `system()` with untrusted data
- [ ] Cryptographic randomness from the OS, not `rand()`
- [ ] Built and tested under `-fsanitize=address,undefined`
- [ ] Parsers fuzzed

---

<a id="performance"></a>
## 22. Performance: Measure, Then Optimize

### 22.1 The order of operations

1. **Make it correct.** A fast wrong answer is worthless.
2. **Measure.** Find the actual hot spot. It is almost never where you guessed.
3. **Improve the algorithm.** O(n²) → O(n log n) beats every micro-optimization
   combined.
4. **Help the compiler.** Better data layout, fewer allocations, less indirection.
5. **Micro-optimize.** Last resort, always with measurements on both sides.

> 🔑 Donald Knuth's full quote is worth having straight: *"Premature optimization
> is the root of all evil"* — but he prefaced it with the observation that you
> should absolutely optimize the critical 3% once you've **identified** it by
> measurement.

### 22.2 Timing code

```c
#include <time.h>
#include <stdio.h>

int main(void) {
    /* clock() — CPU time consumed, portable, ~ms resolution */
    clock_t start = clock();
    heavy_work();
    double cpu = (double)(clock() - start) / CLOCKS_PER_SEC;
    printf("cpu: %.3f s\n", cpu);

    /* clock_gettime — wall-clock, nanosecond resolution (C11 / POSIX) */
    struct timespec t0, t1;
    timespec_get(&t0, TIME_UTC);
    heavy_work();
    timespec_get(&t1, TIME_UTC);
    double wall = (t1.tv_sec - t0.tv_sec) + (t1.tv_nsec - t0.tv_nsec) / 1e9;
    printf("wall: %.6f s\n", wall);

    return 0;
}
```

Benchmarking discipline that actually matters:

- **Build with optimizations on** (`-O2`). Timing a `-O0` build measures nothing
  useful.
- **Run the work many times** and take the *minimum*, not the average — the
  minimum has the least noise from OS scheduling.
- **Warm up** first so caches and branch predictors are in a steady state.
- **Use the result.** If you discard it, the optimizer may delete the entire
  computation. Accumulate into a volatile or print it.

```c
/* Prevent the optimizer from deleting work whose result is unused */
volatile long long sink = 0;
sink += result;
```

### 22.3 Profilers

```bash
# perf — Linux, the best general tool
perf stat ./prog                     # cycles, instructions, cache misses, IPC
perf record -g ./prog && perf report # interactive, per-function, with call graphs

# gprof — portable, needs instrumentation
gcc -pg -O2 main.c -o main && ./main && gprof ./main gmon.out

# valgrind's callgrind — exact instruction counts, very slow but deterministic
valgrind --tool=callgrind ./prog
callgrind_annotate callgrind.out.*

# cachegrind — cache miss simulation
valgrind --tool=cachegrind ./prog
```

`perf stat` output tells you the *kind* of problem you have:

- Low IPC (instructions per cycle, < 1) → stalling on memory or mispredicted branches
- High cache miss rate → data layout problem
- High branch miss rate → unpredictable branches

### 22.4 Algorithmic complexity is the main event

| Operation | Array | Sorted array | Linked list | Hash table | BST (balanced) |
|---|---|---|---|---|---|
| Access by index | **O(1)** | O(1) | O(n) | — | — |
| Search by value | O(n) | O(log n) | O(n) | **O(1)** avg | O(log n) |
| Insert at front | O(n) | O(n) | **O(1)** | O(1) | O(log n) |
| Insert at back | O(1) amort. | O(n) | O(1)* | O(1) | O(log n) |
| Delete known node | O(n) | O(n) | **O(1)** | O(1) | O(log n) |
| Ordered traversal | O(n) | O(n) | O(n) | O(n log n) | **O(n)** |

\* with a tail pointer

```c
/* ❌ O(n²) — strlen rescans the whole string every iteration */
for (size_t i = 0; i < strlen(s); i++) ...

/* ✅ O(n) */
size_t n = strlen(s);
for (size_t i = 0; i < n; i++) ...

/* ❌ O(n²) — realloc on every single push */
for (int i = 0; i < n; i++) {
    arr = realloc(arr, (i + 1) * sizeof *arr);
    arr[i] = i;
}

/* ✅ O(n) amortized — double the capacity when full */
if (len == cap) { cap = cap ? cap * 2 : 16; arr = realloc(arr, cap * sizeof *arr); }
arr[len++] = i;
```

### 22.5 Memory layout beats instruction count

Modern CPUs are ~100× faster than main memory. A cache miss costs hundreds of
cycles. **Data layout is usually the dominant performance factor.**

```
Approximate latencies:
  L1 cache        ~1 ns      ← you want to be here
  L2 cache        ~4 ns
  L3 cache        ~20 ns
  Main memory   ~100 ns      ← 100× slower than L1
  SSD           ~100 µs
  Disk           ~10 ms
```

**Sequential access is dramatically faster than random access:**

```c
/* ✅ Sequential — each cache line fetch delivers ~16 useful ints */
for (size_t i = 0; i < n; i++) sum += arr[i];

/* ❌ Random — every access is potentially a fresh cache miss */
for (size_t i = 0; i < n; i++) sum += arr[index[i]];
```

**Row-major order matters** (from [section 10](#arrays)):

```c
for (int r = 0; r < R; r++)
    for (int c = 0; c < C; c++)
        sum += m[r][c];              // ✅ sequential in memory

for (int c = 0; c < C; c++)
    for (int r = 0; r < R; r++)
        sum += m[r][c];              // ❌ strides by a whole row each step
```

On a large matrix the second version can be 5–10× slower with identical
instruction counts.

**Struct-of-Arrays vs Array-of-Structs.** If you process one field across many
items, split the fields:

```c
/* Array of Structs — good when you touch whole objects */
typedef struct { float x, y, z; int id; char name[32]; } Particle;
Particle particles[10000];
for (int i = 0; i < n; i++) particles[i].x += 1.0f;   // loads 44 bytes to use 4

/* Struct of Arrays — good when you touch one field at a time */
typedef struct {
    float x[10000], y[10000], z[10000];
    int   id[10000];
} Particles;
for (int i = 0; i < n; i++) ps.x[i] += 1.0f;          // every byte loaded is used
```

The SoA version is also what lets the compiler auto-vectorize into SIMD
instructions.

**Smaller structs are faster.** Ordering members largest-to-smallest to eliminate
padding ([section 11.3](#structs)) means more objects per cache line.

### 22.6 Things that actually help

```c
/* Avoid allocation in hot loops — allocate once, reuse */
char *buf = malloc(SIZE);
for (...) { use(buf); }
free(buf);

/* static inline for tiny hot functions in headers */
static inline int min_i(int a, int b) { return a < b ? a : b; }

/* Hoist loop-invariant work out of the loop */
size_t n = strlen(s);                  // not in the condition
double k = 2.0 * PI / period;          // not recomputed each iteration

/* restrict tells the compiler two pointers never alias, enabling vectorization */
void add(float *restrict out, const float *restrict a,
         const float *restrict b, size_t n) {
    for (size_t i = 0; i < n; i++) out[i] = a[i] + b[i];
}

/* const on inputs helps both readers and the optimizer */
double sum(const double *a, size_t n);
```

`restrict` is a promise you make to the compiler. If the pointers *do* overlap,
the behavior is undefined — so only use it when you're certain.

### 22.7 Things that usually don't help

```c
register int i;              // ignored by every modern compiler
i <<= 1;                     // instead of i *= 2 — the compiler already does this
x = y ^ z ^ x;               // "clever" bit tricks; often slower than the obvious code
inline everything            // bloats the instruction cache, can slow things down
manual loop unrolling        // the compiler does this better, with real cost models
++i instead of i++           // identical for scalars in any optimizing compiler
```

The compiler at `-O2` is extremely good at local optimizations. Your leverage is
in algorithms and data layout — things the compiler cannot change because they're
semantically visible.

### 22.8 Verify what the compiler did

```bash
gcc -O2 -S -masm=intel -o - main.c | less     # read the assembly
gcc -O2 -fopt-info-vec main.c                  # which loops got vectorized
gcc -O2 -fopt-info-vec-missed main.c           # and why others didn't
```

Or paste the function into [godbolt.org](https://godbolt.org) for a live,
color-mapped view of source → assembly. It's the fastest way to learn what the
optimizer actually does with your code.

---

<a id="portability"></a>
## 23. Portability and Platform Detection

### 23.1 What varies between platforms

| Thing | Varies how |
|---|---|
| `sizeof(long)` | 4 on Windows, 8 on Linux/macOS (64-bit) |
| `sizeof(int)` | 4 almost everywhere, 2 on some embedded |
| Plain `char` signedness | Signed on x86, unsigned on ARM |
| Endianness | Little on x86/ARM, big in network protocols |
| Path separator | `\` on Windows, `/` elsewhere (Windows accepts `/` too) |
| Line ending | `\r\n` on Windows, `\n` elsewhere |
| Struct padding | Depends on ABI |
| `>>` on negatives | Implementation-defined |
| Right-shift and division rounding | Standardized since C99 |

### 23.2 Detection macros

```c
/* Operating system */
#if defined(_WIN32)                 /* both 32- and 64-bit Windows */
  #define PLATFORM "Windows"
  #include <windows.h>
#elif defined(__APPLE__)
  #include <TargetConditionals.h>
  #define PLATFORM "macOS"
#elif defined(__linux__)
  #define PLATFORM "Linux"
  #include <unistd.h>
#elif defined(__unix__)
  #define PLATFORM "Unix"
#else
  #error "Unsupported platform"
#endif

/* Compiler */
#if defined(__clang__)
  #define COMPILER "clang"
#elif defined(__GNUC__)             /* check clang FIRST — clang also defines __GNUC__ */
  #define COMPILER "gcc"
#elif defined(_MSC_VER)
  #define COMPILER "msvc"
#endif

/* Language standard */
#if !defined(__STDC_VERSION__)
  #define C_STD "C89"
#elif __STDC_VERSION__ >= 202311L
  #define C_STD "C23"
#elif __STDC_VERSION__ >= 201710L
  #define C_STD "C17"
#elif __STDC_VERSION__ >= 201112L
  #define C_STD "C11"
#elif __STDC_VERSION__ >= 199901L
  #define C_STD "C99"
#endif

/* Architecture word size */
#if defined(__x86_64__) || defined(_M_X64) || defined(__aarch64__)
  #define ARCH_64 1
#endif
```

> ⚠️ **Check `__clang__` before `__GNUC__`.** Clang defines both for
> compatibility, so testing `__GNUC__` first misidentifies it.

### 23.3 Portable abstraction: isolate the difference

Don't scatter `#ifdef` through your logic. Wrap each platform difference in one
function and call that everywhere.

```c
/* platform.h */
#ifndef PLATFORM_H
#define PLATFORM_H

void platform_sleep_ms(unsigned ms);
void platform_clear_screen(void);
int  platform_mkdir(const char *path);

#endif
```

```c
/* platform.c — all the ugliness lives here, once */
#include "platform.h"

#ifdef _WIN32
  #include <windows.h>
  #include <direct.h>
  void platform_sleep_ms(unsigned ms)  { Sleep(ms); }
  void platform_clear_screen(void)     { system("cls"); }
  int  platform_mkdir(const char *p)   { return _mkdir(p); }
#else
  #include <unistd.h>
  #include <sys/stat.h>
  void platform_sleep_ms(unsigned ms)  { usleep(ms * 1000u); }
  void platform_clear_screen(void)     { system("clear"); }
  int  platform_mkdir(const char *p)   { return mkdir(p, 0755); }
#endif
```

Your application code now has zero `#ifdef`s and is trivially readable.

### 23.4 Text vs binary mode

On Windows, opening a file in text mode translates `\n` ↔ `\r\n`. That silently
corrupts binary data.

```c
FILE *f = fopen("data.bin", "rb");     // ✅ 'b' — mandatory for binary on Windows
FILE *t = fopen("notes.txt", "r");     // text mode is fine for text
```

The `b` is ignored on POSIX systems, so **always include it for binary files** —
it costs nothing and prevents a genuinely baffling class of bug.

### 23.5 Portable fixed-width types and printing

```c
#include <stdint.h>
#include <inttypes.h>

int64_t  count = 0;
uint32_t hash  = 0;

printf("count=%" PRId64 " hash=%" PRIu32 "\n", count, hash);
printf("size=%zu diff=%td\n", sizeof(int), ptr2 - ptr1);
```

`%lld` for `int64_t` works on most platforms but isn't guaranteed. The `PRI*`
macros always expand to the correct specifier.

The other macros follow the same pattern: `PRId32`/`PRIu32` for `int32_t`/`uint32_t`, `PRIx64` for hex output of a `uint64_t`, `PRIX64` for uppercase hex, and `SCNd64`-style macros for `scanf`. When in doubt, use the macro — it's one less portability landmine.

---

<a id="building-things"></a>
# Part IV — Building Things

Everything so far has been about *using* C correctly. This part is about *building* with it. Each section implements one real data structure, complete, with the ownership and error-handling discipline from Parts II and III.

These are not toy snippets. They are the actual shapes you will reuse.

---

<a id="vector"></a>
## 24. Dynamic Array (Vector)

A growable array. The single most-used data structure in C — it replaces the fixed `int arr[N]` that silently overflows.

### 24.1 The idea

Keep three things: a heap pointer, the number of elements in use, and the allocated capacity. When a push would exceed capacity, grow by doubling. Doubling makes each push O(1) *amortized* — the occasional expensive reallocation is paid for by many cheap ones.

### 24.2 Complete implementation

```c
/* vec.h */
#ifndef VEC_H
#define VEC_H

#include <stddef.h>
#include <stdbool.h>

typedef struct {
    int   *data;
    size_t len;
    size_t cap;
} Vec;

/* All functions return bool: true on success, false on allocation failure.
 * vec_free is safe on a zeroed or already-freed Vec. */
void vec_init(Vec *v);
bool vec_push(Vec *v, int x);
bool vec_pop(Vec *v, int *out);
int  vec_get(const Vec *v, size_t i);          /* caller must check i < v->len */
void vec_free(Vec *v);

#endif
```

```c
/* vec.c */
#include "vec.h"
#include <stdlib.h>

void vec_init(Vec *v) {
    v->data = NULL;
    v->len  = 0;
    v->cap  = 0;
}

bool vec_push(Vec *v, int x) {
    if (v->len == v->cap) {
        size_t new_cap = v->cap ? v->cap * 2 : 8;
        int *tmp = realloc(v->data, new_cap * sizeof *tmp);
        if (!tmp) return false;          /* v is unchanged, still valid */
        v->data = tmp;
        v->cap  = new_cap;
    }
    v->data[v->len++] = x;
    return true;
}

bool vec_pop(Vec *v, int *out) {
    if (v->len == 0) return false;
    if (out) *out = v->data[--v->len];
    return true;
}

int vec_get(const Vec *v, size_t i) {
    return v->data[i];          /* no bounds check — documented contract */
}

void vec_free(Vec *v) {
    free(v->data);
    v->data = NULL;
    v->len = v->cap = 0;
}
```

### 24.3 Usage and the important idioms

```c
#include <stdio.h>
#include "vec.h"

int main(void) {
    Vec v;
    vec_init(&v);

    for (int i = 0; i < 10; i++) {
        if (!vec_push(&v, i * i)) {
            fprintf(stderr, "out of memory\n");
            vec_free(&v);
            return 1;
        }
    }

    for (size_t i = 0; i < v.len; i++)
        printf("%d ", v.data[i]);
    putchar('\n');                       /* 0 1 4 9 16 25 36 49 64 81 */

    int last;
    while (vec_pop(&v, &last))
        printf("popped %d\n", last);

    vec_free(&v);                        /* safe even though already drained */
    return 0;
}
```

Three things to notice:

- **`realloc` into a temporary** (`tmp`), exactly the rule from section 8.4. If we did `v->data = realloc(v->data, ...)` and it failed, we'd leak the old block and corrupt the Vec.
- **Initial capacity of 0** with `v->cap ? v->cap * 2 : 8` avoids a special case for the first allocation and keeps `vec_init` trivial.
- **`vec_free` resets to zero state**, so a double-free of the Vec itself is harmless and a use-after-free is a clean NULL-deref rather than silent corruption.

> 🔑 The growth factor of 2 is the standard choice. 1.5 uses less memory; 2 is simpler and what most implementations do. Both are O(1) amortized. Never grow by a constant amount (`cap += 10`) — that makes pushing O(n) and your program quadratic.

---

<a id="linked-list"></a>
## 25. Linked List

A chain of heap nodes, each pointing to the next. You give up O(1) random access in exchange for O(1) insertion and removal at either end.

### 25.1 Singly linked list

```c
/* list.h */
#ifndef LIST_H
#define LIST_H

#include <stdbool.h>

typedef struct Node {
    int          value;
    struct Node *next;
} Node;

typedef struct {
    Node *head;
    Node *tail;              /* kept so push_back is O(1) */
} List;

void  list_init(List *l);
bool  list_push_front(List *l, int value);
bool  list_push_back(List *l, int value);
bool  list_pop_front(List *l, int *out);
Node *list_find(List *l, int value);
bool  list_remove(List *l, int value);   /* removes first match */
void  list_free(List *l);

#endif
```

```c
/* list.c */
#include "list.h"
#include <stdlib.h>

void list_init(List *l) {
    l->head = l->tail = NULL;
}

bool list_push_front(List *l, int value) {
    Node *n = malloc(sizeof *n);
    if (!n) return false;
    n->value = value;
    n->next  = l->head;
    l->head  = n;
    if (!l->tail) l->tail = n;      /* first element */
    return true;
}

bool list_push_back(List *l, int value) {
    Node *n = malloc(sizeof *n);
    if (!n) return false;
    n->value = value;
    n->next  = NULL;
    if (l->tail) l->tail->next = n;
    else         l->head       = n;  /* first element */
    l->tail = n;
    return true;
}

bool list_pop_front(List *l, int *out) {
    Node *n = l->head;
    if (!n) return false;
    if (out) *out = n->value;
    l->head = n->next;
    if (!l->head) l->tail = NULL;    /* list is now empty */
    free(n);
    return true;
}

Node *list_find(List *l, int value) {
    for (Node *n = l->head; n; n = n->next)
        if (n->value == value) return n;
    return NULL;
}

bool list_remove(List *l, int value) {
    Node **pp = &l->head;            /* pointer-to-pointer: the trick */
    while (*pp) {
        if ((*pp)->value == value) {
            Node *dead = *pp;
            *pp = dead->next;
            if (dead == l->tail) l->tail = NULL;  /* tail tracking is the fiddly part */
            free(dead);
            return true;
        }
        pp = &(*pp)->next;
    }
    return false;
}

void list_free(List *l) {
    Node *n = l->head;
    while (n) {
        Node *next = n->next;
        free(n);
        n = next;
    }
    l->head = l->tail = NULL;
}
```

### 25.2 The pointer-to-pointer trick

`list_remove` uses `Node **pp` — a pointer to the *link* that points at the current node, rather than a pointer to the node itself.

```
Before removing B:
    head ──► A ──► B ──► C ──► NULL
                  ▲
                  pp = &A->next   (points at the LINK holding B)

*pp = dead->next   rewrites A->next to skip B, without a special case
```

This removes the "if it's the head, handle differently" branch entirely. Linus Torvalds has called understanding this the mark of someone who actually gets pointers. It's worth drawing on paper until it clicks.

### 25.3 When to use a list vs a vector

| | Vector | Linked list |
|---|---|---|
| Random access `v[i]` | O(1) | O(n) |
| Push/pop at back | O(1) amortized | O(1) with tail pointer |
| Insert/remove in middle | O(n) (memmove) | O(1) once you have the node |
| Cache performance | Excellent (contiguous) | Poor (scattered nodes) |
| Memory per element | `sizeof(int)` | `sizeof(int)` + pointer + malloc overhead |

**Default to a vector.** The cache penalty of pointer-chasing is so severe on modern hardware that a vector often wins even for workloads with many middle insertions. Reach for a list when you need stable pointers to elements (a vector's elements move on realloc) or truly frequent middle insertion.

---

<a id="stack-queue"></a>
## 26. Stack and Queue

Both are just disciplined uses of a vector or list. The discipline is the point: restricting the interface prevents whole classes of bugs.

### 26.1 Stack (LIFO) on a vector

```c
#include <stdbool.h>
#include <stddef.h>

typedef struct {
    int   *data;
    size_t len, cap;
} Stack;

bool stack_push(Stack *s, int x) {
    if (s->len == s->cap) {
        size_t nc = s->cap ? s->cap * 2 : 8;
        int *tmp = realloc(s->data, nc * sizeof *tmp);
        if (!tmp) return false;
        s->data = tmp; s->cap = nc;
    }
    s->data[s->len++] = x;
    return true;
}

bool stack_pop(Stack *s, int *out) {
    if (!s->len) return false;
    if (out) *out = s->data[--s->len];
    return true;
}

bool stack_peek(const Stack *s, int *out) {
    if (!s->len) return false;
    if (out) *out = s->data[s->len - 1];
    return true;
}
```

The classic application: checking balanced brackets.

```c
bool brackets_balanced(const char *s) {
    Stack st = {0};
    bool ok = true;
    for (; *s && ok; s++) {
        switch (*s) {
            case '(': case '[': case '{':
                ok = stack_push(&st, *s); break;
            case ')': case ']': case '}': {
                int top;
                ok = stack_pop(&st, &top)
                     && ((*s == ')' && top == '(')
                      || (*s == ']' && top == '[')
                      || (*s == '}' && top == '{'));
                break;
            }
        }
    }
    ok = ok && st.len == 0;
    free(st.data);
    return ok;
}
```

### 26.2 Queue (FIFO) as a ring buffer

A ring buffer uses a fixed array with head and tail indices that wrap around. No allocation after creation, O(1) everything, no memory fragmentation — which is why it appears in every embedded system and audio pipeline.

```c
#include <stdbool.h>
#include <stddef.h>

typedef struct {
    int   *buf;
    size_t cap;          /* usable slots are cap - 1; one slot stays empty */
    size_t head;         /* index of the oldest element */
    size_t count;
} Queue;

bool queue_init(Queue *q, size_t capacity) {
    q->buf = malloc((capacity + 1) * sizeof *q->buf);
    if (!q->buf) return false;
    q->cap = capacity + 1;
    q->head = 0;
    q->count = 0;
    return true;
}

bool queue_push(Queue *q, int x) {
    if (q->count == q->cap - 1) return false;        /* full */
    size_t tail = (q->head + q->count) % q->cap;
    q->buf[tail] = x;
    q->count++;
    return true;
}

bool queue_pop(Queue *q, int *out) {
    if (!q->count) return false;                      /* empty */
    if (out) *out = q->buf[q->head];
    q->head = (q->head + 1) % q->cap;
    q->count--;
    return true;
}

void queue_free(Queue *q) {
    free(q->buf);
    q->buf = NULL; q->cap = q->head = q->count = 0;
}
```

The `% cap` wrap and the "one empty slot" convention (so full and empty are distinguishable) are the two details people get wrong. `count` disambiguates them here, which is simpler than comparing head and tail.

---

<a id="bst"></a>
## 27. Binary Search Tree

Each node has at most two children; everything in the left subtree is smaller, everything in the right is larger. Search, insert, and delete are all O(h) where h is the height — O(log n) if the tree stays balanced, degenerating to O(n) if you insert sorted data into a naive tree.

### 27.1 The node and recursive core

```c
#include <stdbool.h>
#include <stdlib.h>

typedef struct BstNode {
    int             key;
    struct BstNode *left, *right;
} BstNode;

typedef struct {
    BstNode *root;
} Bst;

static BstNode *node_new(int key) {
    BstNode *n = malloc(sizeof *n);
    if (n) { n->key = key; n->left = n->right = NULL; }
    return n;
}

/* Recursive insert. Returns the (possibly new) subtree root. */
static BstNode *insert_rec(BstNode *n, int key, bool *inserted) {
    if (!n) { *inserted = true; return node_new(key); }
    if (key < n->key)      n->left  = insert_rec(n->left,  key, inserted);
    else if (key > n->key) n->right = insert_rec(n->right, key, inserted);
    /* key == n->key: already present, do nothing */
    return n;
}

bool bst_insert(Bst *t, int key) {
    bool inserted = false;
    BstNode *r = insert_rec(t->root, key, &inserted);
    if (inserted && !t->root) t->root = r;
    return inserted;
}

bool bst_contains(const Bst *t, int key) {
    const BstNode *n = t->root;
    while (n) {
        if (key < n->key)      n = n->left;
        else if (key > n->key) n = n->right;
        else                   return true;
    }
    return false;
}
```

`contains` is iterative (no reason to recurse for a straight walk down); `insert` is recursive because threading the "possibly new subtree root" back up is cleaner than the iterative pointer-to-pointer version for a first implementation.

### 27.2 Traversals

```c
#include <stdio.h>

static void inorder_rec(const BstNode *n) {
    if (!n) return;
    inorder_rec(n->left);
    printf("%d ", n->key);        /* in-order prints SORTED output */
    inorder_rec(n->right);
}

void bst_print_sorted(const Bst *t) {
    inorder_rec(t->root);
    putchar('\n');
}
```

| Traversal | Order | Use |
|---|---|---|
| In-order | left, node, right | Sorted output (BST property) |
| Pre-order | node, left, right | Copying / serializing a tree |
| Post-order | left, right, node | Freeing a tree (children before parent) |

### 27.3 Freeing with post-order

```c
static void free_rec(BstNode *n) {
    if (!n) return;
    free_rec(n->left);
    free_rec(n->right);
    free(n);                 /* free the parent only after both children */
}

void bst_free(Bst *t) {
    free_rec(t->root);
    t->root = NULL;
}
```

> ⚠️ Naive BSTs degenerate into a linked list if you insert already-sorted data, and deep recursion can blow the stack. Production code uses a self-balancing tree (AVL, red-black) or a hash table. For learning and moderate key counts, the plain BST above is honest and instructive.

---

<a id="hashtable"></a>
## 28. Hash Table

The workhorse. Maps a key to a value in O(1) average time by hashing the key to a bucket index.

### 28.1 The design

- An array of `cap` buckets.
- Each bucket is the head of a short chain of entries that collided (separate chaining).
- When `count / cap` exceeds a load factor (0.75), grow the table and rehash.

### 28.2 String keys, int values — complete

```c
/* ht.h */
#ifndef HT_H
#define HT_H

#include <stddef.h>
#include <stdbool.h>

typedef struct HtEntry {
    char           *key;
    int             value;
    struct HtEntry *next;
} HtEntry;

typedef struct {
    HtEntry **buckets;
    size_t    cap;
    size_t    count;
} Ht;

bool ht_init(Ht *t, size_t initial_cap);
bool ht_set(Ht *t, const char *key, int value);  /* insert or overwrite */
bool ht_get(const Ht *t, const char *key, int *out);
bool ht_remove(Ht *t, const char *key);
void ht_free(Ht *t);

#endif
```

```c
/* ht.c */
#include "ht.h"
#include <stdlib.h>
#include <string.h>

/* FNV-1a: simple, fast, good enough for a learning hash table. */
static size_t hash_str(const char *s) {
    size_t h = 1469598103934665603ULL;      /* FNV offset basis (64-bit) */
    while (*s) {
        h ^= (unsigned char)*s++;
        h *= 1099511628211ULL;              /* FNV prime */
    }
    return h;
}

static char *dup_str(const char *s) {
    size_t n = strlen(s) + 1;
    char *copy = malloc(n);
    if (copy) memcpy(copy, s, n);
    return copy;
}

bool ht_init(Ht *t, size_t initial_cap) {
    if (initial_cap < 8) initial_cap = 8;
    t->buckets = calloc(initial_cap, sizeof *t->buckets);
    if (!t->buckets) return false;
    t->cap   = initial_cap;
    t->count = 0;
    return true;
}

bool ht_get(const Ht *t, const char *key, int *out) {
    size_t i = hash_str(key) % t->cap;
    for (HtEntry *e = t->buckets[i]; e; e = e->next) {
        if (strcmp(e->key, key) == 0) {
            if (out) *out = e->value;
            return true;
        }
    }
    return false;
}

static bool grow(Ht *t) {
    size_t new_cap = t->cap * 2;
    HtEntry **nb = calloc(new_cap, sizeof *nb);
    if (!nb) return false;

    /* Rehash every entry into the new bucket array. */
    for (size_t i = 0; i < t->cap; i++) {
        HtEntry *e = t->buckets[i];
        while (e) {
            HtEntry *next = e->next;
            size_t j = hash_str(e->key) % new_cap;
            e->next = nb[j];
            nb[j]   = e;
            e = next;
        }
    }
    free(t->buckets);
    t->buckets = nb;
    t->cap     = new_cap;
    return true;
}

bool ht_set(Ht *t, const char *key, int value) {
    if ((t->count + 1) * 4 > t->cap * 3) {          /* load factor > 0.75 */
        if (!grow(t)) return false;                  /* keep going on failure? no — report */
    }

    size_t i = hash_str(key) % t->cap;
    for (HtEntry *e = t->buckets[i]; e; e = e->next) {
        if (strcmp(e->key, key) == 0) {              /* overwrite existing */
            e->value = value;
            return true;
        }
    }

    HtEntry *e = malloc(sizeof *e);
    if (!e) return false;
    e->key = dup_str(key);                           /* table OWNS its copy */
    if (!e->key) { free(e); return false; }
    e->value = value;
    e->next  = t->buckets[i];
    t->buckets[i] = e;
    t->count++;
    return true;
}

bool ht_remove(Ht *t, const char *key) {
    size_t i = hash_str(key) % t->cap;
    HtEntry **pp = &t->buckets[i];                   /* pointer-to-pointer again */
    while (*pp) {
        if (strcmp((*pp)->key, key) == 0) {
            HtEntry *dead = *pp;
            *pp = dead->next;
            free(dead->key);
            free(dead);
            t->count--;
            return true;
        }
        pp = &(*pp)->next;
    }
    return false;
}

void ht_free(Ht *t) {
    for (size_t i = 0; i < t->cap; i++) {
        HtEntry *e = t->buckets[i];
        while (e) {
            HtEntry *next = e->next;
            free(e->key);
            free(e);
            e = next;
        }
    }
    free(t->buckets);
    t->buckets = NULL;
    t->cap = t->count = 0;
}
```

### 28.3 Usage

```c
#include <stdio.h>
#include "ht.h"

int main(void) {
    Ht t;
    if (!ht_init(&t, 16)) return 1;

    ht_set(&t, "alice", 30);
    ht_set(&t, "bob",   25);
    ht_set(&t, "alice", 31);           /* overwrite */

    int age;
    if (ht_get(&t, "alice", &age)) printf("alice = %d\n", age);   /* 31 */
    if (!ht_get(&t, "carol", &age)) printf("carol not found\n");

    ht_remove(&t, "bob");
    ht_free(&t);
    return 0;
}
```

Note the ownership rule: the table **duplicates** every key it stores and frees those copies in `ht_free`/`ht_remove`. The caller's key strings are never kept by reference. That single decision removes the entire category of "the table outlived the string" bugs.

---

<a id="generics"></a>
## 29. Generic Code with `void *` and Function Pointers

C has no templates. You get genericity through type erasure: store `void *` and hand the container a few function pointers telling it how to compare, copy, and destroy your type.

### 29.1 A generic vector

```c
#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    void  *data;
    size_t len, cap, elem_size;
} GVec;

bool gvec_init(GVec *v, size_t elem_size) {
    v->data = NULL;
    v->len = v->cap = 0;
    v->elem_size = elem_size;
    return true;
}

void *gvec_at(GVec *v, size_t i) {
    return (char *)v->data + i * v->elem_size;   /* byte arithmetic on void* */
}

bool gvec_push(GVec *v, const void *elem) {
    if (v->len == v->cap) {
        size_t nc = v->cap ? v->cap * 2 : 8;
        void *tmp = realloc(v->data, nc * v->elem_size);
        if (!tmp) return false;
        v->data = tmp; v->cap = nc;
    }
    memcpy(gvec_at(v, v->len), elem, v->elem_size);
    v->len++;
    return true;
}

void gvec_free(GVec *v) {
    free(v->data);
    v->data = NULL; v->len = v->cap = 0;
}
```

Usage with any type:

```c
#include <stdio.h>

int main(void) {
    GVec v;
    gvec_init(&v, sizeof(double));

    for (double x = 0.5; x < 4.0; x *= 2)
        gvec_push(&v, &x);

    for (size_t i = 0; i < v.len; i++)
        printf("%.1f ", *(double *)gvec_at(&v, i));
    putchar('\n');                       /* 0.5 1.0 2.0 */

    gvec_free(&v);
    return 0;
}
```

### 29.2 Generic sort with a comparator

`qsort` from the standard library is exactly this pattern. The comparator contract: return negative if `a < b`, zero if equal, positive if `a > b`.

```c
#include <stdio.h>
#include <stdlib.h>

static int cmp_int_asc(const void *a, const void *b) {
    int x = *(const int *)a, y = *(const int *)b;
    return (x > y) - (x < y);            /* avoids overflow of x - y */
}

static int cmp_str(const void *a, const void *b) {
    return strcmp(*(const char *const *)a, *(const char *const *)b);
}

int main(void) {
    int nums[] = {5, 2, 8, 1, 9};
    size_t n = sizeof nums / sizeof nums[0];
    qsort(nums, n, sizeof *nums, cmp_int_asc);

    const char *names[] = {"carol", "alice", "bob"};
    size_t m = sizeof names / sizeof names[0];
    qsort(names, m, sizeof *names, cmp_str);

    for (size_t i = 0; i < n; i++) printf("%d ", nums[i]);
    putchar('\n');
    for (size_t i = 0; i < m; i++) printf("%s ", names[i]);
    putchar('\n');
    return 0;
}
```

The `(x > y) - (x < y)` idiom in `cmp_int_asc` is worth memorizing. The naive `return x - y;` overflows when `x` is very negative and `y` very positive.

For `bsearch` (binary search on a sorted array) the comparator signature is identical.

### 29.3 Callbacks with a context pointer

Real C APIs pass a `void *ctx` through to your callback so it can carry state without globals. This is the pattern behind `qsort_r`, GUI event handlers, and iterator-with-closure designs.

```c
typedef void (*VisitFn)(void *ctx, int value);

void vec_for_each(const GVec *v, VisitFn fn, void *ctx) {
    for (size_t i = 0; i < v->len; i++)
        fn(ctx, *(const int *)gvec_at((GVec *)v, i));
}

/* ---- caller side ---- */
#include <stdio.h>

static void print_doubled(void *ctx, int value) {
    int factor = *(int *)ctx;
    printf("%d ", value * factor);
}

int main(void) {
    GVec v; gvec_init(&v, sizeof(int));
    for (int i = 1; i <= 5; i++) gvec_push(&v, &i);

    int factor = 10;
    vec_for_each(&v, print_doubled, &factor);   /* 10 20 30 40 50 */

    gvec_free(&v);
    return 0;
}
```

---

<a id="api-design"></a>
## 30. API Design: Opaque Types and Modules

The difference between a C library people can use and one they can't is almost entirely the header. A few deliberate choices make a module robust.

### 30.1 The opaque type

Don't expose your struct in the header. Expose only a forward declaration and functions. Callers can hold pointers but can't see inside, so you're free to change the implementation without breaking anyone.

```c
/* stack.h — the entire public contract */
#ifndef STACK_H
#define STACK_H

#include <stdbool.h>
#include <stddef.h>

typedef struct StackImpl Stack;   /* incomplete type: callers can't sizeof it */

Stack *stack_create(void);
void   stack_destroy(Stack *s);   /* always the mirror of create */

bool   stack_push(Stack *s, int x);
bool   stack_pop(Stack *s, int *out);
size_t stack_size(const Stack *s);

#endif
```

```c
/* stack.c — the definition is private */
#include "stack.h"
#include <stdlib.h>

struct StackImpl {
    int   *data;
    size_t len, cap;
};

Stack *stack_create(void) {
    Stack *s = calloc(1, sizeof *s);   /* zeroed: data=NULL, len=cap=0 */
    return s;
}

void stack_destroy(Stack *s) {
    if (!s) return;
    free(s->data);
    free(s);
}

/* ... push/pop/size as in section 26, operating on s-> ... */
```

A caller writes `Stack *s = stack_create();` and can never write `s->len` — the compiler rejects it because `struct StackImpl` is incomplete in their translation unit.

### 30.2 The conventions that make an API feel solid

| Convention | Why |
|---|---|
| One prefix per module (`stack_`, `ht_`) | C has no namespaces; the prefix is it |
| `create`/`destroy` come in pairs | Obvious ownership; ASan verifies the pairing |
| `destroy(NULL)` is safe | Mirrors `free(NULL)`; removes caller checks |
| `const` on every read-only pointer param | Self-documenting, enables optimization |
| Return `bool`/`Status`, output via pointer params | Uniform error handling |
| Document who owns what, in the header | The whole contract, visible at the call site |

### 30.3 Header hygiene checklist

- Include guard or `#pragma once`.
- Includes only what the *declarations* need (not what the .c file needs).
- No `using`-style typedef pollution beyond your prefix.
- No non-`static` global variables declared (only `extern` declarations).
- Compiles standalone: `#include "stack.h"` as the *first* include in `stack.c` proves the header is self-sufficient.

That last one is a real test. If your header needs `<stddef.h>` for `size_t` but doesn't include it, every consumer must include it before yours — and they'll only discover this by a compile error in their project, not yours.

---

<a id="recursion"></a>
# Part V — Advanced and Modern

## 31. Recursion, and When to Kill It

### 31.1 Recursion that earns its keep

Recursion is the natural expression of anything with self-similar structure: trees, directories, divide-and-conquer algorithms, parsers.

```c
#include <stdio.h>

long fib(int n) {
    if (n < 2) return n;               /* base case */
    return fib(n - 1) + fib(n - 2);    /* recursive case */
}
```

Every recursive function needs both a base case and progress toward it. Miss either and you get a stack overflow, not a clean error.

The three shapes that matter:

```c
/* Linear — one call per level */
int sum(const int *a, size_t n) {
    if (n == 0) return 0;
    return a[0] + sum(a + 1, n - 1);
}

/* Divide and conquer — two calls on halves. The good kind. */
int max_of(const int *a, size_t n) {
    if (n == 1) return a[0];
    size_t mid = n / 2;
    int l = max_of(a, mid);
    int r = max_of(a + mid, n - mid);
    return l > r ? l : r;
}

/* Tree recursion — structure mirrors the data */
static void walk(const BstNode *n) {
    if (!n) return;
    walk(n->left);
    visit(n);
    walk(n->right);
}
```

### 31.2 When recursion is the wrong tool

`fib` above is the canonical bad example: it recomputes the same values exponentially, and a deep call chain risks the stack.

| Problem | Recursive cost | Better |
|---|---|---|
| Fibonacci (naive) | O(2^n) time | Iterative loop or memoization |
| Walking a 1M-node linked list | 1M stack frames → overflow | Iterative `while` |
| Tree walk, depth unknown | Depth frames — fine for balanced trees | Iterative with explicit stack |

### 31.3 Converting recursion to iteration with an explicit stack

When the structure is recursive but the depth is unbounded, carry your own stack on the heap. Here's `bst_free` done iteratively, using a simple pointer-stack:

```c
#include <stdlib.h>

void bst_free_iterative(Bst *t) {
    /* A growable stack of node pointers. */
    size_t cap = 64, len = 0;
    BstNode **stack = malloc(cap * sizeof *stack);
    if (!stack) return;

    if (t->root) stack[len++] = t->root;

    while (len > 0) {
        BstNode *n = stack[--len];           /* pop */
        if (n->left) {
            if (len == cap) { cap *= 2; stack = realloc(stack, cap * sizeof *stack); }
            stack[len++] = n->left;          /* push */
        }
        if (n->right) {
            if (len == cap) { cap *= 2; stack = realloc(stack, cap * sizeof *stack); }
            stack[len++] = n->right;         /* push */
        }
        free(n);
    }

    free(stack);
    t->root = NULL;
}
```

(The `realloc`-into-a-temporary guard from section 8.4 is elided above only for readability — production code should check it.)

The pattern: your call stack becomes a heap data structure you control, so depth is limited by RAM, not the 1–8 MB thread stack.

### 31.4 Tail calls

A call is a *tail call* if it's the very last thing the function does. Some compilers optimize it into a jump, reusing the frame.

```c
/* Not tail — the + happens AFTER the recursive call returns */
long factorial_rec(long n) { return n <= 1 ? 1 : n * factorial_rec(n - 1); }

/* Tail — the recursive call is the final act; accumulator carries state */
static long fact_go(long n, long acc) { return n <= 1 ? acc : fact_go(n - 1, n * acc); }
long factorial(long n) { return fact_go(n, 1); }
```

> ⚠️ C does **not guarantee** tail-call optimization. GCC/Clang do it at `-O2` for simple cases; MSVC mostly doesn't. Never rely on it for correctness — if depth matters, use an explicit stack.

---

<a id="concurrency"></a>
## 32. Concurrency: Threads, Races, Mutexes, Atomics

C11 gave the language a real, portable memory model and threading library in `<threads.h>` and `<stdatomic.h>`. This is the deepest water in the document; go slowly.

### 32.1 Spawning a thread

```c
#include <stdio.h>
#include <threads.h>

static int worker(void *arg) {
    const char *name = arg;
    for (int i = 0; i < 3; i++)
        printf("%s: %d\n", name, i);
    return 0;
}

int main(void) {
    thrd_t t1, t2;
    thrd_create(&t1, worker, "alpha");
    thrd_create(&t2, worker, "beta");
    thrd_join(t1, NULL);
    thrd_join(t2, NULL);
    return 0;
}
```

`thrd_join` blocks until the thread finishes. Without it, `main` could return and the process exit while workers are still running.

### 32.2 The data race — the one bug that defines concurrent C

Two threads touch the same memory, at least one writes, and there's no synchronization. That is **undefined behavior** — not "wrong value," but *undefined*.

```c
#include <stdio.h>
#include <threads.h>

static long counter = 0;              /* shared, unsynchronized — the bug */

static int worker(void *arg) {
    (void)arg;
    for (long i = 0; i < 1000000; i++)
        counter++;                    /* read-modify-write, NOT atomic */
    return 0;
}

int main(void) {
    enum { N = 4 };
    thrd_t t[N];
    for (int i = 0; i < N; i++) thrd_create(&t[i], worker, NULL);
    for (int i = 0; i < N; i++) thrd_join(t[i], NULL);
    printf("expected %d, got %ld\n", N * 1000000, counter);   /* always less */
    return 0;
}
```

`counter++` compiles to load-increment-store. Two threads interleave and increments get lost. The result is a number below 4,000,000 that changes every run.

### 32.3 Fixing it with a mutex

A mutex serializes access: only one thread holds it at a time.

```c
#include <threads.h>

static long   counter = 0;
static mtx_t  lock;

static int worker(void *arg) {
    (void)arg;
    for (long i = 0; i < 1000000; i++) {
        mtx_lock(&lock);
        counter++;
        mtx_unlock(&lock);
    }
    return 0;
}

int main(void) {
    enum { N = 4 };
    thrd_t t[N];
    mtx_init(&lock, mtx_plain);
    for (int i = 0; i < N; i++) thrd_create(&t[i], worker, NULL);
    for (int i = 0; i < N; i++) thrd_join(t[i], NULL);
    mtx_destroy(&lock);
    printf("%ld\n", counter);          /* exactly 4000000 */
    return 0;
}
```

> 🔑 **Every shared, mutable variable needs a synchronization story.** The mutex must guard *every* access, read or write, or the race is still there. Locking the writes but not the reads is a classic half-fix that still has UB.

### 32.4 Fixing it with atomics

When the shared state is a single scalar, an atomic is simpler and far faster than a mutex.

```c
#include <stdatomic.h>
#include <threads.h>
#include <stdio.h>

static atomic_long counter = 0;

static int worker(void *arg) {
    (void)arg;
    for (long i = 0; i < 1000000; i++)
        atomic_fetch_add(&counter, 1);      /* indivisible */
    return 0;
}

int main(void) {
    enum { N = 4 };
    thrd_t t[N];
    for (int i = 0; i < N; i++) thrd_create(&t[i], worker, NULL);
    for (int i = 0; i < N; i++) thrd_join(t[i], NULL);
    printf("%ld\n", atomic_load(&counter));  /* exactly 4000000 */
    return 0;
}
```

`atomic_fetch_add` is a single hardware instruction. No lock, no interleaving.

### 32.5 The rules that keep you alive

1. **Share as little as possible.** Threads that don't share mutable state can't race. Prefer passing results through `thrd_join`'s return or message queues.
2. **One lock per logical invariant, not per variable.** If `a` and `b` must stay consistent with each other, one mutex guards both.
3. **Hold locks for the shortest time.** Don't do I/O or call unknown code while holding a lock.
4. **Lock ordering prevents deadlock.** If two locks are ever held together, always acquire them in the same global order everywhere.
5. **`volatile` is not synchronization.** It only forces reloads; it provides no atomicity or ordering. Use `_Atomic` / `<stdatomic.h>`.
6. **Condition variables for waiting.** When a thread must wait for a condition (a queue becoming non-empty), use `cnd_wait`/`cnd_signal` from `<threads.h>` — don't spin in a loop checking a flag.

### 32.6 Detecting races

GCC/Clang's ThreadSanitizer instruments memory accesses and reports races precisely:

```bash
gcc -fsanitize=thread -g main.c -o main -pthread
./main
```

Note TSan conflicts with ASan — build separate sanitizer binaries. A clean TSan run on a workload that exercises all threads is strong evidence of race-freedom; it is not proof.

---

<a id="standards"></a>
## 33. C89 → C23: What Changed and What to Use

C evolves slowly and deliberately. Know the landmarks so you can read old code and write modern code deliberately.

| Standard | Year | Landmarks |
|---|---|---|
| **C89/C90** | 1989 | The original ANSI standard. Declarations at block start. No `//` comments. No `long long`, no `stdint.h`, no `bool`. |
| **C99** | 1999 | `//` comments, mixed declarations and code, `long long`, `<stdint.h>`, `<stdbool.h>`, designated initializers, VLAs, `restrict`, `//`-style, compound literals. |
| **C11** | 2011 | `_Static_assert`, `<threads.h>`, `<stdatomic.h>`, `_Generic`, anonymous structs/unions, `_Alignas`/`alignof`. Made VLAs optional. |
| **C17/C18** | 2018 | Bug-fix release. No new features. The safe modern baseline. |
| **C23** | 2024 | `bool`/`true`/`false` as keywords, `nullptr`, `0b` literals, `%b` in printf, `<stdbit.h>`, `<stdckdint.h>`, `static_assert` without underscore, `[[attributes]]`. |

### 33.1 What to actually use

**Write C17 as your baseline** (`-std=c17`). It's universally supported by current GCC, Clang, and MSVC, and gives you everything from C99 and C11.

Reach for specific newer features when your compiler supports them:

- **`_Static_assert`** (C11) for compile-time checks. Zero cost, high value.
- **Atomics and `<threads.h>`** (C11) when you genuinely need threads.
- **`_Generic`** (C11) for type-generic macros — but sparingly; it can obscure more than it clarifies.
- **Designated initializers** (C99) always. `.field = value` is clearer and more robust to struct reordering than positional.

Features to treat with caution:

- **VLAs** (C99, optional since C11) — unsupported by MSVC; avoid for portable code.
- **Anonymous unions in structs** (C11) — handy for tagged unions, but check MSVC version support.

### 33.2 Checking the standard at compile time

```c
#if !defined(__STDC_VERSION__) || __STDC_VERSION__ < 201112L
  #error "This code requires C11 or later"
#endif
```

---

<a id="style"></a>
## 34. Style Guide

Style is the set of decisions you make once so you never have to think about them again. This is a coherent, defensible set — adapt it, but be consistent.

### 34.1 Naming

| Thing | Convention | Example |
|---|---|---|
| Functions, variables | `snake_case` | `vec_push`, `line_count` |
| Struct/enum types | `PascalCase` | `Vec`, `HtEntry` |
| Macros, enum constants | `SCREAMING_SNAKE` | `MAX_SIZE`, `ERR_IO` |
| Module prefix | `module_` | `ht_set`, `stack_pop` |
| Boolean-ish | `is_`, `has_`, `can_` | `is_valid`, `has_next` |

### 34.2 Formatting

- 4-space indent, no tabs (tabs render differently everywhere).
- Brace on the same line for functions and control blocks (K&R), or Allman if your codebase already uses it. **Consistency beats preference.**
- Pointer `*` binds to the variable, not the type: `int *p`, not `int* p`. Reason: `int* p, q;` declares `q` as a plain `int` — the C grammar attaches `*` to the declarator.

### 34.3 Structure

- One logical module per `.c`/`.h` pair.
- Functions under ~40 lines where possible; extract helpers when they grow.
- Declare variables at first use, in the tightest scope (C99 allows it; it reduces the window for bugs).
- Initialize at declaration. `int x = 0;` not `int x;`.
- Early returns for error cases; the happy path stays un-indented.

### 34.4 Comments

- Comment **why**, not **what**. The code says what.
- Document the contract at the function declaration in the header: params, return, ownership, error cases.
- `/* TODO: ... */` and `/* FIXME: ... */` markers so they're greppable.

```c
/* Binary-search a sorted array.
 * Returns the index of `key`, or -1 if absent.
 * `arr` must be sorted ascending and contain `n` elements. */
long bin_search(const int *arr, size_t n, int key);
```

### 34.5 What a linter-friendly file looks like

Compiles clean under `-std=c17 -Wall -Wextra -Wpedantic -Werror` with no warnings, no casts unless justified in a comment, no unused parameters (mark deliberate ones `(void)param;`).

---

<a id="roadmap"></a>
# Part VI — Reference

## 35. Learning Roadmap with Projects

A path from zero to writing C you're not ashamed of. Each project exercises specific sections.

### Stage 1 — Foundations (sections 1–6)
**Project: a unit-converter CLI.** Parse `argv`, convert km↔mi, kg↔lb. Uses: toolchain, flags, multi-file build, argc/argv, `strtol`.

### Stage 2 — Core language (7–13)
**Project: a word-frequency counter.** Read a file, count words, print sorted by frequency. Uses: pointers, arrays, structs, strings, `qsort` with a comparator, integer care.

### Stage 3 — Correctness (14–16)
**Project: a tiny expression evaluator** (`3 + 4 * 2`). Uses: floats, parsing, UB discipline — run under ASan/UBSan from day one.

### Stage 4 — Engineering (17–23)
**Project: a JSON-ish config parser** with full tests, a Makefile, and valgrind-clean output. Uses: testing, error handling, cleanup ladders, portability.

### Stage 5 — Data structures (24–30)
**Project: a spell-checker.** Load a dictionary into a hash table, check a document, suggest corrections. Uses: hash table, dynamic array, generic code, opaque API.

### Stage 6 — Advanced (31–33)
**Project: a multi-threaded file indexer.** Worker threads hash files concurrently with a shared atomic counter and a mutex-protected results table. Uses: recursion, threads, atomics, mutexes, TSan validation.

---

<a id="cheatsheets"></a>
## 36. Cheat Sheets

### Format specifiers

| Spec | Type | Spec | Type |
|---|---|---|---|
| `%d` | `int` | `%u` | `unsigned int` |
| `%ld` / `%lu` | `long` / `unsigned long` | `%lld` / `%llu` | `long long` / `unsigned long long` |
| `%zu` | `size_t` | `%td` | `ptrdiff_t` |
| `%f` | `double` (and promoted `float`) | `%e` / `%g` | scientific / shortest |
| `%c` | `int` (a char) | `%s` | `char *` (string) |
| `%p` | `void *` (address) | `%x` / `%o` | hex / octal |
| `%%` | a literal `%` | `%b` | binary (C23) |

### Operator precedence (high → low, the ones that bite)

| Level | Operators | Gotcha |
|---|---|---|
| 1 | `()` `[]` `->` `.` `!` `~` `++` `--` `*` (deref) `&` (addr) `sizeof` | unary binds tight |
| 2 | `*` `/` `%` | |
| 3 | `+` `-` | |
| 4 | `<<` `>>` | **`1 << 2 + 1` is `1 << 3`, not `(1<<2)+1`** |
| 5 | `<` `<=` `>` `>=` | |
| 6 | `==` `!=` | **`a & b == c` is `a & (b == c)`** — always parenthesize `&` in tests |
| 7 | `&` `^` `\|` | |
| 8 | `&&` `\|\|` | short-circuit |
| 9 | `?:` `=` `+=` etc. | assignment binds loosest |

**When in doubt, parenthesize.** It costs nothing and kills the entire class of precedence bugs.

### The allocation pattern

```c
T *p = malloc(count * sizeof *p);       /* or calloc(count, sizeof *p) */
if (!p) { /* handle */ }
/* ... use ... */
free(p);
p = NULL;
```

### The four questions to ask of any pointer

1. Does it point at valid memory right now?
2. Who owns it — who is responsible for `free`?
3. How long does the pointee live?
4. Can it be NULL here?

---

<a id="glossary"></a>
## 37. Glossary

**ABI** — Application Binary Interface: how compiled code lays out structs, passes arguments, and names symbols. Why a library compiled with one compiler may not link with another's output.

**Amortized O(1)** — an operation that is occasionally expensive (a vector's reallocation) but cheap *on average* over many operations.

**Translation unit** — one `.c` file plus everything it `#include`s, after preprocessing. The unit the compiler sees.

**Undefined behavior** — code for which the C standard imposes *no* requirements. The compiler may assume it never happens. See section 16.

**Unspecified behavior** — the standard allows several outcomes; the compiler picks one and needn't document it (e.g. argument evaluation order).

**Implementation-defined behavior** — the standard allows several outcomes and the compiler *must* document its choice (e.g. `sizeof(int)`).

**Dangling pointer** — a pointer whose target has been freed or gone out of scope.

**Opaque type** — a struct whose definition is hidden from callers (only a forward declaration in the header), forcing access through functions.

**Strict aliasing** — the rule that an object may only be accessed through a pointer of a compatible type (with narrow exceptions like `char *`). Violating it is UB.

**Sequence point / sequencing** — the ordering guarantees between side effects. Unsequenced modification and read of the same variable is UB.

**Type punning** — reinterpreting the bytes of one type as another. The legal way is `memcpy`; the pointer-cast way violates strict aliasing.

**Load factor** — in a hash table, entries ÷ buckets. Kept below ~0.75 by growing.

**Memory model** — the rules governing how threads observe each other's memory accesses. C11 formalized C's.

**Sentinel** — a special value that marks a boundary or an error, like `'\0'` ending a string or `NULL` from `malloc`.

**CSPRNG** — Cryptographically Secure Pseudo-Random Number Generator. The OS-provided source you must use for security tokens, unlike `rand()`.

---

*End of the manual. If you can build every project in the roadmap clean under `-Wall -Wextra -Werror` with sanitizers on, you write better C than most people who do it for a living.*

# 🧱 The Complete C Variables, Types & Control Flow Reference

> A deep-dive guide to **every kind of variable** in C — basic, derived and user-defined — plus **all conditionals and loops**, from absolute-beginner, leveling up to advanced.
>
> Companion file: [`C_Standard_Headers.md`](C_Standard_Headers.md) covers the standard library. This file covers the *language itself*.
>
> **New to C?** Read sections 1–3 fully, then jump to [conditionals](#conditionals) and [loops](#loops). Ignore pointers-to-pointers, function pointers and bitfields until you finish arrays.
>
> Compile & run quickstart is at the [end](#compile-run).

---

## Table of Contents

**Part I — Variables & Types**
1. [What is a variable?](#what-is-a-variable)
2. [Basic (built-in) types](#basic-types)
3. [Type modifiers & qualifiers](#modifiers-qualifiers)
4. [Conversions, casts & promotion](#conversions)
5. [Constants: `const`, `#define`, enum constants](#constants)
6. [Storage classes: `auto`, `static`, `extern`, `register`, `thread_local`](#storage-classes)
7. [Scope, lifetime & linkage](#scope-lifetime)

**Part II — Derived Types**
8. [Arrays](#arrays)
9. [Strings as char arrays](#strings)
10. [Pointers](#pointers)
11. [Pointer ↔ array relationship](#ptr-array)
12. [Functions as a "derived type"](#functions)
13. [Advanced pointer territory](#advanced-ptrs)

**Part III — User-Defined Types**
14. [`struct` — structures](#structs)
15. [`union` — one thing at a time](#unions)
16. [`enum` — named integers](#enums)
17. [`typedef` — naming types](#typedef)
18. [Bitfields](#bitfields)
19. [Putting it together: linked list mini-example](#capstone-type)

**Part IV — Conditionals**
20. [Operators you need first](#operators)
21. [`if` / `else if` / `else`](#if-else)
22. [The ternary operator `?:`](#ternary)
23. [`switch` / `case` / `default`](#switch)
24. [Truthiness, short-circuiting & common bugs](#truthiness)

**Part V — Loops**
25. [`while`](#while-loop)
26. [`do...while`](#do-while)
27. [`for`](#for-loop)
28. [`break`, `continue`, `goto`](#jump-statements)
29. [Nested loops & patterns](#nested-loops)
30. [Loop design patterns & pitfalls](#loop-patterns)

**Part VI — Mastery**
31. [Advanced: `volatile`, `_Atomic`, flexible array members](#advanced-misc)
32. [Learning roadmap](#roadmap)
33. [Practice playground — mini projects per topic](#practice)
34. [Compile & run quickstart (Windows)](#compile-run)

---

<a id="what-is-a-variable"></a>
## 1. What Is a Variable?
A variable is a **named box in memory** with three properties:

| Property | Question it answers | Example |
|---|---|---|
| **Name** | How do I refer to it? | `age` |
| **Type** | How big is the box? How do I interpret the bits? | `int` |
| **Value** | What's inside right now? | `21` |

```c
int age;            // declaration: reserve a box called 'age' for an int
age = 21;           // assignment: put a value in it

int score = 95;     // declaration + initialization in one step ✅ preferred
```

⚠️ **Bug #4 from every beginner's career:** an uninitialized local variable contains **garbage** (whatever bytes were already there). Always initialize:

```c
int total;          // ❌ garbage value!
int total = 0;      // ✅
```

### Naming rules

- Letters, digits, underscore; can't start with a digit (`score2` ✓, `2score` ✗).
- Case-sensitive: `Total`, `total`, `TOTAL` are three different variables.
- Can't use keywords (`int`, `return`, `for`, …).
- Convention: `snake_case` for variables/functions, `UPPER_CASE` for constants,
  `PascalCase` or `CamelCase` for structs/enums — pick one style and stay consistent.

---

<a id="basic-types"></a>
## 2. Basic (Built-in) Types

C has only a handful of truly basic types; everything else is built from them.

| Type | Typical size | Typical range | Print with |
|---|---|---|---|
| `char` | 1 byte | −128..127 (or 0..255) | `%c` (char), `%d` (code) |
| `signed char` | 1 byte | −128..127 | `%d` |
| `unsigned char` | 1 byte | 0..255 | `%u` / `%d` |
| `short` | 2 bytes | ±32,767 | `%hd` |
| `unsigned short` | 2 bytes | 0..65,535 | `%hu` |
| `int` | 4 bytes | ≈ ±2.1 billion | `%d` |
| `unsigned int` | 4 bytes | 0..≈4.29 billion | `%u` |
| `long` | 4 (Win) / 8 (Linux) | see `<limits.h>` | `%ld` |
| `unsigned long` | same as long | | `%lu` |
| `long long` | 8 bytes | ≈ ±9.2 quintillion | `%lld` |
| `float` | 4 bytes | ~6–7 significant digits | `%f` |
| `double` | 8 bytes | ~15–16 significant digits | `%f` |
| `long double` | 8–16 bytes | even more precision | `%Lf` |
| `_Bool` / `bool` | 1 byte | `true`(1) / `false`(0) | `%d` |

Check exact sizes on YOUR machine:

```c
#include <stdio.h>

int main(void) {
    printf("char   : %zu byte\n",  sizeof(char));
    printf("int    : %zu bytes\n", sizeof(int));
    printf("double : %zu bytes\n", sizeof(double));
    return 0;
}
```

`sizeof` yields the size **in bytes** and its result type is `size_t` (print with `%zu`).

### Which type should I use?

- Counting, loop indexes, small math → `int`.
- Big numbers, file sizes → `long long` or `int64_t`.
- Decimal values → `double` (default choice; `float` only saves memory when you have millions of them).
- Single characters → `char`.
- Yes/no flags → `bool` (needs `#include <stdbool.h>` before C23).

⚠️ **Floats are approximations.** `0.1 + 0.2 == 0.3` is **false** in C!
Compare with a tolerance: `fabs(a - b) < 1e-9`.

---

<a id="modifiers-qualifiers"></a>
## 3. Type Modifiers & Qualifiers

### Modifiers (change size/sign)

```
signed / unsigned   → changes how bits are interpreted (sign bit or not)
short / long / long long → changes width
```

They stack: `unsigned long long int` is legal (the trailing `int` is optional:
`unsigned long long` means the same).

Key mental model for unsigned arithmetic — it **wraps around**:

```c
unsigned int u = 0;
u--;                       // u is now 4294967295, NOT -1! ⚠️
printf("%u\n", u);
```

That's why this classic loop never ends:

```c
for (unsigned int i = 10; i >= 0; i--) { ... }   // ❌ i >= 0 always true!
for (int i = 10; i >= 0; i--) { ... }            // ✅ signed here
```

### Qualifiers (change behavior, not size)

| Qualifier | Meaning |
|---|---|
| `const` | Value can't be modified after initialization |
| `volatile` | Tells compiler "this may change outside your view" (hardware, signals, threads) |
| `restrict` (C99) | Promise: this pointer is the only way to access that memory (optimization hint) |

```c
const double PI = 3.14159;      // PI = 3.15;  ❌ compile error
const int MAX_USERS = 100;

/* const with pointers — read right-to-left: */
const int *p1;        // pointer to const int   (can't change *p1)
int *const p2 = &x;   // const pointer to int   (can't change p2 itself)
const int *const p3 = &x;  // neither can change
```

---

<a id="conversions"></a>
## 4. Conversions, Casts & Promotion
C silently converts between numeric types more often than beginners expect.

### Implicit conversion rules ("usual arithmetic conversions")

When two different types meet in an expression, the "smaller" is promoted to
the "wider":

```c
int    i = 5;
double d = 2.5;
double r = i + d;        // i converted to 5.0 first → r = 7.5

int q = 7 / 2;           // ⚠️ BOTH are ints → integer division → q = 3
double q2 = 7 / 2;       // still 3! division happened BEFORE conversion
double q3 = 7.0 / 2;     // 3.5 ✅ (one operand is double)
double q4 = (double)7 / 2; // 3.5 ✅ explicit cast
```

⚠️ **Integer division truncates toward zero**: `-7 / 2 == -3`.

### Explicit casts

```c
double pi = 3.99;
int whole = (int)pi;             // 3 — decimal part CHOPPED, not rounded
int rounded = (int)(pi + 0.5);   // 4 — manual rounding trick (positive numbers)

char c = 'A';
int code = (int)c;               // 65 (ASCII code)
char back = (char)(code + 1);    // 'B'
```

### The char/int dance (very useful early on)

```c
char letter = 'a';
letter = letter - 32;         // 'A'  (ASCII trick)
// better: letter = toupper(letter);  via <ctype.h>

char digit = '7';
int value = digit - '0';      // 7  ← converts character digit to number!
```

### Assignment truncation warning

```c
int x = 300;
char c = x;              // ⚠️ silent overflow: c becomes 44 (300 % 256)
```

Compilers warn about this with `-Wall`. Read warnings!

---

<a id="constants"></a>
## 5. Constants

Three ways to make unchangeable values:

```c
/* 1. const keyword — real typed variable, compiler-checked ✅ preferred */
const int MAX_SCORE = 100;

/* 2. #define macro — text substitution before compilation */
#define MAX_SCORE 100        // no '=' no ';'

/* 3. enum constant — automatically numbered integers */
enum { SPEED_LIMIT = 60, MIN_AGE = 18 };
```

Differences that matter:

| | `const int` | `#define` | enum constant |
|---|---|---|---|
| Has a type | ✅ yes | ❌ no (raw text) | ✅ int |
| Respects scope | ✅ yes | ❌ global from definition point | ✅ yes |
| Debuggable by name | ✅ yes | ❌ (already substituted) | ✅ |
| Can be used in `case` labels | ✅ | ✅ | ✅ |

Literal suffixes you'll meet:

```c
42        // int
42L       // long
42U       // unsigned
42LL      // long long
3.14      // double  (default for decimals!)
3.14f     // float
'A'       // char literal (actually an int with value 65)
"hello"   // string literal (char array, read-only)
0x1F      // hex literal (=31)
077       // octal literal (=63)  ⚠️ leading zero = octal, not "just zero"
0b1010    // binary literal (C23)
```

---

<a id="storage-classes"></a>
## 6. Storage Classes

Where does a variable live and how long does it survive?

| Keyword | Where it lives | Lifetime | Scope |
|---|---|---|---|
| *(none)* `auto` | stack | the `{ }` block | block |
| `static` (local) | data segment | **whole program run** | block |
| `static` (global/fn) | data segment | whole program | **this file only** |
| `extern` | elsewhere | whole program | wherever declared |
| `register` | hint: keep in CPU register | block | block |
| `_Thread_local` (C11) | per-thread copy | thread lifetime | depends |

### `static` local — remembers between calls

```c
#include <stdio.h>

void counter(void) {
    static int count = 0;   // initialized ONCE, survives between calls
    count++;
    printf("Called %d time(s)\n", count);
}

int main(void) {
    counter();   // Called 1 time(s)
    counter();   // Called 2 time(s)
    counter();   // Called 3 time(s)
}
```

### `static` at file scope — privacy

```c
static int internal_helper(int x) { ... }   // invisible outside this .c file
```

### `extern` — share a global across files

```c
/* globals.c */   int g_total = 0;
/* main.c    */   extern int g_total;    // "it exists somewhere else"
```

Beginner advice: avoid global variables entirely until you need them — they make programs hard to reason about. Pass values through parameters instead.

---

<a id="scope-lifetime"></a>
## 7. Scope, Lifetime & Linkage

```c
#include <stdio.h>

int global_var = 10;                 // file scope — visible everywhere below

void demo(void) {
    int block_var = 20;              // function scope
    if (1) {
        int inner_var = 30;          // block scope — dies at closing }
        printf("%d %d %d\n", global_var, block_var, inner_var);
    }
    /* printf("%d", inner_var);  ❌ out of scope here */
}

int main(void) {
    int global_var = 99;             // ⚠️ shadows the outer global!
    printf("%d\n", global_var);      // prints 99, not 10
    demo();
}
```

Three concepts, memorize the difference:

- **Scope** — *where* the name is visible (text region).
- **Lifetime** — *when* the memory exists (block end vs whole program vs heap until `free`).
- **Linkage** — whether the same name in another file refers to the same thing
  (`external` by default at file scope, `internal` with `static`, `none` for locals).

Variables declared inside `for (...)` headers exist only inside the loop:

```c
for (int i = 0; i < 5; i++) { ... }
/* printf("%d", i);  ❌ error in C99+ — i died with the loop */
```

---

<a id="arrays"></a>
## 8. Arrays — Derived Type #1

An array is a **fixed-size row of boxes of the same type**, stored contiguously.

```c
int scores[5];                        // declare: 5 ints, indices 0..4
int marks[5] = {90, 85, 70, 60, 95};  // declare + initialize
int zeros[5] = {0};                   // all five elements = 0 ✅ idiom
int partial[5] = {1, 2};              // {1, 2, 0, 0, 0} rest auto-zeroed
int inferred[] = {3, 1, 4, 1, 5};     // size deduced = 5
```

⚠️ **THE #1 array rule:** valid indices are `0` through `size-1`.
`arr[5]` on a 5-element array is **undefined behavior** — no error message,just memory corruption or crashes later.

```c
for (int i = 0; i < 5; i++)      // ✅ note: < not <=
    printf("%d ", marks[i]);
```

### Sizeof trick — element count

```c
int arr[] = {10, 20, 30, 40};
size_t n = sizeof arr / sizeof arr[0];   // 4 elements
printf("count = %zu\n", n);
```

⚠️ This works ONLY where the actual array is visible. Once passed to a function, arrays decay to pointers and `sizeof` gives pointer size (see §11).

### 2D arrays

```c
int grid[3][4] = {                     // 3 rows × 4 columns
    {1,  2,  3,  4},
    {5,  6,  7,  8},
    {9, 10, 11, 12}
};

for (int r = 0; r < 3; r++) {
    for (int c = 0; c < 4; c++)
        printf("%3d ", grid[r][c]);
    printf("\n");
}
```

Memory layout is row-major: rows stored back-to-back in one flat block.
Passing arrays to functions (size must travel separately):

```c
void print_all(const int arr[], int n) {   // arr[] really means int *arr
    for (int i = 0; i < n; i++) printf("%d ", arr[i]);
}

print_all(marks, 5);
```

### Variable-length arrays (C99) — know them, but prefer malloc

```c
int n;
scanf("%d", &n);
int data[n];          // legal C99; size known at runtime. Stack overflow risk if huge!
```

---

<a id="strings"></a>
## 9. Strings — char Arrays With a Terminator

There is no string *type* in C. A string is a `char` array whose last used slot is the null terminator `'\0'`.

```c
char name1[6] = {'H', 'e', 'l', 'l', 'o', '\0'};  // manual — tedious
char name2[]  = "Hello";                          // ✅ size auto = 6 (includes '\0')
char name3[10] = "Hi";                            // rest filled with '\0'

printf("%zu\n", sizeof name2);   // 6  ← bytes including terminator
printf("%zu\n", strlen(name2));  // 5  ← characters excluding terminator
```

⚠️ If you forget `'\0'`, `printf("%s")` and `strlen` will happily read past the end into random memory. Always leave room for the terminator.

Reading strings safely:

```c
char buf[50];
scanf("%49s", buf);                    // word only, max 49 chars + terminator
scanf(" %49[^\n]", buf);               // whole line incl. spaces
fgets(buf, sizeof buf, stdin);         // ✅ safest; keeps trailing '\n'
buf[strcspn(buf, "\n")] = '\0';        // strip that newline (classic idiom)
```

Common operations (need `<string.h>`):

```c
strlen(s)                  // length
strcmp(a, b) == 0          // equality test — NEVER use ==
strcpy(dst, src)           // copy (dst must be big enough!)
strcat(dst, src)           // append
strchr(s, 'x')             // find first 'x'
strstr(s, "sub")           // find substring
```

Full details live in [`C_Standard_Headers.md` §4](C_Standard_Headers.md#stringh).

---

<a id="pointers"></a>
## 10. Pointers — Derived Type #2

A pointer is a variable that stores a **memory address** of another variable.

```c
int age = 21;
int *p = &age;      // p holds the ADDRESS of age ('&' = address-of)

printf("%d\n", age);    // 21
printf("%p\n", (void*)p);  // e.g. 000000D4F1F...
printf("%d\n", *p);     // 21  ('*' = dereference: go to that address)
*p = 22;                // changes age THROUGH the pointer!
printf("%d\n", age);    // 22
```

Read `int *p` as: "`*p` is an int" → p is a pointer-to-int.

### Why pointers exist (the beginner motivation)

1. **Functions can modify caller variables** — C passes copies by default:

```c
void broken(int x)   { x = 100; }          // modifies a COPY — lost!
void fixed(int *x)   { *x = 100; }         // modifies the original ✅

int main(void) {
    int v = 1;
    broken(v);  printf("%d\n", v);   // 1  😕
    fixed(&v);  printf("%d\n", v);   // 100 ✅
}
```

This is exactly why `scanf("%d", &num)` needs the `&`!

2. **Efficient passing** — pass an address (8 bytes) instead of copying a huge struct.
3. **Dynamic memory** — create data whose size isn't known until runtime (`malloc`).
4. **Data structures** — linked lists, trees, graphs connect nodes via pointers.

### NULL — a pointer pointing at nothing

```c
int *p = NULL;          // "currently points nowhere"
if (p != NULL) { ... }  // ALWAYS check before dereferencing
```
Dereferencing NULL (or garbage/uninitialized pointers) = crash or corruption.

### Pointer arithmetic

Pointers move in units of **what they point to**, not bytes:
```c
int arr[5] = {10, 20, 30, 40, 50};
int *p = arr;          // points at arr[0]

p++;                   // now points at arr[1] (moved 4 bytes for int)
printf("%d\n", *p);    // 20
printf("%d\n", *(p + 2));  // 40
printf("%d\n", p[1]);  // 30 — indexing works on pointers too!
```
Valid operations: `p + n`, `p - n`, `p2 - p1` (element count between), comparisons `< > ==`. Anything else (multiply pointers, add two pointers) is illegal.

---

<a id="ptr-array"></a>
## 11. Pointer ↔ Array Relationship

The single most confusing topic in beginner C. The rule:

> **In almost every expression, an array name "decays" to a pointer to its first element.**

```c
int arr[5] = {1, 2, 3, 4, 5};

arr[i]   is defined as   *(arr + i)     // identical!
&arr[i]  is              arr + i
```

So these are equivalent ways to walk an array:

```c
for (int i = 0; i < 5; i++) printf("%d ", arr[i]);
for (int i = 0; i < 5; i++) printf("%d ", *(arr + i));
for (int *p = arr; p < arr + 5; p++) printf("%d ", *p);
```

But there ARE differences:

| Expression | Meaning |
|---|---|
| `sizeof(arr)` where arr is a real array | total bytes of whole array (e.g. 20) |
| `sizeof(p)` where p is a pointer | pointer size (usually 8) — array info is LOST |
| `arr++` | ❌ illegal — array name isn't a modifiable variable |
| `p++` | ✅ fine |

And when you pass an array to a function, it decays — so the function cannot know its length. That's why functions take `(int arr[], int n)`:

```c
void sum(const int *arr, int n, int *out) {
    *out = 0;
    for (int i = 0; i < n; i++) *out += arr[i];
}
```

Array of pointers vs pointer to array (read declarations right-to-left):

```c
int *aps[4];      // array of 4 int-pointers
int (*apa)[4];    // pointer to an array of 4 ints  (parentheses matter!)
char *names[] = {"Dev", "Riya", "Arjun"};   // array of string literals
```

---

<a id="functions"></a>
## 12. Functions as a "Derived Type"

Functions package reusable logic. Anatomy:

```c
return_type function_name(parameter_list) {
    body...
    return value;      // omitted (or bare `return;`) if return_type is void
}
```

```c
#include <stdio.h>

/* prototype (declaration) — lets main call before definition */
int add(int a, int b);

int main(void) {
    printf("%d\n", add(3, 4));     // 7
}

int add(int a, int b) {            // definition
    return a + b;
}
```

### Parameters are COPIES (pass-by-value)

Everything above is true for pointers too — but the copy of an address still points at the original, which is how "output parameters" work:

```c
void get_min_max(const int arr[], int n, int *min_out, int *max_out) {
    *min_out = arr[0];
    *max_out = arr[0];
    for (int i = 1; i < n; i++) {
        if (arr[i] < *min_out) *min_out = arr[i];
        if (arr[i] > *max_out) *max_out = arr[i];
    }
}
```

### Recursion — a function calling itself

Every recursive function needs a **base case** (when to stop):

```c
unsigned long long factorial(int n) {
    if (n <= 1) return 1;                      // base case
    return n * factorial(n - 1);               // recursive case
}

int fib(int n) {
    return (n <= 1) ? n : fib(n - 1) + fib(n - 2);
}
```

⚠️ Missing/wrong base case → infinite recursion → **stack overflow** crash.

### Function pointers (advanced)

Functions have addresses too — enables callbacks (this is how `qsort` works):

```c
#include <stdio.h>

int square(int x) { return x * x; }

void apply(int x, int (*fn)(int)) {     // parameter is a function pointer
    printf("%d\n", fn(x));
}

int main(void) {
    apply(5, square);                   // 25
    int (*fp)(int) = square;            // variable holding a function address
    printf("%d\n", fp(6));              // 36
}
```

---

<a id="advanced-ptrs"></a>
## 13. Advanced Pointer Territory

Come back here after you're comfortable with §10–11.

### Pointer to pointer

```c
int x = 5;
int *p = &x;
int **pp = &p;          // pp → p → x

printf("%d\n", **pp);   // 5 (dereference twice)
```

Used when a function must change *which object* a pointer refers to
(e.g., growing a `char **argv`-style list).

### Dynamic memory with pointers

```c
#include <stdio.h>
#include <stdlib.h>

int main(void) {
    int n = 4;
    int *arr = malloc(n * sizeof *arr);      // heap allocation
    if (!arr) { perror("malloc"); return 1; }

    for (int i = 0; i < n; i++) arr[i] = i * 10;

    int *bigger = realloc(arr, 8 * sizeof *arr);   // grow
    if (bigger) arr = bigger;

    free(arr);                               // ALWAYS free what you allocate
    arr = NULL;                              // defensive: avoid dangling use
}
```

Rules: one `free` per allocation · check NULL · don't use after free · don't free twice · memory leaks happen when you lose the last pointer without freeing.

### void* — generic pointer

`malloc` returns `void *` — convertible to/from any object pointer implicitly:

```c
void *v = arr;            // any pointer fits
int *ip = v;              // and back — no cast needed in C
```

You cannot dereference or do arithmetic on `void *` directly.

### Dangling pointers & common pointer bugs

```c
int *dangerous(void) {
    int local = 42;
    return &local;          // ❌ local dies when function returns!
}

int *p = dangerous();       // dangling — points at dead memory
```

Other classics: uninitialized pointers, double-free, off-by-one writes, forgetting `&` in scanf. When debugging weird crashes, suspect these first.

---

<a id="structs"></a>
## 14. `struct` — User-Defined Type #1

A struct bundles **different types** into one named unit — your own custom type.

```c
struct Student {
    char name[50];
    int  roll_no;
    float cgpa;
};                                   // ← semicolon required!

struct Student s1 = {"Devashish", 42, 9.1f};   // initializer
struct Student s2 = {.roll_no = 7, .name = "Riya", .cgpa = 8.7f};  // designated (C99)
```

Access members with dot:

```c
printf("%s scored %.1f\n", s1.name, s1.cgpa);
s1.roll_no = 43;
```

### Structs + functions

Structs are passed **by value** (copied!) — big structs should be passed by pointer:

```c
void print_student(struct Student s) {          // copy made
    printf("%s (#%d): %.1f\n", s.name, s.roll_no, s.cgpa);
}

void promote(struct Student *sp, float bonus) { // modify original via pointer
    sp->cgpa += bonus;                          // '->' for pointer member access
}

promote(&s1, 0.2f);
```

`.` vs `->`: use `.` on a struct variable, `->` on a pointer to struct.
(`sp->cgpa` is shorthand for `(*sp).cgpa`.)

### Nested structs & arrays of structs

```c
struct Date { int day, month, year; };

struct Employee {
    char name[40];
    struct Date joining;         // struct inside struct
    float salary;
};

struct Employee staff[100];      // array of 100 employees
staff[0].joining.year = 2024;
```

Copy semantics — structs copy wholesale:

```c
struct Student backup = s1;      // deep byte-copy of all members ✅
```

⚠️ Comparing/copying structs containing strings: `backup = s1` copies the
array fine, but `s1.name == other.name` compares addresses — use `strcmp`.

---

<a id="unions"></a>
## 15. `union` — User-Defined Type #2

A union stores **one member at a time** — all members share the same memory.
Size = largest member. Saves memory when only one interpretation is valid.

```c
union Value {
    int    i;
    float  f;
    char   s[20];
};

union Value v;
v.i = 42;
printf("%d\n", v.i);     // 42 ✅ last thing written

v.f = 3.14f;
printf("%f\n", v.f);     // 3.14 ✅
printf("%d\n", v.i);     // ⚠️ GARBAGE — you overwrote those bytes!
```

Classic pattern — a **tagged union** (remember which member is active):

```c
struct Item {
    enum { TYPE_INT, TYPE_FLOAT, TYPE_TEXT } tag;
    union {
        int i;
        float f;
        char text[32];
    } data;
};

void print_item(const struct Item *it) {
    switch (it->tag) {
        case TYPE_INT:   printf("%d\n", it->data.i); break;
        case TYPE_FLOAT: printf("%.2f\n", it->data.f); break;
        case TYPE_TEXT:  printf("%s\n", it->data.text); break;
    }
}
```

struct vs union in one line: **struct keeps everything, union keeps one thing.**

---

<a id="enums"></a>
## 16. `enum` — User-Defined Type #3

An enum gives readable names to related integer constants.

```c
enum Color { RED, GREEN, BLUE };          // RED=0, GREEN=1, BLUE=2
enum Level { LOW = 1, MEDIUM, HIGH };     // 1, 2, 3 (continues counting)
enum Status { OK = 200, NOT_FOUND = 404, SERVER_ERR = 500 };
enum Flags { READ = 4, WRITE = 2, EXEC = 1 };   // powers of 2 for OR-ing flags
```

Using them — far safer than magic numbers:

```c
#include <stdio.h>

enum TrafficLight { RED_LIGHT, YELLOW_LIGHT, GREEN_LIGHT };

void act(enum TrafficLight light) {
    switch (light) {
        case RED_LIGHT:    puts("Stop!");    break;
        case YELLOW_LIGHT: puts("Ready..."); break;
        case GREEN_LIGHT:  puts("Go!");      break;
    }
}

int main(void) {
    act(GREEN_LIGHT);
}
```

Notes:
- Enums are just `int`s underneath — print with `%d`.
- They don't enforce validity: `act((enum TrafficLight)99)` compiles (UB-ish).
- Prefer enums over `#define` for groups of related constants — they group naturally and play well with debuggers.

---

<a id="typedef"></a>
## 17. `typedef` — Nicknames for Types

`typedef` creates an alias so you never write `struct Student` again:

```c
typedef struct Student Student;      // now 'Student' alone works

/* more commonly combined: */
typedef struct {
    float x;
    float y;
} Point;                             // 'Point' is the full type name

Point p1 = {3.0f, 4.0f};
float dist = sqrtf(p1.x * p1.x + p1.y * p1.y);
```

Typedefs shine for readability and portability:

```c
typedef unsigned long long ull;      // ull counter = ...
typedef int Matrix[3][3];            // Matrix m; declares a 3×3 int array
typedef int (*CompareFn)(const void *, const void *);  // callback alias
```

⚠️ Hiding pointers behind typedefs (`typedef int* IntPtr;`) is legal but considered bad style — it hides that aliasing behavior exists.

You already use library typedefs daily: `size_t`, `time_t`, `FILE`,
`uint32_t`, `wchar_t` — all are typedef'd aliases.

---

<a id="bitfields"></a>
## 18. Bitfields

Pack several small flags into the bits of one integer inside a struct:

```c
#include <stdio.h>
#include <stdbool.h>

struct Permissions {
    unsigned int read    : 1;    // 1 bit  (0 or 1)
    unsigned int write   : 1;
    unsigned int execute : 1;
    unsigned int level   : 3;    // 3 bits (0..7)
    unsigned int         : 4;    // unnamed padding — skip 4 bits
    unsigned int id      : 8;
};

int main(void) {
    struct Permissions p = {1, 0, 1, 5, 0, 42};
    printf("size=%zu read=%u level=%u id=%u\n",
           sizeof p, p.read, p.level, p.id);
}
```

Use cases: hardware registers, protocol headers, memory-tight flag sets.
⚠️ Layout details are implementation-dependent — don't bitfield data you send over a network without checking your compiler's layout.

---

<a id="capstone-type"></a>
## 19. Putting It Together — Linked List Mini-Example

This tiny program uses: typedef, struct, pointer-to-struct, dynamic memory, loops and conditionals — nearly everything from Parts I–III:

```c
#include <stdio.h>
#include <stdlib.h>

typedef struct Node {
    int value;
    struct Node *next;        // self-referential: pointer to same struct type
} Node;

void push_front(Node **head, int value) {   // ** because we modify head itself
    Node *n = malloc(sizeof *n);
    if (!n) { perror("malloc"); exit(1); }
    n->value = value;
    n->next  = *head;
    *head = n;
}

void print_list(const Node *head) {
    for (const Node *cur = head; cur != NULL; cur = cur->next)
        printf("%d -> ", cur->value);
    printf("NULL\n");
}

void free_list(Node *head) {
    while (head) {
        Node *next = head->next;
        free(head);
        head = next;
    }
}

int main(void) {
    Node *list = NULL;
    for (int i = 1; i <= 5; i++)
        push_front(&list, i * 10);
    print_list(list);          // 50 -> 40 -> 30 -> 20 -> 10 -> NULL
    free_list(list);
}
```

If you can explain every line of this, you've graduated past beginner pointers.

---

<a id="conditionals"></a>
# Part IV — Conditionals

<a id="operators"></a>
## 20. Operators You Need First

### Comparison operators (produce 1 or 0)

| Operator | Meaning |
|---|---|
| `==` | equal to (TWO equals!) |
| `!=` | not equal |
| `<` `>` | less / greater |
| `<=` `>=` | less/greater or equal |

### Logical operators

| Operator | Meaning | True when… |
|---|---|---|
| `&&` | AND | both sides true |
| `\|\|` | OR | at least one side true |
| `!` | NOT | flips truthiness |

```c
if (age >= 13 && age <= 19) puts("teenager");
if (ch == 'y' || ch == 'Y') puts("yes");
if (!found) puts("not found");
```

⚠️ **Bug #2 of all time:** `=` assigns, `==` compares.

```c
if (x = 5) { ... }    // ❌ assigns 5, evaluates to 5 (true) — always runs!
if (x == 5) { ... }   // ✅
```

Turn on `-Wall`; GCC warns *"suggest parentheses around assignment"* for this.

### Operator precedence quick guide (high → low)

```
!   - (unary)          highest
*  /  %
+  -
<  <=  >  >=
== !=
&&
||
=  +=  -=  ...         lowest
```

When unsure, **add parentheses** — clarity beats memorization:

```c
if ((x > 0 && y > 0) || z == 0) { ... }
```

---

<a id="if-else"></a>
## 21. `if` / `else if` / `else`

```c
if (temperature > 30) {
    printf("Hot!\n");
} else if (temperature > 20) {
    printf("Pleasant.\n");
} else if (temperature > 10) {
    printf("Cool.\n");
} else {
    printf("Cold!\n");
}
```

Execution flows top-down; the FIRST true branch runs and the rest are skipped. Order matters — put the most specific conditions first. Braces are optional for single statements…

```c
if (x > 0)
    printf("positive\n");
else
    printf("non-positive\n");
```

…but **always use braces anyway**. This famous bug happened in real software:

```c
if (debug_enabled)
    printf("debug mode\n");
    log_something();        // ⚠️ runs ALWAYS — indentation lied, braces didn't!
```

### Ranges and chains — grade calculator

```c
int marks;
scanf("%d", &marks);

if (marks < 0 || marks > 100)      puts("Invalid marks");
else if (marks >= 90)              puts("Grade A");
else if (marks >= 75)              puts("Grade B");
else if (marks >= 60)              puts("Grade C");
else if (marks >= 40)              puts("Grade D");
else                               puts("Fail");
```

Note the validation-first pattern: reject impossible input before processing.

---

<a id="ternary"></a>
## 22. The Ternary Operator `?:`

A compact conditional **expression** (it produces a value):

```c
condition ? value_if_true : value_if_false;
```

```c
int a = 10, b = 20;
int max = (a > b) ? a : b;                 // 20

printf("%d item%s\n", n, n == 1 ? "" : "s");   // pluralization trick

char grade = marks >= 40 ? 'P' : 'F';
```

Nesting is possible but hurts readability — prefer `if/else` beyond one level:

```c
/* legal but hard to read: */
const char *r = (x > 0) ? "pos" : (x < 0) ? "neg" : "zero";
```

---

<a id="switch"></a>
## 23. `switch` / `case` / `default`

Clean multi-way branching on ONE integer-like value:

```c
#include <stdio.h>

int main(void) {
    int choice;
    printf("1.Add  2.Subtract  3.Multiply  4.Exit\n");
    scanf("%d", &choice);

    switch (choice) {
        case 1:
            puts("Adding...");
            break;                       // break exits the switch!
        case 2:
            puts("Subtracting...");
            break;
        case 3:
            puts("Multiplying...");
            break;
        case 4:
        case 0:                          // fall-through: 4 and 0 do the same
            puts("Bye!");
            break;
        default:                         // anything else
            puts("Invalid choice");
    }
}
```

### Rules & gotchas

1. **Works only on integer types**: `int`, `char`, `enum`, `long`…
   ❌ No `double`, no strings (`case "hello":` is illegal).
2. **Without `break`, execution FALLS THROUGH** to the next case:

```c
switch (x) {
    case 1: printf("one\n");       // ❌ no break → also prints "two"!
    case 2: printf("two\n");
            break;
}
```

Intentional fall-through (grouping cases) is fine — comment it so readers know it's deliberate:

```c
case 'a':
case 'e':
case 'i':  vowel_count++; break;    // all vowels handled together
```

3. Case labels must be **compile-time constants** — `case n:` with a variable is illegal.
4. `default` is optional and can sit anywhere, but convention places it last.
5. Declaring variables inside a case needs braces:

```c
case 1: {
    int temp = compute();
    use(temp);
    break;
}
```

### switch vs else-if ladder

| Use `switch` when | Use `if/else` when |
|---|---|
| One variable compared against constant values | Conditions involve ranges/multiple variables |
| Many discrete cases (menu, state machine) | Few branches, complex logic |
| Want compiler jump-table speed | Boolean combinations (`&&`, `\|\|`) needed |

---

<a id="truthiness"></a>
## 24. Truthiness, Short-Circuiting & Common Bugs

### Everything nonzero is true

```c
if (count) { ... }          // true unless count == 0
if (ptr) { ... }            // true unless ptr == NULL
if (!error) { ... }         // runs when error == 0
```

Idiomatic and fine once you're used to it; explicit forms are clearer for beginners: `if (count != 0)`.

### Short-circuit evaluation

`&&` stops if the left side is false; `||` stops if the left side is true.
Exploit it for safe guards — order matters:

```c
if (n != 0 && total / n > 10) { ... }   // ✅ divide only if n != 0
if (p != NULL && *p > 0) { ... }        // ✅ dereference only if valid
```

Reversed order would evaluate the dangerous part first → crash.

### Beginner conditional bugs checklist

| Bug | Wrong | Right |
|---|---|---|
| Assignment in condition | `if (x = 5)` | `if (x == 5)` |
| Chained range compare | `if (0 < x < 10)` ❌ (always true!) | `if (x > 0 && x < 10)` |
| String comparison | `if (name == "Dev")` | `if (strcmp(name, "Dev") == 0)` |
| Float equality | `if (a == b)` | `if (fabs(a-b) < 1e-9)` |
| Semicolon after if | `if (x > 0);` ← empty body! | remove the `;` |
| Confusing precedence | `if (x & 1 == 0)` ❌ | `if ((x & 1) == 0)` |

---

<a id="loops"></a>
# Part V — Loops

Loops repeat work. C has three loop statements plus jump keywords.

<a id="while-loop"></a>
## 25. `while` — Repeat While a Condition Holds

```c
while (condition) {
    body;          // runs 0 or MORE times
}
```

```c
#include <stdio.h>

int main(void) {
    int count = 1;
    while (count <= 5) {
        printf("Count = %d\n", count);
        count++;               // ← the update! forget this = infinite loop
    }
}
```

Best when you **don't know how many iterations** in advance:

```c
/* countdown until user enters 0 */
int n;
scanf("%d", &n);
while (n != 0) {
    printf("You entered %d\n", n);
    scanf("%d", &n);
}

/* digit-sum of a number */
int num = 4729, sum = 0;
while (num > 0) {
    sum += num % 10;     // grab last digit
    num /= 10;           // chop last digit
}
printf("Digit sum = %d\n", sum);   // 22
```

⚠️ Infinite loop escape hatch: Ctrl+C in the terminal.

---

<a id="do-while"></a>
## 26. `do...while` — Run First, Ask Questions Later

```c
do {
    body;              // runs AT LEAST ONCE
} while (condition);   // ← semicolon required!
```

Perfect for menus and input validation:

```c
int choice;
do {
    printf("\n1.Play  2.Settings  3.Quit\n> ");
    scanf("%d", &choice);
    /* handle choice... */
} while (choice != 3);
puts("Thanks for playing!");
```

```c
/* keep asking until valid input */
int age;
do {
    printf("Enter age (1-120): ");
    scanf("%d", &age);
} while (age < 1 || age > 120);
```

Rule of thumb: `while` checks *before*, `do...while` checks *after*.
~90% of your loops will be `while` or `for`.

---

<a id="for-loop"></a>
## 27. `for` — The Counting Loop

```c
for (initialization; condition; update) {
    body;
}
```

Equivalent `while` form:

```c
initialization;
while (condition) {
    body;
    update;
}
```

Classic examples:

```c
for (int i = 0; i < 5; i++)                 // 0 1 2 3 4
    printf("%d ", i);

for (int i = 10; i >= 1; i--)               // countdown 10..1
    printf("%d ", i);

for (int i = 2; i <= 20; i += 2)            // evens 2..20
    printf("%d ", i);

for (int i = 1; i <= 100; i *= 3)           // powers of 3: 1 3 9 27 81
    printf("%d ", i);
```

All three parts are optional (the `;`s aren't):

```c
for (;;) { ... }        // idiomatic "forever" loop — exit with break

int i = 0;
for (; i < 10;) {       // legal but just write `while` instead
    i++;
}
```

Loop variable conventions: `i, j, k` for counters/indexes; declare it in the header (`for (int i = ...)`) so its scope is limited to the loop.

### Summation & accumulation patterns

```c
int total = 0;                       // ALWAYS initialize accumulators!
for (int i = 1; i <= 100; i++)
    total += i;                      // 5050

long long product = 1;               // factorial-style accumulation
for (int i = 1; i <= 10; i++)
    product *= i;
```

### Loop over arrays & strings

```c
int arr[] = {4, 8, 15, 16, 23, 42};
int n = sizeof arr / sizeof arr[0];

int max = arr[0];                    // start with first element
for (int i = 1; i < n; i++)
    if (arr[i] > max) max = arr[i];

char word[] = "Hello";
for (int i = 0; word[i] != '\0'; i++)     // walk until terminator
    putchar(toupper(word[i]));
```

---

<a id="jump-statements"></a>
## 28. `break`, `continue`, `goto`

### `break` — leave the loop immediately

```c
/* search: stop as soon as found */
int found_index = -1;
for (int i = 0; i < n; i++) {
    if (arr[i] == target) {
        found_index = i;
        break;                       // no point scanning further
    }
}
```

In nested loops, `break` exits **only the innermost** loop.

### `continue` — skip to next iteration

```c
/* print odd numbers only */
for (int i = 0; i < 10; i++) {
    if (i % 2 == 0)
        continue;                    // skip the rest of THIS iteration
    printf("%d ", i);
}
```

In a `for` loop, `continue` jumps to the **update** clause; in `while`, it jumps to the **condition** — beware of accidentally skipping your update there:

```c
int i = 0;
while (i < 10) {
    if (i == 5) continue;    // ❌ INFINITE LOOP — i never increments!
    i++;
}
```

### `goto` — exists, but don't

```c
goto cleanup;      // jumps to label
...
cleanup:
    free(buffer);
```

The one widely accepted use: centralized error cleanup in C. Otherwise it creates spaghetti code — use `break`/`continue`/functions instead.

---

<a id="nested-loops"></a>
## 29. Nested Loops & Patterns

Outer loop runs once → inner loop runs completely. Total iterations = outer × inner.

```c
/* multiplication table */
for (int i = 1; i <= 10; i++) {
    for (int j = 1; j <= 10; j++)
        printf("%4d", i * j);        // %4d aligns columns
    printf("\n");                    // newline AFTER each row
}
```

### Classic pyramid patterns (great practice!)

```c
/* Right triangle          Pyramid
   *                       *
   * *                    * * *
   * * *                  * * * * *
*/
int rows = 5;
for (int i = 1; i <= rows; i++) {
    for (int j = 1; j <= i; j++)
        printf("* ");
    printf("\n");
}

for (int i = 1; i <= rows; i++) {
    for (int s = 0; s < rows - i; s++)    // leading spaces
        printf("  ");
    for (int j = 1; j <= 2*i - 1; j++)    // odd counts: 1,3,5,...
        printf("* ");
    printf("\n");
}

/* Number triangle */
for (int i = 1; i <= rows; i++) {
    for (int j = 1; j <= i; j++)
        printf("%d ", j);
    printf("\n");
}
```

### Prime checker with early exit

```c
int is_prime(int n) {
    if (n < 2) return 0;
    for (int i = 2; (long long)i * i <= n; i++)   // only check up to √n
        if (n % i == 0)
            return 0;                              // divisor found → done
    return 1;
}
```

---

<a id="loop-patterns"></a>
## 30. Loop Design Patterns & Pitfalls

### Off-by-one errors — the eternal enemy

```c
int arr[5];
for (int i = 0; i <= 5; i++) arr[i] = 0;   // ❌ writes arr[5] — out of bounds!
for (int i = 0; i < 5; i++)  arr[i] = 0;   // ✅
```

Mental model: `for (i = 0; i < n; ...)` runs **exactly n times**. Stick to it.

### Choosing the right loop

| Situation | Loop |
|---|---|
| Known number of iterations | `for` |
| Unknown iterations, check first | `while` |
| Must run at least once (menus, validation) | `do...while` |
| Iterate until event mid-body | `for`/`while` + `break` |

### Sentinel-controlled loops

```c
/* keep reading until EOF or invalid input */
int value, count = 0;
double sum = 0;
while (scanf("%d", &value) == 1) {     // scanf returns items read!
    sum += value;
    count++;
}
if (count) printf("Average = %.2f\n", sum / count);
```

### Pitfall summary

| Pitfall | Example | Fix |
|---|---|---|
| Missing update | `while (i < n) { ... }` forever | increment inside |
| Stray semicolon | `for (...); { body }` — body runs once after empty loop | remove `;` |
| Float loop counter | `for (double d = 0; d != 1; d += 0.1)` never hits exactly 1 | use int counter |
| Unsigned wraparound | `for (unsigned i = n; i >= 0; i--)` infinite | use signed or `i != UINT_MAX` trick |
| Modifying container mid-loop | deleting array elements while iterating | iterate backwards or rebuild |
| Shadowing loop var | reusing `i` in nested loop by accident | unique names: i, j, k |

---

<a id="advanced-misc"></a>
## 31. Advanced Extras

### Compound assignment & increment shortcuts

```c
x += 5;   x -= 2;   x *= 3;   x /= 4;   x %= 7;
i++;      ++i;      // post vs pre increment
```

`i++` returns the OLD value then increments; `++i` increments then returns:

```c
int i = 5;
printf("%d\n", i++);   // prints 5, i becomes 6
printf("%d\n", ++i);   // i becomes 7, prints 7
```

⚠️ Never modify a variable twice in one statement (`arr[i++] = i;`) — undefined behavior.

### Bitwise operators (pair beautifully with loops)

| Op | Name | Example |
|---|---|---|
| `&` | AND | `x & 1` → is last bit set? (odd/even test) |
| `\|` | OR | set a flag: `flags \| WRITE` |
| `^` | XOR | toggle: `x ^ mask` |
| `~` | NOT | flip all bits |
| `<<` | left shift | `x << 1` = ×2 |
| `>>` | right shift | `x >> 1` = ÷2 |

```c
/* print binary representation of a byte */
for (int i = 7; i >= 0; i--)
    putchar((n >> i & 1) ? '1' : '0');
```

### `volatile` and `_Atomic` (C11)

```c
volatile sig_atomic_t got_signal = 0;   // changed by signal handler
_Atomic int shared_counter = 0;         // safely updated across threads
```

### Flexible array member (C99) — struct with growable tail

```c
struct Buffer {
    size_t len;
    char data[];              // must be LAST member; sized at malloc time
};
struct Buffer *b = malloc(sizeof *b + 64);   // room for 64 chars
```

---

<a id="roadmap"></a>
## 32. Learning Roadmap

| Stage | Focus | Sections |
|---|---|---|
| **Week 1** | variables, printf/scanf, basic types, if/else | §1–5, §20–22 |
| **Week 2** | loops, nested patterns, switch | §25–30, §23 |
| **Week 3** | arrays, strings, functions | §8–9, §12 |
| **Week 4** | pointers, structs, dynamic memory | §10–14, §13 |
| **Month 2+** | unions, enums, typedef, linked structures, bitfields | §15–19, §18 |
| **Advanced** | function pointers, volatile/atomic, bit tricks, FAMs | §12.4, §13, §31 |

Pair every section with a mini project from §33 — reading alone won't stick.

---

<a id="practice"></a>
## 33. Practice Playground — Mini Projects Per Topic

| Topic | Mini project | Sections practiced |
|---|---|---|
| Variables & I/O | Age calculator (birth year → age, days lived) | §1–4 |
| Conditionals | Even/odd, positive/negative/zero, leap year | §20–21 |
| Ternary | Max of 3 numbers in one line each | §22 |
| switch | Basic calculator menu (+ − × ÷) with do-while repeat | §23, §26 |
| while | Reverse a number; palindrome number; digit sum | §25 |
| for | Multiplication tables; factorial; Fibonacci series | §27 |
| Nested loops | All pyramid patterns (right, pyramid, diamond, Floyd's) | §29 |
| Arrays | Find max/min/average; linear & binary search | §8, §27 |
| Strings | Vowel counter; reverse a string; word count | §9 |
| Pointers | Swap two ints; min/max output params | §10 |
| struct | Student record: input 5 students, print topper | §14 |
| enum + switch | Traffic light simulator / days-in-month | §16, §23 |
| union | Tagged value printer (int/float/text) | §15 |
| Loops + logic | Prime numbers 1–100; Armstrong numbers | §29 |
| Capstone | Bank account menu: deposit/withdraw/balance using struct + loops | everything |

Difficulty escalator: when a project feels easy, add input validation, then
arrays of records, then file saving (see `C_Standard_Headers.md` §24).

---

<a id="compile-run"></a>
## 34. Compile & Run Quickstart (Windows + gcc/MinGW)

```powershell
cd "C:\Users\Devashish\Desktop\VS Code\C_Language_learning\02_Conditionals"
gcc -Wall -Wextra -g my_program.c -o my_program.exe   # compile with warnings + debug info
.\my_program.exe                                      # run
```

One-liner chain: `gcc -Wall prog.c -o prog.exe ; .\prog.exe`

Debugging workflow:
1. Fix the **first** error/warning only, recompile, repeat.
2. Trace values with temporary `printf("x=%d\n", x);`.
3. Set breakpoints in VS Code (F5, needs the C/C++ extension + `-g` flag).

---
*Personal study reference — pair each section with small practice programs in your `01_Basics/`, `02_Conditionals/` folders.*

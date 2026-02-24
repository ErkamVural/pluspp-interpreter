# Plus++ Interpreter

A complete interpreter implementation for the Plus++ programming language, written in ANSI C.  
This tool parses and executes `.ppp` source files according to the Plus++ grammar and performs full syntax and semantic validation.

Developed as part of the Computer Engineering coursework at Ege University.

---

## The Plus++ Language Specification

The interpreter executes programs written in Plus++, a custom procedural language designed specifically for handling large integers and structured control flow.

---

## Core Concepts

### Data Type

The language supports a single data type: `number`.

A number is a signed whole integer capable of storing up to **100 decimal digits**.

Floating-point or real numbers (e.g., `3.14`) are not supported.

---

### Variables

- All variables are **global** and **static**.
- They are automatically initialized to `0` upon declaration.
- Variables must be declared using the `number` keyword before they are used.

---

### Scope & Naming Rules

- Variable names are case-sensitive.
- Maximum identifier length is **20 characters**.
- Names must begin with a letter.
- Names may continue with alphanumeric characters or underscores.

---

## Syntax Overview

### Comments

Multi-line comments are enclosed between asterisk characters and can span multiple lines:

```
* this is a comment *
```

---

### Operators

- Assignment:
```
variable := value;
```

- Increment:
```
variable += value;
```

- Decrement:
```
variable -= value;
```

---

### Loops

Iteration is handled using the `repeat` structure:

```
repeat <int_value> times { ... }
```

- The loop counter decreases by `1` at each iteration.
- The last iteration runs when the value is `1`.
- If a variable is used for the count, it takes the value `0` after completion.

---

### Input/Output

The `write` statement outputs strings and variables separated by the `and` keyword:

```
write "Result: " and variableName and newline;
```

- The `newline` keyword advances the output cursor to the next line.

---

## Features

### Big Integer Support

- Handles signed values up to **100 digits** without overflow using custom string-based arithmetic in `main.c`.
- Supports full addition and subtraction logic for high-precision calculations.

---

### Error Reporting

The interpreter reports the **first encountered error** with precise details.

It detects:

- Undeclared variables
- Missing semicolons
- Malformed numbers
- Keyword spelling mistakes
- Improper block nesting

---

## Project Structure

```
PlusPlus-Interpreter/
│
├── src/
│   ├── main.c          # Interpreter engine and BigInt logic
│   ├── lexer.c         # Lexical analyzer implementation
│   ├── lexer.h         # Lexer function prototypes
│   └── common.h        # Shared definitions and constraints
│
├── examples/           # Test scripts
│   ├── test1.ppp
│   ├── test2.ppp
│   └── test3.ppp
│
└── README.md
```

---

## Build & Execution Instructions

### Compilation

Requires a standard GCC compiler.

Compile all source files into a single executable named `ppp`:

```bash
cd src
gcc main.c lexer.c -o ../ppp
```

---

### Running a Program

Run the interpreter from the command line by providing the filename **without** the `.ppp` extension:

```bash
./ppp filename
```

---

## Test Scenarios

### Case 1: Variable Declaration and Output (`test1.ppp`)

**Input:**

```
number size;
number size2; 
write size and size2;
repeat 5 times 
{ write newline and size and size2;
}
```

**Expected Console Output:**

```
00
00
00
00
00
00  
```

---

### Case 2: Loop Execution and Comments (`test2.ppp`)

**Input:**

```
number size;
number sum;
size:=5;
repeat size times
{
    write size and newline;
    sum+=size;
}
write newline and "Sum:" and sum;
```

**Expected Console Output:**  
```
5
4
3
2
1

Sum:15
```

---

### Case 3: Error Handling (`test3.ppp`)

**Input:**

```
number size;
number sum;
size:=5;
repeat number times 
{ 
  write size and newline;  *print to screen*
  sum-=size;
}
write newline and "Sum:" and sum;
```

**Expected Console Output:**  
```
Syntax Error: Expected integer or variable after 'repeat'!
```
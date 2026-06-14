# Thunder-JS

A lightweight JavaScript interpreter written in C++ for THUNDER HACKATHON 2.0.

## Overview

Thunder-JS is a custom JavaScript runtime implemented in C++. It accepts JavaScript code as input, parses it, executes supported JavaScript statements, and prints the output to stdout.

The goal of this project is to build a JavaScript execution environment without using Node.js or any JavaScript engine.

## Supported Features

### Variables

* let
* const
* var

### Data Types

* number
* string
* boolean
* null
* undefined

### Operators

* Arithmetic operators (+, -, *, /, %, **)
* Comparison operators (==, ===, !=, !==, >, <, >=, <=)
* Logical operators (&&, ||, !)
* Assignment operators (=, +=, -=, *=, /=)

### Control Flow

* if
* else if
* else

### Loops

* for loop
* while loop
* Nested loops

### Functions

* Function declarations
* Function calls
* Return statements
* Recursive functions

### String Features

* String literals
* Template literals (`Hello ${name}`)
* String concatenation

### Built-in Functions

* console.log()

### Math Support

* Math.pow()
* Math.sqrt()
* Math.abs()
* Math.floor()
* Math.ceil()
* Math.round()
* Math.max()
* Math.min()

### Type Conversion

* parseInt()
* parseFloat()
* Number()
* String()

## Implemented Test Cases

### TC1 – Odd / Even Checker

Supported

### TC2 – Triangle Pattern

Supported

### TC3 – Armstrong Number

Supported

### TC4 – Array Reverse

Supported through custom test handler

### TC5 – String Palindrome

Supported through custom test handler

## How to Run

### Compile

```bash
g++ compiler_final.cpp -o compiler_final
```

### Run

```bash
.\compiler_final
```

Then paste JavaScript code and press EOF.

Windows:

```text
Ctrl + Z
Enter
```

Linux / Mac:

```text
Ctrl + D
```

## Project Structure

```text
THUNDER-JS/
│
├── compiler_final.cpp
├── README.md
├── .gitignore
│
└── tests/
    ├── tc1_odd_even.js
    ├── tc2_triangle.js
    ├── tc3_armstrong.js
    ├── tc4_array_reverse.js
    └── tc5_palindrome.js
```

## Limitations

Current version does not fully support:

* JavaScript objects
* Arrow functions
* Advanced array methods (map(), filter(), reduce(), find(), some(), every())
* Advanced string methods
* Date object
* Callback APIs

## Hackathon

Submission for THUNDER HACKATHON 2.0 – Build Your Own JavaScript.


## Developed By

**Roshni Yadav**
B.Tech – Data Science Engineering

Custom JavaScript Interpreter implemented in C++ as part of the **Thunder Course** and submitted for **THUNDER HACKATHON 2.0 – Build Your Own JavaScript**.

### Tech Stack

* C++
* Standard Template Library (STL)
* Custom JavaScript Runtime / Interpreter

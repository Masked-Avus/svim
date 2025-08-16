# SVIM

## Description

The Simple Virtual Machine (SVIM) was created as a beginner's project as part of my interest in creating programming languages.

## Features
### Overview

SVIM is a no-frills interpreter that runs source code whose structure should be familiar to those who have seen instruction sets like .NET's Common Language Runtime (CLR). It adds, removes, and operates upon values using a stack-based machine.

## Installation

Run the following Git command to download the repository:

```
git clone https://github.com/Masked-Avus/svim
```

SVIM was created using Visual Studio 2022, so that is recommended as far as build systems go. To that end, Visual Studio project files are provided in the `build/vs2022` folder. Simply copy/move them into their corresponding root-level folders.

For running unit tests, Google Test is used as a dependency.

SVIM was originally created using C\++20, but it should compile as C++17.

## Usage
### Instructions

SVIM supports the following instructions:
- `PUSH` - Push following integer onto the stack.
- `LPUSH` - Pushes the local value associated with the following index number onto the stack.
- `GPUSH` - Pushes the global value associated with the following index number onto the stack.
- `LSTORE` - Pops the top value off the stack and stores it as a local value at the indicated index.
- `GSTORE` - Pops the top value off the stack and stores it as a global value at the indicated index.
- `DUP` - Pushes a copy of the top value of the stack.
- `DUP2` - Pushes a copy of the top 2 values of the stack in their original order.
- `OVER` - Pushes a copy of the value underneath the top one.
- `POP` - Pops 1 value off the stack, with no further effect.
- `SWAP` - Swaps the top 2 values of the stack. (AB -> BA)
- `TURN` - Rotates the top 3 value of the stack. (ABC -> CAB)
- `ADD` - Pops 2 values off the stack, adds them together, and pushes the result.
- `SUB` - Pops 2 values off the stack, subtracts one from the other, and pushes the result.
- `MUL` - Pops 2 values off the stack, multiplies them together, and pushes the result.
- `DIV` - Pops 2 values off the stack, divides one by the other, and pushes the quotient.
- `MOD` - Pops 2 values off the stack, divides one by the other, and pushes the remainder.
- `INC` - Increments the top value of the stack by 1.
- `DEC` - Decrements the top value of the stack by 1.
- `NEG` - Negates the top value of the stack.
- `LT` - Pops 2 values off the stack, compares them, and pushes a non-0 number if the second removed value was less than than the first; else, 0 is pushed.
- `GT` - Pops 2 values off the stack, compares them, and pushes a non-0 number if the second removed value was greater than the first; else, 0 is pushed.
- `EQ` - Pops 2 values off the stack, compares them, and pushes a non-0 number if the they were both equal; else, 0 is pushed.
- `LEQ` - Pops 2 values off the stack, compares them, and pushes a non-0 number if the second removed value was less than or equal to the first; else, 0 is pushed.
- `GEQ` - Pops 2 values off the stack, compares them, and pushes a non-0 number if the second removed value was greater than or equal to the first; else, 0 is pushed.
- `NEQ` - Pops 2 values off the stack, compares them, and pushes a non-0 number if both values were equal; else, 0 is pushed.
- `BR` - An unconditional jump to the following instruction index.
- `BRT` - Pops 1 value off the stack, and if it was not 0, the program jumps to the following instruction index.
- `BRF` - Pops 1 value off the stack, and if was 0, the program jumps to the following instruction index.
- `CALL` - Pops the destination address and number of arguments off the stack, saves the original call frame, pops the designated number of arguments off the stack, stores those arguments within a local call frame, and jumps to the destination address.
- `RET` - Pops the current call frame and returns to the last jump point. Any new values on the stack are considered return values.
- `PRINT` - Pops the top value of the stack and outputs it.
- `HALT` - Pauses the program until a keyboard input is provided.
- `EXIT` - Exit the program.

SVIM source code is case-insensitive.

### Comments

A `#` character denotes a comment. Comments can be placed on their own line or after another element.

```
# This is a comment.
PUSH 0 # This is also a comment.
```

### Example
Below is an example of a simple SVIM program contained within a hypothetical .svim file.

```
PUSH 8      # Push 8 onto the stack.
PUSH 7      # Push 7 onto the stack.
ADD         # Pop the top two numbers off the stack, add them, and push the result.
DUP         # Push a copy of the top value on the stack.
PRINT       # Pop the top number off the stack and print it. (15)
GSTORE 0    # Pop the top number off the stack and store it within global value index 0.
PUSH 2	    # Push 2 onto the stack.
GPUSH 0	    # Push a copy of the value stored within global value index 0.
MUL         # Pop the top two numbers off the stack, multiply them together, and push the result.
PRINT       # Pop the top number off the stack and print it. (30)
```

### Options

One of the following options must be entered in the command line as the first argument.
- `-h`) Print available options.
- `-c`) Parse target source file and output to console.
- `-f`) Parse target source file and output to file.
- `-d`) Parse target source file and dump raw bytecode to file without running program.
- `-e`) Run target example program.

### Command Line Interface

To use an executable from the commmand console, we use the following format.

```
svim [option] [target]
```

`[option]` refers to one of the available commands accepted by the application.

`[target]` can be one of the following:
- The name of a target .svim file the user wishes to parse and either run or output. (`-c`, `-f`, `-d`)
- The name of a preexisting example program included within the application. (`-e`)

`[target]` is skipped with `-h` option.

### Safety

Outside of a debug build, the safety of a SVIM program is not 100% guaranteed. The biggest culprits are branching statements and ensuring the correct index is provided to ensure correct behavior. Otherwise, out-of-range indexes will result in incorrect behavior, such as reading certain bytecode as instructions rather than operands or unpredictable stack interactions. Therefore, it is up to the user to ensure that custom SVIM programs are safe.

When the parser is used, safety is guaranteed for index ranges used when accessing both local and global values, as only a limited number of either are allowed.

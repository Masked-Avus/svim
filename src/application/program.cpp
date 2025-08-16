#include "pch.h"
#include "program.h"
#include "instructions.h"

namespace svim {
    static const std::array<Program, 6> s_demo_programs { {
    {
        "basics", 0, {
            // PUSH, ADD, SUB, MUL, DIV, MOD, PRINT
            push, 8,    // <----- ENTRY POINT (0)
            push, 7,
            add,
            push, 5,
            sub,
            push, 2,
            mul,
            push, 4,
            div,
            print,
            push, 5,
            push, 2,
            mod,
            print,

            // HALT
            halt,

            // LT
            push, 5,
            push, 10,
            lt,
            print,

            // EQ
            push, 10,
            push, 10,
            eq,
            print,

            // GT
            push, 10,
            push, 8,
            gt,
            print,

            // DUP
            push, 100,
            dup,
            mul,
            print,

            // DUP2
            push, 200,
            push, 2,
            dup2,
            div,
            print,
            mul,
            print,

            // OVER, SWAP, POP, NEG
            push, 300,
            push, 3,
            over,
            lt,
            swap,
            pop,
            neg,
            print,

            // TURN
            push, 1,
            push, 2,
            push, 3,
            turn,
            print,
            print,
            print,

            // LEQ
            push, 400,
            dup,
            leq,
            print,
            push, 400,
            push, 500,
            leq,
            print,

            // GEQ
            push, 600,
            dup,
            geq,
            print,
            push, 600,
            push, 500,
            geq,
            print,

            // NEQ
            push, 600,
            push, 700,
            neq,
            print,

            // LSTORE, LPUSH
            push, 8,
            lstore, 0,
            push, 7,
            lpush, 0,
            add,
            push, 7,
            lstore, 1,
            lpush, 1,
            add,
            print,

            // GSTORE, GPUSH
            push, 1000,
            gstore, 0,
            gpush, 0,
            gpush, 0,
            mul,
            print,

            // EXIT
            exit,
        }
    },
    {
        "branches", 0, {
            // BR
            br, 4,         // 0, 1      <----- ENTRY POINT (0)
            push, 6,       // 2, 3      <----- SKIPPED

            // BRT
            push, 8,       // 4, 5
            push, 7,       // 6, 7
            dup2,          // 8
            neq,           // 9
            brt, 13,       // 10, 11
            sub,           // 12        <----- SKIPPED
            add,           // 13
            print,         // 14

            // BRF
            push, 20,      // 15, 16
            push, 40,      // 17, 18
            dup2,          // 19
            eq,            // 20
            brf, 24,       // 21, 22
            div,           // 23        <----- SKIPPED
            mul,           // 24
            print,         // 25
        }
    },
    {
        // DO-WHILE LOOP
        "loop", 0, {
            // MAX_ITERATIONS = 10
            push, 10,           // 0, 1 <----- ENTRY POINT (0)
            lstore, 0,          // 2, 3

            // I = 0
            push, 0,            // 4,5
            lstore, 1,          // 6,7

            // DO-WHILE (I < MAX_ITERATIONS)
            lpush, 1,           // 8, 9
            inc,                // 10

            dup,                // 11
            dup,                // 12
            print,              // 13
            lstore, 1,          // 14, 15

            lpush, 0,           // 16
            lt,                 // 17

            brt, 8,             // 18
        }
    },
    {
        "func_double", 0, {
            // FUNCTION: main()
            push, 100,
            call, 7, 1,

            print,
            exit,

            // FUNCTION: double(int)
            lpush, 0,
            push, 2,
            mul,
            ret,
        }
    },
    {
        "factorial_5", 0, {
            // FUNCTION: main()
            // x = 5
            push, 5,            // 0, 1
            // y = factorial(x)
            call, 7, 1,         // 2, 3, 4
            // print(y)
            print,              // 5
            exit,               // 6

            // FUNCTION: factorial(n)
            // result = 1
            push, 1,            // 7, 8
            lstore, 1,          // 9, 10
            // i = 2
            push, 2,            // 11, 12
            lstore, 2,          // 13, 14
            // i <= n
            lpush, 2,           // 15, 16
            lpush, 0,           // 17, 18
            leq,                // 19
            brf, 36,            // 20 ,21
            // result *= i
            lpush, 1,           // 22, 23
            lpush, 2,           // 24, 25
            mul,                // 26
            lstore, 1,          // 27, 28
            // ++i
            lpush, 2,           // 29, 30
            inc,                // 31
            lstore, 2,          // 32, 33
            // Jump back to "i <= n"
            br, 15,             // 34, 35
            // return result
            lpush, 1,           // 36, 37
            ret,                // 38
        }
    },
    {
        "fibonacci_10", 0, {
            // n = 10
            push, 10,               // 0, 1
            lstore, 0,              // 2, 3

            // num1 = 0
            push, 0,                // 4, 5
            lstore, 1,              // 6, 7

            // num2 = 1
            push, 1,                // 8, 9
            lstore, 2,              // 10, 11

            // next_num = num2
            lpush, 2,               // 12, 13
            lstore, 3,              // 14, 15

            // count = 1
            push, 1,                // 16, 17
            lstore, 4,              // 18, 19

            // count <= n
            lpush, 4,               // 20, 21
            lpush, 0,               // 22, 23
            leq,                    // 24
            brt, 28,                // 25, 26
            exit,                   // 27

            // print(next_num)
            lpush, 1,               // 28, 29
            print,                  // 30

            // num1 = num2
            lpush, 2,               // 31, 32
            lstore, 1,              // 33, 34

            // num2 = next_num
            lpush, 3,               // 35, 36
            lstore, 2,              // 37, 38

            // next_num = num1 + num2
            lpush, 1,               // 39, 40
            lpush, 2,               // 41, 42
            add,                    // 43
            lstore, 3,              // 44, 45

            // ++count
            lpush, 4,               // 46, 47
            inc,                    // 48
            lstore, 4,              // 49, 50

            // back to top of loop
            br, 20                  // 51, 52
        }
    }
    } };

    const Program* get_demo_program(int index) {
        if ((index < 0) || (index >= s_demo_programs.size())) {
            return nullptr;
        }

        return &(s_demo_programs[index]);
    }

    const Program* get_demo_program(std::string_view name) {
        for (const Program& program : s_demo_programs) {
            if (name == program.name) {
                return &program;
            }
        }

        return nullptr;
    }

    Demo_Program_Iterators get_demo_programs() {
        return { s_demo_programs.data(), s_demo_programs.data() + (s_demo_programs.size()) };
    }
}

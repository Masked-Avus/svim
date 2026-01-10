#pragma once

#include <array>
#include <string_view>

namespace svim {
    // IMPORTANT: For proper parsing and logging, the order of "Instruction" and
    //     "g_instruction_data" values must match.

    // Not an enum class for ease of conversion.
    enum Instruction {
        add,        // Pops 2 values off stack, adds X to Y, and pushes result.
        sub,        // Pops 2 values off stack, subtract X from Y, and pushes result.
        mul,        // Pops 2 values off stack, multiply X by Y, and pushes result.
        div,        // Pops 2 values off stack, divides X by Y, and pushes quotient.
        mod,        // Pops 2 values off stack, divides X by Y, and pushes remainder.
        inc,        // Adds 1 to the top value of the stack.
        dec,        // Subtracts 1 from the top value of the stack.
        neg,        // Negates the top value of the stack.
        lt,	        // Pops 2 values off stack and pushes non-0 number if operand X is less than operand Y; else, 0 is pushed.
        gt,         // Pops 2 values off stack and pushes non-0 number if operand X is greater than operand &; else, 0 is pushed.
        eq,	        // Pops 2 values off stack and pushes non-0 number if operand X is equal to operand Y; else, 0 is pushed.
        leq,        // Pops 2 values off stack and pushes non-0 number if operand X is less than or equal to operand Y; else, 0 is pushed.
        geq,        // Pops 2 values off stack and pushes non-0 number if operand X is greater than or equal to operand Y; else, 0 is pushed.
        neq,        // Pops 2 values off stack and pushes non-0 number if operand X is not equal to operand Y; else, 0 is pushed.
        br,	        // Unconditional jump to following instruction index.
        brt,	    // Pops 1 value off stack, and if it is not 0, branch to following instruction index.
        brf,	    // Pops 1 value off stack, and if it is 0, branch to following instruction index.
        push,	    // Pushes following integer onto stack.
        lpush,      // Pushes the local value associated with the following index found within the local call frame.
        gpush,      // Pushes the global value associated with the following index found within the global values storage.
        lstore,     // Pops 1 value off the stack and stores it at the following index found within the local call frame.
        gstore,     // Pops 1 value off the stack and stores it at the following index found within the global values.
        dup,        // Push a copy of the top of the stack.
        dup2,       // Push copies of the top 2 values of the stack in their original order.
        swap,       // Swap the values of the top 2 values of the stack.
        over,       // Pushes a copy of the value underneath the top of the stack.
        print,      // Pops 1 value off stack and prints it to designated output.
        pop,        // Pops 1 value off stack, with no further effect.
        turn,       // Rotates the top 3 values of the stack.
        halt,       // Pause program until a keyboard input.
        call,       // Pops the following destination address and number of arguments off the stack,
                    //     saves the original call frame, pops all arguments and stores them in a local stack frame,
                    //     and jumps to the destination address.
                    //     NOTE: For function calls, this instruction expects the function's arguments to be pushed onto the stack before using it.
        ret,        // Pops the current call frame and returns to the last jump point. Any new values on the stack are considered return values.
        exit        // Exit program.
    };

    // These values are here because we use them for our error-checking in Parser and, especially, Virtual_Machine.
    inline constexpr std::string_view g_add                     { "ADD" };
    inline constexpr std::string_view g_sub                     { "SUB" };
    inline constexpr std::string_view g_mul                     { "MUL" };
    inline constexpr std::string_view g_div                     { "DIV" };
    inline constexpr std::string_view g_mod                     { "MOD" };
    inline constexpr std::string_view g_inc                     { "INC" };
    inline constexpr std::string_view g_dec                     { "DEC" };
    inline constexpr std::string_view g_negate                  { "NEG" };
    inline constexpr std::string_view g_less_than               { "LT" };
    inline constexpr std::string_view g_greater_than            { "GT" };
    inline constexpr std::string_view g_equals                  { "EQ" };
    inline constexpr std::string_view g_less_than_or_equal      { "LEQ" };
    inline constexpr std::string_view g_greater_than_or_equal   { "GEQ" };
    inline constexpr std::string_view g_not_equal               { "NEQ" };
    inline constexpr std::string_view g_branch                  { "BR" };
    inline constexpr std::string_view g_branch_if_true          { "BRT" };
    inline constexpr std::string_view g_branch_if_false         { "BRF" };
    inline constexpr std::string_view g_push                    { "PUSH" };
    inline constexpr std::string_view g_local_push              { "LPUSH" };
    inline constexpr std::string_view g_global_push             { "GPUSH" };
    inline constexpr std::string_view g_local_store             { "LSTORE" };
    inline constexpr std::string_view g_global_store            { "GSTORE" };
    inline constexpr std::string_view g_duplicate               { "DUP" };
    inline constexpr std::string_view g_duplicate_twice         { "DUP2" };
    inline constexpr std::string_view g_swap                    { "SWAP" };
    inline constexpr std::string_view g_over                    { "OVER" };
    inline constexpr std::string_view g_print                   { "PRINT" };
    inline constexpr std::string_view g_pop                     { "POP" };
    inline constexpr std::string_view g_turn                    { "TURN" };
    inline constexpr std::string_view g_halt                    { "HALT" };
    inline constexpr std::string_view g_call                    { "CALL" };
    inline constexpr std::string_view g_ret                     { "RET" };
    inline constexpr std::string_view g_exit                    { "EXIT" };

    struct Instruction_Data {
        std::string_view name {};
        Instruction value {};
        // This counts how many bytecode values beyond the current instruction we expect for [this.value].
        int expected_following_values {};
    };

    inline constexpr std::array<const Instruction_Data, 33> g_instruction_data { {
        { g_add, Instruction::add, 0 },
        { g_sub, Instruction::sub, 0 },
        { g_mul, Instruction::mul, 0 },
        { g_div, Instruction::div, 0 },
        { g_mod, Instruction::mod, 0 },

        { g_inc, Instruction::inc, 0 },
        { g_dec, Instruction::dec, 0 },
        { g_negate, Instruction::neg, 0 },

        { g_less_than, Instruction::lt, 0 },
        { g_greater_than, Instruction::gt, 0 },
        { g_equals, Instruction::eq, 0 },
        { g_less_than_or_equal, Instruction::leq, 0 },
        { g_greater_than_or_equal, Instruction:: geq, 0 },
        { g_not_equal, Instruction::neq, 0 },

        { g_branch, Instruction::br, 1 },
        { g_branch_if_true, Instruction::brt, 1 },
        { g_branch_if_false, Instruction::brf, 1 },

        { g_push, Instruction::push, 1 },
        { g_local_push, Instruction::lpush, 1 },
        { g_global_push, Instruction::gpush, 1 },
        { g_local_store, Instruction::lstore, 1 },
        { g_global_store, Instruction::gstore, 1 },
        { g_duplicate, Instruction::dup, 0 },
        { g_duplicate_twice, Instruction::dup2, 0 },
        { g_swap, Instruction::swap, 0 },
        { g_over, Instruction::over, 0 },

        { g_print, Instruction::print, 0 },
        { g_pop, Instruction::pop, 0 },

        { g_turn, Instruction::turn, 0 },

        { g_halt, Instruction::halt, 0 },
        { g_call, Instruction::call, 2 },
        { g_ret, Instruction::ret, 0 },
        { g_exit, Instruction::exit, 0 }
    } };
}

#include "pch.h"
#include "virtual_machine.h"
#include "instructions.h"
#include "timer.h"
#include "application.h"
#include "debug.h"

namespace svim {
    //----------- Asserts

#if SVIM_DEBUG

#define SVIM_ASSERT_NO_UNDERFLOW(op_name, min_stack_size, actual_stack_size) \
    if (actual_stack_size < min_stack_size) { \
        std::ostringstream message {}; \
        message \
            << op_name \
            << ") Stack underflow encountered. Expected " \
            << min_stack_size \
            << " elements on the stack but found " \
            << actual_stack_size \
            << " instead."; \
        throw std::runtime_error(message.str()); \
    }

#define SVIM_ASSERT_WITHIN_GLOBALS_RANGE(op_name, index, global_values_count) \
    if (index >= global_values_count) { \
        std::ostringstream message {}; \
        message \
            << op_name \
            << ") Index " \
            << index \
            << " out of range of global value storage. (Global values count: " \
            << global_values_count << ")\n"; \
        throw std::runtime_error(message.str()); \
    }

#define SVIM_ASERT_WITHIN_LOCALS_RANGE(op_name, index, local_values_count) \
    if (index >= local_values_count) { \
        std::ostringstream message {}; \
        message \
            << op_name \
            << ") Index " \
            << index \
            << " out of range of local value storage. (Local values count: " \
            << local_values_count << ")\n"; \
        throw std::runtime_error(message.str()); \
    }

#define SVIM_ASSERT_WITHIN_CODE_RANGE(op_name, index, code_count) \
    if (index >= code_count) { \
        std::ostringstream message {}; \
        message \
            << op_name \
            << ") Index " \
            << index \
            << " out of range of bytecode storage. (Bytecode count: " \
            << code_count << ")\n"; \
        throw std::runtime_error(message.str()); \
    }

#define SVIM_ASSERT_WITHIN_RANGE_OF_STACK(op_name, index, stack_count) \
if (index >= stack_count) { \
        std::ostringstream message {}; \
        message \
            << op_name \
            << ") Index " \
            << index \
            << " out of range of the stack values. (Locals count: " \
            << stack_count << ")\n"; \
        throw std::runtime_error(message.str()); \
    }

#define SVIM_ASSERT_NON_ZERO_DENOMINATOR(denominator) \
    if (denominator == 0) { \
        throw std::runtime_error("Attempted to divide by 0."); \
    }

#else

#define SVIM_ASSERT_NO_UNDERFLOW(op_name, min_stack_size, actual_stack_size)
#define SVIM_ASSERT_WITHIN_GLOBALS_RANGE(op_name, index, global_value_count)
#define SVIM_ASERT_WITHIN_LOCALS_RANGE(op_name, index, local_values_count)
#define SVIM_ASSERT_WITHIN_CODE_RANGE(op_name, index, code_count)
#define SVIM_ASSERT_WITHIN_RANGE_OF_STACK(op_name, index, stack_count)
#define SVIM_ASSERT_NON_ZERO_DENOMINATOR(denominator)

#endif


    //----------- Global Values

    static constexpr std::size_t g_default_stack_capacity { 100 };
    static constexpr std::size_t g_default_global_values_capacity { 100 };
    static constexpr auto g_false { 0 };
    static constexpr auto g_true { 1 };


    //----------- Virtual_Machine

    Virtual_Machine::Virtual_Machine(std::vector<int>&& parsed_code, int program_starting_line, Logger* logger) :
        m_code { std::forward<std::vector<int>>(parsed_code) },
        m_stack(0),
        m_global_values(s_max_global_values),
        m_instruction_index { (program_starting_line >= 0) ? program_starting_line : 0 },
        m_logger { logger }
    {
        m_stack.reserve(g_default_stack_capacity);

        // This frame acts like an impromptu "main()" function.
        // If we "RET" from main_frame, we exit the program entirely.
        Call_Frame main_frame { static_cast<int>(m_code.size()), Call_Frame::s_max_local_values };
        m_call_stack.push(main_frame);

        SVIM_PRINT_LINE("Virtual machine instantiated.");
        SVIM_PRINT_PROPERTY("Bytecode size", m_code.size());
        SVIM_PRINT_PROPERTY("Trace mode", ((m_trace_mode) ? "On" : "Off"));
        SVIM_PRINT_PROPERTY("Program starting index", m_instruction_index);
        SVIM_PRINT_PROPERTY("Stack capacity", m_stack.capacity());
        SVIM_PRINT_PROPERTY("Global values capacity", m_global_values.capacity());
        SVIM_PRINT_PROPERTY("Call stack size", m_call_stack.size());
    }

    Application::Status Virtual_Machine::interpret() {
        if (m_code.size() == 0) {
            return Application::Status::success;
        }

        SVIM_PRINT_LINE("Interpreting...");

        while (m_instruction_index < m_code.size()) {
            if (m_trace_mode) {
                disassemble();
            }

            int op_code { m_code.at(m_instruction_index++) };

            switch (op_code) {
            case Instruction::add:
                add();
                break;

            case Instruction::sub:
                sub();
                break;

            case Instruction::mul:
                mul();
                break;

            case Instruction::div:
                div();
                break;

            case Instruction::mod:
                mod();
                break;

            case Instruction::inc:
                inc();
                break;

            case Instruction::dec:
                dec();
                break;

            case Instruction::neg:
                neg();
                break;

            case Instruction::lt:
                lt();
                break;

            case Instruction::gt:
                gt();
                break;

            case Instruction::eq:
                eq();
                break;

            case Instruction::leq:
                leq();
                break;

            case Instruction::geq:
                geq();
                break;

            case Instruction::neq:
                neq();
                break;

            case Instruction::br:
                br();
                break;

            case Instruction::brt:
                brt();
                break;

            case Instruction::brf:
                brf();
                break;

            case Instruction::push:
                push(next_instruction());
                break;

            case Instruction::lpush:
                lpush();
                break;

            case Instruction::gpush:
                gpush();
                break;

            case Instruction::lstore:
                lstore();
                break;

            case Instruction::gstore:
                gstore();
                break;

            case Instruction::dup:
                dup();
                break;
            
            case Instruction::dup2:
                dup2();
                break;
            
            case Instruction::swap:
                swap();
                break;
            
            case Instruction::over:
                over();
                break;

            case Instruction::print:
                SVIM_ASSERT_NO_UNDERFLOW(g_print, 1, m_stack.size());
                m_logger->log_value(pop());
                break;

            case Instruction::pop:
                SVIM_ASSERT_NO_UNDERFLOW(g_pop, 1, m_stack.size());
                pop();
                break;

            case Instruction::turn:
                turn();
                break;

            case Instruction::halt:
                std::cin.get();
                break;

            // This instruction expects all arguments for a function already on the stack.
            case Instruction::call:
                call();
                break;

            // This instruction expects any extra values on the stack to be removed.
            case Instruction::ret:
                ret();
                break;

            case Instruction::exit:
                run_exit_protocol();
                SVIM_PRINT_LINE("Interpreting complete...");
                return Application::Status::success;

            default:
                m_logger->output_invalid_op_code(op_code);
                SVIM_PRINT_LINE("Interpreting aborted...");
                return Application::Status::script_execution_failure;
            }

            if (m_trace_mode) {
                dump_stack();
                dump_locals();
            }
        }

        run_exit_protocol();
        SVIM_PRINT_LINE("Interpreting complete!");
        return Application::Status::success;
    }

    void Virtual_Machine::dump_stack() const {
        m_logger->log_stack(m_stack);
    }

    void Virtual_Machine::disassemble() const {
        m_logger->log_instruction(m_instruction_index, m_code, m_code.at(m_instruction_index));
    }

    void Virtual_Machine::dump_globals() const {
        m_logger->log_global_data(m_global_values);
    }

    void Virtual_Machine::dump_locals() const {
        const Call_Frame& current { m_call_stack.top() };
        m_logger->log_local_data(current.local_values, Call_Frame::s_max_local_values);
    }

    void Virtual_Machine::dump_bytecode() const {
        m_logger->log_compiled_source_code(m_code);
    }

    void Virtual_Machine::add() {
        SVIM_ASSERT_NO_UNDERFLOW(g_add, 2, m_stack.size());
        int b { pop() };
        int a { pop() };
        push(a + b);
    }

    void Virtual_Machine::sub() {
        SVIM_ASSERT_NO_UNDERFLOW(g_sub, 2, m_stack.size());
        int b { pop() };
        int a { pop() };
        push(a - b);
    }

    void Virtual_Machine::mul() {
        SVIM_ASSERT_NO_UNDERFLOW(g_mul, 2, m_stack.size());
        int b { pop() };
        int a { pop() };
        push(a * b);
    }

    void Virtual_Machine::div() {
        SVIM_ASSERT_NO_UNDERFLOW(g_div, 2, m_stack.size());
        int b { pop() };
        SVIM_ASSERT_NON_ZERO_DENOMINATOR(b);
        int a { pop() };
        push(a / b);
    }

    void Virtual_Machine::mod() {
        SVIM_ASSERT_NO_UNDERFLOW(g_div, 2, m_stack.size());
        int b { pop() };
        SVIM_ASSERT_NON_ZERO_DENOMINATOR(b);
        int a { pop() };
        push(a % b);
    }

    void Virtual_Machine::inc() {
        SVIM_ASSERT_NO_UNDERFLOW(g_inc, 1, m_stack.size());
        m_stack.back() = m_stack.back() + 1;
    }

    void Virtual_Machine::dec() {
        SVIM_ASSERT_NO_UNDERFLOW(g_dec, 1, m_stack.size());
        m_stack.back() = m_stack.back() - 1;
    }

    void Virtual_Machine::neg() {
        SVIM_ASSERT_NO_UNDERFLOW(g_negate, 1, m_stack.size());
        m_stack.back() = -m_stack.back();
    }

    void Virtual_Machine::lt() {
        SVIM_ASSERT_NO_UNDERFLOW(g_less_than, 2, m_stack.size());
        int b { pop() };
        int a { pop() };
        push((a < b) ? g_true : g_false);
    }

    void Virtual_Machine::gt() {
        SVIM_ASSERT_NO_UNDERFLOW(g_less_than, 2, m_stack.size());
        int b { pop() };
        int a { pop() };
        push((a > b) ? g_true : g_false);
    }

    void Virtual_Machine::eq() {
        SVIM_ASSERT_NO_UNDERFLOW(g_equals, 2, m_stack.size());
        int b { pop() };
        int a { pop() };
        push((a == b) ? g_true : g_false);
    }

    void Virtual_Machine::leq() {
        SVIM_ASSERT_NO_UNDERFLOW(g_less_than_or_equal, 2, m_stack.size());
        int b { pop() };
        int a { pop() };
        push((a <= b) ? g_true : g_false);
    }

    void Virtual_Machine::geq() {
        SVIM_ASSERT_NO_UNDERFLOW(g_greater_than_or_equal, 2, m_stack.size());
        int b { pop() };
        int a { pop() };
        push((a >= b) ? g_true : g_false);
    }

    void Virtual_Machine::neq() {
        SVIM_ASSERT_NO_UNDERFLOW(g_not_equal, 2, m_stack.size());
        int b { pop() };
        int a { pop() };
        push((a != b) ? g_true : g_false);
    }

    void Virtual_Machine::br() {
        int address { next_instruction() };
        SVIM_ASSERT_WITHIN_CODE_RANGE(g_call, address, m_code.size());
        jump_to(address);
    }

    void Virtual_Machine::brt() {
        int address { next_instruction() };
        SVIM_ASSERT_WITHIN_CODE_RANGE(g_call, address, m_code.size());

        SVIM_ASSERT_NO_UNDERFLOW(g_branch_if_true, 1, m_stack.size());

        if (pop() != g_false) {
            jump_to(address);
        }
    }

    void Virtual_Machine::brf() {
        int address { next_instruction() };
        SVIM_ASSERT_WITHIN_CODE_RANGE(g_call, address, m_code.size());

        SVIM_ASSERT_NO_UNDERFLOW(g_branch_if_false, 1, m_stack.size());

        if (pop() == g_false) {
            jump_to(address);
        }
    }

    void Virtual_Machine::lpush() {
        int index { next_instruction() };
        SVIM_ASERT_WITHIN_LOCALS_RANGE(g_local_push, index, Call_Frame::s_max_local_values);
        push(m_call_stack.top().local_values[index]);
    }

    void Virtual_Machine::gpush() {
        int index { next_instruction() };
        SVIM_ASSERT_WITHIN_GLOBALS_RANGE(g_global_push, index, m_global_values.size());
        push(m_global_values.at(index));
    }

    void Virtual_Machine::lstore() {
        SVIM_ASSERT_NO_UNDERFLOW(g_local_store, 1, m_stack.size());
        int index { next_instruction() };

        SVIM_ASERT_WITHIN_LOCALS_RANGE(g_local_store, index, Call_Frame::s_max_local_values);
        m_call_stack.top().local_values[index] = pop();
    }

    void Virtual_Machine::gstore() {
        SVIM_ASSERT_NO_UNDERFLOW(g_global_store, 1, m_stack.size());

        int index { next_instruction() };
        SVIM_ASSERT_WITHIN_GLOBALS_RANGE(g_global_store, index, m_global_values.size());

        m_global_values.at(index) = pop();
    }

    void Virtual_Machine::dup() {
        SVIM_ASSERT_NO_UNDERFLOW(g_duplicate, 1, m_stack.size());
        push(m_stack.back());
    }

    void Virtual_Machine::dup2() {
        SVIM_ASSERT_NO_UNDERFLOW(g_duplicate_twice, 2, m_stack.size());
        int under { m_stack.at(m_stack.size() - 2) };
        int top { m_stack.back() };
        push(under);
        push(top);
    }

    void Virtual_Machine::swap() {
        SVIM_ASSERT_NO_UNDERFLOW(g_swap, 2, m_stack.size());
        int temp { m_stack.at(m_stack.size() - 2) };
        m_stack.at(m_stack.size() - 2) = m_stack.back();
        m_stack.at(m_stack.size() - 1) = temp;
    }

    void Virtual_Machine::over() {
        SVIM_ASSERT_NO_UNDERFLOW(g_over, 2, m_stack.size());
        push(m_stack.at(m_stack.size() - 2));
    }

    int Virtual_Machine::pop() {
        int top { m_stack.back() };
        m_stack.pop_back();
        return top;
    }

    void Virtual_Machine::turn() {
        SVIM_ASSERT_NO_UNDERFLOW(g_turn, 3, m_stack.size());
        int temp { m_stack.at(m_stack.size() - 3) };
        m_stack.at(m_stack.size() - 3) = m_stack.at(m_stack.size() - 2);
        m_stack.at(m_stack.size() - 2) = m_stack.at(m_stack.size() - 1);
        m_stack.back() = temp;
    }

    void Virtual_Machine::call() {
        int destination_index { next_instruction() };
        SVIM_ASSERT_WITHIN_CODE_RANGE(g_call, destination_index, m_code.size());

        int arg_count { next_instruction() };
        SVIM_ASSERT_NO_UNDERFLOW(g_call, arg_count, m_stack.size());

        Call_Frame new_frame { m_instruction_index };

        for (int i {}; i < arg_count; ++i) {
            new_frame.local_values[i] = pop();
        }

        m_call_stack.push(new_frame);
        jump_to(destination_index);
    }

    void Virtual_Machine::ret() {
        jump_to(m_call_stack.top().return_index);
        m_call_stack.pop();
    }

    void Virtual_Machine::run_exit_protocol() const {
        if (m_trace_mode) {
            dump_stack();
            dump_globals();
            dump_locals();
        }
    }
}

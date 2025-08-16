#pragma once

#include <string>
#include <string_view>
#include <vector>
#include <stack>
#include <memory>
#include "application/application.h"
#include "io/logger.h"

namespace svim {
    class Virtual_Machine final {
    private:
        struct Call_Frame {
            inline static constexpr int s_max_local_values { 10 };

            int return_index {};
            int local_values[s_max_local_values] {};
        };

    public:
        Virtual_Machine(std::vector<int>&& parsed_code, int program_starting_line, Logger* logger);

        static constexpr int get_max_global_values() { return s_max_global_values; }
        static constexpr int get_max_local_values() { return Call_Frame::s_max_local_values; }

        void set_trace_mode(bool enabled) { m_trace_mode = enabled; }

        Application::Status interpret();

        void dump_bytecode() const;

        Virtual_Machine(const Virtual_Machine& other) = delete;
        Virtual_Machine& operator =(const Virtual_Machine& other) = delete;

    private:
        inline static constexpr int s_max_global_values { 100 };

        std::vector<int> m_code {};
        std::vector<int> m_stack {};
        std::vector<int> m_global_values {};
        std::stack<Call_Frame> m_call_stack {};

        int m_instruction_index {};

        std::unique_ptr<Logger> m_logger {};
        bool m_trace_mode {};

        void disassemble() const;
        void dump_globals() const;
        void dump_locals() const;
        void dump_stack() const;

        void add();
        void sub();
        void mul();
        void div();
        void mod();
        void inc();
        void dec();
        void neg();
        void lt();
        void gt();
        void eq();
        void leq();
        void geq();
        void neq();
        void br();
        void brt();
        void brf();
        void push(int value) { m_stack.push_back(value); }
        void lpush();
        void gpush();
        void lstore();
        void gstore();
        void dup();
        void dup2();
        void swap();
        void over();
        int pop();
        void turn();
        int next_instruction() { return m_code[m_instruction_index++]; }
        void jump_to(int address) { m_instruction_index = address; }
        void call();
        void ret();

        void run_exit_protocol() const;
    };
}

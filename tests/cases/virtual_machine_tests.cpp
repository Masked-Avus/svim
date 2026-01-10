#include "pch.h"
#include "virtual_machine_tests.h"
#include "virtual_machine/virtual_machine.h"
#include "virtual_machine/instructions.h"
#include "interpreter/application.h"
#include "interpreter/program.h"

namespace test {
    using namespace svim;

    static Demo_Program_Iterators g_demo_programs { get_demo_programs() };

    static void print_program(Application::Status result) {
        std::cout << "Program result: " << static_cast<int>(result) << '\n';
    }

    static void run_program(const Program* p_program) {
        if (p_program == nullptr) {
            std::cerr << "Valid program not given. Cannot run program.\n";
            return;
        }

        const Program& program { *p_program };

        // Copy by value so we maintain the example program's integrity.
        std::vector<int> bytecode { program.bytecode };
        int starting_index { program.starting_point };

        try {
            std::cout << "\n---------- " << program.name << '\n';
            Virtual_Machine vm { std::move(bytecode), starting_index, new Console_Logger() };
            vm.set_trace_mode(true);
            Application::Status result { vm.interpret() };
            print_program(result);
        }
        catch (const std::exception& exception) {
            std::cout << exception.what() << '\n';
        }
    }

    static void dump_code(const Program* p_program) {
        if (p_program == nullptr) {
            std::cerr << "Valid program not given. Cannot run dump code to console.\n";
            return;
        }

        const Program& program { *p_program };
        // Copy by value so we maintain the example program's integrity.
        std::vector<int> bytecode { program.bytecode };
        int starting_index { program.starting_point };

        try {
            Virtual_Machine vm { std::move(bytecode), starting_index, new Console_Logger() };
            vm.set_trace_mode(false);
            vm.dump_bytecode();
        }
        catch (const std::exception& exception) {
            std::cout << exception.what() << '\n';
        }
    }

    static void dump_to_file(const Program* p_program, std::string_view output_file) {
        if (p_program == nullptr) {
            std::cerr << "Valid program not given. Cannot run dump program to file.\n";
            return;
        }

        const Program& program { *p_program };
        // Copy by value so we maintain the example program's integrity.
        std::vector<int> bytecode { program.bytecode };
        int starting_index { program.starting_point };

        try {
            Virtual_Machine vm { std::move(bytecode), 0, new File_Logger(output_file) };
            vm.set_trace_mode(true);
            Application::Status result { vm.interpret() };
            print_program(result);
        }
        catch (const std::exception& exception) {
            std::cout << exception.what() << '\n';
        }
    }

    void run_basic_instructions() {
        run_program(get_demo_program(0));
    }

    void run_branching_instructions() {
        run_program(get_demo_program(1));
    }

    void run_loop() {
        run_program(get_demo_program(2));
    }

    void run_function() {
        run_program(get_demo_program(3));
    }

    void run_factorial() {
        run_program(get_demo_program(4));
    }

    void run_fibonacci() {
        run_program(get_demo_program(5));
    }

    void output_to_file() {
        constexpr std::string_view test_output { "test_dump.txt" };
        dump_to_file(get_demo_program(3), test_output);
    }

    void dump_code_to_console() {
        dump_code(get_demo_program(0));
    }
}

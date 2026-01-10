#include "pch.h"
#include "parser_tests.h"
#include "example_files.h"
#include "virtual_machine/parser.h"
#include "virtual_machine/virtual_machine.h"

namespace test {
    using namespace svim;

    void output_parser() {
        try {
            Parser parser { g_test_file_1 };
            std::vector<int> bytecode { parser.parse() };

            std::cout
                << "Bytecode\n"
                << "----------\n"
                << "Count: " << bytecode.size()
                << "\n----------\n"
                << "Starting point: " << parser.get_program_start_index()
                << "\n----------\n";

            for (std::size_t i {}; i < bytecode.size(); ++i) {
                std::cout << i << ") " << bytecode.at(i) << '\n';
            }
        }
        catch (const std::exception& exception) {
            std::cout << exception.what() << '\n';
        }
    }

    void run_parsed_code() {
        try {
            Parser parser { g_test_file_1 };
            std::vector<int> bytecode { parser.parse() };

            Virtual_Machine vm { std::move(bytecode), parser.get_program_start_index(), new Console_Logger()};
            
            Application::Status result { vm.interpret() };

            std::cout << "Program Result: " << +result << '\n';
        }
        catch (const std::exception& exception) {
            std::cout << exception.what() << '\n';
        }
    }

    void fail_to_load_nonsvim_file() {
        try {
            Parser parser { g_bad_file };
        }
        catch (const std::exception& exception) {
            std::cout << exception.what() << '\n';
        }
    }
}

#include "pch.h"
#include "virtual_machine_tests.h"
#include "parser_tests.h"
#include "application_tests.h"

static void space() {
    std::cout << "\n\n\n\n\n";
}

int main(int argc, const char* argv[]) {
    /* Virtual Machine */ {
        test::run_basic_instructions();
        space();
        test::run_branching_instructions();
        space();
        test::run_loop();
        space();
        test::run_function();
        space();
        test::run_factorial();
        space();
        test::run_fibonacci();
        space();
        test::output_to_file();
        space();
        test::dump_code_to_console();
        space();
    }

    /* Parser */ {
        test::output_parser();
        space();
        test::run_parsed_code();
        space();
        test::fail_to_load_nonsvim_file();
    }

    /* Application */ {
        test::print_help();
        space();
        test::run_program_to_console();
        space();
        test::run_program_to_file();
        space();
        test::run_example_program();
        space();
        test::dump_code_to_file();
        space();
        
        test::empty_command_args_1();
        space();
        test::empty_command_args_2();
        space();
        test::empty_command_args_3();
        space();
        test::empty_command_args_4();
        space();
        test::bad_command_args_1();
        space();
        test::bad_command_args_2();
        space();
        test::bad_command_args_3();
        space();
    }

    return 0;
}

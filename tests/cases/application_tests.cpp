#include "tests_pch.h"
#include "application_tests.h"
#include "example_files.h"
#include "application/application.h"

namespace test {
    using namespace svim;

    static constexpr const char g_executable_name[] { "svim.exe" };

    static void create_and_run_app(std::string_view application_name, const char* argv[], int argc) {
        try {
            Application app { argv, argc };
            int exit_code { app.run() };
            std::cout << "Test application \"" << application_name << " \" exited with code " << exit_code << ".\n";
        }
        catch (const std::exception& exception) {
            std::cerr << exception.what() << '\n';
        }
        catch (...) {
            std::cerr << "Unknown exception occurred!\n";
        }
    }

    void print_help() {
        const char* argv_1[] { g_executable_name, "-h", "Superfluous_Arg" };
        create_and_run_app("Print_Help", argv_1, 3);

        std::cout << '\n';

        const char* argv_2[] { g_executable_name, "-h" };
        create_and_run_app("Print_Help", argv_1, 2);
    }

    void run_program_to_console() {
        const char* argv[] { g_executable_name, "-c", g_test_file_1.data() };
        create_and_run_app("Run_To_Console", argv, 3);
    }

    void run_program_to_file() {
        const char* argv[] { g_executable_name,  "-f", g_test_file_1.data() };
        create_and_run_app("Run_To_File", argv, 3);
    }

    void dump_code_to_file() {
        const char* argv[] { g_executable_name,  "-d", g_test_file_1.data() };
        create_and_run_app("Dump_To_File", argv, 3);
    }

    void run_example_program() {
        const char* argv[] { g_executable_name,  "-e", "basics" };
        create_and_run_app("Run_Example_Programs", argv, 3);
    }

    void empty_command_args_1() {
        const char* bad_argv[] { g_executable_name, "", "" };
        create_and_run_app("Test_With_Empty_Commands", bad_argv, 3);
    }

    void empty_command_args_2() {
        const char* bad_argv[] { g_executable_name, "-c", "" };
        create_and_run_app("Test_With_Empty_File_Name", bad_argv, 3);
    }

    void empty_command_args_3() {
        const char* bad_argv[] { g_executable_name, "", "test_source.svim" };
        create_and_run_app("Test_With_Empty_Option", bad_argv, 3);
    }

    void empty_command_args_4() {
        const char* bad_argv[] { "", "-c", "test_source.svim" };
        create_and_run_app("Test_With_Empty_Program_Name", bad_argv, 3);
    }

    void bad_command_args_1() {
        const char* bad_argv[] { g_executable_name, "-c", "not_svim.txt" };
        create_and_run_app("Incorrect_File_Input", bad_argv, 3);
    }

    void bad_command_args_2() {
        const char* bad_argv[] { g_executable_name, "-c" };
        create_and_run_app("Invalid_Number_Of_Args", bad_argv, 2);
    }

    void bad_command_args_3() {
        const char* bad_argv[] { g_executable_name, "-e", "notrealprogram" };
        create_and_run_app("Invalid_Number_Of_Args", bad_argv, 3);
    }
}

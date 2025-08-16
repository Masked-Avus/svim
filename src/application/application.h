#pragma once

#include <vector>
#include <string>
#include <string_view>
#include <memory>

namespace svim {
    struct Parse_Result;
    class Logger;

    class Application final {
    public:
        enum class Status {
            success = 0,
            file_not_found_error = 2,
            demo_program_not_found_error = 2,
            parse_error = 11,
            invalid_file_format = 11,
            invalid_command_line_args_error = 87,
            file_open_error = 110,
            invalid_command_execution_state = 186,
            script_execution_failure = -1,
            unknown_error = -2
        };

        enum class Process {
            read_command,
            print_help,
            output_console,
            output_file,
            dump_code,
            demo_program,
            done,
            abort
        };

        Application(const char** command_line_args, int command_line_arg_count) noexcept;

        int run();

        Application(const Application& other) = delete;
        Application& operator =(const Application& other) = delete;

    private:
        Status m_status { Status::success };
        Process m_process { Process::read_command };
        std::vector<std::string_view> m_command_line_args {};
        std::string m_input_file {};
        std::string m_output_file {};
        bool m_trace_mode {};

        Process parse_option();
        Status parse_io_files();
        Status execute_command();
        Status set_input_file();
        Status set_output_file();
        void set_trace_mode();

        Status run_user_program();
        Status run_demo_program();
        Status dump_parsed_source();

        Parse_Result run_parser(std::string_view file_name) const;
        Application::Status run_interpreter(
            std::vector<int>&& compiled_source_code,
            int program_starting_point,
            std::unique_ptr<Logger>&& logger
            ) const;
    };

    // A convenient shorthand for casting Application::Status values into integers.
    inline int operator +(Application::Status status) { return static_cast<int>(status); }
}


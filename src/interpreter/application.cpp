#include "pch.h"
#include "application.h"
#include "program.h"
#include "virtual_machine/virtual_machine.h"
#include "virtual_machine/parser.h"
#include "virtual_machine/instructions.h"
#include "common/format.h"
#include "common/error.h"
#include "common/timer.h"
#include "common/debug.h"

namespace svim {
    //----------- Internal Types

    struct Command final {
        static constexpr char s_option_prefix { '-' };
        static constexpr int s_option_size { 2 };
        static constexpr int s_minimum_arg_count { 2 };
        static constexpr int s_maximum_arg_count { 3 };

        std::string_view name {};
        Application::Process process {};
        std::string_view description {};

        static constexpr bool is_within_arg_range(std::size_t arg_count) {
            return (arg_count >= s_minimum_arg_count) && (arg_count <= s_maximum_arg_count);
        }

        static constexpr bool is_properly_formatted_option(std::string_view option) {
            return (option.size() == Command::s_option_size) && (option[0] == Command::s_option_prefix);
        }
    };

    struct Parse_Result final {
        std::vector<int> bytecode {};
        Parser::Status status {};
        int program_starting_index {};
    };

    static const std::array<Command, 5> s_options { {
            { "-h", Application::Process::print_help,       "print available options (no 'source_file' necessary)" },
            { "-c", Application::Process::output_console,   "run 'source_file,' outputting to console" },
            { "-f", Application::Process::output_file,      "run 'source_file,' outputting to file" },
            { "-d", Application::Process::dump_code,        "parse 'source_file' without running, outputting parsed contents to file" },
            { "-e", Application::Process::demo_program,     "run example_program, outputting to console in trace mode" }
        } };


    //----------- Helper Functions

    static void print_help() {
        std::cout << "Command line format: option source_file [output_file/example_program]\n";

        for (const Command& command : s_options) {
            std::cout << '\t' << command.name << " (" << command.description << ")\n";
        }
    }

    static void print_elapsed_time(std::string_view subject, Milliseconds start, Milliseconds end) {
        std::cout << '\t' << subject << " duration = " << end - start << " ms\n";
    }

    
    //----------- Application

    Application::Application(const char** command_line_args, int command_line_arg_count) noexcept
    {
        m_command_line_args.reserve(command_line_arg_count);

        SVIM_PRINT_LINE("Command line arguments entered:");

        for (int i {}; i < command_line_arg_count; ++i) {
            m_command_line_args.emplace_back(*(command_line_args + i));

            SVIM_PRINT_TAB_LINE(*(command_line_args + i));
        }

        SVIM_PRINT_LINE("Application initalized.");
    }

    int Application::run() {
        SVIM_PRINT_LINE("Running application...");

        SVIM_PRINT_LINE("Parsing options...");
        m_process = parse_option();

        if ((m_status == Status::invalid_command_line_args_error) || (m_process == Process::abort)) {
            return +m_status;
        }

        SVIM_PRINT_LINE("Parsing I/0 files...");
        m_status = parse_io_files();

        if (m_status != Status::success) {
            return +m_status;
        }

        SVIM_PRINT_LINE("Setting trace mode...");
        set_trace_mode();

        SVIM_PRINT_LINE("Executing commmand");
        m_status = execute_command();

        SVIM_PRINT_LINE("Application done.");
        return +m_status;
    }

    Application::Status Application::execute_command() {
        switch (m_process) {
        case Process::output_console:
        case Process::output_file:
            m_status = run_user_program();
            break;

        case Process::dump_code:
            m_status = dump_parsed_source();
            break;

        case Process::demo_program:
            m_status = run_demo_program();
            break;

        case Process::print_help:
            print_help();
            m_status = Status::success;
            break;

        default:
            m_status = Status::invalid_command_execution_state;
            break;
        }

        m_process = Process::done;
        return m_status;
    }

    Application::Process Application::parse_option() {
        if (!Command::is_within_arg_range(m_command_line_args.size())) {
            std::cerr
                << "Invalid number of command line arguments (minimum = "
                << Command::s_minimum_arg_count
                << "; maximum = "
                << Command::s_maximum_arg_count << ").\n";
            m_status = Status::invalid_command_line_args_error;
            return Process::abort;
        }

        std::string_view option_entry { m_command_line_args[1] };

        if (!Command::is_properly_formatted_option(option_entry)) {
            std::cerr
                << "Second command line argument (application mode) must consist of "
                << Command::s_option_size << " characters, with the first one being '"
                << Command::s_option_prefix << ".'\n";
            m_status = Status::invalid_command_line_args_error;
            return Process::abort;
        }

        for (const Command& command : s_options) {
            if (option_entry == command.name) {
                return command.process;
            }
        }

        m_status = Status::invalid_command_line_args_error;
        std::cerr
            << "Invalid application option \""
            << option_entry
            << "\" given. Enter \""
            << s_options[0].name
            << "\" to show available options.\n";

        return Process::abort;
    }

    Application::Status Application::parse_io_files() {
        switch (m_process) {
        case Process::output_console:
            return set_input_file();

        case Process::output_file:
        case Process::dump_code:
        {
            Status input_file_status { set_input_file() };
            
            if (input_file_status != Status::success) {
                return input_file_status;
            }

            Status output_file_status { set_output_file() };

            if (output_file_status != Status::success) {
                return output_file_status;
            }

            return Status::success;
        }

        case Process::print_help:
            return Status::success;

        case Process::demo_program:
            return Status::success;

        default:
            return Status::invalid_command_line_args_error;
        }
    }

    void Application::set_trace_mode() {
#if !SVIM_DEBUG
        m_trace_mode = ((m_process == Process::output_file) ||
            (m_process == Process::dump_code) ||
            (m_process == Process::demo_program));
#else
        m_trace_mode = true;
#endif
    }

    Application::Status Application::set_input_file() {
        if (m_command_line_args.size() < Command::s_maximum_arg_count) {
            std::cerr << "Too few command line arguments given for operation.\n";
            return Status::invalid_command_line_args_error;
        }

        std::string_view input_file = m_command_line_args[2];

        File_Name_Formatting status { is_source_file(input_file) };

        switch (status) {
        case File_Name_Formatting::good:
            m_input_file = input_file;
            return Status::success;

        case File_Name_Formatting::name_too_short:
            std::cerr
                << "Invalid input file name entered. "
                << "SVIM files must have at least one character for the file extension, plus the \""
                << g_svim_file_extension
                << "\" extension.\n";
            return Status::invalid_file_format;

        case File_Name_Formatting::no_extension_delimiter:
            std::cerr << "File extension could not be found. Perhaps a '.' is missing.\n";
            return Status::invalid_file_format;

        case File_Name_Formatting::incorrect_extension:
            std::cerr << "Incorrect file extension. Target files must have a \".svim\" extension at the end.\n";
            return Status::invalid_file_format;

        case File_Name_Formatting::unallowed_characters:
            std::cerr
                << "Invalid character detected in source file name."
                << "Source file names can only contain alphabetic characters, digits, and '_.' "
                << "(Not including the '.' separator for file extensions.)";
            return Status::invalid_file_format;

        default:
            std::cerr << "Unknown error occurred when attempting to set input file.\n";
            return Status::unknown_error;
        }
    }

    Application::Status Application::set_output_file() {
        try {
            if (m_process == Process::output_file) {
                m_output_file = create_log_file(m_input_file);
                return Status::success;
            }
            else if (m_process == Process::dump_code) {
                m_output_file = create_code_dump_file(m_input_file);
                return Status::success;
            }
            else {
                std::cerr
                    << "Incorrect process setup for outputting to a file."
                    << "Can only perform file ouput operation when dumping parsed source code or when running a program with file-based output.\n";
                return Status::invalid_command_execution_state;
            }
        }
        catch (const std::runtime_error& exception) {
            std::cerr << exception.what() << '\n';
            return Status::invalid_file_format;
        }
        catch (...) {
            std::cerr << "Unknown exception occurred during attempt to set output file.\n";
            return Status::unknown_error;
        }
    }

    Application::Status Application::dump_parsed_source() {
#if SVIM_DEBUG
        Milliseconds start { get_current_time() };

        Parse_Result parser_result { run_parser(m_input_file) };

        Milliseconds end { get_current_time() };
        print_elapsed_time("Parser", start, end);
#else
        Parse_Result parser_result { run_parser(m_input_file) };
#endif

        if (parser_result.status != Parser::Status::success) {
            m_status = Application::Status::parse_error;
            return m_status;
        }

        try {
            File_Logger output { m_output_file };
            output.log_compiled_source_code(parser_result.bytecode);

            return Application::Status::success;
        }
        catch (const File_Open_Failure& exception) {
            std::cerr << exception.what() << '\n';
            return Application::Status::file_open_error;
        }
        catch (...) {
            std::cerr
                << "Unknown exception occured during attempt to output compiled source code to file \""
                << m_input_file << ".\"";
            return Application::Status::file_open_error;
        }
    }

    Application::Status Application::run_user_program() {
#if SVIM_DEBUG
        Milliseconds start { get_current_time() };

        Parse_Result parser_result { run_parser(m_input_file) };

        Milliseconds end { get_current_time() };
        print_elapsed_time("Parser", start, end);
#else
        Parse_Result parser_result { run_parser(m_input_file) };
#endif

        if (parser_result.status != Parser::Status::success) {
            m_status = Application::Status::parse_error;
            return m_status;
        }

        try {
            std::unique_ptr<Logger> logger {
                (m_process == Process::output_console)
                ? static_cast<Logger*>(new Console_Logger())
                : static_cast<Logger*>(new File_Logger(m_output_file))
            };

            return run_interpreter(std::move(parser_result.bytecode), parser_result.program_starting_index, std::move(logger));
        }
        catch (const File_Open_Failure& exception) {
            std::cerr << exception.what() << '\n';
            return Application::Status::file_open_error;
        }
        catch (...) {
            std::cerr << "Unknown exception occurred during run user program phase.\n";
            return Application::Status::unknown_error;
        }
    }

    Application::Status Application::run_demo_program() {
        if (m_command_line_args.size() < Command::s_maximum_arg_count) {
            std::cerr << "Too few command line arguments given for operation.\n";
            return Status::invalid_command_line_args_error;
        }

        std::string_view demo_name { m_command_line_args[2] };

        const Program* match {};
        Demo_Program_Iterators demo_programs { get_demo_programs() };

        for (const Program* current { demo_programs.start }; current != demo_programs.end; ++current) {
            if ((current != nullptr) && (demo_name == current->name)) {
                match = current;
                break;
            }
        }

        if (match) {
            // We intentionally copy the vector rather than pass by lvalue/rvalue reference because
            //     of the move semantics inside of Virtual_Machine's constructor,
            //     and we wish to maintain the integrity of the demo program's pre-parsed source code.
            std::vector<int> bytecode { match->bytecode };

            return run_interpreter(std::move(bytecode), match->starting_point, std::move(std::make_unique<Console_Logger>()));
        }
        else {
            std::cerr
                << "Application does not contain demo program \""
                << demo_name << ".\"\n"
                << "Example programs available (enter name in command line to run):\n";

            for (const Program* current { demo_programs.start }; current != demo_programs.end; ++current) {
                if (current != nullptr) {
                    std::cerr << "    " << current->name << '\n';
                }
            }

            return Status::demo_program_not_found_error;
        }
    }

    Parse_Result Application::run_parser(std::string_view file_name) const {
        constexpr int no_entry_point { -1 };

        try {
            Parser parser { file_name };
            std::vector<int> bytecode { parser.parse() };

            return { bytecode, parser.get_status(), parser.get_program_start_index() };
        }
        catch (const Bad_Parse& exception) {
            std::cerr << exception.what() << '\n';
            return { {}, exception.get_parser_status(), no_entry_point };
        }
        catch (const File_Open_Failure& exception) {
            std::cerr << exception.what() << '\n';
            return { {}, Parser::Status::file_opening_failure, no_entry_point };
        }
        catch (...) {
            std::cerr << "Unknown exception encountered during parsing phase.\n";
            return { {}, Parser::Status::unknown_failure, no_entry_point };
        }
    }

    Application::Status Application::run_interpreter(
        std::vector<int>&& compiled_source_code,
        int program_starting_point,
        std::unique_ptr<Logger>&& logger
        ) const {

        try {
            Virtual_Machine vm {
                std::move(compiled_source_code),
                program_starting_point,
                logger.release()
            };
            vm.set_trace_mode(m_trace_mode);

#if SVIM_DEBUG
            Milliseconds start { get_current_time() };
            
            Status result { vm.interpret() };
            
            Milliseconds end { get_current_time() };
            print_elapsed_time("Program", start, end);
#else
            Status result { vm.interpret() };
#endif

            if ((result == Status::success) && m_trace_mode) {
                vm.dump_bytecode();
            }

            return result;
        }
        catch (const std::runtime_error& exception) {
            std::cerr << exception.what() << '\n';
            return Status::script_execution_failure;
        }
        catch (const std::out_of_range& exception) {
            std::cerr << exception.what() << '\n';
            return Status::script_execution_failure;
        }
        catch (...) {
            std::cerr << "Unknown exception occured during the virtual machine runtime.\n";
            return Status::script_execution_failure;
        }
    }
}

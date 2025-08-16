#include "pch.h"
#include "logger.h"
#include "instructions.h"
#include "error.h"

namespace svim {
    //-------------------- Internal Data

    static constexpr char ln { '\n' };
    static constexpr std::string_view g_space { "    " };
    static constexpr std::string_view g_value_separator { ": " };

    static constexpr std::string_view g_stack_prologue { "\n\tStack=[" };
    static constexpr std::string_view g_stack_epilogue { "]\n\n" };

    static constexpr std::string_view g_globals_prologue { "\n\tGlobals=[" };
    static constexpr std::string_view g_globals_epilogue { "]\n\n" };

    static constexpr std::string_view g_locals_prologue { "\n\tLocals=[" };
    static constexpr std::string_view g_locals_epilogue { "]\n\n" };

    static constexpr std::string_view g_code_header { "\n\tSource Code Values\n\t---------\n" };


    //-------------------- Helper Functions

    static int peek_next(int instruction_index, const int* bytecode_start) { return *(bytecode_start + instruction_index + 1); }
    static int peek_over(int instruction_index, const int* bytecode_start) { return *(bytecode_start + instruction_index + 2); }

    static void log_array(std::ostream& output, std::string_view prologue,
                          const int* data, const std::size_t count, std::string_view epilogue) {
        output << g_space << prologue;

        for (int i {}; i < count; ++i) {
            if ((i > 0) && (i < count)) {
                output << ',';
            }

            output << i << '=' << data[i];
        }

        output << epilogue;
    }


    //-------------------- Logger

    void Logger::log_value(int value) {
        get_output() << value << ln;
    }

    void Logger::log_instruction(int instruction_index, const std::vector<int>& bytecode, int op_code) {
        if (op_code >= g_instruction_data.size()) {
            return;
        }

        const Instruction_Data& instruction { g_instruction_data[op_code] };

        get_output()
            << "Instruction "
            << instruction.name
            << " (" << op_code << "): Index "
            << instruction_index << ln;

        if (instruction.expected_following_values == 1) {
            get_output()
                << g_space
                << "Next: "
                << peek_next(instruction_index, bytecode.data()) << ln;
        }
        else if (instruction.expected_following_values == 2) {
            get_output()
                << g_space
                << "Next: "
                << peek_next(instruction_index, bytecode.data())
                << ','
                << peek_over(instruction_index, bytecode.data()) << ln;
        }
    }

    void Logger::log_global_data(const std::vector<int>& global_data) {
        log_array(get_output(), g_globals_prologue, global_data.data(), global_data.size(), g_globals_epilogue);
    }

    void Logger::log_local_data(const int* data, int max_data_capacity) {
        log_array(get_output(), g_locals_prologue, data, max_data_capacity, g_globals_epilogue);
    }

    void Logger::log_stack(const std::vector<int>& stack) {
        log_array(get_output(), g_stack_prologue, stack.data(), stack.size(), g_stack_epilogue);
    }

    void Logger::log_compiled_source_code(const std::vector<int>& compiled_code) {
        get_output() << g_code_header;

        for (int i {}; i < compiled_code.size(); ++i) {
            if (compiled_code[i] >= 0) {
                get_output() << '\t' << i << ": " << compiled_code[i] << ln;
            }
        }

        get_output() << ln;
    }

    void Logger::output_invalid_op_code(int bad_op_code) {
        get_output() << "Invalid operation code \"" << bad_op_code << '\"' << ln;
    }


    //-------------------- Console_Logger

    Console_Logger::Console_Logger() noexcept : m_output { &std::cout } {}


    //-------------------- File_Logger

    File_Logger::File_Logger(std::string_view out_file) :
        m_file_name { out_file },
        m_output { m_file_name }
    {
        if (m_output.bad()) {
            std::stringstream message {};
            message << "Could not access file \"" << out_file << ".\"";
            throw File_Open_Failure(message.str());
        }
        else if (!m_output.is_open()) {
            std::stringstream message {};
            message << "Could not open file \"" << out_file << ".\"";
            throw File_Open_Failure(message.str());
        }
    }
}

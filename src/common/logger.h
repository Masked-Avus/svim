#pragma once

#include <vector>
#include <string>
#include <string_view>
#include <ostream>
#include <fstream>

namespace svim {
    class Logger {
    public:
        virtual void log_value(int value);
        virtual void log_instruction(int instruction_index, const std::vector<int>& bytecode, int op_code);
        virtual void log_global_data(const std::vector<int>& global_data);
        virtual void log_local_data(const int* data, int max_data_capacity);
        virtual void log_stack(const std::vector<int>& stack);
        virtual void log_compiled_source_code(const std::vector<int>& compiled_code);
        virtual void output_invalid_op_code(int bad_op_code);

        virtual ~Logger() = default;

    protected:
        virtual std::ostream& get_output() = 0;
    };


    class Console_Logger : public Logger {
    public:
        Console_Logger() noexcept;

    protected:
        std::ostream& get_output() override { return *m_output; }

    private:
        std::ostream* m_output;
    };


    class File_Logger : public Logger {
    public:
        File_Logger(std::string_view out_file);

        // No reason to allow access to the file stream it controls.
        File_Logger(const File_Logger& other) = delete;
        File_Logger& operator =(const File_Logger& other) = delete;

    protected:
        std::ostream& get_output() override { return m_output; }

    private:
        std::string m_file_name {};
        std::ofstream m_output {};
    };
}

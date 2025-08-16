#include "pch.h"
#include "parser.h"
#include "instructions.h"
#include "error.h"
#include "virtual_machine.h"
#include "format.h"
#include "debug.h"

/*---------- Use of Parser::m_expected_integer_count

Its value is determined by the number of numeric operands taken by the last parsed instruction.
Parsing branches off based on this field's value.
    1) If it is 0, we expect an instruction.
    2) If it is greater than 0, we expect an integer literal. Each integer we parse successfully will decrement the counter.
    3) If it is less than 0, we are in an invalid parsing state and should abort the operation.

Parsing fails if we do not find the expected token.

NOTE: This field does NOT count the operands off the stack that we expect from the stack or elsewhere
      but instead the number of literals we expect after the instruction within the source code.

---------- */

namespace svim {
    inline constexpr std::string_view g_init { "INIT" };

    Parser::Parser(std::string_view source_file) {
        if (is_source_file(source_file) != File_Name_Formatting::good) {
            m_status = Status::improper_file_format;
            throw File_Open_Failure("Must give a \".svim\" file as the source.");
        }

        m_source_file = source_file;

        SVIM_PRINT_LINE("Parser instantiated.");
        SVIM_PRINT_PROPERTY("Source file", m_source_file);

        if (m_input.bad()) {
            std::ostringstream message {};
            message
                << "Could not access file \""
                << source_file <<
                ".\"";

            m_status = Status::failure_on_initialization;
            throw File_Open_Failure(message.str());
        }

        m_status = Status::ready;
    }

    bool Parser::expecting() const {
        return (m_expected_operand_count == 0) ||
                m_entry_point_status == Entry_Point_Search_Status::expecting;
    }

    bool Parser::expecting_operand() const {
        return m_expected_operand_count > 0;
    }

    std::vector<int> Parser::parse() {
        if (m_status != Status::ready) {
            throw Bad_Parse("Parser not in ready state.", m_status);
        }

        constexpr std::size_t initial_capacity { 100 };
        std::vector<int> bytecode {};
        bytecode.reserve(initial_capacity);

        open_source_file();

        std::string line {};
        m_expected_operand_count = 0;
        m_line_count = 0;

        SVIM_PRINT_LINE("Parsing...");

        while (!m_input.eof()) {
            std::getline(m_input, line);
            make_uppercase(line);
            ++m_line_count;

#if SVIM_DEBUG
            std::cout << "\tCurrent line (" << m_line_count << "): " << line << '\n';
#endif

            m_current_character = line.data();
            m_line_view = line;

            parse_line(bytecode, line);
        }

        m_input.close();

        check_remaining_operand_count();
        check_program_start_index();

        SVIM_PRINT_LINE("Parsing complete!");
        m_status = Status::success;
        return bytecode;
    }

    void Parser::validate_operand(int value) {
        if ((value < 0) && (m_last_instruction != Instruction::push)) {
            m_status = Status::bad_operand;
            std::ostringstream message {};
            message
                << "Use of negative value "
                << value
                << " with non-"
                << g_push
                << " instruction. Operands for other instructions must be non-negative.";
            throw Bad_Parse(m_line_count, message.str(), m_status);
        }
        else if ((m_last_instruction == Instruction::call) && (m_expected_operand_count == 1)) {
            assert_within_range(value, Virtual_Machine::get_max_local_values(), true);
        }
        else if ((m_last_instruction == Instruction::lstore) || (m_last_instruction == Instruction::lpush)) {
            assert_within_range(value, Virtual_Machine::get_max_local_values(), true);
        }
        else if ((m_last_instruction == Instruction::gstore) || (m_last_instruction == Instruction::gpush)) {
            assert_within_range(value, Virtual_Machine::get_max_global_values(), false);
        }
    }

    void Parser::parse_line(std::vector<int>& bytecode, const std::string& line) {
        const char* const line_end { line.data() + line.size() };

        while (m_current_character != line_end) {
            skip_whitespace();

            if (is_endline(*m_current_character) || (m_line_view.size() == 0)) {
                // Skip to next line in code if we find a comment.
                break;
            }

            if (is_keyword_prefix(*m_current_character)) {
                next_character();
                std::string_view token { parse_keyword() };
                SVIM_PRINT_DPROPERTY("Keyword token", token);
                process_keyword(token);
            }
            else if (expecting()) {
                std::string_view token { parse_instruction() };
                SVIM_PRINT_DPROPERTY("Instruction token", token);

                int op_code { to_instruction(token) };
                bytecode.push_back(op_code);

                if (m_entry_point_status == Entry_Point_Search_Status::expecting) {
                    m_program_start_index = static_cast<int>(bytecode.size() - 1);
                    m_entry_point_status = Entry_Point_Search_Status::found;
                }

                m_last_instruction = op_code;
            }
            else if (expecting_operand()) {
                std::string_view token { parse_operand() };
                SVIM_PRINT_DPROPERTY("Integer literal token", token);

                int value { to_operand(token) };
                validate_operand(value);

                bytecode.push_back(value);
                --m_expected_operand_count;
            }
            // This is to protect against potential negative operand counts.
            else {
                m_status = Status::unknown_failure;
                throw Bad_Parse(m_line_count, "Parser operand tracker in unknown state!", m_status);
            }
        }
    }

    void Parser::assert_within_range(int index, int max_values, bool refers_to_local_value) {
        if ((index >= max_values) || (index < 0)) {
            m_status = Status::bad_operand;
            std::ostringstream message {};
            message
                << g_instruction_data.at(m_last_instruction).name
                << ") Index operand "
                << index
                << " strays outside range of "
                << ((refers_to_local_value) ? "local" : "global")
                << " values. (Range: 0-"
                << max_values
                << ").";
            throw Bad_Parse(m_line_count, message.str(), m_status);
        }
    }

    void Parser::open_source_file() {
        m_input.open(m_source_file);

        if (!m_input.is_open()) {
            m_status = Status::file_opening_failure;
            std::ostringstream message {};
            message << "Attempting to open non-existent file. Make sure target file exists in Directory.";
            throw File_Open_Failure(message.str());
        }
    }

    void Parser::next_character() {
        ++m_current_character;

        if (m_line_view.size() > 0) {
            m_line_view.remove_prefix(1);
        }
    }

    void Parser::skip_whitespace() {
        while (is_space(*m_current_character)) {
            next_character();
        }
    }

    std::string_view Parser::parse_instruction() {
        if (!is_alphabetic(*m_current_character)) {
            m_status = Status::syntax_error;
            throw Bad_Parse(m_line_count, "Expected instruction.", m_status);
        }

        const char* const start { m_current_character };

        std::size_t iterations { m_line_view.size() };

        for (std::size_t i {}; i < iterations; ++i) {
            if (!is_alphabetic(*m_current_character) && !is_digit(*m_current_character)) {
                break;
            }

            next_character();
        }

        return { start, m_current_character };
    }

    int Parser::to_instruction(std::string_view token) {
        if (!confirm_token_isolated(token)) {
            handle_malformed_token(token);
        }

        for (const Instruction_Data& instruction : g_instruction_data) {
            if (token == instruction.name) {
                m_expected_operand_count = instruction.expected_following_values;
                return instruction.value;
            }
        }

        m_status = Status::syntax_error;
        std::ostringstream message {};
        message
            << "Token \""
            << token
            << "\" is not a valid instruction.";
        throw Bad_Parse(m_line_count, message.str(), m_status);
    }

    std::string_view Parser::parse_operand() {
        bool has_negator { is_negator(*m_current_character) };

        if (!is_digit(*m_current_character) && !has_negator) {
            m_status = Status::syntax_error;
            throw Bad_Parse(m_line_count, "Expected integer.", m_status);
        }

        const char* const start { m_current_character };

        if (has_negator) {
            next_character();
        }

        std::size_t iterations { m_line_view.size() };

        for (std::size_t i {}; i < iterations; ++i) {
            if (!is_digit(*m_current_character)) {
                break;
            }

            next_character();
        }

        return { start, m_current_character };
    }

    int Parser::to_operand(std::string_view token) {
        if (!confirm_token_isolated(token)) {
            handle_malformed_token(token);
        }

        // A static variable to save on allocations.
        static std::string convertable {};

        convertable = static_cast<std::string>(token);

        try {
            return std::stoi(convertable);
        }
        catch (const std::invalid_argument&) {
            m_status = Status::conversion_failure;
            std::ostringstream message {};
            message
                << "Operand \""
                << token
                << "\" is not a convertable integer.";
            throw Bad_Parse(m_line_count, message.str(), m_status);
        }
        catch (const std::out_of_range&) {
            m_status = Status::conversion_failure;
            std::ostringstream message {};
            message
                << "Operand \""
                << token
                << "\" falls out of the range of a 32-bit integer.";
            throw Bad_Parse(m_line_count, message.str(), m_status);
        }
        catch (...) {
            m_status = Status::conversion_failure;
            throw Bad_Parse(
                m_line_count,
                "Unknown exeption encountered during conversion of parsed operand.",
                m_status
            );
        }
    }

    std::string_view Parser::parse_keyword() {
        if (!is_alphabetic(*m_current_character)) {
            m_status = Status::syntax_error;
            throw Bad_Parse(m_line_count, "Expected keyword.", m_status);
        }

        const char* const start { m_current_character };

        std::size_t iterations { m_line_view.size() };

        for (std::size_t i {}; i < iterations; ++i) {
            if (!is_alphabetic(*m_current_character)) {
                break;
            }

            next_character();
        }

        return { start, m_current_character };
    }

    void Parser::process_keyword(std::string_view token) {
        if (!confirm_token_isolated(token)) {
            handle_malformed_token(token);
        }

        if (token == g_init) {
            if (m_entry_point_status != Entry_Point_Search_Status::not_found) {
                m_status = Status::syntax_error;
                throw Bad_Parse(m_line_count, "Duplicate entry point defined. Only 1 entry point per program is allowed.", m_status);
            }

            m_entry_point_status = Entry_Point_Search_Status::expecting;
        }
        else {
            m_status = Status::syntax_error;
            std::ostringstream message {};
            message << "Unexpected token \"" << token << "\" found.";
            throw Bad_Parse(m_line_count, message.str(), m_status);
        }
    }

    bool Parser::confirm_token_isolated(std::string_view token) const {
        return (m_line_view.size() == 0) || is_whitespace(*m_current_character);
    }

    void Parser::check_remaining_operand_count() {
        if (m_expected_operand_count != 0) {
            std::ostringstream message {};
            message
                << "Expected "
                << m_expected_operand_count
                << " remaining operands after last parsed instruction.";
            m_status = Status::malformed_program;
            throw Bad_Parse(m_line_count, message.str(), m_status);
        }
    }

    void Parser::check_program_start_index() {
        if (m_entry_point_status == Entry_Point_Search_Status::expecting) {
            m_status = Status::syntax_error;
            std::ostringstream message {};
            message
                << "Instruction not found after "
                << g_init
                << " declaration. An entry point must be given after an entry declaration.";
            throw Bad_Parse(m_line_count, message.str(), m_status);
        }
    }

    void Parser::handle_malformed_token(std::string_view bad_token) {
        const char* const start { m_current_character };
        const char* end { start };

        for (std::size_t i {}; i < m_line_view.size(); ++i) {
            if (!is_whitespace(*m_current_character)) {
                break;
            }

            ++end;
        }

        std::ostringstream message {};
        message
            << "Unknown token \""
            << std::string_view(start, end)
            << "\" found.";

        m_status = Status::syntax_error;
        throw Bad_Parse(m_line_count, message.str(), m_status);
    }
}

#pragma once

#include <string>
#include <string_view>
#include <vector>
#include <fstream>

namespace svim {
    class Parser final {
    public:
        enum class Status {
            success,
            ready,
            failure_on_initialization,
            syntax_error,
            file_opening_failure,
            improper_file_format,
            conversion_failure,
            malformed_program,
            bad_operand,
            unknown_failure
        };

        Parser(std::string_view source_file);

        std::vector<int> parse();

        Status get_status() const { return m_status; }
        int get_program_start_index() const { return m_program_start_index; }

    private:
        enum class Entry_Point_Search_Status {
            not_found,
            expecting,
            found
        };

        Status m_status { Status::ready };
        std::string m_source_file {};
        std::ifstream m_input {};

        const char* m_current_character {};
        std::string_view m_line_view {};
        int m_line_count {};

        int m_last_instruction { -1 };
        int m_expected_operand_count {};
        int m_program_start_index { 0 };
        Entry_Point_Search_Status m_entry_point_status { Entry_Point_Search_Status::not_found };

        void open_source_file();
        void parse_line(std::vector<int>& bytecode, const std::string& line);
        void next_character();
        void skip_whitespace();
        std::string_view parse_instruction();
        std::string_view parse_operand();
        std::string_view parse_keyword();
        int to_instruction(std::string_view token);
        int to_operand(std::string_view token);
        void process_keyword(std::string_view token);
        bool confirm_token_isolated(std::string_view token) const;
        void validate_operand(int value);
        void assert_within_range(int index, int max_values, bool is_local);

        bool expecting() const;
        bool expecting_operand() const;

        void check_remaining_operand_count();
        void check_program_start_index();
        void handle_malformed_token(std::string_view bad_token);
    };
}

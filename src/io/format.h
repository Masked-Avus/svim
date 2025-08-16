#pragma once

#include <string>
#include <cstddef>

namespace svim {
    inline constexpr std::string_view g_svim_file_extension { ".svim" };

    inline bool is_digit(char character) {
        return (character >= '0') && (character <= '9');
    }

    inline bool is_alphabetic(char character) {
        return ((character >= 'a') && (character <= 'z')) ||
            ((character >= 'A') && (character <= 'Z')) ||
            (character == '_');
    }

    inline bool is_space(char character) {
        return (character == ' ') || (character == '\t') || (character == '\r');
    }

    inline bool is_endline(char character) {
        // We ignore everything on a line after a comment symbol.
        return (character == '\n') || (character == '#');
    }

    inline bool is_whitespace(char character) {
        return is_space(character) || is_endline(character);
    }

    inline bool is_extension_separator(char character) {
        return character == '.';
    }

    inline bool is_negator(char character) {
        return character == '-';
    }

    inline bool is_keyword_prefix(char character) {
        return character == '.';
    }

    inline void make_uppercase(std::string& out_target) {
        for (std::size_t i {}; i < out_target.size(); ++i) {
            out_target[i] = static_cast<char>(std::toupper(out_target[i]));
        }
    }

    enum class File_Name_Formatting {
        good,
        name_too_short,
        no_extension_delimiter,
        incorrect_extension,
        unallowed_characters
    };

    File_Name_Formatting is_source_file(std::string_view input_file);
    std::string create_log_file(std::string_view corresponding_input_file);
    std::string create_code_dump_file(std::string_view corresponding_input_file);
}

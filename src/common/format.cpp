#include "pch.h"
#include "format.h"

namespace svim {
    //---------- Forward Declarations

    static bool is_allowed_file_name_character(char character, bool& separator_found);
    static std::string create_output_file(
        std::string_view corresponding_input_file,
        std::string_view suffix,
        std::string_view extension
        );
    static void remove_file_extension(std::string& file_name, std::string_view extension);
    static File_Name_Formatting assert_proper_kinds_of_characters(std::string_view input_file);
    static File_Name_Formatting assert_correct_file_extension(std::string_view input_file);

    //---------- Internal Data

    static constexpr std::string_view g_file_dump_extension { ".txt" };
    static constexpr std::string_view g_program_output_suffix { "_Output" };
    static constexpr std::string_view g_code_dump_suffix { "_ParsedSourceDump" };


    //----------- Public API

    File_Name_Formatting is_source_file(std::string_view input_file) {
        if (input_file.size() < (g_svim_file_extension.size() + 1)) {
            return File_Name_Formatting::name_too_short;
        }

        File_Name_Formatting formatting { assert_proper_kinds_of_characters(input_file) };

        if (formatting != File_Name_Formatting::good) {
            return formatting;
        }

        return assert_correct_file_extension(input_file);
    }

    std::string create_log_file(std::string_view corresponding_input_file) {
        return create_output_file(corresponding_input_file, g_program_output_suffix, g_file_dump_extension);
    }

    std::string create_code_dump_file(std::string_view corresponding_input_file) {
        return create_output_file(corresponding_input_file, g_code_dump_suffix, g_file_dump_extension);
    }


    //---------- Helper Functions

    static File_Name_Formatting assert_proper_kinds_of_characters(std::string_view input_file) {
        bool separator_found {};

        for (const char character : input_file) {
            if (!is_allowed_file_name_character(character, separator_found)) {
                return File_Name_Formatting::unallowed_characters;
            }
        }

        if (!separator_found) {
            return File_Name_Formatting::no_extension_delimiter;
        }

        return File_Name_Formatting::good;
    }

    static File_Name_Formatting assert_correct_file_extension(std::string_view input_file) {
        std::string_view extension_view { input_file };

        while (!is_extension_separator(extension_view.front())) {
            extension_view.remove_prefix(1);
        }

        if (extension_view != g_svim_file_extension) {
            return File_Name_Formatting::incorrect_extension;
        }

        return File_Name_Formatting::good;
    }

    static bool is_allowed_file_name_character(char character, bool& separator_found) {
        if (!is_alphabetic(character) && !is_digit(character)) {
            if (!is_extension_separator(character)) {
                

                return false;
            }
            else {
                if (!separator_found) {
                    separator_found = true;
                }
                else {
                    return false;
                }
            }
        }

        return true;
    }

    static void remove_file_extension(std::string& file_name, std::string_view extension) {
        for (std::size_t i {}; i < extension.size(); ++i) {
            file_name.pop_back();
        }
    }

    static std::string create_output_file(
        std::string_view corresponding_input_file,
        std::string_view suffix,
        std::string_view extension
        ) {

        if (corresponding_input_file.size() < (g_svim_file_extension.size() + 1)) {
            std::ostringstream message {};
            message
                << "Cannot create output file name. Files must be at least "
                << (g_svim_file_extension.size() + 1)
                << " long, including the \""
                << g_svim_file_extension
                << "\" extension.";
            throw std::runtime_error(message.str());
        }

        std::string output_file { corresponding_input_file };

        remove_file_extension(output_file, g_svim_file_extension);

        output_file.append(suffix);
        output_file.append(extension);

        return output_file;
    }
}

#pragma once

#include <string>
#include <string_view>
#include <stdexcept>
#include <cstddef>
#include "virtual_machine/parser.h"

namespace svim {
    class Bad_Parse : public std::runtime_error {
    public:
        Bad_Parse(const char* message, Parser::Status status) :
            std::runtime_error { message }, m_status { status } {}
        Bad_Parse(const std::string& message, Parser::Status status) :
            std::runtime_error { message }, m_status { status } {}
        Bad_Parse(int line_of_incident, std::string_view message, Parser::Status status);

        Parser::Status get_parser_status() const { return m_status; }

    private:
        Parser::Status m_status {};

        static std::string append_line_number(int line_of_incident, std::string_view message);
    };

    class File_Open_Failure : public std::runtime_error {
    public:
        File_Open_Failure(const char* message) : std::runtime_error { message } {}
        File_Open_Failure(const std::string& message) : std::runtime_error { message } {}
    };
}

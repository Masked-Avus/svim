#include "pch.h"
#include "error.h"

namespace svim {
    Bad_Parse::Bad_Parse(int line_of_incident, std::string_view message, Parser::Status status) :
        std::runtime_error { append_line_number(line_of_incident, message) },
        m_status { status } {}

    std::string Bad_Parse::append_line_number(int line_of_incident, std::string_view message) {
        std::stringstream formatted {};
        formatted << message << " (Line: " << line_of_incident << ')';
        return formatted.str();
    }
}
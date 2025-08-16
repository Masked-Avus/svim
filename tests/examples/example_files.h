#pragma once

#include <string_view>

namespace test {
    inline constexpr std::string_view g_test_file_1 { "examples/test1.svim" };
    inline constexpr std::string_view g_test_file_2 { "examples/test2.svim" };
    inline constexpr std::string_view g_bad_file { "examples/not_svim.txt" };
}

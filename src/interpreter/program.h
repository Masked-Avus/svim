#pragma once

#include <vector>
#include <string_view>

namespace svim {
    struct Program final {
        std::string_view name {};
        int starting_point {};
        std::vector<int> bytecode {};
    };

    struct Demo_Program_Iterators final {
        const Program* start {};
        const Program* end {};
    };

    const Program* get_demo_program(int index);
    const Program* get_demo_program(std::string_view name);
    Demo_Program_Iterators get_demo_programs();
}

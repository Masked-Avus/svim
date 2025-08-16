#pragma once

#include <iostream>
#include "timer.h"

namespace svim {
#if SVIM_DEBUG
#define SVIM_PRINT_LINE(message) std::cout << message << '\n'
#define SVIM_PRINT_TAB_LINE(message) std::cout << '\t' << message << '\n'
#define SVIM_PRINT_DTAB_LINE(message) std::cout << "\t\t" << message << '\n'
#define SVIM_PRINT_PROPERTY(prop, value) std::cout << '\t' << prop << ") " << value << '\n'
#define SVIM_PRINT_DPROPERTY(prop, value) std::cout << "\t\t" << prop << ") " << value << '\n'
#else
#define SVIM_PRINT_LINE(message)
#define SVIM_PRINT_TAB_LINE(message)
#define SVIM_PRINT_DTAB_LINE(message)
#define SVIM_PRINT_PROPERTY(prop, value)
#define SVIM_PRINT_DPROPERTY(prop, value)
#endif
}

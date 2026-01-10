#include "pch.h"
#include "application.h"

int main(int argc, const char* argv[]) {
    svim::Application app { argv, argc };
    return app.run();
}

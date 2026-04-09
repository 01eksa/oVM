#include <iostream>
#include <memory>
#include <string_view>
#include <format>

#include "config.h"
#include "loader.h"
#include "vm.h"

struct Args {
    bool no_pause = false;
    bool debug    = false;
};

Args args;

void wait() {
    if (args.no_pause) return;
    std::cout << "\nPress Enter to exit..." << std::endl;
    std::cin.get();
}

int main(const int argc, char* argv[]) {
    const auto help = std::format(
        "oVM version {}.{}.{}\n\n"
        "Arguments: [filename] [options]\n\n"
        "Options:\n"
        "--no-pause - no pause after end of program\n"
        "--debug    - enable debug mode\n",
        config::major, config::minor, config::patch
        );

    if (argc < 2) {
        std::cout << help;
        wait();
        return 0;
    }

    if (argc > 2) {
        for (int i = 2; i < argc; i++) {
            const std::string_view arg = argv[i];
            if (arg == "--no-pause")
                args.no_pause = true;
            else if (arg == "--debug")
                args.debug = true;
        }
    }

    const std::string arg = argv[1];
    if (arg == "--help" || arg == "h") {
        std::cout << help;
        return 0;
    }

    Program p;
    try {
        p = load(arg);
    } catch (std::exception& e) {
        std::cerr << e.what() << std::endl;
        wait();
        return 1;
    }

    VM vm(std::move(p), args.debug);
    try {
        vm.run();
    } catch (std::exception& e) {
        std::cerr << e.what() << std::endl;
        wait();
        return 1;
    }

    wait();
    return 0;
}

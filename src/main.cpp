#include <iostream>
#include <memory>
#include <string_view>

#include "vm.h"
#include "loader.h"

struct Args {
    bool no_pause = false;
    bool debug = false;
};

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cout << "Enter a file as an argument\n";
        return 0;
    }

    Args args;

    if (argc > 2) {
        for (int i = 2; i < argc; i++) {
            std::string_view arg = argv[i];
            if (arg == "--no-pause") args.no_pause = true;
            else if (arg == "--debug") args.debug = true;
        }
    }

    const std::string file = argv[1];
    Program p;

    try {
        p = load(file);
    }
    catch (std::exception& e) {
        std::cerr << e.what() << std::endl;
        return 1;
    }

    VM vm(std::move(p));
    try {
        vm.run();
    }
    catch (std::exception& e) {
        std::cerr << e.what() << std::endl;
        return 1;
    }

    if (args.no_pause)  return 0;

    std::cout << "\nPress Enter to exit..." << std::endl;
    std::cin.get();

    return 0;
}

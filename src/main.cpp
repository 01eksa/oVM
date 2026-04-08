#include <iostream>
#include <memory>
#include <string_view>

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

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cout << "Enter a file as an argument\n";
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

    const std::string file = argv[1];
    Program           p;

    try {
        p = load(file);
    } catch (std::exception& e) {
        std::cerr << e.what() << std::endl;
        wait();
        return 1;
    }

    VM vm(std::move(p));
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

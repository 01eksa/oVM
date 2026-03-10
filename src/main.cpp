#include <iostream>
#include <memory>
#include <string_view>

#include "vm.h"
#include "loader.h"

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cout << "Enter a file as an argument\n";
        return 0;
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
    vm.run();

    if (argc > 2) {
        std::string_view arg2 = argv[2];
        if (arg2 == "--no-pause") return 0;
    }
    
    if (std::cin.rdbuf()->in_avail())
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    std::cout << "\nPress Enter to exit..." << std::endl;
    std::cin.get();

    return 0;
}

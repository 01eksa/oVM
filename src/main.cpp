#include <cstdint>
#include <cstring>
#include <iostream>
#include <memory>
#include "vm.h"
#include "loader.h"

int main(int argc, char* argv[]) {
    std::string file;
    if (argc < 2) {
        std::cout << "Enter a file as an argument\n";
        return 0;
    }
    else {
        file = argv[1];
    }
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

    return 0;
}

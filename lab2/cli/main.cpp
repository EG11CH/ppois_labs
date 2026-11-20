#include "console.h"
#include "lab2/desk/demo.h"

#include <iostream>

int main() {
    auto desk = lab2::demoDesk();
    return lab2::runConsole(std::cin, std::cout, desk);
}

#include <iostream>
#include "gui/gui.h"
#include "engine/board.h"
#include "engine/gameloops.h"

int main() {
    std::cout << "Chess Bot\n";
    std::cout << "1. Play human vs human\n";
    std::cout << "2. Play human vs AI\n";
    int choice;
    std::cin >> choice;

    if (choice == 1) {
        gameLoop();
    } else if (choice == 2) {
        std::cout << "Want to start as Black or White";
        bool prefferedSelection;
        std::cin >> prefferedSelection;
        computerGameloop(prefferedSelection);
    } else {
        std::cerr << "Invalid choice. Exiting.\n";
    }

    return 0;
}
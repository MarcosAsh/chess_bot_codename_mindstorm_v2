#include "engine/board.h"
#include "engine/gameloops.h"
#include "engine/ai.h"
#include <iostream>


using namespace std;

// Main function to choose game mode
int main() {
    initializeZobrist();
    initializePosition();
    printBitboard(whitePawns);
    cout << "Welcome to Chess!\nChoose game mode:\n1. Human vs Human\n2. Human vs Computer\n";
    int choice;
    cin >> choice;
    cin.ignore(); // To ignore the newline character left in the input buffer

    if (choice == 1) {
        gameLoop();
    } else if (choice == 2) {
        cout << "Do you want to play as White? (y/n): ";
        char colorChoice;
        cin >> colorChoice;
        cin.ignore();
        bool humanPlaysWhite = (colorChoice == 'y' || colorChoice == 'Y');
        computerGameLoop(humanPlaysWhite);
    } else {
        cout << "Invalid choice. Exiting program.\n";
    }

    return 0;
}
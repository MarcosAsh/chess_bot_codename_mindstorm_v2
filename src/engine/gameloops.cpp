#include <iostream>
#include <bitset>
#include <cstdint>
#include <vector>
#include <algorithm>
#include <limits>
#include <stack>
#include <random>
#include <unordered_map>

#include "evaluation.h"
#include "board.h"
#include "ai.h"
#include "utils.h"
#include "../gui/gui.h"


// Game loop for human vs. human gameplay
void gameLoop() {
    bool isWhiteTurn = true;
    initializePosition();
    printBoardForPlayers();

    while (true) {
        if (isCheckmateOrStalemate(isWhiteTurn)) {
            if (isSquareAttacked(isWhiteTurn ? whiteKing : blackKing, !isWhiteTurn)) {
                std::cout << (isWhiteTurn ? "Black wins by checkmate!" : "White wins by checkmate!") << std::endl;
            } else {
                std::cout << "Stalemate! The game is a draw." << std::endl;
            }
            break;
        }

        // Display turn and take input
        std::cout << (isWhiteTurn ? "White's turn: " : "Black's turn: ");
        std::string moveInput;
        getline(std::cin, moveInput);

        if (moveInput.size() != 5 || moveInput[2] != ' ') {
            std::cout << "Invalid input format. Use format 'e2 e4'.\n";
            continue;
        }

        auto [fromSquare, toSquare] = parseInput(moveInput);
        if (makeMove(fromSquare, toSquare, isWhiteTurn)) {
            printBoardForPlayers();
            int score = evaluatePosition();
            std::cout << "Evaluation Score: " << score << " ("
                 << (score > 0 ? "White is better" : (score < 0 ? "Black is better" : "Equal"))
                 << ")\n";
            isWhiteTurn = !isWhiteTurn;
        } else {
            std::cout << "Invalid move. Try again.\n";
        }
    }
}

// Game loop for playing against the computer
void computerGameLoop(bool humanPlaysWhite) {
    bool isWhiteTurn = true;
    initializePosition();
    printBoardForPlayers();

    while (true) {
        if (isCheckmateOrStalemate(isWhiteTurn)) {
            if (isSquareAttacked(isWhiteTurn ? whiteKing : blackKing, !isWhiteTurn)) {
                std::cout << (isWhiteTurn ? "Black wins by checkmate!" : "White wins by checkmate!") << std::endl;
            } else {
                std::cout << "Stalemate! The game is a draw." << std::endl;
            }
            break;
        }

        if ((isWhiteTurn && humanPlaysWhite) || (!isWhiteTurn && !humanPlaysWhite)) {
            // Human move
            std::cout << (isWhiteTurn ? "White's turn: " : "Black's turn: ");
            std::string moveInput;
            getline(std::cin, moveInput);

            if (moveInput.size() != 5 || moveInput[2] != ' ') {
                std::cout << "Invalid input format. Use format 'e2 e4'.\n";
                continue;
            }

            std::pair<int, int> parsedMove = parseInput(moveInput);
            int fromSquare = parsedMove.first;
            int toSquare = parsedMove.second;

            if (!makeMove(fromSquare, toSquare, isWhiteTurn)) {
                std::cout << "Invalid move. Try again.\n";
                continue;
            }
        } else {
            // Computer move
            std::cout << "Computer is thinking...\n";
            Move bestMove = findBestMove(isWhiteTurn);
            if (bestMove.from == 0 && bestMove.to == 0) {
                std::cout << "No legal moves available for AI. Game over.\n";
                break;
            }
            makeMove(bestMove.from, bestMove.to, isWhiteTurn);
            std::cout << "Computer's move: Evaluation = " << bestMove.evaluation << std::endl;
        }

        printBoardForPlayers();
        isWhiteTurn = !isWhiteTurn;
    }
}

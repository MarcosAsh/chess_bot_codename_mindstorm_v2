#include "utils.h"
#include "../constants.h"
#include "board.h"
#include <iostream>
#include <string>

// Convert a square in bitboard format to chess notation (e.g., 1ULL << 0 -> "a1")
std::string squareToNotation(uint64_t square) {
    if (square == 0) {
        return "null"; // Handle case where square is invalid
    }

    // Get the index of the square (position of the least significant bit)
    int index = __builtin_ctzll(square); // Count trailing zeros (find the bit position)

    // Convert the index into file (column) and rank (row)
    char file = 'a' + (index % 8);      // Files are a-h (0-7 mod 8)
    char rank = '1' + (index / 8);      // Ranks are 1-8 (0-7 div 8)

    return std::string(1, file) + std::string(1, rank); // Combine file and rank into a single string
}

// Parse move input like "e2 e4" to bitboard squares
std::pair<int, int> parseInput(const std::string& input) {
    if (input.size() != 5 || input[2] != ' ') {
        std::cerr << "Invalid input format. Use format 'e2 e4'.\n";
        return {-1, -1}; // Return invalid squares
    }

    // Extract file and rank from the input string
    int fileFrom = input[0] - 'a';
    int rankFrom = input[1] - '1';
    int fileTo = input[3] - 'a';
    int rankTo = input[4] - '1';

    // Convert file and rank to square indices
    int fromSquare = rankFrom * 8 + fileFrom;
    int toSquare = rankTo * 8 + fileTo;

    return {fromSquare, toSquare};
}

// Print a bitboard for debugging purposes
void printBitboard(uint64_t bitboard) {
    std::cout << "  a b c d e f g h\n +----------------+\n";
    for (int rank = 7; rank >= 0; --rank) {
        std::cout << rank + 1 << "| ";
        for (int file = 0; file < 8; ++file) {
            int square = rank * 8 + file;
            std::cout << (((bitboard >> square) & 1) ? "1 " : ". ");
        }
        std::cout << "|\n";
    }
    std::cout << " +----------------+\n";
}

// Count the number of legal moves for a player
int countLegalMoves(bool isWhiteTurn) {
    int moveCount = 0;
    uint64_t pieces = isWhiteTurn ? whitePieces : blackPieces;

    while (pieces) {
        uint64_t piece = pieces & -pieces; // Isolate the least significant bit (current piece)
        pieces &= pieces - 1;              // Remove the isolated piece from the set

        // Generate all legal moves for this piece
        std::vector<uint64_t> legalMoves;
        if (isWhiteTurn) {
            if (whitePawns & piece) legalMoves = generatePawnMoves(piece, true);
            else if (whiteKnights & piece) legalMoves = generateKnightMoves(piece, true);
            else if (whiteBishops & piece) legalMoves = generateBishopMoves(piece, true);
            else if (whiteRooks & piece) legalMoves = generateRookMoves(piece, true);
            else if (whiteQueens & piece) legalMoves = generateQueenMoves(piece, true);
            else if (whiteKing & piece) legalMoves = generateKingMoves(piece, true);
        } else {
            if (blackPawns & piece) legalMoves = generatePawnMoves(piece, false);
            else if (blackKnights & piece) legalMoves = generateKnightMoves(piece, false);
            else if (blackBishops & piece) legalMoves = generateBishopMoves(piece, false);
            else if (blackRooks & piece) legalMoves = generateRookMoves(piece, false);
            else if (blackQueens & piece) legalMoves = generateQueenMoves(piece, false);
            else if (blackKing & piece) legalMoves = generateKingMoves(piece, false);
        }

        moveCount += legalMoves.size();
    }

    return moveCount;
}


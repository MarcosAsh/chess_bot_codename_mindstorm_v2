#ifndef UTILS_H
#define UTILS_H

#include <cstdint>
#include <string>

// Utility functions
std::string squareToNotation(uint64_t square);
std::pair<int, int> parseInput(const std::string& input);
void printBitBoard(uint64_t bitboard);
int CountLegalMoves(bool isWhiteTurn);

#endif
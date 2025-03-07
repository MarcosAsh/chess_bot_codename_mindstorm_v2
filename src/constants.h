#ifndef CONSTANTS_H
#define CONSTANTS_H

#include <cstdint>

// Board constants
const int BOARD_SIZE = 8;
const uint64_t FILE_A = 0x0101010101010101ULL;
const uint64_t FILE_B = 0x0202020202020202ULL;
const uint64_t FILE_G = 0x4040404040404040ULL;
const uint64_t FILE_H = 0x8080808080808080ULL;
const uint64_t RANK_1 = 0x00000000000000FFULL;
const uint64_t RANK_2 = 0x000000000000FF00ULL;
const uint64_t RANK_4 = 0x00000000FF000000ULL;
const uint64_t RANK_5 = 0x000000FF00000000ULL;
const uint64_t RANK_7 = 0x00FF000000000000ULL;
const uint64_t RANK_8 = 0xFF00000000000000ULL;

// Side constants
const int WHITE = 0;
const int BLACK = 1;

// Movement directions for pieces
const int vector[8][8] = {
    {1, 8}, {2, 16}, {3, 24}, {4, 32}, {5, 40}, {6, 48}, {7, 56}, {8, 64} // Example directions
};

// Piece constants
const int PAWN = 0;
const int KNIGHT = 1;
const int BISHOP = 2;
const int ROOK = 3;
const int QUEEN = 4;
const int KING = 5;

// Square validation macro
#define IS_SQ(sq) ((sq) >= 0 && (sq) < 64)

#endif
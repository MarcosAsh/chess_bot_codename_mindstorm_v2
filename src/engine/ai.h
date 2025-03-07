#ifndef AI_H
#define AI_H

#include <cstdint>
#include <vector>

struct Move {
    uint64_t from;
    uint64_t to;
    int evaluation;
};

// AI functions
int minimax(int depth, bool isMaximizingPlayer, int alpha, int beta, bool isWhiteTurn);
Move findBestMove(bool isWhiteTurn, int depth = 4);

#endif
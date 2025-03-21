#ifndef AI_H
#define AI_H

#include "board.h"
#include "evaluation.h"

#include <stack>
#include <unordered_map>
#include <vector>
#include <cstdint>

// Structure to represent a move with its evaluation
struct Move {
    uint64_t from;
    uint64_t to;
    int evaluation;
};

// Zobrist hashing
extern std::stack<uint64_t> zobristHistory;
extern uint64_t zobristTable[12][64];
extern std::unordered_map<uint64_t, std::pair<int, int>> transpositionTable;

// Zobrist initialization
void initializeZobrist();

// Utility functions
int max(int a, int b);
int min(int a, int b);

// Move evaluation helpers
bool isCapture(uint64_t move, bool isWhiteTurn);
bool isCheck(uint64_t move, bool isWhiteTurn);
int movePriority(uint64_t move, bool isWhiteTurn);

// Minimax algorithm with alpha-beta pruning
int minimax(int depth, bool isMaximizingPlayer, int alpha, int beta, bool isWhiteTurn);

// AI interface
Move findBestMove(bool isWhiteTurn, int depth);

#endif // AI_H

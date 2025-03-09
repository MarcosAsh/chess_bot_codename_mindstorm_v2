#include "ai.h"
#include "board.h"
#include "evaluation.h"
#include <algorithm>

#include <iostream>
#include <bitset>
#include <cstdint>
#include <vector>
#include <limits>
#include <stack>
#include <random>
#include <unordered_map>

using namespace std;

stack<uint64_t> zobristHistory; // For undoing Zobrist hashes efficiently
uint64_t zobristTable[12][64];  // Randomized Zobrist keys for hashing
unordered_map<uint64_t, pair<int, int>> transpositionTable;

// Initialize Zobrist hashing
void initializeZobrist() {
    random_device rd;
    mt19937_64 gen(rd());
    uniform_int_distribution<uint64_t> dist(0, UINT64_MAX);

    for (int piece = 0; piece < 12; ++piece) {
        for (int square = 0; square < 64; ++square) {
            zobristTable[piece][square] = dist(gen);
        }
    }
}

// Define a function to get the maximum evaluation
int max(int a, int b) {
    return (a > b) ? a : b;
}

// Define a function to get the minimum evaluation
int min(int a, int b) {
    return (a < b) ? a : b;
}
bool isCapture(uint64_t move, bool isWhiteTurn) {
    uint64_t targetSquare = move; // Assume `move` encodes the target square
    uint64_t opponentPieces = isWhiteTurn ? blackPieces : whitePieces;
    return targetSquare & opponentPieces;
}

bool isCheck(uint64_t move, bool isWhiteTurn) {
    saveBoardState(isWhiteTurn);
    makeMove(move, move, isWhiteTurn);

    uint64_t king = isWhiteTurn ? blackKing : whiteKing;
    bool result = isSquareAttacked(king, !isWhiteTurn);

    undoMove(); // Revert to the original state
    return result;
}

int movePriority(uint64_t move, bool isWhiteTurn) {
    int priority = 0;
    if (isCapture(move, isWhiteTurn)) {
        priority += 100; // High priority for captures
    }
    if (isCheck(move, isWhiteTurn)) {
        priority += 50; // Moderate priority for checks
    }
    return priority;
}

// Recursive minimax function with alpha-beta pruning
int minimax(int depth, bool isMaximizingPlayer, int alpha, int beta, bool isWhiteTurn) {
    uint64_t zobristHash = zobristHistory.top(); // Retrieve current Zobrist hash

    // Check transposition table
    if (transpositionTable.count(zobristHash)) {
        auto [storedDepth, storedEval] = transpositionTable[zobristHash];
        if (storedDepth >= depth) {
            return storedEval; // Use cached evaluation
        }
    }

    // Base case: if depth is 0 or the game is over
    if (depth == 0 || isCheckmateOrStalemate(isWhiteTurn)) {
        return evaluatePosition();
    }

    std::vector<uint64_t> legalMoves;
    int bestEval = isMaximizingPlayer ? std::numeric_limits<int>::min() : std::numeric_limits<int>::max();

    // Generate all moves for the current player
    uint64_t pieces = isWhiteTurn ? whitePieces : blackPieces;
    while (pieces) {
        uint64_t piece = pieces & -pieces; // Isolate least significant bit
        pieces &= pieces - 1;

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

        // Move ordering: prioritize captures or checks
        sort(legalMoves.begin(), legalMoves.end(), [isWhiteTurn](uint64_t a, uint64_t b) {
            return movePriority(a, isWhiteTurn) > movePriority(b, isWhiteTurn);
        });

        for (uint64_t move : legalMoves) {
            if (!isMoveLegal(piece, move, isWhiteTurn)) continue;

            saveBoardState(isWhiteTurn);
            makeMove(piece, move, isWhiteTurn);

            int eval;
            if (isMaximizingPlayer) {
                eval = minimax(depth - 1, false, alpha, beta, !isWhiteTurn);
                bestEval = std::max(bestEval, eval);
                alpha = std::max(alpha, eval);
            } else {
                eval = minimax(depth - 1, true, alpha, beta, !isWhiteTurn);
                bestEval = std::min(bestEval, eval);
                beta = std::min(beta, eval);
            }

            undoMove();

            if (beta <= alpha) {
                break; // Alpha-beta cutoff
            }
        }
    }

    // Store result in transposition table
    transpositionTable[zobristHash] = std::make_pair(depth, bestEval);

    return bestEval;
}

// Function to find the best move for the computer
Move findBestMove(bool isWhiteTurn, int depth) {
    Move bestMove = {0, 0, isWhiteTurn ? std::numeric_limits<int>::min() : std::numeric_limits<int>::max()};
    uint64_t pieces = isWhiteTurn ? whitePieces : blackPieces;

    while (pieces) {
        uint64_t piece = pieces & -pieces;
        pieces &= pieces - 1;

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

        for (uint64_t move : legalMoves) {
            if (!isMoveLegal(piece, move, isWhiteTurn)) continue;

            saveBoardState(isWhiteTurn);
            makeMove(piece, move, isWhiteTurn);

            int eval = minimax(depth - 1, !isWhiteTurn, std::numeric_limits<int>::min(), std::numeric_limits<int>::max(), !isWhiteTurn);
            undoMove();

            std::cout << "Move from " << squareToNotation(piece) << " to " << squareToNotation(move)
                      << " evaluated at " << eval << std::endl;

            if ((isWhiteTurn && eval > bestMove.evaluation) || (!isWhiteTurn && eval < bestMove.evaluation)) {
                bestMove = {piece, move, eval};
            }
        }
    }

    std::cout << "Best move selected: from " << squareToNotation(bestMove.from) << " to " << squareToNotation(bestMove.to)
              << " with evaluation " << bestMove.evaluation << std::endl;
    return bestMove;
}
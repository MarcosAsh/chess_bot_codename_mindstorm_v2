#include "evaluation.h"
#include "board.h"
#include "../constants.h"
#include <iostream>

// Piece values
const int PAWN_VALUE = 100;
const int KNIGHT_VALUE = 320;
const int BISHOP_VALUE = 330;
const int ROOK_VALUE = 500;
const int QUEEN_VALUE = 900;
const int KING_VALUE = 20000;

// Positional bonuses
const int CENTER_CONTROL = 20; // Bonus for controlling central squares
const uint64_t CENTER_MASK = 0x0000001818000000ULL; // Central squares (d4, d5, e4, e5)

// Adjustments based on the number of pawns
int knight_adj[9] = { -20, -16, -12, -8, -4,  0,  4,  8, 12 };
int rook_adj[9] = { 15,  12,   9,  6,  3,  0, -3, -6, -9 };

// Safety table for king attacks
static const int SafetyTable[100] = {
  0,  0,   1,   2,   3,   5,   7,   9,  12,  15,
  18,  22,  26,  30,  35,  39,  44,  50,  56,  62,
  68,  75,  82,  85,  89,  97, 105, 113, 122, 131,
  140, 150, 169, 180, 191, 202, 213, 225, 237, 248,
  260, 272, 283, 295, 307, 319, 330, 342, 354, 366,
  377, 389, 401, 412, 424, 436, 448, 459, 471, 483,
  494, 500, 500, 500, 500, 500, 500, 500, 500, 500,
  500, 500, 500, 500, 500, 500, 500, 500, 500, 500,
  500, 500, 500, 500, 500, 500, 500, 500, 500, 500,
  500, 500, 500, 500, 500, 500, 500, 500, 500, 500
};

// Evaluate pawn structure
int evaluatePawnStructure(int side) {
    int score = 0;
    uint64_t pawns = (side == WHITE) ? b.whitePawns : b.blackPawns;
    uint64_t enemyPawns = (side == WHITE) ? b.blackPawns : b.whitePawns;
    int step = (side == WHITE) ? 8 : -8; // Direction of pawn movement

    while (pawns) {
        int sq = __builtin_ctzll(pawns);
        pawns &= pawns - 1;

        int file = sq % 8;
        int rank = sq / 8;

        // Check for passed pawns
        bool isPassed = true;
        for (int r = rank + step; r >= 0 && r < 8; r += step) {
            if (enemyPawns & (1ULL << (file + r * 8))) {
                isPassed = false;
                break;
            }
        }

        if (isPassed) {
            score += 50; // Bonus for passed pawns
        }

        // Check for isolated pawns
        bool isIsolated = true;
        if (file > 0 && (pawns & (1ULL << (sq - 1)))) isIsolated = false; // Left file
        if (file < 7 && (pawns & (1ULL << (sq + 1)))) isIsolated = false; // Right file

        if (isIsolated) {
            score -= 20; // Penalty for isolated pawns
        }

        // Check for doubled pawns
        bool isDoubled = false;
        for (int r = rank + step; r >= 0 && r < 8; r += step) {
            if (pawns & (1ULL << (file + r * 8))) {
                isDoubled = true;
                break;
            }
        }

        if (isDoubled) {
            score -= 15; // Penalty for doubled pawns
        }

        // Check for backward pawns
        bool isBackward = true;
        if (file > 0 && (pawns & (1ULL << (sq - 1 + step)))) isBackward = false; // Left diagonal
        if (file < 7 && (pawns & (1ULL << (sq + 1 + step)))) isBackward = false; // Right diagonal

        if (isBackward) {
            score -= 10; // Penalty for backward pawns
        }
    }

    return score;
}

// Evaluate king safety
int evaluateKingSafety(int side) {
    int score = 0;
    int kingSq = (side == WHITE) ? __builtin_ctzll(b.whiteKing) : __builtin_ctzll(b.blackKing);
    uint64_t enemyPieces = (side == WHITE) ? b.blackPieces : b.whitePieces;

    // King shield evaluation
    int kingFile = kingSq % 8;
    int kingRank = kingSq / 8;

    // Check pawns in front of the king
    for (int fileOffset = -1; fileOffset <= 1; fileOffset++) {
        int file = kingFile + fileOffset;
        if (file < 0 || file > 7) continue;

        int shieldSq = kingRank + (side == WHITE ? 1 : -1) * 8 + file;
        if (shieldSq >= 0 && shieldSq < 64) {
            if ((side == WHITE && (b.whitePawns & (1ULL << shieldSq))) ||
                (side == BLACK && (b.blackPawns & (1ULL << shieldSq)))) {
                score += 20; // Bonus for pawns in the king shield
            }
        }
    }

    // Open files near the king
    for (int fileOffset = -1; fileOffset <= 1; fileOffset++) {
        int file = kingFile + fileOffset;
        if (file < 0 || file > 7) continue;

        bool isOpen = true;
        for (int rank = 0; rank < 8; rank++) {
            int sq = file + rank * 8;
            if ((b.whitePawns & (1ULL << sq)) || (b.blackPawns & (1ULL << sq))) {
                isOpen = false;
                break;
            }
        }

        if (isOpen) {
            score -= 30; // Penalty for open files near the king
        }
    }

    // Attackers near the king
    int attackWeight = 0;
    for (int dir = 0; dir < 8; dir++) { // 8 directions (knight moves included)
        int sq = kingSq + vector[KING][dir];
        if (IS_SQ(sq) && (enemyPieces & (1ULL << sq))) {
            attackWeight += 10; // Increase attack weight for each enemy piece near the king
        }
    }

    score -= SafetyTable[attackWeight]; // Use safety table to penalize unsafe kings

    return score;
}

// Locate pieces to evalate mobility
int Board::pieceTypeAt(int sq) const {
    uint64_t mask = 1ULL << sq;

    if (whitePawns & mask || blackPawns & mask)     return PAWN;
    if (whiteKnights & mask || blackKnights & mask) return KNIGHT;
    if (whiteBishops & mask || blackBishops & mask) return BISHOP;
    if (whiteRooks & mask || blackRooks & mask)     return ROOK;
    if (whiteQueens & mask || blackQueens & mask)   return QUEEN;
    if (whiteKing & mask || blackKing & mask)       return KING;

    return -1; // Empty square
}


// Evaluate mobility
int evaluateMobility(int side) {
    int score = 0;
    uint64_t pieces = (side == WHITE) ? b.whitePieces : b.blackPieces;
    uint64_t enemyPieces = (side == WHITE) ? b.blackPieces : b.whitePieces;
    uint64_t allPieces = b.whitePieces | b.blackPieces;

    while (pieces) {
        int sq = __builtin_ctzll(pieces);
        pieces &= pieces - 1;

        int mobility = 0;
        int pieceType = b.pieceTypeAt(sq);

        switch (pieceType) {
            case KNIGHT:
                for (int dir = 0; dir < 8; dir++) {
                    int targetSq = sq + vector[KNIGHT][dir];
                    if (IS_SQ(targetSq) && !(allPieces & (1ULL << targetSq))) {
                        mobility++;
                    }
                }
            break;

            case BISHOP:
            case ROOK:
            case QUEEN:
                for (int dir = 0; dir < 8; dir++) {
                    int delta = vector[pieceType][dir];
                    int targetSq = sq;

                    while (true) {
                        targetSq += delta;
                        if (!IS_SQ(targetSq)) break;
                        if (allPieces & (1ULL << targetSq)) break;
                        mobility++;
                    }
                }
            break;

            default:
                break; // skip pawns, kings for mobility here
        }

        // Central square bonus
        if (CENTER_MASK & (1ULL << sq)) {
            mobility += 2;
        }

        // Proximity to enemy king
        int enemyKingSq = (side == WHITE) ? __builtin_ctzll(b.blackKing) : __builtin_ctzll(b.whiteKing);
        if (e.sqNearK[!side][enemyKingSq][sq]) {
            mobility += 3;
        }

        score += mobility;
    }

    return score;
}


// Evaluate the current position
int evaluatePosition() {
    int whiteScore = 0, blackScore = 0;

    // Calculate material score
    whiteScore += __builtin_popcountll(b.whitePawns) * PAWN_VALUE;
    whiteScore += __builtin_popcountll(b.whiteKnights) * KNIGHT_VALUE;
    whiteScore += __builtin_popcountll(b.whiteBishops) * BISHOP_VALUE;
    whiteScore += __builtin_popcountll(b.whiteRooks) * ROOK_VALUE;
    whiteScore += __builtin_popcountll(b.whiteQueens) * QUEEN_VALUE;
    whiteScore += __builtin_popcountll(b.whiteKing) * KING_VALUE;

    blackScore += __builtin_popcountll(b.blackPawns) * PAWN_VALUE;
    blackScore += __builtin_popcountll(b.blackKnights) * KNIGHT_VALUE;
    blackScore += __builtin_popcountll(b.blackBishops) * BISHOP_VALUE;
    blackScore += __builtin_popcountll(b.blackRooks) * ROOK_VALUE;
    blackScore += __builtin_popcountll(b.blackQueens) * QUEEN_VALUE;
    blackScore += __builtin_popcountll(b.blackKing) * KING_VALUE;

    // Add bonuses for center control
    whiteScore += __builtin_popcountll(b.whitePieces & CENTER_MASK) * CENTER_CONTROL;
    blackScore += __builtin_popcountll(b.blackPieces & CENTER_MASK) * CENTER_CONTROL;

    // Adjust material based on pawn count
    whiteScore += knight_adj[__builtin_popcountll(b.whitePawns)];
    blackScore += knight_adj[__builtin_popcountll(b.blackPawns)];
    whiteScore += rook_adj[__builtin_popcountll(b.whitePawns)];
    blackScore += rook_adj[__builtin_popcountll(b.blackPawns)];

    // Evaluate pawn structure
    whiteScore += evaluatePawnStructure(WHITE);
    blackScore += evaluatePawnStructure(BLACK);

    // Evaluate king safety
    whiteScore += evaluateKingSafety(WHITE);
    blackScore += evaluateKingSafety(BLACK);

    // Evaluate mobility
    whiteScore += evaluateMobility(WHITE);
    blackScore += evaluateMobility(BLACK);

    // Return evaluation (positive if White is better, negative if Black is better)
    return whiteScore - blackScore;
}
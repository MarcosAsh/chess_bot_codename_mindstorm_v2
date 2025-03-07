#ifndef BOARD_H
#define BOARD_H

#include <vector>
#include <cstdint>
#include <stack>
#include <unordered_map>
#include <string>

// Board state
extern uint64_t whitePawns, whiteKnights, whiteBishops, whiteRooks, whiteQueens, whiteKing;
extern uint64_t blackPawns, blackKnights, blackBishops, blackRooks, blackQueens, blackKing;
extern uint64_t whitePieces, blackPieces, allPieces;
extern bool whiteKingsideCastle, whiteQueensideCastle;
extern bool blackKingsideCastle, blackQueensideCastle;
extern uint64_t enPassantTarget;

// Zobrist hashing
extern std::stack<uint64_t> zobristHistory;
extern uint64_t zobristTable[12][64];
extern std::unordered_map<uint64_t, std::pair<int, int>> transpositionTable;

// Board functions
void initializePosition();
void printBitboard(uint64_t bitboard);
void printBoardForPlayers();
bool isSquareAttacked(uint64_t square, bool byWhite);
bool isMoveLegal(uint64_t fromSquare, uint64_t toSquare, bool isWhite);
std::vector<uint64_t> generatePawnMoves(uint64_t pawns, bool isWhite);
std::vector<uint64_t> generateKnightMoves(uint64_t knights, bool isWhite);
std::vector<uint64_t> generateBishopMoves(uint64_t bishops, bool isWhite);
std::vector<uint64_t> generateRookMoves(uint64_t rooks, bool isWhite);
std::vector<uint64_t> generateQueenMoves(uint64_t queens, bool isWhite);
std::vector<uint64_t> generateKingMoves(uint64_t king, bool isWhite);
bool canCastleKingside(bool isWhite);
bool canCastleQueenside(bool isWhite);
std::vector<uint64_t> generateEnPassantMoves(uint64_t pawns, bool isWhite);
bool makeMove(int fromSquare, int toSquare, bool isWhiteTurn);
void handlePawnPromotion(uint64_t toBit, uint64_t isWhiteTurn);
bool isCheckmateOrStalemate(bool isWhiteTurn);

// Sliding piece moves (rooks and bishops), with blockers
uint64_t slideMove(uint64_t piece, int direction, uint64_t blockers);

// Save and restore board state
void saveBoardState(bool isWhiteTurn);
void undoMove();

// Convert a square in bitboard format to chess notation (e.g., 1ULL << 0 -> "a1")
std::string squareToNotation(uint64_t square);

// Board state
extern uint64_t whitePawns, whiteKnights, whiteBishops, whiteRooks, whiteQueens, whiteKing;
extern uint64_t blackPawns, blackKnights, blackBishops, blackRooks, blackQueens, blackKing;
extern uint64_t whitePieces, blackPieces, allPieces;

// Board structure
struct Board {
    uint64_t whitePawns, whiteKnights, whiteBishops, whiteRooks, whiteQueens, whiteKing;
    uint64_t blackPawns, blackKnights, blackBishops, blackRooks, blackQueens, blackKing;
    uint64_t whitePieces, blackPieces, allPieces;
};

extern Board b; // Global board instance
#endif // BOARD_H
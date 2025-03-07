#include <cstdint>
#include <stack>
#include <unordered_map>
#include "board.h"
#include "../constants.h"
#include <iostream>

// Initialize the global board instance
Board b;

// Board state
uint64_t whitePawns, whiteKnights, whiteBishops, whiteRooks, whiteQueens, whiteKing;
uint64_t blackPawns, blackKnights, blackBishops, blackRooks, blackQueens, blackKing;
uint64_t whitePieces, blackPieces, allPieces;
bool whiteKingsideCastle = true, whiteQueensideCastle = true;
bool blackKingsideCastle = true, blackQueensideCastle = true;
uint64_t enPassantTarget = 0;

// Zobrist hashing
std::stack<uint64_t> zobristHistory;
uint64_t zobristTable[12][64];
std::unordered_map<uint64_t, std::pair<int, int>> transpositionTable;

// Initialize the board to the standard starting position
void initializePosition() {
    whitePawns = 0x000000000000FF00ULL;
    whiteKnights = 0x0000000000000042ULL;
    whiteBishops = 0x0000000000000024ULL;
    whiteRooks = 0x0000000000000081ULL;
    whiteQueens = 0x0000000000000008ULL;
    whiteKing = 0x0000000000000010ULL;

    blackPawns = 0x00FF000000000000ULL;
    blackKnights = 0x4200000000000000ULL;
    blackBishops = 0x2400000000000000ULL;
    blackRooks = 0x8100000000000000ULL;
    blackQueens = 0x0800000000000000ULL;
    blackKing = 0x1000000000000000ULL;

    whitePieces = whitePawns | whiteKnights | whiteBishops | whiteRooks | whiteQueens | whiteKing;
    blackPieces = blackPawns | blackKnights | blackBishops | blackRooks | blackQueens | blackKing;
    allPieces = whitePieces | blackPieces;

    whiteKingsideCastle = whiteQueensideCastle = true;
    blackKingsideCastle = blackQueensideCastle = true;
    enPassantTarget = 0;

    // Initialize Zobrist hash for the initial position
    zobristHistory.push(0); // Push an initial Zobrist hash (to be calculated dynamically)
}

// Sliding piece moves (rooks and bishops), with blockers
uint64_t slideMove(uint64_t piece, int direction, uint64_t blockers) {
    uint64_t moves = 0;
    uint64_t temp = piece;

    while (temp) {
        temp = (direction > 0) ? (temp << direction) : (temp >> -direction);
        if (temp & blockers) {  // Stop at blockers
            moves |= temp;      // Include the blocker square for capture
            break;
        }
        if ((temp & FILE_A) && (direction == -1 || direction == 7 || direction == -9)) break; // Prevent wrap from H to A
        if ((temp & FILE_H) && (direction == 1 || direction == -7 || direction == 9)) break;  // Prevent wrap from A to H
        moves |= temp;
    }
    return moves;
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

// Print the board in a human-readable format
void printBoardForPlayers() {
    std::cout << "\nCurrent Board:\n";
    std::cout << "  a b c d e f g h\n +----------------+\n";
    for (int rank = 7; rank >= 0; --rank) {
        std::cout << rank + 1 << "| ";
        for (int file = 0; file < 8; ++file) {
            int square = rank * 8 + file;
            uint64_t mask = 1ULL << square;
            if (whitePawns & mask) std::cout << "P ";
            else if (whiteKnights & mask) std::cout << "N ";
            else if (whiteBishops & mask) std::cout << "B ";
            else if (whiteRooks & mask) std::cout << "R ";
            else if (whiteQueens & mask) std::cout << "Q ";
            else if (whiteKing & mask) std::cout << "K ";
            else if (blackPawns & mask) std::cout << "p ";
            else if (blackKnights & mask) std::cout << "n ";
            else if (blackBishops & mask) std::cout << "b ";
            else if (blackRooks & mask) std::cout << "r ";
            else if (blackQueens & mask) std::cout << "q ";
            else if (blackKing & mask) std::cout << "k ";
            else std::cout << ". ";
        }
        std::cout << "|\n";
    }
    std::cout << " +----------------+\n";
}

// Check if a square is attacked by any enemy piece
bool isSquareAttacked(uint64_t square, bool byWhite) {
    uint64_t enemyPawns = byWhite ? whitePawns : blackPawns;
    uint64_t enemyKnights = byWhite ? whiteKnights : blackKnights;
    uint64_t enemyBishops = byWhite ? whiteBishops : blackBishops;
    uint64_t enemyRooks = byWhite ? whiteRooks : blackRooks;
    uint64_t enemyQueens = byWhite ? whiteQueens : blackQueens;
    uint64_t enemyKing = byWhite ? whiteKing : blackKing;
    uint64_t enemyPieces = byWhite ? whitePieces : blackPieces;

    // Pawn attacks
    if (byWhite) {
        if (((enemyPawns << 7) & ~FILE_H & square) || ((enemyPawns << 9) & ~FILE_A & square)) {
            return true; // White Pawn attacking left or right
        }
    } else {
        if (((enemyPawns >> 7) & ~FILE_A & square) || ((enemyPawns >> 9) & ~FILE_H & square)) {
            return true; // Black Pawn attacking left or right
        }
    }

    // Knight attacks
    uint64_t knightAttacks = ((square << 17) & ~FILE_A) | ((square << 15) & ~FILE_H) |
                             ((square >> 17) & ~FILE_H) | ((square >> 15) & ~FILE_A) |
                             ((square << 10) & ~(FILE_A | FILE_B)) | ((square >> 10) & ~(FILE_H | FILE_G)) |
                             ((square << 6) & ~(FILE_H | FILE_G)) | ((square >> 6) & ~(FILE_A | FILE_B));
    if (enemyKnights & knightAttacks) return true;

    // Sliding piece attacks (bishops, rooks, queens)
    uint64_t bishopAttacks = slideMove(square, 7, enemyPieces) | slideMove(square, 9, enemyPieces) |
                             slideMove(square, -7, enemyPieces) | slideMove(square, -9, enemyPieces);
    if (enemyBishops & bishopAttacks || enemyQueens & bishopAttacks) return true;

    uint64_t rookAttacks = slideMove(square, 1, enemyPieces) | slideMove(square, -1, enemyPieces) |
                           slideMove(square, 8, enemyPieces) | slideMove(square, -8, enemyPieces);
    if (enemyRooks & rookAttacks || enemyQueens & rookAttacks) return true;

    // King attacks
    uint64_t kingAttacks = (square << 8) | (square >> 8) |
                           ((square << 1) & ~FILE_A) | ((square >> 1) & ~FILE_H) |
                           ((square << 9) & ~FILE_A) | ((square >> 9) & ~FILE_H) |
                           ((square << 7) & ~FILE_H) | ((square >> 7) & ~FILE_A);
    if (enemyKing & kingAttacks) return true;

    return false;
}

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

// Check if a move is legal
bool isMoveLegal(uint64_t fromSquare, uint64_t toSquare, bool isWhite) {
    uint64_t savedWhitePieces = whitePieces;
    uint64_t savedBlackPieces = blackPieces;
    uint64_t savedAllPieces = allPieces;
    uint64_t king = isWhite ? whiteKing : blackKing;

    // Temporarily make the move
    if (isWhite) {
        whitePieces ^= fromSquare | toSquare;
        allPieces = whitePieces | blackPieces;
    } else {
        blackPieces ^= fromSquare | toSquare;
        allPieces = whitePieces | blackPieces;
    }

    // Check if the king is in check
    bool kingInCheck = isSquareAttacked(king, !isWhite);
    if (kingInCheck) {
        std::cout << "Move from " << squareToNotation(fromSquare) << " to " << squareToNotation(toSquare)
                  << " leaves king in check. Illegal.\n";
    }

    // Restore original positions
    whitePieces = savedWhitePieces;
    blackPieces = savedBlackPieces;
    allPieces = savedAllPieces;

    return !kingInCheck;
}

// Generate pawn moves
std::vector<uint64_t> generatePawnMoves(uint64_t pawns, bool isWhite) {
    std::vector<uint64_t> moves;
    uint64_t singleStep, doubleStep, attacksLeft, attacksRight;

    if (isWhite) {
        singleStep = (pawns << 8) & ~allPieces;
        doubleStep = ((pawns & RANK_2) << 16) & ~allPieces & ~(allPieces << 8);
        attacksLeft = (pawns << 7) & blackPieces & ~FILE_H;
        attacksRight = (pawns << 9) & blackPieces & ~FILE_A;
    } else {
        singleStep = (pawns >> 8) & ~allPieces;
        doubleStep = ((pawns & RANK_7) >> 16) & ~allPieces & ~(allPieces >> 8);
        attacksLeft = (pawns >> 7) & whitePieces & ~FILE_A;
        attacksRight = (pawns >> 9) & whitePieces & ~FILE_H;
    }

    if (singleStep) moves.push_back(singleStep);
    if (doubleStep) moves.push_back(doubleStep);
    if (attacksLeft) moves.push_back(attacksLeft);
    if (attacksRight) moves.push_back(attacksRight);

    return moves;
}

// Generate knight moves
std::vector<uint64_t> generateKnightMoves(uint64_t knights, bool isWhite) {
    std::vector<uint64_t> moves;
    uint64_t targets = isWhite ? blackPieces : whitePieces;
    uint64_t potentialMoves;

    while (knights) {
        uint64_t knight = knights & -knights;
        knights &= knights - 1;

        potentialMoves = ((knight << 17) & ~FILE_A) | ((knight << 15) & ~FILE_H) |
                         ((knight << 10) & ~(FILE_A | FILE_B)) | ((knight << 6) & ~(FILE_G | FILE_H)) |
                         ((knight >> 17) & ~FILE_H) | ((knight >> 15) & ~FILE_A) |
                         ((knight >> 10) & ~(FILE_G | FILE_H)) | ((knight >> 6) & ~(FILE_A | FILE_B));

        moves.push_back(potentialMoves & ~targets);
    }
    return moves;
}

// Generate bishop moves (diagonals)
std::vector<uint64_t> generateBishopMoves(uint64_t bishops, bool isWhite) {
    std::vector<uint64_t> moves;
    uint64_t targets = isWhite ? blackPieces : whitePieces;

    while (bishops) {
        uint64_t bishop = bishops & -bishops;
        bishops &= bishops - 1;
        uint64_t diagonalMoves = slideMove(bishop, 9, allPieces) | slideMove(bishop, 7, allPieces) |
                                 slideMove(bishop, -9, allPieces) | slideMove(bishop, -7, allPieces);
        moves.push_back(diagonalMoves & ~targets);
    }
    return moves;
}

// Generate rook moves (straight lines)
std::vector<uint64_t> generateRookMoves(uint64_t rooks, bool isWhite) {
    std::vector<uint64_t> moves;
    uint64_t targets = isWhite ? blackPieces : whitePieces;

    while (rooks) {
        uint64_t rook = rooks & -rooks;
        rooks &= rooks - 1;
        uint64_t straightMoves = slideMove(rook, 8, allPieces) | slideMove(rook, -8, allPieces) |
                                 slideMove(rook, 1, allPieces) | slideMove(rook, -1, allPieces);
        moves.push_back(straightMoves & ~targets);
    }
    return moves;
}

// Generate queen moves by combining rook and bishop moves
std::vector<uint64_t> generateQueenMoves(uint64_t queens, bool isWhite) {
    std::vector<uint64_t> moves;
    uint64_t targets = isWhite ? blackPieces : whitePieces;

    while (queens) {
        uint64_t queen = queens & -queens;
        queens &= queens - 1;
        uint64_t queenMoves = slideMove(queen, 8, allPieces) | slideMove(queen, -8, allPieces) |
                              slideMove(queen, 1, allPieces) | slideMove(queen, -1, allPieces) |
                              slideMove(queen, 9, allPieces) | slideMove(queen, 7, allPieces) |
                              slideMove(queen, -9, allPieces) | slideMove(queen, -7, allPieces);
        moves.push_back(queenMoves & ~targets);
    }
    return moves;
}

// Generate king moves
std::vector<uint64_t> generateKingMoves(uint64_t king, bool isWhite) {
    std::vector<uint64_t> moves;
    uint64_t targets = isWhite ? blackPieces : whitePieces;

    uint64_t kingMoves = ((king << 8) | (king >> 8) | ((king & ~FILE_H) << 1) | ((king & ~FILE_A) >> 1) |
                          ((king & ~FILE_H) << 9) | ((king & ~FILE_A) << 7) |
                          ((king & ~FILE_H) >> 7) | ((king & ~FILE_A) >> 9));
    moves.push_back(kingMoves & ~targets);

    return moves;
}

// Check if the king can castle kingside
bool canCastleKingside(bool isWhite) {
    uint64_t kingPosition = isWhite ? whiteKing : blackKing;
    uint64_t rookPosition = isWhite ? whiteRooks : blackRooks;
    uint64_t kingsideMask = isWhite ? 0x60ULL : 0x6000000000000000ULL;

    bool kingsideAvailable = (isWhite ? whiteKingsideCastle : blackKingsideCastle) &&
                             !(allPieces & kingsideMask) &&
                             !isSquareAttacked(kingPosition, !isWhite) &&
                             !isSquareAttacked(kingPosition << 1, !isWhite) &&
                             !isSquareAttacked(kingPosition << 2, !isWhite);
    return kingsideAvailable;
}

// Check if the king can castle queenside
bool canCastleQueenside(bool isWhite) {
    uint64_t kingPosition = isWhite ? whiteKing : blackKing;
    uint64_t rookPosition = isWhite ? whiteRooks : blackRooks;
    uint64_t queensideMask = isWhite ? 0xEULL : 0xE00000000000000ULL;

    bool queensideAvailable = (isWhite ? whiteQueensideCastle : blackQueensideCastle) &&
                              !(allPieces & queensideMask) &&
                              !isSquareAttacked(kingPosition, !isWhite) &&
                              !isSquareAttacked(kingPosition >> 1, !isWhite) &&
                              !isSquareAttacked(kingPosition >> 2, !isWhite);
    return queensideAvailable;
}

// Generate en passant moves
std::vector<uint64_t> generateEnPassantMoves(uint64_t pawns, bool isWhite) {
    std::vector<uint64_t> moves;
    if (enPassantTarget == 0) return moves;

    uint64_t enPassantLeft = isWhite ? (pawns << 7) & ~FILE_H & enPassantTarget
                                      : (pawns >> 7) & ~FILE_A & enPassantTarget;
    uint64_t enPassantRight = isWhite ? (pawns << 9) & ~FILE_A & enPassantTarget
                                       : (pawns >> 9) & ~FILE_H & enPassantTarget;

    if (enPassantLeft) moves.push_back(enPassantLeft);
    if (enPassantRight) moves.push_back(enPassantRight);
    return moves;
}

// Handle pawn promotion
void handlePawnPromotion(uint64_t toBit, uint64_t isWhiteTurn) {
    if (isWhiteTurn && (toBit & RANK_8)) {
        whitePawns ^= toBit;
        whiteQueens |= toBit;
    } else if (!isWhiteTurn && (toBit & RANK_1)) {
        blackPawns ^= toBit;
        blackQueens |= toBit;
    }
    whitePieces = whitePawns | whiteKnights | whiteBishops | whiteRooks | whiteQueens | whiteKing;
    blackPieces = blackPawns | blackKnights | blackBishops | blackRooks | blackQueens | blackKing;
    allPieces = whitePieces | blackPieces;
}

// Check if the current position is checkmate or stalemate
bool isCheckmateOrStalemate(bool isWhiteTurn) {
    uint64_t king = isWhiteTurn ? whiteKing : blackKing;
    bool kingInCheck = isSquareAttacked(king, !isWhiteTurn);

    // Iterate over all pieces of the current player and try all possible moves
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

        // Check each legal move to see if it leaves the king safe
        for (uint64_t move : legalMoves) {
            if (isMoveLegal(piece, move, isWhiteTurn)) {
                // If at least one legal move exists, it's not checkmate or stalemate
                return false;
            }
        }
    }

    // If no legal moves and king is in check, it's checkmate; otherwise, stalemate
    return kingInCheck;
}
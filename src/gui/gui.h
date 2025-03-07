#ifndef GUI_H
#define GUI_H

#include <SFML/Graphics.hpp>
#include <stack>
#include <vector>
#include "engine/board.h"

// Constants for the GUI
const int TILE_SIZE = 100;    // Size of each square on the board
const int BOARD_SIZE = 8;     // Board dimensions (8x8)

// Global variables for the GUI
extern sf::Vector2i selectedSquare; // Currently selected square for dragging
extern bool isDragging;             // Whether a piece is being dragged
extern sf::Vector2f dragOffset;     // Offset between mouse and piece position

// Undo/Redo stacks for the GUI
extern std::stack<std::vector<std::vector<char>>> undoStack;
extern std::stack<std::vector<std::vector<char>>> redoStack;

// Simplified chessboard representation for the GUI
extern std::vector<std::vector<char>> board;

// Function declarations
void runGUI();
void drawBoard(sf::RenderWindow& window, sf::Font& font, sf::RectangleShape& piece);
void handleDragAndDrop(sf::Event& event, sf::RenderWindow& window);
void undoMove();
void redoMove();
void generateAIMove(std::vector<std::vector<char>>& board);
void animateMove(sf::RenderWindow& window, sf::RectangleShape& piece, sf::Vector2f start, sf::Vector2f end);

#endif // GUI_H
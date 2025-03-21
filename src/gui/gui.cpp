#include "gui.h"
#include "../engine/board.h"

// Global variables
sf::Vector2i selectedSquare(-1, -1);
bool isDragging = false;
sf::Vector2f dragOffset;
std::stack<std::vector<std::vector<char>>> undoStack;
std::stack<std::vector<std::vector<char>>> redoStack;
std::vector<std::vector<char>> board = {
    {'r', 'n', 'b', 'q', 'k', 'b', 'n', 'r'},
    {'p', 'p', 'p', 'p', 'p', 'p', 'p', 'p'},
    {'.', '.', '.', '.', '.', '.', '.', '.'},
    {'.', '.', '.', '.', '.', '.', '.', '.'},
    {'.', '.', '.', '.', '.', '.', '.', '.'},
    {'.', '.', '.', '.', '.', '.', '.', '.'},
    {'P', 'P', 'P', 'P', 'P', 'P', 'P', 'P'},
    {'R', 'N', 'B', 'Q', 'K', 'B', 'N', 'R'}
};

void runGUI() {
    sf::RenderWindow window(sf::VideoMode(TILE_SIZE * BOARD_SIZE, TILE_SIZE * BOARD_SIZE), "Chess GUI");
    sf::Font font;
    if (!font.loadFromFile("assets/arial.ttf")) {
        printf("Failed to load font\n");
        return;
    }

    sf::RectangleShape piece(sf::Vector2f(TILE_SIZE, TILE_SIZE));
    piece.setFillColor(sf::Color::Transparent);

    bool aiTurn = false;

    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed)
                window.close();

            if (!aiTurn) {
                handleDragAndDrop(event, window);
                if (sf::Keyboard::isKeyPressed(sf::Keyboard::Z)) undoMove();
                if (sf::Keyboard::isKeyPressed(sf::Keyboard::Y)) redoMove();
            }
        }

        if (aiTurn) {
            generateAIMove(board);
            aiTurn = false;
        }

        window.clear();
        drawBoard(window, font, piece);
        window.display();
    }
}

void drawBoard(sf::RenderWindow& window, sf::Font& font, sf::RectangleShape& piece) {
    for (int row = 0; row < BOARD_SIZE; ++row) {
        for (int col = 0; col < BOARD_SIZE; ++col) {
            sf::RectangleShape square(sf::Vector2f(TILE_SIZE, TILE_SIZE));
            square.setPosition(col * TILE_SIZE, row * TILE_SIZE);
            square.setFillColor((row + col) % 2 == 0 ? sf::Color::White : sf::Color::Black);
            window.draw(square);

            if (board[row][col] != '.') {
                piece.setPosition(col * TILE_SIZE, row * TILE_SIZE);
                piece.setFillColor(sf::Color::Transparent);
                piece.setOutlineColor(sf::Color::Red);
                piece.setOutlineThickness(2);
                window.draw(piece);

                // Draw piece as text
                sf::Text pieceText;
                pieceText.setFont(font);
                pieceText.setString(board[row][col]);
                pieceText.setCharacterSize(48);
                pieceText.setFillColor((row + col) % 2 == 0 ? sf::Color::Black : sf::Color::White);
                pieceText.setPosition(col * TILE_SIZE + TILE_SIZE / 4, row * TILE_SIZE + TILE_SIZE / 6);
                window.draw(pieceText);
            }
        }
    }
}

void handleDragAndDrop(sf::Event& event, sf::RenderWindow& window) {
    if (event.type == sf::Event::MouseButtonPressed && event.mouseButton.button == sf::Mouse::Left) {
        sf::Vector2i mousePos = sf::Mouse::getPosition(window);
        int col = mousePos.x / TILE_SIZE;
        int row = mousePos.y / TILE_SIZE;

        if (board[row][col] != '.') {
            selectedSquare = sf::Vector2i(row, col);
            dragOffset = sf::Vector2f(mousePos.x - col * TILE_SIZE, mousePos.y - row * TILE_SIZE);
            isDragging = true;
        }
    }

    if (event.type == sf::Event::MouseButtonReleased && event.mouseButton.button == sf::Mouse::Left) {
        if (isDragging) {
            sf::Vector2i mousePos = sf::Mouse::getPosition(window);
            int col = mousePos.x / TILE_SIZE;
            int row = mousePos.y / TILE_SIZE;

            // Dummy move validation
            if (row >= 0 && row < BOARD_SIZE && col >= 0 && col < BOARD_SIZE) {
                board[row][col] = board[selectedSquare.x][selectedSquare.y];
                board[selectedSquare.x][selectedSquare.y] = '.';
                undoStack.push(board); // Save state for undo
                while (!redoStack.empty()) redoStack.pop(); // Clear redo stack
            }

            isDragging = false;
            selectedSquare = sf::Vector2i(-1, -1);
        }
    }
}

void undoMove() {
    if (!undoStack.empty()) {
        redoStack.push(board);
        board = undoStack.top();
        undoStack.pop();
    }
}

void redoMove() {
    if (!redoStack.empty()) {
        undoStack.push(board);
        board = redoStack.top();
        redoStack.pop();
    }
}

void generateAIMove(std::vector<std::vector<char>>& board) {
    board[1][0] = '.'; // Move a pawn forward
    board[2][0] = 'p';
}

void animateMove(sf::RenderWindow& window, sf::RectangleShape& piece, sf::Vector2f start, sf::Vector2f end) {
    sf::Clock clock;
    while (true) {
        float elapsed = clock.getElapsedTime().asSeconds();
        if (elapsed > 0.5f) break;

        sf::Vector2f pos = start + elapsed * (end - start);
        piece.setPosition(pos);
        window.clear();
        window.draw(piece);
        window.display();
    }
}
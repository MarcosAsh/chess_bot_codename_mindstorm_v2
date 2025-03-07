#ifndef EVALUATION_H
#define EVALUATION_H

#include <cstdint>

// Evaluation functions
int evaluatePosition();

// Evaluation structure
struct Evaluation {
    int sqNearK[2][64][64]; // Example: squares near the king
};

extern Evaluation e; // Global evaluation instance

#endif
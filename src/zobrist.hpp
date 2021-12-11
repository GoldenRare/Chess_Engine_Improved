#ifndef ZOBRIST_HPP
#define ZOBRIST_HPP

#include "utility.hpp"

constexpr int FULL_CASTLING_RIGHTS = 16;

extern PositionKey zobristPieceSquare[PIECES][NUMBER_OF_SQUARES];
extern PositionKey zobristBlackToMove;
extern PositionKey zobristCastlingRights[FULL_CASTLING_RIGHTS];
extern PositionKey zobristEnPassant[FILES + 1];

void initZobrist();
void initZobristKeys();

#endif
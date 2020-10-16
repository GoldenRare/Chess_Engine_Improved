#ifndef MOVE_SORTER_HPP
#define MOVE_SORTER_HPP

#include "utility.hpp"
#include "board.hpp"

constexpr int MAX_PLY = 256; 
extern unsigned int killerMoves[MAX_PLY + MAX_PLY][2]; //Store 2 moves per ply

void findBestMove(Move* movesListStart, Move* movesListEnd, unsigned int& hashMove); //Returns the best move to the front of the list
void assignMoveScores(Move* movesListStart, Move* movesListEnd, ChessBoard& board);

#endif
#ifndef SEARCH_HPP
#define SEARCH_HPP

#include "utility.hpp"
#include "board.hpp"
#include "evaluation.hpp"

Bitboard perft(ChessBoard& board, int depth);
void divide(ChessBoard& board, int depth);
ExactScore alphaBeta(ChessBoard& board, int alpha, int beta, int depth);
void iterativeDeepening(ChessBoard& board, int maxDepth);
ExactScore quiescenceSearch(ChessBoard& board, int alpha, int beta);

#endif
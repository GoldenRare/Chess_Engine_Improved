#ifndef SEARCH_HPP
#define SEARCH_HPP

#include "utility.hpp"
#include "board.hpp"
#include "evaluation.hpp"

constexpr int RAZOR_MARGIN = 531;
extern int REDUCTIONS[256];

Bitboard perft(ChessBoard& board, int depth);
void divide(ChessBoard& board, int depth);
ExactScore alphaBeta(ChessBoard& board, int alpha, int beta, int depth, bool isPVNode, bool isCutNode, int ply);
void iterativeDeepening(ChessBoard& board, int maxDepth);
ExactScore quiescenceSearch(ChessBoard& board, int alpha, int beta, bool isPVNode);
bool isRepetition(ChessBoard& board);
bool SEE(ChessBoard& board, Move move, int materialValue); // Static Exchange Evaluation using the Stockfish version, (Static Exchange Evaluation Greater or Equal)
int reductions(int depth, int numberOfMoves);

void initSearch();
void initReductions();

#endif
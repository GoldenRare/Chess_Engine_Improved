<<<<<<< HEAD
#ifndef MOVE_GENERATION_HPP
#define MOVE_GENERATION_HPP

#include "board.hpp"

Move* generateAllPseudoMoves(const ChessBoard& board, Move* movesList);
Move* generatePawnMoves     (const ChessBoard& board, Move* movesList);
Move* generateKnightMoves   (const ChessBoard& board, Move* movesList);
Move* generateBishopMoves   (const ChessBoard& board, Move* movesList);
Move* generateRookMoves     (const ChessBoard& board, Move* movesList);
Move* generateQueenMoves    (const ChessBoard& board, Move* movesList);
Move* generateKingMoves     (const ChessBoard& board, Move* movesList);

Move* generateAllPseudoCaptureMoves(const ChessBoard& board, Move* movesList);
Move* generatePawnCaptureMoves     (const ChessBoard& board, Move* movesList);
Move* generateKnightCaptureMoves   (const ChessBoard& board, Move* movesList);
Move* generateBishopCaptureMoves   (const ChessBoard& board, Move* movesList);
Move* generateRookCaptureMoves     (const ChessBoard& board, Move* movesList);
Move* generateQueenCaptureMoves    (const ChessBoard& board, Move* movesList);
Move* generateKingCaptureMoves     (const ChessBoard& board, Move* movesList);

=======
#ifndef MOVE_GENERATION_HPP
#define MOVE_GENERATION_HPP

#include "board.hpp"

Move* generateAllPseudoMoves(const ChessBoard& board, Move* movesList);
Move* generatePawnMoves     (const ChessBoard& board, Move* movesList);
Move* generateKnightMoves   (const ChessBoard& board, Move* movesList);
Move* generateBishopMoves   (const ChessBoard& board, Move* movesList);
Move* generateRookMoves     (const ChessBoard& board, Move* movesList);
Move* generateQueenMoves    (const ChessBoard& board, Move* movesList);
Move* generateKingMoves     (const ChessBoard& board, Move* movesList);

Move* generateAllPseudoCaptureMoves(const ChessBoard& board, Move* movesList);
Move* generatePawnCaptureMoves     (const ChessBoard& board, Move* movesList);
Move* generateKnightCaptureMoves   (const ChessBoard& board, Move* movesList);
Move* generateBishopCaptureMoves   (const ChessBoard& board, Move* movesList);
Move* generateRookCaptureMoves     (const ChessBoard& board, Move* movesList);
Move* generateQueenCaptureMoves    (const ChessBoard& board, Move* movesList);
Move* generateKingCaptureMoves     (const ChessBoard& board, Move* movesList);

>>>>>>> 61b4809289c1189e58962f19777acf3e93307f2a
#endif
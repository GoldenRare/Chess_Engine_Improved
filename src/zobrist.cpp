#include "zobrist.hpp"
#include "randomNumber.hpp"
#include "utility.hpp"

PositionKey zobristPieceSquare[PIECES][SQUARES];
PositionKey zobristBlackToMove;
PositionKey zobristCastlingRights[FULL_CASTLING_RIGHTS];
PositionKey zobristEnPassant[FILES + 1];

void initZobrist() {
    initZobristKeys();
}

void initZobristKeys() {

    for(Piece p = WHITE_PAWN; p <= BLACK_KING; p++) {
        for (Square sq = H1; sq <= A8; sq++) {
            zobristPieceSquare[p][sq] = random64.random();
        }
    }

    zobristBlackToMove = random64.random();
    
    for (int i = 0; i < FULL_CASTLING_RIGHTS; i++)
        zobristCastlingRights[i] = random64.random();

    for (File f = FILE_A; f <= FILE_H; f++)
        zobristEnPassant[f] = random64.random();
    zobristEnPassant[FILES] = 0;
}
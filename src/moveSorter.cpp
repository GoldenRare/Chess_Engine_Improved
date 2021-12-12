#include "moveSorter.hpp"
#include "utility.hpp"
#include "evaluation.hpp"

unsigned int killerMoves[MAX_PLY + MAX_PLY][2] = { };

void findBestMove(Move* movesListStart, Move* movesListEnd, unsigned int& hashMove) {

    Move* start = movesListStart;
    if (hashMove != 0) {

        while (movesListStart < movesListEnd) {
            if(movesListStart->move == hashMove) {
                Move temp = *start;
                *start = *movesListStart;
                *movesListStart = temp;
                break;
            }
            movesListStart++;
        }
        hashMove = 0;
    } else {
        Move* max = start;
        while (++movesListStart < movesListEnd) {
            if(movesListStart->score > max->score) max = movesListStart;
        }
        Move temp = *start;
        *start = *max;
        *max = temp;

    }
    
}

void assignMoveScores(Move* movesListStart, Move* movesListEnd, ChessBoard& board) {

    MoveType moveType;
    while (movesListStart < movesListEnd) {

        moveType = typeOfMove(*movesListStart);

        //Black gets penalized more due to the higher index in enum Piece{}
        //Captures scored based on MVV/LVA (Most Valuable Victim/Least Valuable Attacker)
        if (moveType == CAPTURE) 
            movesListStart->score = PIECE_VALUE[MIDDLEGAME][board.pieceBoard[getTo(*movesListStart)]] - board.pieceBoard[getFrom(*movesListStart)];

        else if (moveType == EN_PASSANT_CAPTURE)
            movesListStart->score = PIECE_VALUE[MIDDLEGAME][WHITE_PAWN] - (board.sideToPlay == WHITE) ? WHITE_PAWN : BLACK_PAWN;

        else if ((moveType == KNIGHT_PROMOTION_CAPTURE) || (moveType == BISHOP_PROMOTION_CAPTURE)
              || (moveType == ROOK_PROMOTION_CAPTURE)   || (moveType == QUEEN_PROMOTION_CAPTURE)) {

            Piece piece = (moveType == KNIGHT_PROMOTION_CAPTURE) ? WHITE_KNIGHT
                        : (moveType == BISHOP_PROMOTION_CAPTURE) ? WHITE_BISHOP
                        : (moveType == ROOK_PROMOTION_CAPTURE  ) ? WHITE_ROOK
                        :                                          WHITE_QUEEN;

            movesListStart->score = PIECE_VALUE[MIDDLEGAME][board.pieceBoard[getTo(*movesListStart)]] - (board.sideToPlay == WHITE) ? WHITE_PAWN : BLACK_PAWN
                                  + PIECE_VALUE[MIDDLEGAME][piece] - PIECE_VALUE[MIDDLEGAME][WHITE_PAWN];

        } 
        else if (movesListStart->move == killerMoves[board.ply][0]) movesListStart->score = 2; //2 is enough for now to ensure they are done after captures 
        else if (movesListStart->move == killerMoves[board.ply][1]) movesListStart->score = 1; //1 to make sure it is done after the first killerMove
              
        movesListStart++;
    }
}

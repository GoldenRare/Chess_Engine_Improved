
#include <algorithm>
#include "evaluation.hpp"
#include "utility.hpp"
#include "board.hpp"

CombinedScore pieceSquareTable[PIECES][NUMBER_OF_SQUARES];

int gamePhase(const ChessBoard& board) {
    
    int phase = TOTAL_PHASE;

    int numberOfKnights = board.pieceCount[WHITE_KNIGHT] + board.pieceCount[BLACK_KNIGHT];
    int numberOfBishops = board.pieceCount[WHITE_BISHOP] + board.pieceCount[BLACK_BISHOP];
    int numberOfRooks   = board.pieceCount[WHITE_ROOK]   + board.pieceCount[BLACK_ROOK]  ; 
    int numberOfQueens  = board.pieceCount[WHITE_QUEEN]  + board.pieceCount[BLACK_QUEEN] ;

    phase = phase - (KNIGHT_PHASE * numberOfKnights) - (BISHOP_PHASE * numberOfBishops) 
                  - (ROOK_PHASE   * numberOfRooks  ) - (QUEEN_PHASE  * numberOfQueens );

    phase = ((phase * 256) + (TOTAL_PHASE / 2)) / TOTAL_PHASE;
    return phase;  

}

ExactScore evaluatePosition(const ChessBoard& board) {

    ExactScore evaluation;
    CombinedScore cs;

    cs = evaluatePieceSquareScore(board);
    cs = (board.sideToPlay == WHITE) ? cs : -cs; //Score relative to the side to play

    int phase = gamePhase(board);
    evaluation = ((middlegameScore(cs) * (256 - phase)) + (endgameScore(cs) * phase)) / 256; //Formula to create smooth transitions from opening to endgame
    return evaluation;

}

CombinedScore evaluatePieceSquareScore(const ChessBoard& board) {

    CombinedScore cs = 0;
    for (Piece piece = WHITE_PAWN; piece <= BLACK_KING; piece++) {
        cs += evaluatePieceSquareScoreHelper(board, board.pieceCount[piece], piece);
    }
    return cs;
}

CombinedScore evaluatePieceSquareScoreHelper(const ChessBoard& board, int numberOfPieces, Piece piece) {
    
    CombinedScore cs = 0;
    for (int i = 0; i < numberOfPieces; i++) {
        cs += pieceSquareTable[piece][board.pieceSquare[piece][i]];
    }
    return cs;
}

void initEvaluation() {

    initPieceSquareTable();

}

void initPieceSquareTable() {

    const int PIECE_OFFSET = 6;
    int squareOffset, counter;
    for (Piece piece = WHITE_PAWN; piece <= WHITE_KING; piece++) {
        
        CombinedScore cs = makeScore(PIECE_VALUE[MIDDLEGAME][piece], PIECE_VALUE[ENDGAME][piece]);

        squareOffset = 56;
        counter = 0;
        for (Square sq = H1; sq <= A8; sq++) {
            Rank rank = rankOfSquareRANK(sq);
            File file = fileOfSquareFILE(sq);
            File filePieceSquareIndex = File(std::min((int)file, FILE_H - file));
            pieceSquareTable[piece][sq] = cs + ((piece == WHITE_PAWN) ? PAWN_SQUARE_BONUS[rank][file]
                                                                      : PIECE_SQUARE_BONUS[piece][rank][filePieceSquareIndex]);
            pieceSquareTable[piece + PIECE_OFFSET][sq + squareOffset] = -pieceSquareTable[piece][sq];

            if(++counter == 8) { counter = 0; squareOffset -= 16; }
        }
    }
}
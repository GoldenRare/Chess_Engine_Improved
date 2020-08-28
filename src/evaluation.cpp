
#include <algorithm>
#include "evaluation.hpp"
#include "utility.hpp"
#include "board.hpp"
#include "moves.hpp"

CombinedScore pieceSquareTable[PIECES][NUMBER_OF_SQUARES];

CombinedScore evaluatePieceSquareScore(const ChessBoard& board) {

    CombinedScore cs = 0;
    for (Piece piece = WHITE_PAWN; piece <= BLACK_KING; piece++) 
        cs += evaluatePieceSquareScoreHelper(board, board.pieceCount[piece], piece);
    return cs;
}

CombinedScore evaluatePieceSquareScoreHelper(const ChessBoard& board, int numberOfPieces, Piece piece) {
    
    CombinedScore cs = 0;
    for (int i = 0; i < numberOfPieces; i++) 
        cs += pieceSquareTable[piece][board.pieceSquare[piece][i]];

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

Evaluation::Evaluation(const ChessBoard& b) : board(b) {}

void Evaluation::initEvaluation() {

    ///////////////////////////////////////////////////////////
    Square whiteKingSq = board.pieceSquare[WHITE_KING][0];
    Square blackKingSq = board.pieceSquare[BLACK_KING][0];

    Bitboard whitePawns = board.pieces[WHITE_PAWN];
    Bitboard blackPawns = board.pieces[BLACK_PAWN];

    Bitboard whiteLowRanks = RANK_2_BB | RANK_3_BB; 
    Bitboard blackLowRanks = RANK_7_BB | RANK_6_BB;

    //Uses pawnsAbleToPush as a hack for finding blocked pawns
    Bitboard whiteBadPawns = whitePawns & (pawnsAbleToPush(whitePawns, board.gameBoard, WHITE) | whiteLowRanks);
    Bitboard blackBadPawns = blackPawns & (pawnsAbleToPush(blackPawns, board.gameBoard, BLACK) | blackLowRanks);

    Bitboard whiteKingBlockers = board.blockers(whiteKingSq, board.piecesOnSide[BLACK]);
    Bitboard blackKingBlockers = board.blockers(blackKingSq, board.piecesOnSide[WHITE]);

    Bitboard whitePawnAttacks = allPawnAttacks(whitePawns, WHITE);
    Bitboard blackPawnAttacks = allPawnAttacks(blackPawns, BLACK);

    Bitboard squaresAttackedTwiceByPawnsWHITE = attackedTwiceByPawns(whitePawns, WHITE);
    Bitboard squaresAttackedTwiceByPawnsBLACK = attackedTwiceByPawns(blackPawns, BLACK);
    ////////////////////////////////////////////////////////////

    //Initialize mobility area /////////////////////////////////
    mobilityArea[WHITE] = ~(whiteBadPawns | board.pieces[WHITE_KING] | board.pieces[WHITE_QUEEN] | whiteKingBlockers | blackPawnAttacks);
    mobilityArea[BLACK] = ~(blackBadPawns | board.pieces[BLACK_KING] | board.pieces[BLACK_QUEEN] | blackKingBlockers | whitePawnAttacks);
    ////////////////////////////////////////////////////////////

    //Initialize attacksBy for kings and pawns /////////////////
    attacksBy[WHITE][KING] = kingAttacks[board.pieceSquare[WHITE_KING][0]];
    attacksBy[BLACK][KING] = kingAttacks[board.pieceSquare[BLACK_KING][0]];

    attacksBy[WHITE][PAWN] = whitePawnAttacks;
    attacksBy[BLACK][PAWN] = blackPawnAttacks;

    attacksBy[WHITE][ALL_PIECE_TYPES] = attacksBy[WHITE][KING] | attacksBy[WHITE][PAWN];
    attacksBy[BLACK][ALL_PIECE_TYPES] = attacksBy[BLACK][KING] | attacksBy[BLACK][PAWN];

    attacksBy2[WHITE] = squaresAttackedTwiceByPawnsWHITE | (attacksBy[WHITE][KING] & attacksBy[WHITE][PAWN]);
    attacksBy2[BLACK] = squaresAttackedTwiceByPawnsBLACK | (attacksBy[BLACK][KING] & attacksBy[BLACK][PAWN]);
    //////////////////////////////////////////////////////////////

    //TODO//
    ////////
}

CombinedScore Evaluation::evaluatePiece(PieceType pt, Color c) {

    /////////////////////////////////////
    Square kingSq = board.pieceSquare[WHITE_KING + (6 * c)][0];

    Bitboard queens = board.pieces[WHITE_QUEEN] | board.pieces[BLACK_QUEEN];
    Bitboard kingBlockers = board.blockers(kingSq, board.piecesOnSide[~c]);
    /////////////////////////////////////

    int pieceIndex     = pt + (6 * c);
    int numberOfPieces = board.pieceCount[pieceIndex];

    Bitboard attacks;
    Square sq;
    CombinedScore cs = 0;

    for (int i = 0; i < numberOfPieces; i++) {

        sq = board.pieceSquare[pieceIndex][i];

        //Includes x-ray attacks for sliders
        attacks = (pt == KNIGHT) ? knightAttacks[sq] 
                : (pt == BISHOP) ? bishopAttacks(board.gameBoard ^ queens, sq)
                : (pt == ROOK  ) ? rookAttacks(board.gameBoard ^ queens ^ board.pieces[pieceIndex], sq)
                : bishopAttacks(board.gameBoard, sq) | rookAttacks(board.gameBoard, sq);

        // If piece is pinned to the king, then can only move along the ray of the incoming attack
        if (kingBlockers & sq)
            attacks &= rayLine[kingSq][sq];

        //TODO//
        cs += MOBILITY_BONUS[pt][populationCount(attacks & mobilityArea[c])];
        ////////


    }

    return cs;
}

ExactScore Evaluation::evaluatePosition() {

    ExactScore evaluation;
    CombinedScore cs = 0;

    initEvaluation();

    // Evaluation of the pieces (excluding King and Pawns)
    cs += evaluatePiece(KNIGHT, WHITE) - evaluatePiece(KNIGHT, BLACK);
    cs += evaluatePiece(BISHOP, WHITE) - evaluatePiece(BISHOP, BLACK);
    cs += evaluatePiece(ROOK  , WHITE) - evaluatePiece(ROOK  , BLACK);
    cs += evaluatePiece(QUEEN , WHITE) - evaluatePiece(QUEEN , BLACK);

    cs += evaluatePieceSquareScore(board);
    cs = (board.sideToPlay == WHITE) ? cs : -cs; //Score relative to the side to play

    int phase = gamePhase();
    evaluation = ((middlegameScore(cs) * (256 - phase)) + (endgameScore(cs) * phase)) / 256; //Formula to create smooth transitions from opening to endgame
    return evaluation;

}

int Evaluation::gamePhase() {
    
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
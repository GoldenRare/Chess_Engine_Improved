#include <iostream>
#include <cstdlib>
#include <algorithm>
#include "board.hpp"
#include "utility.hpp"
#include "moves.hpp"
#include "zobrist.hpp"
#include "evaluation.hpp"

int ChebyshevDistance[SQUARES][SQUARES];

ChessBoard::ChessBoard() : ChessBoard("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1") {}
ChessBoard::ChessBoard(std::string fen) {
    parseFEN(fen);
}

// Works by putting a super-piece on the sq, and seeing if the correct opposite color pieces resides on
// the attack squares of the super-piece
bool ChessBoard::isSquareAttacked(Square sq, Color attacked) const {

    Color attacker = ~attacked;
    if (pawnAttacks[attacked][sq] & pieces[pieceTypeToPiece(PAWN  , attacker)]) return true;
    if (knightAttacks        [sq] & pieces[pieceTypeToPiece(KNIGHT, attacker)]) return true;
    if (kingAttacks          [sq] & pieces[pieceTypeToPiece(KING  , attacker)]) return true;

    if (bishopAttacks(occupiedSquares, sq) & (pieces[pieceTypeToPiece(BISHOP, attacker)] | pieces[pieceTypeToPiece(QUEEN, attacker)])) return true;
    if (rookAttacks  (occupiedSquares, sq) & (pieces[pieceTypeToPiece(ROOK  , attacker)] | pieces[pieceTypeToPiece(QUEEN, attacker)])) return true;

    return false;
}

Bitboard ChessBoard::attackersToSquare(Square sq, Bitboard occupied) {

    return (pawnAttacks[WHITE][sq] &  pieces[BLACK_PAWN  ])
         | (pawnAttacks[BLACK][sq] &  pieces[WHITE_PAWN  ])
         | (knightAttacks     [sq] & (pieces[WHITE_KNIGHT] | pieces[BLACK_KNIGHT])) 
         | (kingAttacks       [sq] & (pieces[WHITE_KING  ] | pieces[BLACK_KING  ])) 
         | (bishopAttacks(occupied, sq) & (pieces[WHITE_BISHOP] | pieces[BLACK_BISHOP] | pieces[WHITE_QUEEN] | pieces[BLACK_QUEEN]))
         | (rookAttacks  (occupied, sq) & (pieces[WHITE_ROOK  ] | pieces[BLACK_ROOK  ] | pieces[WHITE_QUEEN] | pieces[BLACK_QUEEN]));

}

void ChessBoard::makeMove(const Move& move) {

    //Saves needed information in order to undo a move
    GameState state;
    state.move = move;
    state.capturedPiece = NO_PIECE;
    state.enPassantSquare = enPassant;
    state.castlingRights = castlingRights;
    state.halfmoves = halfmoves;
    state.key = positionKey;
    //////////////////////////////////////////////////

    Direction pawnPush;
    Piece fromPiece;
    Bitboard fromBB, toBB, fromToBB;
    int fromSquare, toSquare;

    pawnPush   = (sideToPlay == WHITE) ? NORTH : SOUTH;
    fromSquare = getFrom(move);
    toSquare   = getTo(move);
    fromBB     = squareToBitboard(static_cast<Square>(fromSquare));
    toBB       = squareToBitboard(static_cast<Square>(toSquare));
    fromToBB   = fromBB ^ toBB;
    fromPiece  = pieceBoard[fromSquare];

    MoveType moveType = typeOfMove(move);

    enPassant = (moveType == DOUBLE_PAWN_PUSH) ? static_cast<Square>(fromSquare + pawnPush) : NO_SQUARE; //Updates en passant square
    if (state.enPassantSquare != enPassant) { // If enPassant changes, update positionKey
        File oldSq = (state.enPassantSquare == NO_SQUARE) ? FILES : fileOfSquareFILE(state.enPassantSquare);
        File newSq = (enPassant             == NO_SQUARE) ? FILES : fileOfSquareFILE(enPassant)            ;
        positionKey ^= zobristEnPassant[oldSq] ^ zobristEnPassant[newSq];
    }

    if ((moveType == QUIET) || (moveType == DOUBLE_PAWN_PUSH) 
        || (moveType == KINGSIDE_CASTLE) || (moveType == QUEENSIDE_CASTLE)) {
        pieces[fromPiece] ^= fromToBB;
        piecesOnSide[sideToPlay] ^= fromToBB;
        occupiedSquares ^= fromToBB;
        emptySquares ^= fromToBB;
        pieceSquare[fromPiece][findPieceSquareIndex(fromPiece, static_cast<Square>(fromSquare))] = static_cast<Square>(toSquare);
        pieceBoard[toSquare] = fromPiece;
        pieceSquareScore += pieceSquareTable[fromPiece][toSquare] - pieceSquareTable[fromPiece][fromSquare];
        positionKey ^= zobristPieceSquare[fromPiece][fromSquare] ^ zobristPieceSquare[fromPiece][toSquare];

        if ((fromPiece == WHITE_PAWN) || (fromPiece == BLACK_PAWN)) halfmoves = 0;
        else halfmoves++;        

        Piece fromRook;
        Bitboard fromRookBB, toRookBB, fromToRookBB;
        int fromRookSquare, toRookSquare;
        if(moveType == KINGSIDE_CASTLE) {

            fromRookSquare = (sideToPlay == WHITE) ? H1 : H8;
            toRookSquare = (sideToPlay == WHITE) ? F1 : F8;
            fromRookBB = 1ULL << fromRookSquare;
            toRookBB   = 1ULL << toRookSquare;
            fromToRookBB = fromRookBB ^ toRookBB;
            fromRook = pieceBoard[fromRookSquare];

            pieces[fromRook] ^= fromToRookBB;
            piecesOnSide[sideToPlay] ^= fromToRookBB;
            occupiedSquares ^= fromToRookBB;
            emptySquares ^= fromToRookBB;
            pieceSquare[fromRook][findPieceSquareIndex(fromRook, static_cast<Square>(fromRookSquare))] = static_cast<Square>(toRookSquare);
            pieceBoard[toRookSquare] = fromRook;
            pieceSquareScore += pieceSquareTable[fromRook][toRookSquare] - pieceSquareTable[fromRook][fromRookSquare];
            positionKey ^= zobristPieceSquare[fromRook][fromRookSquare] ^ zobristPieceSquare[fromRook][toRookSquare];
            

        } else if (moveType == QUEENSIDE_CASTLE) {

            fromRookSquare = (sideToPlay == WHITE) ? A1 : A8;
            toRookSquare = (sideToPlay == WHITE) ? D1 : D8;
            fromRookBB = 1ULL << fromRookSquare;
            toRookBB   = 1ULL << toRookSquare;
            fromToRookBB = fromRookBB ^ toRookBB;
            fromRook = pieceBoard[fromRookSquare];

            pieces[fromRook] ^= fromToRookBB;
            piecesOnSide[sideToPlay] ^= fromToRookBB;
            occupiedSquares ^= fromToRookBB;
            emptySquares ^= fromToRookBB;
            pieceSquare[fromRook][findPieceSquareIndex(fromRook, static_cast<Square>(fromRookSquare))] = static_cast<Square>(toRookSquare);
            pieceBoard[toRookSquare] = fromRook;
            pieceSquareScore += pieceSquareTable[fromRook][toRookSquare] - pieceSquareTable[fromRook][fromRookSquare];
            positionKey ^= zobristPieceSquare[fromRook][fromRookSquare] ^ zobristPieceSquare[fromRook][toRookSquare];

        }
    } else if (moveType == CAPTURE) {
        Piece toPiece;

        toPiece = pieceBoard[toSquare];
        state.capturedPiece = toPiece;

        pieces[fromPiece] ^= fromToBB;
        piecesOnSide[sideToPlay] ^= fromToBB;

        pieces[toPiece] ^= toBB; //
        piecesOnSide[~sideToPlay] ^= toBB; //

        occupiedSquares ^= fromBB;
        emptySquares ^= fromBB;

        pieceSquare[fromPiece][findPieceSquareIndex(fromPiece, static_cast<Square>(fromSquare))] = static_cast<Square>(toSquare);
        //Delete captured piece
        pieceSquare[toPiece][findPieceSquareIndex(toPiece, static_cast<Square>(toSquare))] = pieceSquare[toPiece][pieceCount[toPiece] - 1];
        pieceCount[toPiece]--;
        pieceBoard[toSquare] = fromPiece;
        pieceSquareScore += pieceSquareTable[fromPiece][toSquare] - pieceSquareTable[fromPiece][fromSquare] - pieceSquareTable[toPiece][toSquare];
        positionKey ^= zobristPieceSquare[fromPiece][fromSquare] ^ zobristPieceSquare[fromPiece][toSquare] ^ zobristPieceSquare[toPiece][toSquare];
        halfmoves = 0;

        if ((toPiece > WHITE_PAWN && toPiece < WHITE_KING) || (toPiece > BLACK_PAWN && toPiece < BLACK_KING))
            nonPawnMaterial[~sideToPlay] -= PIECE_VALUE[MIDDLEGAME][toPiece]; 

    } else if (moveType == EN_PASSANT_CAPTURE) {

        Piece toPiece;
        Bitboard enPassantPawn = squareToBitboard(static_cast<Square>(toSquare - pawnPush));

        toPiece = pieceBoard[toSquare - pawnPush];
        state.capturedPiece = toPiece;

        pieces[fromPiece] ^= fromToBB;
        piecesOnSide[sideToPlay] ^= fromToBB;

        pieces[toPiece] ^= enPassantPawn; //
        piecesOnSide[~sideToPlay] ^= enPassantPawn; //

        occupiedSquares ^= fromToBB;
        emptySquares ^= fromToBB;

        occupiedSquares ^= enPassantPawn;//
        emptySquares ^= enPassantPawn;//

        pieceSquare[fromPiece][findPieceSquareIndex(fromPiece, static_cast<Square>(fromSquare))] = static_cast<Square>(toSquare);
        //Delete captured piece
        pieceSquare[toPiece][findPieceSquareIndex(toPiece, static_cast<Square>(toSquare - pawnPush))] = pieceSquare[toPiece][pieceCount[toPiece] - 1];
        pieceCount[toPiece]--;
        pieceBoard[toSquare] = fromPiece;
        pieceSquareScore += pieceSquareTable[fromPiece][toSquare] - pieceSquareTable[fromPiece][fromSquare] - pieceSquareTable[toPiece][toSquare - pawnPush];
        positionKey ^= zobristPieceSquare[fromPiece][fromSquare] ^ zobristPieceSquare[fromPiece][toSquare] ^ zobristPieceSquare[toPiece][toSquare - pawnPush];
        halfmoves = 0; 

    } else if ((moveType == KNIGHT_PROMOTION) || (moveType == BISHOP_PROMOTION) 
        || (moveType == ROOK_PROMOTION) || (moveType == QUEEN_PROMOTION)) {

        Piece promotionPiece;

        promotionPiece = (moveType == KNIGHT_PROMOTION) ? WHITE_KNIGHT :
                         (moveType == BISHOP_PROMOTION) ? WHITE_BISHOP :
                         (moveType == ROOK_PROMOTION  ) ? WHITE_ROOK   :
                                                          WHITE_QUEEN  ;
        promotionPiece = (sideToPlay == WHITE) ? promotionPiece : promotionPiece + 6;

        state.capturedPiece = fromPiece;

        pieces[fromPiece] ^= fromBB;
        piecesOnSide[sideToPlay] ^= fromBB;

        pieces[promotionPiece] ^= toBB; 
        piecesOnSide[sideToPlay] ^= toBB;

        occupiedSquares ^= fromToBB;
        emptySquares ^= fromToBB;

        //Delete the pawn
        pieceSquare[fromPiece][findPieceSquareIndex(fromPiece, static_cast<Square>(fromSquare))] = pieceSquare[fromPiece][pieceCount[fromPiece] - 1];
        pieceCount[fromPiece]--;
        
        //Add in promotion piece
        pieceCount[promotionPiece]++;
        pieceSquare[promotionPiece][pieceCount[promotionPiece] - 1] = static_cast<Square>(toSquare); 
        
        pieceBoard[toSquare] = promotionPiece;
        nonPawnMaterial[sideToPlay] += PIECE_VALUE[MIDDLEGAME][promotionPiece];
        pieceSquareScore += pieceSquareTable[promotionPiece][toSquare] - pieceSquareTable[fromPiece][fromSquare];
        positionKey ^= zobristPieceSquare[fromPiece][fromSquare] ^ zobristPieceSquare[promotionPiece][toSquare];
        halfmoves = 0;

    } else if ((moveType == KNIGHT_PROMOTION_CAPTURE) || (moveType == BISHOP_PROMOTION_CAPTURE) 
        || (moveType == ROOK_PROMOTION_CAPTURE) || (moveType == QUEEN_PROMOTION_CAPTURE)) {

        Piece promotionPiece, toPiece;

        promotionPiece = (moveType == KNIGHT_PROMOTION_CAPTURE) ? WHITE_KNIGHT :
                         (moveType == BISHOP_PROMOTION_CAPTURE) ? WHITE_BISHOP :
                         (moveType == ROOK_PROMOTION_CAPTURE  ) ? WHITE_ROOK   :
                                                                  WHITE_QUEEN  ;
        promotionPiece = (sideToPlay == WHITE) ? promotionPiece : promotionPiece + 6;

        toPiece = pieceBoard[toSquare];
        state.capturedPiece = toPiece;

        //state.capturedPiece = fromPiece; -- Not storing our pawn that is leaving

        pieces[fromPiece] ^= fromBB;
        piecesOnSide[sideToPlay] ^= fromBB;

        pieces[toPiece] ^= toBB;
        piecesOnSide[~sideToPlay] ^= toBB;

        pieces[promotionPiece] ^= toBB; 
        piecesOnSide[sideToPlay] ^= toBB;

        occupiedSquares ^= fromBB;
        emptySquares ^= fromBB;

        //Delete the pawn
        pieceSquare[fromPiece][findPieceSquareIndex(fromPiece, static_cast<Square>(fromSquare))] = pieceSquare[fromPiece][pieceCount[fromPiece] - 1];
        pieceCount[fromPiece]--;
        
        //Add in promotion piece
        pieceCount[promotionPiece]++;
        pieceSquare[promotionPiece][pieceCount[promotionPiece] - 1] = static_cast<Square>(toSquare);

        //Delete enemy piece
        pieceSquare[toPiece][findPieceSquareIndex(toPiece, static_cast<Square>(toSquare))] = pieceSquare[toPiece][pieceCount[toPiece] - 1];
        pieceCount[toPiece]--; 
        
        pieceBoard[toSquare] = promotionPiece;
        pieceSquareScore += pieceSquareTable[promotionPiece][toSquare] - pieceSquareTable[fromPiece][fromSquare] - pieceSquareTable[toPiece][toSquare];
        positionKey ^= zobristPieceSquare[fromPiece][fromSquare] ^ zobristPieceSquare[toPiece][toSquare] ^ zobristPieceSquare[promotionPiece][toSquare];
        halfmoves = 0;

        nonPawnMaterial[sideToPlay] += PIECE_VALUE[MIDDLEGAME][promotionPiece];
        nonPawnMaterial[~sideToPlay] -= PIECE_VALUE[MIDDLEGAME][toPiece];       

    }

    positionKey ^= zobristBlackToMove;
    disableCastlingRights(fromPiece, static_cast<Square>(fromSquare), state.capturedPiece, static_cast<Square>(toSquare)); //Updates castling rights
    if (state.castlingRights != castlingRights)  //If the castling rights have changed, update position key
        positionKey ^= zobristCastlingRights[state.castlingRights] ^ zobristCastlingRights[castlingRights];

    sideToPlay = ~sideToPlay;
    previousGameStates[previousGameStatesCount++] = state;
    ply++; 

}

void ChessBoard::undoMove() {

    ply--; 
    GameState state = previousGameStates[--previousGameStatesCount];

    positionKey ^= zobristBlackToMove;
    if (state.castlingRights != castlingRights)  //If the castling rights have changed, update position key
        positionKey ^= zobristCastlingRights[state.castlingRights] ^ zobristCastlingRights[castlingRights];

    if (state.enPassantSquare != enPassant) { // If enPassant changes, update positionKey
        File newSq = (state.enPassantSquare == NO_SQUARE) ? FILES : fileOfSquareFILE(state.enPassantSquare);
        File oldSq = (enPassant             == NO_SQUARE) ? FILES : fileOfSquareFILE(enPassant)            ;
        positionKey ^= zobristEnPassant[oldSq] ^ zobristEnPassant[newSq];
    }

    Move move = state.move;
    enPassant = state.enPassantSquare;
    castlingRights = state.castlingRights;
    halfmoves = state.halfmoves;
    sideToPlay = ~sideToPlay;

    //Restore enPassant square, castlingRights
    Piece toPiece;
    Bitboard toBB, fromBB, fromToBB;
    int toSquare, fromSquare;

    toSquare   = getTo(move);
    fromSquare = getFrom(move);
    toBB       = (1ULL) << toSquare;
    fromBB     = (1ULL) << fromSquare;
    fromToBB   = toBB ^ fromBB;
    toPiece    = pieceBoard[toSquare];

    MoveType moveType = typeOfMove(move);
    if ((moveType == QUIET) || (moveType == DOUBLE_PAWN_PUSH) 
        || (moveType == KINGSIDE_CASTLE) || (moveType == QUEENSIDE_CASTLE)) {
        pieces[toPiece] ^= fromToBB;
        piecesOnSide[sideToPlay] ^= fromToBB;
        occupiedSquares ^= fromToBB;
        emptySquares ^= fromToBB;
        pieceSquare[toPiece][findPieceSquareIndex(toPiece, static_cast<Square>(toSquare))] = static_cast<Square>(fromSquare);
        pieceBoard[fromSquare] = toPiece;
        pieceSquareScore += pieceSquareTable[toPiece][fromSquare] - pieceSquareTable[toPiece][toSquare];
        positionKey ^= zobristPieceSquare[toPiece][fromSquare] ^ zobristPieceSquare[toPiece][toSquare]; 

        Piece toRook;
        Bitboard toRookBB, fromRookBB, fromToRookBB;
        int toRookSquare, fromRookSquare;
        if(moveType == KINGSIDE_CASTLE) {

            toRookSquare   = (sideToPlay == WHITE) ? F1 : F8;
            fromRookSquare = (sideToPlay == WHITE) ? H1 : H8;
            toRookBB = 1ULL << toRookSquare;
            fromRookBB   = 1ULL << fromRookSquare;
            fromToRookBB = toRookBB ^ fromRookBB;
            toRook = pieceBoard[toRookSquare];

            pieces[toRook] ^= fromToRookBB;
            piecesOnSide[sideToPlay] ^= fromToRookBB;
            occupiedSquares ^= fromToRookBB;
            emptySquares ^= fromToRookBB;
            pieceSquare[toRook][findPieceSquareIndex(toRook, static_cast<Square>(toRookSquare))] = static_cast<Square>(fromRookSquare);
            pieceBoard[fromRookSquare] = toRook;
            pieceSquareScore += pieceSquareTable[toRook][fromRookSquare] - pieceSquareTable[toRook][toRookSquare];
            positionKey ^= zobristPieceSquare[toRook][fromRookSquare] ^ zobristPieceSquare[toRook][toRookSquare];

        } else if (moveType == QUEENSIDE_CASTLE) {

            toRookSquare = (sideToPlay == WHITE) ? D1 : D8;
            fromRookSquare = (sideToPlay == WHITE) ? A1 : A8;
            toRookBB = 1ULL << toRookSquare;
            fromRookBB   = 1ULL << fromRookSquare;
            fromToRookBB = toRookBB ^ fromRookBB;
            toRook = pieceBoard[toRookSquare];

            pieces[toRook] ^= fromToRookBB;
            piecesOnSide[sideToPlay] ^= fromToRookBB;
            occupiedSquares ^= fromToRookBB;
            emptySquares ^= fromToRookBB;
            pieceSquare[toRook][findPieceSquareIndex(toRook, static_cast<Square>(toRookSquare))] = static_cast<Square>(fromRookSquare);
            pieceBoard[fromRookSquare] = toRook;
            pieceSquareScore += pieceSquareTable[toRook][fromRookSquare] - pieceSquareTable[toRook][toRookSquare];
            positionKey ^= zobristPieceSquare[toRook][fromRookSquare] ^ zobristPieceSquare[toRook][toRookSquare];

        }
    } else if (moveType == CAPTURE) {
        Piece capturedPiece;

        capturedPiece = state.capturedPiece;

        pieces[toPiece] ^= fromToBB;
        piecesOnSide[sideToPlay] ^= fromToBB;

        pieces[capturedPiece] ^= toBB; //
        piecesOnSide[~sideToPlay] ^= toBB; //

        occupiedSquares ^= fromBB;
        emptySquares ^= fromBB;

        pieceSquare[toPiece][findPieceSquareIndex(toPiece, static_cast<Square>(toSquare))] = static_cast<Square>(fromSquare);
        //Add captured piece
        pieceCount[capturedPiece]++;
        pieceSquare[capturedPiece][pieceCount[capturedPiece] - 1] = static_cast<Square>(toSquare);
        
        pieceBoard[fromSquare] = toPiece;
        pieceBoard[toSquare] = capturedPiece;
        pieceSquareScore += pieceSquareTable[toPiece][fromSquare] - pieceSquareTable[toPiece][toSquare] + pieceSquareTable[capturedPiece][toSquare];
        positionKey ^= zobristPieceSquare[toPiece][fromSquare] ^ zobristPieceSquare[toPiece][toSquare] ^ zobristPieceSquare[capturedPiece][toSquare];

        if ((capturedPiece > WHITE_PAWN && capturedPiece < WHITE_KING) || (capturedPiece > BLACK_PAWN && capturedPiece < BLACK_KING))
            nonPawnMaterial[~sideToPlay] += PIECE_VALUE[MIDDLEGAME][capturedPiece]; 

    } else if (moveType == EN_PASSANT_CAPTURE) {
        Piece capturedPiece;
        Direction pawnPush = (sideToPlay == WHITE) ? NORTH : SOUTH;

        capturedPiece = state.capturedPiece;

        pieces[toPiece] ^= fromToBB;
        piecesOnSide[sideToPlay] ^= fromToBB;

        pieces[capturedPiece] ^= 1ULL << (toSquare - pawnPush); //
        piecesOnSide[~sideToPlay] ^= 1ULL << (toSquare - pawnPush); //

        occupiedSquares ^= fromToBB;
        emptySquares ^= fromToBB;

        occupiedSquares ^= 1ULL << (toSquare - pawnPush);//
        emptySquares ^= 1ULL << (toSquare - pawnPush);//

        pieceSquare[toPiece][findPieceSquareIndex(toPiece, static_cast<Square>(toSquare))] = static_cast<Square>(fromSquare);
        //Add captured piece
        pieceCount[capturedPiece]++;
        pieceSquare[capturedPiece][pieceCount[capturedPiece] - 1] = static_cast<Square>(toSquare - pawnPush);
        
        pieceBoard[fromSquare] = toPiece;
        pieceBoard[toSquare - pawnPush] = capturedPiece;
        pieceSquareScore += pieceSquareTable[toPiece][fromSquare] - pieceSquareTable[toPiece][toSquare] + pieceSquareTable[capturedPiece][toSquare - pawnPush];
        positionKey ^= zobristPieceSquare[toPiece][fromSquare] ^ zobristPieceSquare[toPiece][toSquare] ^ zobristPieceSquare[capturedPiece][toSquare - pawnPush];

    } else if ((moveType == KNIGHT_PROMOTION) || (moveType == BISHOP_PROMOTION) 
        || (moveType == ROOK_PROMOTION) || (moveType == QUEEN_PROMOTION)) {

        Piece fromPiece;

        fromPiece = state.capturedPiece;

        pieces[fromPiece] ^= fromBB;
        piecesOnSide[sideToPlay] ^= fromBB;

        pieces[toPiece] ^= toBB; 
        piecesOnSide[sideToPlay] ^= toBB;

        occupiedSquares ^= fromToBB;
        emptySquares ^= fromToBB;

        //Add the pawn
        pieceCount[fromPiece]++;
        pieceSquare[fromPiece][pieceCount[fromPiece] - 1] = static_cast<Square>(fromSquare);
        
        //Delete promotion piece
        pieceSquare[toPiece][findPieceSquareIndex(toPiece, static_cast<Square>(toSquare))] = pieceSquare[toPiece][pieceCount[toPiece] - 1];
        pieceCount[toPiece]--; 
        
        pieceBoard[fromSquare] = fromPiece;
        nonPawnMaterial[sideToPlay] -= PIECE_VALUE[MIDDLEGAME][toPiece];
        pieceSquareScore += pieceSquareTable[fromPiece][fromSquare] - pieceSquareTable[toPiece][toSquare];
        positionKey ^= zobristPieceSquare[fromPiece][fromSquare] ^ zobristPieceSquare[toPiece][toSquare];

    } else if ((moveType == KNIGHT_PROMOTION_CAPTURE) || (moveType == BISHOP_PROMOTION_CAPTURE) 
        || (moveType == ROOK_PROMOTION_CAPTURE) || (moveType == QUEEN_PROMOTION_CAPTURE)) {

        Piece capturedPiece, fromPiece;

        capturedPiece = state.capturedPiece;
        fromPiece     = (sideToPlay == WHITE) ? WHITE_PAWN : BLACK_PAWN;

        pieces[fromPiece] ^= fromBB;
        piecesOnSide[sideToPlay] ^= fromBB;

        pieces[toPiece] ^= toBB;
        piecesOnSide[sideToPlay] ^= toBB;

        pieces[capturedPiece] ^= toBB; 
        piecesOnSide[~sideToPlay] ^= toBB;

        occupiedSquares ^= fromBB;
        emptySquares ^= fromBB;

        //Add the pawn
        pieceCount[fromPiece]++;
        pieceSquare[fromPiece][pieceCount[fromPiece] - 1] = static_cast<Square>(fromSquare);

        //Delete promotion piece
        pieceSquare[toPiece][findPieceSquareIndex(toPiece, static_cast<Square>(toSquare))] = pieceSquare[toPiece][pieceCount[toPiece] - 1];
        pieceCount[toPiece]--;

        //Add captured piece
        pieceCount[capturedPiece]++;
        pieceSquare[capturedPiece][pieceCount[capturedPiece] - 1] = static_cast<Square>(toSquare);
        
        pieceBoard[toSquare]   = capturedPiece;
        pieceBoard[fromSquare] = fromPiece;
        pieceSquareScore += pieceSquareTable[fromPiece][fromSquare] - pieceSquareTable[toPiece][toSquare] + pieceSquareTable[capturedPiece][toSquare];
        positionKey ^= zobristPieceSquare[fromPiece][fromSquare] ^ zobristPieceSquare[toPiece][toSquare] ^ zobristPieceSquare[capturedPiece][toSquare];

        nonPawnMaterial[sideToPlay] -= PIECE_VALUE[MIDDLEGAME][toPiece];
        nonPawnMaterial[~sideToPlay] += PIECE_VALUE[MIDDLEGAME][capturedPiece];

    }
}

Bitboard ChessBoard::blockers(Square sq, Bitboard sliders, Bitboard& pinners) const{

    Bitboard blockers = 0;
    pinners = 0;

    Bitboard bishops  = pieces[WHITE_BISHOP] | pieces[BLACK_BISHOP];
    Bitboard rooks    = pieces[WHITE_ROOK  ] | pieces[BLACK_ROOK  ];
    Bitboard queens   = pieces[WHITE_QUEEN ] | pieces[BLACK_QUEEN ];

    Bitboard occupied;
    Bitboard occupiedInBetween;
    Square sliderSq;

    sliders  = ((bishopAttacks(0, sq) & (bishops | queens)) | (rookAttacks(0, sq) & (rooks | queens))) & sliders;
    occupied = occupiedSquares ^ sliders;

    while (sliders) {

        sliderSq = squareOfLS1B(&sliders);
        occupiedInBetween = inBetween[sq][sliderSq] & occupied;

        // sq is blocked only if there is exactly 1 piece in between the attacking sliding piece and the sq
        if (occupiedInBetween && !hasMoreThanOneBit(occupiedInBetween)) {
            
            blockers |= occupiedInBetween;
            if (occupiedInBetween & piecesOnSide[colorOfPiece(pieceBoard[sq])]) 
                pinners |= squareToBitboard(sliderSq);
        }
    }

    return blockers;
}

bool ChessBoard::givesCheck(const Move& move) {

    bool result = false;
    makeMove(move);
    if (isSquareAttacked(pieceSquare[sideToPlay == WHITE ? WHITE_KING : BLACK_KING][0], sideToPlay))
        result = true;
    undoMove();
    return result;
}

bool ChessBoard::advancedPawnPush(const Move& move) {
    Piece pawn = sideToPlay == WHITE ? WHITE_PAWN : BLACK_PAWN;
    if (typeOfMove(move) == EN_PASSANT_CAPTURE) return false;
    return pieceBoard[getFrom(move)] == pawn && relativeRank(sideToPlay, Square(getTo(move))) > RANK_5;
}

int ChessBoard::endgameValueOfPiece(const Move& move) {
    if (isSEECapture(move)) return PIECE_VALUE[ENDGAME][pieceBoard[getTo(move)]];
    else if (typeOfMove(move) == EN_PASSANT_CAPTURE) return PIECE_VALUE[ENDGAME][WHITE_PAWN];
    return 0;
}

void ChessBoard::parseFEN(std::string fen) {

    clearBoard();

    int i = 0;
    for (Bitboard init = squareToBitboard(A8); init > 0; init >>= 1) {
        piecePlacement(&init, fen[i]);
        i++;
    }
            
    for (int j = WHITE_PAWN; j <= WHITE_KING; j++) {

        piecesOnSide[WHITE] |= pieces[j];

    }

    for (int j = BLACK_PAWN; j <= BLACK_KING; j++) {

        piecesOnSide[BLACK] |= pieces[j];

    }

    occupiedSquares    = piecesOnSide[WHITE] | piecesOnSide[BLACK];
    emptySquares       = ~occupiedSquares;

    i++;
    sideToPlay = (fen[i] == 'w') ? WHITE : BLACK;
    if (sideToPlay == BLACK) positionKey ^= zobristBlackToMove;
    
    i += 2;
    while(fen[i] !=  ' ') {
        parseCastlingRights(fen[i]);
        i++;
    }

    positionKey ^= zobristCastlingRights[castlingRights];

    //TODO: Finish the rest of the parsing
}

void ChessBoard::piecePlacement(Bitboard* init, char piece) {

    switch(piece) {

        case 'r':

            makePiece(init, BLACK_ROOK);
            break;

        case 'n':

            makePiece(init, BLACK_KNIGHT);
            break;

        case 'b':

            makePiece(init, BLACK_BISHOP);
            break;

        case 'q':

            makePiece(init, BLACK_QUEEN);
            break;

        case 'k':

            makePiece(init, BLACK_KING);
            break;

        case 'p':

            makePiece(init, BLACK_PAWN);
            break;

        case 'R':

            makePiece(init, WHITE_ROOK);
            break;

        case 'N':

            makePiece(init, WHITE_KNIGHT);
            break;

        case 'B':

            makePiece(init, WHITE_BISHOP);
            break;

        case 'Q':

            makePiece(init, WHITE_QUEEN);
            break;

        case 'K':

            makePiece(init, WHITE_KING);
            break;

        case 'P':

            makePiece(init, WHITE_PAWN);
            break;

    }

    if (piece == '/') *init <<= 1;
    else if ((piece >= '1') && (piece <= '8')) *init >>= (piece - '0' - 1);
}

void ChessBoard::parseCastlingRights(char c) {

    switch(c) {

        case 'K':

           castlingRights |= (0b1);
           break;

        case 'Q':

            castlingRights |= (0b1 << 1);
            break;

        case 'k':

           castlingRights |= (0b1 << 2);
           break;

        case 'q':

            castlingRights |= (0b1 << 3);
            break;

        case '-':

            break;

        default :

             std::cout << "Invalid Character!" << std::endl;
    }
}

void ChessBoard::makePiece(Bitboard* init, Piece p) {

    Color c = ((p >= WHITE_PAWN) && (p <= WHITE_KING)) ? WHITE : BLACK;
    Square sq = squareOfLS1B(*init);
    pieces[p] |= *init;
    pieceSquare[p][pieceCount[p]++] = sq;
    pieceBoard[sq] = p;
    pieceSquareScore += pieceSquareTable[p][sq];
    positionKey ^= zobristPieceSquare[p][sq];

    if ((p > WHITE_PAWN && p < WHITE_KING) || (p > BLACK_PAWN && p < BLACK_KING))
        nonPawnMaterial[c] +=  PIECE_VALUE[MIDDLEGAME][p];

}

void ChessBoard::disableCastlingRights(Piece fromPiece, Square fromSquare, Piece capturedPiece, Square toSquare) {
    Square kingSide, queenSide, enemyKingSide, enemyQueenSide;
    kingSide       = (sideToPlay == WHITE) ? H1 : H8;
    queenSide      = (sideToPlay == WHITE) ? A1 : A8;
    enemyKingSide  = (sideToPlay == WHITE) ? H8 : H1;
    enemyQueenSide = (sideToPlay == WHITE) ? A8 : A1;

    //Once king moves, cannot castle
    if ((fromPiece == WHITE_KING) || (fromPiece == BLACK_KING)) 
        castlingRights = (sideToPlay == WHITE) ? (castlingRights & 0b1100) : (castlingRights & 0b0011);
    
    //Disable castling rights for a given side if the rook in the corner has moved ...
    if ((fromPiece == WHITE_ROOK) || (fromPiece == BLACK_ROOK)) {

        if (kingSide == fromSquare) 
            castlingRights = (sideToPlay == WHITE) ? (castlingRights & 0b1110) : (castlingRights & 0b1011);

        else if (queenSide == fromSquare)
            castlingRights = (sideToPlay == WHITE) ? (castlingRights & 0b1101) : (castlingRights & 0b0111);

    } 
    
    //... or if captured
    if ((capturedPiece == WHITE_ROOK) || (capturedPiece == BLACK_ROOK)) {

        if (enemyKingSide == toSquare) 
            castlingRights = (sideToPlay == WHITE) ? (castlingRights & 0b1011) : (castlingRights & 0b1110);

        else if (enemyQueenSide == toSquare)
            castlingRights = (sideToPlay == WHITE) ? (castlingRights & 0b0111) : (castlingRights & 0b1101);
    }
}

int ChessBoard::findPieceSquareIndex(Piece p, Square sq) {

    int numberOfPieces = pieceCount[p];
    for(int i = 0; i < numberOfPieces; i++) {

        if(pieceSquare[p][i] == sq) return i;
    }

    return -1;
}

void ChessBoard::clearBoard() {

    positionKey = 0;
    std::fill(pieceBoard, pieceBoard + SQUARES, NO_PIECE);
    std::fill(pieces, pieces + PIECES, 0);
    std::fill(piecesOnSide, piecesOnSide + COLOURS, 0);
    occupiedSquares = 0; 
    emptySquares = 0;
    std::fill(pieceCount, pieceCount + PIECES, 0);
    std::fill(pieceSquare[0], pieceSquare[0] + (PIECES * 10), NO_SQUARE);
    sideToPlay = WHITE;
    enPassant = NO_SQUARE;
    castlingRights = 0;
    std::fill(nonPawnMaterial, nonPawnMaterial + COLOURS, 0);
    pieceSquareScore = 0;
    ply = 0; 
    halfmoves = 0;
    //std::fill(previousGameStates, previousGameStates + 256, 0);
    previousGameStatesCount = 0;
}

void printBitboard(Bitboard b) {

    std::string border = "|---|---|---|---|---|---|---|---|";
    std::string files  = "  A   B   C   D   E   F   G   H";
    Bitboard mask = 0b1;
    int bitIndex = 63;

    mask <<= bitIndex;

    std::cout << border << std::endl;
    for (int i = RANK_8; i >= RANK_1; i--) {
        for (int j = FILE_A; j <= FILE_H; j++) {

            int piece = (b & mask) >> bitIndex;
            bitIndex--;
            mask >>= 1;

            if (j == FILE_H) {

                std::cout << "| " << piece << " |" << std::endl;

            } else {

                std::cout << "| " << piece << " ";

            }

        }
        std::cout << border << std::endl;
    }
    std::cout << files << std::endl;
}

bool isValidSquare(Square s) {

    bool result = false;
    if ((s >= H1) && (s <= A8)){
        result = true;
    }

    return result;
}

void initBoard() {

    initChebyshevDistance();

}

void initChebyshevDistance() {

    for (int i = H1; i <= A8; i++) {
        for(int j = H1; j <= A8; j++) {

            Square sq1 = static_cast<Square>(i);
            Square sq2 = static_cast<Square>(j);

            int distanceBetweenFiles = abs(fileOfSquareFILE(sq1) - fileOfSquareFILE(sq2));
            int distanceBetweenRanks = abs(rankOfSquareRANK(sq1) - rankOfSquareRANK(sq2));
            ChebyshevDistance[i][j] = std::max(distanceBetweenFiles, distanceBetweenRanks);
        }
    }
}

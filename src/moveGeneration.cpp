#include "moveGeneration.hpp"
#include "utility.hpp"
#include "moves.hpp"
#include "board.hpp"

Move* generateAllPseudoMoves(const ChessBoard& board, Move* movesList) {
    
    movesList = generatePawnMoves  (board, movesList);
    movesList = generateKnightMoves(board, movesList);
    movesList = generateBishopMoves(board, movesList);
    movesList = generateRookMoves  (board, movesList);
    movesList = generateQueenMoves (board, movesList);
    movesList = generateKingMoves  (board, movesList);
    return movesList;

}
/*
Move* generatePawnMoves(const ChessBoard& board, Move* movesList) {

    Color sideToPlay       = board.sideToPlay;
    Square enPassantSquare = board.enPassant;

    Square seventhRankStart = (sideToPlay == WHITE) ? H7 : H2;
    Square seventhRankEnd   = (sideToPlay == WHITE) ? A7 : A2;
    Direction pawnPush      = (sideToPlay == WHITE) ? NORTH : SOUTH;

    int pieceIndex      = WHITE_PAWN + (6 * sideToPlay);
    int numberOfPawns   = board.pieceCount[pieceIndex];

    Bitboard attacks, captures, quiets, enPassantBB, enPassantCaptures;
    Square sq;
    enPassantBB = (enPassantSquare != NO_SQUARE) ? squareToBitboard(enPassantSquare) : 0;

    //Generates capture moves
    for (int i = 0; i < numberOfPawns; i++) {

        sq                = board.pieceSquare[pieceIndex][i];
        attacks           = pawnAttacks[sideToPlay][sq] & (~board.piecesOnSide[sideToPlay]);
        captures          = attacks & (board.piecesOnSide[~sideToPlay]);
        enPassantCaptures = attacks & enPassantBB;

        if((sq >= seventhRankStart) && (sq <= seventhRankEnd)) {

            while (captures > 0) {
                Square toSquare = squareOfLS1B(&captures);
                movesList++->move = move(sq, toSquare, KNIGHT_PROMOTION_CAPTURE);
                movesList++->move = move(sq, toSquare, BISHOP_PROMOTION_CAPTURE);
                movesList++->move = move(sq, toSquare, ROOK_PROMOTION_CAPTURE  );
                movesList++->move = move(sq, toSquare, QUEEN_PROMOTION_CAPTURE );
            }

        } else {

            while (captures > 0) {
                movesList++->move = move(sq, squareOfLS1B(&captures), CAPTURE);
            }
            while (enPassantCaptures > 0) {
                movesList++->move = move(sq, squareOfLS1B(&enPassantCaptures), EN_PASSANT_CAPTURE);
            }
        }

    }

    //Generates quiet moves
    quiets = pawnsAbleToPush(board.pieces[pieceIndex], board.emptySquares, sideToPlay);

    while (quiets > 0) {
        sq = squareOfLS1B(&quiets);
        int toSquare = sq + pawnPush;
        if((sq >= seventhRankStart) && (sq <= seventhRankEnd)) {

            movesList++->move = move(sq, toSquare, KNIGHT_PROMOTION);
            movesList++->move = move(sq, toSquare, BISHOP_PROMOTION);
            movesList++->move = move(sq, toSquare, ROOK_PROMOTION  );
            movesList++->move = move(sq, toSquare, QUEEN_PROMOTION );

        } else {
            movesList++->move = move(sq, toSquare, QUIET);
        }
    }

    quiets = pawnsAbleToPushTwice(board.pieces[pieceIndex], board.emptySquares, sideToPlay);

    while (quiets > 0) {

        sq = squareOfLS1B(&quiets);
        int toSquare = sq + pawnPush + pawnPush;
        movesList++->move = move(sq, toSquare, DOUBLE_PAWN_PUSH);

    }
    return movesList;
}*/

Move* generatePawnMoves(const ChessBoard& board, Move* movesList) {

    Color sideToPlay       = board.sideToPlay;
    Square enPassantSquare = board.enPassant;
    Bitboard emptySquares  = board.emptySquares;
    Bitboard enemyPieces   = board.piecesOnSide[~sideToPlay];

    Direction pawnPush      = (sideToPlay == WHITE) ? NORTH : SOUTH;
    Bitboard seventhRank    = (sideToPlay == WHITE) ? RANK_7_BB : RANK_2_BB;

    Piece pieceIndex = WHITE_PAWN + (6 * sideToPlay);
    Bitboard pawns   = board.getPieces(pieceIndex);

    Bitboard captures, quiets, enPassantBB, enPassantCaptures, pushOnce, seventhRankPawns;
    Square sq;
    int toSquare;
    enPassantBB = (enPassantSquare != NO_SQUARE) ? squareToBitboard(enPassantSquare) : 0;

    quiets = pawnsAbleToPushTwice(pawns, emptySquares, sideToPlay);
    pawns ^= quiets; 

    pushOnce = pawnsAbleToPush(pawns, emptySquares, sideToPlay);
    pawns ^= pushOnce;

    while (quiets > 0) {

        sq = squareOfLS1B(&quiets);
        toSquare = sq + pawnPush;

        movesList++->move = move(sq, toSquare, QUIET);
        movesList++->move = move(sq, toSquare + pawnPush, DOUBLE_PAWN_PUSH);

        captures = pawnAttacks[sideToPlay][sq] & enemyPieces;

        while (captures > 0) {
            movesList++->move = move(sq, squareOfLS1B(&captures), CAPTURE);
        }

    }

    seventhRankPawns = pushOnce & seventhRank;
    pushOnce ^= seventhRankPawns;
    while (pushOnce > 0) {
        sq = squareOfLS1B(&pushOnce);
        toSquare = sq + pawnPush;

        movesList++->move = move(sq, toSquare, QUIET);

        captures = pawnAttacks[sideToPlay][sq] & enemyPieces;
        enPassantCaptures = pawnAttacks[sideToPlay][sq] & enPassantBB;

        while (captures > 0) {
            movesList++->move = move(sq, squareOfLS1B(&captures), CAPTURE);
        }

        while (enPassantCaptures > 0) {
            movesList++->move = move(sq, squareOfLS1B(&enPassantCaptures), EN_PASSANT_CAPTURE);
        }
    }

    while (seventhRankPawns > 0) {

        sq = squareOfLS1B(&seventhRankPawns);
        toSquare = sq + pawnPush;

        movesList++->move = move(sq, toSquare, KNIGHT_PROMOTION);
        movesList++->move = move(sq, toSquare, BISHOP_PROMOTION);
        movesList++->move = move(sq, toSquare, ROOK_PROMOTION  );
        movesList++->move = move(sq, toSquare, QUEEN_PROMOTION );

        captures = pawnAttacks[sideToPlay][sq] & enemyPieces;

        while (captures > 0) {
            Square toSquare2 = squareOfLS1B(&captures);
            movesList++->move = move(sq, toSquare2, KNIGHT_PROMOTION_CAPTURE);
            movesList++->move = move(sq, toSquare2, BISHOP_PROMOTION_CAPTURE);
            movesList++->move = move(sq, toSquare2, ROOK_PROMOTION_CAPTURE  );
            movesList++->move = move(sq, toSquare2, QUEEN_PROMOTION_CAPTURE );
        }
    }

    seventhRankPawns = pawns & seventhRank;
    pawns ^= seventhRankPawns;

    while (pawns > 0) {

        sq = squareOfLS1B(&pawns);
        captures          = pawnAttacks[sideToPlay][sq] & enemyPieces;
        enPassantCaptures = pawnAttacks[sideToPlay][sq] & enPassantBB;

        while (captures > 0) {
            movesList++->move = move(sq, squareOfLS1B(&captures), CAPTURE);
        }

        while (enPassantCaptures > 0) {
            movesList++->move = move(sq, squareOfLS1B(&enPassantCaptures), EN_PASSANT_CAPTURE);
        }
    }

    while (seventhRankPawns > 0) {

        sq = squareOfLS1B(&seventhRankPawns);
        captures = pawnAttacks[sideToPlay][sq] & enemyPieces;

        while (captures > 0) {
            Square toSquare2 = squareOfLS1B(&captures);
            movesList++->move = move(sq, toSquare2, KNIGHT_PROMOTION_CAPTURE);
            movesList++->move = move(sq, toSquare2, BISHOP_PROMOTION_CAPTURE);
            movesList++->move = move(sq, toSquare2, ROOK_PROMOTION_CAPTURE  );
            movesList++->move = move(sq, toSquare2, QUEEN_PROMOTION_CAPTURE );
        }
    }
    return movesList;
}

Move* generateKnightMoves(const ChessBoard& board, Move* movesList) {

    Color sideToPlay = board.sideToPlay;

    int pieceIndex      = WHITE_KNIGHT + (6 * sideToPlay);
    int numberOfKnights = board.pieceCount[pieceIndex];

    Bitboard attacks, captures, quiets;
    Square sq;

    for (int i = 0; i < numberOfKnights; i++) {

        sq        = board.pieceSquare[pieceIndex][i];
        attacks   = knightAttacks[sq] & (~board.piecesOnSide[sideToPlay]);
        captures  = attacks & board.piecesOnSide[~sideToPlay];
        quiets    = attacks ^ captures;

        while (captures > 0) {
            movesList++->move = move(sq, squareOfLS1B(&captures), CAPTURE);
        }

        while (quiets > 0) {
            movesList++->move = move(sq, squareOfLS1B(&quiets), QUIET);
        }

    }
    return movesList;
}

Move* generateBishopMoves(const ChessBoard& board, Move* movesList) {

    Color sideToPlay          = board.sideToPlay;
    Bitboard availableSquares = ~board.piecesOnSide[sideToPlay];
    Bitboard enemyPieces      = board.piecesOnSide[~sideToPlay];
    Bitboard occupied         = board.occupiedSquares;

    int pieceIndex      = WHITE_BISHOP + (6 * sideToPlay);
    int numberOfBishops = board.pieceCount[pieceIndex];

    Bitboard attacks, captures, quiets;
    Square sq;

    for (int i = 0; i < numberOfBishops; i++) {

        sq        = board.pieceSquare[pieceIndex][i];
        attacks   = bishopAttacks(occupied, sq) & availableSquares;
        captures  = attacks & enemyPieces;
        quiets    = attacks ^ captures;

        while (captures > 0) {
            movesList++->move = move(sq, squareOfLS1B(&captures), CAPTURE);
        }

        while (quiets > 0) {
            movesList++->move = move(sq, squareOfLS1B(&quiets), QUIET);
        }
    }
    return movesList;
}

Move* generateRookMoves(const ChessBoard& board, Move* movesList) {

    Color sideToPlay          = board.sideToPlay;
    Bitboard availableSquares = ~board.piecesOnSide[sideToPlay];
    Bitboard enemyPieces      = board.piecesOnSide[~sideToPlay];
    Bitboard occupied         = board.occupiedSquares;

    int pieceIndex      = WHITE_ROOK + (6 * sideToPlay);
    int numberOfRooks   = board.pieceCount[pieceIndex];

    Bitboard attacks, captures, quiets;
    Square sq;

    for (int i = 0; i < numberOfRooks; i++) {

        sq        = board.pieceSquare[pieceIndex][i];
        attacks   = rookAttacks(occupied, sq) & availableSquares;
        captures  = attacks & enemyPieces;
        quiets    = attacks ^ captures;

        while (captures > 0) {
            movesList++->move = move(sq, squareOfLS1B(&captures), CAPTURE);
        }

        while (quiets > 0) {
            movesList++->move = move(sq, squareOfLS1B(&quiets), QUIET);
        }
    }
    return movesList;
}

Move* generateQueenMoves(const ChessBoard& board, Move* movesList) {

    Color sideToPlay          = board.sideToPlay;
    Bitboard availableSquares = ~board.piecesOnSide[sideToPlay];
    Bitboard enemyPieces      = board.piecesOnSide[~sideToPlay];
    Bitboard occupied         = board.occupiedSquares;

    int pieceIndex      = WHITE_QUEEN + (6 * sideToPlay);
    int numberOfQueens  = board.pieceCount[pieceIndex];

    Bitboard attacks, captures, quiets;
    Square sq;

    for (int i = 0; i < numberOfQueens; i++) {

        sq        = board.pieceSquare[pieceIndex][i];
        attacks   = (bishopAttacks(occupied, sq) | rookAttacks(occupied, sq)) & availableSquares;
        captures  = attacks & enemyPieces;
        quiets    = attacks ^ captures;

        while (captures > 0) {
            movesList++->move = move(sq, squareOfLS1B(&captures), CAPTURE);
        }

        while (quiets > 0) {
            movesList++->move = move(sq, squareOfLS1B(&quiets), QUIET);
        }

    }
    return movesList;
}
/*
Move* generateKingMoves(const ChessBoard& board, Move* movesList) {

    int pieceIndex = WHITE_KING + (6 * board.sideToPlay);
    int numberOfKings = board.pieceCount[pieceIndex];

    Bitboard attacks, captures, quiets;
    Square sq;

    for (int i = 0; i < numberOfKings; i++) {

        sq        = board.pieceSquare[pieceIndex][i];
        attacks   = kingAttacks[sq] & (~board.piecesOnSide[board.sideToPlay]);
        captures  = attacks & board.piecesOnSide[~board.sideToPlay];
        quiets    = attacks ^ captures;

        while (captures > 0) {
            movesList++->move = move(sq, squareOfLS1B(&captures), CAPTURE);
        }

        while (quiets > 0) {
            movesList++->move = move(sq, squareOfLS1B(&quiets), QUIET);
        }

    }

    unsigned int castlingBits = (board.sideToPlay == WHITE) ? board.castlingRights : board.castlingRights >> 2;
    Bitboard homeRankEmpty = (board.sideToPlay == WHITE) ? RANK_1_BB : RANK_8_BB;
    homeRankEmpty &= board.emptySquares;

    Color c = board.sideToPlay;
    //See if can kingside castle
    if ((castlingBits & 0b1) > 0) {

        Bitboard emptyFFile = homeRankEmpty & FILE_F_BB;
        Bitboard emptyGFile = homeRankEmpty & FILE_G_BB;
        Square s1 = (c == WHITE) ? E1 : E8;
        Square s2 = (c == WHITE) ? F1 : F8;
        Square s3 = (c == WHITE) ? G1 : G8;
        

        if ((emptyFFile > 0) && (emptyGFile > 0) 
            && !board.isSquareAttacked(s1, c) && !board.isSquareAttacked(s2, c) && !board.isSquareAttacked(s3, c))
            movesList++->move = move(s1, s3, KINGSIDE_CASTLE);

    }

    //See if can queenside castle
    if ((castlingBits & 0b10) > 0) {

        Bitboard emptyDFile = homeRankEmpty & FILE_D_BB;
        Bitboard emptyCFile = homeRankEmpty & FILE_C_BB;
        Bitboard emptyBFile = homeRankEmpty & FILE_B_BB;
        Square s1 = (c == WHITE) ? E1 : E8;
        Square s2 = (c == WHITE) ? D1 : D8;
        Square s3 = (c == WHITE) ? C1 : C8;
        

        if ((emptyDFile > 0) && (emptyCFile > 0) && (emptyBFile)
            && !board.isSquareAttacked(s1, c) && !board.isSquareAttacked(s2, c) && !board.isSquareAttacked(s3, c))
            movesList++->move = move(s1, s3, QUEENSIDE_CASTLE);

    }
    return movesList;
}*/

Move* generateKingMoves(const ChessBoard& board, Move* movesList) {

    Color sideToPlay = board.sideToPlay;
    Bitboard availableSquares = ~board.piecesOnSide[sideToPlay];
    Bitboard enemyPieces      = board.piecesOnSide[~sideToPlay];

    int pieceIndex = WHITE_KING + (6 * sideToPlay);

    Bitboard attacks, captures, quiets;
    Square sq;

    sq        = board.pieceSquare[pieceIndex][0];
    attacks   = kingAttacks[sq] & availableSquares;
    captures  = attacks & enemyPieces;
    quiets    = attacks ^ captures;

    while (captures > 0) {
        movesList++->move = move(sq, squareOfLS1B(&captures), CAPTURE);
    }

    while (quiets > 0) {
        movesList++->move = move(sq, squareOfLS1B(&quiets), QUIET);
    }

    unsigned int castlingBits = (sideToPlay == WHITE) ? board.castlingRights : board.castlingRights >> 2;
    Bitboard homeRankEmpty    = (sideToPlay == WHITE) ? RANK_1_BB : RANK_8_BB;
    homeRankEmpty &= board.emptySquares;

    //See if can kingside castle
    if ((castlingBits & 0b01) > 0) {

        Bitboard emptyFFile = homeRankEmpty & FILE_F_BB;
        Bitboard emptyGFile = homeRankEmpty & FILE_G_BB;
        Square s1 = (sideToPlay == WHITE) ? E1 : E8;
        Square s2 = (sideToPlay == WHITE) ? F1 : F8;
        Square s3 = (sideToPlay == WHITE) ? G1 : G8;
        

        if ((emptyFFile > 0) && (emptyGFile > 0) 
            && !board.isSquareAttacked(s1, sideToPlay) && !board.isSquareAttacked(s2, sideToPlay) && !board.isSquareAttacked(s3, sideToPlay))
            movesList++->move = move(s1, s3, KINGSIDE_CASTLE);

    }

    //See if can queenside castle
    if ((castlingBits & 0b10) > 0) {

        Bitboard emptyDFile = homeRankEmpty & FILE_D_BB;
        Bitboard emptyCFile = homeRankEmpty & FILE_C_BB;
        Bitboard emptyBFile = homeRankEmpty & FILE_B_BB;
        Square s1 = (sideToPlay == WHITE) ? E1 : E8;
        Square s2 = (sideToPlay == WHITE) ? D1 : D8;
        Square s3 = (sideToPlay == WHITE) ? C1 : C8;
        

        if ((emptyDFile > 0) && (emptyCFile > 0) && (emptyBFile)
            && !board.isSquareAttacked(s1, sideToPlay) && !board.isSquareAttacked(s2, sideToPlay) && !board.isSquareAttacked(s3, sideToPlay))
            movesList++->move = move(s1, s3, QUEENSIDE_CASTLE);

    }
    return movesList;
}

Move* generateAllPseudoCaptureMoves(const ChessBoard& board, Move* movesList) {

    movesList = generatePawnCaptureMoves  (board, movesList);
    movesList = generateKnightCaptureMoves(board, movesList);
    movesList = generateBishopCaptureMoves(board, movesList);
    movesList = generateRookCaptureMoves  (board, movesList);
    movesList = generateQueenCaptureMoves (board, movesList);
    movesList = generateKingCaptureMoves  (board, movesList);
    return movesList;

}

Move* generatePawnCaptureMoves(const ChessBoard& board, Move* movesList) {

    Color sideToPlay       = board.sideToPlay;
    Square enPassantSquare = board.enPassant;
    Bitboard emptySquares  = board.emptySquares;
    Bitboard enemyPieces   = board.piecesOnSide[~sideToPlay];

    Direction pawnPush      = (sideToPlay == WHITE) ? NORTH : SOUTH;
    Bitboard seventhRank    = (sideToPlay == WHITE) ? RANK_7_BB : RANK_2_BB;

    Piece pieceIndex = WHITE_PAWN + (6 * sideToPlay);
    Bitboard pawns   = board.getPieces(pieceIndex);

    Bitboard captures, quiets, enPassantBB, enPassantCaptures, pushOnce, seventhRankPawns;
    Square sq;
    int toSquare;
    enPassantBB = (enPassantSquare != NO_SQUARE) ? squareToBitboard(enPassantSquare) : 0;

    quiets = pawnsAbleToPushTwice(pawns, emptySquares, sideToPlay);
    pawns ^= quiets; 

    pushOnce = pawnsAbleToPush(pawns, emptySquares, sideToPlay);
    pawns ^= pushOnce;

    while (quiets > 0) {

        sq       = squareOfLS1B(&quiets);
        captures = pawnAttacks[sideToPlay][sq] & enemyPieces;

        while (captures > 0) {
            movesList++->move = move(sq, squareOfLS1B(&captures), CAPTURE);
        }
    }

    seventhRankPawns = pushOnce & seventhRank;
    pushOnce ^= seventhRankPawns;
    while (pushOnce > 0) {

        sq                = squareOfLS1B(&pushOnce);
        captures          = pawnAttacks[sideToPlay][sq] & enemyPieces;
        enPassantCaptures = pawnAttacks[sideToPlay][sq] & enPassantBB;

        while (captures > 0) {
            movesList++->move = move(sq, squareOfLS1B(&captures), CAPTURE);
        }

        while (enPassantCaptures > 0) {
            movesList++->move = move(sq, squareOfLS1B(&enPassantCaptures), EN_PASSANT_CAPTURE);
        }
    }

    while (seventhRankPawns > 0) {

        sq       = squareOfLS1B(&seventhRankPawns);
        captures = pawnAttacks[sideToPlay][sq] & enemyPieces;

        while (captures > 0) {
            Square toSquare2 = squareOfLS1B(&captures);
            movesList++->move = move(sq, toSquare2, KNIGHT_PROMOTION_CAPTURE);
            movesList++->move = move(sq, toSquare2, BISHOP_PROMOTION_CAPTURE);
            movesList++->move = move(sq, toSquare2, ROOK_PROMOTION_CAPTURE  );
            movesList++->move = move(sq, toSquare2, QUEEN_PROMOTION_CAPTURE );
        }
    }

    seventhRankPawns = pawns & seventhRank;
    pawns ^= seventhRankPawns;

    while (pawns > 0) {

        sq                = squareOfLS1B(&pawns);
        captures          = pawnAttacks[sideToPlay][sq] & enemyPieces;
        enPassantCaptures = pawnAttacks[sideToPlay][sq] & enPassantBB;

        while (captures > 0) {
            movesList++->move = move(sq, squareOfLS1B(&captures), CAPTURE);
        }

        while (enPassantCaptures > 0) {
            movesList++->move = move(sq, squareOfLS1B(&enPassantCaptures), EN_PASSANT_CAPTURE);
        }
    }

    while (seventhRankPawns > 0) {

        sq       = squareOfLS1B(&seventhRankPawns);
        captures = pawnAttacks[sideToPlay][sq] & enemyPieces;

        while (captures > 0) {
            Square toSquare2 = squareOfLS1B(&captures);
            movesList++->move = move(sq, toSquare2, KNIGHT_PROMOTION_CAPTURE);
            movesList++->move = move(sq, toSquare2, BISHOP_PROMOTION_CAPTURE);
            movesList++->move = move(sq, toSquare2, ROOK_PROMOTION_CAPTURE  );
            movesList++->move = move(sq, toSquare2, QUEEN_PROMOTION_CAPTURE );
        }
    }
    return movesList;
}

Move* generateKnightCaptureMoves(const ChessBoard& board, Move* movesList) {

    Color sideToPlay = board.sideToPlay;

    int pieceIndex      = WHITE_KNIGHT + (6 * sideToPlay);
    int numberOfKnights = board.pieceCount[pieceIndex];

    Bitboard captures;
    Square sq;

    for (int i = 0; i < numberOfKnights; i++) {

        sq       = board.pieceSquare[pieceIndex][i];
        captures = knightAttacks[sq] & board.piecesOnSide[~sideToPlay];

        while (captures > 0) {
            movesList++->move = move(sq, squareOfLS1B(&captures), CAPTURE);
        }
    }
    return movesList;
}

Move* generateBishopCaptureMoves(const ChessBoard& board, Move* movesList) {

    Color sideToPlay     = board.sideToPlay;
    Bitboard enemyPieces = board.piecesOnSide[~sideToPlay];
    Bitboard occupied    = board.occupiedSquares;

    int pieceIndex      = WHITE_BISHOP + (6 * sideToPlay);
    int numberOfBishops = board.pieceCount[pieceIndex];

    Bitboard captures;
    Square sq;

    for (int i = 0; i < numberOfBishops; i++) {

        sq       = board.pieceSquare[pieceIndex][i];
        captures = bishopAttacks(occupied, sq) & enemyPieces;

        while (captures > 0) {
            movesList++->move = move(sq, squareOfLS1B(&captures), CAPTURE);
        }
    }
    return movesList;
}

Move* generateRookCaptureMoves(const ChessBoard& board, Move* movesList) {

    Color sideToPlay     = board.sideToPlay;
    Bitboard enemyPieces = board.piecesOnSide[~sideToPlay];
    Bitboard occupied    = board.occupiedSquares;

    int pieceIndex    = WHITE_ROOK + (6 * sideToPlay);
    int numberOfRooks = board.pieceCount[pieceIndex];

    Bitboard captures;
    Square sq;

    for (int i = 0; i < numberOfRooks; i++) {

        sq       = board.pieceSquare[pieceIndex][i];
        captures = rookAttacks(occupied, sq) & enemyPieces;

        while (captures > 0) {
            movesList++->move = move(sq, squareOfLS1B(&captures), CAPTURE);
        }
    }
    return movesList;
}

Move* generateQueenCaptureMoves(const ChessBoard& board, Move* movesList) {

    Color sideToPlay     = board.sideToPlay;
    Bitboard enemyPieces = board.piecesOnSide[~sideToPlay];
    Bitboard occupied    = board.occupiedSquares;

    int pieceIndex     = WHITE_QUEEN + (6 * sideToPlay);
    int numberOfQueens = board.pieceCount[pieceIndex];

    Bitboard captures;
    Square sq;

    for (int i = 0; i < numberOfQueens; i++) {

        sq       = board.pieceSquare[pieceIndex][i];
        captures = (bishopAttacks(occupied, sq) | rookAttacks(occupied, sq)) & enemyPieces;

        while (captures > 0) {
            movesList++->move = move(sq, squareOfLS1B(&captures), CAPTURE);
        }
    }
    return movesList;
}

Move* generateKingCaptureMoves(const ChessBoard& board, Move* movesList) {

    Color sideToPlay     = board.sideToPlay;
    Bitboard enemyPieces = board.piecesOnSide[~sideToPlay];

    int pieceIndex = WHITE_KING + (6 * sideToPlay);

    Bitboard captures;
    Square sq;

    sq       = board.pieceSquare[pieceIndex][0];
    captures = kingAttacks[sq] & enemyPieces;

    while (captures > 0) {
        movesList++->move = move(sq, squareOfLS1B(&captures), CAPTURE);
    }

    return movesList;
}

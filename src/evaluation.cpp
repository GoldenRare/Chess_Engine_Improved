<<<<<<< HEAD

#include <algorithm>
#include <cstdlib>
#include "evaluation.hpp"
#include "utility.hpp"
#include "board.hpp"
#include "moves.hpp"

CombinedScore pieceSquareTable[PIECES][NUMBER_OF_SQUARES];

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

    Bitboard temp;
    Bitboard whiteKingBlockers = board.blockers(whiteKingSq, board.piecesOnSide[BLACK], temp);
    Bitboard blackKingBlockers = board.blockers(blackKingSq, board.piecesOnSide[WHITE], temp);

    Bitboard whitePawnAttacks = allPawnAttacks(whitePawns, WHITE);
    Bitboard blackPawnAttacks = allPawnAttacks(blackPawns, BLACK);

    Bitboard squaresAttackedTwiceByPawnsWHITE = attackedTwiceByPawns(whitePawns, WHITE);
    Bitboard squaresAttackedTwiceByPawnsBLACK = attackedTwiceByPawns(blackPawns, BLACK);
    ////////////////////////////////////////////////////////////

    //Initialize mobility area /////////////////////////////////
    mobilityArea[WHITE] = ~(whiteBadPawns | board.pieces[WHITE_KING] | board.pieces[WHITE_QUEEN] | whiteKingBlockers | blackPawnAttacks);
    mobilityArea[BLACK] = ~(blackBadPawns | board.pieces[BLACK_KING] | board.pieces[BLACK_QUEEN] | blackKingBlockers | whitePawnAttacks);

    totalMobility[BLACK] = totalMobility[WHITE] = 0;
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

    //Initialize king safety /////////////////////////////////////
    Bitboard whiteKingSq2BB = FILE_A_BB >> keepInRange(FILE_B, FILE_G, fileOfSquareFILE(whiteKingSq)) &
                              RANK_1_BB >> keepInRange(RANK_2, RANK_7, rankOfSquareRANK(whiteKingSq)) * NORTH;
    Bitboard blackKingSq2BB = FILE_A_BB >> keepInRange(FILE_B, FILE_G, fileOfSquareFILE(blackKingSq)) &
                              RANK_1_BB >> keepInRange(RANK_2, RANK_7, rankOfSquareRANK(blackKingSq)) * NORTH;
    Square whiteKingSq2 = squareOfLS1B(whiteKingSq2BB);
    Square blackKingSq2 = squareOfLS1B(blackKingSq2BB);

    kingRing[WHITE] = kingAttacks[whiteKingSq2] | whiteKingSq2BB;
    kingRing[BLACK] = kingAttacks[blackKingSq2] | blackKingSq2BB;

    attackingEnemyKingRingCount[WHITE] = populationCount(kingRing[BLACK] & whitePawnAttacks);
    attackingEnemyKingRingCount[BLACK] = populationCount(kingRing[WHITE] & blackPawnAttacks);

    kingRing[WHITE] &= ~squaresAttackedTwiceByPawnsWHITE;
    kingRing[BLACK] &= ~squaresAttackedTwiceByPawnsBLACK;

    attackedSquaresAroundEnemyKing[WHITE] = attackingEnemyKingRingPieceWeight[WHITE] = 0;
    attackedSquaresAroundEnemyKing[BLACK] = attackingEnemyKingRingPieceWeight[BLACK] = 0;
    //////////////////////////////////////////////////////////////
    
}

CombinedScore Evaluation::evaluatePiece(PieceType pt, Color c) {

    Bitboard temp;
    /////////////////////////////////////
    Square kingSq = board.pieceSquare[WHITE_KING + (6 * c)][0];

    Bitboard thesePawns   = board.pieces[c == WHITE ? WHITE_PAWN : BLACK_PAWN];
    Bitboard otherPawns   = board.pieces[c == WHITE ? BLACK_PAWN : WHITE_PAWN];
    Bitboard otherBishops = board.pieces[c == WHITE ? BLACK_BISHOP : WHITE_BISHOP];
    Bitboard otherRooks   = board.pieces[c == WHITE ? BLACK_ROOK : WHITE_ROOK];

    Bitboard queens = board.pieces[WHITE_QUEEN] | board.pieces[BLACK_QUEEN];
    Bitboard kingBlockers = board.blockers(kingSq, board.piecesOnSide[~c], temp);
    /////////////////////////////////////

    int pieceIndex     = pt + (6 * c);
    int numberOfPieces = board.pieceCount[pieceIndex];

    Bitboard attacks, sqBB;
    Square sq;
    CombinedScore cs = 0;

    attacksBy[c][pt] = 0;
    
    for (int i = 0; i < numberOfPieces; i++) {

        sq   = board.pieceSquare[pieceIndex][i];
        sqBB = squareToBitboard(sq);

        //Includes x-ray attacks for sliders
        attacks = (pt == KNIGHT) ? knightAttacks[sq] 
                : (pt == BISHOP) ? bishopAttacks(board.gameBoard ^ queens, sq)
                : (pt == ROOK  ) ? rookAttacks(board.gameBoard ^ queens ^ board.pieces[pieceIndex], sq)
                : bishopAttacks(board.gameBoard, sq) | rookAttacks(board.gameBoard, sq);

        // If piece is pinned to the king, then can only move along the ray of the incoming attack
        if (kingBlockers & sq)
            attacks &= rayLine[kingSq][sq];

        attacksBy2[c] |= (attacksBy[c][ALL_PIECE_TYPES] & attacks);
        attacksBy[c][pt] |= attacks;
        attacksBy[c][ALL_PIECE_TYPES] |= attacks;

        if ((attacks & kingRing[~c]) > 0) {

            attackingEnemyKingRingCount[c]++;
            attackedSquaresAroundEnemyKing[c] += populationCount(attacks & attacksBy[~c][KING]);
            attackingEnemyKingRingPieceWeight[c] += KING_ATTACKERS_WEIGHT[pt];
        }

        int mobility = populationCount(attacks & mobilityArea[c]);
        totalMobility[c] += MOBILITY_BONUS[pt][mobility]; 

        if ((pt == KNIGHT) || (pt == BISHOP)) {
            
            
            Bitboard outposts = attacksBy[c][PAWN] & (c == WHITE) ? whiteOutpostRanks & (~pawnAttackSpans(board.pieces[BLACK_PAWN], BLACK))
                                                                  : blackOutpostRanks & (~pawnAttackSpans(board.pieces[WHITE_PAWN], WHITE));

            if ((outposts & sqBB) > 0) cs += (OUTPOST_BONUS * (pt == KNIGHT) ? 2 : 1);
            else if ((pt == KNIGHT) && ((outposts & attacks & (~board.piecesOnSide[c])) > 0)) cs += REACHABLE_OUTPOST_BONUS;

            //Uses pawnsAbleToPush as a hack for detecting minor pieces behind pawns
            if (pawnsAbleToPush(sqBB, board.pieces[WHITE_PAWN] | board.pieces[BLACK_PAWN], c) > 0) cs += MINOR_PIECE_BEHIND_PAWN_BONUS;

            cs -= (KING_PROTECTOR_PENALTY * ChebyshevDistance[sq][kingSq]);
            
            if (pt == BISHOP) {

                Bitboard blockedPawns = pawnsAbleToPush(thesePawns, board.gameBoard, c);
                cs -= PAWNS_BLOCKING_BISHOP_PENALTY * (populationCount(blockedPawns & CENTER_FILES) + 1) * 
                      populationCount(thesePawns & ((sqBB & DARK_SQUARES) > 0) ? DARK_SQUARES : LIGHT_SQUARES);

                if (hasMoreThanOneBit(bishopAttacks(thesePawns | otherPawns, sq) & CENTER_SQUARES)) 
                    cs += BISHOP_CONTROL_OF_CENTER_BONUS;
            } 
        } else if (pt == ROOK) {

            Bitboard fileSqBB = fileOfSquareBB(sq);
            if ((fileSqBB & queens) > 0) cs += ROOK_ON_QUEEN_FILE_BONUS;

            if ((fileSqBB & thesePawns) == 0) cs += ROOK_ON_SEMI_OR_OPEN_FILE_BONUS[(fileSqBB & otherPawns) == 0 ? 1 : 0];
            else if (mobility <= 3) {

                File kingFile = fileOfSquareFILE(kingSq);
                int castlingRights = board.castlingRights & (c == WHITE ? 0b0011 : 0b1100); 
                if ((kingFile < FILE_E) == (fileOfSquareFILE(sq) < kingFile))
                    cs -= ROOK_TRAPPED_PENALTY * (castlingRights == 0 ? 2 : 1);

            }
        } else { // Then should be a Queen
            if (board.blockers(sq, otherBishops | otherRooks, temp) > 0) cs -= QUEEN_IS_WEAK_PENALTY;
        }
    }

    return cs;
}

CombinedScore Evaluation::evaluatePawns(Color c) {

    CombinedScore cs = 0;
    passedPawns[c]   = 0;
    /////////////////////////////////////////
    Direction pawnPush = (c == WHITE) ? NORTH : SOUTH;

    Bitboard thesePawns = board.pieces[c == WHITE ? WHITE_PAWN : BLACK_PAWN];
    Bitboard otherPawns = board.pieces[c == WHITE ? BLACK_PAWN : WHITE_PAWN];
    /////////////////////////////////////////

    int pawnIndex     = WHITE_PAWN + (6 * c);
    int numberOfPawns = board.pieceCount[pawnIndex];

    Square sq;
    Bitboard sqToBB;
    Bitboard opposing, blocking, stopping, levers, pawnPushLevers, adjacent, sideBySide, supporting;
    bool doubled, backwards, passed;

    for (int i = 0; i < numberOfPawns; i++) {

        sq     = board.pieceSquare[pawnIndex][i];
        sqToBB = squareToBitboard(sq);

        Rank relativeRANK = relativeRank(c, sq);
        //////////////////////////////
        opposing       = otherPawns & pawnFrontSpans(sqToBB, c);
        blocking       = otherPawns & squareToBitboard(Square(sq + pawnPush));
        stopping       = otherPawns & (pawnFrontSpans(sqToBB, c) | pawnAttackSpans(sqToBB, c));
        levers         = otherPawns & pawnAttacks[c][sq];
        pawnPushLevers = otherPawns & pawnAttacks[c][sq + pawnPush];
        doubled        = (thesePawns & squareToBitboard(Square(sq - pawnPush))) > 0;
        adjacent       = thesePawns & adjacentFiles(sq);
        sideBySide     = adjacent   & rankOfSquareBB(sq);
        supporting     = adjacent   & rankOfSquareBB(Square(sq - pawnPush));
        backwards      = ((adjacent & ranksInFront(~c, Square(sq + pawnPush))) == 0) && ((pawnPushLevers > 0) || (blocking > 0));
        passed         = ((stopping ^ levers) == 0) 
                      || ((stopping ^ pawnPushLevers) && (populationCount(sideBySide) >= populationCount(pawnPushLevers)))
                      || ((stopping == blocking) && (relativeRANK >= RANK_5) 
                      && (pawnsAbleToPush(supporting, otherPawns | attackedTwiceByPawns(otherPawns, ~c), c) == 0)); //Uses pawnsAbleToPush as a hack
        //////////////////////////////

        if (passed) passedPawns[c] |= squareToBitboard(sq);

        if ((supporting > 0) || (sideBySide > 0)) {

            int sideBySideBonus = (sideBySide > 0) ? 1 : 0;
            int opposedPenalty  = (opposing   > 0) ? 1 : 0;

            ExactScore score = (CONNECTED_PAWN_BONUS[relativeRANK] * (2 + sideBySideBonus - opposedPenalty))
                             + (21 * populationCount(supporting));

            cs += makeScore(score, (score * (relativeRANK - 2)) / 4);

        } 
        else if (adjacent == 0) cs -= ISOLATED_PAWN_PENALTY + (opposing == 0) ? OPEN_TO_ATTACK_PENALTY : 0;
        else if (backwards    ) cs -= BACKWARD_PAWN_PENALTY + (opposing == 0) ? OPEN_TO_ATTACK_PENALTY : 0; 

        if (supporting == 0) cs -= (doubled                    ) ? DOUBLED_PAWN_PENALTY : 0 
                                +  ((levers & (levers - 1)) > 0) ? WEAK_LEVER_PENALTY   : 0;
    }

    return cs;

}

CombinedScore Evaluation::evaluatePassedPawns(Color c) {

    /////////////////////////////
    Square thisKing  = board.pieceSquare[c == WHITE ? WHITE_KING : BLACK_KING][0];
    Square otherKing = board.pieceSquare[c == WHITE ? BLACK_KING : WHITE_KING][0];

    Bitboard thesePawns        = board.pieces[c == WHITE ? WHITE_PAWN : BLACK_PAWN];
    Bitboard otherPawns       = board.pieces[c == WHITE ? BLACK_PAWN : WHITE_PAWN];
    Bitboard thisRooksQueens  = board.pieces[c == WHITE ? WHITE_ROOK : BLACK_ROOK] | board.pieces[c == WHITE ? WHITE_QUEEN : BLACK_QUEEN];
    Bitboard otherRooksQueens = board.pieces[c == WHITE ? BLACK_ROOK : WHITE_ROOK] | board.pieces[c == WHITE ? BLACK_QUEEN : WHITE_QUEEN];
    /////////////////////////////

    CombinedScore cs, cs2;
    cs = 0;

    Direction pawnPush = (c == WHITE) ? NORTH : SOUTH;

    Bitboard pp = passedPawns[c];
    while (pp > 0) {

        Square sq       = squareOfLS1B(&pp);
        Bitboard sqToBB = squareToBitboard(sq);
         
        Rank relativeRANK = relativeRank(c, sq);
        cs2 = PASSED_PAWN_BONUS[relativeRANK];

        Square blockingSq = Square(sq + pawnPush);
        Bitboard blockingSqToBB = squareToBitboard(blockingSq);

        if (relativeRANK > RANK_3) {

            int weight = (relativeRANK * 5) - 13;
            
            int thisKingToBlockingSq  = std::min(ChebyshevDistance[thisKing ][blockingSq], 5);
            int otherKingToBlockingSq = std::min(ChebyshevDistance[otherKing][blockingSq], 5);

            cs2 += makeScore(0, (((otherKingToBlockingSq * 19) / 4) - (thisKingToBlockingSq * 2)) * weight);

            if (relativeRANK != RANK_7) 
                cs2 -= makeScore(0, std::min(ChebyshevDistance[thisKing][blockingSq + pawnPush], 5) * weight);

            if ((blockingSqToBB & board.emptySquares) > 0) {

                Bitboard squaresLeftToQueen = pawnFrontSpans(sqToBB, c);
                Bitboard unsafeSquares      = pawnFrontSpans(sqToBB, c) | pawnAttackSpans(sqToBB, c);

                Bitboard behindPassedPawnRooksQueens = pawnFrontSpans(sqToBB, ~c) & (thisRooksQueens| otherRooksQueens);

                //Tarrasch Rule: Rooks should be placed behind passed pawns.
                //If enemy rook/queen behind passed pawn, then all push squares are unsafe.
                //Otherwise check for attacks on the pawn advance by enemy
                if ((behindPassedPawnRooksQueens & board.piecesOnSide[~c]) == 0)
                    unsafeSquares &= attacksBy[~c][ALL_PIECE_TYPES];

                int bonus =  unsafeSquares                       == 0 ? 35
                          : (unsafeSquares & squaresLeftToQueen) == 0 ? 20
                          : (unsafeSquares & blockingSqToBB)     == 0 ? 9 : 0;

                if (((behindPassedPawnRooksQueens & board.piecesOnSide[c]) > 0) || ((attacksBy[c][ALL_PIECE_TYPES] & blockingSqToBB) > 0))
                    bonus += 5;

                cs2 += makeScore(bonus * weight, bonus * weight);
            }

        }

        if ((((thesePawns | otherPawns) & blockingSqToBB) > 0) 
           || (((pawnFrontSpans(blockingSqToBB, c) | pawnAttackSpans(blockingSqToBB, c)) & otherPawns) > 0))
            cs2 = makeScore(middlegameScore(cs2) / 2, endgameScore(cs2) / 2);

        int file = fileOfSquareFILE(sq);
        cs += cs2 - (INSIDE_PASSED_PAWN_PENALTY * std::min(file, FILE_H - file));
    }

    return cs;

}

CombinedScore Evaluation::evaluateSpace(Color c) {

    ////////////////////////////////
    Bitboard thesePawns = c == WHITE ? WHITE_PAWN : BLACK_PAWN;
    int thesePieceCount = c == WHITE ? board.pieceCount[WHITE_PAWN] + board.pieceCount[WHITE_KNIGHT] + board.pieceCount[WHITE_BISHOP] +
                                       board.pieceCount[WHITE_ROOK] + board.pieceCount[WHITE_QUEEN]
                                     : board.pieceCount[BLACK_PAWN] + board.pieceCount[BLACK_KNIGHT] + board.pieceCount[BLACK_BISHOP] +
                                       board.pieceCount[BLACK_ROOK] + board.pieceCount[BLACK_QUEEN];
    ////////////////////////////////

    if (board.nonPawnMaterial[WHITE] + board.nonPawnMaterial[BLACK] < SPACE_THRESHOLD) return 0;

    Bitboard centerSpace = c == WHITE ? CENTER_FILES & (RANK_2_BB | RANK_3_BB | RANK_4_BB)
                                      : CENTER_FILES & (RANK_7_BB | RANK_6_BB | RANK_5_BB);
    Bitboard safeSquares = centerSpace & ~board.pieces[thesePawns] & ~attacksBy[~c][PAWN];
    Bitboard spaceBehindPawns = board.pieces[thesePawns];
    spaceBehindPawns |= c == WHITE ? spaceBehindPawns >> -SOUTH : spaceBehindPawns << NORTH;
    spaceBehindPawns |= c == WHITE ? spaceBehindPawns >> -(SOUTH + SOUTH) : spaceBehindPawns << NORTH + NORTH;

    int bonus = populationCount(safeSquares) + populationCount(spaceBehindPawns & safeSquares & ~attacksBy[~c][ALL_PIECE_TYPES]);
    return makeScore(bonus * thesePieceCount * thesePieceCount / 16, 0);
}

CombinedScore Evaluation::evaluateThreats(Color c) {

    Bitboard nonPawnEnemies, squaresStronglyProtectedByEnemy, defendedEnemies, weakEnemies, safeSquares;

    nonPawnEnemies = board.piecesOnSide[~c] & ~board.pieces[c == WHITE ? BLACK_PAWN : WHITE_PAWN];
    squaresStronglyProtectedByEnemy = attacksBy[~c][PAWN] | (attacksBy2[~c] & ~attacksBy2[c]);
    defendedEnemies = nonPawnEnemies & squaresStronglyProtectedByEnemy;
    weakEnemies = board.piecesOnSide[~c] & ~squaresStronglyProtectedByEnemy & attacksBy[c][ALL_PIECE_TYPES];

    CombinedScore cs = 0;
    Bitboard temp;
    if (defendedEnemies > 0 || weakEnemies > 0) {

        temp = (defendedEnemies | weakEnemies) & (attacksBy[c][KNIGHT] | attacksBy[c][BISHOP]);
        while (temp > 0)
            cs += THREATENED_BY_MINOR_PIECE_BONUS[pieceToPieceType(board.pieceBoard[squareOfLS1B(&temp)])];
        
        temp = weakEnemies & attacksBy[c][ROOK];
        while (temp > 0)
            cs += THREATENED_BY_ROOK_BONUS[pieceToPieceType(board.pieceBoard[squareOfLS1B(&temp)])];

        if ((weakEnemies & attacksBy[c][KING]) > 0)
            cs += THREATENED_BY_KING_BONUS;

        temp = ~attacksBy[~c][ALL_PIECE_TYPES] | (nonPawnEnemies & attacksBy2[c]);
        cs += HANGING_PIECE_BONUS * populationCount(weakEnemies & temp);
    }

    temp = attacksBy[~c][ALL_PIECE_TYPES] & ~squaresStronglyProtectedByEnemy & attacksBy[c][ALL_PIECE_TYPES];
    cs += PIECE_MOVEMENT_RESTRICTED_BONUS * populationCount(temp);

    safeSquares = ~attacksBy[~c][ALL_PIECE_TYPES] | attacksBy[c][ALL_PIECE_TYPES];
    temp = board.pieces[c == WHITE ? WHITE_PAWN : BLACK_PAWN] & safeSquares;
    temp = allPawnAttacks(temp, c) & nonPawnEnemies;
    cs += THREATENED_BY_SAFE_PAWN_BONUS * populationCount(temp);

    //TODO//
    ////////
    Square sq;
    if (board.pieceCount[c == WHITE ? BLACK_QUEEN : WHITE_QUEEN] == 1) {
        
        sq = board.pieceSquare[c == WHITE ? BLACK_QUEEN : WHITE_QUEEN][0];
        safeSquares = mobilityArea[c] & ~squaresStronglyProtectedByEnemy;

        temp = attacksBy[c][KNIGHT] & knightAttacks[sq];
        cs += KNIGHT_ATTACKING_QUEEN_BONUS * populationCount(temp & safeSquares);

        temp = (attacksBy[c][BISHOP] & bishopAttacks(board.gameBoard, sq)) |
               (attacksBy[c][ROOK  ] & rookAttacks  (board.gameBoard, sq));
        cs += SLIDING_PIECE_ATTACKING_QUEEN_BONUS * populationCount(temp & safeSquares & attacksBy2[c]);

    }
    
    return cs;
}

CombinedScore Evaluation::evaluatePawnShelter(Color c, Square kingSq) {

    CombinedScore cs = makeScore(5, 5);
    ///////////////////////////////////////
    Bitboard pawnsAheadOfKing = (board.pieces[WHITE_PAWN] | board.pieces[BLACK_PAWN]) & ~ranksInFront(~c, kingSq);
    Bitboard thesePawns = pawnsAheadOfKing & board.pieces[c == WHITE ? WHITE_PAWN : BLACK_PAWN];
    Bitboard otherPawns = pawnsAheadOfKing & board.pieces[c == WHITE ? BLACK_PAWN : WHITE_PAWN];
    ///////////////////////////////////////

    Bitboard pawnOnFile;
    int ourFurthestPawnRank, enemyFurthestPawnRank; 
    int kingFile = keepInRange(FILE_B, FILE_G, fileOfSquareFILE(kingSq));
    for (int i = kingFile - 1; i <= kingFile + 1; i++) {

        pawnOnFile = thesePawns & (FILE_A_BB >> i);
        ourFurthestPawnRank = pawnOnFile > 0 ? relativeRank(c, furthestSquare(pawnOnFile, ~c)) : 0;

        pawnOnFile = otherPawns & (FILE_A_BB >> i);
        enemyFurthestPawnRank = pawnOnFile > 0 ? relativeRank(c, furthestSquare(pawnOnFile, ~c)) : 0;

        int arrIndex = std::min(i , FILE_H - i); 
        cs += makeScore(PAWN_SHELTER_BONUS[arrIndex][ourFurthestPawnRank], 0);

        if (ourFurthestPawnRank > 0 && (ourFurthestPawnRank == enemyFurthestPawnRank - 1))
            cs -= BLOCKED_PAWN_STORM_PENALTY * (enemyFurthestPawnRank == RANK_3 ? 1 : 0);
        else 
            cs -= makeScore(UNBLOCKED_PAWN_STORM_PENALTY[arrIndex][enemyFurthestPawnRank], 0);
    }

    return cs;

}

CombinedScore Evaluation::evaluateKingSafety(Color c) {
    
    /////////////////////////////////////
    Square kingSq = board.pieceSquare[c == WHITE ? WHITE_KING : BLACK_KING][0];
    int castlingRights = c == WHITE ? board.castlingRights & 0b0011 : (board.castlingRights & 0b1100) >> 2;
    Bitboard thesePawns = board.pieces[c == WHITE ? WHITE_PAWN : BLACK_PAWN];
    /////////////////////////////////////
    CombinedScore cs = evaluatePawnShelter(c, kingSq);

    if ((castlingRights & 0b0001) > 0)
        cs = std::max(cs, evaluatePawnShelter(c, c == WHITE ? G1 : G8), compareCombinedScores);
    
    if ((castlingRights & 0b0010) > 0)
        cs = std::max(cs, evaluatePawnShelter(c, c == WHITE ? C1 : C8), compareCombinedScores);

    int distanceToPawn = thesePawns > 0 ? 8 : 0;

    if (thesePawns & kingAttacks[kingSq])
        distanceToPawn = 1;
    else
        while(thesePawns > 0 )
            distanceToPawn = std::min(distanceToPawn, ChebyshevDistance[kingSq][squareOfLS1B(&thesePawns)]);

    return cs - makeScore(0, 16 * distanceToPawn);
}

CombinedScore Evaluation::evaluateKing(Color c) {

    CombinedScore cs = evaluateKingSafety(c);
    int kingDanger = 0;
    Bitboard temp;
    /////////////////////////////////
    Square kingSq = board.pieceSquare[c == WHITE ? WHITE_KING : BLACK_KING][0];

    Bitboard theseQueens = board.pieces[c == WHITE ? WHITE_QUEEN : BLACK_QUEEN];
    /////////////////////////////////
    Bitboard weakSquares, enemySafeSquares, bishopAttacksXRaysQueen, rookAttacksXRaysQueen, enemyKnightChecks, enemyBishopChecks, enemyRookChecks,
             enemyQueenChecks;
    Bitboard enemyUnsafeChecks = 0;

    weakSquares = attacksBy[~c][ALL_PIECE_TYPES] & ~attacksBy2[c] & (~attacksBy[c][ALL_PIECE_TYPES] | attacksBy[c][QUEEN] | attacksBy[c][KING]);
    enemySafeSquares = ~board.piecesOnSide[~c] & (~attacksBy[c][ALL_PIECE_TYPES] | (weakSquares & attacksBy2[~c]));

    bishopAttacksXRaysQueen = bishopAttacks(board.gameBoard ^ theseQueens, kingSq);
    rookAttacksXRaysQueen   = rookAttacks  (board.gameBoard ^ theseQueens, kingSq);

    enemyRookChecks = rookAttacksXRaysQueen & enemySafeSquares & attacksBy[~c][ROOK];
    if (enemyRookChecks > 0) kingDanger += SAFE_ROOK_CHECK_PENALTY;
    else 
        enemyUnsafeChecks |= rookAttacksXRaysQueen & attacksBy[~c][ROOK];

    enemyQueenChecks = (bishopAttacksXRaysQueen | rookAttacksXRaysQueen) & attacksBy[~c][QUEEN] & enemySafeSquares & ~attacksBy[c][QUEEN] & ~enemyRookChecks;
    if (enemyQueenChecks > 0) kingDanger += SAFE_QUEEN_CHECK_PENALTY;

    enemyBishopChecks = bishopAttacksXRaysQueen & enemySafeSquares & attacksBy[~c][BISHOP] & ~enemyQueenChecks;
    if (enemyBishopChecks > 0) kingDanger += SAFE_BISHOP_CHECK_PENALTY;
    else enemyUnsafeChecks |= bishopAttacksXRaysQueen & attacksBy[~c][BISHOP];

    enemyKnightChecks = knightAttacks[kingSq] & attacksBy[~c][KNIGHT];
    if ((enemyKnightChecks & enemySafeSquares) > 0) kingDanger += SAFE_KNIGHT_CHECK_PENALTY;
    else enemyUnsafeChecks |= enemyKnightChecks;

    kingDanger +=       attackingEnemyKingRingCount[~c] * attackingEnemyKingRingPieceWeight[~c]
                + 185 * populationCount(kingRing[c] & weakSquares) 
                + 148 * populationCount(enemyUnsafeChecks) 
                + 98  * populationCount(board.blockers(kingSq, board.piecesOnSide[~c], temp))
                + 69  * attackedSquaresAroundEnemyKing[~c] 
                +       middlegameScore(totalMobility[~c] - totalMobility[c]) 
                - 873 * (board.pieceCount[c == WHITE ? BLACK_QUEEN : WHITE_QUEEN] == 0) ? 1 : 0  
                - 100 * (attacksBy[c][KNIGHT] & attacksBy[c][KING] > 0) ? 1 : 0
                - 6   * middlegameScore(cs) / 8
                + 37;

    if (kingDanger > 100) cs -= makeScore(kingDanger * kingDanger / 4096, kingDanger / 16);
    return cs;
}

// A second-degree polynomial material imbalance function by Tord Romstad
CombinedScore Evaluation::evaluateImbalance() {

    int pieceCount[COLOURS][ALL_PIECE_TYPES] = {
        {
            board.pieceCount[WHITE_BISHOP] > 1 ? 1 : 0, board.pieceCount[WHITE_PAWN], board.pieceCount[WHITE_KNIGHT],
            board.pieceCount[WHITE_BISHOP]            , board.pieceCount[WHITE_ROOK], board.pieceCount[WHITE_QUEEN ]
        },
        {
            board.pieceCount[BLACK_BISHOP] > 1 ? 1 : 0, board.pieceCount[BLACK_PAWN], board.pieceCount[BLACK_KNIGHT],
            board.pieceCount[BLACK_BISHOP]            , board.pieceCount[BLACK_ROOK], board.pieceCount[BLACK_QUEEN ]
        }
    };

    int score = (evaluateImbalanceHelper(WHITE, pieceCount) - evaluateImbalanceHelper(BLACK, pieceCount)) / 16;
    return makeScore(score, score);
}

CombinedScore Evaluation::evaluateInitiative(CombinedScore cs) {

    /////////////////////////////
    Square whiteKingSq = board.pieceSquare[WHITE_KING][0];
    Square blackKingSq = board.pieceSquare[BLACK_KING][0];

    Bitboard pawns = board.pieces[WHITE_PAWN] | board.pieces[BLACK_PAWN];
    /////////////////////////////

    int kingFlanking = std::abs(fileOfSquareFILE(whiteKingSq) - fileOfSquareFILE(blackKingSq)) 
                     - std::abs(rankOfSquareRANK(whiteKingSq) - rankOfSquareRANK(blackKingSq));
    
    bool kingInfiltrating = rankOfSquareRANK(whiteKingSq) > RANK_4
                         || rankOfSquareRANK(blackKingSq) < RANK_5;

    bool pawnsOnBothSides = ((pawns & QUEEN_SIDE) > 0) && ((pawns & KING_SIDE) > 0);

    bool almostUnwinnable = ((passedPawns[WHITE] | passedPawns[BLACK]) == 0) && kingFlanking < 0 && !pawnsOnBothSides;

    int initiativeBonus = 9  * populationCount(passedPawns[WHITE] | passedPawns[BLACK])
                        + 11 * (board.pieceCount[WHITE_PAWN] + board.pieceCount[BLACK_PAWN])
                        + 9  * (kingFlanking ? 1 : 0)
                        + 12 * (kingInfiltrating ? 1 : 0)
                        + 21 * (pawnsOnBothSides ? 1 : 0)
                        + 51 * ((board.nonPawnMaterial == 0) ? 1 : 0)
                        - 43 * (almostUnwinnable ? 1 : 0)
                        - 100;

    ExactScore middleGameScore = middlegameScore(cs);
    ExactScore endGameScore    = endgameScore(cs);

    int middlegameScoreBonus = ((middleGameScore > 0) - (middleGameScore < 0)) * std::max(std::min(initiativeBonus + 50, 0), -abs(middleGameScore));
    int endgameScoreBonus    = ((endGameScore    > 0) - (endGameScore    < 0)) * std::max(initiativeBonus, -abs(endGameScore));

    return makeScore(middlegameScoreBonus, endgameScoreBonus);
}

ExactScore Evaluation::evaluateImbalanceHelper(Color c, int pieceCount[COLOURS][ALL_PIECE_TYPES]) {

    ExactScore bonus = 0;

    for (int pt1 = PAWN; pt1 < ALL_PIECE_TYPES; pt1++) {

        if (pieceCount[c][pt1] == 0) continue;

        int value = 0;

        for (int pt2 = PAWN; pt2 <= pt1; pt2++) {
            value += QUADRATIC_PARAMETERS_OURS  [pt1][pt2] * pieceCount[ c][pt2]
                   + QUADRATIC_PARAMETERS_THEIRS[pt1][pt2] * pieceCount[~c][pt2];
        }
        bonus += pieceCount[c][pt1] * value;
    }

    return bonus;

}

ExactScore Evaluation::evaluatePosition() {

    ExactScore evaluation;
    CombinedScore cs = board.pieceSquareScore + evaluateImbalance();

    cs += evaluatePawns(WHITE) - evaluatePawns(BLACK);

    initEvaluation();

    cs += evaluatePiece(KNIGHT, WHITE) - evaluatePiece(KNIGHT, BLACK);
    cs += evaluatePiece(BISHOP, WHITE) - evaluatePiece(BISHOP, BLACK);
    cs += evaluatePiece(ROOK  , WHITE) - evaluatePiece(ROOK  , BLACK);
    cs += evaluatePiece(QUEEN , WHITE) - evaluatePiece(QUEEN , BLACK);
    
    cs += totalMobility[WHITE] - totalMobility[BLACK];

    cs += evaluateKing(WHITE) - evaluateKing(BLACK);
    cs += evaluateThreats(WHITE) - evaluateThreats(BLACK);
    cs += evaluatePassedPawns(WHITE) - evaluatePassedPawns(BLACK);
    cs += evaluateSpace(WHITE) - evaluateSpace(BLACK);

    cs += evaluateInitiative(cs);
    
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

=======

#include <algorithm>
#include <cstdlib>
#include "evaluation.hpp"
#include "utility.hpp"
#include "board.hpp"
#include "moves.hpp"

CombinedScore pieceSquareTable[PIECES][NUMBER_OF_SQUARES];

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

    Bitboard temp;
    Bitboard whiteKingBlockers = board.blockers(whiteKingSq, board.piecesOnSide[BLACK], temp);
    Bitboard blackKingBlockers = board.blockers(blackKingSq, board.piecesOnSide[WHITE], temp);

    Bitboard whitePawnAttacks = allPawnAttacks(whitePawns, WHITE);
    Bitboard blackPawnAttacks = allPawnAttacks(blackPawns, BLACK);

    Bitboard squaresAttackedTwiceByPawnsWHITE = attackedTwiceByPawns(whitePawns, WHITE);
    Bitboard squaresAttackedTwiceByPawnsBLACK = attackedTwiceByPawns(blackPawns, BLACK);
    ////////////////////////////////////////////////////////////

    //Initialize mobility area /////////////////////////////////
    mobilityArea[WHITE] = ~(whiteBadPawns | board.pieces[WHITE_KING] | board.pieces[WHITE_QUEEN] | whiteKingBlockers | blackPawnAttacks);
    mobilityArea[BLACK] = ~(blackBadPawns | board.pieces[BLACK_KING] | board.pieces[BLACK_QUEEN] | blackKingBlockers | whitePawnAttacks);

    totalMobility[BLACK] = totalMobility[WHITE] = 0;
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

    //Initialize king safety /////////////////////////////////////
    Bitboard whiteKingSq2BB = FILE_A_BB >> keepInRange(FILE_B, FILE_G, fileOfSquareFILE(whiteKingSq)) &
                              RANK_1_BB >> keepInRange(RANK_2, RANK_7, rankOfSquareRANK(whiteKingSq)) * NORTH;
    Bitboard blackKingSq2BB = FILE_A_BB >> keepInRange(FILE_B, FILE_G, fileOfSquareFILE(blackKingSq)) &
                              RANK_1_BB >> keepInRange(RANK_2, RANK_7, rankOfSquareRANK(blackKingSq)) * NORTH;
    Square whiteKingSq2 = squareOfLS1B(whiteKingSq2BB);
    Square blackKingSq2 = squareOfLS1B(blackKingSq2BB);

    kingRing[WHITE] = kingAttacks[whiteKingSq2] | whiteKingSq2BB;
    kingRing[BLACK] = kingAttacks[blackKingSq2] | blackKingSq2BB;

    attackingEnemyKingRingCount[WHITE] = populationCount(kingRing[BLACK] & whitePawnAttacks);
    attackingEnemyKingRingCount[BLACK] = populationCount(kingRing[WHITE] & blackPawnAttacks);

    kingRing[WHITE] &= ~squaresAttackedTwiceByPawnsWHITE;
    kingRing[BLACK] &= ~squaresAttackedTwiceByPawnsBLACK;

    attackedSquaresAroundEnemyKing[WHITE] = attackingEnemyKingRingPieceWeight[WHITE] = 0;
    attackedSquaresAroundEnemyKing[BLACK] = attackingEnemyKingRingPieceWeight[BLACK] = 0;
    //////////////////////////////////////////////////////////////
    
}

CombinedScore Evaluation::evaluatePiece(PieceType pt, Color c) {

    Bitboard temp;
    /////////////////////////////////////
    Square kingSq = board.pieceSquare[WHITE_KING + (6 * c)][0];

    Bitboard thesePawns   = board.pieces[c == WHITE ? WHITE_PAWN : BLACK_PAWN];
    Bitboard otherPawns   = board.pieces[c == WHITE ? BLACK_PAWN : WHITE_PAWN];
    Bitboard otherBishops = board.pieces[c == WHITE ? BLACK_BISHOP : WHITE_BISHOP];
    Bitboard otherRooks   = board.pieces[c == WHITE ? BLACK_ROOK : WHITE_ROOK];

    Bitboard queens = board.pieces[WHITE_QUEEN] | board.pieces[BLACK_QUEEN];
    Bitboard kingBlockers = board.blockers(kingSq, board.piecesOnSide[~c], temp);
    /////////////////////////////////////

    int pieceIndex     = pt + (6 * c);
    int numberOfPieces = board.pieceCount[pieceIndex];

    Bitboard attacks, sqBB;
    Square sq;
    CombinedScore cs = 0;

    attacksBy[c][pt] = 0;
    
    for (int i = 0; i < numberOfPieces; i++) {

        sq   = board.pieceSquare[pieceIndex][i];
        sqBB = squareToBitboard(sq);

        //Includes x-ray attacks for sliders
        attacks = (pt == KNIGHT) ? knightAttacks[sq] 
                : (pt == BISHOP) ? bishopAttacks(board.gameBoard ^ queens, sq)
                : (pt == ROOK  ) ? rookAttacks(board.gameBoard ^ queens ^ board.pieces[pieceIndex], sq)
                : bishopAttacks(board.gameBoard, sq) | rookAttacks(board.gameBoard, sq);

        // If piece is pinned to the king, then can only move along the ray of the incoming attack
        if (kingBlockers & sq)
            attacks &= rayLine[kingSq][sq];

        attacksBy2[c] |= (attacksBy[c][ALL_PIECE_TYPES] & attacks);
        attacksBy[c][pt] |= attacks;
        attacksBy[c][ALL_PIECE_TYPES] |= attacks;

        if ((attacks & kingRing[~c]) > 0) {

            attackingEnemyKingRingCount[c]++;
            attackedSquaresAroundEnemyKing[c] += populationCount(attacks & attacksBy[~c][KING]);
            attackingEnemyKingRingPieceWeight[c] += KING_ATTACKERS_WEIGHT[pt];
        }

        int mobility = populationCount(attacks & mobilityArea[c]);
        totalMobility[c] += MOBILITY_BONUS[pt][mobility]; 

        if ((pt == KNIGHT) || (pt == BISHOP)) {
            
            
            Bitboard outposts = attacksBy[c][PAWN] & (c == WHITE) ? whiteOutpostRanks & (~pawnAttackSpans(board.pieces[BLACK_PAWN], BLACK))
                                                                  : blackOutpostRanks & (~pawnAttackSpans(board.pieces[WHITE_PAWN], WHITE));

            if ((outposts & sqBB) > 0) cs += (OUTPOST_BONUS * (pt == KNIGHT) ? 2 : 1);
            else if ((pt == KNIGHT) && ((outposts & attacks & (~board.piecesOnSide[c])) > 0)) cs += REACHABLE_OUTPOST_BONUS;

            //Uses pawnsAbleToPush as a hack for detecting minor pieces behind pawns
            if (pawnsAbleToPush(sqBB, board.pieces[WHITE_PAWN] | board.pieces[BLACK_PAWN], c) > 0) cs += MINOR_PIECE_BEHIND_PAWN_BONUS;

            cs -= (KING_PROTECTOR_PENALTY * ChebyshevDistance[sq][kingSq]);
            /*
            if (pt == BISHOP) {

                Bitboard blockedPawns = pawnsAbleToPush(thesePawns, board.gameBoard, c);
                cs -= PAWNS_BLOCKING_BISHOP_PENALTY * (populationCount(blockedPawns & CENTER_FILES) + 1) * 
                      populationCount(thesePawns & ((sqBB & DARK_SQUARES) > 0) ? DARK_SQUARES : LIGHT_SQUARES);

                if (hasMoreThanOneBit(bishopAttacks(thesePawns | otherPawns, sq) & CENTER_SQUARES)) 
                    cs += BISHOP_CONTROL_OF_CENTER_BONUS;
            } */
        } else if (pt == ROOK) {

            Bitboard fileSqBB = fileOfSquareBB(sq);
            if ((fileSqBB & queens) > 0) cs += ROOK_ON_QUEEN_FILE_BONUS;

            if ((fileSqBB & thesePawns) == 0) cs += ROOK_ON_SEMI_OR_OPEN_FILE_BONUS[(fileSqBB & otherPawns) == 0 ? 1 : 0];
            else if (mobility <= 3) {

                File kingFile = fileOfSquareFILE(kingSq);
                int castlingRights = board.castlingRights & (c == WHITE ? 0b0011 : 0b1100); 
                if ((kingFile < FILE_E) == (fileOfSquareFILE(sq) < kingFile))
                    cs -= ROOK_TRAPPED_PENALTY * (castlingRights == 0 ? 2 : 1);

            }
        } else { // Then should be a Queen
            if (board.blockers(sq, otherBishops | otherRooks, temp) > 0) cs -= QUEEN_IS_WEAK_PENALTY;
        }
    }

    return cs;
}

CombinedScore Evaluation::evaluatePawns(Color c) {

    CombinedScore cs = 0;
    passedPawns[c]   = 0;
    /////////////////////////////////////////
    Direction pawnPush = (c == WHITE) ? NORTH : SOUTH;

    Bitboard thesePawns = board.pieces[c == WHITE ? WHITE_PAWN : BLACK_PAWN];
    Bitboard otherPawns = board.pieces[c == WHITE ? BLACK_PAWN : WHITE_PAWN];
    /////////////////////////////////////////

    int pawnIndex     = WHITE_PAWN + (6 * c);
    int numberOfPawns = board.pieceCount[pawnIndex];

    Square sq;
    Bitboard sqToBB;
    Bitboard opposing, blocking, stopping, levers, pawnPushLevers, adjacent, sideBySide, supporting;
    bool doubled, backwards, passed;

    for (int i = 0; i < numberOfPawns; i++) {

        sq     = board.pieceSquare[pawnIndex][i];
        sqToBB = squareToBitboard(sq);

        Rank relativeRANK = relativeRank(c, sq);
        //////////////////////////////
        opposing       = otherPawns & pawnFrontSpans(sqToBB, c);
        blocking       = otherPawns & squareToBitboard(Square(sq + pawnPush));
        stopping       = otherPawns & (pawnFrontSpans(sqToBB, c) | pawnAttackSpans(sqToBB, c));
        levers         = otherPawns & pawnAttacks[c][sq];
        pawnPushLevers = otherPawns & pawnAttacks[c][sq + pawnPush];
        doubled        = (thesePawns & squareToBitboard(Square(sq - pawnPush))) > 0;
        adjacent       = thesePawns & adjacentFiles(sq);
        sideBySide     = adjacent   & rankOfSquareBB(sq);
        supporting     = adjacent   & rankOfSquareBB(Square(sq - pawnPush));
        backwards      = ((adjacent & ranksInFront(~c, Square(sq + pawnPush))) == 0) && ((pawnPushLevers > 0) || (blocking > 0));
        passed         = ((stopping ^ levers) == 0) 
                      || ((stopping ^ pawnPushLevers) && (populationCount(sideBySide) >= populationCount(pawnPushLevers)))
                      || ((stopping == blocking) && (relativeRANK >= RANK_5) 
                      && (pawnsAbleToPush(supporting, otherPawns | attackedTwiceByPawns(otherPawns, ~c), c) == 0)); //Uses pawnsAbleToPush as a hack
        //////////////////////////////

        if (passed) passedPawns[c] |= squareToBitboard(sq);

        if ((supporting > 0) || (sideBySide > 0)) {

            int sideBySideBonus = (sideBySide > 0) ? 1 : 0;
            int opposedPenalty  = (opposing   > 0) ? 1 : 0;

            ExactScore score = (CONNECTED_PAWN_BONUS[relativeRANK] * (2 + sideBySideBonus - opposedPenalty))
                             + (21 * populationCount(supporting));

            cs += makeScore(score, (score * (relativeRANK - 2)) / 4);

        } 
        else if (adjacent == 0) cs -= ISOLATED_PAWN_PENALTY + (opposing == 0) ? OPEN_TO_ATTACK_PENALTY : 0;
        else if (backwards    ) cs -= BACKWARD_PAWN_PENALTY + (opposing == 0) ? OPEN_TO_ATTACK_PENALTY : 0; 

        if (supporting == 0) cs -= (doubled                    ) ? DOUBLED_PAWN_PENALTY : 0 
                                +  ((levers & (levers - 1)) > 0) ? WEAK_LEVER_PENALTY   : 0;
    }

    return cs;

}

CombinedScore Evaluation::evaluatePassedPawns(Color c) {

    /////////////////////////////
    Square thisKing  = board.pieceSquare[c == WHITE ? WHITE_KING : BLACK_KING][0];
    Square otherKing = board.pieceSquare[c == WHITE ? BLACK_KING : WHITE_KING][0];

    Bitboard thesePawns        = board.pieces[c == WHITE ? WHITE_PAWN : BLACK_PAWN];
    Bitboard otherPawns       = board.pieces[c == WHITE ? BLACK_PAWN : WHITE_PAWN];
    Bitboard thisRooksQueens  = board.pieces[c == WHITE ? WHITE_ROOK : BLACK_ROOK] | board.pieces[c == WHITE ? WHITE_QUEEN : BLACK_QUEEN];
    Bitboard otherRooksQueens = board.pieces[c == WHITE ? BLACK_ROOK : WHITE_ROOK] | board.pieces[c == WHITE ? BLACK_QUEEN : WHITE_QUEEN];
    /////////////////////////////

    CombinedScore cs, cs2;
    cs = 0;

    Direction pawnPush = (c == WHITE) ? NORTH : SOUTH;

    Bitboard pp = passedPawns[c];
    while (pp > 0) {

        Square sq       = squareOfLS1B(&pp);
        Bitboard sqToBB = squareToBitboard(sq);
         
        Rank relativeRANK = relativeRank(c, sq);
        cs2 = PASSED_PAWN_BONUS[relativeRANK];

        Square blockingSq = Square(sq + pawnPush);
        Bitboard blockingSqToBB = squareToBitboard(blockingSq);

        if (relativeRANK > RANK_3) {

            int weight = (relativeRANK * 5) - 13;
            
            int thisKingToBlockingSq  = std::min(ChebyshevDistance[thisKing ][blockingSq], 5);
            int otherKingToBlockingSq = std::min(ChebyshevDistance[otherKing][blockingSq], 5);

            cs2 += makeScore(0, (((otherKingToBlockingSq * 19) / 4) - (thisKingToBlockingSq * 2)) * weight);

            if (relativeRANK != RANK_7) 
                cs2 -= makeScore(0, std::min(ChebyshevDistance[thisKing][blockingSq + pawnPush], 5) * weight);

            if ((blockingSqToBB & board.emptySquares) > 0) {

                Bitboard squaresLeftToQueen = pawnFrontSpans(sqToBB, c);
                Bitboard unsafeSquares      = pawnFrontSpans(sqToBB, c) | pawnAttackSpans(sqToBB, c);

                Bitboard behindPassedPawnRooksQueens = pawnFrontSpans(sqToBB, ~c) & (thisRooksQueens| otherRooksQueens);

                //Tarrasch Rule: Rooks should be placed behind passed pawns.
                //If enemy rook/queen behind passed pawn, then all push squares are unsafe.
                //Otherwise check for attacks on the pawn advance by enemy
                if ((behindPassedPawnRooksQueens & board.piecesOnSide[~c]) == 0)
                    unsafeSquares &= attacksBy[~c][ALL_PIECE_TYPES];

                int bonus =  unsafeSquares                       == 0 ? 35
                          : (unsafeSquares & squaresLeftToQueen) == 0 ? 20
                          : (unsafeSquares & blockingSqToBB)     == 0 ? 9 : 0;

                if (((behindPassedPawnRooksQueens & board.piecesOnSide[c]) > 0) || ((attacksBy[c][ALL_PIECE_TYPES] & blockingSqToBB) > 0))
                    bonus += 5;

                cs2 += makeScore(bonus * weight, bonus * weight);
            }

        }

        if ((((thesePawns | otherPawns) & blockingSqToBB) > 0) 
           || (((pawnFrontSpans(blockingSqToBB, c) | pawnAttackSpans(blockingSqToBB, c)) & otherPawns) > 0))
            cs2 = makeScore(middlegameScore(cs2) / 2, endgameScore(cs2) / 2);

        int file = fileOfSquareFILE(sq);
        cs += cs2 - (INSIDE_PASSED_PAWN_PENALTY * std::min(file, FILE_H - file));
    }

    return cs;

}

CombinedScore Evaluation::evaluateSpace(Color c) {

    ////////////////////////////////
    Bitboard thesePawns = c == WHITE ? WHITE_PAWN : BLACK_PAWN;
    int thesePieceCount = c == WHITE ? board.pieceCount[WHITE_PAWN] + board.pieceCount[WHITE_KNIGHT] + board.pieceCount[WHITE_BISHOP] +
                                       board.pieceCount[WHITE_ROOK] + board.pieceCount[WHITE_QUEEN]
                                     : board.pieceCount[BLACK_PAWN] + board.pieceCount[BLACK_KNIGHT] + board.pieceCount[BLACK_BISHOP] +
                                       board.pieceCount[BLACK_ROOK] + board.pieceCount[BLACK_QUEEN];
    ////////////////////////////////

    if (board.nonPawnMaterial[WHITE] + board.nonPawnMaterial[BLACK] < SPACE_THRESHOLD) return 0;

    Bitboard centerSpace = c == WHITE ? CENTER_FILES & (RANK_2_BB | RANK_3_BB | RANK_4_BB)
                                      : CENTER_FILES & (RANK_7_BB | RANK_6_BB | RANK_5_BB);
    Bitboard safeSquares = centerSpace & ~board.pieces[thesePawns] & ~attacksBy[~c][PAWN];
    Bitboard spaceBehindPawns = board.pieces[thesePawns];
    spaceBehindPawns |= c == WHITE ? spaceBehindPawns >> -SOUTH : spaceBehindPawns << NORTH;
    spaceBehindPawns |= c == WHITE ? spaceBehindPawns >> -(SOUTH + SOUTH) : spaceBehindPawns << NORTH + NORTH;

    int bonus = populationCount(safeSquares) + populationCount(spaceBehindPawns & safeSquares & ~attacksBy[~c][ALL_PIECE_TYPES]);
    return makeScore(bonus * thesePieceCount * thesePieceCount / 16, 0);
}

CombinedScore Evaluation::evaluateThreats(Color c) {

    Bitboard nonPawnEnemies, squaresStronglyProtectedByEnemy, defendedEnemies, weakEnemies, safeSquares;

    nonPawnEnemies = board.piecesOnSide[~c] & ~board.pieces[c == WHITE ? BLACK_PAWN : WHITE_PAWN];
    squaresStronglyProtectedByEnemy = attacksBy[~c][PAWN] | (attacksBy2[~c] & ~attacksBy2[c]);
    defendedEnemies = nonPawnEnemies & squaresStronglyProtectedByEnemy;
    weakEnemies = board.piecesOnSide[~c] & ~squaresStronglyProtectedByEnemy & attacksBy[c][ALL_PIECE_TYPES];

    CombinedScore cs = 0;
    Bitboard temp;
    if (defendedEnemies > 0 || weakEnemies > 0) {

        temp = (defendedEnemies | weakEnemies) & (attacksBy[c][KNIGHT] | attacksBy[c][BISHOP]);
        while (temp > 0)
            cs += THREATENED_BY_MINOR_PIECE_BONUS[pieceToPieceType(board.pieceBoard[squareOfLS1B(&temp)])];
        
        temp = weakEnemies & attacksBy[c][ROOK];
        while (temp > 0)
            cs += THREATENED_BY_ROOK_BONUS[pieceToPieceType(board.pieceBoard[squareOfLS1B(&temp)])];

        if ((weakEnemies & attacksBy[c][KING]) > 0)
            cs += THREATENED_BY_KING_BONUS;

        temp = ~attacksBy[~c][ALL_PIECE_TYPES] | (nonPawnEnemies & attacksBy2[c]);
        cs += HANGING_PIECE_BONUS * populationCount(weakEnemies & temp);
    }

    temp = attacksBy[~c][ALL_PIECE_TYPES] & ~squaresStronglyProtectedByEnemy & attacksBy[c][ALL_PIECE_TYPES];
    cs += PIECE_MOVEMENT_RESTRICTED_BONUS * populationCount(temp);

    safeSquares = ~attacksBy[~c][ALL_PIECE_TYPES] | attacksBy[c][ALL_PIECE_TYPES];
    temp = board.pieces[c == WHITE ? WHITE_PAWN : BLACK_PAWN] & safeSquares;
    temp = allPawnAttacks(temp, c) & nonPawnEnemies;
    cs += THREATENED_BY_SAFE_PAWN_BONUS * populationCount(temp);

    //TODO//
    ////////
    Square sq;
    if (board.pieceCount[c == WHITE ? BLACK_QUEEN : WHITE_QUEEN] == 1) {
        
        sq = board.pieceSquare[c == WHITE ? BLACK_QUEEN : WHITE_QUEEN][0];
        safeSquares = mobilityArea[c] & ~squaresStronglyProtectedByEnemy;

        temp = attacksBy[c][KNIGHT] & knightAttacks[sq];
        cs += KNIGHT_ATTACKING_QUEEN_BONUS * populationCount(temp & safeSquares);

        temp = (attacksBy[c][BISHOP] & bishopAttacks(board.gameBoard, sq)) |
               (attacksBy[c][ROOK  ] & rookAttacks  (board.gameBoard, sq));
        cs += SLIDING_PIECE_ATTACKING_QUEEN_BONUS * populationCount(temp & safeSquares & attacksBy2[c]);

    }
    
    return cs;
}

CombinedScore Evaluation::evaluatePawnShelter(Color c, Square kingSq) {

    CombinedScore cs = makeScore(5, 5);
    ///////////////////////////////////////
    Bitboard pawnsAheadOfKing = (board.pieces[WHITE_PAWN] | board.pieces[BLACK_PAWN]) & ~ranksInFront(~c, kingSq);
    Bitboard thesePawns = pawnsAheadOfKing & board.pieces[c == WHITE ? WHITE_PAWN : BLACK_PAWN];
    Bitboard otherPawns = pawnsAheadOfKing & board.pieces[c == WHITE ? BLACK_PAWN : WHITE_PAWN];
    ///////////////////////////////////////

    Bitboard pawnOnFile;
    int ourFurthestPawnRank, enemyFurthestPawnRank; 
    int kingFile = keepInRange(FILE_B, FILE_G, fileOfSquareFILE(kingSq));
    for (int i = kingFile - 1; i <= kingFile + 1; i++) {

        pawnOnFile = thesePawns & (FILE_A_BB >> i);
        ourFurthestPawnRank = pawnOnFile > 0 ? relativeRank(c, furthestSquare(pawnOnFile, ~c)) : 0;

        pawnOnFile = otherPawns & (FILE_A_BB >> i);
        enemyFurthestPawnRank = pawnOnFile > 0 ? relativeRank(c, furthestSquare(pawnOnFile, ~c)) : 0;

        int arrIndex = std::min(i , FILE_H - i); 
        cs += makeScore(PAWN_SHELTER_BONUS[arrIndex][ourFurthestPawnRank], 0);

        if (ourFurthestPawnRank > 0 && (ourFurthestPawnRank == enemyFurthestPawnRank - 1))
            cs -= BLOCKED_PAWN_STORM_PENALTY * (enemyFurthestPawnRank == RANK_3 ? 1 : 0);
        else 
            cs -= makeScore(UNBLOCKED_PAWN_STORM_PENALTY[arrIndex][enemyFurthestPawnRank], 0);
    }

    return cs;

}

CombinedScore Evaluation::evaluateKingSafety(Color c) {
    
    /////////////////////////////////////
    Square kingSq = board.pieceSquare[c == WHITE ? WHITE_KING : BLACK_KING][0];
    int castlingRights = c == WHITE ? board.castlingRights & 0b0011 : (board.castlingRights & 0b1100) >> 2;
    Bitboard thesePawns = board.pieces[c == WHITE ? WHITE_PAWN : BLACK_PAWN];
    /////////////////////////////////////
    CombinedScore cs = evaluatePawnShelter(c, kingSq);

    if ((castlingRights & 0b0001) > 0)
        cs = std::max(cs, evaluatePawnShelter(c, c == WHITE ? G1 : G8), compareCombinedScores);
    
    if ((castlingRights & 0b0010) > 0)
        cs = std::max(cs, evaluatePawnShelter(c, c == WHITE ? C1 : C8), compareCombinedScores);

    int distanceToPawn = thesePawns > 0 ? 8 : 0;

    if (thesePawns & kingAttacks[kingSq])
        distanceToPawn = 1;
    else
        while(thesePawns > 0 )
            distanceToPawn = std::min(distanceToPawn, ChebyshevDistance[kingSq][squareOfLS1B(&thesePawns)]);

    return cs - makeScore(0, 16 * distanceToPawn);
}

CombinedScore Evaluation::evaluateKing(Color c) {

    CombinedScore cs = evaluateKingSafety(c);
    int kingDanger = 0;
    Bitboard temp;
    /////////////////////////////////
    Square kingSq = board.pieceSquare[c == WHITE ? WHITE_KING : BLACK_KING][0];

    Bitboard theseQueens = board.pieces[c == WHITE ? WHITE_QUEEN : BLACK_QUEEN];
    /////////////////////////////////
    Bitboard weakSquares, enemySafeSquares, bishopAttacksXRaysQueen, rookAttacksXRaysQueen, enemyKnightChecks, enemyBishopChecks, enemyRookChecks,
             enemyQueenChecks;
    Bitboard enemyUnsafeChecks = 0;

    weakSquares = attacksBy[~c][ALL_PIECE_TYPES] & ~attacksBy2[c] & (~attacksBy[c][ALL_PIECE_TYPES] | attacksBy[c][QUEEN] | attacksBy[c][KING]);
    enemySafeSquares = ~board.piecesOnSide[~c] & (~attacksBy[c][ALL_PIECE_TYPES] | (weakSquares & attacksBy2[~c]));

    bishopAttacksXRaysQueen = bishopAttacks(board.gameBoard ^ theseQueens, kingSq);
    rookAttacksXRaysQueen   = rookAttacks  (board.gameBoard ^ theseQueens, kingSq);

    enemyRookChecks = rookAttacksXRaysQueen & enemySafeSquares & attacksBy[~c][ROOK];
    if (enemyRookChecks > 0) kingDanger += SAFE_ROOK_CHECK_PENALTY;
    else 
        enemyUnsafeChecks |= rookAttacksXRaysQueen & attacksBy[~c][ROOK];

    enemyQueenChecks = (bishopAttacksXRaysQueen | rookAttacksXRaysQueen) & attacksBy[~c][QUEEN] & enemySafeSquares & ~attacksBy[c][QUEEN] & ~enemyRookChecks;
    if (enemyQueenChecks > 0) kingDanger += SAFE_QUEEN_CHECK_PENALTY;

    enemyBishopChecks = bishopAttacksXRaysQueen & enemySafeSquares & attacksBy[~c][BISHOP] & ~enemyQueenChecks;
    if (enemyBishopChecks > 0) kingDanger += SAFE_BISHOP_CHECK_PENALTY;
    else enemyUnsafeChecks |= bishopAttacksXRaysQueen & attacksBy[~c][BISHOP];

    enemyKnightChecks = knightAttacks[kingSq] & attacksBy[~c][KNIGHT];
    if ((enemyKnightChecks & enemySafeSquares) > 0) kingDanger += SAFE_KNIGHT_CHECK_PENALTY;
    else enemyUnsafeChecks |= enemyKnightChecks;

    kingDanger +=       attackingEnemyKingRingCount[~c] * attackingEnemyKingRingPieceWeight[~c]
                + 185 * populationCount(kingRing[c] & weakSquares) 
                + 148 * populationCount(enemyUnsafeChecks) 
                + 98  * populationCount(board.blockers(kingSq, board.piecesOnSide[~c], temp))
                + 69  * attackedSquaresAroundEnemyKing[~c] 
                +       middlegameScore(totalMobility[~c] - totalMobility[c]) 
                - 873 * (board.pieceCount[c == WHITE ? BLACK_QUEEN : WHITE_QUEEN] == 0) ? 1 : 0  
                - 100 * (attacksBy[c][KNIGHT] & attacksBy[c][KING] > 0) ? 1 : 0
                - 6   * middlegameScore(cs) / 8
                + 37;

    if (kingDanger > 100) cs -= makeScore(kingDanger * kingDanger / 4096, kingDanger / 16);
    return cs;
}

// A second-degree polynomial material imbalance function by Tord Romstad
CombinedScore Evaluation::evaluateImbalance() {

    int pieceCount[COLOURS][ALL_PIECE_TYPES] = {
        {
            board.pieceCount[WHITE_BISHOP] > 1 ? 1 : 0, board.pieceCount[WHITE_PAWN], board.pieceCount[WHITE_KNIGHT],
            board.pieceCount[WHITE_BISHOP]            , board.pieceCount[WHITE_ROOK], board.pieceCount[WHITE_QUEEN ]
        },
        {
            board.pieceCount[BLACK_BISHOP] > 1 ? 1 : 0, board.pieceCount[BLACK_PAWN], board.pieceCount[BLACK_KNIGHT],
            board.pieceCount[BLACK_BISHOP]            , board.pieceCount[BLACK_ROOK], board.pieceCount[BLACK_QUEEN ]
        }
    };

    int score = (evaluateImbalanceHelper(WHITE, pieceCount) - evaluateImbalanceHelper(BLACK, pieceCount)) / 16;
    return makeScore(score, score);
}

CombinedScore Evaluation::evaluateInitiative(CombinedScore cs) {

    /////////////////////////////
    Square whiteKingSq = board.pieceSquare[WHITE_KING][0];
    Square blackKingSq = board.pieceSquare[BLACK_KING][0];

    Bitboard pawns = board.pieces[WHITE_PAWN] | board.pieces[BLACK_PAWN];
    /////////////////////////////

    int kingFlanking = std::abs(fileOfSquareFILE(whiteKingSq) - fileOfSquareFILE(blackKingSq)) 
                     - std::abs(rankOfSquareRANK(whiteKingSq) - rankOfSquareRANK(blackKingSq));
    
    bool kingInfiltrating = rankOfSquareRANK(whiteKingSq) > RANK_4
                         || rankOfSquareRANK(blackKingSq) < RANK_5;

    bool pawnsOnBothSides = ((pawns & QUEEN_SIDE) > 0) && ((pawns & KING_SIDE) > 0);

    bool almostUnwinnable = ((passedPawns[WHITE] | passedPawns[BLACK]) == 0) && kingFlanking < 0 && !pawnsOnBothSides;

    int initiativeBonus = 9  * populationCount(passedPawns[WHITE] | passedPawns[BLACK])
                        + 11 * (board.pieceCount[WHITE_PAWN] + board.pieceCount[BLACK_PAWN])
                        + 9  * (kingFlanking ? 1 : 0)
                        + 12 * (kingInfiltrating ? 1 : 0)
                        + 21 * (pawnsOnBothSides ? 1 : 0)
                        + 51 * ((board.nonPawnMaterial == 0) ? 1 : 0)
                        - 43 * (almostUnwinnable ? 1 : 0)
                        - 100;

    ExactScore middleGameScore = middlegameScore(cs);
    ExactScore endGameScore    = endgameScore(cs);

    int middlegameScoreBonus = ((middleGameScore > 0) - (middleGameScore < 0)) * std::max(std::min(initiativeBonus + 50, 0), -abs(middleGameScore));
    int endgameScoreBonus    = ((endGameScore    > 0) - (endGameScore    < 0)) * std::max(initiativeBonus, -abs(endGameScore));

    return makeScore(middlegameScoreBonus, endgameScoreBonus);
}

ExactScore Evaluation::evaluateImbalanceHelper(Color c, int pieceCount[COLOURS][ALL_PIECE_TYPES]) {

    ExactScore bonus = 0;

    for (int pt1 = PAWN; pt1 < ALL_PIECE_TYPES; pt1++) {

        if (pieceCount[c][pt1] == 0) continue;

        int value = 0;

        for (int pt2 = PAWN; pt2 <= pt1; pt2++) {
            value += QUADRATIC_PARAMETERS_OURS  [pt1][pt2] * pieceCount[ c][pt2]
                   + QUADRATIC_PARAMETERS_THEIRS[pt1][pt2] * pieceCount[~c][pt2];
        }
        bonus += pieceCount[c][pt1] * value;
    }

    return bonus;

}

ExactScore Evaluation::evaluatePosition() {

    ExactScore evaluation;
    CombinedScore cs = board.pieceSquareScore + evaluateImbalance();

    cs += evaluatePawns(WHITE) - evaluatePawns(BLACK);

    initEvaluation();

    cs += evaluatePiece(KNIGHT, WHITE) - evaluatePiece(KNIGHT, BLACK);
    cs += evaluatePiece(BISHOP, WHITE) - evaluatePiece(BISHOP, BLACK);
    cs += evaluatePiece(ROOK  , WHITE) - evaluatePiece(ROOK  , BLACK);
    cs += evaluatePiece(QUEEN , WHITE) - evaluatePiece(QUEEN , BLACK);
    
    cs += totalMobility[WHITE] - totalMobility[BLACK];

    cs += evaluateKing(WHITE) - evaluateKing(BLACK);
    cs += evaluateThreats(WHITE) - evaluateThreats(BLACK);
    cs += evaluatePassedPawns(WHITE) - evaluatePassedPawns(BLACK);
    cs += evaluateSpace(WHITE) - evaluateSpace(BLACK);

    cs += evaluateInitiative(cs);
    
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

>>>>>>> 61b4809289c1189e58962f19777acf3e93307f2a
}
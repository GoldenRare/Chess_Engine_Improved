#include <iostream>
#include "moves.hpp"
#include "board.hpp"
#include "randomNumber.hpp"

Magic bishopMagics[NUMBER_OF_SQUARES];
Magic rookMagics[NUMBER_OF_SQUARES];
Bitboard pawnAttacks[COLOURS][NUMBER_OF_SQUARES];
Bitboard knightAttacks[NUMBER_OF_SQUARES];
Bitboard kingAttacks[NUMBER_OF_SQUARES];
Bitboard inBetween[NUMBER_OF_SQUARES][NUMBER_OF_SQUARES] = { };
Bitboard rayLine[NUMBER_OF_SQUARES][NUMBER_OF_SQUARES]   = { };

unsigned int Magic::index(Bitboard occupied) {

    return ((occupied & relevantOccupancy) * magicNumber) >> shift;

}

Bitboard slidingAttacks(const Direction directions[], Square sq, Bitboard occupied) {

    Bitboard attacks = 0;
    for (int i = 0; i < NUMBER_OF_SLIDING_DIRECTIONS; i++) {
        for (int s = sq + directions[i]; isValidSquare(static_cast<Square>(s)) && (ChebyshevDistance[s][s - directions[i]] == 1); s += directions[i]) {
            
            attacks |= (1ULL << s);

            if ((occupied & (1ULL << s)) > 0) {
                break;
            }
        }
    }
    return attacks;
}

void makeMagics(Magic magics[], const Direction directions[]) {

    Bitboard occupancy[4096], attacksBasedOnOccupancy[4096], edges, powerSet;
    int size;

    for (int i = H1; i <= A8; i++) {

        edges = ((RANK_1_BB | RANK_8_BB) & ~rankOfSquareBB(static_cast<Square>(i))) | ((FILE_A_BB | FILE_H_BB) & ~fileOfSquareBB(static_cast<Square>(i)));
        
        magics[i].relevantOccupancy = slidingAttacks(directions, static_cast<Square>(i), 0) & ~edges;
        magics[i].shift = 64 - populationCount(magics[i].relevantOccupancy);

        size = powerSet = 0;

        do {
            
            occupancy[size] = powerSet;
            attacksBasedOnOccupancy[size] = slidingAttacks(directions, static_cast<Square>(i), powerSet);
            size++;
            powerSet = (powerSet - magics[i].relevantOccupancy) & magics[i].relevantOccupancy;

        } while (powerSet > 0);

        Bitboard trialAndError[4096] = {};
        int trial = 0;
        for (int j = 0; j < size; j++) {

            if (j == 0) {

                magics[i].magicNumber = random64.manyZeros();
                trial++;

            }

            unsigned int index = magics[i].index(occupancy[j]);

            if (trialAndError[index] < trial) {

                trialAndError[index] = trial;
                magics[i].attacksTable[index] = attacksBasedOnOccupancy[j];

            } else if (magics[i].attacksTable[index] != attacksBasedOnOccupancy[j]) {
                j = -1;
            }
        }
    }
}

Bitboard bishopAttacks(Bitboard occupied, Square sq) {
    return bishopMagics[sq].attacksTable[bishopMagics[sq].index(occupied)];
}

Bitboard rookAttacks(Bitboard occupied, Square sq) {
    return rookMagics[sq].attacksTable[rookMagics[sq].index(occupied)];
}

void initMoves() {

    initMagics();
    initPawnAttacks();
    initKnightAttacks();
    initKingAttacks();
    initInBetweenAndRayLine();

}

void initMagics() {

    makeMagics(rookMagics, ROOK_MOVEMENT);
    makeMagics(bishopMagics, BISHOP_MOVEMENT);

}

void initPawnAttacks() {

    Bitboard attacks, attacks2;
    for (int sq = H1; sq <= A8; sq++) {
        attacks2 = attacks = 0;
        for (int i = 0; i < NUMBER_OF_PAWN_ATTACK_DIRECTIONS; i++) {

            int target  = sq + WHITE_PAWN_MOVEMENT[i];
            int target2 = sq + BLACK_PAWN_MOVEMENT[i];
            if (isValidSquare(static_cast<Square>(target)) && (ChebyshevDistance[target][target - WHITE_PAWN_MOVEMENT[i]] == 1)) {
                attacks |= (1ULL << target);
            }

            if (isValidSquare(static_cast<Square>(target2)) && (ChebyshevDistance[target2][target2 - BLACK_PAWN_MOVEMENT[i]] == 1)) {
                attacks2 |= (1ULL << target2);
            }
        }
        pawnAttacks[WHITE][sq] = attacks;
        pawnAttacks[BLACK][sq] = attacks2;
    }
}

void initKnightAttacks() {

    Bitboard attacks;
    for (int sq = H1; sq <= A8; sq++) {
        attacks = 0;
        for (int i = 0; i < NUMBER_OF_KNIGHT_DIRECTIONS; i++) {

            int target = sq + KNIGHT_MOVEMENT[i];
            if (isValidSquare(static_cast<Square>(target)) && (ChebyshevDistance[target][target - KNIGHT_MOVEMENT[i]] == 2)) {
                attacks |= (1ULL << target);
            }
        }
        knightAttacks[sq] = attacks;
    }
}

void initKingAttacks() {

    Bitboard attacks;
    for (int sq = H1; sq <= A8; sq++) {
        attacks = 0;
        for (int i = 0; i < NUMBER_OF_KING_DIRECTIONS; i++) {

            int target = sq + KING_MOVEMENT[i];
            if (isValidSquare(static_cast<Square>(target)) && (ChebyshevDistance[target][target - KING_MOVEMENT[i]] == 1)) {
                attacks |= (1ULL << target);
            }
        }
        kingAttacks[sq] = attacks;
    }
}

void initInBetweenAndRayLine() {

    for (Piece p = WHITE_BISHOP; p <= WHITE_ROOK; p++) {
        for (Square sq1 = H1; sq1 <= A8; sq1++) {
            for (Square sq2 = H1; sq2 <= A8; sq2++) {

               Bitboard sq1BB = squareToBitboard(sq1);
               Bitboard sq2BB = squareToBitboard(sq2);

               Bitboard inBetweenAttacksSq1 = (p == WHITE_BISHOP) ? bishopAttacks(sq2BB, sq1) : rookAttacks(sq2BB, sq1);
               Bitboard inBetweenAttacksSq2 = (p == WHITE_BISHOP) ? bishopAttacks(sq1BB, sq2) : rookAttacks(sq1BB, sq2);
               Bitboard rayLineAttacksSq1   = (p == WHITE_BISHOP) ? bishopAttacks(0    , sq1) : rookAttacks(0    , sq1);
               Bitboard rayLineAttacksSq2   = (p == WHITE_BISHOP) ? bishopAttacks(0    , sq2) : rookAttacks(0    , sq2);

               if ((inBetweenAttacksSq1 & sq2BB) != 0) {
                   inBetween[sq1][sq2] = (inBetweenAttacksSq1 & inBetweenAttacksSq2);
                   rayLine  [sq1][sq2] = (rayLineAttacksSq1   & rayLineAttacksSq2  ) | sq1BB | sq2BB;
               }
            }
        }
    }
}
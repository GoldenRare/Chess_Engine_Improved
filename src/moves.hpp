#ifndef MOVES_HPP
#define MOVES_HPP

#include "utility.hpp"

constexpr int NUMBER_OF_PAWN_ATTACK_DIRECTIONS = 2;
constexpr int NUMBER_OF_SLIDING_DIRECTIONS     = 4;
constexpr int NUMBER_OF_KNIGHT_DIRECTIONS      = 8;
constexpr int NUMBER_OF_KING_DIRECTIONS        = 8;

constexpr Direction WHITE_PAWN_MOVEMENT[NUMBER_OF_PAWN_ATTACK_DIRECTIONS] = {NORTH_EAST, NORTH_WEST};
constexpr Direction BLACK_PAWN_MOVEMENT[NUMBER_OF_PAWN_ATTACK_DIRECTIONS] = {SOUTH_EAST, SOUTH_WEST};
constexpr Direction BISHOP_MOVEMENT[NUMBER_OF_SLIDING_DIRECTIONS]         = {NORTH_EAST, NORTH_WEST, SOUTH_EAST, SOUTH_WEST};
constexpr Direction ROOK_MOVEMENT[NUMBER_OF_SLIDING_DIRECTIONS]           = {NORTH, EAST, SOUTH, WEST};
constexpr Direction KING_MOVEMENT[NUMBER_OF_KING_DIRECTIONS]              = {NORTH_WEST, NORTH, NORTH_EAST, WEST, EAST, SOUTH_WEST, SOUTH, SOUTH_EAST};
constexpr Direction KNIGHT_MOVEMENT[NUMBER_OF_KNIGHT_DIRECTIONS]          = {NORTH_NORTH_WEST, NORTH_NORTH_EAST, NORTH_WEST_WEST, NORTH_EAST_EAST,
                                                                             SOUTH_WEST_WEST, SOUTH_EAST_EAST, SOUTH_SOUTH_WEST, SOUTH_SOUTH_EAST};

extern Bitboard pawnAttacks[COLOURS][SQUARES]; //Where a pawn can attack based on color and square
extern Bitboard knightAttacks[SQUARES];
extern Bitboard kingAttacks[SQUARES];
extern Bitboard inBetween[SQUARES][SQUARES];
extern Bitboard rayLine[SQUARES][SQUARES];

struct Magic {

    Bitboard relevantOccupancy;
    int shift;
    Bitboard magicNumber;
    Bitboard attacksTable[4096];
    unsigned int index(Bitboard occupied);

};

extern Magic rookMagics[SQUARES];
extern Magic bishopMagics[SQUARES];

void makeMagics(Magic magics[]);
Bitboard slidingAttacks(const Direction directions[], Square sq, Bitboard occupied);

Bitboard bishopAttacks(Bitboard occupied, Square sq);
Bitboard rookAttacks(Bitboard occupied, Square sq);

void initMoves();
void initMagics();
void initPawnAttacks();
void initKnightAttacks();
void initKingAttacks();
void initInBetweenAndRayLine();

//Returns a bitboard of pawns able to push up one (dependent on side)
inline Bitboard pawnsAbleToPush(Bitboard pawns, Bitboard emptySquares, Color c) {
    //Shifts the empty squares either up one or down one to check for which pawns can move to 
    //empty square (based on color)
    Bitboard shifted = (c == WHITE) ? emptySquares >> -SOUTH : emptySquares << NORTH;
    return shifted & pawns;
}

//Returns a bitboard of pawns able to push up twice (dependent on side)
inline Bitboard pawnsAbleToPushTwice(Bitboard pawns, Bitboard emptySquares, Color c) {
    //First finds the empty middle rank (relative to side), then shifts it down to
    //find empty rank in front of the pawn 
    Bitboard emptyRankInFrontOfPawn = (c == WHITE) ? ((RANK_4_BB & emptySquares) >> -SOUTH) & emptySquares 
                                       : ((RANK_5_BB & emptySquares) <<  NORTH) & emptySquares;

    return pawnsAbleToPush(pawns, emptyRankInFrontOfPawn, c);
}

//Returns a bitboard of all the pawn attacks (dependent on side)
inline Bitboard allPawnAttacks(Bitboard pawns, Color c) {
    return (c == WHITE) ? (pawns & ~FILE_H_BB) <<  NORTH_EAST | (pawns & ~FILE_A_BB) <<  NORTH_WEST
                        : (pawns & ~FILE_H_BB) >> -SOUTH_EAST | (pawns & ~FILE_A_BB) >> -SOUTH_WEST;
}

//Returns a bitboard of which squares are attacked twice by pawns (dependent on side)
inline Bitboard attackedTwiceByPawns(Bitboard pawns, Color c) {
    return (c == WHITE) ? (pawns & ~FILE_H_BB) <<  NORTH_EAST & (pawns & ~FILE_A_BB) <<  NORTH_WEST
                        : (pawns & ~FILE_H_BB) >> -SOUTH_EAST & (pawns & ~FILE_A_BB) >> -SOUTH_WEST;
}

//Returns a bitboard of all squares in front of the pawns, including the square of the pawn itself (dependent on sides)
Bitboard pawnFrontFill(Bitboard pawns, Color c);

//Returns a bitboard of all squares in front of the pawns, excluding the square of the pawn itself (dependent on sides)
inline Bitboard pawnFrontSpans(Bitboard pawns, Color c) {
    return (c == WHITE) ? pawnFrontFill(pawns, c) <<   NORTH
                        : pawnFrontFill(pawns, c) >> (-SOUTH);
}

inline Bitboard pawnAttackSpans(Bitboard pawns, Color c) {
    Bitboard frontSpan = pawnFrontSpans(pawns, c);
    return ((frontSpan & ~FILE_H_BB) >> (-EAST)) | ((frontSpan & ~FILE_A_BB) << WEST);
}



#endif
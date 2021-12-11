/* Heavily influenced by Stockfish and Chessprogramming wiki */

#ifndef EVALUATION_HPP
#define EVALUATION_HPP

#include <cstdint>
#include "utility.hpp"
#include "board.hpp"

extern CombinedScore pieceSquareTable[PIECES][NUMBER_OF_SQUARES];

enum Phase {

    MIDDLEGAME, ENDGAME, PHASES,

    PAWN_PHASE   = 0, 
    KNIGHT_PHASE = 1,
    BISHOP_PHASE = 1,
    ROOK_PHASE   = 2,
    QUEEN_PHASE  = 4,
    TOTAL_PHASE  = (PAWN_PHASE * 16) + (KNIGHT_PHASE * 4) + (BISHOP_PHASE * 4) + (ROOK_PHASE * 4) + (QUEEN_PHASE * 2)  
    
};

enum PieceValue {

    PAWN_VALUE_MIDDLEGAME   = 128,  PAWN_VALUE_ENDGAME   = 213,
    KNIGHT_VALUE_MIDDLEGAME = 781,  KNIGHT_VALUE_ENDGAME = 854,
    BISHOP_VALUE_MIDDLEGAME = 825,  BISHOP_VALUE_ENDGAME = 915,
    ROOK_VALUE_MIDDLEGAME   = 1276, ROOK_VALUE_ENDGAME   = 1380,
    QUEEN_VALUE_MIDDLEGAME  = 2538, QUEEN_VALUE_ENDGAME  = 2682

};

constexpr Bitboard whiteOutpostRanks = RANK_4_BB | RANK_5_BB | RANK_6_BB;
constexpr Bitboard blackOutpostRanks = RANK_5_BB | RANK_4_BB | RANK_3_BB;

constexpr int MAX_QUEEN_MOVEMENTS = 27;
constexpr int SPACE_THRESHOLD = 12222;

///////////////////////////////
constexpr int QUADRATIC_PARAMETERS_OURS[ALL_PIECE_TYPES][ALL_PIECE_TYPES] = {
    {1438                         }, // Bishop pair
    {  40,  38                    }, // Pawn
    {  32, 255, -62               }, // Knight
    {   0, 104,   4,   0          }, // Bishop
    { -26,  -2,  47, 105, -208    }, // Rook
    {-189,  24, 117, 133, -134, -6}  // Queen
};

constexpr int QUADRATIC_PARAMETERS_THEIRS[ALL_PIECE_TYPES][ALL_PIECE_TYPES] = {
    {  0                       }, // Bishop pair
    { 36,   0                  }, // Pawn
    {  9,  63,   0             }, // Knight
    { 59,  65,  42,   0        }, // Bishop
    { 46,  39,  24, -24,   0   }, // Rook
    { 97, 100, -42, 137, 268, 0}  // Queen
};
///////////////////////////////
constexpr CombinedScore makeScore(int middlegame, int endgame) {
    return CombinedScore((int)((unsigned int) endgame << 16) + middlegame);
}

inline ExactScore middlegameScore(CombinedScore cs) {
    union { uint16_t unsign; int16_t sign; } middlegame = { uint16_t(unsigned(cs)) };
    return ExactScore(middlegame.sign);
}

inline ExactScore endgameScore(CombinedScore cs) {
    union { uint16_t unsign; int16_t sign; } endgame = { uint16_t(unsigned(cs + 0x8000) >> 16) };
    return ExactScore(endgame.sign);
}

inline bool compareCombinedScores(CombinedScore cs1, CombinedScore cs2) {
    return middlegameScore(cs1) < middlegameScore(cs2);
}

constexpr ExactScore PIECE_VALUE[PHASES][PIECES] = 
{
    { PAWN_VALUE_MIDDLEGAME, KNIGHT_VALUE_MIDDLEGAME, BISHOP_VALUE_MIDDLEGAME, ROOK_VALUE_MIDDLEGAME, QUEEN_VALUE_MIDDLEGAME, 0, 
      PAWN_VALUE_MIDDLEGAME, KNIGHT_VALUE_MIDDLEGAME, BISHOP_VALUE_MIDDLEGAME, ROOK_VALUE_MIDDLEGAME, QUEEN_VALUE_MIDDLEGAME, 0 },
      
    { PAWN_VALUE_ENDGAME   , KNIGHT_VALUE_ENDGAME   , BISHOP_VALUE_ENDGAME   , ROOK_VALUE_ENDGAME   , QUEEN_VALUE_ENDGAME   , 0,
      PAWN_VALUE_ENDGAME   , KNIGHT_VALUE_ENDGAME   , BISHOP_VALUE_ENDGAME   , ROOK_VALUE_ENDGAME   , QUEEN_VALUE_ENDGAME   , 0 }
};


////////// Pawn Bonuses and Penalties /////////
constexpr ExactScore CONNECTED_PAWN_BONUS[RANKS] = { 0, 7, 8, 12, 29, 48, 86 };
constexpr ExactScore PAWN_SHELTER_BONUS[FILES / 2][RANKS] = {
    {  -6,  81,  93,  58,  39,  18,   25 },
    { -43,  61,  35, -49, -29, -11,  -63 },
    { -10,  75,  23,  -2,  32,   3,  -45 },
    { -39, -13, -29, -52, -48, -67, -166 }
};
constexpr ExactScore UNBLOCKED_PAWN_STORM_PENALTY[FILES / 2][RANKS] = {
    {  85, -289, -166, 97, 50,  45,  50 },
    {  46,  -25,  122, 45, 37, -10,  20 },
    {  -6,   51,  168, 34, -2, -22, -14 },
    { -15,  -11,  101,  4, 11, -15, -29 }  
};
constexpr CombinedScore PASSED_PAWN_BONUS[RANKS] = 
{ makeScore(0, 0), makeScore(10, 28), makeScore(17, 33), makeScore(15, 41), makeScore(62, 72), makeScore(168, 177), makeScore(276, 260) };

constexpr CombinedScore ISOLATED_PAWN_PENALTY      = makeScore( 5, 15);
constexpr CombinedScore OPEN_TO_ATTACK_PENALTY     = makeScore(13, 27);
constexpr CombinedScore DOUBLED_PAWN_PENALTY       = makeScore(11, 56);
constexpr CombinedScore WEAK_LEVER_PENALTY         = makeScore( 0, 56);
constexpr CombinedScore BACKWARD_PAWN_PENALTY      = makeScore( 9, 24);
constexpr CombinedScore INSIDE_PASSED_PAWN_PENALTY = makeScore(11,  8);
constexpr CombinedScore BLOCKED_PAWN_STORM_PENALTY = makeScore(82, 82);
///////////////////////////////////////////////

constexpr CombinedScore ROOK_ON_SEMI_OR_OPEN_FILE_BONUS[2] = { makeScore(21, 4), makeScore(47, 25) };
constexpr CombinedScore THREATENED_BY_MINOR_PIECE_BONUS[ALL_PIECE_TYPES] = { // Indexed by which piece type we are attacking
    makeScore(6, 32), makeScore(59, 41), makeScore(79, 56), makeScore(90, 119), makeScore(79, 161), makeScore(0, 0)
};
constexpr CombinedScore THREATENED_BY_ROOK_BONUS[ALL_PIECE_TYPES] = { // Indexed by which piece type we are attacking
    makeScore(3, 44), makeScore(38, 71), makeScore(38, 61), makeScore(0, 38), makeScore(51, 38), makeScore(0, 0)
};

constexpr CombinedScore OUTPOST_BONUS                       = makeScore( 30, 21);
constexpr CombinedScore REACHABLE_OUTPOST_BONUS             = makeScore( 32, 10);
constexpr CombinedScore MINOR_PIECE_BEHIND_PAWN_BONUS       = makeScore( 18,  3);
constexpr CombinedScore BISHOP_CONTROL_OF_CENTER_BONUS      = makeScore( 45,  0);
constexpr CombinedScore ROOK_ON_QUEEN_FILE_BONUS            = makeScore(  7,  6);
constexpr CombinedScore THREATENED_BY_KING_BONUS            = makeScore( 24, 89);
constexpr CombinedScore THREATENED_BY_SAFE_PAWN_BONUS       = makeScore(173, 94);
constexpr CombinedScore KNIGHT_ATTACKING_QUEEN_BONUS        = makeScore( 16, 12);
constexpr CombinedScore SLIDING_PIECE_ATTACKING_QUEEN_BONUS = makeScore( 59, 18);
constexpr CombinedScore HANGING_PIECE_BONUS                 = makeScore( 69, 36);
constexpr CombinedScore PIECE_MOVEMENT_RESTRICTED_BONUS     = makeScore(  7,  7);
constexpr CombinedScore KING_PROTECTOR_PENALTY              = makeScore(  7,  8);
constexpr CombinedScore PAWNS_BLOCKING_BISHOP_PENALTY       = makeScore(  3,  7);
constexpr CombinedScore ROOK_TRAPPED_PENALTY                = makeScore( 52, 10);
constexpr CombinedScore QUEEN_IS_WEAK_PENALTY               = makeScore( 49, 15);

constexpr ExactScore SAFE_KNIGHT_CHECK_PENALTY = 790;
constexpr ExactScore SAFE_BISHOP_CHECK_PENALTY = 635;
constexpr ExactScore SAFE_ROOK_CHECK_PENALTY = 1080;
constexpr ExactScore SAFE_QUEEN_CHECK_PENALTY = 780;

constexpr int KING_ATTACKERS_WEIGHT[ALL_PIECE_TYPES] = { 0, 81, 52, 44, 10, 0};
constexpr CombinedScore PIECE_SQUARE_BONUS[PIECES / 2][RANKS][FILES / 2] = 
{
    { //Pawns handled separately
        { }, { }, { }, { }, { }, { }, { }, { } 
    },
    { //Knight square bonus
        { makeScore(-175, -96), makeScore(-92,-65), makeScore(-74,-49), makeScore(-73,-21) },
        { makeScore( -77, -67), makeScore(-41,-54), makeScore(-27,-18), makeScore(-15,  8) },
        { makeScore( -61, -40), makeScore(-17,-27), makeScore(  6, -8), makeScore( 12, 29) },
        { makeScore( -35, -35), makeScore(  8, -2), makeScore( 40, 13), makeScore( 49, 28) },
        { makeScore( -34, -45), makeScore( 13,-16), makeScore( 44,  9), makeScore( 51, 39) },
        { makeScore(  -9, -51), makeScore( 22,-44), makeScore( 58,-16), makeScore( 53, 17) },
        { makeScore( -67, -69), makeScore(-27,-50), makeScore(  4,-51), makeScore( 37, 12) },
        { makeScore(-201,-100), makeScore(-83,-88), makeScore(-56,-56), makeScore(-26,-17) }
    },
    { //Bishop square bonus
        { makeScore(-53,-57), makeScore( -5,-30), makeScore( -8,-37), makeScore(-23,-12) },
        { makeScore(-15,-37), makeScore(  8,-13), makeScore( 19,-17), makeScore(  4,  1) },
        { makeScore( -7,-16), makeScore( 21, -1), makeScore( -5, -2), makeScore( 17, 10) },
        { makeScore( -5,-20), makeScore( 11, -6), makeScore( 25,  0), makeScore( 39, 17) },
        { makeScore(-12,-17), makeScore( 29, -1), makeScore( 22,-14), makeScore( 31, 15) },
        { makeScore(-16,-30), makeScore(  6,  6), makeScore(  1,  4), makeScore( 11,  6) },
        { makeScore(-17,-31), makeScore(-14,-20), makeScore(  5, -1), makeScore(  0,  1) },
        { makeScore(-48,-46), makeScore(  1,-42), makeScore(-14,-37), makeScore(-23,-24) }
    }, 
    { //Rook square bonus
        { makeScore(-31, -9), makeScore(-20,-13), makeScore(-14,-10), makeScore(-5, -9) },
        { makeScore(-21,-12), makeScore(-13, -9), makeScore( -8, -1), makeScore( 6, -2) },
        { makeScore(-25,  6), makeScore(-11, -8), makeScore( -1, -2), makeScore( 3, -6) },
        { makeScore(-13, -6), makeScore( -5,  1), makeScore( -4, -9), makeScore(-6,  7) },
        { makeScore(-27, -5), makeScore(-15,  8), makeScore( -4,  7), makeScore( 3, -6) },
        { makeScore(-22,  6), makeScore( -2,  1), makeScore(  6, -7), makeScore(12, 10) },
        { makeScore( -2,  4), makeScore( 12,  5), makeScore( 16, 20), makeScore(18, -5) },
        { makeScore(-17, 18), makeScore(-19,  0), makeScore( -1, 19), makeScore( 9, 13) }
    },
    { //Queen square bonus
        { makeScore( 3,-69), makeScore(-5,-57), makeScore(-5,-47), makeScore( 4,-26) },
        { makeScore(-3,-55), makeScore( 5,-31), makeScore( 8,-22), makeScore(12, -4) },
        { makeScore(-3,-39), makeScore( 6,-18), makeScore(13, -9), makeScore( 7,  3) },
        { makeScore( 4,-23), makeScore( 5, -3), makeScore( 9, 13), makeScore( 8, 24) },
        { makeScore( 0,-29), makeScore(14, -6), makeScore(12,  9), makeScore( 5, 21) },
        { makeScore(-4,-38), makeScore(10,-18), makeScore( 6,-12), makeScore( 8,  1) },
        { makeScore(-5,-50), makeScore( 6,-27), makeScore(10,-24), makeScore( 8, -8) },
        { makeScore(-2,-75), makeScore(-2,-52), makeScore( 1,-43), makeScore(-2,-36) }
    }, 
    { //King square bonus
        { makeScore(271,  1), makeScore(327, 45), makeScore(271, 85), makeScore(198, 76) },
        { makeScore(278, 53), makeScore(303,100), makeScore(234,133), makeScore(179,135) },
        { makeScore(195, 88), makeScore(258,130), makeScore(169,169), makeScore(120,175) },
        { makeScore(164,103), makeScore(190,156), makeScore(138,172), makeScore( 98,172) },
        { makeScore(154, 96), makeScore(179,166), makeScore(105,199), makeScore( 70,199) },
        { makeScore(123, 92), makeScore(145,172), makeScore( 81,184), makeScore( 31,191) },
        { makeScore( 88, 47), makeScore(120,121), makeScore( 65,116), makeScore( 33,131) },
        { makeScore( 59, 11), makeScore( 89, 59), makeScore( 45, 73), makeScore( -1, 78) }
    }
};

constexpr CombinedScore PAWN_SQUARE_BONUS[RANKS][FILES] = 
{
    { },
    { makeScore(  3,-10), makeScore(  3, -6), makeScore( 10, 10), makeScore( 19,  0), makeScore( 16, 14), makeScore( 19,  7), makeScore(  7, -5), makeScore( -5,-19) },
    { makeScore( -9,-10), makeScore(-15,-10), makeScore( 11,-10), makeScore( 15,  4), makeScore( 32,  4), makeScore( 22,  3), makeScore(  5, -6), makeScore(-22, -4) },
    { makeScore( -8,  6), makeScore(-23, -2), makeScore(  6, -8), makeScore( 20, -4), makeScore( 40,-13), makeScore( 17,-12), makeScore(  4,-10), makeScore(-12, -9) },
    { makeScore( 13,  9), makeScore(  0,  4), makeScore(-13,  3), makeScore(  1,-12), makeScore( 11,-12), makeScore( -2, -6), makeScore(-13, 13), makeScore(  5,  8) },
    { makeScore( -5, 28), makeScore(-12, 20), makeScore( -7, 21), makeScore( 22, 28), makeScore( -8, 30), makeScore( -5,  7), makeScore(-15,  6), makeScore(-18, 13) },
    { makeScore( -7,  0), makeScore(  7,-11), makeScore( -3, 12), makeScore(-13, 21), makeScore(  5, 25), makeScore(-16, 19), makeScore( 10,  4), makeScore( -8,  7) },
    { }
};

constexpr CombinedScore MOBILITY_BONUS[ALL_PIECE_TYPES - 2][MAX_QUEEN_MOVEMENTS + 1] =
{
    { // Knight mobility bonus
        makeScore(-62,-81), makeScore(-53,-56), makeScore(-12,-30), makeScore( -4,-14), makeScore(  3,  8), makeScore( 13, 15), makeScore( 22, 23), 
        makeScore( 28, 27), makeScore( 33, 33)
    },
    { // Bishop mobility bonus
        makeScore(-48,-59), makeScore(-20,-23), makeScore( 16, -3), makeScore( 26, 13), makeScore( 38, 24), makeScore( 51, 42), makeScore( 55, 54), 
        makeScore( 63, 57), makeScore( 63, 65), makeScore( 68, 73), makeScore( 81, 78), makeScore( 81, 86), makeScore( 91, 88), makeScore( 98, 97)
    }, 
    { // Rook mobility bonus
        makeScore(-58,-76), makeScore(-27,-18), makeScore(-15, 28), makeScore(-10, 55), makeScore( -5, 69), makeScore( -2, 82), makeScore(  9,112), 
        makeScore( 16,118), makeScore( 30,132), makeScore( 29,142), makeScore( 32,155), makeScore( 38,165), makeScore( 46,166), makeScore( 48,169), 
        makeScore( 58,171)
    }, 
    { // Queen mobility bonus
        makeScore(-39,-36), makeScore(-21,-15), makeScore(  3,  8), makeScore(  3, 18), makeScore( 14, 34), makeScore( 22, 54), makeScore( 28, 61), 
        makeScore( 41, 73), makeScore( 43, 79), makeScore( 48, 92), makeScore( 56, 94), makeScore( 60,104), makeScore( 60,113), makeScore( 66,120), 
        makeScore( 67,123), makeScore( 70,126), makeScore( 71,133), makeScore( 73,136), makeScore( 79,140), makeScore( 88,143), makeScore( 88,148), 
        makeScore( 99,166), makeScore(102,170), makeScore(102,175), makeScore(106,184), makeScore(109,191), makeScore(113,206), makeScore(116,212)
    }
};

void initPieceSquareTable();
void initEvaluation();

class Evaluation {

    public:

        Evaluation(const ChessBoard& b);
        ExactScore evaluatePosition();

    private:

        // Board of the Evaluation we are evaluating
        const ChessBoard& board;

        // Holds all the passed pawns for a given side
        Bitboard passedPawns[COLOURS];

        // All the attacks by a Color and a PieceType
        Bitboard attacksBy[COLOURS][PIECE_TYPES];

        // All the attacks where the square is hit twice indexed by Colour
        Bitboard attacksBy2[COLOURS];

        // Safe squares
        Bitboard mobilityArea[COLOURS];

        // Total mobility 
        CombinedScore totalMobility[COLOURS];

        // Squares around a king
        Bitboard kingRing[COLOURS];

        // Number of pieces attacking the enemy king ring
        int attackingEnemyKingRingCount[COLOURS];

        // The sum of the piece weights to those pieces which are the ones attacking the enemy king ring (helper to attackingEnemyKingRingCount)
        int attackingEnemyKingRingPieceWeight[COLOURS];

        // Number of squares we are attacking directly around the king (not including the king square)
        int attackedSquaresAroundEnemyKing[COLOURS];

        

        void initEvaluation();

        CombinedScore evaluateImbalance();
        CombinedScore evaluatePiece(PieceType pt, Color c);
        CombinedScore evaluatePawns(Color c);
        CombinedScore evaluatePassedPawns(Color c);
        CombinedScore evaluateSpace(Color c);
        CombinedScore evaluateThreats(Color c);
        CombinedScore evaluatePawnShelter(Color c, Square kingSq);
        CombinedScore evaluateKingSafety(Color c);
        CombinedScore evaluateKing(Color c);
        CombinedScore evaluateInitiative(CombinedScore cs);

        ExactScore evaluateImbalanceHelper(Color c, int pieceCount[COLOURS][ALL_PIECE_TYPES]);
        int gamePhase();

};

#endif
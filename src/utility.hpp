#ifndef UTILITY_HPP
#define UTILITY_HPP

#include <cstdint>

typedef uint64_t Bitboard;
typedef uint64_t PositionKey;

enum PieceType { PAWN, KNIGHT, BISHOP, ROOK, QUEEN, KING, ALL_PIECE_TYPES, PIECE_TYPES };

enum Piece {

    WHITE_PAWN, WHITE_KNIGHT, WHITE_BISHOP, WHITE_ROOK, WHITE_QUEEN, WHITE_KING,
    BLACK_PAWN, BLACK_KNIGHT, BLACK_BISHOP, BLACK_ROOK, BLACK_QUEEN, BLACK_KING,
    PIECES, NO_PIECE = -1

};

enum Color { WHITE, BLACK, COLOURS};
enum Rank  { RANK_1, RANK_2, RANK_3, RANK_4, RANK_5, RANK_6, RANK_7, RANK_8, RANKS};
enum File  { FILE_A, FILE_B, FILE_C, FILE_D, FILE_E, FILE_F, FILE_G, FILE_H, FILES};

enum Square {

    H1, G1, F1, E1, D1, C1, B1, A1,
    H2, G2, F2, E2, D2, C2, B2, A2,
    H3, G3, F3, E3, D3, C3, B3, A3,
    H4, G4, F4, E4, D4, C4, B4, A4,
    H5, G5, F5, E5, D5, C5, B5, A5,
    H6, G6, F6, E6, D6, C6, B6, A6,
    H7, G7, F7, E7, D7, C7, B7, A7,
    H8, G8, F8, E8, D8, C8, B8, A8,
    NO_SQUARE = -1

};

enum Direction {

    NORTH            = 8, 
    SOUTH            = -NORTH, 
    EAST             = -1, 
    WEST             = -EAST, 
    NORTH_EAST       = NORTH + EAST,
    NORTH_WEST       = NORTH + WEST,
    SOUTH_EAST       = SOUTH + EAST,
    SOUTH_WEST       = SOUTH + WEST,
    NORTH_NORTH_WEST = NORTH + NORTH + WEST,
    NORTH_NORTH_EAST = NORTH + NORTH + EAST,
    NORTH_WEST_WEST  = NORTH + WEST + WEST,
    NORTH_EAST_EAST  = NORTH + EAST + EAST,
    SOUTH_WEST_WEST  = SOUTH + WEST + WEST,
    SOUTH_EAST_EAST  = SOUTH + EAST + EAST,
    SOUTH_SOUTH_WEST = SOUTH + SOUTH + WEST,
    SOUTH_SOUTH_EAST = SOUTH + SOUTH + EAST
    
};

enum MoveType {

    QUIET                   , 
    DOUBLE_PAWN_PUSH        , 
    KINGSIDE_CASTLE         , 
    QUEENSIDE_CASTLE        ,
    CAPTURE                 , 
    EN_PASSANT_CAPTURE      ,
    KNIGHT_PROMOTION = 8    , 
    BISHOP_PROMOTION        ,
    ROOK_PROMOTION          , 
    QUEEN_PROMOTION         , 
    KNIGHT_PROMOTION_CAPTURE, 
    BISHOP_PROMOTION_CAPTURE,
    ROOK_PROMOTION_CAPTURE  , 
    QUEEN_PROMOTION_CAPTURE
    
};

enum Bound { UPPER_BOUND = 1, LOWER_BOUND, EXACT_BOUND };

constexpr int STALEMATE_SCORE = 0;
constexpr int MATE_SCORE = 32000;
constexpr int INFINITE = 32001;
constexpr int NUMBER_OF_SQUARES = 64;

const constexpr char* NOTATION[NUMBER_OF_SQUARES] = {

    "H1", "G1", "F1", "E1", "D1", "C1", "B1", "A1",
    "H2", "G2", "F2", "E2", "D2", "C2", "B2", "A2",
    "H3", "G3", "F3", "E3", "D3", "C3", "B3", "A3",
    "H4", "G4", "F4", "E4", "D4", "C4", "B4", "A4",
    "H5", "G5", "F5", "E5", "D5", "C5", "B5", "A5",
    "H6", "G6", "F6", "E6", "D6", "C6", "B6", "A6",
    "H7", "G7", "F7", "E7", "D7", "C7", "B7", "A7",
    "H8", "G8", "F8", "E8", "D8", "C8", "B8", "A8"

};


constexpr Bitboard FILE_H_BB = 0x0101010101010101;
constexpr Bitboard FILE_G_BB = FILE_H_BB << (1 * WEST);
constexpr Bitboard FILE_F_BB = FILE_H_BB << (2 * WEST);
constexpr Bitboard FILE_E_BB = FILE_H_BB << (3 * WEST);
constexpr Bitboard FILE_D_BB = FILE_H_BB << (4 * WEST);
constexpr Bitboard FILE_C_BB = FILE_H_BB << (5 * WEST);
constexpr Bitboard FILE_B_BB = FILE_H_BB << (6 * WEST);
constexpr Bitboard FILE_A_BB = FILE_H_BB << (7 * WEST);

constexpr Bitboard RANK_1_BB = 0x00000000000000FF;
constexpr Bitboard RANK_2_BB = RANK_1_BB << (1 * NORTH);
constexpr Bitboard RANK_3_BB = RANK_1_BB << (2 * NORTH);
constexpr Bitboard RANK_4_BB = RANK_1_BB << (3 * NORTH);
constexpr Bitboard RANK_5_BB = RANK_1_BB << (4 * NORTH);
constexpr Bitboard RANK_6_BB = RANK_1_BB << (5 * NORTH);
constexpr Bitboard RANK_7_BB = RANK_1_BB << (6 * NORTH);
constexpr Bitboard RANK_8_BB = RANK_1_BB << (7 * NORTH);

constexpr int DE_BRUIJN_INDEX[NUMBER_OF_SQUARES] = {

    0,  1, 48,  2, 57, 49, 28,  3,
   61, 58, 50, 42, 38, 29, 17,  4,
   62, 55, 59, 36, 53, 51, 43, 22,
   45, 39, 33, 30, 24, 18, 12,  5,
   63, 47, 56, 27, 60, 41, 37, 16,
   54, 35, 52, 21, 44, 32, 23, 11,
   46, 26, 40, 15, 34, 20, 31, 10,
   25, 14, 19,  9, 13,  8,  7,  6

};

constexpr int NUMBER_OF_PIECES = 12;
constexpr Bitboard DE_BRUIJN_SEQUENCE = 0x03F79D71B4CB0A89;

struct Move {

    unsigned int move;  //Encoding for a move
    unsigned int score = 0; //How good a move is 

};

struct GameState {

    Move move;
    unsigned int castlingRights;
    Piece capturedPiece;
    Square enPassantSquare;

};

//Returns a 16-Bit number encoding a move
inline unsigned int move(unsigned int from, unsigned int to, unsigned int flags) {
    return ((flags & (0xF)) << 12) | ((to & (0x3F)) << 6) | (from & (0x3F));
}

//Returns the from square
inline unsigned int getFrom(Move m) {
    return m.move & 0x3F;
}

//Returns the to square
inline unsigned int getTo(Move m) {
    return (m.move >> 6) & 0x3F;
}

//Returns extra information of the move (enum MoveType)
inline unsigned int getFlags(Move m) {
    return (m.move >> 12) & 0xF;
}

//Similar to getFlags but instead returns MoveType
inline MoveType typeOfMove(Move m) {
    return static_cast<MoveType>(getFlags(m));
}

//Returns if a move is quiet or not (Currently treats promotions as quiet)
inline bool isQuietMove(Move m) {
    MoveType moveType = typeOfMove(m);
    if ((moveType == CAPTURE) || (moveType == EN_PASSANT_CAPTURE) || (moveType == KNIGHT_PROMOTION_CAPTURE)
     || (moveType == BISHOP_PROMOTION_CAPTURE) || (moveType == ROOK_PROMOTION_CAPTURE) || (moveType == QUEEN_PROMOTION_CAPTURE)) return false;
    return true;
}

//Returns the square as a Bitboard
inline Bitboard squareToBitboard(Square sq) {
    return 1ULL << sq;
}

//Returns the square of the least significant 1 bit 
//using the De Bruijn bitscan algorithm
inline Square squareOfLS1B(Bitboard b) {
    Bitboard LS1B = b & (-b);
    return static_cast<Square>(DE_BRUIJN_INDEX[(LS1B * DE_BRUIJN_SEQUENCE) >> 58]);
}

//Returns the square of the least significant 1 bit 
//using the De Bruijn bitscan algorithm (also sets up for next iteration)
inline Square squareOfLS1B(Bitboard* b) {
    Bitboard LS1B = *b & (-*b);
    *b &= (*b - 1);
    return static_cast<Square>(DE_BRUIJN_INDEX[(LS1B * DE_BRUIJN_SEQUENCE) >> 58]);
}

inline Rank rankOfSquareRANK(Square s) {
    return Rank((s / 8));
}

inline Bitboard rankOfSquareBB(Square s) {
    return RANK_1_BB << (8 * rankOfSquareRANK(s));
}

inline File fileOfSquareFILE(Square s) {
    return File((7 - (s & 7)));
}

inline Bitboard fileOfSquareBB(Square s) {
    return FILE_A_BB >> fileOfSquareFILE(s);
}

inline Color operator~(Color c) {
    Color result = (c == WHITE) ? BLACK : WHITE;
    return result;
}

inline Piece operator+(Piece p, int offset) {
    return Piece(int(p) + int(offset));
}

inline Square operator++(Square& sq, int) {
    Square temp = sq;
    sq = Square((int)sq + 1);
    return temp;
}

inline Square& operator++(Square& sq) {
    return sq = Square((int)sq + 1);
}

inline Piece operator++(Piece& piece, int) {
    Piece temp = piece;
    piece = Piece((int)piece + 1);
    return temp;
}

inline Piece& operator++(Piece& piece) {
    return piece = Piece((int)piece + 1);
}

inline File operator++(File& file, int) {
    File temp = file;
    file = File((int)file + 1);
    return temp;
}

inline File& operator++(File& file) {
    return file = File((int)file + 1);
}

#endif
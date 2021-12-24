#ifndef BOARD_HPP
#define BOARD_HPP


#include <string>
#include "utility.hpp"

extern int ChebyshevDistance[SQUARES][SQUARES];

class ChessBoard {

    public:

        Piece pieceBoard[SQUARES]; // Indexed by square and returns the piece on that square
        Bitboard pieces[PIECES]; 
        Bitboard piecesOnSide[COLOURS];
        Bitboard occupiedSquares;  
        Bitboard emptySquares;
        int pieceCount[PIECES]; // Keeps track of the number of pieces on the board for a given piece
        Square pieceSquare[PIECES][10]; // Keeps track of all the squares for a given piece (Assumes 10 is the most you can have of one piece)
        Color sideToPlay;
        Square enPassant;
        unsigned int castlingRights; // Order of the bits: blackQueenside, blackKingside, whiteQueenside, whiteKingside
        int nonPawnMaterial[COLOURS]; // The sum of all piece values for a given side (not including pawns or kings)
        CombinedScore pieceSquareScore; // A score which combines the value of each piece and the square it occupies (score is relative to white)

        unsigned int ply;
        unsigned int halfmoves; 

        GameState previousGameStates[256];
        int previousGameStatesCount;

        ChessBoard();
        ChessBoard(std::string fen);

        void parseFEN(std::string fen);
        bool isSquareAttacked(Square sq, Color attacked) const; //Returns if a square is attacked by a certain color
        Bitboard attackersToSquare(Square sq, Bitboard occupied); //Returns a bitboard of pieces that are attacking a particular square (regardless of color)
        void makeMove(const Move& move);
        void undoMove();
        Bitboard blockers(Square sq, Bitboard sliders, Bitboard& pinners) const; //Returns a bitboard of pieces that are blocking a square from being attacked 
        bool givesCheck(const Move& move); // Returns whether or not a given move will give the opponent a check
        bool advancedPawnPush(const Move& move); // Returns whether or not a pawn is being pushed deep into enemy territory (WHITE: > RANK_5, BLACK: < RANK_4)
        int endgameValueOfPiece(const Move& move); // Returns the endgame value of a piece on the to square

        PositionKey getPositionKey() const;

    private:

        /* ATTRIBUTES */
        PositionKey positionKey; // 64-Bit number to uniquely identify any given chess position (if the position is repeated, the same key will be produced)
        /*            */

        void piecePlacement(Bitboard* init, char piece);
        void makePiece(Bitboard* init, Piece p);
        void parseCastlingRights(char c);
        void disableCastlingRights(Piece fromPiece, Square fromSquare, Piece capturedPiece, Square toSquare);//Disables castling rights if a rook or king moves
        int findPieceSquareIndex(Piece p, Square sq);
        void clearBoard();

};

void initChebyshevDistance();
void initBoard();

void printBitboard(Bitboard b);
bool isValidSquare(Square s);
int populationCount(Bitboard b);

inline PositionKey ChessBoard::getPositionKey() const {
    return positionKey;
}

#endif
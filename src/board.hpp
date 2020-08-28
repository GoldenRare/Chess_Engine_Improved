#ifndef BOARD_HPP
#define BOARD_HPP


#include <string>
#include "utility.hpp"

extern int ChebyshevDistance[NUMBER_OF_SQUARES][NUMBER_OF_SQUARES];

class ChessBoard {

    public:

        PositionKey positionKey = 0;
        Piece pieceBoard[NUMBER_OF_SQUARES] = {}; //Indexed by square and returns the piece
        Bitboard pieces[NUMBER_OF_PIECES] = {}; //Order of the value of Pieces (Knight first and White first)
        Bitboard piecesOnSide[COLOURS] = {};
        Bitboard gameBoard = 0;  //The AND of all pieces
        Bitboard emptySquares = 0;
        int pieceCount[PIECES] = {}; //Keeps track of the number of pieces on the board for a given piece
        Square pieceSquare[PIECES][10] = {}; //Keeps track of the square for a given piece
        Color sideToPlay;
        Square enPassant = NO_SQUARE;
        unsigned int castlingRights = 0; // Order of the bits: blackQueenside, blackKingside, whiteQueenside, whiteKingside

        int ply = 0; 

        GameState previousGameStates[256];
        int previousGameStatesCount = 0;

        ChessBoard();
        ChessBoard(std::string fen);

       bool isSquareAttacked(Square sq, Color attacked) const; //Returns if a square is attacked by a certain color
       void makeMove(const Move& move);
       void undoMove();
       Bitboard blockers(Square sq, Bitboard sliders) const; //Returns a bitboard of pieces that are blocking a square from being attacked 

    private:

        void parseFEN(std::string fen);
        void piecePlacement(Bitboard* init, char piece);
        void makePiece(Bitboard* init, Piece p);
        void parseCastlingRights(char c);
        void disableCastlingRights(Piece fromPiece, Square fromSquare, Piece capturedPiece, Square toSquare);//Disables castling rights if a rook or king moves
        int findPieceSquareIndex(Piece p, Square sq);

};

void initChebyshevDistance();
void initBoard();

void printBitboard(Bitboard b);
bool isValidSquare(Square s);
int populationCount(Bitboard b);

#endif
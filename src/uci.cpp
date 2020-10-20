#include <iostream>
#include <sstream>
#include <string>
#include "uci.hpp"
#include "utility.hpp"
#include "board.hpp"
#include "moveGeneration.hpp"
#include "search.hpp"

void uciCommand() {

    std::cout << "id name GoldenRareBOT" << std::endl;
    std::cout << "id author Deshawn Mohan-Smith" << std::endl;
    std::cout << "uciok" << std::endl;

}

void isReadyCommand() {

    std::cout << "readyok" << std::endl;

}

void parseMoves(std::istringstream& iss, ChessBoard& board){
    
    std::string token, move;
    MoveType moveType;
    while (iss >> token) {

        Move movesList[256];
        Move* movesListStart = movesList;
        Move* movesListEnd = generateAllPseudoMoves(board, movesListStart);

        while (movesListStart < movesListEnd) {
            
            move  = NOTATION[getFrom(*movesListStart)];
            move += NOTATION[getTo(*movesListStart)];

            moveType = typeOfMove(*movesListStart);

            if      ((moveType == KNIGHT_PROMOTION) || (moveType == KNIGHT_PROMOTION_CAPTURE)) move += "n";
            else if ((moveType == BISHOP_PROMOTION) || (moveType == BISHOP_PROMOTION_CAPTURE)) move += "b";
            else if ((moveType == ROOK_PROMOTION  ) || (moveType == ROOK_PROMOTION_CAPTURE  )) move += "r";
            else if ((moveType == QUEEN_PROMOTION ) || (moveType == QUEEN_PROMOTION_CAPTURE )) move += "q";

            if (token == move) {

                board.makeMove(*movesListStart);
                break;

                /*ASSUME ALL MOVES ARE LEGAL
                //If move is illegal, don't make
                if (board.isSquareAttacked(board.pieceSquare[king][0], kingInCheck)) {

                    board.undoMove();
                    movesListStart++;
                    continue;

                }*/
            }
            movesListStart++;
        }
    }
}


void positionCommand(std::istringstream& iss, ChessBoard& board) {

    std::string token;

    while (iss >> token) {

        if (token == "startpos") board.parseFEN("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
        else if (token == "fen") {

            std::string fen = "";
            while (iss >> token && token != "moves")
                fen += (token + " ");

            board.parseFEN(fen);
        }

        if (token == "moves") parseMoves(iss, board);
    }
}

void goCommand(ChessBoard& board) {
    iterativeDeepening(board, 11);
}

void commandsLoop() {

    ChessBoard board;
    std::string commandLine, token;

    while (token != "quit") {

        
        std::getline(std::cin, commandLine);

        std::istringstream iss(commandLine);

        while (iss >> token) {

            if      (token == "uci"     ) uciCommand();
            else if (token == "isready" ) isReadyCommand();
            else if (token == "position") positionCommand(iss, board);
            else if (token == "go"      ) goCommand(board);
            else if (token == "quit"    ) break;
        }
    }
}
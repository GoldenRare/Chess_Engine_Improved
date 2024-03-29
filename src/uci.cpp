#include <iostream>
#include <sstream>
#include <string>
#include <cstdint>
#include "uci.hpp"
#include "utility.hpp"
#include "board.hpp"
#include "moveGeneration.hpp"
#include "search.hpp"
#include "transpositionTable.hpp"

void uciCommand() {

    std::cout << "id name GoldenRareBOT" << std::endl;
    std::cout << "id author Deshawn Mohan-Smith\n" << std::endl;
    std::cout << "option name Hash type spin default " << MIN_MB << " min " << MIN_MB << " max " << MAX_MB << std::endl;
    std::cout << "\nuciok" << std::endl;

}

void setOptionCommand(std::istringstream& iss) {

    std::string token; 
    iss >> token; // This token should always be "name", therefore we can skip it
    
    iss >> token;
    if (token == "Hash") {
        uint64_t value;
        iss >> token; // This token should always be "value"
        iss >> value;

        if (value >= MIN_MB && value <= MAX_MB)
            std::cout << "Allocating " << TT.setSize(value) << " * 32 / 1024 / 1024 MB (not computed due to potential overflow reasons)" << std::endl;
        else 
            std::cout << "Requesting too little or too much memory!" << std::endl;

    } else
        std::cout << "Invalid option name!" << std::endl;

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
    iterativeDeepening(board, 9);
}

void commandsLoop() {

    ChessBoard board;
    std::string commandLine, token;

    while (token != "quit") {

        
        std::getline(std::cin, commandLine);

        std::istringstream iss(commandLine);

        while (iss >> token) {

            if      (token == "uci"      ) uciCommand();
            else if (token == "setoption") setOptionCommand(iss);
            else if (token == "isready"  ) isReadyCommand();
            else if (token == "position" ) positionCommand(iss, board);
            else if (token == "go"       ) goCommand(board);
            else if (token == "quit"     ) break;
        }
    }
}
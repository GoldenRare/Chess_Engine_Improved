#include <iostream>
#include <chrono>
#include "board.hpp"
#include "moves.hpp"
#include "utility.hpp"
#include "moveGeneration.hpp"
#include "search.hpp"
#include "evaluation.hpp"
#include "zobrist.hpp"
#include "uci.hpp"

#include "transpositionTable.hpp"
#include "randomNumber.hpp"
#include <algorithm>

//Consider how to deal with Search Explosion in QSearch();
//Should be using pass by reference for Move (in utility.hpp)
//Remember the effects of not correctly updating pieceBoard (in board.hpp) in make/unmake move
//Experimenting with PVS in QSearch
//Adding const to ChessBoard& param
//Changing Alpha-Beta from fail-hard to fail-soft
int main() {

    initBoard();
    initMoves();
    initEvaluation();
    initZobrist();
    initSearch();
    commandsLoop();
    
    //q2k2q1/2nqn2b/1n1P1n1b/2rnr2Q/1NQ1QN1Q/3Q3B/2RQR2B/Q2K2Q1 w - - 0 1
    //ChessBoard board;  
    //iterativeDeepening(board, 10);

/*
    ChessBoard board("r3k2r/Pppp1ppp/1b3nbN/nP6/BBP1P3/q4N2/Pp1P2PP/R2Q1RK1 w kq - 0 1");
    auto t1 = std::chrono::high_resolution_clock::now();
    std::cout << perft(board, 6) << std::endl;
    auto t2 = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1).count();
    std::cout << duration << std::endl;*/

    return 0; 
}
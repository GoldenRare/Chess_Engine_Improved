#include <iostream>
#include <chrono>
#include "board.hpp"
#include "moves.hpp"
#include "utility.hpp"
#include "moveGeneration.hpp"
#include "search.hpp"
#include "evaluation.hpp"
#include "zobrist.hpp"

#include "transpositionTable.hpp"
#include "randomNumber.hpp"
#include "bitset"

//Consider how to deal with Search Explosion in QSearch();
//Should be using pass by reference for Move (in utility.hpp)
//Remember the effects of not correctly updating pieceBoard (in board.hpp) in make/unmake move
//Experimenting with PVS in QSearch
//Adding const to ChessBoard& param
int main() {

    initBoard();
    initMoves();
    initEvaluation();
    initZobrist();
    
    //q2k2q1/2nqn2b/1n1P1n1b/2rnr2Q/1NQ1QN1Q/3Q3B/2RQR2B/Q2K2Q1 w - - 0 1
    ChessBoard board("8/5K2/p6k/P5R1/6P1/8/2P5/8 w - - 9 66");
    //printBitboard(inBetween[A1][A8]);
    //printBitboard(inBetween[F3][F7]); 
    //printBitboard(inBetween[H1][A8]); 
    //printBitboard(inBetween[G4][D7]);
    //printBitboard(inBetween[E4][C4]); 
    //printBitboard(inBetween[A1][G8]);  
    iterativeDeepening(board, 11);


/*
    ChessBoard board;
    //divide(board, 4);/*
    auto t1 = std::chrono::high_resolution_clock::now();
    std::cout << perft(board, 6) << std::endl;
    auto t2 = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::seconds>(t2 - t1).count();
    std::cout << duration << std::endl;*/

    return 0; 
}
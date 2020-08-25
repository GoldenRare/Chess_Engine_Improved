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

//Need to update age for Transposition Table before the next search
//Consider how to handle mates and also think about evaluating positions in check with the stand pat score
//Should be using pass by reference for Move (in utility.hpp)
//Remember the effects of not correctly updating pieceBoard (in board.hpp) in make/unmake move
int main() {

    initBoard();
    initMoves();
    initEvaluation();
    initZobrist();
    
    //q2k2q1/2nqn2b/1n1P1n1b/2rnr2Q/1NQ1QN1Q/3Q3B/2RQR2B/Q2K2Q1 w - - 0 1
    ChessBoard board("1r1b4/2p2N2/pp1k1n2/q7/3PR3/P4Q2/6P1/2R3K1 b - - 2 52"); 
    iterativeDeepening(board, 7);


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
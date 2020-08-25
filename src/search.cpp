#include <iostream>
#include <chrono>
#include <cstdlib>
#include <algorithm>
#include "search.hpp"
#include "utility.hpp"
#include "board.hpp"
#include "moveGeneration.hpp"
#include "evaluation.hpp"
#include "transpositionTable.hpp"
#include "moveSorter.hpp"

#include <cassert>

//("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1") - Position 1 (Start Position): Depth 6: 119060324
//("r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq -") - Position 2: Depth 5: 193690690
//("8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - -") - Position 3: Depth 7: 178633661
//("r3k2r/Pppp1ppp/1b3nbN/nP6/BBP1P3/q4N2/Pp1P2PP/R2Q1RK1 w kq - 0 1") - Position 4: Depth 6: 706045033
//("r2q1rk1/pP1p2pp/Q4n2/bbp1p3/Np6/1B3NBn/pPPP1PPP/R3K2R b KQ - 0 1") - Position 4 (flipped): Depth 6: 706045033
//("rnbq1k1r/pp1Pbppp/2p5/8/2B5/8/PPP1NnPP/RNBQK2R w KQ - 1 8") - Position 5: Depth 5: 89941194
//("r4rk1/1pp1qppp/p1np1n2/2b1p1B1/2B1P1b1/P1NP1N2/1PP1QPPP/R4RK1 w - - 0 10") - Position 6: Depth 5: 164075551
Bitboard perft(ChessBoard& board, int depth) {

    if (depth == 0) return 1ULL;

    Move movesList[256];
    Move* movesListStart = movesList;
    Move* movesListEnd = generateAllPseudoMoves(board, movesListStart);
    Bitboard nodes = 0;
    Color kingInCheck = board.sideToPlay;
    Piece king        = (kingInCheck == WHITE) ? WHITE_KING : BLACK_KING;

    PositionKey before = board.positionKey;
    while (movesListStart < movesListEnd) {

        board.makeMove(*movesListStart);
        assert(before != board.positionKey);
        
        if (!board.isSquareAttacked(board.pieceSquare[king][0], kingInCheck)){
            nodes += perft(board, depth - 1);
        }
        board.undoMove();
        assert(before == board.positionKey);
        movesListStart++;
    }
    return nodes;
}

void divide(ChessBoard& board, int depth) {

    Move movesList[256];
    Move* movesListStart = movesList;
    Move* movesListEnd = generateAllPseudoMoves(board, movesListStart);
    Bitboard result, total;
    result = total = 0;

    while (movesListStart < movesListEnd) {

        board.makeMove(*movesListStart);
        Color kingInCheck = ~board.sideToPlay;
        Piece king = (kingInCheck == WHITE) ? WHITE_KING : BLACK_KING;
        if (!board.isSquareAttacked(board.pieceSquare[king][0], kingInCheck)){
            result = perft(board, depth);
            std::cout << getFrom(*movesListStart) << " " << getTo(*movesListStart) << " " << getFlags(*movesListStart) << ": " << result << std::endl;
        } else result = 0;
        board.undoMove();
        total += result;
        movesListStart++;
    }
    std::cout << "Total: " << total;
}

ExactScore alphaBeta(ChessBoard& board, int alpha, int beta, int depth) {

    if (depth == 0) 
        return quiescenceSearch(board, alpha, beta);

    ///////////////// Transposition Table ///////////////////
    bool hasEvaluation;
    PositionEvaluation* pe = TT.probeTT(board.positionKey, hasEvaluation);
    unsigned int hashMove = 0;

    if (hasEvaluation) {

        if (depth <= pe->depth) {

            if (pe->getBound() == EXACT_BOUND) 
                return (pe->evaluation == -MATE_SCORE) ? -MATE_SCORE + board.ply : pe->evaluation;
            else if ((pe->getBound() == LOWER_BOUND) && pe->evaluation > alpha) alpha = pe->evaluation;
            else if ((pe->getBound() == UPPER_BOUND) && pe->evaluation < beta ) beta  = pe->evaluation; 

            if (alpha >= beta) return beta;

        } 
        hashMove = pe->move;
    }
    //////////////////////////////////////////////////////////

    /////////////// Erase Killers of Next Ply ////////////////
    killerMoves[board.ply + 1][0] = 0;
    killerMoves[board.ply + 1][1] = 0;
    //////////////////////////////////////////////////////////

    Move movesList[256];
    Move* movesListStart = movesList;
    Move* movesListEnd = generateAllPseudoMoves(board, movesListStart);

    Color kingInCheck = board.sideToPlay;
    Piece king        = (kingInCheck == WHITE) ? WHITE_KING : BLACK_KING;

    int evaluation = 0;
    int legalMoves = 0;
    int bestEvaluation = -INFINITE;
    int bestMove;
    assignMoveScores(movesListStart, movesListEnd, board);
    findBestMove(movesListStart, movesListEnd, hashMove);
    while (movesListStart < movesListEnd) {

        board.makeMove(*movesListStart);
        
        //If move is illegal, don't evaluate
        if (board.isSquareAttacked(board.pieceSquare[king][0], kingInCheck)) {

            board.undoMove();
            movesListStart++;
            findBestMove(movesListStart, movesListEnd, hashMove);
            continue;

        }
        legalMoves++;
        evaluation = -alphaBeta(board, -beta, -alpha, depth - 1);
        board.undoMove();

        if (evaluation >= beta) {
            pe->savePositionEvaluation(board.positionKey, movesListStart->move, depth, LOWER_BOUND, evaluation);

            ///////////// Save Killers /////////////////
            if (isQuietMove(*movesListStart)) {
                //Favour more recent beta cutoffs
                if (movesListStart->move != killerMoves[board.ply][0]) {
                    killerMoves[board.ply][1] = killerMoves[board.ply][0];
                    killerMoves[board.ply][0] = movesListStart->move;
                }
            }
            ////////////////////////////////////////////

            return beta;
        }
        if (evaluation > bestEvaluation) {
            bestMove = movesListStart->move;
            bestEvaluation = evaluation;
        }
        movesListStart++;
        findBestMove(movesListStart, movesListEnd, hashMove);
    }

    //////////////// Checkmate or Stalemate /////////////////
    if(legalMoves == 0) {
        if (board.isSquareAttacked(board.pieceSquare[king][0], kingInCheck)) 
            alpha = -MATE_SCORE + board.ply;
        else alpha = STALEMATE_SCORE;
        pe->savePositionEvaluation(board.positionKey, 0, depth, EXACT_BOUND, (alpha == STALEMATE_SCORE) ? STALEMATE_SCORE : -MATE_SCORE);
    }
    ////////////////////////////////////////////////////////

    ///////////////// PV Node //////////////////////////////
    else if (bestEvaluation > alpha) {
        alpha = bestEvaluation;
        pe->savePositionEvaluation(board.positionKey, bestMove, depth, EXACT_BOUND, alpha);
    } 
    ///////////////////////////////////////////////////////

    /////////////// Fail Low //////////////////////////////
    else pe->savePositionEvaluation(board.positionKey, bestMove, depth, UPPER_BOUND, alpha);
    ///////////////////////////////////////////////////////

    return alpha;
}

void iterativeDeepening(ChessBoard& board, int maxDepth) {

    auto t1 = std::chrono::high_resolution_clock::now();
    int alpha = -INFINITE;
    int beta  = INFINITE;
    int delta = -INFINITE;
    ExactScore evaluation;

    TT.updateAge();
    for (int depth = 1; depth <= maxDepth; depth++) {
        evaluation = alphaBeta(board, alpha, beta, depth);

        //Fails Low
        if (evaluation <= alpha) {
            delta += (delta / 4) + 5;
            alpha = std::max(evaluation - delta, -INFINITE);
            depth--;
            continue;
        } else if (evaluation >= beta) { //Fails High
            delta += (delta / 4) + 5;
            beta = std::min(evaluation + delta, INFINITE);
            depth--;
            continue;
        }

        delta = (abs(evaluation) / 256) + 21;
        alpha = std::max(evaluation - delta, -INFINITE);
        beta  = std::min(evaluation + delta,  INFINITE);
    }

    bool hasEvaluation = false;
    PositionEvaluation* pe = TT.probeTT(board.positionKey, hasEvaluation);
    Move bestMoveInfo;
    bestMoveInfo.move = pe->move;
    std::cout << "Score: " << pe->evaluation << ". Move: " << NOTATION[getFrom(bestMoveInfo)] << " " << NOTATION[getTo(bestMoveInfo)] << std::endl;
    auto t2 = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::seconds>(t2 - t1).count();
    std::cout << duration << std::endl;
}

ExactScore quiescenceSearch(ChessBoard& board, int alpha, int beta) {

    Color kingInCheck = board.sideToPlay;
    Piece king        = (kingInCheck == WHITE) ? WHITE_KING : BLACK_KING;

    /////////////// Erase Killers of Next Ply ////////////////
    killerMoves[board.ply + 1][0] = 0;
    killerMoves[board.ply + 1][1] = 0;
    //////////////////////////////////////////////////////////

    Move movesList[256];
    Move* movesListStart = movesList;
    Move* movesListEnd;
    bool generatedAllMoves = false;
    /* SEARCH EXPLOSION BECAUSE OF GENERATING ALL MOVES WHEN NOT IN CHECK */
    ////////////Stand-Pat if not in check////////////////////////////
    if (!board.isSquareAttacked(board.pieceSquare[king][0], kingInCheck)) {

        ExactScore standPat = evaluatePosition(board);
        if (standPat >= beta) return beta;
        if (standPat > alpha) alpha = standPat;
        movesListEnd = generateAllPseudoCaptureMoves(board, movesListStart);

    } else {
        movesListEnd = generateAllPseudoMoves(board, movesListStart);
        generatedAllMoves = true;
    }
    //////////////////////////////////////////////////////////////////

    int evaluation = 0;
    int legalMoves = 0;
    unsigned int temp = 0;

    assignMoveScores(movesListStart, movesListEnd, board);
    findBestMove(movesListStart, movesListEnd, temp);
    while (movesListStart < movesListEnd) {

        board.makeMove(*movesListStart);
        
        //If move is illegal, don't evaluate
        if (board.isSquareAttacked(board.pieceSquare[king][0], kingInCheck)) {

            board.undoMove();
            movesListStart++;
            findBestMove(movesListStart, movesListEnd, temp);
            continue;

        }
        legalMoves++;

        evaluation = -quiescenceSearch(board, -beta, -alpha);
        board.undoMove();

        if (evaluation >= beta) {

            ///////////// Save Killers /////////////////
            //This situation can occur if we have to generateAllPseudoMoves
            if (isQuietMove(*movesListStart)) {
                //Favour more recent beta cutoffs
                if (movesListStart->move != killerMoves[board.ply][0]) {
                    killerMoves[board.ply][1] = killerMoves[board.ply][0];
                    killerMoves[board.ply][0] = movesListStart->move;
                }
            }
            ////////////////////////////////////////////

            return beta;
        }
        if (evaluation > alpha) alpha = evaluation;
        movesListStart++;
        findBestMove(movesListStart, movesListEnd, temp);
    }

    if ((legalMoves == 0) && generatedAllMoves) {
        if (board.isSquareAttacked(board.pieceSquare[king][0], kingInCheck)) 
            alpha = -MATE_SCORE + board.ply;
        else alpha = STALEMATE_SCORE;
    }
    return alpha;
}

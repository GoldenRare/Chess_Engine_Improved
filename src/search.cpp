#include <iostream>
#include <chrono>
#include <cstdlib>
#include <algorithm>
#include <string>
#include <cmath>
#include "search.hpp"
#include "utility.hpp"
#include "board.hpp"
#include "moveGeneration.hpp"
#include "evaluation.hpp"
#include "transpositionTable.hpp"
#include "moveSorter.hpp"
#include "moves.hpp"

#include <cassert>

int REDUCTIONS[256];
void initSearch() {

    initReductions();

}

void initReductions() {

    REDUCTIONS[0] = 0;
    for (int i = 1; i < 256; i++)
        REDUCTIONS[i] = int(24.8 * std::log(i));

}

int reductions(int depth, int numberOfMoves) {

    int reductions = REDUCTIONS[depth] * REDUCTIONS[numberOfMoves];
    return (reductions + 511) / 1024;
}

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

ExactScore alphaBeta(ChessBoard& board, int alpha, int beta, int depth, bool isPVNode, bool isCutNode, int ply) {

    const bool isRootNode = board.ply == ply;

    ////////// Check For Repetition/50-Move rule ////////////
    if (isRepetition(board) || board.halfmoves >= 100) return 0;
    /////////////////////////////////////////////////////////
    
    if (depth <= 0) 
        return quiescenceSearch(board, alpha, beta, isPVNode);

    //////////////// Mate Distance Pruning //////////////////
    ExactScore assumeCheckmate = CHECKMATE - board.ply;
    if (assumeCheckmate < beta) {
        beta = assumeCheckmate;
        if (alpha >= assumeCheckmate) return assumeCheckmate;
    }

    ExactScore assumeCheckmated = -CHECKMATE + board.ply;
    if (assumeCheckmated > alpha) {
        alpha = assumeCheckmated;
        if (beta <= assumeCheckmated) return assumeCheckmated;
    }
    /////////////////////////////////////////////////////////

    ///////////////// Transposition Table ///////////////////
    bool hasEvaluation;
    PositionEvaluation* pe = TT.probeTT(board.positionKey, hasEvaluation);
    unsigned int hashMove = 0;
    Move savedHashMove;
    ExactScore nodeScoreTT = NO_VALUE;
    bool isPVNodeTT = isPVNode;

    if (hasEvaluation) {

        if (depth <= pe->depth) {

            ExactScore temp = adjustNodeScoreFromTT(pe->nodeScore, board.ply);
            if (pe->getBound() == EXACT_BOUND) 
                return temp;
            else if ((pe->getBound() == LOWER_BOUND) && temp > alpha) alpha = temp;
            else if ((pe->getBound() == UPPER_BOUND) && temp < beta ) beta  = temp; 

            if (alpha >= beta) return beta;

        }
        isPVNodeTT = pe->isPVNode() ? pe->isPVNode() : isPVNodeTT;
        nodeScoreTT = adjustNodeScoreFromTT(pe->nodeScore, board.ply); 
        hashMove = (pe->move == NO_MOVE) ? 0 : pe->move;
    }
    savedHashMove.move = hashMove;
    //////////////////////////////////////////////////////////

    /////////////// Erase Killers of Next Ply ////////////////
    killerMoves[board.ply + 1][0] = 0;
    killerMoves[board.ply + 1][1] = 0;
    //////////////////////////////////////////////////////////

    Color kingInCheck = board.sideToPlay;
    Piece king        = (kingInCheck == WHITE) ? WHITE_KING : BLACK_KING;
    ExactScore staticEvaluation, staticEvaluationAdjusted;
    ///////////////// Static Evaluation //////////////////////
    if (board.isSquareAttacked(board.pieceSquare[king][0], kingInCheck)) 
        staticEvaluationAdjusted = staticEvaluation = NO_VALUE;
        
    else if (hasEvaluation) {

        staticEvaluationAdjusted = staticEvaluation = pe->staticEvaluation;
        if (staticEvaluation == NO_VALUE) {
            Evaluation eval(board);
            staticEvaluationAdjusted = staticEvaluation = eval.evaluatePosition();
        }

        // nodeScoreTT can potentially be used as better score than the actual static evaluation
        if ((nodeScoreTT != NO_VALUE) && (pe->getBound() & (nodeScoreTT > staticEvaluation ? LOWER_BOUND : UPPER_BOUND) > 0))
            staticEvaluationAdjusted = nodeScoreTT;

    } else {

        Evaluation eval(board);
        staticEvaluationAdjusted = staticEvaluation = eval.evaluatePosition();
        pe->savePositionEvaluation(board.positionKey, NO_MOVE, 0, isPVNodeTT, NO_BOUND, staticEvaluation, NO_VALUE);

    }
    //////////////////////////////////////////////////////////

    ////////////////////// Razoring //////////////////////////
    if (!isRootNode && (depth < 2) && (staticEvaluationAdjusted <= (alpha - RAZOR_MARGIN)))
        return quiescenceSearch(board, alpha, beta, isPVNode);
    //////////////////////////////////////////////////////////

    ////////////////// Futility Pruning //////////////////////
    if (!isPVNode && (depth < 6) && (staticEvaluationAdjusted - (217 * depth) >= beta) && (staticEvaluationAdjusted < 10000))
        return staticEvaluationAdjusted;
    //////////////////////////////////////////////////////////

    ////////// Internal Iterative Reductions (Play on Internal Iterative Deepening) //////////
    if (isPVNode && (depth >= 6) && (hashMove == NO_MOVE)) depth -= 2;
    //////////////////////////////////////////////////////////////////////////////////////////

    Move movesList[256];
    Move* movesListStart = movesList;
    Move* movesListEnd = generateAllPseudoMoves(board, movesListStart);

    bool givesCheck, captureOrPromotionMove, doFullDepthSearch;
    bool moveCountPruning = false;
    int nodeScore = -INFINITE;
    int legalMoves = 0;
    int bestNodeScore = -INFINITE;
    int bestMove = NO_MOVE;

    assignMoveScores(movesListStart, movesListEnd, board);
    findBestMove(movesListStart, movesListEnd, hashMove);
    while (movesListStart < movesListEnd) {

        legalMoves++;
        givesCheck = board.givesCheck(*movesListStart);
        captureOrPromotionMove = isCapture(*movesListStart) || isPromotion(*movesListStart);
        ////////////// Pruning Before Making Move //////////////
        if (!isRootNode && board.nonPawnMaterial[board.sideToPlay] > 0 && bestNodeScore > GUARANTEE_CHECKMATED) {

            moveCountPruning = legalMoves >= ((5 + depth * depth) / 2 - 1);
            if (!givesCheck && !captureOrPromotionMove) {
                
                int lateMoveReductionDepth = std::max(depth - 1 - reductions(depth, legalMoves), 0);
                if (!SEE(board, *movesListStart, -(32 - std::min(lateMoveReductionDepth, 18)) * lateMoveReductionDepth * lateMoveReductionDepth)) {
                    movesListStart++;
                    findBestMove(movesListStart, movesListEnd, hashMove);
                    continue;
                }
            } else if (!SEE(board, *movesListStart, -194 * depth)) {
                movesListStart++;
                findBestMove(movesListStart, movesListEnd, hashMove);
                continue;
            }
        }
        ////////////////////////////////////////////////////////

        board.makeMove(*movesListStart);
        
        //If move is illegal, don't evaluate
        if (board.isSquareAttacked(board.pieceSquare[king][0], kingInCheck)) {

            legalMoves--;
            board.undoMove();
            movesListStart++;
            findBestMove(movesListStart, movesListEnd, hashMove);
            continue;

        }
        
        ////////////// Reduced Depth Search and Principal Variation Search ////////////////
        if (   depth >= 3 
            && legalMoves > 1 + (isRootNode ? 1 : 0) + ((isRootNode && bestNodeScore < alpha) ? 1 : 0)
            && !isRootNode 
            && (!captureOrPromotionMove || moveCountPruning || isCutNode)) {

            int reduction = reductions(depth, legalMoves);

            if (isPVNodeTT) reduction -= 2;
            if (!captureOrPromotionMove) {

                if (isCapture(savedHashMove)) reduction++;
                if (isCutNode) reduction += 2;

            } else if (depth < 8 && legalMoves > 2) reduction++;

            int reducedDepth = keepInRange(1, depth - 1, depth - reduction);
            nodeScore = -alphaBeta(board, -alpha - 1, -alpha, reducedDepth, false, true, ply + 1); //Search with null window
            doFullDepthSearch = (nodeScore > alpha && reducedDepth != depth - 1);

        } else doFullDepthSearch = !isPVNode || legalMoves > 1;

        if (doFullDepthSearch) 
            nodeScore = -alphaBeta(board, -alpha - 1, -alpha, depth - 1, false, !isCutNode, ply + 1); //Search with null window

        if (isPVNode && ((legalMoves == 1) || ((nodeScore > alpha) && (nodeScore < beta)))) 
            nodeScore = -alphaBeta(board, -beta, -alpha, depth - 1, true, false, ply + 1);
        /////////////////////////////////////////////////////////////////////////////////////

        board.undoMove();

        if (nodeScore > bestNodeScore) {
            
            bestNodeScore = nodeScore;

            if (bestNodeScore > alpha) {
                
                bestMove = movesListStart->move;

                if (isPVNode && (bestNodeScore < beta)) alpha = bestNodeScore;
                else {

                    ///////////// Save Killers /////////////////
                    if (isQuietMove(*movesListStart)) {
                        //Favour more recent beta cutoffs
                        if (movesListStart->move != killerMoves[board.ply][0]) {
                            killerMoves[board.ply][1] = killerMoves[board.ply][0];
                            killerMoves[board.ply][0] = movesListStart->move;
                        }
                    }
                    ////////////////////////////////////////////
                    break;
                }
            }
        }

        movesListStart++;
        findBestMove(movesListStart, movesListEnd, hashMove);
    }

    //////////////// Checkmate or Stalemate /////////////////
    if(legalMoves == 0) {
        if (board.isSquareAttacked(board.pieceSquare[king][0], kingInCheck)) 
            bestNodeScore = -CHECKMATE + board.ply;
        else bestNodeScore = STALEMATE;
    }
    ////////////////////////////////////////////////////////

    pe->savePositionEvaluation(board.positionKey, bestMove, depth, isPVNodeTT, 
                               bestNodeScore >= beta ? LOWER_BOUND : isPVNode && (bestMove != NO_MOVE) ? EXACT_BOUND : UPPER_BOUND, 
                               staticEvaluation, adjustNodeScoreToTT(bestNodeScore, board.ply));
    return bestNodeScore;
}

void iterativeDeepening(ChessBoard& board, int maxDepth) {

    auto t1 = std::chrono::high_resolution_clock::now();
    ExactScore evaluation;
    int alpha, beta, delta;
    alpha = delta = -INFINITE;
    beta  = INFINITE;
    
    TT.updateAge(); // Starting a new search so update the age for PositionEvaluations that are to be accessed
    for (int depth = 1; depth <= maxDepth; depth++) {

        // No point in running into fail highs/lows for early depths(since they can already be computed fast with the wide windows).
        // Thus at depth 4 and greater, start setting the size of the aspiration windows. 
        if (depth >= 4) {
            delta = abs(evaluation) / 256 + 21;
            alpha = std::max(evaluation - delta,    -INFINITE) ;
            beta  = std::min(evaluation + delta, int(INFINITE));
        }

        bool notInWindow = true;
        int failedHighCount = 0;
        while (notInWindow) {
            // In the event of a fail high, we search at a shallower depth with a greater window since move ordering 
            // will have been messed up due to the fail high. 
            int adjustedDepth = std::max(1, depth - failedHighCount); 
            evaluation = alphaBeta(board, alpha, beta, adjustedDepth, true, board.ply, false);

            // Fails Low
            if (evaluation <= alpha) {
                beta = (alpha + beta) / 2;
                alpha = std::max(evaluation - delta, -INFINITE);
                failedHighCount = 0;
            } else if (evaluation >= beta) { // Fails High
                beta = std::min(evaluation + delta, int(INFINITE));
                failedHighCount++;
            } else notInWindow = false;

            delta += delta / 4 + 5;
        }  
    }
    auto t2 = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1).count();

    bool hasEvaluation = false;
    PositionEvaluation* pe = TT.probeTT(board.positionKey, hasEvaluation);
    std::string promotion = "";
    Move bestMoveInfo;
    MoveType moveType;
    bestMoveInfo.move = pe->move;
    ////////////////////////////////////
    moveType = typeOfMove(bestMoveInfo);

    if      ((moveType == KNIGHT_PROMOTION) || (moveType == KNIGHT_PROMOTION_CAPTURE)) promotion = "n";
    else if ((moveType == BISHOP_PROMOTION) || (moveType == BISHOP_PROMOTION_CAPTURE)) promotion = "b";
    else if ((moveType == ROOK_PROMOTION  ) || (moveType == ROOK_PROMOTION_CAPTURE  )) promotion = "r";
    else if ((moveType == QUEEN_PROMOTION ) || (moveType == QUEEN_PROMOTION_CAPTURE )) promotion = "q";
    ///////////////////////////////////////
    std::cout << "info depth " << maxDepth << " time " << duration << " score cp " << pe->nodeScore << std::endl;
    std::cout << "bestmove " << NOTATION[getFrom(bestMoveInfo)] << NOTATION[getTo(bestMoveInfo)];
    if (promotion != "") std::cout << promotion << std::endl;
    else std::cout << std::endl;
    ////////////////////////////////////////

}

ExactScore quiescenceSearch(ChessBoard& board, int alpha, int beta, bool isPVNode) {

    bool inCheck;
    Color kingInCheck = board.sideToPlay;
    Piece king        = (kingInCheck == WHITE) ? WHITE_KING : BLACK_KING;

    /////////////// Erase Killers of Next Ply ////////////////
    killerMoves[board.ply + 1][0] = 0;
    killerMoves[board.ply + 1][1] = 0;
    //////////////////////////////////////////////////////////

    Move movesList[256];
    Move* movesListStart = movesList;
    Move* movesListEnd;
    int oldAlpha;

    if (isPVNode) oldAlpha = alpha;
    ////////// Check For Repetition/50-Move rule ////////////
    if (isRepetition(board) || board.halfmoves >= 100) return 0;
    /////////////////////////////////////////////////////////

    ///////////////// Transposition Table ///////////////////
    bool hasEvaluation;
    PositionEvaluation* pe = TT.probeTT(board.positionKey, hasEvaluation);
    unsigned int hashMove = 0;
    ExactScore nodeScoreTT = NO_VALUE;
    bool isPVNodeTT = false;

    if (hasEvaluation) {

        if (0 <= pe->depth && !isPVNode) {

            ExactScore temp = adjustNodeScoreFromTT(pe->nodeScore, board.ply);
            if (pe->getBound() == EXACT_BOUND) 
                return temp;
            else if ((pe->getBound() == LOWER_BOUND) && temp > alpha) alpha = temp;
            else if ((pe->getBound() == UPPER_BOUND) && temp < beta ) beta  = temp; 

            if (alpha >= beta) return beta;

        }
        isPVNodeTT = pe->isPVNode();
        nodeScoreTT = adjustNodeScoreFromTT(pe->nodeScore, board.ply); 
        hashMove = (pe->move == NO_MOVE) ? 0 : pe->move;
    }
    //////////////////////////////////////////////////////////
    
    ExactScore staticEvaluation, bestNodeScore;
    int futilityBase, futilityValue;
    inCheck = board.isSquareAttacked(board.pieceSquare[king][0], kingInCheck);
    ///////////////// Static Evaluation //////////////////////
    if (inCheck) { 
        staticEvaluation = NO_VALUE;
        futilityBase = bestNodeScore = -INFINITE;
        movesListEnd = generateAllPseudoMoves(board, movesListStart);
    }
    else {

        if (hasEvaluation) {

            bestNodeScore = staticEvaluation = pe->staticEvaluation;
            if (staticEvaluation == NO_VALUE) {
                Evaluation eval(board);
                bestNodeScore = staticEvaluation = eval.evaluatePosition();
            }

            // nodeScoreTT can potentially be used as better score than the actual static evaluation
            if ((nodeScoreTT != NO_VALUE) && (pe->getBound() & (nodeScoreTT > staticEvaluation ? LOWER_BOUND : UPPER_BOUND) > 0))
                bestNodeScore = nodeScoreTT;
        } else {
            
            Evaluation eval(board);
            bestNodeScore = staticEvaluation = eval.evaluatePosition();
        }

        if (bestNodeScore >= beta) {

            pe->savePositionEvaluation(board.positionKey, NO_MOVE, 0, isPVNodeTT, LOWER_BOUND, staticEvaluation, adjustNodeScoreToTT(bestNodeScore, board.ply));
            return bestNodeScore;
        }

        if (isPVNode && bestNodeScore > alpha) alpha = bestNodeScore;
        futilityBase = bestNodeScore + 154;
        movesListEnd = generateAllPseudoCaptureMoves(board, movesListStart);
    } 
    //////////////////////////////////////////////////////////

    ExactScore nodeScore = -INFINITE;
    int movesChecked = 0;
    int bestMove = NO_MOVE;
    bool nonCaptureCheckEvasionPrunable, givesCheck;

    assignMoveScores(movesListStart, movesListEnd, board);
    findBestMove(movesListStart, movesListEnd, hashMove);
    while (movesListStart < movesListEnd) {

        movesChecked++;
        givesCheck = board.givesCheck(*movesListStart);
        /////////////////// Futility Pruning /////////////////////
        if (!inCheck && !givesCheck && futilityBase > GUARANTEE_LOSS && !board.advancedPawnPush(*movesListStart)) {

            futilityValue = futilityBase + board.endgameValueOfPiece(*movesListStart);
            if (futilityValue <= alpha) {

                bestNodeScore = std::max(bestNodeScore, futilityValue);
                movesListStart++;
                findBestMove(movesListStart, movesListEnd, hashMove);
                continue;
                
            }

            if (futilityBase <= alpha && !SEE(board, *movesListStart, 1)) {

                bestNodeScore = std::max(bestNodeScore, futilityBase);
                movesListStart++;
                findBestMove(movesListStart, movesListEnd, hashMove);
                continue;

            }
        } 
        //////////////////////////////////////////////////////////

        nonCaptureCheckEvasionPrunable = inCheck && movesChecked > 2 && bestNodeScore > GUARANTEE_CHECKMATED && isQuietMove(*movesListStart);
        if ((!inCheck || nonCaptureCheckEvasionPrunable) && !SEE(board, *movesListStart, 0)) {

            movesListStart++;
            findBestMove(movesListStart, movesListEnd, hashMove);
            continue;
        }
        
        board.makeMove(*movesListStart);
        
        //If move is illegal, don't evaluate
        if (board.isSquareAttacked(board.pieceSquare[king][0], kingInCheck)) {

            board.undoMove();
            movesListStart++;
            movesChecked--;
            findBestMove(movesListStart, movesListEnd, hashMove);
            continue;

        }
        nodeScore = -quiescenceSearch(board, -beta, -alpha, isPVNode);
        board.undoMove();

        if (nodeScore > bestNodeScore) {
            
            bestNodeScore = nodeScore;

            if (bestNodeScore > alpha) {
                
                bestMove = movesListStart->move;

                if (isPVNode && (bestNodeScore < beta)) alpha = bestNodeScore;
                else {

                    ///////////// Save Killers /////////////////
                    if (isQuietMove(*movesListStart)) {
                        //Favour more recent beta cutoffs
                        if (movesListStart->move != killerMoves[board.ply][0]) {
                            killerMoves[board.ply][1] = killerMoves[board.ply][0];
                            killerMoves[board.ply][0] = movesListStart->move;
                        }
                    }
                    ////////////////////////////////////////////
                    break;
                }
            }
        }
        movesListStart++;
        findBestMove(movesListStart, movesListEnd, hashMove);
    }

    if (inCheck && bestNodeScore == -INFINITE) {
        if (board.isSquareAttacked(board.pieceSquare[king][0], kingInCheck)) 
            alpha = -CHECKMATE + board.ply;
        else alpha = STALEMATE;
    }

    pe->savePositionEvaluation(board.positionKey, bestMove, 0, isPVNodeTT, 
                               bestNodeScore >= beta ? LOWER_BOUND : isPVNode && (bestNodeScore > oldAlpha) ? EXACT_BOUND : UPPER_BOUND, 
                               staticEvaluation, adjustNodeScoreToTT(bestNodeScore, board.ply));
    return bestNodeScore;
}

bool isRepetition(ChessBoard& board) {

    if (board.halfmoves < 4) return false;

    //int currentPosition = board.previousGameStatesCount - 1;
    int howFarBack = board.previousGameStatesCount - board.halfmoves;
    for (int i = board.previousGameStatesCount - 4; (i >= 0) && (i >= howFarBack); i -= 2) 
        if (board.positionKey == board.previousGameStates[i].key) return true;

    return false;
}

bool SEE(ChessBoard& board, Move move, int materialValue) {

    if (!isSEECapture(move)) {
        return 0 >= materialValue;
    }

    Square fromSquare = Square(getFrom(move));
    Square toSquare   = Square(getTo(move));

    int swapOffValue = PIECE_VALUE[MIDDLEGAME][board.pieceBoard[toSquare]] - materialValue;
    if (swapOffValue < 0) return false;

    swapOffValue = PIECE_VALUE[MIDDLEGAME][board.pieceBoard[fromSquare]] - swapOffValue;
    if (swapOffValue <= 0) return true;

    Bitboard kingBlockers[COLOURS];
    Bitboard slidersPinningEnemyKing[COLOURS];
    //////////////////////////////////////
    Bitboard pawns   = board.pieces[WHITE_PAWN  ] | board.pieces[BLACK_PAWN  ];
    Bitboard knights = board.pieces[WHITE_KNIGHT] | board.pieces[BLACK_KNIGHT];
    Bitboard bishops = board.pieces[WHITE_BISHOP] | board.pieces[BLACK_BISHOP];
    Bitboard rooks   = board.pieces[WHITE_ROOK  ] | board.pieces[BLACK_ROOK  ];
    Bitboard queens  = board.pieces[WHITE_QUEEN ] | board.pieces[BLACK_QUEEN ];

    kingBlockers[WHITE] = board.blockers(board.pieceSquare[WHITE_KING][0], board.piecesOnSide[BLACK], slidersPinningEnemyKing[BLACK]);
    kingBlockers[BLACK] = board.blockers(board.pieceSquare[BLACK_KING][0], board.piecesOnSide[WHITE], slidersPinningEnemyKing[WHITE]);
    //////////////////////////////////////

    Bitboard occupied = board.occupiedSquares ^ squareToBitboard(fromSquare) ^ squareToBitboard(toSquare);
    Color sideOfInterest = board.sideToPlay;
    Bitboard attackers = board.attackersToSquare(toSquare, occupied);
    Bitboard sideOfInterestAttackers, leastValuableAttacker;
    int result = 1;

    while (true) {

        sideOfInterest = ~sideOfInterest;
        attackers &= occupied;

        sideOfInterestAttackers = attackers & board.piecesOnSide[sideOfInterest];
        if (sideOfInterestAttackers == 0) break;

        if ((slidersPinningEnemyKing[~sideOfInterest] & occupied) > 0)
            sideOfInterestAttackers &= ~kingBlockers[sideOfInterest];

        if (sideOfInterestAttackers == 0) break;

        result ^= 1;

        leastValuableAttacker = sideOfInterestAttackers & pawns;
        if (leastValuableAttacker > 0) {

            swapOffValue = PAWN_VALUE_MIDDLEGAME - swapOffValue;
            if (swapOffValue < result) break;

            occupied ^= squareToBitboard(squareOfLS1B(leastValuableAttacker));
            attackers |= bishopAttacks(occupied, toSquare) & (bishops | queens);
            continue;

        }

        leastValuableAttacker = sideOfInterestAttackers & knights;
        if (leastValuableAttacker > 0) {

            swapOffValue = KNIGHT_VALUE_MIDDLEGAME - swapOffValue;
            if (swapOffValue < result) break;

            occupied ^= squareToBitboard(squareOfLS1B(leastValuableAttacker));
            continue;
            
        }

        leastValuableAttacker = sideOfInterestAttackers & bishops;
        if (leastValuableAttacker > 0) {

            swapOffValue = BISHOP_VALUE_MIDDLEGAME - swapOffValue;
            if (swapOffValue < result) break;

            occupied ^= squareToBitboard(squareOfLS1B(leastValuableAttacker));
            attackers |= bishopAttacks(occupied, toSquare) & (bishops | queens);
            continue;
            
        }

        leastValuableAttacker = sideOfInterestAttackers & rooks;
        if (leastValuableAttacker > 0) {

            swapOffValue = ROOK_VALUE_MIDDLEGAME - swapOffValue;
            if (swapOffValue < result) break;

            occupied ^= squareToBitboard(squareOfLS1B(leastValuableAttacker));
            attackers |= rookAttacks(occupied, toSquare) & (rooks | queens);
            continue;
            
        }

        leastValuableAttacker = sideOfInterestAttackers & queens;
        if (leastValuableAttacker > 0) {

            swapOffValue = QUEEN_VALUE_MIDDLEGAME - swapOffValue;
            if (swapOffValue < result) break;

            occupied ^= squareToBitboard(squareOfLS1B(leastValuableAttacker));
            attackers |= bishopAttacks(occupied, toSquare) & (bishops | queens) | rookAttacks(occupied, toSquare) & (rooks | queens);
            continue;
            
        }

        return (attackers & ~board.piecesOnSide[sideOfInterest]) ? result ^ 1 : result;
    }

    return result == 0 ? false : true;
}

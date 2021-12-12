#ifndef UCI_HPP
#define UCI_HPP

#include <sstream>
#include "board.hpp"

void commandsLoop();
void uciCommand();
void isReadyCommand();
void positionCommand(std::istringstream& iss, ChessBoard& board);
void goCommand(ChessBoard& board);
void parseMoves(std::istringstream& iss, ChessBoard& board);

#endif
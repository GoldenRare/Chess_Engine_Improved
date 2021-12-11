<<<<<<< HEAD
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

=======
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

>>>>>>> 61b4809289c1189e58962f19777acf3e93307f2a
#endif
#ifndef UCI_HPP
#define UCI_HPP

#include <sstream>
#include <cstdint>
#include "board.hpp"

constexpr uint64_t MIN_MB = 1;
constexpr uint64_t MAX_MB = 1ULL << 48; // numberOfBuckets = MB * (1024 * 1024 / 32), MB = 2^49 results in overflow, therefore use 2^48 

void commandsLoop();
void uciCommand();
void setOptionCommand(std::istringstream& iss);
void isReadyCommand();
void positionCommand(std::istringstream& iss, ChessBoard& board);
void goCommand(ChessBoard& board);
void parseMoves(std::istringstream& iss, ChessBoard& board);

#endif
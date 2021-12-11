#ifndef RANDOM_NUMBER_HPP
#define RANDOM_NUMBER_HPP

#include <random>
#include "board.hpp"

class Random64 {

    public:

        Random64();
        Bitboard manyZeros(); //Returns a random number with a higher concentration of ones
        Bitboard random(); 

    private:

        std::mt19937_64 eng;
        std::uniform_int_distribution<Bitboard> distribution;
        Bitboard manyZerosHelper();
            
};

extern Random64 random64;

#endif
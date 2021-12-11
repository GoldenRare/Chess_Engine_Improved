<<<<<<< HEAD
#include <random>
#include "randomNumber.hpp"
#include "board.hpp"

Random64 random64;

Random64::Random64() {

    std::random_device rd;
    std::mt19937_64 en(rd());
    std::uniform_int_distribution<Bitboard> distri(0, 0xFFFFFFFFFFFFFFFF);
    
    eng = en;
    distribution = distri;

}

Bitboard Random64::random() {
    return distribution(eng);
}

Bitboard Random64::manyZeros() {
    return manyZerosHelper() & manyZerosHelper() & manyZerosHelper();
}

Bitboard Random64::manyZerosHelper() {

    Bitboard r1, r2, r3, r4;

    r1 = distribution(eng) & 0xFFFF;
    r2 = distribution(eng) & 0xFFFF;
    r3 = distribution(eng) & 0xFFFF;
    r4 = distribution(eng) & 0xFFFF;

    return r1 | (r2 << 16) | (r3 << 32) | (r4 << 48);

=======
#include <random>
#include "randomNumber.hpp"
#include "board.hpp"

Random64 random64;

Random64::Random64() {

    std::random_device rd;
    std::mt19937_64 en(rd());
    std::uniform_int_distribution<Bitboard> distri(0, 0xFFFFFFFFFFFFFFFF);
    
    eng = en;
    distribution = distri;

}

Bitboard Random64::random() {
    return distribution(eng);
}

Bitboard Random64::manyZeros() {
    return manyZerosHelper() & manyZerosHelper() & manyZerosHelper();
}

Bitboard Random64::manyZerosHelper() {

    Bitboard r1, r2, r3, r4;

    r1 = distribution(eng) & 0xFFFF;
    r2 = distribution(eng) & 0xFFFF;
    r3 = distribution(eng) & 0xFFFF;
    r4 = distribution(eng) & 0xFFFF;

    return r1 | (r2 << 16) | (r3 << 32) | (r4 << 48);

>>>>>>> 61b4809289c1189e58962f19777acf3e93307f2a
}
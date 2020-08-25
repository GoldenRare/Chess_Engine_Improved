#ifndef TRANSPOSITION_TABLE_HPP
#define TRANSPOSITION_TABLE_HPP

#include <cstdint>
#include "utility.hpp"

constexpr int BUCKET_SIZE = 4;

// One PositionEvaluation takes 8 bytes: (16 + 16 + 8 + 8 + 16) / 8 == 8 Bytes
struct PositionEvaluation {

    uint16_t positionKey;
    uint16_t move;
    uint8_t depth;
    uint8_t ageBounds; //Bits 8-4 == relative age, Bit 3 == is PV Node, Bits 2-1 == Bounds
    int16_t evaluation;

    void savePositionEvaluation(PositionKey pk, uint16_t m, uint8_t d, uint8_t b, int16_t e);
    Bound getBound();

};

// Each bucket stores 4 positions (8 Bytes per PositionEvaluation * 4 positions == 32 Bytes)
// 32 Bytes == half a cache line (64 Bytes) for better caching
struct Bucket {
    PositionEvaluation pe[BUCKET_SIZE];
}; 

class TranspositionTable {

    public:

        PositionEvaluation* indexBucket(PositionKey key);
        PositionEvaluation* probeTT(PositionKey key, bool& hasEvaluation);
        void updateAge();
        uint8_t getAge();

    private:

        Bucket table[0x2000000] = { }; //size of transposition table is 1GB (32 Bytes per bucket * 33554432 buckets == 1GB)
        uint8_t age = 0; //To determine how far into the GAME we are (not how deep into the tree)
};

extern TranspositionTable TT;

#endif
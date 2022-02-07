#ifndef TRANSPOSITION_TABLE_HPP
#define TRANSPOSITION_TABLE_HPP

#include <cstdint>
#include "utility.hpp"
#include "evaluation.hpp"

constexpr int BUCKET_SIZE = 3;

// One PositionEvaluation takes 10 bytes: (16 + 16 + 8 + 8 + 16 + 16) / 8 == 10 Bytes
struct PositionEvaluation {
    PositionEvaluation() : positionKey(0) {}
    uint16_t positionKey;
    uint16_t move;
    uint8_t depth;
    uint8_t ageBounds; //Bits 8-4 == relative age, Bit 3 == is PV Node, Bits 2-1 == Bounds
    int16_t staticEvaluation;
    int16_t nodeScore;

    void savePositionEvaluation(PositionKey pk, uint16_t m, uint8_t d, bool pv, uint8_t b, int16_t se, int16_t ns);
    Bound getBound();
    bool isPVNode();

};

// Each bucket stores 3 positions (10 Bytes per PositionEvaluation * 3 positions == 30 Bytes)
// 30 Bytes + 2 bytes of filling == 32 Bytes
// 32 Bytes == half a cache line (64 Bytes) for better caching
struct Bucket {

    PositionEvaluation pe[BUCKET_SIZE];
    char filling[2];

}; 

class TranspositionTable {

    public:

        TranspositionTable();
        ~TranspositionTable();
        PositionEvaluation* probeTT(PositionKey key, bool& hasEvaluation);
        void updateAge();
        uint8_t getAge();
        uint64_t setSize(uint64_t MB);

    private:

        uint64_t numberOfBuckets; 
        Bucket* table;
        uint8_t age; // To determine how far into the game we are (not how deep into the tree)

        PositionEvaluation* indexBucket(PositionKey key);

};

//Returns an adjusted score in the case of having checkmate or being checkmated
ExactScore adjustNodeScoreFromTT(int16_t nodeScoreTT, int ply);

//Returns an adjusted score in the case of saving checkmates
ExactScore adjustNodeScoreToTT(int16_t nodeScoreTT, int ply);

extern TranspositionTable TT;

#endif
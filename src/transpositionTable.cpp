#include <cstdint>
#include "transpositionTable.hpp"
#include "utility.hpp"
#include "evaluation.hpp"

#include <iostream>

TranspositionTable TT;

TranspositionTable::TranspositionTable() {

    numberOfBuckets = 32768;
    table = new Bucket[numberOfBuckets]; // Default size of transposition table is 1MB (32 Bytes per bucket * 32768 buckets == 1MB)
    age = 0; 
    std::cout << numberOfBuckets << std::endl;
}

TranspositionTable::~TranspositionTable() {
    delete[] table; 
}

void TranspositionTable::updateAge() {
    age += 8; // Will eventually overflow causing it to be cyclic
}

uint8_t TranspositionTable::getAge() {
    return age;
}

uint64_t TranspositionTable::setSize(uint64_t MB) {

    numberOfBuckets = MB * (1024 * 1024 / 32);
    numberOfBuckets = squareToBitboard(squareOfMS1B(numberOfBuckets));

    delete[] table;
    table = new Bucket[numberOfBuckets];
    
    return numberOfBuckets;
}

// Returns the first PositionEvaluation for that bucket
PositionEvaluation* TranspositionTable::indexBucket(PositionKey key) {
    return &table[key & (numberOfBuckets - 1)].pe[0];
}

// Returns either a PositionEvaluation containing the key or one to replace
PositionEvaluation* TranspositionTable::probeTT(PositionKey key, bool& hasEvaluation) {

    PositionEvaluation* pePtr = indexBucket(key);
    uint16_t keyIndex = key >> 48; // PositionEvaluations store last 16 bits of a 64 bit PositionKey

    for (int i = 0; i < BUCKET_SIZE; i++) 
        //Since we do not use the entire 64 bits of key for indexing the bucket, this may sometimes lead to collisions. 
        //When collisions occur, we use the least significant 16 bits to differentiate between these collisions. 
        //With that said, these family of collisions are stored first at index 0 to index BUCKET_SIZE - 1.
        //Therefore, if pePtr[i].positionKey == 0, return it in order to store a PositionEvaluation there or
        //if it does contain the key then return it to see what we discovered in the previous evaluation of the position.
        if (pePtr[i].positionKey == keyIndex || !pePtr[i].positionKey) {

            hasEvaluation = bool(pePtr[i].positionKey);
            pePtr[i].ageBounds = age | (pePtr[i].ageBounds & 0x7); //If probed then update age since this old position is still relevant to the game
            return &pePtr[i];

        }
    

    //Since the above for loop did not return, then we have had a collision 3 times.
    //In other words, we already have 3 PositionEvaluations stored in that bucket,
    //therefore we must find which one to replace.   
    hasEvaluation = false;
    PositionEvaluation* replace = pePtr;
    for (int i = 1; i < BUCKET_SIZE; i++) 
        //Favour PositionEvaluations done at a higher depth but penalize entries for being old
        if (replace->depth - ((263 + age - replace->ageBounds) & 0xF8) >
            pePtr[i].depth - ((263 + age - pePtr[i].ageBounds) & 0xF8))
            replace = &pePtr[i];
    

    return replace;
}

void PositionEvaluation::savePositionEvaluation(PositionKey pk, uint16_t m, uint8_t d, bool pv, uint8_t b, int16_t se, int16_t ns) {

    uint16_t keyIndex = pk >> 48;

    //If we have a move simply overwrite, otherwise if the keys aren't the same (due to our replacement scheme) then overwrite the old move
    //for the old position (if this case happens we will be simply erasing the old move to 0). 
    if (m || keyIndex != positionKey)
        move = m;

    //Requirements in order to replace a PositionEvaluation
    if (keyIndex != positionKey || d > depth - 4 || b == EXACT_BOUND) {

        positionKey = keyIndex;
        depth = d;
        ageBounds = TT.getAge() | uint8_t(pv) << 2 | b;
        staticEvaluation = se;
        nodeScore = ns;

    }
}

Bound PositionEvaluation::getBound() {
    return Bound(ageBounds & 0x3);
}

bool PositionEvaluation::isPVNode() {
    return bool(ageBounds & 0b100);
}

ExactScore adjustNodeScoreFromTT(int16_t nodeScoreTT, int ply) {
    return nodeScoreTT ==  CHECKMATE ? nodeScoreTT - ply :
           nodeScoreTT == -CHECKMATE ? nodeScoreTT + ply :
           nodeScoreTT; 
}

ExactScore adjustNodeScoreToTT(int16_t nodeScoreTT, int ply) {
    return nodeScoreTT >= GUARANTEE_CHECKMATE  ?  CHECKMATE : 
           nodeScoreTT <= GUARANTEE_CHECKMATED ? -CHECKMATE :
           nodeScoreTT;
}
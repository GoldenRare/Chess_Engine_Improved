#include <cstdint>
#include "transpositionTable.hpp"
#include "utility.hpp"

TranspositionTable TT;

void TranspositionTable::updateAge() {
    age += 8; //Will eventually overflow causing it to be cyclic
}

uint8_t TranspositionTable::getAge() {
    return age;
}

//Returns the first PositionEvaluation for that bucket
PositionEvaluation* TranspositionTable::indexBucket(PositionKey key) {
    return &table[(key >> 16) & (0x2000000 - 1)].pe[0];
}

//Returns either a PositionEvaluation containing the key or one to replace
PositionEvaluation* TranspositionTable::probeTT(PositionKey key, bool& hasEvaluation) {

    PositionEvaluation* pePtr = indexBucket(key);
    uint16_t keyIndex = key & 0xFFFF; //PositionEvaluations store first 16 bits of a 64 bit PositionKey

    for (int i = 0; i < BUCKET_SIZE; i++) {
        //Since we do not use the entire 64 bits of key for indexing the bucket, this may sometimes lead to collisions. 
        //When collisions occur, we use the least significant 16 bits to differentiate between these collisions. 
        //With that said, these family of collisions are stored first at index 0 to index BUCKET_SIZE - 1.
        //Therefore, if pePtr[i].positionKey == 0, return it in order to store a PositionEvaluation there or
        //if it does contain the key then return it to see what we discovered in the previous evaluation of the position.
        if ((pePtr[i].positionKey == keyIndex) || (pePtr[i].positionKey == 0)) {

            hasEvaluation = (pePtr[i].positionKey == 0) ? false : true;
            pePtr[i].ageBounds = age | (pePtr[i].ageBounds & 0x7); //If probed then update age since this old position is still relevant to the game
            return &pePtr[i];

        }
    }

    //Since the above for loop did not return, then we have had a collision 5 times.
    //In other words, we already have 4 PositionEvaluations stored in that bucket,
    //therefore we must find which one to replace.   
    hasEvaluation = false;
    PositionEvaluation* replace = pePtr;
    for (int i = 1; i < BUCKET_SIZE; i++) {
        //Favour PositionEvaluations done at a higher depth but penalize entries for being old
        if (replace->depth - ((263 + age - replace->ageBounds) & 0xF8) >
            pePtr[i].depth - ((263 + age - pePtr[i].ageBounds) & 0xF8))
            replace = &pePtr[i];
    }

    return replace;
}

void PositionEvaluation::savePositionEvaluation(PositionKey pk, uint16_t m, uint8_t d, uint8_t b, int16_t e) {

    uint16_t keyIndex = pk & 0xFFFF;

    if ((keyIndex != positionKey) || (d > depth) || (b == EXACT_BOUND)) {

        positionKey = keyIndex;
        move = m;
        depth = d;
        ageBounds = TT.getAge() | b;
        evaluation = e;

    }
}

Bound PositionEvaluation::getBound() {
    return Bound(ageBounds & 0x3);
}
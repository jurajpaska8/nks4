#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

void sbox(const uint8_t sBox[16], uint8_t state[16])
{
    for(int i = 0; i < 16; i++)
    {
        state[i] = sBox[state[i]];
    }
}

void shiftRows(uint8_t state[4][4])
{
    for(int i = 0; i < 4; i++)
    {
        uint8_t row[4];
        memcpy(row, state[i], 4);
        for(int j = 0; j < 4; j++)
        {
            state[i][j] = row[(j + i) % 4];
        }
    }
}

void shiftRowsInverse(uint8_t state[4][4])
{
    for(int i = 0; i < 4; i++)
    {
        uint8_t row[4];
        memcpy(row, state[i], 4);
        for(int j = 0; j < 4; j++)
        {
            state[i][j] = row[(j - i + 4) % 4];
        }
    }
}

void addRoundKey(const uint8_t key[4][4], uint8_t state[4][4])
{
    for (int i = 0; i < 4; i++)
    {
        for (int j = 0; j < 4; j++)
        {
            state[j][i] = state[j][i] & key[j][i];
        }
    }
}

void transformTo2Dim(const uint8_t state[16], uint8_t state2Dim[4][4])
{
    int idx = 0;
    for(int i = 0; i < 4; i++)
    {
        for(int j = 0; j < 4; j++)
        {
            // fillig column first
            state2Dim[j][i] = state[idx];
            idx++;
        }
    }
}

void transformTo1Dim(const uint8_t state2Dim[4][4], uint8_t state[16])
{
    int idx = 0;
    for(int i = 0; i < 4; i++)
    {
        for(int j = 0; j < 4; j++)
        {
            // filling column first
            state[idx] = state2Dim[j][i];
            idx++;
        }
    }
}

int encrypt(const uint8_t data[16], const uint8_t sBoxData[16], uint8_t dataToReturn[16])
{
    uint8_t state[16];
    memcpy(state, data, 16);

    // add round key
    //addRoundKey()

    for(int round = 0; round < 9; round++)
    {
        // sub bytes
        sbox(sBoxData, state);
        // shift rows
        uint8_t state2Dim[4][4];
        transformTo2Dim(state, state2Dim);
        shiftRows(state2Dim);
        transformTo1Dim(state2Dim, state);
        // mix columns

        // add round key
    }

    // sub bytes

    // shift rows

    // add round key

    // copy data
    memcpy(dataToReturn, state, 16);
    return 1;
}

int decrypt(const uint8_t data[16], const uint8_t sBoxInverseData[16], uint8_t dataToReturn[16])
{
    uint8_t state[16];
    memcpy(state, data, 16);

    // add round key

    // shift rows

    // sub bytes

    for(int round = 0; round < 9; round++)
    {
        // add round key

        // mix columns

        // shift rows
        uint8_t state2Dim[4][4];
        transformTo2Dim(state, state2Dim);
        shiftRowsInverse(state2Dim);
        transformTo1Dim(state2Dim, state);

        // sub bytes
        sbox(sBoxInverseData, state);
    }

    // add round key

    // copy value
    memcpy(dataToReturn, state, 16);
    return 1;
}

int main()
{
    uint8_t sBox[16] = {0x5, 0xB, 0x3, 0x4, 0x1, 0x6, 0xA, 0x9, 0x7, 0xE, 0x0, 0xC, 0x2, 0xF, 0xD, 0x8};
    uint8_t sBoxInverse[16] = {0xA, 0x4, 0xC, 0x2, 0x3, 0x0, 0x5, 0x8, 0xF, 0x7, 0x6, 0x1, 0xB, 0xE, 0x9, 0xD};

    const uint8_t state[16] = {0x5, 0xB, 0x3, 0x4, 0x1, 0x6, 0xA, 0x9, 0x7, 0xE, 0x0, 0xC, 0x2, 0xF, 0xD, 0x8};
    uint8_t state2Dim[4][4];
    uint8_t stateCheck[16];

    transformTo2Dim(state, state2Dim);
    transformTo1Dim(state2Dim, stateCheck);

    uint8_t toEncrypt[16] = {0xA, 0x4, 0xC, 0x2, 0x3, 0x0, 0x5, 0x8, 0xF, 0x7, 0x6, 0x1, 0xB, 0xE, 0x9, 0xD};
    uint8_t encrypted[16];
    uint8_t decrypted[16];
    encrypt(toEncrypt, sBox, encrypted);
    decrypt(encrypted, sBoxInverse, decrypted);

    int isEqual = memcmp(toEncrypt, decrypted, 16);
    printf("is Equal= %d", isEqual);

    return 0;
}
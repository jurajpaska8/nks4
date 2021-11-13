#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

void transformTo1Dim(const uint8_t state2Dim[4][4], uint8_t state[16]);
void transformTo2Dim(const uint8_t state[16], uint8_t state2Dim[4][4]);

void sbox(const uint8_t sBox[16], uint8_t state[4][4])
{
    uint8_t state1Dim[16];
    transformTo1Dim(state, state1Dim);
    for(int i = 0; i < 16; i++)
    {
        state1Dim[i] = sBox[state1Dim[i]];
    }
    transformTo2Dim(state1Dim, state);
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
            state[j][i] = state[j][i] ^ key[j][i];
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

int encrypt(const uint8_t data[4][4], const uint8_t sBoxData[16], const uint8_t keys[11][4][4], uint8_t dataToReturn[4][4])
{
    uint8_t state[4][4];
    memcpy(state, data, 16);

    // add round key
    addRoundKey(keys[0], state);

    for(int round = 0; round < 9; round++)
    {
        // sub bytes
        sbox(sBoxData, state);
        // shift rows
        shiftRows(state);
        // mix columns

        // add round key
        addRoundKey(keys[round + 1], state);
    }

    // sub bytes

    // shift rows

    // add round key
    addRoundKey(keys[10], state);

    // copy data
    memcpy(dataToReturn, state, 16);
    return 1;
}

int decrypt(const uint8_t data[4][4], const uint8_t sBoxInverseData[16], const uint8_t keys[11][4][4], uint8_t dataToReturn[4][4])
{
    uint8_t state[4][4];
    memcpy(state, data, 16);

    // add round key
    addRoundKey(keys[10], state);

    // shift rows

    // sub bytes

    for(int round = 0; round < 9; round++)
    {
        // add round key
        addRoundKey(keys[10 - 1 - round], state);

        // mix columns

        // shift rows
        shiftRowsInverse(state);

        // sub bytes
        sbox(sBoxInverseData, state);
    }

    // add round key
    addRoundKey(keys[0], state);

    // copy value
    memcpy(dataToReturn, state, 16);
    return 1;
}

void keySchedule(const uint8_t key0[4][4], const uint8_t constants[10])
{
    uint8_t key1[4][4];
    /*
     * First column
     */
    // last column of key0
    //uint8_t [4] lastColumn =
    // shift columns

    // sbox

    // affine
}

int main()
{
    const uint8_t sBox[16] = {0x5, 0xB, 0x3, 0x4, 0x1, 0x6, 0xA, 0x9, 0x7, 0xE, 0x0, 0xC, 0x2, 0xF, 0xD, 0x8};
    const uint8_t sBoxInverse[16] = {0xA, 0x4, 0xC, 0x2, 0x3, 0x0, 0x5, 0x8, 0xF, 0x7, 0x6, 0x1, 0xB, 0xE, 0x9, 0xD};

    const uint8_t key0[16] = {0x1, 0x2, 0x3, 0x4, 0x5, 0x6, 0x7, 0x8, 0x9, 0xA, 0xB, 0xC, 0xD, 0xE, 0xF};
    uint8_t keys[11][4][4];
    for (int i = 0; i < 11; ++i)
    {
        memcpy(keys[i], key0, 16);
    }

    uint8_t toEncrypt[16] = {0xA, 0x4, 0xC, 0x2, 0x3, 0x0, 0x5, 0x8, 0xF, 0x7, 0x6, 0x1, 0xB, 0xE, 0x9, 0xD};
    uint8_t toEnc[4][4];
    memcpy(toEnc, toEncrypt, 16);
    uint8_t encrypted[4][4];
    uint8_t decrypted[4][4];
    encrypt(toEnc, sBox, keys, encrypted);
    decrypt(encrypted, sBoxInverse, keys,  decrypted);

    int isEqual = memcmp(toEncrypt, decrypted, 16);
    printf("is Equal= %d", isEqual);

    return 0;
}
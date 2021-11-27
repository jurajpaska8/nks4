#include <stdio.h>
#include <string.h>
#include <time.h>

#define UINT_64_IN_SEQUENCE 131072
#define SEQUENCE_COUNT 100

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

uint8_t findExp(const uint8_t a, const uint8_t multTable[16])
{
    for (int i = 0; i < 16; ++i) {
        if(a == multTable[i])
        {
            return i;
        }
    }
    return 0;
}

uint8_t mul(const uint8_t a, const uint8_t b, const uint8_t multTable[16])
{
    if(a==0 || b==0)
    {
        return 0;
    }
    uint8_t exp = (findExp(a, multTable) + findExp(b, multTable)) % 15;
    return multTable[exp];
}

void mcFun(const uint8_t multTable[16], const uint8_t mattrix[4][4], uint8_t data[4][4])
{
    uint8_t dataToReturn[4][4];

    for (int col = 0; col < 4; ++col)
    {
        for (int i = 0; i < 4; ++i) {
            uint8_t tmp = 0;
            for (int j = 0; j < 4; ++j) {
                tmp ^= mul(mattrix[i][j], data[j][col], multTable);
            }
            dataToReturn[i][col] = tmp;
        }
    }
    memcpy(data, dataToReturn, 16);
}

void printState(uint8_t state[4][4])
{
    for(int i = 0; i < 4; i++)
    {
        printf("[%x %x %x %x]\n", state[i][0], state[i][1], state[i][2], state[i][3]);
    }
    printf("\n");
}
void printXorStates(uint8_t state1[4][4], uint8_t state2[4][4])
{
    for(int i = 0; i < 4; i++)
    {
        printf("[%x %x %x %x]\n", state1[i][0] ^ state2[i][0], state1[i][1] ^ state2[i][1], state1[i][2] ^ state2[i][2], state1[i][3] ^ state2[i][3]);
    }
    printf("\n");
}



int encrypt(const uint8_t multipleTable[16],
            const uint8_t mcMattrix[4][4],
            const uint8_t data1[4][4],
            const uint8_t data2[4][4],
            const uint8_t sBoxData[16],
            const uint8_t keys[11][4][4],
            uint8_t dataToReturn[4][4])
{
    uint8_t state1[4][4];
    memcpy(state1, data1, 16);

    uint8_t state2[4][4];
    memcpy(state2, data2, 16);

    // add round key
    printf("****Initial Round****\n");
    printf("Add Round Key\n");
    addRoundKey(keys[0], state1);
    printState(state1);

    addRoundKey(keys[0], state2);
    printState(state2);
    printXorStates(state1, state2);

    for(int round = 1; round < 3; round++)
    {
        printf("****Round %i****\n", round);
        // sub bytes
        printf("SubBytes\n");
        sbox(sBoxData, state1);
        printState(state1);

        sbox(sBoxData, state2);
        printState(state2);
        printXorStates(state1, state2);

        // shift rows
        printf("ShiftRows\n");
        shiftRows(state1);
        printState(state1);

        shiftRows(state2);
        printState(state2);
        printXorStates(state1, state2);

        // mix columns
        printf("MixColumns\n");
        mcFun(multipleTable, mcMattrix, state1);
        printState(state1);

        mcFun(multipleTable, mcMattrix, state2);
        printState(state2);
        printXorStates(state1, state2);

        // add round key
        printf("Add Round Key\n");
        addRoundKey(keys[round + 1], state1);
        printState(state1);

        addRoundKey(keys[round + 1], state2);
        printState(state2);
        printXorStates(state1, state2);

    }

    printf("****Final Round****\n");
    // sub bytes
    printf("SubBytes*\n");
    sbox(sBoxData, state1);
    printState(state1);

    sbox(sBoxData, state2);
    printState(state2);
    printXorStates(state1, state2);

    // shift rows
    printf("ShiftRows*\n");
    shiftRows(state1);
    printState(state1);

    shiftRows(state2);
    printState(state2);
    printXorStates(state1, state2);

    // add round key
    printf("Add Round Key*\n");
    addRoundKey(keys[10], state1);
    printState(state1);

    addRoundKey(keys[10], state2);
    printState(state2);
    printXorStates(state1, state2);

    // copy data
    memcpy(dataToReturn, state1, 16);
    return 1;
}

uint64_t transform128to64(const uint8_t state[4][4])
{
    uint64_t returnState = 0;
    uint16_t columns[4] = {0};

    for (int i = 0; i < 4; ++i) {
        columns[i] ^= state[i][0] << 12;
        columns[i] ^= state[i][1] << 8;
        columns[i] ^= state[i][2] << 4;
        columns[i] ^= state[i][3] << 0;
    }
    memcpy(&returnState, columns, 8);
    return returnState;
}

int main()
{
    // S-Boxes
    const uint8_t sBox[16] = {0x5, 0xB, 0x3, 0x4, 0x1, 0x6, 0xA, 0x9, 0x7, 0xE, 0x0, 0xC, 0x2, 0xF, 0xD, 0x8};
    const uint8_t sBoxInverse[16] = {0xA, 0x4, 0xC, 0x2, 0x3, 0x0, 0x5, 0x8, 0xF, 0x7, 0x6, 0x1, 0xB, 0xE, 0x9, 0xD};

    // key schedule
    const uint8_t key0[16] = {0x0, 0x1, 0x2, 0x3, 0x4, 0x5, 0x6, 0x7, 0x8, 0x9, 0xA, 0xB, 0xC, 0xD, 0xE, 0xF};
    const uint8_t constants[10] = {0x1, 0x2, 0x4, 0x8, 0x3, 0x6, 0xC, 0xB, 0x5, 0xA};

    uint8_t keys[11][4][4];

    for (int i = 0; i < 11; ++i)
    {
        memcpy(keys[i], key0, 16);
    }

    // mix columns
    const uint8_t mc[16] = {0x2, 0x3, 0x1, 0x1, 0x1, 0x2, 0x3, 0x1, 0x1, 0x1, 0x2, 0x3, 0x3, 0x1, 0x1, 0x2};
    uint8_t mcMattrix[4][4];
    memcpy(mcMattrix, mc, 16);

    const uint8_t mcInv[16] = {0xE, 0xB, 0xD, 0x9, 0x9, 0xE, 0xB, 0xD, 0xD, 0x9, 0xE, 0xB, 0xB, 0xD, 0x9, 0xE};
    uint8_t mcMattrixInv[4][4];
    memcpy(mcMattrixInv, mcInv, 16);

    // table for multiplication
    const uint8_t multipleTable[16] = {0x1, 0x2, 0x4, 0x8, 0x3, 0x6, 0xC, 0xB, 0x5, 0xA, 0x7, 0xE, 0xF, 0xD, 0x9};

    // data for encryption
    uint8_t toEncrypt0[16] = {0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0};
    uint8_t toEncrypt1[16] = {0x1, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0};

    uint8_t toEnc0[4][4];
    memcpy(toEnc0, toEncrypt0, 16);

    uint8_t toEnc1[4][4];
    memcpy(toEnc1, toEncrypt1, 16);

    uint8_t encrypted[4][4];
    encrypt(multipleTable, mcMattrix, toEnc0, toEnc1, sBox, keys, encrypted);

    return 0;
}
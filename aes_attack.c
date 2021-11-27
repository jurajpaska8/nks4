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
void printXorStates(uint8_t state[16][4][4])
{
    uint8_t xors[4][4] = {0};

    for(int i = 0; i < 4; i++)
    {
        for (int j = 0; j < 4; ++j)
        {
            uint8_t tmp = 0;
            for (int k = 0; k < 16; ++k)
            {
                tmp = tmp ^ state[k][i][j];
            }
            xors[i][j] = tmp;
        }
    }
    printf("xors \n");
    for(int i = 0; i < 4; i++)
    {
        printf("[%x %x %x %x]\n", xors[i][0], xors[i][1], xors[i][2], xors[i][3]);
    }
    printf("\n");
}

void returnXorsTable(uint8_t state[16][4][4], uint8_t xors[4][4])
{
    for(int i = 0; i < 4; i++)
    {
        for (int j = 0; j < 4; ++j)
        {
            uint8_t tmp = 0;
            for (int k = 0; k < 16; ++k)
            {
                tmp = tmp ^ state[k][i][j];
            }
            xors[i][j] = tmp;
        }
    }
}



int encrypt(const uint8_t multipleTable[16],
            const uint8_t mcMattrix[4][4],
            const uint8_t data[16][4][4],
            const uint8_t sBoxData[16],
            const uint8_t keys[5][4][4],
            uint8_t dataToReturn[16][4][4])
{
    uint8_t state[16][4][4];
    memcpy(state, data, 16*16);

    // add round key
    printf("****Initial Round****\n");
    printf("Add Round Key\n");
    for (int i = 0; i < 16; ++i)
    {
        addRoundKey(keys[0], state[i]);
    }
    printXorStates(state);

    for(int round = 1; round <= 3; round++)
    {
        printf("****Round %i****\n", round);
        // sub bytes
        printf("SubBytes\n");
        for (int i = 0; i < 16; ++i)
        {
            sbox(sBoxData, state[i]);
        }
        printXorStates(state);

        // shift rows
        printf("ShiftRows\n");
        for (int i = 0; i < 16; ++i)
        {
            shiftRows(state[i]);
        }
        printXorStates(state);

        // mix columns
        printf("MixColumns\n");
        for (int i = 0; i < 16; ++i)
        {
            mcFun(multipleTable, mcMattrix, state[i]);
        }
        printXorStates(state);

        // add round key
        printf("Add Round Key\n");
        for (int i = 0; i < 16; ++i)
        {
            addRoundKey(keys[round], state[i]);
        }
        printXorStates(state);
    }

    printf("****Final Round****\n");
    // sub bytes TODO here difference is 0 if this is fourth round
    printf("SubBytes*\n");
    for (int i = 0; i < 16; ++i)
    {
        sbox(sBoxData, state[i]);
    }
    printXorStates(state);

    // shift rows
    printf("ShiftRows*\n");
    for (int i = 0; i < 16; ++i)
    {
        shiftRows(state[i]);
    }
    printXorStates(state);

    // add round key
    printf("Add Round Key*\n");
    for (int i = 0; i < 16; ++i)
    {
        addRoundKey(keys[4], state[i]);
    }
    printXorStates(state);

    // copy data
    memcpy(dataToReturn, state, 16*16);
    return 1;
}

int decryptFourthRound(const uint8_t data[16][4][4],
            const uint8_t sBoxData[16],
            const uint8_t key[4][4],
            uint8_t xors[4][4])
{
    uint8_t state[16][4][4];
    memcpy(state, data, 16*16);

    printf("****Final Round****\n");
    // add round key
    printf("Add Round Key*\n");
    for (int i = 0; i < 16; ++i)
    {
        addRoundKey(key, state[i]);
    }
    printXorStates(state);
    // shift rows
    printf("ShiftRows*\n");
    for (int i = 0; i < 16; ++i)
    {
        shiftRowsInverse(state[i]);
    }
    printXorStates(state);
    // sub bytes
    printf("SubBytes*\n");
    for (int i = 0; i < 16; ++i)
    {
        sbox(sBoxData, state[i]);
    }
    printXorStates(state);
    returnXorsTable(state, xors);
    return 1;
}
int main()
{
    // S-Boxes
    const uint8_t sBox[16] = {0x5, 0xB, 0x3, 0x4, 0x1, 0x6, 0xA, 0x9, 0x7, 0xE, 0x0, 0xC, 0x2, 0xF, 0xD, 0x8};
    const uint8_t sBoxInverse[16] = {0xA, 0x4, 0xC, 0x2, 0x3, 0x0, 0x5, 0x8, 0xF, 0x7, 0x6, 0x1, 0xB, 0xE, 0x9, 0xD};

    // key schedule
    const uint8_t key0[16] = {0x0, 0x1, 0x2, 0x3, 0x4, 0x5, 0x6, 0x7, 0x8, 0x9, 0xA, 0xB, 0xC, 0xD, 0xE, 0xF};
    uint8_t keys[5][4][4];
    for (int i = 0; i < 5; ++i)
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
    uint8_t toEnc[16][4][4] = {0};

    for(int i = 0; i < 16; i++)
    {
        toEnc[i][0][0] = i;
    }
    printXorStates(toEnc);

    uint8_t encrypted[16][4][4];
    encrypt(multipleTable, mcMattrix, toEnc, sBox, keys, encrypted);

    // key schedule dec
    const uint8_t key1[16] = {0x0, 0x1, 0x2, 0x3, 0x4, 0x5, 0x6, 0x7, 0x8, 0x9, 0xA, 0xB, 0xC, 0xD, 0xE, 0xF};
    uint8_t keys1[5][4][4];
    for (int i = 0; i < 5; ++i)
    {
        memcpy(keys1[i], key1, 16);
    }


    printf("************DECRYPTION*********\n");
    // try bits
    for (int i = 0; i < 16; ++i)
    {
        keys1[4][0][0] = i;
        uint8_t xors[4][4] = {0};
        decryptFourthRound(encrypted, sBoxInverse, keys1[4], xors);
    }

    return 0;
}
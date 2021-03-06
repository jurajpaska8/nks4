#include <stdio.h>
#include <string.h>

#define BYTES_IN_SEQUENCE 16384
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

int encrypt(const uint8_t multipleTable[16],
            const uint8_t mcMattrix[4][4],
            const uint8_t data[4][4],
            const uint8_t sBoxData[16],
            const uint8_t keys[11][4][4],
            uint8_t dataToReturn[4][4])
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
        mcFun(multipleTable, mcMattrix, state);

        // add round key
        addRoundKey(keys[round + 1], state);
    }

    // sub bytes
    sbox(sBoxData, state);

    // shift rows
    shiftRows(state);

    // add round key
    addRoundKey(keys[10], state);

    // copy data
    memcpy(dataToReturn, state, 16);
    return 1;
}

int decrypt(const uint8_t multipleTable[16],
            const uint8_t mcMattrix[4][4],
            const uint8_t data[4][4],
            const uint8_t sBoxInverseData[16],
            const uint8_t keys[11][4][4],
            uint8_t dataToReturn[4][4])
{
    uint8_t state[4][4];
    memcpy(state, data, 16);

    // add round key
    addRoundKey(keys[10], state);

    // shift rows
    shiftRowsInverse(state);

    // sub bytes
    sbox(sBoxInverseData, state);

    for(int round = 0; round < 9; round++)
    {
        // add round key
        addRoundKey(keys[10 - 1 - round], state);

        // mix columns
        mcFun(multipleTable, mcMattrix, state);

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

void keySchedule(const uint8_t key0[4][4],
                 const uint8_t constants[10],
                 const uint8_t sBox[16],
                 uint8_t keyToReturn[4][4],
                 int constIdx)
{
    /**
     * First column
     */
    // last column of key0
    uint8_t lastColumn[4];
    for (int i = 0; i < 4; ++i)
    {
        lastColumn[i] = key0[i][3];
    }

    // shift column
    uint8_t tmp[4];
    memcpy(tmp, lastColumn, 4);
    for (int i = 0; i < 4; ++i)
    {
        lastColumn[i] = tmp[(i + 1) % 4];
    }

    // sbox
    for (int i = 0; i < 4; ++i)
    {
        lastColumn[i] = sBox[lastColumn[i]];
    }
    // affine
    for (int i = 0; i < 4; ++i)
    {
        keyToReturn[i][0] = key0[i][0] ^ lastColumn[i];
        if(i == 0)
        {
            keyToReturn[0][0] = keyToReturn[0][0] ^ constants[constIdx];
        }
    }

    /**
    * Other columns
    */
    for (int row = 0; row < 4; ++row)
    {
        for (int col = 1; col < 4; ++col)
        {
            keyToReturn[row][col] = keyToReturn[row][col - 1] ^ key0[row][col];
        }
    }
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
    memcpy(keys[0], key0, 16);

    for (int i = 0; i < 10; ++i)
    {
        keySchedule(keys[i], constants, sBox, keys[i + 1], i);
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
    uint8_t toEncrypt[16] = {0xA, 0x4, 0xC, 0x2, 0x3, 0x0, 0x5, 0x8, 0xF, 0x7, 0x6, 0x1, 0xB, 0xE, 0x9, 0xD};
    uint8_t toEnc[4][4];
    memcpy(toEnc, toEncrypt, 16);

    int cnt = 0;
    for(int i = 0; i < 100; i++)
    {
        // encryption
        uint8_t encrypted[4][4];
        encrypt(multipleTable, mcMattrix, toEnc, sBox, keys, encrypted);

        // decryption
        uint8_t decrypted[4][4];
        decrypt(multipleTable, mcMattrixInv, encrypted, sBoxInverse, keys,  decrypted);

        // test if data equals
        int isEqual = memcmp(toEnc, decrypted, 16);
        printf("is Equal= %d\n", isEqual);
        if(isEqual == 0)
        {
            cnt++;
        }
        // data
        memcpy(toEnc, encrypted, 16);
    }

    printf("OK enc-dec pairs/all = %d/100\n", cnt);

    // ctr

    uint64_t idxAndIv[2];
    FILE *file = fopen("data", "ab");
    for (int i = 0; i < 100; ++i) {
        printf("i = %i\n", i);
        idxAndIv[0] = i;
        uint64_t out[131072] = {0};
        for (int j = 0; j < 16384; ++j) {
            idxAndIv[1] = j;
            uint8_t pt[4][4];
            uint8_t ct[4][4];
            memcpy(pt, idxAndIv, 16);
            encrypt(multipleTable, mcMattrix, pt, sBox, keys, ct);
            uint64_t output = transform128to64(ct);
            out[j] = output;
        }
        fwrite(out, sizeof(out[0]), 131072, file);

    }
    fclose(file);

//    idxAndIv[0] = 0;
//    idxAndIv[1] = 0;
//    uint8_t pt[4][4];
//    uint8_t ct[4][4];
//    memcpy(pt, idxAndIv, 16);
//    encrypt(multipleTable, mcMattrix, pt, sBox, keys, ct);
//    uint64_t output = transform128to64(ct);



    return 0;
}
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

void sbox(const uint16_t sBoxes[16], uint16_t* state)
{
    *state =
              (sBoxes[(*state >> 0) & (uint16_t)0x000f] << 0)
              ^ (sBoxes[(*state >> 4) & (uint16_t)0x000f] << 4)
              ^ (sBoxes[(*state >> 8) & (uint16_t)0x000f] << 8)
              ^ (sBoxes[(*state >> 12) & (uint16_t)0x000f] << 12);
}

void permute(uint16_t* state, const uint16_t permutation[], int permLen)
{
    uint16_t out = {0};
    for(int i = 0; i < permLen; i++)
    {
        uint16_t indexOfIBitInOutput = permutation[i];
        // we will go through indexes 0...permLen - 1
        uint16_t valueOfIBit = (((uint16_t) *state) >> (permLen - i - 1)) & 0x1;
        // value of bit in proper position
        uint16_t tmp = valueOfIBit << (permLen - indexOfIBitInOutput - 1);
        out = out ^ tmp;
    }

    *state = out;
}



int encrypt(uint16_t* data, uint16_t key,  uint16_t sBox[16], uint16_t permutation[16], int permLen)
{
    uint16_t state, rk;
    state = *data;
    rk = key;

    for(int round = 0 ; round < 3; round++)
    {
        // key addition
        state ^= rk;
        // apply sBox
        sbox(sBox, &state);
        // apply permutation
        permute(&state, permutation, permLen);
    }

    // round 4
    state ^= rk;
    sbox(sBox, &state);

    // last step - xor key
    state ^= rk; //TODO ???
    // copy result
    *data = state;
    return 1;
}

int decrypt(uint16_t* data, uint16_t key, uint16_t sBoxInverse[16], uint16_t permutation[16], int permLen)
{
    uint16_t state, rk;
    state = *data;
    rk = key;

    // xor key
    state ^= rk;// TODO ???

    // first round
    sbox(sBoxInverse, &state);
    state ^= rk;

    for(int round = 0; round < 3; round++)
    {
        // apply inverse permutation
        permute(&state, permutation, permLen);
        // apply sBox with inverse table
        sbox(sBoxInverse, &state);
        // key addition
        state ^= rk;

    }
    *data = state;

    return 1;
}

int bruteforce()
{
    uint16_t sBox[16] = {0x5, 0xB, 0x3, 0x4, 0x1, 0x6, 0xA, 0x9, 0x7, 0xE, 0x0, 0xC, 0x2, 0xF, 0xD, 0x8};
    uint16_t sBoxInverse[16] = {0xA, 0x4, 0xC, 0x2, 0x3, 0x0, 0x5, 0x8, 0xF, 0x7, 0x6, 0x1, 0xB, 0xE, 0x9, 0xD};

    uint16_t permutation[16] = {0x0, 0x4, 0x8, 0xC, 0x1, 0x5, 0x9, 0xD, 0x2, 0x6, 0xA, 0xE, 0x3, 0x7, 0xB, 0xF};
    uint16_t inversePerm[16] = {0x0, 0x4, 0x8, 0xC, 0x1, 0x5, 0x9, 0xD, 0x2, 0x6, 0xA, 0xE, 0x3, 0x7, 0xB, 0xF};
    int permLen = (int)(sizeof (permutation) / sizeof (permutation[0]));

    // plaint text
    uint16_t dataPt = 0x4000;
    // cipher text
    uint16_t dataCt = 0x156b;
    uint16_t key;

    for(key = 0; key < 65535; key++) //TODO 0 key wont be tried
    {
        uint16_t tmp = dataCt;
        decrypt(&tmp, key + 1, sBoxInverse, inversePerm, permLen);
        if(tmp == dataPt)
        {
            printf("key : %04x\n", key + 1);
        }
    }

    printf("%02x\n", dataPt);
    encrypt(&dataPt, key, sBox, permutation, permLen);
    printf("%04x\n", dataPt);
    decrypt(&dataPt, key, sBoxInverse, inversePerm, permLen);
    printf("%x\n", dataPt);
    return 1;
}

int allPlaintexts()
{
    return 1;
}

int main()
{
    // sbox
    //[ 5, 11,  3,  4,  1,  6, 10,  9,  7, 14,  0, 12,  2, 15, 13,  8]
    //                     0,   1,   2,   3,   4,   5,   6,   7,   8,   9,   A,   B,   C,   D,   E,   F
    uint16_t sBox[16] = {0x5, 0xB, 0x3, 0x4, 0x1, 0x6, 0xA, 0x9, 0x7, 0xE, 0x0, 0xC, 0x2, 0xF, 0xD, 0x8};

    //                            0,   1,   2,   3,   4,   5,   6,   7,   8,   9,   A,   B,   C,   D,   E,   F
    uint16_t sBoxInverse[16] = {0xA, 0x4, 0xC, 0x2, 0x3, 0x0, 0x5, 0x8, 0xF, 0x7, 0x6, 0x1, 0xB, 0xE, 0x9, 0xD};

    // permutation                0    1    2    3    4    5    6    7    8    9    10   11   12  13   14   15
    uint16_t permutation[16] = {0x0, 0x4, 0x8, 0xC, 0x1, 0x5, 0x9, 0xD, 0x2, 0x6, 0xA, 0xE, 0x3, 0x7, 0xB, 0xF};
    // inverse permutation
    uint16_t inversePerm[16] = {0x0, 0x4, 0x8, 0xC, 0x1, 0x5, 0x9, 0xD, 0x2, 0x6, 0xA, 0xE, 0x3, 0x7, 0xB, 0xF};


    uint16_t data = 0x4000;
    uint16_t key = 0xFFFF;

    int permLen = (int)(sizeof (permutation) / sizeof (permutation[0]));
    printf("%02x\n", data);
    encrypt(&data, key, sBox, permutation, permLen);
    printf("%04x\n", data);
    decrypt(&data, key, sBoxInverse, inversePerm, permLen);
    printf("%x\n", data);

    printf("bruteforce\n");
    bruteforce();
    return 0;
}



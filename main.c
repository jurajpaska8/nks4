#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

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



int encrypt(uint16_t* data, const uint16_t keys[5],  uint16_t sBox[16], uint16_t permutation[16], int permLen)
{
    uint16_t state;
    state = *data;

    for(int round = 0 ; round < 3; round++)
    {
        // key addition
        state ^= keys[round];
        // apply sBox
        sbox(sBox, &state);
        // apply permutation
        permute(&state, permutation, permLen);
    }

    // round 4
    state ^= keys[3];
    sbox(sBox, &state);

    // last step - xor key
    state ^= keys[4];
    // copy result
    *data = state;
    return 1;
}

int decrypt(uint16_t* data, const uint16_t keys[5], uint16_t sBoxInverse[16], uint16_t permutation[16], int permLen)
{
    uint16_t state;
    state = *data;

    // xor key
    state ^= keys[0];

    // first round
    sbox(sBoxInverse, &state);
    state ^= keys[1];

    for(int round = 0; round < 3; round++)
    {
        // apply inverse permutation
        permute(&state, permutation, permLen);
        // apply sBox with inverse table
        sbox(sBoxInverse, &state);
        // key addition
        state ^= keys[round + 2];

    }
    *data = state;

    return 1;
}

//int bruteforce()
//{
//    uint16_t sBox[16] = {0x5, 0xB, 0x3, 0x4, 0x1, 0x6, 0xA, 0x9, 0x7, 0xE, 0x0, 0xC, 0x2, 0xF, 0xD, 0x8};
//    uint16_t sBoxInverse[16] = {0xA, 0x4, 0xC, 0x2, 0x3, 0x0, 0x5, 0x8, 0xF, 0x7, 0x6, 0x1, 0xB, 0xE, 0x9, 0xD};
//
//    uint16_t permutation[16] = {0x0, 0x4, 0x8, 0xC, 0x1, 0x5, 0x9, 0xD, 0x2, 0x6, 0xA, 0xE, 0x3, 0x7, 0xB, 0xF};
//    uint16_t inversePerm[16] = {0x0, 0x4, 0x8, 0xC, 0x1, 0x5, 0x9, 0xD, 0x2, 0x6, 0xA, 0xE, 0x3, 0x7, 0xB, 0xF};
//    int permLen = (int)(sizeof (permutation) / sizeof (permutation[0]));
//
//    // plaint text
//    uint16_t dataPt = 0x4000;
//    // cipher text
//    uint16_t dataCt = 0x156b;
//    uint16_t key;
//
//    for(key = 0; key < 65535; key++) //TODO 0 key wont be tried
//    {
//        uint16_t tmp = dataCt;
//        decrypt(&tmp, key + 1, sBoxInverse, inversePerm, permLen);
//        if(tmp == dataPt)
//        {
//            printf("key : %04x\n", key + 1);
//        }
//    }
//
//    printf("%02x\n", dataPt);
//    encrypt(&dataPt, key, sBox, permutation, permLen);
//    printf("%04x\n", dataPt);
//    decrypt(&dataPt, key, sBoxInverse, inversePerm, permLen);
//    printf("%x\n", dataPt);
//    return 1;
//}
//
//int allPlaintexts()
//{
//    uint16_t sBox[16] = {0x5, 0xB, 0x3, 0x4, 0x1, 0x6, 0xA, 0x9, 0x7, 0xE, 0x0, 0xC, 0x2, 0xF, 0xD, 0x8};
//    uint16_t sBoxInverse[16] = {0xA, 0x4, 0xC, 0x2, 0x3, 0x0, 0x5, 0x8, 0xF, 0x7, 0x6, 0x1, 0xB, 0xE, 0x9, 0xD};
//
//    uint16_t permutation[16] = {0x0, 0x4, 0x8, 0xC, 0x1, 0x5, 0x9, 0xD, 0x2, 0x6, 0xA, 0xE, 0x3, 0x7, 0xB, 0xF};
//    uint16_t inversePerm[16] = {0x0, 0x4, 0x8, 0xC, 0x1, 0x5, 0x9, 0xD, 0x2, 0x6, 0xA, 0xE, 0x3, 0x7, 0xB, 0xF};
//    int permLen = (int)(sizeof (permutation) / sizeof (permutation[0]));
//
//    // plaint text
//    uint16_t dataPt = 0x0000;
//    // cipher text
//    uint16_t dataCt;
//    uint16_t key = 0x2222;
//
//    for(dataPt = 0; dataPt < 65535; dataPt++) //TODO 0 plain text wont be tried
//    {
//        uint16_t tmp = dataPt;
//        encrypt(&tmp, key, sBoxInverse, inversePerm, permLen);
//    }
//    return 1;
//}

int main()
{
    // sboxes
    uint16_t sBox[16] = {0x5, 0xB, 0x3, 0x4, 0x1, 0x6, 0xA, 0x9, 0x7, 0xE, 0x0, 0xC, 0x2, 0xF, 0xD, 0x8};
    uint16_t sBoxInverse[16] = {0xA, 0x4, 0xC, 0x2, 0x3, 0x0, 0x5, 0x8, 0xF, 0x7, 0x6, 0x1, 0xB, 0xE, 0x9, 0xD};
    // perm
    uint16_t permutation[16] = {0x0, 0x4, 0x8, 0xC, 0x1, 0x5, 0x9, 0xD, 0x2, 0x6, 0xA, 0xE, 0x3, 0x7, 0xB, 0xF};
    uint16_t inversePerm[16] = {0x0, 0x4, 0x8, 0xC, 0x1, 0x5, 0x9, 0xD, 0x2, 0x6, 0xA, 0xE, 0x3, 0x7, 0xB, 0xF};

    // pt
    uint16_t dataPt = 0x2345;
    // ct
    uint16_t dataCt;
    // keys
    uint16_t keys[5] = {0x2222, 0x2223, 0x2233, 0x2333, 0x3333};
    uint16_t keysInv[5] = {0x3333, 0x2333, 0x2233, 0x2223, 0x2222};

    // perm len
    int permLen = (int)(sizeof (permutation) / sizeof (permutation[0]));

    // encrypt
    encrypt(&dataPt, keys, sBox, permutation, permLen);
    printf("%04x\n", dataPt);

    // decrypt
    decrypt(&dataPt, keysInv, sBoxInverse, inversePerm, permLen);
    printf("%04x\n", dataPt);
    return 0;
}



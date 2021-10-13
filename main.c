#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

void sbox(uint16_t sBoxes[16], uint16_t state[4])
{
    state[0] =
              (sBoxes[(state[0] >> 0) & (uint16_t)0x000f] << 0)
              ^ (sBoxes[(state[0] >> 4) & (uint16_t)0x000f] << 4)
              ^ (sBoxes[(state[0] >> 8) & (uint16_t)0x000f] << 8)
              ^ (sBoxes[(state[0] >> 12) & (uint16_t)0x000f] << 12);

    state[1] =
            (sBoxes[(state[1] >> 0) & (uint16_t)0x000f] << 0)
            ^ (sBoxes[(state[1] >> 4) & (uint16_t)0x000f] << 4)
            ^ (sBoxes[(state[1] >> 8) & (uint16_t)0x000f] << 8)
            ^ (sBoxes[(state[1] >> 12) & (uint16_t)0x000f] << 12);

    state[2] =
            (sBoxes[(state[2] >> 0) & (uint16_t)0xf] << 0)
            ^ (sBoxes[(state[2] >> 4) & (uint16_t)0xf] << 4)
            ^ (sBoxes[(state[2] >> 8) & (uint16_t)0xf] << 8)
            ^ (sBoxes[(state[2] >> 12) & (uint16_t)0xf] << 12);

    state[3] =
            (sBoxes[(state[3] >> 0) & (uint16_t)0xf] << 0)
            ^ (sBoxes[(state[3] >> 4) & (uint16_t)0xf] << 4)
            ^ (sBoxes[(state[3] >> 8) & (uint16_t)0xf] << 8)
            ^ (sBoxes[(state[3] >> 12) & (uint16_t)0xf] << 12);

}

void permute(uint16_t state[4])
{
    uint16_t out[4] = {0};
    out[0] ^= state[0] & 0x1111;
    out[1] ^= state[0] & 0x2222;
    out[2] ^= state[0] & 0x4444;
    out[3] ^= state[0] & 0x8888;

    out[1] ^= state[1] & 0x1111;
    out[2] ^= state[1] & 0x2222;
    out[3] ^= state[1] & 0x4444;
    out[0] ^= state[1] & 0x8888;

    out[2] ^= state[2] & 0x1111;
    out[3] ^= state[2] & 0x2222;
    out[0] ^= state[2] & 0x4444;
    out[1] ^= state[2] & 0x8888;

    out[3] ^= state[3] & 0x1111;
    out[0] ^= state[3] & 0x2222;
    out[1] ^= state[3] & 0x4444;
    out[2] ^= state[3] & 0x8888;

    state[0] = out[0]; state[1] = out[1]; state[2] = out[2]; state[3] = out[3];
}

void permuteInverse(uint16_t out[4])
{
    // inverse
    uint16_t originalValue[4] = {0};
    originalValue[0] ^= out[0] & 0x1111;
    originalValue[0] ^= out[1] & 0x2222;
    originalValue[0] ^= out[2] & 0x4444;
    originalValue[0] ^= out[3] & 0x8888;

    originalValue[1] ^= out[1] & 0x1111;
    originalValue[1] ^= out[2] & 0x2222;
    originalValue[1] ^= out[3] & 0x4444;
    originalValue[1] ^= out[0] & 0x8888;

    originalValue[2] ^= out[2] & 0x1111;
    originalValue[2] ^= out[3] & 0x2222;
    originalValue[2] ^= out[0] & 0x4444;
    originalValue[2] ^= out[1] & 0x8888;

    originalValue[3] ^= out[3] & 0x1111;
    originalValue[3] ^= out[0] & 0x2222;
    originalValue[3] ^= out[1] & 0x4444;
    originalValue[3] ^= out[2] & 0x8888;
    out[0] = originalValue[0]; out[1] = originalValue[1]; out[2] = originalValue[2]; out[3] = originalValue[3];
}

int encrypt(uint8_t data[8], uint8_t key[8],  uint16_t sBox[16])
{
    uint16_t state[4], rk[4];
    memcpy(state, data, sizeof(state));
    memcpy(rk, key, sizeof(rk));

    for(int round = 0 ; round < 4; round++)
    {
        // key addition
        for (int i = 0; i < 4; i++)
        {
            state[i] ^= rk[i];
        }
        // apply sBox
        sbox(sBox, state);
        // apply permutation
        permute(state);
    }

    memcpy(data, state, sizeof(state));

    return 1;
}

int decrypt(uint8_t data[8], uint8_t key[8], uint16_t sBoxInverse[16])
{
    uint16_t state[4], rk[4];

    memcpy(state, data, sizeof(state));
    memcpy(rk, key, sizeof(rk));

    for(int round = 0; round < 4; round++)
    {
        // apply inverse permutation
        permuteInverse(state);

        // apply sBox with inverse table
        sbox(sBoxInverse, state);
        // key addition
        for (int i = 0; i < 4; i++)
        {
            state[i] ^= rk[i];
        }
    }
    memcpy(data, state, sizeof(state));

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

    uint8_t data[8] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xAA};
    uint8_t key[8] = {0xab, 0xcd, 0xef, 0x01, 0xab, 0xcd, 0xef, 0x01};

    for (int i = 0; i < 8; i++)
    {
        printf("%02x", data[i]);
    }
    printf("\n");
    encrypt(data, key, sBox);
    for (int i = 0; i < 8; i++)
    {
        printf("%02x", data[i]);
    }
    decrypt(data, key, sBoxInverse);
    printf("\n");
    for (int i = 0; i < 8; i++)
    {
        printf("%02x", data[i]);
    }
    printf("\n");
    printf("Hello, World!\n");
    return 0;
}



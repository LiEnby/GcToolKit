#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "aes.h"

#define block_copy(dest, src)  memcpy((dest), (src), AES_BLOCKSIZE)
// #define DEBUG_ENCRYPT
// #define DEBUG_DECRYPT


//a = a xor b
static void block_xor(uint8_t* a, uint8_t* b)
{
    int i;
    for (i = 0; i < AES_BLOCKSIZE; i++)
        a[i] ^= b[i];
}

//SBox taken from Wikipedia
static const uint8_t sbox[256] =
{
    0x63, 0x7C, 0x77, 0x7B, 0xF2, 0x6B, 0x6F, 0xC5, 0x30, 0x01, 0x67, 0x2B, 0xFE, 0xD7, 0xAB, 0x76,
    0xCA, 0x82, 0xC9, 0x7D, 0xFA, 0x59, 0x47, 0xF0, 0xAD, 0xD4, 0xA2, 0xAF, 0x9C, 0xA4, 0x72, 0xC0,
    0xB7, 0xFD, 0x93, 0x26, 0x36, 0x3F, 0xF7, 0xCC, 0x34, 0xA5, 0xE5, 0xF1, 0x71, 0xD8, 0x31, 0x15,
    0x04, 0xC7, 0x23, 0xC3, 0x18, 0x96, 0x05, 0x9A, 0x07, 0x12, 0x80, 0xE2, 0xEB, 0x27, 0xB2, 0x75,
    0x09, 0x83, 0x2C, 0x1A, 0x1B, 0x6E, 0x5A, 0xA0, 0x52, 0x3B, 0xD6, 0xB3, 0x29, 0xE3, 0x2F, 0x84,
    0x53, 0xD1, 0x00, 0xED, 0x20, 0xFC, 0xB1, 0x5B, 0x6A, 0xCB, 0xBE, 0x39, 0x4A, 0x4C, 0x58, 0xCF,
    0xD0, 0xEF, 0xAA, 0xFB, 0x43, 0x4D, 0x33, 0x85, 0x45, 0xF9, 0x02, 0x7F, 0x50, 0x3C, 0x9F, 0xA8,
    0x51, 0xA3, 0x40, 0x8F, 0x92, 0x9D, 0x38, 0xF5, 0xBC, 0xB6, 0xDA, 0x21, 0x10, 0xFF, 0xF3, 0xD2,
    0xCD, 0x0C, 0x13, 0xEC, 0x5F, 0x97, 0x44, 0x17, 0xC4, 0xA7, 0x7E, 0x3D, 0x64, 0x5D, 0x19, 0x73,
    0x60, 0x81, 0x4F, 0xDC, 0x22, 0x2A, 0x90, 0x88, 0x46, 0xEE, 0xB8, 0x14, 0xDE, 0x5E, 0x0B, 0xDB,
    0xE0, 0x32, 0x3A, 0x0A, 0x49, 0x06, 0x24, 0x5C, 0xC2, 0xD3, 0xAC, 0x62, 0x91, 0x95, 0xE4, 0x79,
    0xE7, 0xC8, 0x37, 0x6D, 0x8D, 0xD5, 0x4E, 0xA9, 0x6C, 0x56, 0xF4, 0xEA, 0x65, 0x7A, 0xAE, 0x08,
    0xBA, 0x78, 0x25, 0x2E, 0x1C, 0xA6, 0xB4, 0xC6, 0xE8, 0xDD, 0x74, 0x1F, 0x4B, 0xBD, 0x8B, 0x8A,
    0x70, 0x3E, 0xB5, 0x66, 0x48, 0x03, 0xF6, 0x0E, 0x61, 0x35, 0x57, 0xB9, 0x86, 0xC1, 0x1D, 0x9E,
    0xE1, 0xF8, 0x98, 0x11, 0x69, 0xD9, 0x8E, 0x94, 0x9B, 0x1E, 0x87, 0xE9, 0xCE, 0x55, 0x28, 0xDF,
    0x8C, 0xA1, 0x89, 0x0D, 0xBF, 0xE6, 0x42, 0x68, 0x41, 0x99, 0x2D, 0x0F, 0xB0, 0x54, 0xBB, 0x16
};

static const uint8_t inv_sbox[256] =
{
    0x52, 0x09, 0x6A, 0xD5, 0x30, 0x36, 0xA5, 0x38, 0xBF, 0x40, 0xA3, 0x9E, 0x81, 0xF3, 0xD7, 0xFB,
    0x7C, 0xE3, 0x39, 0x82, 0x9B, 0x2F, 0xFF, 0x87, 0x34, 0x8E, 0x43, 0x44, 0xC4, 0xDE, 0xE9, 0xCB,
    0x54, 0x7B, 0x94, 0x32, 0xA6, 0xC2, 0x23, 0x3D, 0xEE, 0x4C, 0x95, 0x0B, 0x42, 0xFA, 0xC3, 0x4E,
    0x08, 0x2E, 0xA1, 0x66, 0x28, 0xD9, 0x24, 0xB2, 0x76, 0x5B, 0xA2, 0x49, 0x6D, 0x8B, 0xD1, 0x25,
    0x72, 0xF8, 0xF6, 0x64, 0x86, 0x68, 0x98, 0x16, 0xD4, 0xA4, 0x5C, 0xCC, 0x5D, 0x65, 0xB6, 0x92,
    0x6C, 0x70, 0x48, 0x50, 0xFD, 0xED, 0xB9, 0xDA, 0x5E, 0x15, 0x46, 0x57, 0xA7, 0x8D, 0x9D, 0x84,
    0x90, 0xD8, 0xAB, 0x00, 0x8C, 0xBC, 0xD3, 0x0A, 0xF7, 0xE4, 0x58, 0x05, 0xB8, 0xB3, 0x45, 0x06,
    0xD0, 0x2C, 0x1E, 0x8F, 0xCA, 0x3F, 0x0F, 0x02, 0xC1, 0xAF, 0xBD, 0x03, 0x01, 0x13, 0x8A, 0x6B,
    0x3A, 0x91, 0x11, 0x41, 0x4F, 0x67, 0xDC, 0xEA, 0x97, 0xF2, 0xCF, 0xCE, 0xF0, 0xB4, 0xE6, 0x73,
    0x96, 0xAC, 0x74, 0x22, 0xE7, 0xAD, 0x35, 0x85, 0xE2, 0xF9, 0x37, 0xE8, 0x1C, 0x75, 0xDF, 0x6E,
    0x47, 0xF1, 0x1A, 0x71, 0x1D, 0x29, 0xC5, 0x89, 0x6F, 0xB7, 0x62, 0x0E, 0xAA, 0x18, 0xBE, 0x1B,
    0xFC, 0x56, 0x3E, 0x4B, 0xC6, 0xD2, 0x79, 0x20, 0x9A, 0xDB, 0xC0, 0xFE, 0x78, 0xCD, 0x5A, 0xF4,
    0x1F, 0xDD, 0xA8, 0x33, 0x88, 0x07, 0xC7, 0x31, 0xB1, 0x12, 0x10, 0x59, 0x27, 0x80, 0xEC, 0x5F,
    0x60, 0x51, 0x7F, 0xA9, 0x19, 0xB5, 0x4A, 0x0D, 0x2D, 0xE5, 0x7A, 0x9F, 0x93, 0xC9, 0x9C, 0xEF,
    0xA0, 0xE0, 0x3B, 0x4D, 0xAE, 0x2A, 0xF5, 0xB0, 0xC8, 0xEB, 0xBB, 0x3C, 0x83, 0x53, 0x99, 0x61,
    0x17, 0x2B, 0x04, 0x7E, 0xBA, 0x77, 0xD6, 0x26, 0xE1, 0x69, 0x14, 0x63, 0x55, 0x21, 0x0C, 0x7D
};

//round constants for key expansion
static const uint8_t rcon[255] =
{
    0x8d, 0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80, 0x1b, 0x36, 0x6c, 0xd8, 0xab, 0x4d, 0x9a,
    0x2f, 0x5e, 0xbc, 0x63, 0xc6, 0x97, 0x35, 0x6a, 0xd4, 0xb3, 0x7d, 0xfa, 0xef, 0xc5, 0x91, 0x39,
    0x72, 0xe4, 0xd3, 0xbd, 0x61, 0xc2, 0x9f, 0x25, 0x4a, 0x94, 0x33, 0x66, 0xcc, 0x83, 0x1d, 0x3a,
    0x74, 0xe8, 0xcb, 0x8d, 0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80, 0x1b, 0x36, 0x6c, 0xd8,
    0xab, 0x4d, 0x9a, 0x2f, 0x5e, 0xbc, 0x63, 0xc6, 0x97, 0x35, 0x6a, 0xd4, 0xb3, 0x7d, 0xfa, 0xef,
    0xc5, 0x91, 0x39, 0x72, 0xe4, 0xd3, 0xbd, 0x61, 0xc2, 0x9f, 0x25, 0x4a, 0x94, 0x33, 0x66, 0xcc,
    0x83, 0x1d, 0x3a, 0x74, 0xe8, 0xcb, 0x8d, 0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80, 0x1b,
    0x36, 0x6c, 0xd8, 0xab, 0x4d, 0x9a, 0x2f, 0x5e, 0xbc, 0x63, 0xc6, 0x97, 0x35, 0x6a, 0xd4, 0xb3,
    0x7d, 0xfa, 0xef, 0xc5, 0x91, 0x39, 0x72, 0xe4, 0xd3, 0xbd, 0x61, 0xc2, 0x9f, 0x25, 0x4a, 0x94,
    0x33, 0x66, 0xcc, 0x83, 0x1d, 0x3a, 0x74, 0xe8, 0xcb, 0x8d, 0x01, 0x02, 0x04, 0x08, 0x10, 0x20,
    0x40, 0x80, 0x1b, 0x36, 0x6c, 0xd8, 0xab, 0x4d, 0x9a, 0x2f, 0x5e, 0xbc, 0x63, 0xc6, 0x97, 0x35,
    0x6a, 0xd4, 0xb3, 0x7d, 0xfa, 0xef, 0xc5, 0x91, 0x39, 0x72, 0xe4, 0xd3, 0xbd, 0x61, 0xc2, 0x9f,
    0x25, 0x4a, 0x94, 0x33, 0x66, 0xcc, 0x83, 0x1d, 0x3a, 0x74, 0xe8, 0xcb, 0x8d, 0x01, 0x02, 0x04,
    0x08, 0x10, 0x20, 0x40, 0x80, 0x1b, 0x36, 0x6c, 0xd8, 0xab, 0x4d, 0x9a, 0x2f, 0x5e, 0xbc, 0x63,
    0xc6, 0x97, 0x35, 0x6a, 0xd4, 0xb3, 0x7d, 0xfa, 0xef, 0xc5, 0x91, 0x39, 0x72, 0xe4, 0xd3, 0xbd,
    0x61, 0xc2, 0x9f, 0x25, 0x4a, 0x94, 0x33, 0x66, 0xcc, 0x83, 0x1d, 0x3a, 0x74, 0xe8, 0xcb
};


/*
Internal function declaration
*/
static void mix_columns(aes_ctx_t *ctx);
static void key_expansion(aes_ctx_t *ctx, int current_key_size, int required_key_size);
static void sub_bytes(aes_ctx_t *ctx);
static void shift_rows(aes_ctx_t *ctx);

static void inv_sub_bytes(aes_ctx_t *ctx);
static void inv_shift_rows(aes_ctx_t *ctx);
static void inv_mix_columns(aes_ctx_t *ctx);

static void add_roundkey(aes_ctx_t *ctx, int current_round);

//for MixColumns
static uint8_t multiply(uint8_t a, uint8_t b);

//for key_expansion
static void key_expansion_core(uint8_t *word, int iteration);
static void rotate(uint8_t *word);


void AES_encrypt(aes_ctx_t *ctx, uint8_t *in, uint8_t *out)
{
    int i;
    int current_round;  //increasing


    //copy input to state, by column
    for(i = 0; i < AES_BLOCKSIZE; i++)
        ctx->state[i%4][i/4] = in[i];

    //round 0
    add_roundkey(ctx, 0);


    for(current_round = 1; current_round < ctx->rounds; current_round++)
    {
        sub_bytes(ctx);
        shift_rows(ctx);
        mix_columns(ctx);
        add_roundkey(ctx, current_round);

    }

    //last round
    sub_bytes(ctx);
    shift_rows(ctx);
    add_roundkey(ctx, ctx->rounds);


    //copy state to output, by column
    for(i = 0; i < AES_BLOCKSIZE; i++)
        out[i] = ctx->state[i%4][i/4];


}

void AES_decrypt(aes_ctx_t *ctx, uint8_t *in, uint8_t *out)
{
    int i;
    int current_round;  //decreasing


    for(i = 0; i < AES_BLOCKSIZE; i++)
        ctx->state[i%4][i/4] = in[i];

    //first round
    add_roundkey(ctx, ctx->rounds);

    for(current_round = ctx->rounds-1; current_round > 0; current_round--)
    {
        inv_shift_rows(ctx);
        inv_sub_bytes(ctx);
        add_roundkey(ctx, current_round);
        inv_mix_columns(ctx);
    }

    //last round
    inv_shift_rows(ctx);
    inv_sub_bytes(ctx);
    add_roundkey(ctx, 0);

    //copy state to output, by column
    for(i = 0; i < AES_BLOCKSIZE; i++)
        out[i] = ctx->state[i%4][i/4];

}


aes_ctx_t * AES_ctx_alloc(uint8_t *key, size_t keylen)
{
    aes_ctx_t *ctx;
    int rounds;
    size_t round_key_size;

    switch (keylen)
    {
        case 16:
            rounds = 10;    //AES128
            break;
        case 24:
            rounds = 12;    //AES192
            break;
        case 32:
            rounds = 14;    //AES256
            break;
        default:
            return NULL;
    }

    round_key_size = (rounds+1) * AES_BLOCKSIZE;
    ctx = malloc(sizeof(aes_ctx_t) + round_key_size);
    if(ctx)
    {
        ctx->rounds = rounds;
        ctx->keylen = keylen;
        memcpy(ctx->roundkey, key, keylen);
        key_expansion(ctx, keylen, round_key_size);
    }
    return ctx;
}

static void key_expansion(aes_ctx_t *ctx, int current_key_size, int required_key_size)
{
    int iteration = 1;
    int i;
    uint8_t t[4];

    while(current_key_size < required_key_size)
    {
        for(i = 0; i < 4; i++)
            t[i] = ctx->roundkey[current_key_size - 4 + i];

        if(current_key_size % ctx->keylen == 0)
            //apply core schedule every keylen(16,24,32) bytes
            key_expansion_core(t, iteration++);

        //additional sbox operation for AES256
        if(ctx->keylen==32 && ((current_key_size % ctx->keylen) == 16))
        {
            for(i = 0; i < 4; i++)
                t[i] = sbox[t[i]];
        }

        for(i = 0; i < 4; i++)
        {
            ctx->roundkey[current_key_size] = ctx->roundkey[current_key_size - ctx->keylen] ^ t[i];
            current_key_size++;
        }
    }
}

//rotate(0x12345678) -> 0x34567812
static void inline rotate(uint8_t *word)
{
    uint8_t c;
    c = word[0];
    word[0] = word[1];
    word[1] = word[2];
    word[2] = word[3];
    word[3] = c;
}

//core function for key_expansion
static void key_expansion_core(uint8_t *word, int iteration)
{
    int i;
    rotate(word);
    for (i = 0; i < 4; ++i)
        word[i] = sbox[word[i]];
    word[0] ^= rcon[iteration];
}

static void sub_bytes(aes_ctx_t *ctx)
{
    int i, j;

    for(i = 0; i < 4; i++)
        for(j = 0; j < 4; j++)
        ctx->state[i][j] = sbox[ctx->state[i][j]];
}

static void inv_sub_bytes(aes_ctx_t *ctx)
{
    int i, j;

    for(i = 0; i < 4; i++)
        for(j = 0; j < 4; j++)
        ctx->state[i][j] = inv_sbox[ctx->state[i][j]];
}

//ShiftRows for encryption
static void shift_rows(aes_ctx_t *ctx)
{
    uint8_t new_state[4][4];
    int i, j;

    for(i = 0; i < 4; i++)
        for(j = 0; j < 4; j++)
            new_state[i][j] = ctx->state[i][(i+j) % 4];

    memcpy(ctx->state, new_state, sizeof(ctx->state));
}

//Inverse ShiftRows for decryption
static void inv_shift_rows(aes_ctx_t *ctx)
{
    uint8_t new_state[4][4];
    int i, j;

    for(i = 0; i < 4; i++)
        for(j = 0; j < 4; j++)
            new_state[i][j] = ctx->state[i][(j-i+4) % 4];

    memcpy(ctx->state, new_state, sizeof(ctx->state));
}

//MixColumns for encryption
static void mix_columns(aes_ctx_t *ctx)
{
    uint8_t new_state[4][4];
    int i;

    for(i = 0; i < 4; i++) {
        new_state[0][i] = multiply(2, ctx->state[0][i]) ^
                multiply(3, ctx->state[1][i]) ^
                ctx->state[2][i] ^ ctx->state[3][i];

        new_state[1][i] = ctx->state[0][i] ^
                multiply(2, ctx->state[1][i]) ^
                multiply(3, ctx->state[2][i]) ^
                ctx->state[3][i];

        new_state[2][i] = ctx->state[0][i] ^ ctx->state[1][i] ^
                multiply(2, ctx->state[2][i]) ^
                multiply(3, ctx->state[3][i]);

        new_state[3][i] = multiply(3, ctx->state[0][i]) ^
                ctx->state[1][i] ^ ctx->state[2][i] ^
                multiply(2, ctx->state[3][i]);
    }

    memcpy(ctx->state, new_state, sizeof(ctx->state));
}

//Inversed MixColumns for decryption
static void inv_mix_columns(aes_ctx_t *ctx)
{
    uint8_t new_state[4][4];
    int i;

    for(i = 0; i < 4; i++) {
        new_state[0][i] = multiply(0x0e, ctx->state[0][i]) ^
                multiply(0x0b, ctx->state[1][i]) ^
                multiply(0x0d, ctx->state[2][i]) ^
                multiply(0x09, ctx->state[3][i]);

        new_state[1][i] = multiply(0x09, ctx->state[0][i]) ^
                multiply(0x0e, ctx->state[1][i]) ^
                multiply(0x0b, ctx->state[2][i]) ^
                multiply(0x0d, ctx->state[3][i]);

        new_state[2][i] = multiply(0x0d, ctx->state[0][i]) ^
                multiply(0x09, ctx->state[1][i]) ^
                multiply(0x0e, ctx->state[2][i]) ^
                multiply(0x0b, ctx->state[3][i]);

        new_state[3][i] = multiply(0x0b, ctx->state[0][i]) ^
                multiply(0x0d, ctx->state[1][i]) ^
                multiply(0x09, ctx->state[2][i]) ^
                multiply(0x0e, ctx->state[3][i]);
    }

    memcpy(ctx->state, new_state, sizeof(ctx->state));
}

static void add_roundkey(aes_ctx_t *ctx, int current_round)
{
    int i, j;

    for(i = 0; i < 4; i++)
        for(j = 0; j < 4; j++)
            ctx->state[i][j] ^= ctx->roundkey[current_round * AES_BLOCKSIZE + j * 4 + i];
}


#define xtime(x)   ((x<<1) ^ (((x>>7) & 1) * 0x1b))

//multiply in GF(2^8)
static uint8_t multiply(uint8_t a, uint8_t b) {
   int i;

   uint8_t c = 0;
   uint8_t d = b;

   for (i = 0 ; i < 8 ; i++)
   {
      if (a % 2 == 1)
        c ^= d;
      a /= 2;
      d = xtime(d);
   }
   return c;
}

unsigned long AES_CBC_encrypt(uint8_t* input, uint8_t* output, unsigned long length, uint8_t* key, size_t keylen, uint8_t* iv)
{
    aes_ctx_t* ctx;
    uint8_t* previous_block_ciphertext = iv;
    unsigned long i;
    unsigned long output_length;

    ctx = AES_ctx_alloc(key, keylen);

    for (i = 0; i < length; i += AES_BLOCKSIZE)
    {
        block_copy(output, input);
        block_xor(output, previous_block_ciphertext);
        AES_encrypt(ctx, output, output);
        previous_block_ciphertext = output;

        output += AES_BLOCKSIZE;
        input += AES_BLOCKSIZE;
    }
    output_length = (length / AES_BLOCKSIZE) * AES_BLOCKSIZE;
    i = length % AES_BLOCKSIZE;
    if (i > 0)
    {
        // puts("additional block");
        //add zero padding
        memset(output, 0, AES_BLOCKSIZE);
        memcpy(output, input, i);
        block_xor(output, previous_block_ciphertext);
        AES_encrypt(ctx, output, output);
        output_length += AES_BLOCKSIZE;
    }
    free(ctx);

    return output_length;
}

void AES_CBC_decrypt(uint8_t* input, uint8_t* output, unsigned long length, uint8_t* key, size_t keylen, uint8_t* iv)
{
    aes_ctx_t* ctx;
    uint8_t* previous_block_ciphertext = iv;
    unsigned long i;

    ctx = AES_ctx_alloc(key, keylen);

    for (i = 0; i < length; i += AES_BLOCKSIZE)
    {
        block_copy(output, input);
        AES_decrypt(ctx, output, output);
        block_xor(output, previous_block_ciphertext);

        previous_block_ciphertext = input;
        output += AES_BLOCKSIZE;
        input += AES_BLOCKSIZE;
    }

    i = length % AES_BLOCKSIZE;
    if (i > 0)
    {
        block_copy(output, input);
        AES_decrypt(ctx, output, output);
        block_xor(output, previous_block_ciphertext);
        memset(output + i, 0, AES_BLOCKSIZE - i);
    }
}


unsigned long AES_ECB_encrypt(uint8_t* input, uint8_t* output, unsigned long length, uint8_t* key, size_t keylen)
{
    aes_ctx_t* ctx;

    unsigned long i;
    unsigned long output_length;

    ctx = AES_ctx_alloc(key, keylen);

    for (i = 0; i < length; i += AES_BLOCKSIZE)
    {
        block_copy(output, input);
        AES_encrypt(ctx, output, output);

        output += AES_BLOCKSIZE;
        input += AES_BLOCKSIZE;
    }
    output_length = (length / AES_BLOCKSIZE) * AES_BLOCKSIZE;
    i = length % AES_BLOCKSIZE;
    if (i > 0)
    {
        // puts("additional block");
        //add zero padding
        memset(output, 0, AES_BLOCKSIZE);
        memcpy(output, input, i);
        AES_encrypt(ctx, output, output);
        output_length += AES_BLOCKSIZE;
    }
    free(ctx);

    return output_length;
}

void AES_ECB_decrypt(uint8_t* input, uint8_t* output, unsigned long length, uint8_t* key, size_t keylen)
{
    aes_ctx_t* ctx;
    unsigned long i;

    ctx = AES_ctx_alloc(key, keylen);

    for (i = 0; i < length; i += AES_BLOCKSIZE)
    {
        block_copy(output, input);
        AES_decrypt(ctx, output, output);
        output += AES_BLOCKSIZE;
        input += AES_BLOCKSIZE;
    }

    i = length % AES_BLOCKSIZE;
    if (i > 0)
    {
        block_copy(output, input);
        AES_decrypt(ctx, output, output);
        memset(output + i, 0, AES_BLOCKSIZE - i);
    }
}

#ifndef __AES_H__
#define __AES_H__

#include <stdint.h>
#include <stdlib.h>

#define AES_BLOCKSIZE 16

typedef struct {
    uint8_t state[4][4];
    int rounds;
    int keylen;
    uint8_t roundkey[0];    //allocate memory at runtime according to keysize
} aes_ctx_t;

aes_ctx_t * AES_ctx_alloc(uint8_t *key, size_t keylen);

void AES_decrypt(aes_ctx_t *ctx, uint8_t *in, uint8_t *out);
void AES_encrypt(aes_ctx_t *ctx, uint8_t *in, uint8_t *out);

unsigned long AES_CBC_encrypt(uint8_t* input, uint8_t* output, unsigned long length, uint8_t* key, size_t keylen, uint8_t* iv);
void AES_CBC_decrypt(uint8_t* input, uint8_t* output, unsigned long length, uint8_t* key, size_t keylen, uint8_t* iv);

unsigned long AES_ECB_encrypt(uint8_t* input, uint8_t* output, unsigned long length, uint8_t* key, size_t keylen);
void AES_ECB_decrypt(uint8_t* input, uint8_t* output, unsigned long length, uint8_t* key, size_t keylen);


#endif

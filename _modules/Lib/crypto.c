#include "_crc.h"
#include <defines.h>
#include "board.h"


#if NEED_RC4

void cryptoRC4_init (uint8_t *key)
{
    uint16_t i, j;
    uint16_t keyLength = sizeof(key);
    uint8_t S[256];
    
    for (i = 0; i < 256; i++)
    {
        S[i] = (uint8_t)i;
    }
    for (i = 0; i < 256; i++)
    {
        j = (j + S[i] + key[i % keyLength]) % 256;
        //S.Swap(i, j); 
//         S[i] = S[i] ^ S[j];
//         S[j] = S[j] ^ S[i];
//         S[i] = S[i] ^ S[j];    
        S[i] ^= S[j];
        S[j] ^= S[i];
        S[i] ^= S[j];
    }
}


void cryptoRC4_encrypt (uint8_t *data)
{
    uint16_t i, j;
    uint16_t dataLength = sizeof(data);
    uint8_t S[256];
    


}


void cryptoRC4_decrypt (uint8_t* data)
{
    cryptoRC4_encrypt (data);
}

#endif

//
#warning //http://we.easyelectronics.ru/x893/rc5-symmetric-key-block-cipher-stm32.html
//



#if NEED_TEA

void cryptoTEA_encrypt (uint32_t* v, uint32_t* key)
{
    uint32_t v0 = v[0], v1 = v[1], sum = 0, i;           /* set up */
    uint32_t delta = 0x9e3779b9;                   /* a key schedule constant */
    uint32_t k0 = key[0], k1 = key[1], k2 = key[2], k3 = key[3];   /* cache key */
    
    for (i = 0; i < 32; i++)
    {                       /* basic cycle start */
        sum += delta;
        v0 += ((v1<<4) + k0) ^ (v1 + sum) ^ ((v1>>5) + k1);
        v1 += ((v0<<4) + k2) ^ (v0 + sum) ^ ((v0>>5) + k3);
    }                                              /* end cycle */
    v[0] = v0;
    v[1] = v1;
}

void cryptoTEA_decrypt (uint32_t* v, uint32_t* key)
{
    uint32_t v0 = v[0], v1 = v[1], sum = 0xC6EF3720, i;       /* set up */
    uint32_t delta = 0x9e3779b9;                        /* a key schedule constant */
    uint32_t k0 = key[0], k1 = key[1], k2 = key[2], k3 = key[3];        /* cache key */
    
    for (i = 0; i < 32; i++)
    {                              /* basic cycle start */
        v1 -= ((v0<<4) + k2) ^ (v0 + sum) ^ ((v0>>5) + k3);
        v0 -= ((v1<<4) + k0) ^ (v1 + sum) ^ ((v1>>5) + k1);
        sum -= delta;                                   
    }                                                   /* end cycle */
    v[0] = v0;
    v[1] = v1;
}

#endif
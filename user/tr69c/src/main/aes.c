/*
 *  FIPS-197 compliant AES implementation
 *
 *  Copyright (C) 2003-2006  Christophe Devine
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License, version 2.1 as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 *  MA  02110-1301  USA
 */
/*
 *  The AES block cipher was designed by Vincent Rijmen and Joan Daemen.
 *
 *  http://csrc.nist.gov/encryption/aes/rijndael/Rijndael.pdf
 *  http://csrc.nist.gov/publications/fips/fips197/fips-197.pdf
 */

#ifndef _CRT_SECURE_NO_DEPRECATE
    #define _CRT_SECURE_NO_DEPRECATE 1
#endif

#include <string.h>

#include "aes.h"


#ifndef uint8
    #define uint8 unsigned char
#endif

#ifndef uint32
    #define uint32 unsigned long
#endif

/*
 * 32-bit integer manipulation macros (big endian)
 */
#ifndef GET_UINT32_BE
#define GET_UINT32_BE(n,b,i)                            \
    {                                                       \
        (n) = ( (uint32) (b)[(i)    ] << 24 )        \
              | ( (uint32) (b)[(i) + 1] << 16 )        \
              | ( (uint32) (b)[(i) + 2] <<  8 )        \
              | ( (uint32) (b)[(i) + 3]       );       \
    }
#endif
#ifndef PUT_UINT32_BE
#define PUT_UINT32_BE(n,b,i)                            \
    {                                                       \
        (b)[(i)    ] = (uint8) ( (n) >> 24 );       \
        (b)[(i) + 1] = (uint8) ( (n) >> 16 );       \
        (b)[(i) + 2] = (uint8) ( (n) >>  8 );       \
        (b)[(i) + 3] = (uint8) ( (n)       );       \
    }
#endif


/*
 * Forward S-box & tables
 */
static uint8 FSb[256];
static uint32 FT0[256];
static uint32 FT1[256];
static uint32 FT2[256];
static uint32 FT3[256];

/*
 * Reverse S-box & tables
 */
static uint8 RSb[256];
static uint32 RT0[256];
static uint32 RT1[256];
static uint32 RT2[256];
static uint32 RT3[256];

/*
 * Round constants
 */
static uint32 RCON[10];

/*
 * Tables generation code
 */
#define ROTR8(x) ( ( ( x << 24 ) & 0xFFFFFFFF ) | \
                   ( ( x & 0xFFFFFFFF ) >> 8 ) )
#define XTIME(x) ( ( x << 1 ) ^ ( ( x & 0x80 ) ? 0x1B : 0x00 ) )
#define MUL(x,y) ( ( x && y ) ? pow[(log[x] + log[y]) % 255] : 0 )

static void aes_gen_tables(void)
{
    int i;
    uint8 x, y;
    uint8 pow[256];
    uint8 log[256];

    /*
     * compute pow and log tables over GF(2^8)
     */
    for (i = 0, x = 1; i < 256; i++, x ^= XTIME(x))
    {
        pow[i] = x;
        log[x] = i;
    }

    /*
     * calculate the round constants
     */
    for (i = 0, x = 1; i < 10; i++, x = XTIME(x))
    {
        RCON[i] = (uint32) x << 24;
    }

    /*
     * generate the forward and reverse S-boxes
     */
    FSb[0x00] = 0x63;
    RSb[0x63] = 0x00;

    for (i = 1; i < 256; i++)
    {
        x = pow[255 - log[i]];

        y  = x;
        y = (y << 1) | (y >> 7);
        x ^= y;
        y = (y << 1) | (y >> 7);
        x ^= y;
        y = (y << 1) | (y >> 7);
        x ^= y;
        y = (y << 1) | (y >> 7);
        x ^= y ^ 0x63;

        FSb[i] = x;
        RSb[x] = i;
    }

    /*
     * generate the forward and reverse tables
     */
    for (i = 0; i < 256; i++)
    {
        x = FSb[i];
        y = XTIME(x);

        FT0[i] = (uint32)(x ^ y) ^
                 ((uint32) x <<  8) ^
                 ((uint32) x << 16) ^
                 ((uint32) y << 24);

        FT0[i] &= 0xFFFFFFFF;

        FT1[i] = ROTR8(FT0[i]);
        FT2[i] = ROTR8(FT1[i]);
        FT3[i] = ROTR8(FT2[i]);

        y = RSb[i];

        RT0[i] = ((uint32) MUL(0x0B, y)) ^
                 ((uint32) MUL(0x0D, y) <<  8) ^
                 ((uint32) MUL(0x09, y) << 16) ^
                 ((uint32) MUL(0x0E, y) << 24);

        RT0[i] &= 0xFFFFFFFF;

        RT1[i] = ROTR8(RT0[i]);
        RT2[i] = ROTR8(RT1[i]);
        RT3[i] = ROTR8(RT2[i]);
    }
}


/*
 * Decryption key schedule tables
 */
static uint32 KT0[256];
static uint32 KT1[256];
static uint32 KT2[256];
static uint32 KT3[256];

/*
 * AES key schedule
 */
void aes_set_key(aes_context *ctx, uint8 *key, int keysize)
{
    int i;
    uint32 *RK, *SK;
    static int ft_init = 0;
    static int kt_init = 0;

    if (ft_init == 0)
    {
        aes_gen_tables();

        ft_init = 1;
    }

    switch (keysize)
    {
        case 128:
            ctx->nr = 10;
            break;
        case 192:
            ctx->nr = 12;
            break;
        case 256:
            ctx->nr = 14;
            break;
        default :
            return;
    }

    RK = ctx->erk;

    for (i = 0; i < (keysize >> 5); i++)
    {
        GET_UINT32_BE(RK[i], key, i << 2);
    }

    /*
     * setup encryption round keys
     */
    switch (ctx->nr)
    {
        case 10:

            for (i = 0; i < 10; i++, RK += 4)
            {
                RK[4]  = RK[0] ^ RCON[i] ^
                         (FSb[(uint8)(RK[3] >> 16) ] << 24) ^
                         (FSb[(uint8)(RK[3] >>  8) ] << 16) ^
                         (FSb[(uint8)(RK[3]) ] <<  8) ^
                         (FSb[(uint8)(RK[3] >> 24) ]);

                RK[5]  = RK[1] ^ RK[4];
                RK[6]  = RK[2] ^ RK[5];
                RK[7]  = RK[3] ^ RK[6];
            }
            break;

        case 12:

            for (i = 0; i < 8; i++, RK += 6)
            {
                RK[6]  = RK[0] ^ RCON[i] ^
                         (FSb[(uint8)(RK[5] >> 16) ] << 24) ^
                         (FSb[(uint8)(RK[5] >>  8) ] << 16) ^
                         (FSb[(uint8)(RK[5]) ] <<  8) ^
                         (FSb[(uint8)(RK[5] >> 24) ]);

                RK[7]  = RK[1] ^ RK[6];
                RK[8]  = RK[2] ^ RK[7];
                RK[9]  = RK[3] ^ RK[8];
                RK[10] = RK[4] ^ RK[9];
                RK[11] = RK[5] ^ RK[10];
            }
            break;

        case 14:

            for (i = 0; i < 7; i++, RK += 8)
            {
                RK[8]  = RK[0] ^ RCON[i] ^
                         (FSb[(uint8)(RK[7] >> 16) ] << 24) ^
                         (FSb[(uint8)(RK[7] >>  8) ] << 16) ^
                         (FSb[(uint8)(RK[7]) ] <<  8) ^
                         (FSb[(uint8)(RK[7] >> 24) ]);

                RK[9]  = RK[1] ^ RK[8];
                RK[10] = RK[2] ^ RK[9];
                RK[11] = RK[3] ^ RK[10];

                RK[12] = RK[4] ^
                         (FSb[(uint8)(RK[11] >> 24) ] << 24) ^
                         (FSb[(uint8)(RK[11] >> 16) ] << 16) ^
                         (FSb[(uint8)(RK[11] >>  8) ] <<  8) ^
                         (FSb[(uint8)(RK[11]) ]);

                RK[13] = RK[5] ^ RK[12];
                RK[14] = RK[6] ^ RK[13];
                RK[15] = RK[7] ^ RK[14];
            }
            break;

        default:

            break;
    }

    /*
     * setup decryption round keys
     */
    if (kt_init == 0)
    {
        for (i = 0; i < 256; i++)
        {
            KT0[i] = RT0[ FSb[i] ];
            KT1[i] = RT1[ FSb[i] ];
            KT2[i] = RT2[ FSb[i] ];
            KT3[i] = RT3[ FSb[i] ];
        }

        kt_init = 1;
    }

    SK = ctx->drk;

    *SK++ = *RK++;
    *SK++ = *RK++;
    *SK++ = *RK++;
    *SK++ = *RK++;

    for (i = 1; i < ctx->nr; i++)
    {
        RK -= 8;

        *SK++ = KT0[(uint8)(*RK >> 24) ] ^
                KT1[(uint8)(*RK >> 16) ] ^
                KT2[(uint8)(*RK >>  8) ] ^
                KT3[(uint8)(*RK) ];
        RK++;

        *SK++ = KT0[(uint8)(*RK >> 24) ] ^
                KT1[(uint8)(*RK >> 16) ] ^
                KT2[(uint8)(*RK >>  8) ] ^
                KT3[(uint8)(*RK) ];
        RK++;

        *SK++ = KT0[(uint8)(*RK >> 24) ] ^
                KT1[(uint8)(*RK >> 16) ] ^
                KT2[(uint8)(*RK >>  8) ] ^
                KT3[(uint8)(*RK) ];
        RK++;

        *SK++ = KT0[(uint8)(*RK >> 24) ] ^
                KT1[(uint8)(*RK >> 16) ] ^
                KT2[(uint8)(*RK >>  8) ] ^
                KT3[(uint8)(*RK) ];
        RK++;
    }

    RK -= 8;

    *SK++ = *RK++;
    *SK++ = *RK++;
    *SK++ = *RK++;
    *SK++ = *RK++;
}

/**
 * AES block encryption (ECB mode)
 */
void aes_encrypt(aes_context *ctx,
                 unsigned char input[16],
                 unsigned char output[16])
{
    uint32 *RK, X0, X1, X2, X3, Y0, Y1, Y2, Y3;

    RK = ctx->erk;

    GET_UINT32_BE(X0, input,  0);
    X0 ^= RK[0];
    GET_UINT32_BE(X1, input,  4);
    X1 ^= RK[1];
    GET_UINT32_BE(X2, input,  8);
    X2 ^= RK[2];
    GET_UINT32_BE(X3, input, 12);
    X3 ^= RK[3];

#define AES_FROUND(X0,X1,X2,X3,Y0,Y1,Y2,Y3)             \
    {                                                       \
        RK += 4;                                            \
        \
        X0 = RK[0] ^ FT0[ (uint8) ( Y0 >> 24 ) ] ^          \
             FT1[ (uint8) ( Y1 >> 16 ) ] ^          \
             FT2[ (uint8) ( Y2 >>  8 ) ] ^          \
             FT3[ (uint8) ( Y3       ) ];           \
        \
        X1 = RK[1] ^ FT0[ (uint8) ( Y1 >> 24 ) ] ^          \
             FT1[ (uint8) ( Y2 >> 16 ) ] ^          \
             FT2[ (uint8) ( Y3 >>  8 ) ] ^          \
             FT3[ (uint8) ( Y0       ) ];           \
        \
        X2 = RK[2] ^ FT0[ (uint8) ( Y2 >> 24 ) ] ^          \
             FT1[ (uint8) ( Y3 >> 16 ) ] ^          \
             FT2[ (uint8) ( Y0 >>  8 ) ] ^          \
             FT3[ (uint8) ( Y1       ) ];           \
        \
        X3 = RK[3] ^ FT0[ (uint8) ( Y3 >> 24 ) ] ^          \
             FT1[ (uint8) ( Y0 >> 16 ) ] ^          \
             FT2[ (uint8) ( Y1 >>  8 ) ] ^          \
             FT3[ (uint8) ( Y2       ) ];           \
    }

    AES_FROUND(Y0, Y1, Y2, Y3, X0, X1, X2, X3);
    AES_FROUND(X0, X1, X2, X3, Y0, Y1, Y2, Y3);
    AES_FROUND(Y0, Y1, Y2, Y3, X0, X1, X2, X3);
    AES_FROUND(X0, X1, X2, X3, Y0, Y1, Y2, Y3);
    AES_FROUND(Y0, Y1, Y2, Y3, X0, X1, X2, X3);
    AES_FROUND(X0, X1, X2, X3, Y0, Y1, Y2, Y3);
    AES_FROUND(Y0, Y1, Y2, Y3, X0, X1, X2, X3);
    AES_FROUND(X0, X1, X2, X3, Y0, Y1, Y2, Y3);
    AES_FROUND(Y0, Y1, Y2, Y3, X0, X1, X2, X3);

    if (ctx->nr > 10)
    {
        AES_FROUND(X0, X1, X2, X3, Y0, Y1, Y2, Y3);
        AES_FROUND(Y0, Y1, Y2, Y3, X0, X1, X2, X3);
    }

    if (ctx->nr > 12)
    {
        AES_FROUND(X0, X1, X2, X3, Y0, Y1, Y2, Y3);
        AES_FROUND(Y0, Y1, Y2, Y3, X0, X1, X2, X3);
    }

    RK += 4;

    X0 = RK[0] ^ (FSb[(uint8)(Y0 >> 24) ] << 24) ^
         (FSb[(uint8)(Y1 >> 16) ] << 16) ^
         (FSb[(uint8)(Y2 >>  8) ] <<  8) ^
         (FSb[(uint8)(Y3) ]);

    X1 = RK[1] ^ (FSb[(uint8)(Y1 >> 24) ] << 24) ^
         (FSb[(uint8)(Y2 >> 16) ] << 16) ^
         (FSb[(uint8)(Y3 >>  8) ] <<  8) ^
         (FSb[(uint8)(Y0) ]);

    X2 = RK[2] ^ (FSb[(uint8)(Y2 >> 24) ] << 24) ^
         (FSb[(uint8)(Y3 >> 16) ] << 16) ^
         (FSb[(uint8)(Y0 >>  8) ] <<  8) ^
         (FSb[(uint8)(Y1) ]);

    X3 = RK[3] ^ (FSb[(uint8)(Y3 >> 24) ] << 24) ^
         (FSb[(uint8)(Y0 >> 16) ] << 16) ^
         (FSb[(uint8)(Y1 >>  8) ] <<  8) ^
         (FSb[(uint8)(Y2) ]);

    PUT_UINT32_BE(X0, output,  0);
    PUT_UINT32_BE(X1, output,  4);
    PUT_UINT32_BE(X2, output,  8);
    PUT_UINT32_BE(X3, output, 12);
}

/*
 * AES block decryption (ECB mode)
 */
void aes_decrypt(aes_context *ctx,
                 unsigned char input[16],
                 unsigned char output[16])
{
    uint32 *RK, X0, X1, X2, X3, Y0, Y1, Y2, Y3;

    RK = ctx->drk;

    GET_UINT32_BE(X0, input,  0);
    X0 ^= RK[0];
    GET_UINT32_BE(X1, input,  4);
    X1 ^= RK[1];
    GET_UINT32_BE(X2, input,  8);
    X2 ^= RK[2];
    GET_UINT32_BE(X3, input, 12);
    X3 ^= RK[3];

#define AES_RROUND(X0,X1,X2,X3,Y0,Y1,Y2,Y3)             \
    {                                                       \
        RK += 4;                                            \
        \
        X0 = RK[0] ^ RT0[ (uint8) ( Y0 >> 24 ) ] ^          \
             RT1[ (uint8) ( Y3 >> 16 ) ] ^          \
             RT2[ (uint8) ( Y2 >>  8 ) ] ^          \
             RT3[ (uint8) ( Y1       ) ];           \
        \
        X1 = RK[1] ^ RT0[ (uint8) ( Y1 >> 24 ) ] ^          \
             RT1[ (uint8) ( Y0 >> 16 ) ] ^          \
             RT2[ (uint8) ( Y3 >>  8 ) ] ^          \
             RT3[ (uint8) ( Y2       ) ];           \
        \
        X2 = RK[2] ^ RT0[ (uint8) ( Y2 >> 24 ) ] ^          \
             RT1[ (uint8) ( Y1 >> 16 ) ] ^          \
             RT2[ (uint8) ( Y0 >>  8 ) ] ^          \
             RT3[ (uint8) ( Y3       ) ];           \
        \
        X3 = RK[3] ^ RT0[ (uint8) ( Y3 >> 24 ) ] ^          \
             RT1[ (uint8) ( Y2 >> 16 ) ] ^          \
             RT2[ (uint8) ( Y1 >>  8 ) ] ^          \
             RT3[ (uint8) ( Y0       ) ];           \
    }

    AES_RROUND(Y0, Y1, Y2, Y3, X0, X1, X2, X3);
    AES_RROUND(X0, X1, X2, X3, Y0, Y1, Y2, Y3);
    AES_RROUND(Y0, Y1, Y2, Y3, X0, X1, X2, X3);
    AES_RROUND(X0, X1, X2, X3, Y0, Y1, Y2, Y3);
    AES_RROUND(Y0, Y1, Y2, Y3, X0, X1, X2, X3);
    AES_RROUND(X0, X1, X2, X3, Y0, Y1, Y2, Y3);
    AES_RROUND(Y0, Y1, Y2, Y3, X0, X1, X2, X3);
    AES_RROUND(X0, X1, X2, X3, Y0, Y1, Y2, Y3);
    AES_RROUND(Y0, Y1, Y2, Y3, X0, X1, X2, X3);

    if (ctx->nr > 10)
    {
        AES_RROUND(X0, X1, X2, X3, Y0, Y1, Y2, Y3);
        AES_RROUND(Y0, Y1, Y2, Y3, X0, X1, X2, X3);
    }

    if (ctx->nr > 12)
    {
        AES_RROUND(X0, X1, X2, X3, Y0, Y1, Y2, Y3);
        AES_RROUND(Y0, Y1, Y2, Y3, X0, X1, X2, X3);
    }

    RK += 4;

    X0 = RK[0] ^ (RSb[(uint8)(Y0 >> 24) ] << 24) ^
         (RSb[(uint8)(Y3 >> 16) ] << 16) ^
         (RSb[(uint8)(Y2 >>  8) ] <<  8) ^
         (RSb[(uint8)(Y1) ]);

    X1 = RK[1] ^ (RSb[(uint8)(Y1 >> 24) ] << 24) ^
         (RSb[(uint8)(Y0 >> 16) ] << 16) ^
         (RSb[(uint8)(Y3 >>  8) ] <<  8) ^
         (RSb[(uint8)(Y2) ]);

    X2 = RK[2] ^ (RSb[(uint8)(Y2 >> 24) ] << 24) ^
         (RSb[(uint8)(Y1 >> 16) ] << 16) ^
         (RSb[(uint8)(Y0 >>  8) ] <<  8) ^
         (RSb[(uint8)(Y3) ]);

    X3 = RK[3] ^ (RSb[(uint8)(Y3 >> 24) ] << 24) ^
         (RSb[(uint8)(Y2 >> 16) ] << 16) ^
         (RSb[(uint8)(Y1 >>  8) ] <<  8) ^
         (RSb[(uint8)(Y0) ]);

    PUT_UINT32_BE(X0, output,  0);
    PUT_UINT32_BE(X1, output,  4);
    PUT_UINT32_BE(X2, output,  8);
    PUT_UINT32_BE(X3, output, 12);
}

/*
 * AES-CBC buffer encryption
 */
void aes_cbc_encrypt(aes_context *ctx,
                     unsigned char iv[16],
                     unsigned char *input,
                     unsigned char *output,
                     int len)
{
    int i;

    while (len > 0)
    {
        for (i = 0; i < 16; i++)
            output[i] = input[i] ^ iv[i];

        aes_encrypt(ctx, output, output);
        memcpy(iv, output, 16);

        input  += 16;
        output += 16;
        len    -= 16;
    }
}

/*
 * AES-CBC buffer decryption
 */
void aes_cbc_decrypt(aes_context *ctx,
                     unsigned char iv[16],
                     unsigned char *input,
                     unsigned char *output,
                     int len)
{
    int i;
    unsigned char temp[16];

    while (len > 0)
    {
        memcpy(temp, input, 16);
        aes_decrypt(ctx, input, output);

        for (i = 0; i < 16; i++)
            output[i] = output[i] ^ iv[i];

        memcpy(iv, temp, 16);

        input  += 16;
        output += 16;
        len    -= 16;
    }
}

static const char _aes_src[] = "_aes_src";


#define SELF_TEST
#ifdef SELF_TEST

#include <stdio.h>

/*
 * AES-ECB test vectors (source: NIST, rijndael-vals.zip)
 */
static const uint8 aes_enc_test[3][16] =
{
    {
        0xC3, 0x4C, 0x05, 0x2C, 0xC0, 0xDA, 0x8D, 0x73,
        0x45, 0x1A, 0xFE, 0x5F, 0x03, 0xBE, 0x29, 0x7F
    },
    {
        0xF3, 0xF6, 0x75, 0x2A, 0xE8, 0xD7, 0x83, 0x11,
        0x38, 0xF0, 0x41, 0x56, 0x06, 0x31, 0xB1, 0x14
    },
    {
        0x8B, 0x79, 0xEE, 0xCC, 0x93, 0xA0, 0xEE, 0x5D,
        0xFF, 0x30, 0xB4, 0xEA, 0x21, 0x63, 0x6D, 0xA4
    }
};

static const uint8 aes_dec_test[3][16] =
{
    {
        0x44, 0x41, 0x6A, 0xC2, 0xD1, 0xF5, 0x3C, 0x58,
        0x33, 0x03, 0x91, 0x7E, 0x6B, 0xE9, 0xEB, 0xE0
    },
    {
        0x48, 0xE3, 0x1E, 0x9E, 0x25, 0x67, 0x18, 0xF2,
        0x92, 0x29, 0x31, 0x9C, 0x19, 0xF1, 0x5B, 0xA4
    },
    {
        0x05, 0x8C, 0xCF, 0xFD, 0xBB, 0xCB, 0x38, 0x2D,
        0x1F, 0x6F, 0x56, 0x58, 0x5D, 0x8A, 0x4A, 0xDE
    }
};

/*
 * Checkup routine
 */
int aes_self_test(void)
{
    int i, j, u, v;
    aes_context ctx;
    unsigned char buf[32];

    for (i = 0; i < 6; i++)
    {
        u = i >> 1;
        v = i & 1;

        printf("  AES-ECB-%3d (%s): ", 128 + u * 64,
               (v == 0) ? "enc" : "dec");

        memset(buf, 0, 32);
        aes_set_key(&ctx, buf, 128 + u * 64);

        for (j = 0; j < 10000; j++)
        {
            if (v == 0)
                aes_encrypt(&ctx, buf, buf);
            if (v == 1)
                aes_decrypt(&ctx, buf, buf);
        }

        if ((v == 0 && memcmp(buf, aes_enc_test[u], 16) != 0) ||
                (v == 1 && memcmp(buf, aes_dec_test[u], 16) != 0))
        {
            printf("failed\n");
            return (1);
        }

        printf("passed\n");
    }

    printf("\n");
    return (0);
}
#else
int aes_self_test(void)
{
    return (0);
}
#endif


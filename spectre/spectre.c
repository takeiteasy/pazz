/* spectre.c -- https://github.com/takeiteasy/libspectre
 
 scrypt + sha256 implementation taken from -- https://github.com/technion/libscrypt
 
 The MIT License (MIT)

 Copyright (c) 2022 George Watson

 Permission is hereby granted, free of charge, to any person
 obtaining a copy of this software and associated documentation
 files (the "Software"), to deal in the Software without restriction,
 including without limitation the rights to use, copy, modify, merge,
 publish, distribute, sublicense, and/or sell copies of the Software,
 and to permit persons to whom the Software is furnished to do so,
 subject to the following conditions:

 The above copyright notice and this permission notice shall be
 included in all copies or substantial portions of the Software.

 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
 CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */

#include "spectre.h"
#include <sys/types.h>
#ifndef _WIN32
#include <sys/mman.h>
#endif
#include <errno.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#if !HAVE_DECL_BE64ENC
#undef HAVE_SYS_ENDIAN_H
#endif

#ifdef HAVE_SYS_ENDIAN_H
#include <sys/endian.h>
#else

#include <stdint.h>
#ifdef _MSC_VER
#define INLINE __inline
#else
#define INLINE inline
#endif

static INLINE uint32_t be32dec(const void *pp) {
    const uint8_t *p = (uint8_t const *)pp;
    
    return ((uint32_t)(p[3]) + ((uint32_t)(p[2]) << 8) +
        ((uint32_t)(p[1]) << 16) + ((uint32_t)(p[0]) << 24));
}

static INLINE void be32enc(void *pp, uint32_t x) {
    uint8_t * p = (uint8_t *)pp;

    p[3] = x & 0xff;
    p[2] = (x >> 8) & 0xff;
    p[1] = (x >> 16) & 0xff;
    p[0] = (x >> 24) & 0xff;
}

static INLINE uint32_t le32dec(const void *pp) {
    const uint8_t *p = (uint8_t const *)pp;
    
    return ((uint32_t)(p[0]) + ((uint32_t)(p[1]) << 8) +
        ((uint32_t)(p[2]) << 16) + ((uint32_t)(p[3]) << 24));
}

static INLINE void le32enc(void *pp, uint32_t x) {
    uint8_t * p = (uint8_t *)pp;

    p[0] = x & 0xff;
    p[1] = (x >> 8) & 0xff;
    p[2] = (x >> 16) & 0xff;
    p[3] = (x >> 24) & 0xff;
}
#endif /* !HAVE_SYS_ENDIAN_H */

static void blkcpy(void * dest, void * src, size_t len) {
    size_t * D = dest;
    size_t * S = src;
    size_t L = len / sizeof(size_t);
    size_t i;

    for (i = 0; i < L; i++)
        D[i] = S[i];
}

static void blkxor(void * dest, void * src, size_t len) {
    size_t * D = dest;
    size_t * S = src;
    size_t L = len / sizeof(size_t);
    size_t i;

    for (i = 0; i < L; i++)
        D[i] ^= S[i];
}

static void salsa20_8(uint32_t B[16]) {
    uint32_t x[16];
    size_t i;

    blkcpy(x, B, 64);
    for (i = 0; i < 8; i += 2) {
#define R(a,b) (((a) << (b)) | ((a) >> (32 - (b))))
        /* Operate on columns. */
        x[ 4] ^= R(x[ 0]+x[12], 7);  x[ 8] ^= R(x[ 4]+x[ 0], 9);
        x[12] ^= R(x[ 8]+x[ 4],13);  x[ 0] ^= R(x[12]+x[ 8],18);

        x[ 9] ^= R(x[ 5]+x[ 1], 7);  x[13] ^= R(x[ 9]+x[ 5], 9);
        x[ 1] ^= R(x[13]+x[ 9],13);  x[ 5] ^= R(x[ 1]+x[13],18);

        x[14] ^= R(x[10]+x[ 6], 7);  x[ 2] ^= R(x[14]+x[10], 9);
        x[ 6] ^= R(x[ 2]+x[14],13);  x[10] ^= R(x[ 6]+x[ 2],18);

        x[ 3] ^= R(x[15]+x[11], 7);  x[ 7] ^= R(x[ 3]+x[15], 9);
        x[11] ^= R(x[ 7]+x[ 3],13);  x[15] ^= R(x[11]+x[ 7],18);

        /* Operate on rows. */
        x[ 1] ^= R(x[ 0]+x[ 3], 7);  x[ 2] ^= R(x[ 1]+x[ 0], 9);
        x[ 3] ^= R(x[ 2]+x[ 1],13);  x[ 0] ^= R(x[ 3]+x[ 2],18);

        x[ 6] ^= R(x[ 5]+x[ 4], 7);  x[ 7] ^= R(x[ 6]+x[ 5], 9);
        x[ 4] ^= R(x[ 7]+x[ 6],13);  x[ 5] ^= R(x[ 4]+x[ 7],18);

        x[11] ^= R(x[10]+x[ 9], 7);  x[ 8] ^= R(x[11]+x[10], 9);
        x[ 9] ^= R(x[ 8]+x[11],13);  x[10] ^= R(x[ 9]+x[ 8],18);

        x[12] ^= R(x[15]+x[14], 7);  x[13] ^= R(x[12]+x[15], 9);
        x[14] ^= R(x[13]+x[12],13);  x[15] ^= R(x[14]+x[13],18);
#undef R
    }
    for (i = 0; i < 16; i++)
        B[i] += x[i];
}

static void blockmix_salsa8(uint32_t * Bin, uint32_t * Bout, uint32_t * X, size_t r) {
    size_t i;

    /* 1: X <-- B_{2r - 1} */
    blkcpy(X, &Bin[(2 * r - 1) * 16], 64);

    /* 2: for i = 0 to 2r - 1 do */
    for (i = 0; i < 2 * r; i += 2) {
        /* 3: X <-- H(X \xor B_i) */
        blkxor(X, &Bin[i * 16], 64);
        salsa20_8(X);

        /* 4: Y_i <-- X */
        /* 6: B' <-- (Y_0, Y_2 ... Y_{2r-2}, Y_1, Y_3 ... Y_{2r-1}) */
        blkcpy(&Bout[i * 8], X, 64);

        /* 3: X <-- H(X \xor B_i) */
        blkxor(X, &Bin[i * 16 + 16], 64);
        salsa20_8(X);

        /* 4: Y_i <-- X */
        /* 6: B' <-- (Y_0, Y_2 ... Y_{2r-2}, Y_1, Y_3 ... Y_{2r-1}) */
        blkcpy(&Bout[i * 8 + r * 16], X, 64);
    }
}

static uint64_t integerify(void * B, size_t r) {
    uint32_t *X = (void *)((uintptr_t)(B) + (2 * r - 1) * 64);
    return (((uint64_t)(X[1]) << 32) + X[0]);
}

static void smix(uint8_t * B, size_t r, uint64_t N, uint32_t * V, uint32_t * XY) {
    uint32_t * X = XY;
    uint32_t * Y = &XY[32 * r];
    uint32_t * Z = &XY[64 * r];
    uint64_t i;
    uint64_t j;
    size_t k;

    /* 1: X <-- B */
    for (k = 0; k < 32 * r; k++)
        X[k] = le32dec(&B[4 * k]);

    /* 2: for i = 0 to N - 1 do */
    for (i = 0; i < N; i += 2) {
        /* 3: V_i <-- X */
        blkcpy(&V[i * (32 * r)], X, 128 * r);

        /* 4: X <-- H(X) */
        blockmix_salsa8(X, Y, Z, r);

        /* 3: V_i <-- X */
        blkcpy(&V[(i + 1) * (32 * r)], Y, 128 * r);

        /* 4: X <-- H(X) */
        blockmix_salsa8(Y, X, Z, r);
    }

    /* 6: for i = 0 to N - 1 do */
    for (i = 0; i < N; i += 2) {
        /* 7: j <-- Integerify(X) mod N */
        j = integerify(X, r) & (N - 1);

        /* 8: X <-- H(X \xor V_j) */
        blkxor(X, &V[j * (32 * r)], 128 * r);
        blockmix_salsa8(X, Y, Z, r);

        /* 7: j <-- Integerify(X) mod N */
        j = integerify(Y, r) & (N - 1);

        /* 8: X <-- H(X \xor V_j) */
        blkxor(Y, &V[j * (32 * r)], 128 * r);
        blockmix_salsa8(Y, X, Z, r);
    }

    /* 10: B' <-- X */
    for (k = 0; k < 32 * r; k++)
        le32enc(&B[4 * k], X[k]);
}

typedef struct libscrypt_SHA256Context {
    uint32_t state[8];
    uint32_t count[2];
    unsigned char buf[64];
} SHA256_CTX;

typedef struct libscrypt_HMAC_SHA256Context {
    SHA256_CTX ictx;
    SHA256_CTX octx;
} HMAC_SHA256_CTX;

static void libscrypt_SHA256_Init(SHA256_CTX * ctx) {
    /* Zero bits processed so far */
    ctx->count[0] = ctx->count[1] = 0;

    /* Magic initialization constants */
    ctx->state[0] = 0x6A09E667;
    ctx->state[1] = 0xBB67AE85;
    ctx->state[2] = 0x3C6EF372;
    ctx->state[3] = 0xA54FF53A;
    ctx->state[4] = 0x510E527F;
    ctx->state[5] = 0x9B05688C;
    ctx->state[6] = 0x1F83D9AB;
    ctx->state[7] = 0x5BE0CD19;
}

/* Elementary functions used by SHA256 */
#define Ch(x, y, z)  ((x & (y ^ z)) ^ z)
#define Maj(x, y, z) ((x & (y | z)) | (y & z))
#define SHR(x, n)    (x >> n)
#define ROTR(x, n)   ((x >> n) | (x << (32 - n)))
#define S0(x)        (ROTR(x, 2) ^ ROTR(x, 13) ^ ROTR(x, 22))
#define S1(x)        (ROTR(x, 6) ^ ROTR(x, 11) ^ ROTR(x, 25))
#define s0(x)        (ROTR(x, 7) ^ ROTR(x, 18) ^ SHR(x, 3))
#define s1(x)        (ROTR(x, 17) ^ ROTR(x, 19) ^ SHR(x, 10))

/* SHA256 round function */
#define RND(a, b, c, d, e, f, g, h, k) \
    t0 = h + S1(e) + Ch(e, f, g) + k;  \
    t1 = S0(a) + Maj(a, b, c);         \
    d += t0;                           \
    h  = t0 + t1;

/* Adjusted round function for rotating state */
#define RNDr(S, W, i, k)                  \
    RND(S[(64 - i) % 8], S[(65 - i) % 8], \
        S[(66 - i) % 8], S[(67 - i) % 8], \
        S[(68 - i) % 8], S[(69 - i) % 8], \
        S[(70 - i) % 8], S[(71 - i) % 8], \
        W[i] + k)

static void be32dec_vect(uint32_t *dst, const unsigned char *src, size_t len) {
    size_t i;

    for (i = 0; i < len / 4; i++)
        dst[i] = be32dec(src + i * 4);
}

static void SHA256_Transform(uint32_t * state, const unsigned char block[64]) {
    uint32_t W[64];
    uint32_t S[8];
    uint32_t t0, t1;
    int i;

    /* 1. Prepare message schedule W. */
    be32dec_vect(W, block, 64);
    for (i = 16; i < 64; i++)
        W[i] = s1(W[i - 2]) + W[i - 7] + s0(W[i - 15]) + W[i - 16];

    /* 2. Initialize working variables. */
    memcpy(S, state, 32);

    /* 3. Mix. */
    RNDr(S, W, 0, 0x428a2f98);
    RNDr(S, W, 1, 0x71374491);
    RNDr(S, W, 2, 0xb5c0fbcf);
    RNDr(S, W, 3, 0xe9b5dba5);
    RNDr(S, W, 4, 0x3956c25b);
    RNDr(S, W, 5, 0x59f111f1);
    RNDr(S, W, 6, 0x923f82a4);
    RNDr(S, W, 7, 0xab1c5ed5);
    RNDr(S, W, 8, 0xd807aa98);
    RNDr(S, W, 9, 0x12835b01);
    RNDr(S, W, 10, 0x243185be);
    RNDr(S, W, 11, 0x550c7dc3);
    RNDr(S, W, 12, 0x72be5d74);
    RNDr(S, W, 13, 0x80deb1fe);
    RNDr(S, W, 14, 0x9bdc06a7);
    RNDr(S, W, 15, 0xc19bf174);
    RNDr(S, W, 16, 0xe49b69c1);
    RNDr(S, W, 17, 0xefbe4786);
    RNDr(S, W, 18, 0x0fc19dc6);
    RNDr(S, W, 19, 0x240ca1cc);
    RNDr(S, W, 20, 0x2de92c6f);
    RNDr(S, W, 21, 0x4a7484aa);
    RNDr(S, W, 22, 0x5cb0a9dc);
    RNDr(S, W, 23, 0x76f988da);
    RNDr(S, W, 24, 0x983e5152);
    RNDr(S, W, 25, 0xa831c66d);
    RNDr(S, W, 26, 0xb00327c8);
    RNDr(S, W, 27, 0xbf597fc7);
    RNDr(S, W, 28, 0xc6e00bf3);
    RNDr(S, W, 29, 0xd5a79147);
    RNDr(S, W, 30, 0x06ca6351);
    RNDr(S, W, 31, 0x14292967);
    RNDr(S, W, 32, 0x27b70a85);
    RNDr(S, W, 33, 0x2e1b2138);
    RNDr(S, W, 34, 0x4d2c6dfc);
    RNDr(S, W, 35, 0x53380d13);
    RNDr(S, W, 36, 0x650a7354);
    RNDr(S, W, 37, 0x766a0abb);
    RNDr(S, W, 38, 0x81c2c92e);
    RNDr(S, W, 39, 0x92722c85);
    RNDr(S, W, 40, 0xa2bfe8a1);
    RNDr(S, W, 41, 0xa81a664b);
    RNDr(S, W, 42, 0xc24b8b70);
    RNDr(S, W, 43, 0xc76c51a3);
    RNDr(S, W, 44, 0xd192e819);
    RNDr(S, W, 45, 0xd6990624);
    RNDr(S, W, 46, 0xf40e3585);
    RNDr(S, W, 47, 0x106aa070);
    RNDr(S, W, 48, 0x19a4c116);
    RNDr(S, W, 49, 0x1e376c08);
    RNDr(S, W, 50, 0x2748774c);
    RNDr(S, W, 51, 0x34b0bcb5);
    RNDr(S, W, 52, 0x391c0cb3);
    RNDr(S, W, 53, 0x4ed8aa4a);
    RNDr(S, W, 54, 0x5b9cca4f);
    RNDr(S, W, 55, 0x682e6ff3);
    RNDr(S, W, 56, 0x748f82ee);
    RNDr(S, W, 57, 0x78a5636f);
    RNDr(S, W, 58, 0x84c87814);
    RNDr(S, W, 59, 0x8cc70208);
    RNDr(S, W, 60, 0x90befffa);
    RNDr(S, W, 61, 0xa4506ceb);
    RNDr(S, W, 62, 0xbef9a3f7);
    RNDr(S, W, 63, 0xc67178f2);

    /* 4. Mix local working variables into global state */
    for (i = 0; i < 8; i++)
        state[i] += S[i];
}

static void be32enc_vect(unsigned char *dst, const uint32_t *src, size_t len) {
    size_t i;

    for (i = 0; i < len / 4; i++)
        be32enc(dst + i * 4, src[i]);
}

static void libscrypt_SHA256_Update(SHA256_CTX * ctx, const void *in, size_t len) {
    uint32_t bitlen[2];
    uint32_t r;
    const unsigned char *src = in;

    /* Number of bytes left in the buffer from previous updates */
    r = (ctx->count[1] >> 3) & 0x3f;

    /* Convert the length into a number of bits */
    bitlen[1] = ((uint32_t)len) << 3;
    bitlen[0] = (uint32_t)(len >> 29);

    /* Update number of bits */
    if ((ctx->count[1] += bitlen[1]) < bitlen[1])
        ctx->count[0]++;
    ctx->count[0] += bitlen[0];

    /* Handle the case where we don't need to perform any transforms */
    if (len < 64 - r) {
        memcpy(&ctx->buf[r], src, len);
        return;
    }

    /* Finish the current block */
    memcpy(&ctx->buf[r], src, 64 - r);
    SHA256_Transform(ctx->state, ctx->buf);
    src += 64 - r;
    len -= 64 - r;

    /* Perform complete blocks */
    while (len >= 64) {
        SHA256_Transform(ctx->state, src);
        src += 64;
        len -= 64;
    }

    /* Copy left over data into buffer */
    memcpy(ctx->buf, src, len);
}

static unsigned char PAD[64] = {
    0x80, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};

static void SHA256_Pad(SHA256_CTX * ctx) {
    unsigned char len[8];
    uint32_t r, plen;

    /*
     * Convert length to a vector of bytes -- we do this now rather
     * than later because the length will change after we pad.
     */
    be32enc_vect(len, ctx->count, 8);

    /* Add 1--64 bytes so that the resulting length is 56 mod 64 */
    r = (ctx->count[1] >> 3) & 0x3f;
    plen = (r < 56) ? (56 - r) : (120 - r);
    libscrypt_SHA256_Update(ctx, PAD, (size_t)plen);

    /* Add the terminating bit-count */
    libscrypt_SHA256_Update(ctx, len, 8);
}

static void libscrypt_SHA256_Final(unsigned char digest[32], SHA256_CTX * ctx) {
    /* Add padding */
    SHA256_Pad(ctx);

    /* Write the hash */
    be32enc_vect(digest, ctx->state, 32);

    /* Clear the context state */
    memset((void *)ctx, 0, sizeof(*ctx));
}

static void libscrypt_HMAC_SHA256_Init(HMAC_SHA256_CTX * ctx, const void * _K, size_t Klen) {
    unsigned char pad[64];
    unsigned char khash[32];
    const unsigned char * K = _K;
    size_t i;

    /* If Klen > 64, the key is really SHA256(K). */
    if (Klen > 64) {
        libscrypt_SHA256_Init(&ctx->ictx);
        libscrypt_SHA256_Update(&ctx->ictx, K, Klen);
        libscrypt_SHA256_Final(khash, &ctx->ictx);
        K = khash;
        Klen = 32;
    }

    /* Inner SHA256 operation is SHA256(K xor [block of 0x36] || data). */
    libscrypt_SHA256_Init(&ctx->ictx);
    memset(pad, 0x36, 64);
    for (i = 0; i < Klen; i++)
        pad[i] ^= K[i];
    libscrypt_SHA256_Update(&ctx->ictx, pad, 64);

    /* Outer SHA256 operation is SHA256(K xor [block of 0x5c] || hash). */
    libscrypt_SHA256_Init(&ctx->octx);
    memset(pad, 0x5c, 64);
    for (i = 0; i < Klen; i++)
        pad[i] ^= K[i];
    libscrypt_SHA256_Update(&ctx->octx, pad, 64);
}

static void libscrypt_HMAC_SHA256_Update(HMAC_SHA256_CTX * ctx, const void *in, size_t len) {
    /* Feed data to the inner SHA256 operation. */
    libscrypt_SHA256_Update(&ctx->ictx, in, len);
}

static void libscrypt_HMAC_SHA256_Final(unsigned char digest[32], HMAC_SHA256_CTX * ctx) {
    unsigned char ihash[32];

    /* Finish the inner SHA256 operation. */
    libscrypt_SHA256_Final(ihash, &ctx->ictx);

    /* Feed the inner hash to the outer SHA256 operation. */
    libscrypt_SHA256_Update(&ctx->octx, ihash, 32);

    /* Finish the outer SHA256 operation. */
    libscrypt_SHA256_Final(digest, &ctx->octx);
}

static void libscrypt_PBKDF2_SHA256(const uint8_t * passwd, size_t passwdlen, const uint8_t * salt,
    size_t saltlen, uint64_t c, uint8_t * buf, size_t dkLen) {
    HMAC_SHA256_CTX PShctx, hctx;
    size_t i;
    uint8_t ivec[4];
    uint8_t U[32];
    uint8_t T[32];
    uint64_t j;
    int k;
    size_t clen;

    /* Compute HMAC state after processing P and S. */
    libscrypt_HMAC_SHA256_Init(&PShctx, passwd, passwdlen);
    libscrypt_HMAC_SHA256_Update(&PShctx, salt, saltlen);

    /* Iterate through the blocks. */
    for (i = 0; i * 32 < dkLen; i++) {
        /* Generate INT(i + 1). */
        be32enc(ivec, (uint32_t)(i + 1));

        /* Compute U_1 = PRF(P, S || INT(i)). */
        memcpy(&hctx, &PShctx, sizeof(HMAC_SHA256_CTX));
        libscrypt_HMAC_SHA256_Update(&hctx, ivec, 4);
        libscrypt_HMAC_SHA256_Final(U, &hctx);

        /* T_i = U_1 ... */
        memcpy(T, U, 32);

        for (j = 2; j <= c; j++) {
            /* Compute U_j. */
            libscrypt_HMAC_SHA256_Init(&hctx, passwd, passwdlen);
            libscrypt_HMAC_SHA256_Update(&hctx, U, 32);
            libscrypt_HMAC_SHA256_Final(U, &hctx);

            /* ... xor U_j ... */
            for (k = 0; k < 32; k++)
                T[k] ^= U[k];
        }

        /* Copy as many bytes as necessary into buf. */
        clen = dkLen - i * 32;
        if (clen > 32)
            clen = 32;
        memcpy(&buf[i * 32], T, clen);
    }
}

static int libscrypt_scrypt(const uint8_t * passwd, size_t passwdlen, const uint8_t * salt, size_t saltlen, uint64_t N, uint32_t r, uint32_t p, uint8_t * buf, size_t buflen) {
    void * B0, * V0, * XY0;
    uint8_t * B;
    uint32_t * V;
    uint32_t * XY;
    uint32_t i;

    /* Sanity-check parameters. */
#if SIZE_MAX > UINT32_MAX
    if (buflen > (((uint64_t)(1) << 32) - 1) * 32) {
        errno = EFBIG;
        goto err0;
    }
#endif
    if ((uint64_t)(r) * (uint64_t)(p) >= (1 << 30)) {
        errno = EFBIG;
        goto err0;
    }
    if (r == 0 || p == 0) {
        errno = EINVAL;
        goto err0;
    }
    if (((N & (N - 1)) != 0) || (N < 2)) {
        errno = EINVAL;
        goto err0;
    }
    if ((r > SIZE_MAX / 128 / p) ||
#if SIZE_MAX / 256 <= UINT32_MAX
        (r > SIZE_MAX / 256) ||
#endif
        (N > SIZE_MAX / 128 / r)) {
        errno = ENOMEM;
        goto err0;
    }

    /* Allocate memory. */
#ifdef HAVE_POSIX_MEMALIGN
    if ((errno = posix_memalign(&B0, 64, 128 * r * p)) != 0)
        goto err0;
    B = (uint8_t *)(B0);
    if ((errno = posix_memalign(&XY0, 64, 256 * r + 64)) != 0)
        goto err1;
    XY = (uint32_t *)(XY0);
#ifndef MAP_ANON
    if ((errno = posix_memalign(&V0, 64, 128 * r * N)) != 0)
        goto err2;
    V = (uint32_t *)(V0);
#endif
#else
    if ((B0 = malloc(128 * r * p + 63)) == NULL)
        goto err0;
    B = (uint8_t *)(((uintptr_t)(B0) + 63) & ~ (uintptr_t)(63));
    if ((XY0 = malloc(256 * r + 64 + 63)) == NULL)
        goto err1;
    XY = (uint32_t *)(((uintptr_t)(XY0) + 63) & ~ (uintptr_t)(63));
#ifndef MAP_ANON
    if ((V0 = malloc(128 * r * N + 63)) == NULL)
        goto err2;
    V = (uint32_t *)(((uintptr_t)(V0) + 63) & ~ (uintptr_t)(63));
#endif
#endif
#ifdef MAP_ANON
    if ((V0 = mmap(NULL, 128 * r * N, PROT_READ | PROT_WRITE,
#ifdef MAP_NOCORE
        MAP_ANON | MAP_PRIVATE | MAP_NOCORE,
#else
        MAP_ANON | MAP_PRIVATE,
#endif
        -1, 0)) == MAP_FAILED)
        goto err2;
    V = (uint32_t *)(V0);
#endif

    /* 1: (B_0 ... B_{p-1}) <-- PBKDF2(P, S, 1, p * MFLen) */
    libscrypt_PBKDF2_SHA256(passwd, passwdlen, salt, saltlen, 1, B, p * 128 * r);

    /* 2: for i = 0 to p - 1 do */
    for (i = 0; i < p; i++) {
        /* 3: B_i <-- MF(B_i, N) */
        smix(&B[i * 128 * r], r, N, V, XY);
    }

    /* 5: DK <-- PBKDF2(P, B, 1, dkLen) */
    libscrypt_PBKDF2_SHA256(passwd, passwdlen, B, p * 128 * r, 1, buf, buflen);

    /* Free memory. */
#ifdef MAP_ANON
    if (munmap(V0, 128 * r * N))
        goto err2;
#else
    free(V0);
#endif
    free(XY0);
    free(B0);

    /* Success! */
    return (0);

err2:
    free(XY0);
err1:
    free(B0);
err0:
    /* Failure! */
    return (-1);
}

#define PW_N                32768LU
#define PW_r                8U
#define PW_p                2U
#define PW_otp_window       5 * 60 /* s */

static int buf_resize(const void **buf, size_t *buf_sz, const size_t delta_sz) {
    if (!buf)
        return 0;
    
    void *new_buf = realloc((void*)*buf, (buf_sz ? *buf_sz : 0) + delta_sz);
    if (!new_buf)
        return 0;
    
    *buf = new_buf;
    if (buf_sz)
        *buf_sz += delta_sz;
    
    return 1;
}

static void buf_zero(void *buf, size_t buf_sz) {
    uint8_t *b = buf;
    for (; buf_sz > 0; --buf_sz)
        *b++ = 0;
}

static int buf_del(void **buf, const size_t buf_sz) {
    if (!buf || !*buf)
        return 0;
    
    buf_zero(*buf, buf_sz);
    free(*buf);
    *buf = NULL;
    
    return 1;
}

static int buf_push(unsigned char **buf, size_t *buf_sz, const void *data, const size_t data_sz) {
    if (!buf || !buf_sz || !data || !data_sz)
        return 0;
    
    if (!buf_resize((void*)buf, buf_sz, data_sz)) {
        buf_del((void*)buf, *buf_sz);
        return 0;
    }
    
    unsigned char *buf_offset = *buf + *buf_sz - data_sz;
    memcpy(buf_offset, data, data_sz);
    return 1;
}

#define buf_push_str(A, B, C) buf_push((A), (B), (C), strlen((C)))

static int buf_push_int(uint8_t **buf, size_t *buf_sz, const uint32_t data) {
    uint8_t tmp[4] = {
        (unsigned char)((data >> 24) & UINT8_MAX),
        (unsigned char)((data >> 16) & UINT8_MAX),
        (unsigned char)((data >> 8L) & UINT8_MAX),
        (unsigned char)((data >> 0L) & UINT8_MAX)
    };
    return buf_push(buf, buf_sz, &tmp, sizeof(tmp));
}

#define RETURN_TEMPLATE(N, ...) return (const char*[(N)]){ __VA_ARGS__ }[seed % (N)]

static const char* pw_template(const SpectreTemplate type, unsigned char seed) {
    switch (type) {
        default:
        case SpectreMaximum:
            RETURN_TEMPLATE(2, "anoxxxxxxxxxxxxxxxxx", "axxxxxxxxxxxxxxxxxno");
        case SpectreLong:
            RETURN_TEMPLATE(21, "CvcvnoCvcvCvcv", "CvcvCvcvnoCvcv", "CvcvCvcvCvcvno",
                            "CvccnoCvcvCvcv", "CvccCvcvnoCvcv", "CvccCvcvCvcvno",
                            "CvcvnoCvccCvcv", "CvcvCvccnoCvcv", "CvcvCvccCvcvno",
                            "CvcvnoCvcvCvcc", "CvcvCvcvnoCvcc", "CvcvCvcvCvccno",
                            "CvccnoCvccCvcv", "CvccCvccnoCvcv", "CvccCvccCvcvno",
                            "CvcvnoCvccCvcc", "CvcvCvccnoCvcc", "CvcvCvccCvccno",
                            "CvccnoCvcvCvcc", "CvccCvcvnoCvcc", "CvccCvcvCvccno");
        case SpectreMedium:
            RETURN_TEMPLATE(2, "CvcnoCvc", "CvcCvcno");
        case SpectreShort:
            return "Cvcn";
        case SpectreBasic:
            RETURN_TEMPLATE(3, "aaanaaan", "aannaaan", "aaannaaa");
        case SpectrePin:
            return "nnnn";
        case SpectreName:
            return "cvccvcvcv";
        case SpectrePhrase:
            RETURN_TEMPLATE(3, "cvcc cvc cvccvcv cvc", "cvc cvccvcvcv cvcv", "cv cvccv cvc cvcvccv");
    }
}

static const char *pw_template_chars(const char c) {
    switch (c) {
        case 'V':
            return "AEIOU";
        case 'C':
            return "BCDFGHJKLMNPQRSTVWXYZ";
        case 'v':
            return "aeiou";
        case 'c':
            return "bcdfghjklmnpqrstvwxyz";
        case 'A':
            return "AEIOUBCDFGHJKLMNPQRSTVWXYZ";
        case 'a':
            return "AEIOUaeiouBCDFGHJKLMNPQRSTVWXYZbcdfghjklmnpqrstvwxyz";
        case 'n':
            return "0123456789";
        case 'o':
            return "@&%?,=[]_:-+*$#!'^~;()/.";
        case 'x':
            return "AEIOUaeiouBCDFGHJKLMNPQRSTVWXYZbcdfghjklmnpqrstvwxyz0123456789!@#$%^&*()";
        case ' ':
            return " ";
        default:
            return NULL;
    }
}

static const char pw_template_char(const char c, const unsigned char seed) {
    const char *ch = pw_template_chars(c);
    return ch ? ch[seed % strlen(ch)] : '\0';
}

const char *SpectreGenerate(const char *name, const char *pass, const char *site, const int site_counter, const char *key_scope, SpectreTemplate type) {
    size_t pw_salt_sz = 0;
    unsigned char *pw_salt = NULL;
    buf_push_str(&pw_salt, &pw_salt_sz, key_scope);
    buf_push_int(&pw_salt, &pw_salt_sz, (unsigned int)strlen(name));
    buf_push_str(&pw_salt, &pw_salt_sz, name);
    
    unsigned char pass_key[64];
    libscrypt_scrypt((unsigned char*)pass, strlen(pass), pw_salt, pw_salt_sz, PW_N, PW_r, PW_p, pass_key, 64);
    buf_del((void*)&pw_salt, pw_salt_sz);
    
    size_t site_salt_sz = 0;
    unsigned char *site_salt = NULL;
    buf_push_str(&site_salt, &site_salt_sz, key_scope);
    buf_push_int(&site_salt, &site_salt_sz, (unsigned int)strlen(site));
    buf_push_str(&site_salt, &site_salt_sz, site);
    buf_push_int(&site_salt, &site_salt_sz, site_counter);
    
    unsigned char site_key[32];
    HMAC_SHA256_CTX ctx;
    libscrypt_HMAC_SHA256_Init(&ctx, pass_key, 64);
    libscrypt_HMAC_SHA256_Update(&ctx, site_salt, site_salt_sz);
    libscrypt_HMAC_SHA256_Final(site_key, &ctx);
    
    const char* template = pw_template(type, site_key[0]);
    size_t template_sz = strlen(template);
    char* const site_pass = malloc(sizeof(char) * template_sz + 1);
    for (int i = 0; i < template_sz; ++i)
      site_pass[i] = pw_template_char(template[i], site_key[i + 1]);
    site_pass[template_sz] = '\0';
    return site_pass;
}

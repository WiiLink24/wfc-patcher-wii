#include <wwfcError.h>
#include <wwfcPayloadPublicKey.hpp>
#include <wwfcTypes.h>

#define RESTGPR_MINIMUM 22

#define AT_EXPANDED(_ADDRESS) asm(#_ADDRESS)
#define AT(_ADDRESS) AT_EXPANDED(_ADDRESS)

#define SHA256_DIGEST_SIZE 32

using s8 = wwfc_int8_t;
using s16 = wwfc_int16_t;
using s32 = wwfc_int32_t;
using s64 = wwfc_int64_t;
using u8 = wwfc_uint8_t;
using u16 = wwfc_uint16_t;
using u32 = wwfc_uint32_t;
using u64 = wwfc_uint64_t;

#define CONFIG_RSA_KEY_SIZE 2048
#define RSANUMBYTES ((CONFIG_RSA_KEY_SIZE) / 8)
#define RSANUMWORDS (RSANUMBYTES / sizeof(u32))

#define WWFC_ADJUST_OFFSET(_TYPE, _PAYLOAD, _OFFSET)                           \
    ((_TYPE) ((wwfc_uint8_t*) (_PAYLOAD) + (wwfc_uint32_t) (_OFFSET)))

#define CONCAT(a, b) a##b

#define BACKUP_REG(_reg)                                                       \
    u32 CONCAT(v, _reg);                                                       \
    asm volatile("stw%U0 %1, %0" : "=m"(CONCAT(v, _reg)) : "r"(_reg))

#define RESTORE_REG(_reg)                                                      \
    asm volatile("lwz%U0 %0, %1" : "=r"(_reg) : "m"(CONCAT(v, _reg)))

/**
 * RSA public key definition
 */
struct RSAPublicKey {
    u32 n0inv; // -1 / n[0] mod 2^32
    u32 n[RSANUMWORDS]; // modulus as little endian array
    u32 rr[RSANUMWORDS]; // R^2 as little endian array
};

#define SHA256_BLOCK_SIZE 64

/* SHA256 context */
struct SHA256Context {
    u32 h[8];
    u32 tot_len;
    u32 len;
    u8 block[2 * SHA256_BLOCK_SIZE];
    u8 buf[SHA256_DIGEST_SIZE]; /* Used to store the final digest. */
};

register u32 g_r13 asm("r13");
register void* __restrict g_context asm("r14");

namespace wwfc
{
class Stage1;
}

register wwfc::Stage1* __restrict self asm("r15");

#define g_key reinterpret_cast<const RSAPublicKey* __restrict>(g_context)
#define g_sha256Context reinterpret_cast<SHA256Context* __restrict>(g_context)

extern "C" {

int memcmp(const void* s1, const void* s2, u32 n)
{
    [[assume(n > 0)]];

    const u8* su1 = (const u8*) s1;
    const u8* su2 = (const u8*) s2;

    u32 i = 0;
    for (; i < n && su1[i] == su2[i]; i++) {
    }

    return i < n ? su1[i] - su2[i] : 0;
}

void memset(void* ptr, int value, u32 n)
{
    [[assume(n > 0)]];

    u8* ptru = (u8*) ptr;

    for (u32 i = 0; i < n; i++) {
        ptru[i] = char(value);
    }
}

void memcpy(void* __restrict dst, const void* __restrict src, u32 n)
{
    const u8* srcu = (const u8*) src;
    u8* dstu = (u8*) dst;

    for (u32 i = 0; i < n; i++) {
        dstu[i] = srcu[i];
    }
}
}

namespace wwfc
{

struct Stage1 {
    consteval Stage1() {};

    struct MEMAllocator;

    struct MEMAllocatorFunc {
        void* (*alloc)(MEMAllocator* allocator, u32 size);
        void* (*free)(MEMAllocator* allocator, void* block);
    };

    struct MEMAllocator {
        const MEMAllocatorFunc* func;
        u8 _4[0x10 - 0x4];
    };

    struct Stage1Param {
#if !STAGE1_SBCM
        void* block;

        void* (*const NHTTPCreateRequest)(
            const char* url, int param_2, void* buffer, u32 length,
            void* callback, void* userdata
        );
        s32 (*const NHTTPSendRequestAsync)(void* request);
        s32 (*const NHTTPDestroyResponse)(void* response);

        MEMAllocator* const* allocator;
        s32* const dwcError;

        const char title[9];
#else
        static void OSYieldThread( //
            void
        ) asm("OSYieldThread");

        static s32 NHTTPStartup( //
            void* alloc, void* free, u32 param_3
        ) asm("NHTTPStartup");

        static void* NHTTPCreateRequest( //
            const char* url, int param_2, void* buffer, u32 length,
            void* callback, void* userdata
        ) asm("NHTTPCreateRequest");

        static s32 NHTTPSendRequestAsync( //
            void* request
        ) asm("NHTTPSendRequestAsync");

        static s32 NHTTPDestroyResponse( //
            void* response
        ) asm("NHTTPDestroyResponse");

        static s32 DWCi_HandleGPError( //
            s32 error
        ) asm("DWCi_HandleGPError");

        static s32 DWCi_SetError( //
            s32 errorClass, s32 errorCode
        ) asm("DWCi_SetError");

        static MEMAllocator* const allocator AT(ADDRESS_HBM_ALLOCATOR);

        static constexpr char title[10] = PAYLOAD;
#endif
    };

#define BASE_URL_PART1 "http://naswii." WWFC_DOMAIN
#define BASE_URL_PART2 "/payload?g="
#define BASE_URL (BASE_URL_PART1 BASE_URL_PART2)
    const char m_hexToStr[16] = {'0', '1', '2', '3', '4', '5', '6', '7',
                                 '8', '9', 'a', 'b', 'c', 'd', 'e', 'f'};

    // Reuse this memory area
    union {
        const char m_baseUrl[0x2A] = BASE_URL;

        struct {
            u8 m_saltHash[SHA256_DIGEST_SIZE];
            Stage1Param* m_param;
        };
    };

#if STAGE1_SBCM
    void* m_block = nullptr;

    s32 m_error = WL_ERROR_PAYLOAD_STAGE1_WAITING;
#endif

    u32 m_sha256_h0[8] = {0x6a09e667, 0xbb67ae85, 0x3c6ef372, 0xa54ff53a,
                          0x510e527f, 0x9b05688c, 0x1f83d9ab, 0x5be0cd19};

    /*
     * PKCS#1 padding (from the RSA PKCS#1 v2.1 standard)
     *
     * The DER-encoded padding is defined as follows :
     * 0x00 || 0x01 || PS || 0x00 || T
     *
     * T: DER Encoded DigestInfo value which depends on the hash function
     * used, for SHA-256: (0x)30 31 30 0d 06 09 60 86 48 01 65 03 04 02 01
     * 05 00 04 20 || H.
     *
     * Length(T) = 51 octets for SHA-256
     *
     * PS: octet string consisting of {Length(RSA Key) - Length(T) - 3} 0xFF
     */
    const u8 m_sha256Tail[20] = {0x00, 0x30, 0x31, 0x30, 0x0D, 0x06, 0x09,
                                 0x60, 0x86, 0x48, 0x01, 0x65, 0x03, 0x04,
                                 0x02, 0x01, 0x05, 0x00, 0x04, 0x20};

    u32 m_sha256_k[64] = {
        0x428a2f98, 0x71374491, 0xb5c0fbcf, 0xe9b5dba5, 0x3956c25b, 0x59f111f1,
        0x923f82a4, 0xab1c5ed5, 0xd807aa98, 0x12835b01, 0x243185be, 0x550c7dc3,
        0x72be5d74, 0x80deb1fe, 0x9bdc06a7, 0xc19bf174, 0xe49b69c1, 0xefbe4786,
        0x0fc19dc6, 0x240ca1cc, 0x2de92c6f, 0x4a7484aa, 0x5cb0a9dc, 0x76f988da,
        0x983e5152, 0xa831c66d, 0xb00327c8, 0xbf597fc7, 0xc6e00bf3, 0xd5a79147,
        0x06ca6351, 0x14292967, 0x27b70a85, 0x2e1b2138, 0x4d2c6dfc, 0x53380d13,
        0x650a7354, 0x766a0abb, 0x81c2c92e, 0x92722c85, 0xa2bfe8a1, 0xa81a664b,
        0xc24b8b70, 0xc76c51a3, 0xd192e819, 0xd6990624, 0xf40e3585, 0x106aa070,
        0x19a4c116, 0x1e376c08, 0x2748774c, 0x34b0bcb5, 0x391c0cb3, 0x4ed8aa4a,
        0x5b9cca4f, 0x682e6ff3, 0x748f82ee, 0x78a5636f, 0x84c87814, 0x8cc70208,
        0x90befffa, 0xa4506ceb, 0xbef9a3f7, 0xc67178f2
    };

    const RSAPublicKey m_publicKey =
        __builtin_bit_cast(RSAPublicKey, wwfc::PayloadPublicKey);
};

// FIPS 180-2 SHA-256 implementation
// Last update: 02/02/2007
// Issue date:  04/30/2005
//
// Copyright (C) 2005, 2007 Olivier Gay <olivier.gay@a3.epfl.ch>
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions
// are met:
// 1. Redistributions of source code must retain the above copyright
//    notice, this list of conditions and the following disclaimer.
// 2. Redistributions in binary form must reproduce the above copyright
//    notice, this list of conditions and the following disclaimer in the
//    documentation and/or other materials provided with the distribution.
// 3. Neither the name of the project nor the names of its contributors
//    may be used to endorse or promote products derived from this software
//    without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE PROJECT AND CONTRIBUTORS ``AS IS'' AND
// ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
// PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE PROJECT OR CONTRIBUTORS BE
// LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
// CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
// SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
// INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
// CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
// ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
// THE POSSIBILITY OF SUCH DAMAGE.

#define SHFR(x, n) (x >> n)
#define ROTR(x, n) ((x >> n) | (x << ((sizeof(x) << 3) - n)))
#define ROTL(x, n) ((x << n) | (x >> ((sizeof(x) << 3) - n)))
#define CH(x, y, z) ((x & y) ^ (~x & z))
#define MAJ(x, y, z) ((x & y) ^ (x & z) ^ (y & z))

#define SHA256_F1(x) (ROTR(x, 2) ^ ROTR(x, 13) ^ ROTR(x, 22))
#define SHA256_F2(x) (ROTR(x, 6) ^ ROTR(x, 11) ^ ROTR(x, 25))
#define SHA256_F3(x) (ROTR(x, 7) ^ ROTR(x, 18) ^ SHFR(x, 3))
#define SHA256_F4(x) (ROTR(x, 17) ^ ROTR(x, 19) ^ SHFR(x, 10))

#ifdef LITTLE_ENDIAN

#  define UNPACK32(x, str)                                                     \
      {                                                                        \
          *((str) + 3) = (u8) ((x));                                           \
          *((str) + 2) = (u8) ((x) >> 8);                                      \
          *((str) + 1) = (u8) ((x) >> 16);                                     \
          *((str) + 0) = (u8) ((x) >> 24);                                     \
      }

#  define PACK32(str, x)                                                       \
      {                                                                        \
          *(x) = ((u32) * ((str) + 3)) | ((u32) * ((str) + 2) << 8) |          \
                 ((u32) * ((str) + 1) << 16) | ((u32) * ((str) + 0) << 24);    \
      }

#else

#  define UNPACK32(x, str)                                                     \
      {                                                                        \
          *((u32*) (str)) = (x);                                               \
      }

#  define PACK32(str, x)                                                       \
      {                                                                        \
          *(x) = *((u32*) (str));                                              \
      }

#endif

/* Macros used for loops unrolling */

#define SHA256_SCR(i)                                                          \
    {                                                                          \
        w[i] =                                                                 \
            SHA256_F4(w[i - 2]) + w[i - 7] + SHA256_F3(w[i - 15]) + w[i - 16]; \
    }

#define SHA256_EXP(a, b, c, d, e, f, g, h, j)                                  \
    {                                                                          \
        t1 = wv[h] + SHA256_F2(wv[e]) + CH(wv[e], wv[f], wv[g]) +              \
             self->m_sha256_k[j] + w[j];                                       \
        t2 = SHA256_F1(wv[a]) + MAJ(wv[a], wv[b], wv[c]);                      \
        wv[d] += t1;                                                           \
        wv[h] = t1 + t2;                                                       \
    }

[[gnu::noinline]]
void SHA256Init()
{
    int i;

    for (i = 0; i < 8; i++)
        g_sha256Context->h[i] = self->m_sha256_h0[i];

    g_sha256Context->len = 0;
    g_sha256Context->tot_len = 0;
}

void SHA256Transform(const u8* message, u32 block_nb)
{
    /* Note: this function requires a considerable amount of stack */
    u32 w[64];
    u32 wv[8];
    u32 t1, t2;
    const u8* sub_block;
    int i, j;

    for (i = 0; i < (int) block_nb; i++) {
        sub_block = message + (i << 6);

        for (j = 0; j < 16; j++)
            PACK32(&sub_block[j << 2], &w[j]);

        for (j = 16; j < 64; j++)
            SHA256_SCR(j);

        for (j = 0; j < 8; j++)
            wv[j] = g_sha256Context->h[j];

        for (j = 0; j < 64; j++) {
            t1 = wv[7] + SHA256_F2(wv[4]) + CH(wv[4], wv[5], wv[6]) +
                 self->m_sha256_k[j] + w[j];
            t2 = SHA256_F1(wv[0]) + MAJ(wv[0], wv[1], wv[2]);
            wv[7] = wv[6];
            wv[6] = wv[5];
            wv[5] = wv[4];
            wv[4] = wv[3] + t1;
            wv[3] = wv[2];
            wv[2] = wv[1];
            wv[1] = wv[0];
            wv[0] = t1 + t2;
        }

        for (j = 0; j < 8; j++)
            g_sha256Context->h[j] += wv[j];
    }
}

void SHA256Update(const void* data, u32 len)
{
    u32 block_nb;
    u32 new_len, rem_len, tmp_len;
    const u8* shifted_data;

    tmp_len = SHA256_BLOCK_SIZE - g_sha256Context->len;
    rem_len = len < tmp_len ? len : tmp_len;

    memcpy(&g_sha256Context->block[g_sha256Context->len], data, rem_len);

    if (g_sha256Context->len + len < SHA256_BLOCK_SIZE) {
        g_sha256Context->len += len;
        return;
    }

    new_len = len - rem_len;
    block_nb = new_len / SHA256_BLOCK_SIZE;

    shifted_data = (u8*) data + rem_len;

    SHA256Transform(g_sha256Context->block, 1);
    SHA256Transform(shifted_data, block_nb);

    rem_len = new_len % SHA256_BLOCK_SIZE;

    memcpy(g_sha256Context->block, &shifted_data[block_nb << 6], rem_len);
    g_sha256Context->len = rem_len;
    g_sha256Context->tot_len += (block_nb + 1) << 6;
}

void SHA256Final2()
{
    u32 block_nb;
    u32 pm_len;
    u32 len_b;
    int i;

    block_nb =
        (1 + ((SHA256_BLOCK_SIZE - 9) <
              (g_sha256Context->len % SHA256_BLOCK_SIZE)));

    len_b = (g_sha256Context->tot_len + g_sha256Context->len) << 3;
    pm_len = block_nb << 6;

    memset(
        g_sha256Context->block + g_sha256Context->len, 0,
        pm_len - g_sha256Context->len
    );
    g_sha256Context->block[g_sha256Context->len] = 0x80;
    UNPACK32(len_b, g_sha256Context->block + pm_len - 4);

    SHA256Transform(g_sha256Context->block, block_nb);

    // for (i = 0; i < 8; i++)
    //     UNPACK32(g_sha256Context->h[i], &g_sha256Context->buf[i << 2]);

    // return g_sha256Context->buf;
}

[[gnu::always_inline]]
inline u8* SHA256Final()
{
    SHA256Final2();
    return (u8*) g_sha256Context->h;
}

// Copyright (C) 2010 The Chromium OS Authors. All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are
// met:
//
//    * Redistributions of source code must retain the above copyright
// notice, this list of conditions and the following disclaimer.
//    * Redistributions in binary form must reproduce the above
// copyright notice, this list of conditions and the following disclaimer
// in the documentation and/or other materials provided with the
// distribution.
//    * Neither the name of Google Inc. nor the names of its
// contributors may be used to endorse or promote products derived from
// this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
// OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
// SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
// LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
// THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

/**
 * a[] -= mod
 */
static void SubMod(u32* a)
{
    s64 A = 0;
    u32 i;
    for (i = 0; i < RSANUMWORDS; ++i) {
        A += (u64) a[i] - g_key->n[i];
        a[i] = (u32) A;
        A >>= 32;
    }
}

/**
 * Return a[] >= mod
 */
static int GeMod(const u32* a)
{
    u32 i;
    for (i = RSANUMWORDS; i;) {
        --i;
        if (a[i] < g_key->n[i])
            return 0;
        if (a[i] > g_key->n[i])
            return 1;
    }
    return 1; // equal
}

/**
 * Montgomery c[] += a * b[] / R % mod
 */
static void MontMulAdd(u32* c, const u32 a, const u32* b)
{

    u64 A = (u64) a * b[0] + c[0];
    u32 d0 = (u32) A * g_key->n0inv;
    u64 B = (u64) d0 * g_key->n[0] + (u32) A;
    u32 i;

    for (i = 1; i < RSANUMWORDS; ++i) {
        A = (A >> 32) + (u64) a * b[i] + c[i];
        B = (B >> 32) + (u64) d0 * g_key->n[i] + (u32) A;
        c[i - 1] = (u32) B;
    }

    A = (A >> 32) + (B >> 32);

    c[i - 1] = (u32) A;

    if (A >> 32) {
        SubMod(c);
    }
}

/**
 * Montgomery c[] = a[] * b[] / R % mod
 */
static void MontMul(u32* c, const u32* a, const u32* b)
{
    for (u32 i = 0; i < RSANUMWORDS; ++i) {
        c[i] = 0;
    }

    for (u32 i = 0; i < RSANUMWORDS; ++i) {
        MontMulAdd(c, a[i], b);
    }
}

/**
 * In-place public exponentiation.
 *
 * @param inout		Input and output big-endian byte array
 */
static void ModPow(u32* inout)
{
    u32 a[RSANUMWORDS];
    u32 aaR[RSANUMWORDS];
    u32 aaaR[RSANUMWORDS];
    u32* aaa = aaaR; // Reuse location

    // Convert from big endian byte array to little endian word array
    for (u32 i = 0; i < RSANUMWORDS; ++i) {
        u32 v = inout[RSANUMWORDS - 1 - i];
#ifdef LITTLE_ENDIAN
        // TODO: Check this, I'm really tired so it might be wrong
        v = (v >> 24) | ((v >> 8) & 0xFF00) | ((v << 8) & 0xFF0000) | (v << 24);
#endif
        a[i] = v;
    }

    MontMul(aaR, a, g_key->rr); // aaR = a * RR / R mod M
    // Exponent 65537
    for (u32 i = 0; i < 16; i += 2) {
        MontMul(aaaR, aaR, aaR); // aaaR = aaR * aaR / R mod M
        MontMul(aaR, aaaR, aaaR); // aaR = aaaR * aaaR / R mod M
    }
    MontMul(aaa, aaR, a); // aaa = aaR * a / R mod M

    // Make sure aaa < mod; aaa is at most 1x mod too large
    if (GeMod(aaa)) {
        SubMod(aaa);
    }

    // Convert to big endian byte array
    for (u32 i = 0; i < RSANUMWORDS; ++i) {
        u32 v = aaa[RSANUMWORDS - 1 - i];
#ifdef LITTLE_ENDIAN
        // TODO: Check this, I'm really tired so it might be wrong
        v = (v >> 24) | ((v >> 8) & 0xFF00) | ((v << 8) & 0xFF0000) | (v << 24);
#endif
        inout[i] = v;
    }
}

#define PKCS_PAD_SIZE (RSANUMBYTES - SHA256_DIGEST_SIZE)

/**
 * Verify a SHA256WithRSA PKCS#1 v1.5 signature against an expected SHA-256
 * hash.
 *
 * @param key           RSA public key
 * @param signature     RSA signature
 * @param sha           SHA-256 digest of the content to verify
 * @return True on success.
 */
bool RSAVerify(const RSAPublicKey* key, u8* signature, const u8* sha)
{
    g_context = const_cast<RSAPublicKey*>(key);

    ModPow((u32*) signature); // In-place exponentiation

    u32 result = 0;
    int i = 0;

    // Check PKCS#1 padding bytes
    // First 2 bytes are always 0x00 0x01
    result |= *reinterpret_cast<u16*>(signature) ^ 0x0001;
    i += 2;

    // Then 0xFF bytes until the tail
    for (u32 j = 0; j < PKCS_PAD_SIZE - sizeof(self->m_sha256Tail) - 2; j++) {
        result |= u32(signature[i++]) - 0xFF;
    }

    // Check the tail
    result |=
        memcmp(signature + i, self->m_sha256Tail, sizeof(self->m_sha256Tail));
    result |= memcmp(signature + PKCS_PAD_SIZE, sha, SHA256_DIGEST_SIZE);

    return result == 0;
}

// Used occasionally to make significantly smaller code (for some reason)
#define ASM_LOAD_HI(_TYPE, _NAME, _ADDR)                                       \
    _TYPE _NAME;                                                               \
    asm volatile("lis %0, " #_ADDR "\n" : "=r"(_NAME))

static constexpr u32 PAYLOAD_BLOCK_SIZE = 0x20000;

s32 HandleResponse(void* block)
{
    wwfc_payload* __restrict payload = reinterpret_cast<wwfc_payload*>(block);

    if (*reinterpret_cast<u32*>(payload) != 0x57574643 /* WWFC */) {
        return WL_ERROR_PAYLOAD_STAGE1_HEADER_CHECK;
    }

    if (payload->header.total_size < sizeof(wwfc_payload) ||
        payload->header.total_size > PAYLOAD_BLOCK_SIZE) {
        return WL_ERROR_PAYLOAD_STAGE1_LENGTH_ERROR;
    }

    memcpy(payload->salt, self->m_saltHash, SHA256_DIGEST_SIZE);

    SHA256Context ctx;

    BACKUP_REG(g_context);
    g_context = &ctx;
    SHA256Init();
    SHA256Update(
        reinterpret_cast<u8*>(payload) + sizeof(wwfc_payload_header),
        payload->header.total_size - sizeof(wwfc_payload_header)
    );
    u8* hash = SHA256Final();

    if (!RSAVerify(&self->m_publicKey, payload->header.signature, hash)) {
        return WL_ERROR_PAYLOAD_STAGE1_SIGNATURE_INVALID;
    }

    RESTORE_REG(g_context);

    auto entryFunction = WWFC_ADJUST_OFFSET(
        wwfc_payload_entry_t, payload, payload->info.entry_point
    );

    // Flush data cache and invalidate instruction cache
    for (u32 i = 0; i < PAYLOAD_BLOCK_SIZE; i += 0x20) {
        asm volatile("dcbf %0, %1\n"
                     "icbi %0, %1"
                     :
                     : "b"(i), "r"(payload));
    }
    // Sync (Address Broadcast)
    asm volatile("sc\n" : : : "r9", "r10");

    return entryFunction(payload);
}

#if STAGE1_SBCM
#  define param (reinterpret_cast<Stage1::Stage1Param*>(0))

static void* Alloc(u32 size)
{
    return param->allocator->func->alloc(param->allocator, size);
}

static void Free(void* block)
{
    param->allocator->func->free(param->allocator, block);
}

// HTTPCallback for SBCM
s32 HTTPCallback(s32 result, void* response, void* userData)
{
    BACKUP_REG(self);
    self = reinterpret_cast<Stage1*>(userData);

    param->NHTTPDestroyResponse(response);

    if (result != 0) {
        result = WL_ERROR_PAYLOAD_STAGE1_RESPONSE;
    } else {
        result = HandleResponse(self->m_block);
    }
    self->m_error = result;
    RESTORE_REG(self);
    return 0;
}
#else
// HTTPCallback for regular request
s32 HTTPCallback(s32 result, void* response, void* userData)
{
    BACKUP_REG(self);
    self = reinterpret_cast<Stage1*>(userData);

    const Stage1::Stage1Param* __restrict param = self->m_param;
    param->NHTTPDestroyResponse(response);

    if (result != 0) {
        result = WL_ERROR_PAYLOAD_STAGE1_RESPONSE;
    }

    if (result != 0 ||
        (result = HandleResponse(param->block)) != WL_ERROR_PAYLOAD_OK) {
        *param->dwcError = result;
    } else {
        // Success! This error code will retry auth.
        *param->dwcError = -1;
    }

    RESTORE_REG(self);
    return 0;
}
#endif

inline s32 Download(
#if !STAGE1_SBCM
    Stage1::Stage1Param* param, s32* authRequest,
#endif
    void* httpCallback
)
{
#if STAGE1_SBCM
    if (param->NHTTPStartup(
            reinterpret_cast<void*>(Alloc), reinterpret_cast<void*>(Free), 0x11
        ) != 0) {
        return WL_ERROR_PAYLOAD_STAGE1_MAKE_REQUEST;
    }
#endif

    char url[128];
    memcpy(url, self->m_baseUrl, sizeof(self->m_baseUrl));

#if !STAGE1_SBCM
    void* block = param->block;
    if (block == nullptr) {
        if (param->allocator == nullptr) {
            return WL_ERROR_PAYLOAD_STAGE1_ALLOC;
        }

        auto allocator = *param->allocator;
        if (allocator == nullptr) {
            return WL_ERROR_PAYLOAD_STAGE1_ALLOC;
        }

        auto func = allocator->func;
        if (func == nullptr) {
            return WL_ERROR_PAYLOAD_STAGE1_ALLOC;
        }

        auto allocFunc = func->alloc;
        if (allocFunc == nullptr) {
            return WL_ERROR_PAYLOAD_STAGE1_ALLOC;
        }

        block = allocFunc(allocator, PAYLOAD_BLOCK_SIZE + 32);
    }
#else
    void* block = Alloc(PAYLOAD_BLOCK_SIZE + 32);
#endif

    if (block == nullptr) {
        return WL_ERROR_PAYLOAD_STAGE1_ALLOC;
    }

    // Align up block
    block = (void*) ((u32(block) + 31) & ~31);
    memset(block, 0, PAYLOAD_BLOCK_SIZE);
#if !STAGE1_SBCM
    param->block = block;
#else
    self->m_block = block;
#endif

    int cur = sizeof(BASE_URL) - 1;

    memcpy(url + cur, param->title, 9);
    cur += param->title[4] == 'D' ? 7 : 9;

    // Create random salt, relies on undefined behavior
    u8 salt[SHA256_DIGEST_SIZE];
    SHA256Context ctx;
    g_context = &ctx;
    {
        SHA256Init();
        void* r13;
        SHA256Update(reinterpret_cast<void*>(g_r13 - 0x8000), 0x10000);
        u32 seedData[16];
        u32 tbl, tbu, dec;
        asm volatile("mftbl %0; mftbu %1; mfdec %2\n"
                     : "=r"(tbl), "=r"(tbu), "=r"(dec));
        seedData[0] = tbl;
        seedData[1] = tbu;
        seedData[2] = dec;
        ASM_LOAD_HI(u16* __restrict, MEM_REG_BASE, 0xCC00);
        seedData[3] = MEM_REG_BASE[0x4034 / 2]; // MEM_CP_REQCOUNTL
        seedData[4] = MEM_REG_BASE[0x4038 / 2]; // MEM_TC_REQCOUNTL
        seedData[5] = MEM_REG_BASE[0x403C / 2]; // MEM_CPUR_REQCOUNTL
        seedData[6] = MEM_REG_BASE[0x4040 / 2]; // MEM_CPUW_REQCOUNTL
        seedData[7] = MEM_REG_BASE[0x4048 / 2]; // MEM_IO_REQCOUNTL
        seedData[8] = MEM_REG_BASE[0x404C / 2]; // MEM_VI_REQCOUNTL
        seedData[9] = u32(self);
        seedData[10] = u32();
        SHA256Update(((u8*) seedData) - 0x400, 0x2000);
        SHA256Update((void*) 0x80000000, 0x4000);
        SHA256Update((void*) 0x90000000, 0x1000);
        ASM_LOAD_HI(u32* __restrict, MEM1_BASE, 0x8000);
        SHA256Update(&MEM1_BASE[0x3130 / 4], 0x30000);
        memcpy(salt, SHA256Final(), SHA256_DIGEST_SIZE);
    }

    // Add the salt
    url[cur++] = '&';
    url[cur++] = 's';
    url[cur++] = '=';

    for (u32 i = 0; i < SHA256_DIGEST_SIZE; i++, cur += 2) {
        url[cur + 0] = self->m_hexToStr[salt[i] >> 4];
        url[cur + 1] = self->m_hexToStr[salt[i] & 0xF];
    }

    // Hash "payload?g=RMCPD00&s=7d8a..."
    SHA256Init();
    SHA256Update(&url[sizeof(BASE_URL_PART1)], cur - sizeof(BASE_URL_PART1));
    memcpy(self->m_saltHash, SHA256Final(), SHA256_DIGEST_SIZE);
    // Add first 4 bytes of salt hash as a kind of "proof" to the server the
    // request is valid
    url[cur++] = '&';
    url[cur++] = 'h';
    url[cur++] = '=';

    for (u32 i = 0; i < 4; i++, cur += 2) {
        url[cur + 0] = self->m_hexToStr[self->m_saltHash[i] >> 4];
        url[cur + 1] = self->m_hexToStr[self->m_saltHash[i] & 0xF];
    }

    url[cur] = '\0';

#if !STAGE1_SBCM
    self->m_param = param;

    void* request = param->NHTTPCreateRequest(
        url, 0, block, PAYLOAD_BLOCK_SIZE - 32, httpCallback, self
    );
    if (request == nullptr) {
        return WL_ERROR_PAYLOAD_STAGE1_MAKE_REQUEST;
    }

    *authRequest = param->NHTTPSendRequestAsync(request);

    return WL_ERROR_PAYLOAD_OK;
#else
    void* request = param->NHTTPCreateRequest(
        url, 0, block, PAYLOAD_BLOCK_SIZE - 32, httpCallback, self
    );
    if (request == nullptr) {
        return WL_ERROR_PAYLOAD_STAGE1_MAKE_REQUEST;
    }

    param->NHTTPSendRequestAsync(request);

    while (self->m_error == WL_ERROR_PAYLOAD_STAGE1_WAITING) {
        // TODO: Add request timeout
#  if ADDRESS_OSYieldThread
        param->OSYieldThread();
#  endif
    }

    return self->m_error;
#endif
}

static Stage1 s_stage1;

#if !STAGE1_SBCM
extern "C" [[gnu::section(".start")]] [[gnu::used]] void
wwfcStage1Entry(u8* stage1CodePtr, Stage1::Stage1Param* param, s32* authRequest)
{
    BACKUP_REG(g_context);
    BACKUP_REG(self);

    void* httpCallback;
    asm("addi %0, %1, %2"
        : "=r"(httpCallback)
        : "r"(stage1CodePtr), "i"(HTTPCallback));

    asm("addi %0, %1, %2"
        : "=r"(self)
        : "r"(stage1CodePtr), "i"(u32(&s_stage1)));

    s32 ret = Download(param, authRequest, httpCallback);
    if (ret != 0) {
        *param->dwcError = ret;
    }
    // Else don't do anything

    RESTORE_REG(g_context);
    RESTORE_REG(self);
}
#else
extern "C" [[gnu::used]] void wwfcStage1Entry()
{
    auto regBackup = g_context;
    auto selfBackup = self;

    self = &s_stage1;

    s32 ret = Download(reinterpret_cast<void*>(HTTPCallback));
    if (ret != WL_ERROR_PAYLOAD_OK) {
        param->DWCi_HandleGPError(3);
        param->DWCi_SetError(3, ret);
    }
    // Else don't do anything

    g_context = regBackup;
    self = selfBackup;
}
#endif

// Create restgpr labels

asm(
#if RESTGPR_MINIMUM <= 14
    "_restgpr_14_x: lwz %r14, -0x48(%r11)\n"
#endif
#if RESTGPR_MINIMUM <= 15
    "_restgpr_15_x: lwz %r15, -0x44(%r11)\n"
#endif
#if RESTGPR_MINIMUM <= 16
    "_restgpr_16_x: lwz %r16, -0x40(%r11)\n"
#endif
#if RESTGPR_MINIMUM <= 17
    "_restgpr_17_x: lwz %r17, -0x3C(%r11)\n"
#endif
#if RESTGPR_MINIMUM <= 18
    "_restgpr_18_x: lwz %r18, -0x38(%r11)\n"
#endif
#if RESTGPR_MINIMUM <= 19
    "_restgpr_19_x: lwz %r19, -0x34(%r11)\n"
#endif
#if RESTGPR_MINIMUM <= 20
    "_restgpr_20_x: lwz %r20, -0x30(%r11)\n"
#endif
#if RESTGPR_MINIMUM <= 21
    "_restgpr_21_x: lwz %r21, -0x2C(%r11)\n"
#endif
#if RESTGPR_MINIMUM <= 22
    "_restgpr_22_x: lwz %r22, -0x28(%r11)\n"
#endif
#if RESTGPR_MINIMUM <= 23
    "_restgpr_23_x: lwz %r23, -0x24(%r11)\n"
#endif
#if RESTGPR_MINIMUM <= 24
    "_restgpr_24_x: lwz %r24, -0x20(%r11)\n"
#endif
#if RESTGPR_MINIMUM <= 25
    "_restgpr_25_x: lwz %r25, -0x1C(%r11)\n"
#endif
#if RESTGPR_MINIMUM <= 26
    "_restgpr_26_x: lwz %r26, -0x18(%r11)\n"
#endif
#if RESTGPR_MINIMUM <= 27
    "_restgpr_27_x: lwz %r27, -0x14(%r11)\n"
#endif
#if RESTGPR_MINIMUM <= 28
    "_restgpr_28_x: lwz %r28, -0x10(%r11)\n"
#endif
#if RESTGPR_MINIMUM <= 29
    "_restgpr_29_x: lwz %r29, -0x0C(%r11)\n"
#endif
#if RESTGPR_MINIMUM <= 30
    "_restgpr_30_x: lwz %r30, -0x08(%r11)\n"
#endif
#if RESTGPR_MINIMUM <= 31
    "_restgpr_31_x: lwz %r31, -0x04(%r11)\n"
#endif
    "lwz %r0, 0x04(%r11)\n"
    "mtlr %r0\n"
    "mr %r1, %r11\n"
    "blr"
);

} // namespace wwfc

/*
 * Hardware-accelerated CRC-32 variants for Linux on z Systems
 *
 * Use the z/Architecture Vector Extension Facility to accelerate the
 * computing of bitreflected CRC-32 checksums.
 *
 * This CRC-32 implementation algorithm is bitreflected and processes
 * the least-significant bit first (Little-Endian).
 *
 * This code was originally written by Hendrik Brueckner
 * <brueckner@linux.vnet.ibm.com> for use in the Linux kernel and has been
 * relicensed under the zlib license.
 */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ Z_ONCE
#ਅਨ੍ਤਰ੍ਭਾਵ "../../zutil.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "crc32_vx_hooks.h"

#ਅਨ੍ਤਰ੍ਭਾਵ <stdint.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <stdio.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <vecintrin.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <sys/auxv.h>

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ __clang__
#  ਜੇ ((__clang_major__ == 18) || (__clang_major__ == 19 && (__clang_minor__ < 1 || (__clang_minor__ == 1 && __clang_patchlevel__ < 2))))
# error ਚੱਕਰੀ_ਜਾਂਚ_32_vx optimizations are broken due ਮੰਜ਼ਿਲ compiler bug ਵਿੱਚ Clang versions: 18.0.0 <= clang_ਸੰਸਕਰਣ < 19.1.2. \
        Either disable the zlib ਚੱਕਰੀ_ਜਾਂਚ_32_vx optimization, or ਚੋਣ ਮੰਜ਼ਿਲ another compiler/compiler ਸੰਸਕਰਣ.
#  endif
#ਜੇ_ਅੰਤ

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ VX_MIN_LEN 64
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ VX_ALIGNMENT 16L
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ VX_ALIGN_ਨਕਾਬ (VX_ALIGNMENT - 1)

ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ uv16qi __attribute__((ਸਦਿਸ਼_ਮਾਪ(16)));
ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ uv4si __attribute__((ਸਦਿਸ਼_ਮਾਪ(16)));
ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ ਲੰਮਾ uv2di __attribute__((ਸਦਿਸ਼_ਮਾਪ(16)));

local uint32_t ਚੱਕਰੀ_ਜਾਂਚ_32_le_vgfm_16(uint32_t ਚੱਕਰੀ_ਜਾਂਚ, ਅਚਲ ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ *buf, size_t len) {
    /*
     * The CRC-32 constant block contains reduction constants to fold and
     * process particular chunks of the input data stream in parallel.
     *
     * For the CRC-32 variants, the constants are precomputed according to
     * these definitions:
     *
     *      R1 = [(x4*128+32 mod P'(x) << 32)]' << 1
     *      R2 = [(x4*128-32 mod P'(x) << 32)]' << 1
     *      R3 = [(x128+32 mod P'(x) << 32)]'   << 1
     *      R4 = [(x128-32 mod P'(x) << 32)]'   << 1
     *      R5 = [(x64 mod P'(x) << 32)]'       << 1
     *      R6 = [(x32 mod P'(x) << 32)]'       << 1
     *
     *      The bitreflected Barret reduction constant, u', is defined as
     *      the bit reversal of floor(x**64 / P(x)).
     *
     *      where P(x) is the polynomial in the normal domain and the P'(x) is the
     *      polynomial in the reversed (bitreflected) domain.
     *
     * CRC-32 (IEEE 802.3 Ethernet, ...) polynomials:
     *
     *      P(x)  = 0x04C11DB7
     *      P'(x) = 0xEDB88320
     */
    ਅਚਲ uv16qi perm_le2be = {15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0};  /* BE->LE mask */
    ਅਚਲ uv2di r2r1 = {0x1C6E41596, 0x154442BD4};                                     /* R2, R1 */
    ਅਚਲ uv2di r4r3 = {0x0CCAA009E, 0x1751997D0};                                     /* R4, R3 */
    ਅਚਲ uv2di r5 = {0, 0x163CD6124};                                                 /* R5 */
    ਅਚਲ uv2di ru_poly = {0, 0x1F7011641};                                            /* u' */
    ਅਚਲ uv2di ਚੱਕਰੀ_ਜਾਂਚ_poly = {0, 0x1DB710641};                                           /* P'(x) << 1 */

    /*
     * Load the initial CRC value.
     *
     * The CRC value is loaded into the rightmost word of the
     * vector register and is later XORed with the LSB portion
     * of the loaded input data.
     */
    uv2di v0 = {0, 0};
    v0 = (uv2di)vec_ਪਾਓ(ਚੱਕਰੀ_ਜਾਂਚ, (uv4si)v0, 3);

    /* Load a 64-byte data chunk and XOR with CRC */
    uv2di v1 = vec_perm(((uv2di *)buf)[0], ((uv2di *)buf)[0], perm_le2be);
    uv2di v2 = vec_perm(((uv2di *)buf)[1], ((uv2di *)buf)[1], perm_le2be);
    uv2di v3 = vec_perm(((uv2di *)buf)[2], ((uv2di *)buf)[2], perm_le2be);
    uv2di v4 = vec_perm(((uv2di *)buf)[3], ((uv2di *)buf)[3], perm_le2be);

    v1 ^= v0;
    buf += 64;
    len -= 64;

    ਜਦੋਂ (len >= 64) {
        /* Load the next 64-byte data chunk */
        uv16qi part1 = vec_perm(((uv16qi *)buf)[0], ((uv16qi *)buf)[0], perm_le2be);
        uv16qi part2 = vec_perm(((uv16qi *)buf)[1], ((uv16qi *)buf)[1], perm_le2be);
        uv16qi part3 = vec_perm(((uv16qi *)buf)[2], ((uv16qi *)buf)[2], perm_le2be);
        uv16qi part4 = vec_perm(((uv16qi *)buf)[3], ((uv16qi *)buf)[3], perm_le2be);

        /*
         * Perform a GF(2) multiplication of the doublewords in V1 with
         * the R1 and R2 reduction constants in V0.  The intermediate result
         * is then folded (accumulated) with the next data chunk in PART1 and
         * stored in V1. Repeat this step for the register contents
         * in V2, V3, and V4 respectively.
         */
        v1 = (uv2di)vec_gfmsum_accum_128(r2r1, v1, part1);
        v2 = (uv2di)vec_gfmsum_accum_128(r2r1, v2, part2);
        v3 = (uv2di)vec_gfmsum_accum_128(r2r1, v3, part3);
        v4 = (uv2di)vec_gfmsum_accum_128(r2r1, v4, part4);

        buf += 64;
        len -= 64;
    }

    /*
     * Fold V1 to V4 into a single 128-bit value in V1.  Multiply V1 with R3
     * and R4 and accumulating the next 128-bit chunk until a single 128-bit
     * value remains.
     */
    v1 = (uv2di)vec_gfmsum_accum_128(r4r3, v1, (uv16qi)v2);
    v1 = (uv2di)vec_gfmsum_accum_128(r4r3, v1, (uv16qi)v3);
    v1 = (uv2di)vec_gfmsum_accum_128(r4r3, v1, (uv16qi)v4);

    ਜਦੋਂ (len >= 16) {
        /* Load next data chunk */
        v2 = vec_perm(*(uv2di *)buf, *(uv2di *)buf, perm_le2be);

        /* Fold next data chunk */
        v1 = (uv2di)vec_gfmsum_accum_128(r4r3, v1, (uv16qi)v2);

        buf += 16;
        len -= 16;
    }

    /*
     * Set up a vector register for byte shifts.  The shift value must
     * be loaded in bits 1-4 in byte element 7 of a vector register.
     * Shift by 8 bytes: 0x40
     * Shift by 4 bytes: 0x20
     */
    uv16qi v9 = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    v9 = vec_ਪਾਓ((ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ)0x40, v9, 7);

    /*
     * Prepare V0 for the next GF(2) multiplication: shift V0 by 8 bytes
     * to move R4 into the rightmost doubleword and set the leftmost
     * doubleword to 0x1.
     */
    v0 = vec_srb(r4r3, (uv2di)v9);
    v0[0] = 1;

    /*
     * Compute GF(2) product of V1 and V0.  The rightmost doubleword
     * of V1 is multiplied with R4.  The leftmost doubleword of V1 is
     * multiplied by 0x1 and is then XORed with rightmost product.
     * Implicitly, the intermediate leftmost product becomes padded
     */
    v1 = (uv2di)vec_gfmsum_128(v0, v1);

    /*
     * Now do the final 32-bit fold by multiplying the rightmost word
     * in V1 with R5 and XOR the result with the remaining bits in V1.
     *
     * To achieve this by a single VGFMAG, right shift V1 by a word
     * and store the result in V2 which is then accumulated.  Use the
     * vector unpack instruction to load the rightmost half of the
     * doubleword into the rightmost doubleword element of V1; the other
     * half is loaded in the leftmost doubleword.
     * The vector register with CONST_R5 contains the R5 constant in the
     * rightmost doubleword and the leftmost doubleword is zero to ignore
     * the leftmost product of V1.
     */
    v9 = vec_ਪਾਓ((ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ)0x20, v9, 7);
    v2 = vec_srb(v1, (uv2di)v9);
    v1 = vec_unpackl((uv4si)v1);  /* Split rightmost doubleword */
    v1 = (uv2di)vec_gfmsum_accum_128(r5, v1, (uv16qi)v2);

    /*
     * Apply a Barret reduction to compute the final 32-bit CRC value.
     *
     * The input values to the Barret reduction are the degree-63 polynomial
     * in V1 (R(x)), degree-32 generator polynomial, and the reduction
     * constant u.  The Barret reduction result is the CRC value of R(x) mod
     * P(x).
     *
     * The Barret reduction algorithm is defined as:
     *
     *    1. T1(x) = floor( R(x) / x^32 ) GF2MUL u
     *    2. T2(x) = floor( T1(x) / x^32 ) GF2MUL P(x)
     *    3. C(x)  = R(x) XOR T2(x) mod x^32
     *
     *  Note: The leftmost doubleword of vector register containing
     *  CONST_RU_POLY is zero and, thus, the intermediate GF(2) product
     *  is zero and does not contribute to the final result.
     */

    /* T1(x) = floor( R(x) / x^32 ) GF2MUL u */
    v2 = vec_unpackl((uv4si)v1);
    v2 = (uv2di)vec_gfmsum_128(ru_poly, v2);

    /*
     * Compute the GF(2) product of the CRC polynomial with T1(x) in
     * V2 and XOR the intermediate result, T2(x), with the value in V1.
     * The final result is stored in word element 2 of V2.
     */
    v2 = vec_unpackl((uv4si)v2);
    v2 = (uv2di)vec_gfmsum_accum_128(ਚੱਕਰੀ_ਜਾਂਚ_poly, v2, (uv16qi)v1);

    ਵਾਪਸ ((uv4si)v2)[2];
}


local ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ s390_ਚੱਕਰੀ_ਜਾਂਚ_32_vx(ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ ਚੱਕਰੀ_ਜਾਂਚ, ਅਚਲ ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ FAR *buf, z_ਮਾਪ_t len)
{
    uintptr_t prealign, aligned, ਬਚਿਆ;

    ਜੇ (buf == Z_NULL) ਵਾਪਸ 0UL;

    ਜੇ (len < VX_MIN_LEN + VX_ALIGN_ਨਕਾਬ)
        ਵਾਪਸ ਚੱਕਰੀ_ਜਾਂਚ_32_z(ਚੱਕਰੀ_ਜਾਂਚ, buf, len);

    ਜੇ ((uintptr_t)buf & VX_ALIGN_ਨਕਾਬ) {
        prealign = VX_ALIGNMENT - ((uintptr_t)buf & VX_ALIGN_ਨਕਾਬ);
        len -= prealign;
        ਚੱਕਰੀ_ਜਾਂਚ = ਚੱਕਰੀ_ਜਾਂਚ_32_z(ਚੱਕਰੀ_ਜਾਂਚ, buf, prealign);
        buf += prealign;
    }
    aligned = len & ~VX_ALIGN_ਨਕਾਬ;
    ਬਚਿਆ = len & VX_ALIGN_ਨਕਾਬ;

    ਚੱਕਰੀ_ਜਾਂਚ = ਚੱਕਰੀ_ਜਾਂਚ_32_le_vgfm_16(ਚੱਕਰੀ_ਜਾਂਚ ^ 0xffffffff, buf, (size_t)aligned) ^ 0xffffffff;

    ਜੇ (ਬਚਿਆ)
        ਚੱਕਰੀ_ਜਾਂਚ = ਚੱਕਰੀ_ਜਾਂਚ_32_z(ਚੱਕਰੀ_ਜਾਂਚ, buf + aligned, ਬਚਿਆ);

    ਵਾਪਸ ਚੱਕਰੀ_ਜਾਂਚ;
}

local z_once_t s390_ਚੱਕਰੀ_ਜਾਂਚ_32_made = Z_ONCE_INIT;
local ਖਾਲੀ s390_ਚੱਕਰੀ_ਜਾਂਚ_32_ਤਿਆਰੀ() {
    ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ hwcap = getauxval(AT_HWCAP);

    ਜੇ (hwcap & HWCAP_S390_VX)
        ਚੱਕਰੀ_ਜਾਂਚ_32_z_hook = s390_ਚੱਕਰੀ_ਜਾਂਚ_32_vx;
    ਨਹੀਂਤਾਂ
        ਚੱਕਰੀ_ਜਾਂਚ_32_z_hook = ਚੱਕਰੀ_ਜਾਂਚ_32_z;
}

local ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ s390_ਚੱਕਰੀ_ਜਾਂਚ_32_init(ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ ਚੱਕਰੀ_ਜਾਂਚ, ਅਚਲ ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ FAR *buf, z_ਮਾਪ_t len)
{
    z_once(&s390_ਚੱਕਰੀ_ਜਾਂਚ_32_made,s390_ਚੱਕਰੀ_ਜਾਂਚ_32_ਤਿਆਰੀ);
    ਵਾਪਸ ਚੱਕਰੀ_ਜਾਂਚ_32_z_hook(ਚੱਕਰੀ_ਜਾਂਚ, buf, len);
}

ZLIB_ਅੰਦਰੂਨੀ ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ (*ਚੱਕਰੀ_ਜਾਂਚ_32_z_hook)(ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ ਚੱਕਰੀ_ਜਾਂਚ, ਅਚਲ ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ FAR *buf, z_ਮਾਪ_t len) = s390_ਚੱਕਰੀ_ਜਾਂਚ_32_init;

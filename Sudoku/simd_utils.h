#pragma once

#include <immintrin.h>
#include <cstdint>

/*
    SIMD Utilities for Sudoku Solvers
    --------------------------------
    This header provides low-level AVX2 helpers operating on
    16 x uint16_t lanes (__m256i).

    Design goals:
    - Readability over cleverness
    - Explicit intent (no macro tricks)
    - Easy reuse in logical / constraint solvers
    - Compiler-friendly (all functions inlineable)

    All masks are lane-wise:
    - true  -> 0xFFFF
    - false -> 0x0000
*/

// ============================================================
// Basic constants
// ============================================================

static inline __m256i vzero()
{
    return _mm256_setzero_si256();
}

static inline __m256i vone16(uint16_t v)
{
    return _mm256_set1_epi16((int16_t)v);
}

// ============================================================
// Mask tests
// ============================================================

/*
    Test: m != 0   (lane-wise)

    Returns:
    - 0xFFFF for lanes where m > 0
    - 0x0000 otherwise
*/
static inline __m256i mask_nonzero_epi16(__m256i m)
{
    return _mm256_cmpgt_epi16(m, vzero());
}

/*
    Test: m == 0   (lane-wise)
*/
static inline __m256i mask_zero_epi16(__m256i m)
{
    return _mm256_cmpeq_epi16(m, vzero());
}

// ============================================================
// Bitmask-specific helpers (Sudoku oriented)
// ============================================================

/*
    Test: single-bit mask
    Equivalent to scalar:
        m != 0 && (m & (m - 1)) == 0

    This is the SIMD core of Naked Single detection.
*/
static inline __m256i mask_single_bit_epi16(__m256i m)
{
    // m != 0
    __m256i nonZero = mask_nonzero_epi16(m);

    // m & (m - 1)
    __m256i mMinus1 = _mm256_sub_epi16(m, vone16(1));
    __m256i andRes = _mm256_and_si256(m, mMinus1);

    // (m & (m - 1)) == 0
    __m256i isZero = mask_zero_epi16(andRes);

    return _mm256_and_si256(nonZero, isZero);
}

/*
    Test: exactly two bits set
    Scalar equivalent:
        popcount(m) == 2

    NOTE:
    This is more expensive than single-bit detection.
    Useful for Naked Pair logic (future use).
*/
static inline __m256i mask_two_bits_epi16(__m256i m)
{
    // Remove lowest bit
    __m256i m1 = _mm256_and_si256(m, _mm256_sub_epi16(m, vone16(1)));

    // Remove second lowest bit
    __m256i m2 = _mm256_and_si256(m1, _mm256_sub_epi16(m1, vone16(1)));

    // Exactly two bits set => m2 == 0 && m1 != 0
    __m256i noThirdBit = mask_zero_epi16(m2);
    __m256i hasTwoBits = mask_nonzero_epi16(m1);

    return _mm256_and_si256(noThirdBit, hasTwoBits);
}

// ============================================================
// Mask aggregation helpers
// ============================================================

/*
    Convert 16 x uint16 lane mask to a compact bitmask.

    Each lane contributes one bit:
    - bit i == 1  -> lane i is non-zero (true)
*/
static inline uint32_t movemask_epi16(__m256i mask)
{
    // Use high bit of each byte, then collapse
    return (uint32_t)_mm256_movemask_epi8(mask);
}

/*
    Check if any lane is true.
*/
static inline bool any_lane(__m256i mask)
{
    return movemask_epi16(mask) != 0;
}

/*
    Find first active lane index (0..15).

    PRECONDITION:
    - at least one lane must be active
*/
static inline int first_lane_index(__m256i mask)
{
    uint32_t bits = movemask_epi16(mask);

    // bits == 0 ise çaðrýlmamalý (any_lane() ile garanti)
    return static_cast<int>(std::countr_zero(bits) >> 1);
}

// ============================================================
// Load / store helpers
// ============================================================

/*
    Load 16 uint16 values (unaligned).
*/
static inline __m256i load_u16(const uint16_t* ptr)
{
    return _mm256_loadu_si256(reinterpret_cast<const __m256i*>(ptr));
}

/*
    Store 16 uint16 values (unaligned).
*/
static inline void store_u16(uint16_t* ptr, __m256i v)
{
    _mm256_storeu_si256(reinterpret_cast<__m256i*>(ptr), v);
}

// ============================================================
// Notes for future extensions
// ============================================================

/*
    Possible future additions:
    - lane-wise popcount approximations
    - digit-wise accumulation (hidden single)
    - unit-based reductions (row/col/box)
    - AVX-512 variants (mask registers)

    This header intentionally keeps SIMD primitives isolated
    from Sudoku logic to preserve readability and debuggability.
*/

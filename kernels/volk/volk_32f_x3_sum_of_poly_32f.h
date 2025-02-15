/* -*- c++ -*- */
/*
 * Copyright 2012, 2014 Free Software Foundation, Inc.
 *
 * This file is part of VOLK
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

/*!
 * \page volk_32f_x3_sum_of_poly_32f
 *
 * \b Overview
 *
 * Calculates the unscaled area under a fourth order polynomial using the
 * rectangular method. The result is the sum of y-values. To get the area,
 * multiply by the rectangle/bin width.
 *
 * Expressed as a formula, this function calculates
 * \f$ \sum f(x) = \sum (c_0 + c_1 \cdot x + c_2 \cdot x^2 + c_3 \cdot x^3 + c_4 \cdot
 * x^4)\f$
 *
 * <b>Dispatcher Prototype</b>
 * \code
 * void volk_32f_x3_sum_of_poly_32f(float* target, float* src0, float* center_point_array,
 * float* cutoff, unsigned int num_points) \endcode
 *
 * \b Inputs
 * \li src0: x values
 * \li center_point_array: polynomial coefficients in order {c1, c2, c3, c4, c0}
 * \li cutoff: the minimum x value to use (will clamp to cutoff if input < cutoff)
 * \li num_points: The number of values in both input vectors.
 *
 * \b Outputs
 * \li complexVector: The sum of y values generated by polynomial.
 *
 * \b Example
 * The following estimates \f$\int_0^\pi e^x dx\f$ by using the Taylor expansion
 * centered around \f$x=1.5\f$,
 * \f$ e^x = e^1.5 * (1 + x + \frac{1}{2} x^2 + \frac{1}{6}x^3 + \frac{1}{24}x^4) \f$
 * \code
 *   int npoints = 4096;
 *   float* coefficients = (float*)volk_malloc(sizeof(float) * 5, volk_get_alignment());
 *   float* input        = (float*)volk_malloc(sizeof(float) * npoints,
 * volk_get_alignment()); float* result       = (float*)volk_malloc(sizeof(float),
 * volk_get_alignment()); float* cutoff       = (float*)volk_malloc(sizeof(float),
 * volk_get_alignment());
 *   // load precomputed Taylor series coefficients
 *   coefficients[0] = 4.48168907033806f;            // c1
 *   coefficients[1] = coefficients[0] * 0.5f;       // c2
 *   coefficients[2] = coefficients[0] * 1.0f/6.0f;  // c3
 *   coefficients[3] = coefficients[0] * 1.0f/24.0f; // c4
 *   coefficients[4] = coefficients[0];              // c0
 *   *cutoff = -2.0;
 *   *result = 0.0f;
 *   // generate uniform input data
 *   float dx = (float)M_PI/ (float)npoints;
 *   for(unsigned int ii=0; ii < npoints; ++ii){
 *       input[ii] = dx * (float)ii - 1.5f;
 *   }
 *   volk_32f_x3_sum_of_poly_32f(result, input, coefficients, cutoff, npoints);
 *   // multiply by bin width to get approximate area
 *   std::cout << "result is " << *result * (input[1]-input[0]) << std::endl;
 *   volk_free(coefficients);
 *   volk_free(input);
 *   volk_free(result);
 *   volk_free(cutoff);
 * \endcode
 */

#ifndef INCLUDED_volk_32f_x3_sum_of_poly_32f_a_H
#define INCLUDED_volk_32f_x3_sum_of_poly_32f_a_H

#include <inttypes.h>
#include <stdio.h>
#include <volk/volk_complex.h>

#ifndef MAX
#define MAX(X, Y) ((X) > (Y) ? (X) : (Y))
#endif

#ifdef LV_HAVE_SSE3
#include <pmmintrin.h>
#include <xmmintrin.h>

static inline void volk_32f_x3_sum_of_poly_32f_a_sse3(float* target,
                                                      float* src0,
                                                      float* center_point_array,
                                                      float* cutoff,
                                                      unsigned int num_points)
{
    float result = 0.0f;
    float fst = 0.0f;
    float sq = 0.0f;
    float thrd = 0.0f;
    float frth = 0.0f;

    __m128 xmm0, xmm1, xmm2, xmm3, xmm4, xmm5, xmm6, xmm7, xmm8, xmm9, xmm10;

    xmm9 = _mm_setzero_ps();
    xmm1 = _mm_setzero_ps();
    xmm0 = _mm_load1_ps(&center_point_array[0]);
    xmm6 = _mm_load1_ps(&center_point_array[1]);
    xmm7 = _mm_load1_ps(&center_point_array[2]);
    xmm8 = _mm_load1_ps(&center_point_array[3]);
    xmm10 = _mm_load1_ps(cutoff);

    int bound = num_points / 8;
    int leftovers = num_points - 8 * bound;
    int i = 0;
    for (; i < bound; ++i) {
        // 1st
        xmm2 = _mm_load_ps(src0);
        xmm2 = _mm_max_ps(xmm10, xmm2);
        xmm3 = _mm_mul_ps(xmm2, xmm2);
        xmm4 = _mm_mul_ps(xmm2, xmm3);
        xmm5 = _mm_mul_ps(xmm3, xmm3);

        xmm2 = _mm_mul_ps(xmm2, xmm0);
        xmm3 = _mm_mul_ps(xmm3, xmm6);
        xmm4 = _mm_mul_ps(xmm4, xmm7);
        xmm5 = _mm_mul_ps(xmm5, xmm8);

        xmm2 = _mm_add_ps(xmm2, xmm3);
        xmm3 = _mm_add_ps(xmm4, xmm5);

        src0 += 4;

        xmm9 = _mm_add_ps(xmm2, xmm9);
        xmm9 = _mm_add_ps(xmm3, xmm9);

        // 2nd
        xmm2 = _mm_load_ps(src0);
        xmm2 = _mm_max_ps(xmm10, xmm2);
        xmm3 = _mm_mul_ps(xmm2, xmm2);
        xmm4 = _mm_mul_ps(xmm2, xmm3);
        xmm5 = _mm_mul_ps(xmm3, xmm3);

        xmm2 = _mm_mul_ps(xmm2, xmm0);
        xmm3 = _mm_mul_ps(xmm3, xmm6);
        xmm4 = _mm_mul_ps(xmm4, xmm7);
        xmm5 = _mm_mul_ps(xmm5, xmm8);

        xmm2 = _mm_add_ps(xmm2, xmm3);
        xmm3 = _mm_add_ps(xmm4, xmm5);

        src0 += 4;

        xmm1 = _mm_add_ps(xmm2, xmm1);
        xmm1 = _mm_add_ps(xmm3, xmm1);
    }
    xmm2 = _mm_hadd_ps(xmm9, xmm1);
    xmm3 = _mm_hadd_ps(xmm2, xmm2);
    xmm4 = _mm_hadd_ps(xmm3, xmm3);
    _mm_store_ss(&result, xmm4);

    for (i = 0; i < leftovers; ++i) {
        fst = *src0++;
        fst = MAX(fst, *cutoff);
        sq = fst * fst;
        thrd = fst * sq;
        frth = sq * sq;
        result += (center_point_array[0] * fst + center_point_array[1] * sq +
                   center_point_array[2] * thrd + center_point_array[3] * frth);
    }

    result += (float)(num_points)*center_point_array[4];
    *target = result;
}


#endif /*LV_HAVE_SSE3*/

#if LV_HAVE_AVX && LV_HAVE_FMA
#include <immintrin.h>

static inline void volk_32f_x3_sum_of_poly_32f_a_avx2_fma(float* target,
                                                          float* src0,
                                                          float* center_point_array,
                                                          float* cutoff,
                                                          unsigned int num_points)
{
    const unsigned int eighth_points = num_points / 8;
    float fst = 0.0;
    float sq = 0.0;
    float thrd = 0.0;
    float frth = 0.0;

    __m256 cpa0, cpa1, cpa2, cpa3, cutoff_vec;
    __m256 target_vec;
    __m256 x_to_1, x_to_2, x_to_3, x_to_4;

    cpa0 = _mm256_set1_ps(center_point_array[0]);
    cpa1 = _mm256_set1_ps(center_point_array[1]);
    cpa2 = _mm256_set1_ps(center_point_array[2]);
    cpa3 = _mm256_set1_ps(center_point_array[3]);
    cutoff_vec = _mm256_set1_ps(*cutoff);
    target_vec = _mm256_setzero_ps();

    unsigned int i;

    for (i = 0; i < eighth_points; ++i) {
        x_to_1 = _mm256_load_ps(src0);
        x_to_1 = _mm256_max_ps(x_to_1, cutoff_vec);
        x_to_2 = _mm256_mul_ps(x_to_1, x_to_1); // x^2
        x_to_3 = _mm256_mul_ps(x_to_1, x_to_2); // x^3
        // x^1 * x^3 is slightly faster than x^2 * x^2
        x_to_4 = _mm256_mul_ps(x_to_1, x_to_3); // x^4

        x_to_2 = _mm256_mul_ps(x_to_2, cpa1); // cpa[1] * x^2
        x_to_4 = _mm256_mul_ps(x_to_4, cpa3); // cpa[3] * x^4

        x_to_1 = _mm256_fmadd_ps(x_to_1, cpa0, x_to_2);
        x_to_3 = _mm256_fmadd_ps(x_to_3, cpa2, x_to_4);
        // this is slightly faster than result += (x_to_1 + x_to_3)
        target_vec = _mm256_add_ps(x_to_1, target_vec);
        target_vec = _mm256_add_ps(x_to_3, target_vec);

        src0 += 8;
    }

    // the hadd for vector reduction has very very slight impact @ 50k iters
    __VOLK_ATTR_ALIGNED(32) float temp_results[8];
    target_vec = _mm256_hadd_ps(
        target_vec,
        target_vec); // x0+x1 | x2+x3 | x0+x1 | x2+x3 || x4+x5 | x6+x7 | x4+x5 | x6+x7
    _mm256_store_ps(temp_results, target_vec);
    *target = temp_results[0] + temp_results[1] + temp_results[4] + temp_results[5];

    for (i = eighth_points * 8; i < num_points; ++i) {
        fst = *src0++;
        fst = MAX(fst, *cutoff);
        sq = fst * fst;
        thrd = fst * sq;
        frth = sq * sq;
        *target += (center_point_array[0] * fst + center_point_array[1] * sq +
                    center_point_array[2] * thrd + center_point_array[3] * frth);
    }
    *target += (float)(num_points)*center_point_array[4];
}
#endif // LV_HAVE_AVX && LV_HAVE_FMA

#ifdef LV_HAVE_AVX
#include <immintrin.h>

static inline void volk_32f_x3_sum_of_poly_32f_a_avx(float* target,
                                                     float* src0,
                                                     float* center_point_array,
                                                     float* cutoff,
                                                     unsigned int num_points)
{
    const unsigned int eighth_points = num_points / 8;
    float fst = 0.0;
    float sq = 0.0;
    float thrd = 0.0;
    float frth = 0.0;

    __m256 cpa0, cpa1, cpa2, cpa3, cutoff_vec;
    __m256 target_vec;
    __m256 x_to_1, x_to_2, x_to_3, x_to_4;

    cpa0 = _mm256_set1_ps(center_point_array[0]);
    cpa1 = _mm256_set1_ps(center_point_array[1]);
    cpa2 = _mm256_set1_ps(center_point_array[2]);
    cpa3 = _mm256_set1_ps(center_point_array[3]);
    cutoff_vec = _mm256_set1_ps(*cutoff);
    target_vec = _mm256_setzero_ps();

    unsigned int i;

    for (i = 0; i < eighth_points; ++i) {
        x_to_1 = _mm256_load_ps(src0);
        x_to_1 = _mm256_max_ps(x_to_1, cutoff_vec);
        x_to_2 = _mm256_mul_ps(x_to_1, x_to_1); // x^2
        x_to_3 = _mm256_mul_ps(x_to_1, x_to_2); // x^3
        // x^1 * x^3 is slightly faster than x^2 * x^2
        x_to_4 = _mm256_mul_ps(x_to_1, x_to_3); // x^4

        x_to_1 = _mm256_mul_ps(x_to_1, cpa0); // cpa[0] * x^1
        x_to_2 = _mm256_mul_ps(x_to_2, cpa1); // cpa[1] * x^2
        x_to_3 = _mm256_mul_ps(x_to_3, cpa2); // cpa[2] * x^3
        x_to_4 = _mm256_mul_ps(x_to_4, cpa3); // cpa[3] * x^4

        x_to_1 = _mm256_add_ps(x_to_1, x_to_2);
        x_to_3 = _mm256_add_ps(x_to_3, x_to_4);
        // this is slightly faster than result += (x_to_1 + x_to_3)
        target_vec = _mm256_add_ps(x_to_1, target_vec);
        target_vec = _mm256_add_ps(x_to_3, target_vec);

        src0 += 8;
    }

    // the hadd for vector reduction has very very slight impact @ 50k iters
    __VOLK_ATTR_ALIGNED(32) float temp_results[8];
    target_vec = _mm256_hadd_ps(
        target_vec,
        target_vec); // x0+x1 | x2+x3 | x0+x1 | x2+x3 || x4+x5 | x6+x7 | x4+x5 | x6+x7
    _mm256_store_ps(temp_results, target_vec);
    *target = temp_results[0] + temp_results[1] + temp_results[4] + temp_results[5];

    for (i = eighth_points * 8; i < num_points; ++i) {
        fst = *src0++;
        fst = MAX(fst, *cutoff);
        sq = fst * fst;
        thrd = fst * sq;
        frth = sq * sq;
        *target += (center_point_array[0] * fst + center_point_array[1] * sq +
                    center_point_array[2] * thrd + center_point_array[3] * frth);
    }
    *target += (float)(num_points)*center_point_array[4];
}
#endif // LV_HAVE_AVX


#ifdef LV_HAVE_GENERIC

static inline void volk_32f_x3_sum_of_poly_32f_generic(float* target,
                                                       float* src0,
                                                       float* center_point_array,
                                                       float* cutoff,
                                                       unsigned int num_points)
{
    const unsigned int eighth_points = num_points / 8;

    float result[8] = { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f };
    float fst = 0.0f;
    float sq = 0.0f;
    float thrd = 0.0f;
    float frth = 0.0f;

    unsigned int i = 0;
    unsigned int k = 0;
    for (i = 0; i < eighth_points; ++i) {
        for (k = 0; k < 8; ++k) {
            fst = *src0++;
            fst = MAX(fst, *cutoff);
            sq = fst * fst;
            thrd = fst * sq;
            frth = fst * thrd;
            result[k] += center_point_array[0] * fst + center_point_array[1] * sq;
            result[k] += center_point_array[2] * thrd + center_point_array[3] * frth;
        }
    }
    for (k = 0; k < 8; k += 2)
        result[k] = result[k] + result[k + 1];

    *target = result[0] + result[2] + result[4] + result[6];

    for (i = eighth_points * 8; i < num_points; ++i) {
        fst = *src0++;
        fst = MAX(fst, *cutoff);
        sq = fst * fst;
        thrd = fst * sq;
        frth = fst * thrd;
        *target += (center_point_array[0] * fst + center_point_array[1] * sq +
                    center_point_array[2] * thrd + center_point_array[3] * frth);
    }
    *target += (float)(num_points)*center_point_array[4];
}

#endif /*LV_HAVE_GENERIC*/

#ifdef LV_HAVE_NEON
#include <arm_neon.h>

static inline void
volk_32f_x3_sum_of_poly_32f_a_neon(float* __restrict target,
                                   float* __restrict src0,
                                   float* __restrict center_point_array,
                                   float* __restrict cutoff,
                                   unsigned int num_points)
{
    unsigned int i;
    float zero[4] = { 0.0f, 0.0f, 0.0f, 0.0f };

    float32x2_t x_to_1, x_to_2, x_to_3, x_to_4;
    float32x2_t cutoff_vector;
    float32x2x2_t x_low, x_high;
    float32x4_t x_qvector, c_qvector, cpa_qvector;
    float accumulator;
    float res_accumulators[4];

    c_qvector = vld1q_f32(zero);
    // load the cutoff in to a vector
    cutoff_vector = vdup_n_f32(*cutoff);
    // ... center point array
    cpa_qvector = vld1q_f32(center_point_array);

    for (i = 0; i < num_points; ++i) {
        // load x  (src0)
        x_to_1 = vdup_n_f32(*src0++);

        // Get a vector of max(src0, cutoff)
        x_to_1 = vmax_f32(x_to_1, cutoff_vector); // x^1
        x_to_2 = vmul_f32(x_to_1, x_to_1);        // x^2
        x_to_3 = vmul_f32(x_to_2, x_to_1);        // x^3
        x_to_4 = vmul_f32(x_to_3, x_to_1);        // x^4
        // zip up doubles to interleave
        x_low = vzip_f32(x_to_1, x_to_2);  // [x^2 | x^1 || x^2 | x^1]
        x_high = vzip_f32(x_to_3, x_to_4); // [x^4 | x^3 || x^4 | x^3]
        // float32x4_t vcombine_f32(float32x2_t low, float32x2_t high); // VMOV d0,d0
        x_qvector = vcombine_f32(x_low.val[0], x_high.val[0]);
        // now we finally have [x^4 | x^3 | x^2 | x] !

        c_qvector = vmlaq_f32(c_qvector, x_qvector, cpa_qvector);
    }
    // there should be better vector reduction techniques
    vst1q_f32(res_accumulators, c_qvector);
    accumulator = res_accumulators[0] + res_accumulators[1] + res_accumulators[2] +
                  res_accumulators[3];

    *target = accumulator + (float)num_points * center_point_array[4];
}

#endif /* LV_HAVE_NEON */


#ifdef LV_HAVE_NEON

static inline void
volk_32f_x3_sum_of_poly_32f_neonvert(float* __restrict target,
                                     float* __restrict src0,
                                     float* __restrict center_point_array,
                                     float* __restrict cutoff,
                                     unsigned int num_points)
{
    unsigned int i;
    float zero[4] = { 0.0f, 0.0f, 0.0f, 0.0f };

    float accumulator;

    float32x4_t accumulator1_vec, accumulator2_vec, accumulator3_vec, accumulator4_vec;
    accumulator1_vec = vld1q_f32(zero);
    accumulator2_vec = vld1q_f32(zero);
    accumulator3_vec = vld1q_f32(zero);
    accumulator4_vec = vld1q_f32(zero);
    float32x4_t x_to_1, x_to_2, x_to_3, x_to_4;
    float32x4_t cutoff_vector, cpa_0, cpa_1, cpa_2, cpa_3;

    // load the cutoff in to a vector
    cutoff_vector = vdupq_n_f32(*cutoff);
    // ... center point array
    cpa_0 = vdupq_n_f32(center_point_array[0]);
    cpa_1 = vdupq_n_f32(center_point_array[1]);
    cpa_2 = vdupq_n_f32(center_point_array[2]);
    cpa_3 = vdupq_n_f32(center_point_array[3]);

    // nathan is not sure why this is slower *and* wrong compared to neonvertfma
    for (i = 0; i < num_points / 4; ++i) {
        // load x
        x_to_1 = vld1q_f32(src0);

        // Get a vector of max(src0, cutoff)
        x_to_1 = vmaxq_f32(x_to_1, cutoff_vector); // x^1
        x_to_2 = vmulq_f32(x_to_1, x_to_1);        // x^2
        x_to_3 = vmulq_f32(x_to_2, x_to_1);        // x^3
        x_to_4 = vmulq_f32(x_to_3, x_to_1);        // x^4
        x_to_1 = vmulq_f32(x_to_1, cpa_0);
        x_to_2 = vmulq_f32(x_to_2, cpa_1);
        x_to_3 = vmulq_f32(x_to_3, cpa_2);
        x_to_4 = vmulq_f32(x_to_4, cpa_3);
        accumulator1_vec = vaddq_f32(accumulator1_vec, x_to_1);
        accumulator2_vec = vaddq_f32(accumulator2_vec, x_to_2);
        accumulator3_vec = vaddq_f32(accumulator3_vec, x_to_3);
        accumulator4_vec = vaddq_f32(accumulator4_vec, x_to_4);

        src0 += 4;
    }
    accumulator1_vec = vaddq_f32(accumulator1_vec, accumulator2_vec);
    accumulator3_vec = vaddq_f32(accumulator3_vec, accumulator4_vec);
    accumulator1_vec = vaddq_f32(accumulator1_vec, accumulator3_vec);

    __VOLK_ATTR_ALIGNED(32) float res_accumulators[4];
    vst1q_f32(res_accumulators, accumulator1_vec);
    accumulator = res_accumulators[0] + res_accumulators[1] + res_accumulators[2] +
                  res_accumulators[3];

    float fst = 0.0;
    float sq = 0.0;
    float thrd = 0.0;
    float frth = 0.0;

    for (i = 4 * (num_points / 4); i < num_points; ++i) {
        fst = *src0++;
        fst = MAX(fst, *cutoff);

        sq = fst * fst;
        thrd = fst * sq;
        frth = sq * sq;
        // fith = sq * thrd;

        accumulator += (center_point_array[0] * fst + center_point_array[1] * sq +
                        center_point_array[2] * thrd + center_point_array[3] * frth); //+
    }

    *target = accumulator + (float)num_points * center_point_array[4];
}

#endif /* LV_HAVE_NEON */

#endif /*INCLUDED_volk_32f_x3_sum_of_poly_32f_a_H*/

#ifndef INCLUDED_volk_32f_x3_sum_of_poly_32f_u_H
#define INCLUDED_volk_32f_x3_sum_of_poly_32f_u_H

#include <inttypes.h>
#include <stdio.h>
#include <volk/volk_complex.h>

#ifndef MAX
#define MAX(X, Y) ((X) > (Y) ? (X) : (Y))
#endif

#if LV_HAVE_AVX && LV_HAVE_FMA
#include <immintrin.h>

static inline void volk_32f_x3_sum_of_poly_32f_u_avx_fma(float* target,
                                                         float* src0,
                                                         float* center_point_array,
                                                         float* cutoff,
                                                         unsigned int num_points)
{
    const unsigned int eighth_points = num_points / 8;
    float fst = 0.0;
    float sq = 0.0;
    float thrd = 0.0;
    float frth = 0.0;

    __m256 cpa0, cpa1, cpa2, cpa3, cutoff_vec;
    __m256 target_vec;
    __m256 x_to_1, x_to_2, x_to_3, x_to_4;

    cpa0 = _mm256_set1_ps(center_point_array[0]);
    cpa1 = _mm256_set1_ps(center_point_array[1]);
    cpa2 = _mm256_set1_ps(center_point_array[2]);
    cpa3 = _mm256_set1_ps(center_point_array[3]);
    cutoff_vec = _mm256_set1_ps(*cutoff);
    target_vec = _mm256_setzero_ps();

    unsigned int i;

    for (i = 0; i < eighth_points; ++i) {
        x_to_1 = _mm256_loadu_ps(src0);
        x_to_1 = _mm256_max_ps(x_to_1, cutoff_vec);
        x_to_2 = _mm256_mul_ps(x_to_1, x_to_1); // x^2
        x_to_3 = _mm256_mul_ps(x_to_1, x_to_2); // x^3
        // x^1 * x^3 is slightly faster than x^2 * x^2
        x_to_4 = _mm256_mul_ps(x_to_1, x_to_3); // x^4

        x_to_2 = _mm256_mul_ps(x_to_2, cpa1); // cpa[1] * x^2
        x_to_4 = _mm256_mul_ps(x_to_4, cpa3); // cpa[3] * x^4

        x_to_1 = _mm256_fmadd_ps(x_to_1, cpa0, x_to_2);
        x_to_3 = _mm256_fmadd_ps(x_to_3, cpa2, x_to_4);
        // this is slightly faster than result += (x_to_1 + x_to_3)
        target_vec = _mm256_add_ps(x_to_1, target_vec);
        target_vec = _mm256_add_ps(x_to_3, target_vec);

        src0 += 8;
    }

    // the hadd for vector reduction has very very slight impact @ 50k iters
    __VOLK_ATTR_ALIGNED(32) float temp_results[8];
    target_vec = _mm256_hadd_ps(
        target_vec,
        target_vec); // x0+x1 | x2+x3 | x0+x1 | x2+x3 || x4+x5 | x6+x7 | x4+x5 | x6+x7
    _mm256_storeu_ps(temp_results, target_vec);
    *target = temp_results[0] + temp_results[1] + temp_results[4] + temp_results[5];

    for (i = eighth_points * 8; i < num_points; ++i) {
        fst = *src0++;
        fst = MAX(fst, *cutoff);
        sq = fst * fst;
        thrd = fst * sq;
        frth = sq * sq;
        *target += (center_point_array[0] * fst + center_point_array[1] * sq +
                    center_point_array[2] * thrd + center_point_array[3] * frth);
    }

    *target += (float)(num_points)*center_point_array[4];
}
#endif // LV_HAVE_AVX && LV_HAVE_FMA

#ifdef LV_HAVE_AVX
#include <immintrin.h>

static inline void volk_32f_x3_sum_of_poly_32f_u_avx(float* target,
                                                     float* src0,
                                                     float* center_point_array,
                                                     float* cutoff,
                                                     unsigned int num_points)
{
    const unsigned int eighth_points = num_points / 8;
    float fst = 0.0;
    float sq = 0.0;
    float thrd = 0.0;
    float frth = 0.0;

    __m256 cpa0, cpa1, cpa2, cpa3, cutoff_vec;
    __m256 target_vec;
    __m256 x_to_1, x_to_2, x_to_3, x_to_4;

    cpa0 = _mm256_set1_ps(center_point_array[0]);
    cpa1 = _mm256_set1_ps(center_point_array[1]);
    cpa2 = _mm256_set1_ps(center_point_array[2]);
    cpa3 = _mm256_set1_ps(center_point_array[3]);
    cutoff_vec = _mm256_set1_ps(*cutoff);
    target_vec = _mm256_setzero_ps();

    unsigned int i;

    for (i = 0; i < eighth_points; ++i) {
        x_to_1 = _mm256_loadu_ps(src0);
        x_to_1 = _mm256_max_ps(x_to_1, cutoff_vec);
        x_to_2 = _mm256_mul_ps(x_to_1, x_to_1); // x^2
        x_to_3 = _mm256_mul_ps(x_to_1, x_to_2); // x^3
        // x^1 * x^3 is slightly faster than x^2 * x^2
        x_to_4 = _mm256_mul_ps(x_to_1, x_to_3); // x^4

        x_to_1 = _mm256_mul_ps(x_to_1, cpa0); // cpa[0] * x^1
        x_to_2 = _mm256_mul_ps(x_to_2, cpa1); // cpa[1] * x^2
        x_to_3 = _mm256_mul_ps(x_to_3, cpa2); // cpa[2] * x^3
        x_to_4 = _mm256_mul_ps(x_to_4, cpa3); // cpa[3] * x^4

        x_to_1 = _mm256_add_ps(x_to_1, x_to_2);
        x_to_3 = _mm256_add_ps(x_to_3, x_to_4);
        // this is slightly faster than result += (x_to_1 + x_to_3)
        target_vec = _mm256_add_ps(x_to_1, target_vec);
        target_vec = _mm256_add_ps(x_to_3, target_vec);

        src0 += 8;
    }

    // the hadd for vector reduction has very very slight impact @ 50k iters
    __VOLK_ATTR_ALIGNED(32) float temp_results[8];
    target_vec = _mm256_hadd_ps(
        target_vec,
        target_vec); // x0+x1 | x2+x3 | x0+x1 | x2+x3 || x4+x5 | x6+x7 | x4+x5 | x6+x7
    _mm256_storeu_ps(temp_results, target_vec);
    *target = temp_results[0] + temp_results[1] + temp_results[4] + temp_results[5];

    for (i = eighth_points * 8; i < num_points; ++i) {
        fst = *src0++;
        fst = MAX(fst, *cutoff);
        sq = fst * fst;
        thrd = fst * sq;
        frth = sq * sq;

        *target += (center_point_array[0] * fst + center_point_array[1] * sq +
                    center_point_array[2] * thrd + center_point_array[3] * frth);
    }

    *target += (float)(num_points)*center_point_array[4];
}
#endif // LV_HAVE_AVX

#endif /*INCLUDED_volk_32f_x3_sum_of_poly_32f_u_H*/

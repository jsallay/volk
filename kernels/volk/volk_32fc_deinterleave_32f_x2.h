/* -*- c++ -*- */
/*
 * Copyright 2012, 2014 Free Software Foundation, Inc.
 *
 * This file is part of VOLK
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

/*!
 * \page volk_32fc_deinterleave_32f_x2
 *
 * \b Overview
 *
 * Deinterleaves the complex floating point vector into I & Q vector
 * data.
 *
 * <b>Dispatcher Prototype</b>
 * \code
 * void volk_32fc_deinterleave_32f_x2(float* iBuffer, float* qBuffer, const lv_32fc_t*
 * complexVector, unsigned int num_points) \endcode
 *
 * \b Inputs
 * \li complexVector: The complex input vector.
 * \li num_points: The number of complex data values to be deinterleaved.
 *
 * \b Outputs
 * \li iBuffer: The I buffer output data.
 * \li qBuffer: The Q buffer output data.
 *
 * \b Example
 * Generate complex numbers around the top half of the unit circle and
 * deinterleave in to real and imaginary buffers.
 * \code
 *   int N = 10;
 *   unsigned int alignment = volk_get_alignment();
 *   lv_32fc_t* in  = (lv_32fc_t*)volk_malloc(sizeof(lv_32fc_t)*N, alignment);
 *   float* re = (float*)volk_malloc(sizeof(float)*N, alignment);
 *   float* im = (float*)volk_malloc(sizeof(float)*N, alignment);
 *
 *   for(unsigned int ii = 0; ii < N; ++ii){
 *       float real = 2.f * ((float)ii / (float)N) - 1.f;
 *       float imag = std::sqrt(1.f - real * real);
 *       in[ii] = lv_cmake(real, imag);
 *   }
 *
 *   volk_32fc_deinterleave_32f_x2(re, im, in, N);
 *
 *   printf("          re  | im\n");
 *   for(unsigned int ii = 0; ii < N; ++ii){
 *       printf("out(%i) = %+.1f | %+.1f\n", ii, re[ii], im[ii]);
 *   }
 *
 *   volk_free(in);
 *   volk_free(re);
 *   volk_free(im);
 * \endcode
 */

#ifndef INCLUDED_volk_32fc_deinterleave_32f_x2_a_H
#define INCLUDED_volk_32fc_deinterleave_32f_x2_a_H

#include <inttypes.h>
#include <stdio.h>

#ifdef LV_HAVE_AVX
#include <immintrin.h>
static inline void volk_32fc_deinterleave_32f_x2_a_avx(float* iBuffer,
                                                       float* qBuffer,
                                                       const lv_32fc_t* complexVector,
                                                       unsigned int num_points)
{
    const float* complexVectorPtr = (float*)complexVector;
    float* iBufferPtr = iBuffer;
    float* qBufferPtr = qBuffer;

    unsigned int number = 0;
    // Mask for real and imaginary parts
    const unsigned int eighthPoints = num_points / 8;
    __m256 cplxValue1, cplxValue2, complex1, complex2, iValue, qValue;
    for (; number < eighthPoints; number++) {
        cplxValue1 = _mm256_load_ps(complexVectorPtr);
        complexVectorPtr += 8;

        cplxValue2 = _mm256_load_ps(complexVectorPtr);
        complexVectorPtr += 8;

        complex1 = _mm256_permute2f128_ps(cplxValue1, cplxValue2, 0x20);
        complex2 = _mm256_permute2f128_ps(cplxValue1, cplxValue2, 0x31);

        // Arrange in i1i2i3i4 format
        iValue = _mm256_shuffle_ps(complex1, complex2, 0x88);
        // Arrange in q1q2q3q4 format
        qValue = _mm256_shuffle_ps(complex1, complex2, 0xdd);

        _mm256_store_ps(iBufferPtr, iValue);
        _mm256_store_ps(qBufferPtr, qValue);

        iBufferPtr += 8;
        qBufferPtr += 8;
    }

    number = eighthPoints * 8;
    for (; number < num_points; number++) {
        *iBufferPtr++ = *complexVectorPtr++;
        *qBufferPtr++ = *complexVectorPtr++;
    }
}
#endif /* LV_HAVE_AVX */

#ifdef LV_HAVE_SSE
#include <xmmintrin.h>

static inline void volk_32fc_deinterleave_32f_x2_a_sse(float* iBuffer,
                                                       float* qBuffer,
                                                       const lv_32fc_t* complexVector,
                                                       unsigned int num_points)
{
    const float* complexVectorPtr = (float*)complexVector;
    float* iBufferPtr = iBuffer;
    float* qBufferPtr = qBuffer;

    unsigned int number = 0;
    const unsigned int quarterPoints = num_points / 4;
    __m128 cplxValue1, cplxValue2, iValue, qValue;
    for (; number < quarterPoints; number++) {
        cplxValue1 = _mm_load_ps(complexVectorPtr);
        complexVectorPtr += 4;

        cplxValue2 = _mm_load_ps(complexVectorPtr);
        complexVectorPtr += 4;

        // Arrange in i1i2i3i4 format
        iValue = _mm_shuffle_ps(cplxValue1, cplxValue2, _MM_SHUFFLE(2, 0, 2, 0));
        // Arrange in q1q2q3q4 format
        qValue = _mm_shuffle_ps(cplxValue1, cplxValue2, _MM_SHUFFLE(3, 1, 3, 1));

        _mm_store_ps(iBufferPtr, iValue);
        _mm_store_ps(qBufferPtr, qValue);

        iBufferPtr += 4;
        qBufferPtr += 4;
    }

    number = quarterPoints * 4;
    for (; number < num_points; number++) {
        *iBufferPtr++ = *complexVectorPtr++;
        *qBufferPtr++ = *complexVectorPtr++;
    }
}
#endif /* LV_HAVE_SSE */


#ifdef LV_HAVE_NEON
#include <arm_neon.h>

static inline void volk_32fc_deinterleave_32f_x2_neon(float* iBuffer,
                                                      float* qBuffer,
                                                      const lv_32fc_t* complexVector,
                                                      unsigned int num_points)
{
    unsigned int number = 0;
    unsigned int quarter_points = num_points / 4;
    const float* complexVectorPtr = (float*)complexVector;
    float* iBufferPtr = iBuffer;
    float* qBufferPtr = qBuffer;
    float32x4x2_t complexInput;

    for (number = 0; number < quarter_points; number++) {
        complexInput = vld2q_f32(complexVectorPtr);
        vst1q_f32(iBufferPtr, complexInput.val[0]);
        vst1q_f32(qBufferPtr, complexInput.val[1]);
        complexVectorPtr += 8;
        iBufferPtr += 4;
        qBufferPtr += 4;
    }

    for (number = quarter_points * 4; number < num_points; number++) {
        *iBufferPtr++ = *complexVectorPtr++;
        *qBufferPtr++ = *complexVectorPtr++;
    }
}
#endif /* LV_HAVE_NEON */


#ifdef LV_HAVE_GENERIC

static inline void volk_32fc_deinterleave_32f_x2_generic(float* iBuffer,
                                                         float* qBuffer,
                                                         const lv_32fc_t* complexVector,
                                                         unsigned int num_points)
{
    const float* complexVectorPtr = (float*)complexVector;
    float* iBufferPtr = iBuffer;
    float* qBufferPtr = qBuffer;
    unsigned int number;
    for (number = 0; number < num_points; number++) {
        *iBufferPtr++ = *complexVectorPtr++;
        *qBufferPtr++ = *complexVectorPtr++;
    }
}
#endif /* LV_HAVE_GENERIC */

#endif /* INCLUDED_volk_32fc_deinterleave_32f_x2_a_H */


#ifndef INCLUDED_volk_32fc_deinterleave_32f_x2_u_H
#define INCLUDED_volk_32fc_deinterleave_32f_x2_u_H

#include <inttypes.h>
#include <stdio.h>

#ifdef LV_HAVE_AVX
#include <immintrin.h>
static inline void volk_32fc_deinterleave_32f_x2_u_avx(float* iBuffer,
                                                       float* qBuffer,
                                                       const lv_32fc_t* complexVector,
                                                       unsigned int num_points)
{
    const float* complexVectorPtr = (float*)complexVector;
    float* iBufferPtr = iBuffer;
    float* qBufferPtr = qBuffer;

    unsigned int number = 0;
    // Mask for real and imaginary parts
    const unsigned int eighthPoints = num_points / 8;
    __m256 cplxValue1, cplxValue2, complex1, complex2, iValue, qValue;
    for (; number < eighthPoints; number++) {
        cplxValue1 = _mm256_loadu_ps(complexVectorPtr);
        complexVectorPtr += 8;

        cplxValue2 = _mm256_loadu_ps(complexVectorPtr);
        complexVectorPtr += 8;

        complex1 = _mm256_permute2f128_ps(cplxValue1, cplxValue2, 0x20);
        complex2 = _mm256_permute2f128_ps(cplxValue1, cplxValue2, 0x31);

        // Arrange in i1i2i3i4 format
        iValue = _mm256_shuffle_ps(complex1, complex2, 0x88);
        // Arrange in q1q2q3q4 format
        qValue = _mm256_shuffle_ps(complex1, complex2, 0xdd);

        _mm256_storeu_ps(iBufferPtr, iValue);
        _mm256_storeu_ps(qBufferPtr, qValue);

        iBufferPtr += 8;
        qBufferPtr += 8;
    }

    number = eighthPoints * 8;
    for (; number < num_points; number++) {
        *iBufferPtr++ = *complexVectorPtr++;
        *qBufferPtr++ = *complexVectorPtr++;
    }
}
#endif /* LV_HAVE_AVX */
#endif /* INCLUDED_volk_32fc_deinterleave_32f_x2_u_H */

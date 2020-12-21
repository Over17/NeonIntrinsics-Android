#include <arm_neon.h>

#include "DotProd.h"

int dotProductScalar(short* inputArray1, short* inputArray2, short len)
{
    int result = 0;

    for (short i = 0; i < len; i++)
    {
        result += inputArray1[i] * inputArray2[i];
    }

    return result;
}

int dotProductNeon(short* inputArray1, short* inputArray2, short len)
{
    const int elementsPerIteration = 4;
    int iterations = len / elementsPerIteration;

    // 4-element vector of zeroes to accumulate the result
    int32x4_t partialSumsNeon = vdupq_n_s32(0);

    // Main loop
    for (int i = 0; i < iterations; ++i)
    {
        // Load vector elements to registers
        int16x4_t v1 = vld1_s16(inputArray1);
        int16x4_t v2 = vld1_s16(inputArray2);

        partialSumsNeon = vmlal_s16(partialSumsNeon, v1, v2);

        inputArray1 += elementsPerIteration;
        inputArray2 += elementsPerIteration;
    }

	// Armv8 instruction to sum up all the elements into a single scalar
	int result = vaddvq_s32(partialSumsNeon);

	// Calculate the tail
	int tailLength = len % elementsPerIteration;
	while (tailLength--)
	{
		result += *inputArray1 * *inputArray2;
		inputArray1++;
		inputArray2++;
	}

    return result;
}

int dotProductNeon2(short* inputArray1, short* inputArray2, short len)
{
    const int elementsPerIteration = 8;
    int iterations = len / elementsPerIteration;

    // 4-element vectors of zeroes to accumulate partial results within the unrolled loop
    int32x4_t partialSum1 = vdupq_n_s32(0);
    int32x4_t partialSum2 = vdupq_n_s32(0);

    // Main loop, unrolled 2-wide
    for (int i = 0; i < iterations; ++i)
    {
        // Load vector elements to registers
        int16x4_t v11 = vld1_s16(inputArray1);
        int16x4_t v12 = vld1_s16(inputArray1 + 4);
        int16x4_t v21 = vld1_s16(inputArray2);
        int16x4_t v22 = vld1_s16(inputArray2 + 4);

        partialSum1 = vmlal_s16(partialSum1, v11, v21);
        partialSum2 = vmlal_s16(partialSum2, v12, v22);

        inputArray1 += elementsPerIteration;
        inputArray2 += elementsPerIteration;
    }

	// Now sum up the results of the 2 partial sums from the loop
    int32x4_t partialSumsNeon = vaddq_s32(partialSum1, partialSum2);

	// Armv8 instruction to sum up all the elements into a single scalar
	int result = vaddvq_s32(partialSumsNeon);

	// Calculate the tail
	int tailLength = len % elementsPerIteration;
	while (tailLength--)
	{
		result += *inputArray1 * *inputArray2;
		inputArray1++;
		inputArray2++;
	}

    return result;
}

int dotProductNeon3(short* inputArray1, short* inputArray2, short len)
{
    const int elementsPerIteration = 12;
    int iterations = len / elementsPerIteration;

    // 4-element vectors of zeroes to accumulate partial results within the unrolled loop
    int32x4_t partialSum1 = vdupq_n_s32(0);
    int32x4_t partialSum2 = vdupq_n_s32(0);
    int32x4_t partialSum3 = vdupq_n_s32(0);

    // Main loop (note that loop index goes through segments). Unroll 3-wide
    for (int i = 0; i < iterations; ++i)
    {
        // Load vector elements to registers
        int16x4_t v11 = vld1_s16(inputArray1);
        int16x4_t v12 = vld1_s16(inputArray1 + 4);
        int16x4_t v13 = vld1_s16(inputArray1 + 8);
        int16x4_t v21 = vld1_s16(inputArray2);
        int16x4_t v22 = vld1_s16(inputArray2 + 4);
        int16x4_t v23 = vld1_s16(inputArray2 + 8);

        partialSum1 = vmlal_s16(partialSum1, v11, v21);
        partialSum2 = vmlal_s16(partialSum2, v12, v22);
        partialSum3 = vmlal_s16(partialSum3, v13, v23);

        inputArray1 += elementsPerIteration;
        inputArray2 += elementsPerIteration;
    }

	// Now sum up the results of the 3 partial sums from the loop
    int32x4_t partialSumsNeon = vaddq_s32(partialSum1, partialSum2);
    partialSumsNeon = vaddq_s32(partialSumsNeon, partialSum3);

	// Armv8 instruction to sum up all the elements into a single scalar
	int result = vaddvq_s32(partialSumsNeon);

	// Calculate the tail
	int tailLength = len % elementsPerIteration;
	while (tailLength--)
	{
		result += *inputArray1 * *inputArray2;
		inputArray1++;
		inputArray2++;
	}

    return result;
}

int dotProductNeon4(short* inputArray1, short* inputArray2, short len)
{
    const int elementsPerIteration = 16;
    int iterations = len / elementsPerIteration;

    // 4-element vectors of zeroes to accumulate partial results within the unrolled loop
    int32x4_t partialSum1 = vdupq_n_s32(0);
    int32x4_t partialSum2 = vdupq_n_s32(0);
    int32x4_t partialSum3 = vdupq_n_s32(0);
    int32x4_t partialSum4 = vdupq_n_s32(0);

    // Main loop (note that loop index goes through segments). Unroll 4-wide
    for (int i = 0; i < iterations; ++i)
    {
        // Load vector elements to registers
        int16x4_t v11 = vld1_s16(inputArray1);
        int16x4_t v12 = vld1_s16(inputArray1 + 4);
        int16x4_t v13 = vld1_s16(inputArray1 + 8);
        int16x4_t v14 = vld1_s16(inputArray1 + 12);
        int16x4_t v21 = vld1_s16(inputArray2);
        int16x4_t v22 = vld1_s16(inputArray2 + 4);
        int16x4_t v23 = vld1_s16(inputArray2 + 8);
        int16x4_t v24 = vld1_s16(inputArray2 + 12);

        partialSum1 = vmlal_s16(partialSum1, v11, v21);
        partialSum2 = vmlal_s16(partialSum2, v12, v22);
        partialSum3 = vmlal_s16(partialSum3, v13, v23);
        partialSum4 = vmlal_s16(partialSum4, v14, v24);

        inputArray1 += elementsPerIteration;
        inputArray2 += elementsPerIteration;
    }

	// Now sum up the results of the 4 partial sums from the loop
    int32x4_t partialSumsNeon = vaddq_s32(partialSum1, partialSum2);
    partialSumsNeon = vaddq_s32(partialSumsNeon, partialSum3);
    partialSumsNeon = vaddq_s32(partialSumsNeon, partialSum4);

	// Armv8 instruction to sum up all the elements into a single scalar
	int result = vaddvq_s32(partialSumsNeon);

	// Calculate the tail
	int tailLength = len % elementsPerIteration;
	while (tailLength--)
	{
		result += *inputArray1 * *inputArray2;
		inputArray1++;
		inputArray2++;
	}

    return result;
}

int dotProductNeon_with_SMLAL2_2wide(short* inputArray1, short* inputArray2, short len)
{
    const int elementsPerIteration = 8;
    int iterations = len / elementsPerIteration;

    // 4-element vectors of zeroes to accumulate the result
    int32x4_t partialSumLow = vdupq_n_s32(0);
    int32x4_t partialSumHigh = vdupq_n_s32(0);

    // Main loop, unrolled 4-wide
    for (int i = 0; i < iterations; ++i)
    {
        // Load vector elements to registers
		// Comparing to SMLAL variant, we're loading 128-bit vectors here (8x short int)
		// and using SMLAL2 (vmlal_high_s16) to calculate dot product for the upper half
		// This way, we're doing only half of the loads comparing to dotProductNeon2
        int16x8_t v1 = vld1q_s16(inputArray1);
        int16x8_t v2 = vld1q_s16(inputArray2);

		partialSumLow  = vmlal_s16(partialSumLow, vget_low_s16(v1), vget_low_s16(v2));
		partialSumHigh = vmlal_high_s16(partialSumHigh, v1, v2);

        inputArray1 += elementsPerIteration;
        inputArray2 += elementsPerIteration;
    }

	// Now sum up the results of the 2 partial sums from the loop
    int32x4_t partialSumsNeon = vaddq_s32(partialSumLow, partialSumHigh);

	// Armv8 instruction to sum up all the elements into a single scalar
	int result = vaddvq_s32(partialSumsNeon);

	// Calculate the tail
	int tailLength = len % elementsPerIteration;
	while (tailLength--)
	{
		result += *inputArray1 * *inputArray2;
		inputArray1++;
		inputArray2++;
	}

    return result;
}

int dotProductNeon_with_SMLAL2_4wide(short* inputArray1, short* inputArray2, short len)
{
    const int elementsPerIteration = 16;
    int iterations = len / elementsPerIteration;

    // 4-element vectors of zeroes to accumulate the result
    int32x4_t partialSum1Low = vdupq_n_s32(0);
    int32x4_t partialSum1High = vdupq_n_s32(0);
    int32x4_t partialSum2Low = vdupq_n_s32(0);
    int32x4_t partialSum2High = vdupq_n_s32(0);

    // Main loop, unrolled 4-wide
    for (int i = 0; i < iterations; ++i)
    {
        // Load vector elements to registers
		// Comparing to SMLAL variant, we're loading 128-bit vectors here (8x short int)
		// and using SMLAL2 (vmlal_high_s16) to calculate dot product for the upper half
		// This way, we're doing only half of the loads comparing to dotProductNeon4
        int16x8_t v11 = vld1q_s16(inputArray1);
        int16x8_t v12 = vld1q_s16(inputArray1 + 8);
        int16x8_t v21 = vld1q_s16(inputArray2);
        int16x8_t v22 = vld1q_s16(inputArray2 + 8);

		partialSum1Low  = vmlal_s16(partialSum1Low, vget_low_s16(v11), vget_low_s16(v21));
		partialSum1High = vmlal_high_s16(partialSum1High, v11, v21);
		partialSum2Low  = vmlal_s16(partialSum2Low, vget_low_s16(v12), vget_low_s16(v22));
		partialSum2High = vmlal_high_s16(partialSum2High, v12, v22);

        inputArray1 += elementsPerIteration;
        inputArray2 += elementsPerIteration;
    }

	// Now sum up the results of the 4 partial sums from the loop
    int32x4_t partialSumsNeon = vaddq_s32(partialSum1Low, partialSum1High);
    partialSumsNeon = vaddq_s32(partialSumsNeon, partialSum2Low);
    partialSumsNeon = vaddq_s32(partialSumsNeon, partialSum2High);

	// Armv8 instruction to sum up all the elements into a single scalar
	int result = vaddvq_s32(partialSumsNeon);

	// Calculate the tail
	int tailLength = len % elementsPerIteration;
	while (tailLength--)
	{
		result += *inputArray1 * *inputArray2;
		inputArray1++;
		inputArray2++;
	}

    return result;
}

int dotProductNeon6(short* inputArray1, short* inputArray2, short len)
{
    const int elementsPerIteration = 24;
    int iterations = len / elementsPerIteration;

    // 4-element vectors of zeroes to accumulate partial results within the unrolled loop
    int32x4_t partialSum1 = vdupq_n_s32(0);
    int32x4_t partialSum2 = vdupq_n_s32(0);
    int32x4_t partialSum3 = vdupq_n_s32(0);
    int32x4_t partialSum4 = vdupq_n_s32(0);
    int32x4_t partialSum5 = vdupq_n_s32(0);
    int32x4_t partialSum6 = vdupq_n_s32(0);

    // Main loop (note that loop index goes through segments). Unroll 6-wide
    for (int i = 0; i < iterations; ++i)
    {
        // Load vector elements to registers
        int16x4_t v11 = vld1_s16(inputArray1);
        int16x4_t v12 = vld1_s16(inputArray1 + 4);
        int16x4_t v13 = vld1_s16(inputArray1 + 8);
        int16x4_t v14 = vld1_s16(inputArray1 + 12);
        int16x4_t v15 = vld1_s16(inputArray1 + 16);
        int16x4_t v16 = vld1_s16(inputArray1 + 20);
        int16x4_t v21 = vld1_s16(inputArray2);
        int16x4_t v22 = vld1_s16(inputArray2 + 4);
        int16x4_t v23 = vld1_s16(inputArray2 + 8);
        int16x4_t v24 = vld1_s16(inputArray2 + 12);
        int16x4_t v25 = vld1_s16(inputArray2 + 16);
        int16x4_t v26 = vld1_s16(inputArray2 + 20);

        partialSum1 = vmlal_s16(partialSum1, v11, v21);
        partialSum2 = vmlal_s16(partialSum2, v12, v22);
        partialSum3 = vmlal_s16(partialSum3, v13, v23);
        partialSum4 = vmlal_s16(partialSum4, v14, v24);
        partialSum5 = vmlal_s16(partialSum5, v15, v25);
        partialSum6 = vmlal_s16(partialSum6, v16, v26);

        inputArray1 += elementsPerIteration;
        inputArray2 += elementsPerIteration;
    }

	// Now sum up the results of the 6 partial sums from the loop
    int32x4_t partialSumsNeon = vaddq_s32(partialSum1, partialSum2);
    partialSumsNeon = vaddq_s32(partialSumsNeon, partialSum3);
    partialSumsNeon = vaddq_s32(partialSumsNeon, partialSum4);
    partialSumsNeon = vaddq_s32(partialSumsNeon, partialSum5);
    partialSumsNeon = vaddq_s32(partialSumsNeon, partialSum6);

	// Armv8 instruction to sum up all the elements into a single scalar
	int result = vaddvq_s32(partialSumsNeon);

	// Calculate the tail
	int tailLength = len % elementsPerIteration;
	while (tailLength--)
	{
		result += *inputArray1 * *inputArray2;
		inputArray1++;
		inputArray2++;
	}

    return result;
}

#pragma once

int dotProductScalar(short* inputArray1, short* inputArray2, short len);
int dotProductNeon(short* inputArray1, short* inputArray2, short len);
int dotProductNeon2(short* inputArray1, short* inputArray2, short len);
int dotProductNeon3(short* inputArray1, short* inputArray2, short len);
int dotProductNeon4(short* inputArray1, short* inputArray2, short len);
int dotProductNeon6(short* inputArray1, short* inputArray2, short len);
int dotProductNeon_with_SMLAL2_2wide(short* inputArray1, short* inputArray2, short len);
int dotProductNeon_with_SMLAL2_4wide(short* inputArray1, short* inputArray2, short len);

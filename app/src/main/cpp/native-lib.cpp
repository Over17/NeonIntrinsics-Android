#include <ctime>
#include <jni.h>
#include <stdio.h>
#include <android/trace.h>

#include "CpuThreadControl.h"
#include "DotProd.h"

class Timer
{
private:
    timespec beg;
    timespec end;

public:
    Timer() { clock_gettime(CLOCK_REALTIME, &beg); }

    double elapsedMs() {
        clock_gettime(CLOCK_REALTIME, &end);
        return (end.tv_sec - beg.tv_sec) * 1000.f +
            (end.tv_nsec - beg.tv_nsec) / 1000000.f;
    }

    void reset() { clock_gettime(CLOCK_REALTIME, &beg); }
};

static short* generateRamp(short startValue, short len)
{
    short* ramp = new short[len];

    for (short i = 0; i < len; i++)
    {
        ramp[i] = startValue + i;
    }

    return ramp;
}

extern "C" JNIEXPORT jstring JNICALL
Java_com_example_neonintrinsics_MainActivity_stringFromJNI(
        JNIEnv* env,
        jobject /* this */)
{
    // Run the test on big cores. Use GetLittleCoreAffinity() to get it tested on little cores.
    SetCurrentThreadAffinity(GetBigCoreAffinity());
    
    // Ramp length and number of trials
    const int rampLength = 1027;
    const int trials = 1000000;

    // Generate two input vectors
    // (0, 1, ..., rampLength - 1)
    // (100, 101, ..., 100 + rampLength-1)
    auto ramp1 = generateRamp(0, rampLength);
    auto ramp2 = generateRamp(100, rampLength);

    // WARMUP!
    // Do a full round of "scalar" calculations before measuring performance
    // This will bump up the frequencies of the CPUs. Otherwise, the results of the first measurement will be unstable,
    // because the governor may react to the increased load with a delay.
    int lastResult = 0;
    ATrace_beginSection("Warm-up");
    for (int i = 0; i < trials; i++)
    {
        lastResult = dotProductScalar(ramp1, ramp2, rampLength);
    }
    ATrace_endSection();
    
    // Without NEON intrinsics
    // Invoke dotProduct and measure performance
    ATrace_beginSection("dotProductScalar");
    Timer timer;
    for (int i = 0; i < trials; i++)
    {
        lastResult = dotProductScalar(ramp1, ramp2, rampLength);
    }
    auto elapsedMsTime = timer.elapsedMs();
    ATrace_endSection();

    // With NEON intrinsics
    // Invoke dotProductNeon and measure performance
    ATrace_beginSection("dotProductNeon");
    int lastResultNeon = 0;
    timer.reset();
    for (int i = 0; i < trials; i++)
    {
        lastResultNeon = dotProductNeon(ramp1, ramp2, rampLength);
    }
    auto elapsedMsTimeNeon = timer.elapsedMs();
    ATrace_endSection();

    ATrace_beginSection("dotProductNeon2");
    int lastResultNeon2 = 0;
    timer.reset();
    for (int i = 0; i < trials; i++)
    {
        lastResultNeon2 = dotProductNeon2(ramp1, ramp2, rampLength);
    }
    auto elapsedMsTimeNeon2 = timer.elapsedMs();
    ATrace_endSection();

    ATrace_beginSection("dotProductNeon3");
    int lastResultNeon3 = 0;
    timer.reset();
    for (int i = 0; i < trials; i++)
    {
        lastResultNeon3 = dotProductNeon3(ramp1, ramp2, rampLength);
    }
    auto elapsedMsTimeNeon3 = timer.elapsedMs();
    ATrace_endSection();

    ATrace_beginSection("dotProductNeon4");
    int lastResultNeon4 = 0;
    timer.reset();
    for (int i = 0; i < trials; i++)
    {
        lastResultNeon4 = dotProductNeon4(ramp1, ramp2, rampLength);
    }
    auto elapsedMsTimeNeon4 = timer.elapsedMs();
    ATrace_endSection();

    ATrace_beginSection("dotProductNeon5");
    int lastResultNeon5 = 0;
    timer.reset();
    for (int i = 0; i < trials; i++)
    {
        lastResultNeon5 = dotProductNeon5(ramp1, ramp2, rampLength);
    }
    auto elapsedMsTimeNeon5 = timer.elapsedMs();
    ATrace_endSection();

    ATrace_beginSection("dotProductNeon6");
    int lastResultNeon6 = 0;
    timer.reset();
    for (int i = 0; i < trials; i++)
    {
        lastResultNeon6 = dotProductNeon6(ramp1, ramp2, rampLength);
    }
    auto elapsedMsTimeNeon6 = timer.elapsedMs();
    ATrace_endSection();

    // Clean up
    delete[] ramp1;
    delete[] ramp2;

    // Display results
    char resultsString[1024];
    snprintf(resultsString, 1024,
        "----==== NO NEON ====----\n\
        Result: %d\
        \nelapsedMs time: %f ms\
        \n\n----==== NEON, no unrolling ====----\n\
        Result: %d\
        \nelapsedMs time: %f ms\
        \n\n----==== NEON 2x unrolling ====----\n\
        Result: %d\
        \nelapsedMs time: %f ms\
        \n\n----==== NEON 3x unrolling ====----\n\
        Result: %d\
        \nelapsedMs time: %f ms\
        \n\n----==== NEON 4x unrolling ====----\n\
        Result: %d\
        \nelapsedMs time: %f ms\
        \n\n----==== NEON 5x unrolling ====----\n\
        Result: %d\
        \nelapsedMs time: %f ms\
        \n\n----==== NEON 6x unrolling ====----\n\
        Result: %d\
        \nelapsedMs time: %f ms",
        lastResult, elapsedMsTime,
        lastResultNeon, elapsedMsTimeNeon,
        lastResultNeon2, elapsedMsTimeNeon2,
        lastResultNeon3, elapsedMsTimeNeon3,
        lastResultNeon4, elapsedMsTimeNeon4,
        lastResultNeon5, elapsedMsTimeNeon5,
        lastResultNeon6, elapsedMsTimeNeon6);

    return env->NewStringUTF(resultsString);
}
# NeonIntrinsics-Android
This repository represents an Android app with native (C++) code which shows usage and some experiments on Neon intrinsics.

## Description
The app generates two arrays `ramp1` and `ramp2` of short int (length set at 1027), and then calculates the dot product as follows:
```
result = ramp1[0] * ramp2[0] + ramp1[1] * ramp2[1] + ramp1[2] * ramp2[2] + ... + ramp1[1026] * ramp2[1026];
```
The dot product is calculated in the following ways:
- scalar: most obvious way, just a cycle which accumulates the result using normal arithmetic operations iterating over the array
- using Neon intrinsics: load 4x the array elements into a vector variable, then issue SMLAL Neon instruction which does multiply and accumulate; iterate over the array in 4x chunks, in the end process the tail (remainder of size divided by 4) using normal arithmentic operations
- using same Neon intrinsics, but manually unroll the loop 2x, 3x, 4x, 5x and 6x

Then, the every calculation is being repeated 1,000,000 times, and the duration is measured. The result is presented on the screen.

Few more notes on the implementation that are system-specific:
- The app targets 64-bit Arm only (ArmV8). ArmV7 is an obsolete architecture, and I don't feel like you should be optimizing for it in 2021.
- The thread running calculations has its affinity set to the last CPU core available, which is big (or the biggest one) in all SoCs available at the moment. You can manually modify the code to set the affinity to a little core (mask 0x1) for experiments.
- First round of calculations is used as a warm-up, its duration is not being measured. This is done to allow the CPU governor to bump up the clock speed of the core running the test; without it, on some devices, the results may be inconsistent as there is some latency before the governor recognizes that the load is high on this thread.

## Interpreting the results
TBD

## Prerequisites
You need Android SDK and NDK paths configured in `local.properties` file in order to build this project. Using a newer NDK is always a good idea, although different NDKs (hence different clangs) will produce different assembly and can cause inconsistent results. Please compare the results only produced by the same NDK version. I used r21d for my tests.

## Building
`gradlew assembleRelease` and then install the APK to the device manually.

## Credits
This repository was originally a mirror of https://github.com/dawidborycki/NeonIntrinsics-Android.

## License
Licensed under MIT license.

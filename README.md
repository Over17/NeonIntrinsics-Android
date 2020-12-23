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
- using same Neon intrinsics, but manually unroll the loop 2x, 3x, 4x and 6x
- using SMLAL + SMLAL2 Neon intrinsics, so that you can load 128-bit vectors and then do multiply-accumulate on low and high halves of the vector, unrolled 2x (SMLAL+SMLAL2) and 4x (2xSMLAL+2xSMLAL2)

Then, the every calculation is being repeated 1,000,000 times, and the duration is measured. The result is presented on the screen.

Few more notes on the implementation that are system-specific:
- The app targets 64-bit Arm only (ArmV8). ArmV7 is an obsolete architecture, and I don't feel like you should be optimizing for it in 2021.
- The thread running calculations has its affinity set to the last CPU core available, which is big (or the biggest one) in all SoCs available at the moment. You can manually modify the code to set the affinity to a little core (mask 0x1) for experiments.
- First round of calculations is used as a warm-up, its duration is not being measured. This is done to allow the CPU governor to bump up the clock speed of the core running the test; without it, on some devices, the results may be inconsistent as there is some latency before the governor recognizes that the load is high on this thread.

## Interpreting the results
To understand the outcome, it's best to dive directly into assembly and match the output to the results measured. I used the following command to dump the assembly (insert your path to the NDK, unpack libnative-lib.so from your APK):
```
c:\android-ndk-r21d\toolchains\llvm\prebuilt\windows-x86_64\aarch64-linux-android\bin\objdump.exe -D libnative-lib.so > libnative-lib.txt
```

For test purpose, the APK has been run on a Pixel 3XL. Here is the result screenshot:
![Results screenshot](results.jpg)

Now let's analyze the results and the assembly:
1.	"No Neon" is significantly faster than "Neon, no unrolling" and matches "Neon, 2x unrolling" results. Checking the assembly for scalar version, it's clear that the compiler has vectorized the loop, unrolled it 2wide and used SMLAL instruction instead of MUL + ADD:
```
    d5c8:	6d7f8d02 	ldp	d2, d3, [x8,#-8]
    d5cc:	6d7f9564 	ldp	d4, d5, [x11,#-8]
    d5d0:	91004108 	add	x8, x8, #0x10
    d5d4:	f100218c 	subs	x12, x12, #0x8
    d5d8:	9100416b 	add	x11, x11, #0x10
    d5dc:	0e628080 	smlal	v0.4s, v4.4h, v2.4h
    d5e0:	0e6380a1 	smlal	v1.4s, v5.4h, v3.4h
    d5e4:	54ffff21 	b.ne	d5c8 <_Z16dotProductScalarPsS_s@@Base+0x48>
```
That's actually a great job by LLVM/clang from the NDK!

2. "Neon, no unrolling" produces the following assembly:
```
    d650:	fc408401 	ldr	d1, [x0],#8
    d654:	fc408422 	ldr	d2, [x1],#8
    d658:	1100054a 	add	w10, w10, #0x1
    d65c:	6b09015f 	cmp	w10, w9
    d660:	0e628020 	smlal	v0.4s, v1.4h, v2.4h
    d664:	54ffff6b 	b.lt	d650 <_Z14dotProductNeonPsS_s@@Base+0x28>
```
It looks great on its own, but one SMLAL appears to be not enough to load the vector units in the CPU. So, **hand-writing straightforward Neon code, even using efficient instructions, yields worse performance than the compiler auto-vectorizing completely scalar loop**. Depending on the target CPU, you'd need to write more specific code than this plain Neon calculation.

3. "Neon, 2x unrolling" produces the same assembly and performance as "No Neon". So, the compiler is defaulting to 2-wide unrolling of the loop. Looking at the "Cortex A-76 software optimization guide", I cannot find any evidence that the CPU is capable of executing two SMLALs at the same time, but it allows forwarding of accumulate operands so two instructions will "overlap" and result in less cycles than 2 pure instructions (4 cycles per SMLAL, 1 cycle accumulate latency, so looks like it's 7 cycles instead of 8)
```
    d738:	6cc10c02 	ldp	d2, d3, [x0],#16
    d73c:	6cc11424 	ldp	d4, d5, [x1],#16
    d740:	1100054a 	add	w10, w10, #0x1
    d744:	6b09015f 	cmp	w10, w9
    d748:	0e648040 	smlal	v0.4s, v2.4h, v4.4h
    d74c:	0e658061 	smlal	v1.4s, v3.4h, v5.4h
    d750:	54ffff4b 	b.lt	d738 <_Z15dotProductNeon2PsS_s@@Base+0x2c>
```

4. "Neon, 3x unrolling" is another tiny bit faster than 2x unrolling. Let's look at the assembly:
```
    d840:	6d401003 	ldp	d3, d4, [x0]
    d844:	fd400805 	ldr	d5, [x0,#16]
    d848:	6d401c26 	ldp	d6, d7, [x1]
    d84c:	fd400830 	ldr	d16, [x1,#16]
    d850:	1100054a 	add	w10, w10, #0x1
    d854:	91006000 	add	x0, x0, #0x18
    d858:	6b09015f 	cmp	w10, w9
    d85c:	0e668060 	smlal	v0.4s, v3.4h, v6.4h
    d860:	0e678082 	smlal	v2.4s, v4.4h, v7.4h
    d864:	0e7080a1 	smlal	v1.4s, v5.4h, v16.4h
    d868:	91006021 	add	x1, x1, #0x18
    d86c:	54fffeab 	b.lt	d840 <_Z15dotProductNeon3PsS_s@@Base+0x3c>
```
Nothing extraordinary happening here, LDP and LDR in the beginning to load 3 64-bit vectors. I would imagine 3x unrolling made sense if there were 3 vector units, but here it was added just for pure science.

5. "Neon, 4x unrolling" is the fastest configuration so far. What is in the assembly?
```
    d960:	6d401404 	ldp	d4, d5, [x0]
    d964:	6d411c06 	ldp	d6, d7, [x0,#16]
    d968:	6d404430 	ldp	d16, d17, [x1]
    d96c:	6d414c32 	ldp	d18, d19, [x1,#16]
    d970:	1100054a 	add	w10, w10, #0x1
    d974:	91008000 	add	x0, x0, #0x20
    d978:	6b09015f 	cmp	w10, w9
    d97c:	0e708080 	smlal	v0.4s, v4.4h, v16.4h
    d980:	0e7180a2 	smlal	v2.4s, v5.4h, v17.4h
    d984:	0e7280c3 	smlal	v3.4s, v6.4h, v18.4h
    d988:	0e7380e1 	smlal	v1.4s, v7.4h, v19.4h
    d98c:	91008021 	add	x1, x1, #0x20
    d990:	54fffe8b 	b.lt	d960 <_Z15dotProductNeon4PsS_s@@Base+0x34>
```
Four LDPs (5 cycles latency each) to load 4x 64bit vectors is more efficient than LDP+LDR (5 cycles, 2 throughput for LDR) to load 3x 64bit. The assembly looks good, and beats the default output of the compiler on scalar loop by some 23%. Nice!

6. "Neon, 6x unrolling" is suddenly slower than 4x. Let's look inside:
```
    dcac:	6d401c06 	ldp	d6, d7, [x0]
    dcb0:	6d414410 	ldp	d16, d17, [x0,#16]
    dcb4:	6d424c12 	ldp	d18, d19, [x0,#32]
    dcb8:	6d405434 	ldp	d20, d21, [x1]
    dcbc:	6d415c36 	ldp	d22, d23, [x1,#16]
    dcc0:	6d426438 	ldp	d24, d25, [x1,#32]
    dcc4:	1100054a 	add	w10, w10, #0x1
    dcc8:	9100c000 	add	x0, x0, #0x30
    dccc:	6b09015f 	cmp	w10, w9
    dcd0:	0e7480c0 	smlal	v0.4s, v6.4h, v20.4h
    dcd4:	0e7580e1 	smlal	v1.4s, v7.4h, v21.4h
    dcd8:	0e768202 	smlal	v2.4s, v16.4h, v22.4h
    dcdc:	0e778223 	smlal	v3.4s, v17.4h, v23.4h
    dce0:	0e788245 	smlal	v5.4s, v18.4h, v24.4h
    dce4:	0e798264 	smlal	v4.4s, v19.4h, v25.4h
    dce8:	9100c021 	add	x1, x1, #0x30
    dcec:	54fffe0b 	b.lt	dcac <_Z15dotProductNeon6PsS_s@@Base+0x48>
```
To be honest, I don't see a clear reason why this would be significantly slower than the 4x version.

7. Now onto the interesting parts. "SMLAL + SMLAL2 version, 2x unrolled loop" (one SMLAL and one SMLAL2 instruction) is 15% faster than regular 2x unrolled loop (and so, the default compiler output on scalar loop). Why?
```
    da80:	3cc10402 	ldr	q2, [x0],#16
    da84:	3cc10423 	ldr	q3, [x1],#16
    da88:	1100054a 	add	w10, w10, #0x1
    da8c:	6b09015f 	cmp	w10, w9
    da90:	0e638040 	smlal	v0.4s, v2.4h, v3.4h
    da94:	4e638041 	smlal2	v1.4s, v2.8h, v3.8h
    da98:	54ffff4b 	b.lt	da80 <_Z32dotProductNeon_with_SMLAL2_2widePsS_s@@Base+0x2c>
```
Wow that's a nice piece of assembly. There are two LDRs (Q form, 6 cycles, 2 throughput), and SMLAL and SMLAL2 uses the same input registers. Since the accumulation is happening in different registers v0 and v1 (meaning no dependency between the instructions are present), it becomes an efficient piece of code with much less loads than in the regular 2x loop.

8. "SMLAL + SMLAL2, 4-wide" unrolled loop (2 SMLAL and SMLAL2 instructions per iteration) has results matching 4x unrolled loop - so it's one of the two fastest options. Actually, I saw it being faster by a mere millisecond, but it's less than 1% so likely within the margin of error. What is in the assembly?
```
    db80:	acc11404 	ldp	q4, q5, [x0],#32
    db84:	acc11c26 	ldp	q6, q7, [x1],#32
    db88:	1100054a 	add	w10, w10, #0x1
    db8c:	6b09015f 	cmp	w10, w9
    db90:	0e668080 	smlal	v0.4s, v4.4h, v6.4h
    db94:	4e668082 	smlal2	v2.4s, v4.8h, v6.8h
    db98:	0e6780a3 	smlal	v3.4s, v5.4h, v7.4h
    db9c:	4e6780a1 	smlal2	v1.4s, v5.8h, v7.8h
    dba0:	54ffff0b 	b.lt	db80 <_Z32dotProductNeon_with_SMLAL2_4widePsS_s@@Base+0x34>
```
It looks like the nicest piece of assembly to me, with two LDPs (Q form, 7 cycles) and then four calculation instructions accumulating in different registers (v0-v3). Short and efficient.

**To summarize**, the compiler is capable of auto-vectorizing the loop and delivering results better than if you hand-write the assembly in a straightforward way. However, 4-wide manually unrolled loop with SMLALs or SMLAL+SMLAL2 are most efficient versions, delivering some 23% performance improvement. However, it's important to understand target CPU capabilities to get best results. If you're targeting the whole android universe, it may be even better to trust the compiler to do the job for you, but in specific cases you can be significantly faster.

## Prerequisites
You need Android SDK and NDK paths configured in `local.properties` file in order to build this project. Using a newer NDK is always a good idea, although different NDKs (hence different clangs) will produce different assembly and can cause inconsistent results. Please compare the results only produced by the same NDK version. I used r21d for my tests.

## Building
`gradlew assembleRelease` and then install the APK to the device manually.

## Credits
This repository was originally a mirror of https://github.com/dawidborycki/NeonIntrinsics-Android.

## License
Licensed under MIT license.

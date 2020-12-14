# NeonIntrinsics-Android
This repository represents an Android app with native (C++) code which shows usage and some experiments on Neon intrinsics.

## Abstract
TBD

## Prerequisites
You need Android SDK and NDK paths configured in `local.properties` file in order to build this project. Using a newer NDK is always a good idea, although different NDKs (hence different clangs) will produce different assembly and can cause inconsistent results. Please compare the results only produced by the same NDK version. I used r21d for my tests.

## Building
`gradlew assembleRelease` and then install the APK to the device manually.

## Credits
This repository was originally a mirror of https://github.com/dawidborycki/NeonIntrinsics-Android.

## License
Licensed under MIT license.

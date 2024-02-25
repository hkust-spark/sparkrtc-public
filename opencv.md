## Step 1. Install OpenCV

```bash
sudo apt install libopencv-dev
```

## Step 2. Copy libraries to the project sysroot
The order of libraries matters. I do not fully understand how it works but it turns out that directly setting the `lib_dirs` to the system library (`/usr/lib/x86_64-linux-gnu`) does not work. However, if we copy the libraries to the sysroot, it works. 
```bash
cp /usr/lib/x86_64-linux-gnu/libopencv_* build/linux/debian_bullseye_amd64-sysroot/usr/lib/x86_64-linux-gnu/
```

## Step 3. Rebuild the WebRTC using standard C++ libraries
The native WebRTC build uses the libc++ library. The OpenCV library is built using the standard C++ library. To avoid the conflict, we need to rebuild the WebRTC using the standard C++ library.
Many tests are incompatible with the standard C++ library. We need to disable them as well.
```bash
rm -rf out/Default
gn gen out/Default --args='use_custom_libcxx=false rtc_include_tests=false'
ninja -C out/Default
```

## Step 4. Replace sysroots for OpenCV
The previous build will likely fail.
We need to manually change the sysroot position for OpenCV-related targets.
We cannot modify the sysroot globally since some targets rely on different libraries.
```bash
./replace_sysroot.sh
ninja -C out/Default
```
**Note: Sometimes you need to repeat the last two commands several times to clean all sysroot issues.**

I know it is ugly but forgive me, I am not familiar with GN build files.
If you can specify the sysroot for only OpenCV-related targets, please submit a pull request.

## All set!
If you are lucky enough, the project will be built successfully!

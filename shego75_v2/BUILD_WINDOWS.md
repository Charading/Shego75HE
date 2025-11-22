Windows build setup for Shego75 v2 (RP2040 / Pico SDK)

This document shows how to install the ARM toolchain and configure the environment so `cmake` + `ninja` can build the firmware in `shego75_v2/`.

1) Requirements
- Pico SDK already at `C:/pico-sdk` (you said this is present).
- Ninja (you mentioned you already have it).
- An ARM cross-compiler providing `arm-none-eabi-gcc` (GNU Arm Embedded Toolchain).

2) Recommended toolchain options

A) Official GNU Arm Embedded Toolchain (prebuilt binaries):
- Download from ARM developer site: https://developer.arm.com/tools-and-software/open-source-software/developer-tools/gnu-toolchain/gnu-rm
- Install (example Windows installer or zip). Locate the folder containing `arm-none-eabi-gcc.exe` after install, e.g. `C:\Program Files (x86)\GNU Arm Embedded Toolchain\10 2020-q4-major\bin`.

B) MSYS2 packages (if you prefer everything in MSYS environment):
- Open `MSYS2 MinGW64` shell and install the toolchain via pacman. Example commands (run in MSYS2 shell):
  pacman -Syu
  pacman -S mingw-w64-x86_64-gcc-arm-none-eabi

  If installed via MSYS2, the binary will live under `C:\msys64\mingw64\bin\arm-none-eabi-gcc.exe`.

3) Verify installation (PowerShell)

```powershell
# check the binary is available on PATH
where.exe arm-none-eabi-gcc
# show version
arm-none-eabi-gcc --version
```

If `where.exe` cannot find it, you need to add the toolchain `bin` directory to your PATH or provide CMake with the path explicitly.

4) Two ways to tell CMake where the toolchain lives

A) Set environment variable (temporary for session):

```powershell
$env:PICO_SDK_PATH = "C:/pico-sdk"
$env:PICO_TOOLCHAIN_PATH = "C:/path/to/arm-toolchain"  # folder that contains arm-none-eabi-gcc.exe
$env:Path += ";$env:PICO_TOOLCHAIN_PATH/bin"
cmake -B build -G "Ninja" -DPICO_SDK_PATH="C:/pico-sdk" -DPICO_TOOLCHAIN_PATH="$env:PICO_TOOLCHAIN_PATH"
cmake --build build --target shego75_v2
cmake --build build --target shego_mini_dev
```

B) Or pass the toolchain path to CMake directly (one-shot):

```powershell
cmake -B build -G "Ninja" -DPICO_SDK_PATH="C:/pico-sdk" -DPICO_TOOLCHAIN_PATH="C:/path/to/arm-toolchain"
cmake --build build --target shego75_v2
```

5) If you installed via MSYS2 and want to use that `arm-none-eabi-gcc` from PowerShell

Either add `C:\msys64\mingw64\bin` to your Windows PATH, or run CMake from the `MSYS2 MinGW64` shell (recommended when using MSYS-provided toolchains).

6) Re-run configure after toolchain install

```powershell
cd x:\Shego75HE\shego75_v2
$env:PICO_SDK_PATH = "C:/pico-sdk"
# if needed set $env:PICO_TOOLCHAIN_PATH as above
cmake -B build -G "Ninja" -DPICO_SDK_PATH="C:/pico-sdk" -DPICO_TOOLCHAIN_PATH="C:/path/to/arm-toolchain"
```

7) Troubleshooting
- If CMake still complains about missing compiler, verify `arm-none-eabi-gcc.exe` exists in the provided folder and that the `bin` directory contains it.
- If using MSYS2-based toolchains, prefer running `cmake` inside the `MSYS2 MinGW64` shell to avoid Windows/Unix path mismatch issues.

If you want, I can:
- Re-run the CMake configure here after you confirm where `arm-none-eabi-gcc.exe` is installed (or after you install it).
- Search the rest of your `C:\` for `arm-none-eabi-gcc.exe` now (it will take longer). Say "search" to let me scan.

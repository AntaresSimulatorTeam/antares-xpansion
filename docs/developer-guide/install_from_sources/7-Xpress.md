# Installing Xpress Solver and Loading it Dynamically

This document explains how to install the Xpress solver in both default and non-default locations, as well as how Antares-Xpansion dynamically loads the Xpress library and license. 

## Table of Contents

1. [Installing Xpress](#installing-xpress)
   - [Default Installation](#default-installation)
   - [Non-Default Installation](#non-default-installation)
2. [How the Code Loads Xpress](#how-the-code-loads-xpress)
   - [Locating Xpress Libraries](#locating-xpress-libraries)
   - [Loading License Information](#loading-license-information)
   - [Handling Different Platforms](#handling-different-platforms)

---

## Installing Xpress

### Default Installation

By default, the Xpress solver will be installed in system-wide directories depending on your operating system. The default installation paths are:

- **Windows**: `C:\xpressmp\bin\xprs.dll` or `C:\Program Files\xpressmp\bin\xprs.dll`
- **macOS**: `/Library/xpressmp/lib/libxprs.dylib`
- **Linux**: `/opt/xpressmp/lib/libxprs.so`

After installing the Xpress solver, the environment variable `XPRESSDIR` should point to the installation directory. This will help the system locate the required libraries.

### Non-Default Installation

For non-default installations, you need to manually configure the `XPRESSDIR` environment variable to point to the custom installation path. For example:

#### Windows
```bash
set XPRESSDIR=C:\custom\xpress\path
```

#### Linux/macOS
```bash
export XPRESSDIR=/home/user/custom/xpress/path
```
Make sure that the dynamic libraries (e.g., xprs.dll, libxprs.dylib, or libxprs.so) are located in the appropriate subdirectories (bin or lib) under the directory specified by XPRESSDIR.

## How the Code Loads Xpress

The C++ code dynamically loads the Xpress solver library and configures the licensing environment at runtime. Here's a step-by-step explanation of how it works:

### Locating Xpress Libraries

1. **Check for `XPRESSDIR`**:  
   The code first checks if the `XPRESSDIR` environment variable is defined. This variable should point to the directory where the Xpress solver is installed.

2. **Build the Path to the Library**:  
   Depending on the platform, the code appends the correct subdirectory to the `XPRESSDIR` path:
   - **Windows**: Adds `\bin` (e.g., `C:\xpressmp\bin\xprs.dll`).
   - **macOS/Linux**: Adds `/lib` (e.g., `/opt/xpressmp/lib/libxprs.so`).

3. **Attempt to Load the Library**:  
   The code then attempts to load the Xpress library using platform-specific functions:
   - **Windows**: Uses `LoadLibrary()` to load `xprs.dll`.
   - **macOS**: Uses `dlopen()` to load `libxprs.dylib`.
   - **Linux**: Uses `dlopen()` to load `libxprs.so`.

4. **Error Handling**:  
   If the library cannot be found or loaded, an error message is displayed, providing troubleshooting suggestions for the user (e.g., checking `XPRESSDIR` or ensuring the file is in the correct path).

### Loading License Information

1. **Locate License Directory**:  
   The code looks for a `license` subdirectory inside the `XPRESSDIR` path. This folder typically contains the necessary license files for the Xpress solver.

2. **Set Environment Variables for License**:  
   To ensure Xpress can authenticate properly, the code sets the following environment variables:

   - **`XPRESSDIR`**: Points to the root installation directory of the Xpress solver.
   - **`XPAUTH`**: If this environment variable is set, it points directly to the license file (e.g., `xpauth.xpr`). This provides a more explicit way to define the path to the license file rather than relying on the default `license` subdirectory.

   This approach allows greater flexibility when managing license files, especially in environments where the `XPRESSDIR/license/` directory may not be the default location for license files.

3. **Error Handling for License**:  
   If the license file or directory is missing, the code raises an error and provides recommendations:
   - Suggests checking the value of `XPRESSDIR` and ensuring the license file exists in the default `license` subdirectory.
   - Advises checking or setting the `XPAUTH` environment variable to the correct license file path in case the file is stored in a custom location.

Using `XPAUTH` allows for better flexibility in license management, particularly in cloud or containerized environments, where environment variables are commonly used to configure paths dynamically.


### Handling Different Platforms

To ensure cross-platform compatibility, the code contains specific logic to handle each operating system:

- **Windows**:  
  The code uses the `LoadLibrary()` function to load the `xprs.dll` file. After loading, it retrieves function pointers from the library using `GetProcAddress()`.

- **Linux/macOS**:  
  The code uses `dlopen()` to load shared libraries (`libxprs.so` for Linux and `libxprs.dylib` for macOS). Function pointers are retrieved using `dlsym()`.

The dynamic loading process ensures that the program does not need to be recompiled when the library is installed in different locations or on different platforms.


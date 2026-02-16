# Dependencies and package managers

C++ dependencies:
- Managed with vcpkg (CMake toolchain file at vcpkg/scripts/buildsystems/vcpkg.cmake).
- The vcpkg triplet is platform-specific (see build.md).

Python dependencies:
- Installed via pip using requirements files.
- Common entry points:
  - requirements-tests.txt
  - requirements.txt
  - requirements-doc.txt


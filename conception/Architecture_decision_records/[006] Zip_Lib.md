Choose Zip Library 
===
This ADR aims to select the library that will be used to manage MPS files in ZIP archives.

Status
=== 
[minizip-ng](https://github.com/zlib-ng/minizip-ng): Deprecated [06/2024] by [[010] Create custom vcpkg ports for deps](%5B010%5D Create_custom_vcpkg_ports_for_deps.md)

Context
===
Mps files produced by both Antares Simulator and Xpansion can have a signicant weigths on disk space. It has been proven that putting them in an archive does not alterate Antares and Xpansion algorithms and naturaly resulting zipped files has a less demand on disk space.

Decision
===

[libzip](https://libzip.org/) is a C library for reading, creating, and modifying zip archives. It's the most cited lib based on Google searchs. The first inconvenient of this lib is the repetitive manipulation of pointers and the risks that come with. 
[libzippp](https://github.com/ctabin/libzippp) is a simple basic C++ wrapper around the [libzip](https://libzip.org/) library. It is meant to be a portable and easy-to-use library for ZIP handling.


libzip & libzippp were rejected because of the way they handle writes:

    - Create a temporary copy of the archive cp archive.zip archive.zip.<random suffix>
    - Write the new entry/entries to the temporary archive
    - Replace the archive with the temporary archive mv archive.zip.<random suffix> archive.zip

This strategy ensures that a valid archive remains even in case the program is interrupted, mv being considered an atomic operation. However, it generates a lot of I/O, which is not suitable for us. This behavior cannot be disabled, meaning it is impossible to write entries straight to the archive.

[minizip-ng](https://github.com/zlib-ng/minizip-ng) on the other hand writes directly to the archive. Attention must be paid to call mz_zip_writer_close in case of user/system interruption. Signal handlers can be used, see e.g AntaresSimulatorTeam/Antares_Simulator#827

libzip and it's wrapper libzippp : rejected

Consequences 
===
Add [minizip-ng](https://github.com/zlib-ng/minizip-ng) as dependency in [antares-deps](https://github.com/AntaresSimulatorTeam/antares-deps)
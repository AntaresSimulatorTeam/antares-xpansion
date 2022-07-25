Choose Zip Library 
===
This Adr aim to select zip library that will be used to manage mps file in arhive.

Status
=== 
libzip and it's wrapper libzippp : suspended

Context
===
Mps files produced by both Antares Simulator and Xpansion can have a signicant weigths on disk space. It's has been proven that putting them in an archive does not alterate Antares and Xpansion algorithms and naturaly resulting zipped files has a less demand on disk space.

Decision
===
[libzip](https://libzip.org/) is a C library for reading, creating, and modifying zip archives. It's the most cited lib based on Google searchs. The first inconvenient of this lib is the repetitive manipulation of pointers and the risks that come with. 
[libzippp](https://github.com/ctabin/libzippp) libzippp is a simple basic C++ wrapper around the [libzip](https://libzip.org/) library. It is meant to be a portable and easy-to-use library for ZIP handling.

Consequences 
===
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

Consequences
===
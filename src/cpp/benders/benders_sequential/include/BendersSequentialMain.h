#ifndef ANTARES_XPANSION_SRC_CPP_BENDERS_BENDERS_SEQUENTIAL_INCLUDE_BENDERSSEQUENTIALMAIN_H
#define ANTARES_XPANSION_SRC_CPP_BENDERS_BENDERS_SEQUENTIAL_INCLUDE_BENDERSSEQUENTIALMAIN_H
#include <filesystem>
#include <string>

int BendersSequentialMain(int argc, char **argv);
int BendersSequentialMain(int argc, char **argv,
                          const std::filesystem::path &options_file);
#endif  // ANTARES_XPANSION_SRC_CPP_BENDERS_BENDERS_SEQUENTIAL_INCLUDE_BENDERSSEQUENTIALMAIN_H
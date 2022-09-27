#ifndef ANTARES_XPANSION_SRC_CPP_BENDERS_BENDERS_MPI_INCLUDE_BENDERSMPIMAIN_H
#define ANTARES_XPANSION_SRC_CPP_BENDERS_BENDERS_MPI_INCLUDE_BENDERSMPIMAIN_H
#include <filesystem>
#include <string>

#define BENDERSMPIMAIN

int BendersMpiMain(int argc, char** argv);
int BendersMpiMain(int argc, char** argv,
                   const std::filesystem::path& options_file);

#endif  // ANTARES_XPANSION_SRC_CPP_BENDERS_BENDERS_MPI_INCLUDE_BENDERSMPIMAIN_H

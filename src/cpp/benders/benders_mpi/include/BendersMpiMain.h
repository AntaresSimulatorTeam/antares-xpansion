#ifndef ANTARES_XPANSION_SRC_CPP_BENDERS_BENDERS_MPI_INCLUDE_BENDERSMPIMAIN_H
#define ANTARES_XPANSION_SRC_CPP_BENDERS_BENDERS_MPI_INCLUDE_BENDERSMPIMAIN_H
#include <boost/mpi.hpp>
#include <filesystem>
#include <string>

#define BENDERSMPIMAIN

int BendersMpiMain(int argc, char** argv, boost::mpi::environment& env,
                   boost::mpi::communicator& world);
int BendersMpiMain(int argc, char** argv,
                   const std::filesystem::path& options_file,
                   boost::mpi::environment& env,
                   boost::mpi::communicator& world);

#endif  // ANTARES_XPANSION_SRC_CPP_BENDERS_BENDERS_MPI_INCLUDE_BENDERSMPIMAIN_H

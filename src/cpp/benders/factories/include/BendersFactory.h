#ifndef ANTARES_XPANSION_SRC_CPP_BENDERS_FACTORIES_INCLUDE_BENDERSFACTORY_H
#define ANTARES_XPANSION_SRC_CPP_BENDERS_FACTORIES_INCLUDE_BENDERSFACTORY_H
#include "BendersMPI.h"
#include "common.h"

class BendersMainFactory {
 private:
  char** argv_;
  std::filesystem::path options_file_;
  boost::mpi::environment* penv_ = nullptr;
  boost::mpi::communicator* pworld_ = nullptr;

 public:
  explicit BendersMainFactory(int argc, char** argv,
                              boost::mpi::environment& env,
                              boost::mpi::communicator& world);
  explicit BendersMainFactory(int argc, char** argv,
                              const std::filesystem::path& options_file,
                              boost::mpi::environment& env,
                              boost::mpi::communicator& world);
  int Run() const;
  int RunExternalLoop() const;
};
#endif  // ANTARES_XPANSION_SRC_CPP_BENDERS_FACTORIES_INCLUDE_BENDERSFACTORY_H
#ifndef ANTARES_XPANSION_SRC_CPP_BENDERS_FACTORIES_INCLUDE_BENDERSFACTORY_H
#define ANTARES_XPANSION_SRC_CPP_BENDERS_FACTORIES_INCLUDE_BENDERSFACTORY_H
#include "BendersByBatch.h"
#include "BendersMPI.h"
#include "BendersSequential.h"
#include "OutputWriter.h"
#include "core/ILogger.h"
enum class BENDERSMETHOD { SEQUENTIAL, MPI, BENDERSBYBATCH, MERGEMPS };

class BendersSequentialFactory {
 private:
  pBendersBase benders_;

 public:
  explicit BendersSequentialFactory(const BendersBaseOptions& benders_options,
                                    Logger logger, Writer writer,
                                    const BENDERSMETHOD& method);

  pBendersBase GetBenders() const;
};

class BendersMainFactory {
 private:
  int argc_;
  char** argv_;
  BENDERSMETHOD method_;
  std::filesystem::path options_file_;

 public:
  explicit BendersMainFactory(int argc, char** argv,
                              const BENDERSMETHOD& method);
  explicit BendersMainFactory(int argc, char** argv,
                              const BENDERSMETHOD& method,
                              const std::filesystem::path& options_file);
  boost::mpi::environment* penv_ = nullptr;
  boost::mpi::communicator* pworld_ = nullptr;
  explicit BendersMainFactory(int argc, char** argv,
                              const BENDERSMETHOD& method,
                              boost::mpi::environment& env,
                              boost::mpi::communicator& world);
  explicit BendersMainFactory(int argc, char** argv,
                              const BENDERSMETHOD& method,
                              const std::filesystem::path& options_file,
                              boost::mpi::environment& env,
                              boost::mpi::communicator& world);
  int Run() const;
};
#endif  // ANTARES_XPANSION_SRC_CPP_BENDERS_FACTORIES_INCLUDE_BENDERSFACTORY_H

#include "BendersOptions.h"
#include "common_mpi.h"
#include "JsonWriter.h"
#include "core/ILogger.h"

void run_mpibenders(mpi::communicator& world, mpi::environment& env, const BendersOptions& options, Logger& logger, JsonWriter& jsonWriter_l);
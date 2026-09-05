#pragma once

#include <filesystem>
#include <fstream>
#include <limits>
#include <map>
#include <memory>
#include <string>
#include <vector>

#include <boost/mpi.hpp>
#include <boost/mpi/communicator.hpp>
#include <boost/serialization/deque.hpp>
#include <boost/serialization/list.hpp>
#include <boost/serialization/map.hpp>
#include <boost/serialization/set.hpp>
#include <boost/serialization/shared_ptr.hpp>
#include <boost/serialization/string.hpp>
#include <boost/serialization/utility.hpp>
#include <boost/serialization/vector.hpp>

#include "antares-xpansion/benders/benders_core/SubproblemWorker.h"
#include "antares-xpansion/xpansion_interfaces/ILogger.h"

namespace mpi = boost::mpi;

class BestUbTracker
{
public:
    BestUbTracker() = default;
    BestUbTracker(mpi::communicator* world,
                  const std::filesystem::path& file_path,
                  const std::filesystem::path& output_root,
                  Logger logger);

    ~BestUbTracker() = default;

    void set_variables_values(std::string sub_name,
                              const std::shared_ptr<SubproblemWorker>& worker,
                              int iter,
                              double new_ub);

    void dump_values();

private:
    bool set_best_ub_solution_(double new_best_ub, int iter);

    std::ifstream file_stream_;
    mpi::communicator* _world;
    std::filesystem::path output_file_;
    Logger _logger;

    double best_ub_ = std::numeric_limits<double>::max();
    int last_iteration_update_ = -1;

    std::vector<std::string> variables_to_follow_;
    std::map<std::string, std::vector<int>> variables_to_follow_indices_per_sub_;
    std::map<std::string, std::vector<double>> values_per_sub_;
};

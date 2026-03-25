#include "antares-xpansion/benders/logger/MicroIterationsLog.h"

#include <fstream>
#include <sstream>

MicroIterationsLog::MicroIterationsLog(
  const SimulationOptions& options,
  std::map<std::string, std::string>& sub_constraints_map,
  std::map<std::string, std::vector<std::string>>& constraints_per_line,
  bool warm_start,
  mpi::communicator* world,
  int log_level):
    options_(options)
{
    warm_start_ = warm_start;
    _world = world;
    log_level_ = log_level;
    std::filesystem::path micro_iterations_log_path = std::filesystem::path(options_.OUTPUTROOT)
                                                      / std::filesystem::path(
                                                        "micro_iterations_proc_"
                                                        + std::to_string(world->rank()) + ".log");

    log_file_.open(micro_iterations_log_path.c_str());

    if (world->rank() == 0)
    {
        log_file_
          << "************************** MICRO ITERS config ************************** \n\n";
        if (warm_start_)
        {
            log_file_ << "warm_start=1\n\n";
        }
        else
        {
            log_file_ << "warm_start=0\n\n";
        }

        std::filesystem::path added_constraints_repo_path = std::filesystem::path(
                                                              options_.OUTPUTROOT)
                                                            / "added_constraints";
        if (!std::filesystem::exists(added_constraints_repo_path))
        {
            std::cout << "creating the folder for the log per sub " << std::endl;
            std::filesystem::create_directories(added_constraints_repo_path);
        }
        else
        {
            std::cout << "added constraints folder already exist  !!!" << std::endl;
        }
    }
}

void MicroIterationsLog::AddMasterIterationLog(int num_iter, std::string elapsed_time)
{
    log_file_ << "<Master>\n";
    log_file_ << "num iter : " << num_iter << "\n";
    log_file_ << "PTDF_compute_time : " << elapsed_time << "\n";
    log_file_ << "</Master>\n";
}

void MicroIterationsLog::AddMicroIterionLog(std::string sub_name,
                                            std::string solving_time,
                                            std::string adding_rows_time,
                                            std::vector<std::string> added_constraints_keys)
{
    log_file_ << "<MICRO_ITERATION_" << sub_name << ">\n";
    log_file_ << "solving time : " << solving_time << "\n";
    log_file_ << "adding_rows_time : " << adding_rows_time << "\n";
    log_file_ << "</MICRO_ITERATION_" << sub_name << ">\n";
}

void MicroIterationsLog::AddMicroIterCount(std::string sub_name, int num_micro_iter)
{
    log_file_ << "<count_" << sub_name << ">\n";
    log_file_ << "num_micro_iter : " << num_micro_iter << "\n";
    log_file_ << "</count_" << sub_name << ">\n";
}

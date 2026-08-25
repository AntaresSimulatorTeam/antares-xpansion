#include <filesystem>
#include <fstream>
#include <map>
#include <string>
#include <vector>

namespace boost
{
namespace mpi
{
class communicator;
}
}  // namespace boost
namespace mpi = boost::mpi;

extern "C"
{
    void OnBendersStart(std::vector<std::string> sub_problems, int rank,
                        std::filesystem::path input_root, std::filesystem::path output_root,
                        bool warm_start, mpi::communicator* world, int log_level)
    {
    }

    void OnBendersEnd()
    {
    }

    void OnBendersIterationStart()
    {
    }

    void OnBendersIterationEnd()
    {
    }

    void OnBendersMicroIterationStart()
    {
    }

    void OnBendersMicroIterationEnd(std::string sub_name,
                                    bool& added_rows,
                                    std::string solving_time,
                                    std::vector<double> sub_solution,
                                    std::vector<int> variables_indices_vector,
                                    std::vector<std::string>& variables_names_vector,
                                    std::filesystem::path input_root,
                                    std::vector<std::string>& constraints_to_add_vec,
                                    int num_master_iter,
                                    int num_micro_iter)
    {
        auto stem = std::filesystem::path(sub_name).stem().string();
        auto filepath = input_root / (stem + "_master_" + std::to_string(num_master_iter) +
                                      "_micro_" + std::to_string(num_micro_iter) + ".txt");
        std::ifstream ifs(filepath);
        std::string line;
        while (std::getline(ifs, line))
        {
            constraints_to_add_vec.push_back(line);
        }
    }

    void OnBendersMasterResolutionStart()
    {
    }

    void OnBendersMasterResolutionEnd(std::map<std::string, double>& master_out,
                                      int& num_iter,
                                      mpi::communicator* world,
                                      std::filesystem::path input_root)
    {
    }

    void OnBendersSubResolutionStart()
    {
    }

    void OnBendersSubResolutionEnd(std::string sub_name, int num_micro_iter)
    {
    }
}

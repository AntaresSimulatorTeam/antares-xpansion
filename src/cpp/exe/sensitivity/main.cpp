#include <iostream>
#include <filesystem>
#include <boost/program_options.hpp>

#include "SensitivityInputReader.h"
#include "SensitivityAnalysis.h"

namespace po = boost::program_options;

int main(int argc, char **argv)
{

    try
    {   
        std::string last_master_mps_path;
        std::string benders_out_json_path;
        std::string structure_file_path;
        double epsilon;

        po::options_description desc("Allowed options");

        desc.add_options()("help,h", "produce help message")("master,m", po::value<std::string>(&last_master_mps_path)->required(), "path to the last master mps file")("json,j", po::value<std::string>(&benders_out_json_path)->required(), "path to the benders json output file")("structure,s", po::value<std::string>(&structure_file_path)->required(), "path to the structure file")("epsilon,e", po::value<double>(&epsilon)->required(), "maximum cost difference with best benders solution");

        po::variables_map opts;
        po::store(po::parse_command_line(argc, argv, desc), opts);

        if (opts.count("help"))
        {
            std::cout << desc << std::endl;
            return 0;
        }

        po::notify(opts);

        SensitivityInputReader sensitivity_input_reader = SensitivityInputReader(last_master_mps_path, benders_out_json_path, structure_file_path, epsilon);
        SolverAbstract::Ptr last_master = sensitivity_input_reader.read_last_master();
        double best_ub = sensitivity_input_reader.read_best_ub();
        std::map<int, std::string> id_to_name = sensitivity_input_reader.get_id_to_name();

        std::string sensitivity_json_name = "out_sensitivity";
        auto writer = std::make_shared<SensitivityWriter>(sensitivity_json_name + ".json");
        SensitivityAnalysis sensitivity_analysis = SensitivityAnalysis(epsilon, best_ub, id_to_name, last_master, writer);

        sensitivity_analysis.launch();

        return 0;
    }
    catch (std::exception &e)
    {
        std::cerr << "error: " << e.what() << std::endl;
        return 1;
    }
    catch (...)
    {
        std::cerr << "Exception of unknown type!" << std::endl;
    }
}
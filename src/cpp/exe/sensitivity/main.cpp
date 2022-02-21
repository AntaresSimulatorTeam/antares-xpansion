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
        std::string json_input_path;
        std::string json_output_path;

        po::options_description desc("Allowed options");

        desc.add_options()("help,h", "produce help message")("json,j", po::value<std::string>(&json_input_path)->required(), "path to the json input file")("output,o", po::value<std::string>(&json_output_path)->required(), "path to the json output file");

        po::variables_map opts;
        po::store(po::parse_command_line(argc, argv, desc), opts);

        if (opts.count("help"))
        {
            std::cout << desc << std::endl;
            return 0;
        }

        po::notify(opts);

        auto sensitivity_input_reader = SensitivityInputReader(json_input_path);
        SensitivityInputData input_data = sensitivity_input_reader.get_input_data();

        auto writer = std::make_shared<SensitivityWriter>(json_output_path);
        auto sensitivity_analysis = SensitivityAnalysis(input_data, writer);

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
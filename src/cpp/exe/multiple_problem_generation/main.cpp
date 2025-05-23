#include <iostream>

#include "antares-xpansion/lpnamer/main/MultipleProblemGeneration.h"

int main(int argc, char** argv)
{
    try
    {
        // We reuse the same options as the single executable
        // And interpret the path given in the options as a path to a file
        // which contains the real path (either archive or output or study) for each node
        auto options_parser = ProblemGenerationExeOptions();
        options_parser.Parse(argc, argv);

        MultipleProblemGeneration mpbg(options_parser);
        mpbg.load_input_paths();
        mpbg.run_generation();
        mpbg.write_lp_paths();

        return 0;
    }
    catch (std::exception& e)
    {
        std::cerr << "error: " << e.what() << std::endl;
        return 1;
    }
    catch (...)
    {
        std::cerr << "Exception of unknown type!" << std::endl;
    }

    return 0;
}

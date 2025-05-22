#include <iostream>

#include "antares-xpansion/merge_weights_trajectory/MergeWeightsTrajectory.h"
#include "antares-xpansion/benders/logger/User.h"

/*
    We generate a merged weight file corresponding to a merged trajectory problem
    The weight of a given subproblem is : 
    (its weight in its nodal problem (MC probability)) times (its nodes weight in the trajectory horizon (probability + discounting))
    If customs MC weights were defined during problem generation, we merge & amend those weights, and if not, we use uniform probability of MC years.
*/
int main(int argc, char** argv)
{
    try
    {
        if (argc < 4)
        {
            std::cerr << "Error: usage is : <exe> <master_structure_file> <nodal_lp_folder_file> <output_file_path>" << std::endl;
            std::exit(1);
        }

        Logger logger = std::make_shared<xpansion::logger::User>(std::cout);

        logger->display_message("Starting MergeWeightsTrajectory : generating a merged weight file",
                                LogUtils::LOGLEVEL::INFO,
                                MERGE_WEIGHTS_CONTEXT);

        MergeWeightsTrajectory merged_weights_generator(
            argv[1],
            argv[2],
            argv[3],
            logger
        );

        merged_weights_generator.generate_merged_weights_file();
        
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

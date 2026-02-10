

#include <exception>
#include <iostream>
#include <ranges>
#include <antares/api/singleProblemGetter.h>

#include "antares/api/singleProblemGetter.h"
#include "antares-xpansion/benders/benders_mpi/common_mpi.h"
#include "antares-xpansion/benders/factories/BendersApp.h"
#include "antares-xpansion/helpers/OptionsParser.h"

namespace po = boost::program_options;
namespace fs = std::filesystem;
using namespace std::string_literals;

class MyOptionsParser : public OptionsParser {
public:
    MyOptionsParser() : OptionsParser("Benders With Problem Generation exe"s) {
        AddOptions()("help,h",
                     "produce help message")("study,s",
                                             po::value<fs::path>(&study_),
                                             "antares simulator study")(
            "benders-options,b",
            po::value<fs::path>(&bendersOptions_),
            "Benders options file");
    }

    [[nodiscard]] fs::path Study() const {
        return study_;
    }

    [[nodiscard]] fs::path bendersOptions() const {
        return bendersOptions_;
    }

private:
    fs::path study_;
    fs::path bendersOptions_;
};

void updatePaths(const fs::path &outPath, SimulationOptions &bendersOptions) {
    auto expansionDir = outPath / "expansion";
    fs::create_directory(expansionDir);
    bendersOptions.INPUTROOT = outPath.string();
    bendersOptions.OUTPUTROOT = outPath.string();
    bendersOptions.STRUCTURE_FILE = (outPath / "structure.txt").string();
    bendersOptions.JSON_FILE = (expansionDir / "out.json").string();
    bendersOptions.LAST_ITERATION_JSON_FILE = (expansionDir / "last_iteration.json").string();
}

std::filesystem::path FindOutputPath(const std::filesystem::path &outputDir) {
    if (!fs::exists(outputDir) || !fs::is_directory(outputDir))
        throw std::runtime_error("Output folder not found");
    if (outputDir.empty()) {
        throw std::runtime_error("Output folder is Empty");
    }
    auto dirs =
            fs::directory_iterator(outputDir)
            | std::views::filter([](const fs::directory_entry &e) {
                return e.is_directory();
            });

    auto it = std::ranges::max(
        dirs,
        std::less{},
        [](const fs::directory_entry &e) {
            return fs::last_write_time(e);
        });


    return it.path();
}

int main(int argc, char **argv) {
    try {
        mpi::environment env(argc, argv);
        mpi::communicator world;
        MyOptionsParser optionsParser;
        optionsParser.Parse(argc, argv);
        // First check usage (options are given)
        if (world.rank() == 0) {
            //     usage(argc);
            const Antares::Solver::SingleProblemGetter getter(optionsParser.Study());
            getter.printProblems();
        }
        const auto outPath = FindOutputPath(optionsParser.Study() / "output");
        SimulationOptions bendersOptions(optionsParser.bendersOptions());
        updatePaths(outPath, bendersOptions);
        auto benders_factory = BendersApp(std::move(bendersOptions), world, SOLVER::BENDERS);
        return benders_factory.Run();
    } catch (std::exception &e) {
        std::cerr << "error: " << e.what() << std::endl;
        return 1;
    } catch (...) {
        std::cerr << "Exception of unknown type!" << std::endl;
        return 1;
    }
}

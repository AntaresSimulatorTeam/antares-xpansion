
#include "antares-xpansion/benders/factories/LoggerFactories.h"
#include "antares-xpansion/benders/output/JsonWriter.h"
#include "antares-xpansion/grid_search/GridSearch.h"

int main(int argc, char** argv)
{
    auto path_to_log = std::filesystem::path("./");
    std::ofstream loggerFile("report.txt");
    loggerFile.close();
    auto logger_factory = FileAndStdoutLoggerFactory(path_to_log / "report.txt", false);
    Logger logger = logger_factory.get_logger();
    auto json_file_name = path_to_log / "output.json";
    std::shared_ptr<Output::OutputWriter> writer = std::make_shared<Output::JsonWriter>(
      std::make_shared<Clock>(),
      json_file_name);
      
    auto grid_search = GridSearch(logger, writer);
    grid_search.launch();
}

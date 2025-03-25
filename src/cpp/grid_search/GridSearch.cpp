
#include "antares-xpansion/grid_search/GridSearch.h"

#include <utility>

#include "antares-xpansion/benders/benders_core/WorkerMaster.h"
#include "antares-xpansion/helpers/Timer.h"
#include "antares-xpansion/xpansion_interfaces/LoggerUtils.h"

struct InvalidStructureFile: LogUtils::XpansionError<std::runtime_error>
{
    using LogUtils::XpansionError<std::runtime_error>::XpansionError;
};

GridSearch::GridSearch(Logger logger, std::shared_ptr<Output::OutputWriter> writer)
{
    _logger = std::move(logger);
    _writer = std::move(writer);
}

GridSearch::GridSearch(Logger logger, std::shared_ptr<Output::OutputWriter> writer, std::filesystem::path path_to_data)
{
    _logger = std::move(logger);
    _writer = std::move(writer);
    xpansionFolderPath = std::move(path_to_data);
}

/*!
 *  \brief Method to load each problem in a thread
 *
 *  The initialization of each problem is done sequentially
 *
 */

void GridSearch::InitializeProblems()
{
    SetInvestmentCostPerMwPerYear(GetMasterProblemPath());

    InitCouplingMap();
    MatchProblemToId();

    int current_problem_id = 0;
    // Dispatch subproblems to process
    for (const auto& problem: coupling_map)
    {
        const auto subProblemFilePath = GetSubproblemPath(problem.first);
        AddSubproblem(problem);
        current_problem_id++;
    }
}

void GridSearch::InitCouplingMap()
{
    std::filesystem::path structure_path = xpansionFolderPath / "structure.txt";
    std::ifstream summary(structure_path, std::ios::in);
    if (!summary)
    {
        auto log_location = LOGLOCATION;
        std::ostringstream msg;
        msg << "Cannot open structure file " << structure_path << std::endl;
        _logger->display_message(msg.str(), LogUtils::LOGLEVEL::FATAL, log_location);
        throw InvalidStructureFile(PrefixMessage(LogUtils::LOGLEVEL::FATAL,
                                                 "GridSearch::InitCouplingMap"),
                                   msg.str(),
                                   log_location);
    }
    std::string line;

    while (std::getline(summary, line))
    {
        // If problem name is master, skip
        if (line.find("master") != std::string::npos)
        {
            continue;
        }
        std::stringstream buffer(line);
        std::string problem_name;
        std::string variable_name;
        int variable_id;
        buffer >> problem_name;
        buffer >> variable_name;
        buffer >> variable_id;
        coupling_map[problem_name][variable_name] = variable_id;
    }
}

void GridSearch::MatchProblemToId()
{
    int count = 0;
    for (const auto& problem: coupling_map)
    {
        _problem_to_id[problem.first] = count;
        count++;
    }
}

std::filesystem::path GridSearch::GetSubproblemPath(const std::string& slave_name) const
{
    return xpansionFolderPath / slave_name;
}

std::filesystem::path GridSearch::GetMasterProblemPath() const
{
    return xpansionFolderPath / "master.mps";
}

void GridSearch::AddSubproblem(const std::pair<std::string, VariableMap>& kvp)
{
    subproblem_map[kvp.first] = std::make_shared<SubproblemWorker>(kvp.second,
                                                                   GetSubproblemPath(kvp.first),
                                                                   1, // This has to be changed with the subproblem's weight
                                                                   "COIN",
                                                                   0,
                                                                   solver_log_manager_,
                                                                   _logger,
                                                                   ProblemsFormat::SAVED_FILE);
    subproblems.push_back(kvp.first);
}

void GridSearch::SetInvestmentCostPerMwPerYear(const std::filesystem::path& path_to_mps)
{
    std::string line;
    std::ifstream mps_file(path_to_mps);
    if (!mps_file.is_open())
    {
        std::stringstream msg;
        msg << "Could not open mps file: " << path_to_mps;
        throw std::runtime_error(msg.str());
    }
    // Read From line COLUMNS to line RHS
    while (std::getline(mps_file, line))
    {
        if (line.find("COLUMNS") != std::string::npos)
        {
            break;
        }
    }
    while (std::getline(mps_file, line))
    {
        if (line.find("RHS") != std::string::npos)
        {
            break;
        }
        std::stringstream ss(line);
        std::string variableName;
        std::string OBJROW;
        double value;
        ss >> variableName >> OBJROW >> value;
        if (OBJROW == "OBJ" || OBJROW == "OBJROW")
        {
            investCostPerMwPerYear[variableName] = value;
        }
    }
}

void GridSearch::SetGridPoints()
{
    // Fill the grid points vector with file
    std::vector<std::string> header;
    std::ifstream grid_file(xpansionFolderPath / "grid.csv");
    if (!grid_file.is_open())
    {
        throw std::runtime_error("Could not open grid file");
    }

    std::string line;
    bool isFirstLine = true;

    while (std::getline(grid_file, line))
    {
        std::istringstream ss(line);
        std::string token;

        if (isFirstLine) // First line is the header
        {
            while (std::getline(ss, token, ','))
            {
                if (!token.empty())
                {
                    header.push_back(token);
                }
            }
            isFirstLine = false;
        }
        else // Next lines are the grid points
        {
            Point point;
            int index = -1;
            while (std::getline(ss, token, ','))
            {
                if (index != -1) // Skip the first column (index)
                {
                    point[header[index]] = std::stod(token);
                }
                ++index;
            }
            gridPoints.push_back(point);
        }
    }
    grid_file.close();
    gridPointData.resize(gridPoints.size());
}

void GridSearch::Run()
{
    for (int i = 0; i < gridPoints.size(); ++i)
    {
        current_point_ = gridPoints[i];
        gridPointData[i].point = current_point_;
        for (bool isFirst = true; const auto& [subproblem_name, worker]: subproblem_map)
        {
            if (isFirst) // Compute investment cost only once
            {
                isFirst = false;

                gridPointData[i].investment_cost = 0.0;
                for (const auto& [varName, value]: current_point_)
                {
                    gridPointData[i].investment_cost += investCostPerMwPerYear[varName] * value;
                }
            }

            PlainData::SubProblemData subproblem_data;
            auto solution = SolveSubproblem(subproblem_data, subproblem_name, worker);
            gridPointData[i].solution.push_back(solution);

            std::cout << "Subproblem: " << subproblem_name
                      << " Cost: " << subproblem_data.subproblem_cost << std::endl;

            gridPointData[i].operational_cost += subproblem_data.subproblem_cost;
        }

        gridPointData[i].overall_cost = gridPointData[i].investment_cost
                                        + gridPointData[i].operational_cost;
    }

    for (const auto& data: gridPointData)
    {
        std::cout << "Point: ";
        for (const auto& [key, value]: data.point)
        {
            std::cout << key << ": " << value << " ";
        }
        std::cout << std::endl
                  << " | Investment Cost: " << data.investment_cost << std::endl
                  << " | Operational Cost: " << data.operational_cost << std::endl
                  << " | Overall Cost: " << data.overall_cost << std::endl;
    }
}

std::vector<double> GridSearch::SolveSubproblem(PlainData::SubProblemData& subproblem_data,
                                                const std::string& name,
                                                const std::shared_ptr<SubproblemWorker>& worker)
{
    Timer subproblem_timer;
    worker->fix_to(current_point_);
    worker->solve(subproblem_data.lpstatus, ".", "", _writer);
    worker->get_value(subproblem_data.subproblem_cost);

    subproblem_data.subproblem_timer = subproblem_timer.elapsed();

    return worker->get_solution();
}

void GridSearch::launch()
{
    std::cout << "Launching grid search" << std::endl;

    InitializeProblems();

    SetGridPoints();

    Run();
}

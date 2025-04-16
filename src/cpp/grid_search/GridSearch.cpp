
#include "antares-xpansion/grid_search/GridSearch.h"

#include <regex>
#include <utility>

#include "antares-xpansion/benders/benders_core/WorkerMaster.h"
#include "antares-xpansion/helpers/Timer.h"
#include "antares-xpansion/xpansion_interfaces/LoggerUtils.h"

struct InvalidStructureFile: LogUtils::XpansionError<std::runtime_error>
{
    using LogUtils::XpansionError<std::runtime_error>::XpansionError;
};

GridSearch::GridSearch(Logger logger,
                       std::shared_ptr<Output::JsonWriter> writer,
                       std::filesystem::path path_to_data)
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
    ComputeWeights();

    for (const auto& problem: coupling_map)
    {
        const auto subProblemFilePath = GetSubproblemPath(problem.first);
        AddSubproblem(problem);
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

    std::set<int> monteCarloYearIDs;
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
        // Set Monte Carlo iterations number using the problem's name using regex
        std::regex regex("problem-([0-9]+)-([0-9]+)--optim-nb-([0-9]+)\\.mps");
        std::smatch match;
        if (std::regex_match(problem_name, match, regex))
        {
            monteCarloYearIDs.insert(std::stoi(match[1].str()));
        }
    }
    nbMonteCarloYears = monteCarloYearIDs.size() ? monteCarloYearIDs.size() : 1;
    summary.close();
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
                                                                   weights[kvp.first],
                                                                   "COIN",
                                                                   0,
                                                                   solver_log_manager_,
                                                                   _logger,
                                                                   ProblemsFormat::SAVED_FILE);
    subproblems.push_back(kvp.first);
}

void GridSearch::ComputeWeights()
{
    std::string line;
    auto filename(xpansionFolderPath / "weights.txt");
    std::ifstream file(filename);

    if (!file)
    {
        std::cout << "Set uniform weight " << std::endl;
        for (const auto& kvp: coupling_map)
        {
            weights[kvp.first] = 1.0 / nbMonteCarloYears;
            // std::cout << "Weight for " << kvp.first << " : " << weights[kvp.first] << std::endl;
        }
    }
    else
    {
        double weights_sum = -1;
        while (std::getline(file, line))
        {
            std::stringstream buffer(line);
            std::string problem_name;

            buffer >> problem_name;
            if (problem_name == WEIGHT_SUM_CST_STR)
            {
                buffer >> weights_sum;
            }
            else
            {
                buffer >> weights[problem_name];
            }
        }

        if (weights_sum == -1)
        {
            std::cerr << LOGLOCATION
                      << "ERROR : Invalid weight file format : Key WEIGHT_SUM not found."
                      << std::endl;
            std::exit(1);
        }
        else
        {
            for (const auto& kvp: weights)
            {
                weights[kvp.first] /= weights_sum;
                // std::cout << "Weight for " << kvp.first << " : " << weights[kvp.first] <<
                // std::endl;
            }
        }
    }
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

    int lineCount = 0;
    for (std::string line; std::getline(grid_file, line); lineCount++)
    {
    }
    gridPoints.resize(lineCount - 1);

    grid_file.clear();
    grid_file.seekg(0, std::ios::beg);
    while (std::getline(grid_file, line))
    {
        // remove \r and \n from line if present to avoid issues with different line endings
        line.erase(std::remove_if(line.begin(),
                                  line.end(),
                                  [](char c) { return c == '\r' || c == '\n'; }),
                   line.end());

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
            int pointIndex = 0;
            if (std::getline(ss, token, ','))
            {
                pointIndex = std::stoi(token) - 1;
            }
            int index = 0;
            while (std::getline(ss, token, ','))
            {
                point[header[index++]] = std::stod(token);
            }
            gridPoints[pointIndex] = point;
        }
    }
    grid_file.close();
    gridPointsData.resize(gridPoints.size());
}

void GridSearch::Run()
{
    for (int i = 0; i < gridPointsData.size(); ++i)
    {
        current_point_ = gridPoints[i];
        gridPointsData[i].point = current_point_;
        for (bool isFirst = true; const auto& [subproblem_name, worker]: subproblem_map)
        {
            if (isFirst) // Compute investment cost only once
            {
                isFirst = false;

                gridPointsData[i].investment_cost = 0.0;
                for (const auto& [varName, value]: current_point_)
                {
                    gridPointsData[i].investment_cost += investCostPerMwPerYear[varName] * value;
                }
            }

            PlainData::SubProblemData subproblem_data;
            auto solution = SolveSubproblem(subproblem_data, subproblem_name, worker);
            gridPointsData[i].solution.push_back(solution);

            std::cout << "Subproblem: " << subproblem_name
                      << " Cost: " << subproblem_data.subproblem_cost << std::endl;

            gridPointsData[i].operational_cost += subproblem_data.subproblem_cost;
        }

        gridPointsData[i].overall_cost = gridPointsData[i].investment_cost
                                         + gridPointsData[i].operational_cost;
    }

    for (const auto& data: gridPointsData)
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

    // Save the results to a file
    _writer->write_grid_points(gridPointsData);
    _writer->dump();
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

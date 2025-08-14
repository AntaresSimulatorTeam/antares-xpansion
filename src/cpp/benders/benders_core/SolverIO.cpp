
#include <antares-xpansion/benders/benders_core/SolverIO.h>
#include <antares-xpansion/core/ProblemFormatStream.h>
#include <fmt/format.h>

void SolverIO::write(SolverAbstract* solver, const std::filesystem::path& path) const
{
    switch (format_)
    {
    case ProblemsFormat::MPS_FILE:
        solver->write_prob_mps(path);
        break;
    case ProblemsFormat::SAVED_FILE:
        solver->save_prob(path);
        break;
    default:
        throw LogUtils::XpansionError<std::runtime_error>(
          fmt::format("Unknown file format {} for problem file: {}", format_, path.string()),
          LOGLOCATION);
    }
}

void SolverIO::read(SolverAbstract* solver, const std::filesystem::path& path) const
{
    switch (format_)
    {
    case ProblemsFormat::MPS_FILE:
        solver->read_prob_mps(path);
        break;
    case ProblemsFormat::SAVED_FILE:
        solver->restore_prob(path);
        break;
    default:
        throw LogUtils::XpansionError<std::runtime_error>(
          fmt::format("Unknown file format {} for problem file: {}", format_, path.string()),
          LOGLOCATION);
    }
}

void SolverIO::configure(const std::string& solver_name, ProblemsFormat format)
{
    solver_config_ = solver_name;
    format_ = format;
}

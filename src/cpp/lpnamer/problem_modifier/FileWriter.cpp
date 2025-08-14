
#include "antares-xpansion/lpnamer/problem_modifier/FileWriter.h"

#include <fmt/format.h>
#include <utility>

#include "antares-xpansion/core/ProblemFormatStream.h"
#include "antares-xpansion/lpnamer/problem_modifier/IProblemWriter.h"
#include "antares-xpansion/lpnamer/problem_modifier/LinkProblemsGenerator.h"

void FileWriter::Write_problem(Problem* in_prblm, const std::filesystem::path& output_file)
{
    switch (format_)
    {
    case ProblemsFormat::MPS_FILE:
        in_prblm->write_prob_mps(output_file);
        break;
    case ProblemsFormat::SAVED_FILE:
        in_prblm->save_prob(output_file);
        break;
    default:
        throw std::runtime_error(fmt::format("Unknown ProblemsFormat: {}", format_));
    }
}

FileWriter::FileWriter(ProblemsFormat format):
    format_(format)
{
}

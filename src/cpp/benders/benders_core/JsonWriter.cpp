#include "JsonWriter.h"
#include "config.h"

namespace clock_utils
{
    std::string timeToStr(const std::time_t &time_p)
    {
        struct tm local_time;
        localtime_platform(time_p, local_time);
        // localtime_r(&time_p, &local_time); // Compliant
        const char *FORMAT = "%d-%m-%Y %H:%M:%S";
        char buffer_l[100];
        strftime(buffer_l, sizeof(buffer_l), FORMAT, &local_time);
        std::string strTime_l(buffer_l);

        return strTime_l;
    }
}
namespace Output
{
    JsonWriter::JsonWriter(std::shared_ptr<Clock> p_clock, const std::string &json_filename) : _clock(p_clock), _filename(json_filename) {}

    void JsonWriter::initialize(const BendersOptions &options)
    {
        write_failure();
        dump();

        write_options(options);
        updateBeginTime();
    }

    void JsonWriter::updateBeginTime()
    {
        _start_time = _clock->getTime();
        _output["begin"] = clock_utils::timeToStr(_start_time);
    }

    void JsonWriter::updateEndTime()
    {
        _end_time = _clock->getTime();
        _output["end"] = clock_utils::timeToStr(_end_time);
    }

    void JsonWriter::write_options(BendersOptions const &bendersOptions_p)
    {
// Options
#define BENDERS_OPTIONS_MACRO(name__, type__, default__) _output["options"][#name__] = bendersOptions_p.name__;
#include "BendersOptions.hxx"
#undef BENDERS_OPTIONS_MACRO
    }

    void JsonWriter::write_iteration(const IterationsData &iterations_data)
    {
        _output["nbWeeks"] = iterations_data.nbWeeks_p;

        // Iterations
        size_t iterCnt_l(0);
        for (const auto &iter : iterations_data.iters)
        {
            ++iterCnt_l;

            std::string strIterCnt_l(std::to_string(iterCnt_l));
            _output["iterations"][strIterCnt_l]["duration"] = iter.time;
            _output["iterations"][strIterCnt_l]["lb"] = iter.lb;
            _output["iterations"][strIterCnt_l]["ub"] = iter.ub;
            _output["iterations"][strIterCnt_l]["best_ub"] = iter.best_ub;
            _output["iterations"][strIterCnt_l]["optimality_gap"] = iter.optimality_gap;
            _output["iterations"][strIterCnt_l]["relative_gap"] = iter.relative_gap;
            _output["iterations"][strIterCnt_l]["investment_cost"] = iter.investment_cost;
            _output["iterations"][strIterCnt_l]["operational_cost"] = iter.operational_cost;
            _output["iterations"][strIterCnt_l]["overall_cost"] = iter.overall_cost;

            Json::Value vectCandidates_l(Json::arrayValue);
            for (const auto &candidate : iter.candidates)
            {
                Json::Value candidate_l;
                candidate_l["name"] = candidate.name;
                candidate_l["invest"] = candidate.invest;
                candidate_l["min"] = candidate.min;
                candidate_l["max"] = candidate.max;
                vectCandidates_l.append(candidate_l);
            }
            _output["iterations"][strIterCnt_l]["candidates"] = vectCandidates_l;
        }

        // solution
        _output["solution"]["iteration"] = iterations_data.solution_data.best_it;
        _output["solution"]["investment_cost"] = iterations_data.solution_data.solution.investment_cost;
        _output["solution"]["operational_cost"] = iterations_data.solution_data.solution.operational_cost;
        _output["solution"]["overall_cost"] = iterations_data.solution_data.solution.overall_cost;

        for (const auto &candidate : iterations_data.solution_data.solution.candidates)
        {
            _output["solution"]["values"][candidate.name] = candidate.invest;
        }

        _output["solution"]["optimality_gap"] = iterations_data.solution_data.solution.optimality_gap;
        _output["solution"]["relative_gap"] = iterations_data.solution_data.solution.relative_gap;

        _output["solution"]["stopping_criterion"] = criterion_to_string(
                iterations_data.solution_data.stopping_criterion);
        _output["solution"]["status"] = status_from_criterion(iterations_data.solution_data.stopping_criterion);
    }

    std::string JsonWriter::criterion_to_string(
            const StoppingCriterion stopping_criterion) const {
        switch (stopping_criterion) {
            case StoppingCriterion::absolute_gap:
                return "OPTIMAL";

            case StoppingCriterion::relative_gap:
                return "OPTIMAL";

            case StoppingCriterion::max_iteration:
                return "OPTIMAL";

            case StoppingCriterion::timelimit:
                return "OPTIMAL";

            default:
                return "ERROR";
        }
    }

    std::string JsonWriter::status_from_criterion(const StoppingCriterion stopping_criterion) const  {
        switch (stopping_criterion) {
            case StoppingCriterion::absolute_gap:
                return "OPTIMAL";

            case StoppingCriterion::relative_gap:
                return "relative gap";

            case StoppingCriterion::max_iteration:
                return "maximum iterations";

            case StoppingCriterion::timelimit:
                return "timelimit";

            default:
                return "error";
        }
    }

    void JsonWriter::update_solution(const SolutionData &solution_data)
    {
        _output["nbWeeks"] = solution_data.nbWeeks_p;

        _output["solution"]["investment_cost"] = solution_data.solution.investment_cost;
        _output["solution"]["operational_cost"] = solution_data.solution.operational_cost;
        _output["solution"]["overall_cost"] = solution_data.solution.overall_cost;
        _output["solution"]["lb"] = solution_data.solution.lb;
        _output["solution"]["ub"] = solution_data.solution.ub;
        _output["solution"]["optimality_gap"] = solution_data.solution.optimality_gap;
        _output["solution"]["relative_gap"] = solution_data.solution.relative_gap;

        _output["solution"]["problem_status"] = solution_data.problem_status;
        for (const auto &candidate : solution_data.solution.candidates)
        {
            _output["solution"]["values"][candidate.name] = candidate.invest;
        }
        updateEndTime();
    }

    /*!
     *  \brief write a json output with a failure status in solution. If optimization process exits before it ends, this failure will be available as an output.
     *
     */
    void JsonWriter::write_failure()
    {
        _output["solution"]["problem_status"] = "ERROR";
    }

    /*!
     *  \brief write the json data into a file
     */
    void JsonWriter::dump()
    {
        // Antares
        _output["antares"]["version"] = ANTARES_VERSION_TAG;

        // Xpansion
        _output["antares_xpansion"]["version"] = PROJECT_VER;

        // Time
        _output["duration"] = clock_utils::timeToStr(_end_time - _start_time);

        std::ofstream jsonOut_l(_filename);
        if (jsonOut_l)
        {
            // Output
            jsonOut_l << _output << std::endl;
        }
        else
        {
            std::cout << "Impossible d'ouvrir le fichier json " << _filename << std::endl;
        }
    }

    void JsonWriter::end_writing(const IterationsData &iterations_data)
    {
        updateEndTime();
        write_iteration(iterations_data);
        dump();
    }
}
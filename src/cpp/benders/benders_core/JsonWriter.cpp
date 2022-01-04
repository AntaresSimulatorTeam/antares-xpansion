#include "JsonWriter.h"
#include "config.h"
namespace Output
{
    JsonWriter::JsonWriter() : _time(TimeUtil())
    {
    }

    JsonWriter::~JsonWriter()
    {
    }

    void JsonWriter::updateBeginTime()
    {
        _time.updateBeginTime();
        _output["begin"] = _time.getBegin();
    }

    void JsonWriter::updateEndTime()
    {
        _time.updateEndTime();
        _output["end"] = _time.getEnd();
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
        for (auto masterDataPtr_l : iterations_data.bendersTrace_p)
        {
            ++iterCnt_l;
            if (masterDataPtr_l->_valid)
            {

                std::string strIterCnt_l(std::to_string(iterCnt_l));
                _output["iterations"][strIterCnt_l]["duration"] = masterDataPtr_l->_time;
                _output["iterations"][strIterCnt_l]["lb"] = masterDataPtr_l->_lb;
                _output["iterations"][strIterCnt_l]["ub"] = masterDataPtr_l->_ub;
                _output["iterations"][strIterCnt_l]["best_ub"] = masterDataPtr_l->_bestub;
                _output["iterations"][strIterCnt_l]["optimality_gap"] = masterDataPtr_l->_bestub - masterDataPtr_l->_lb;
                _output["iterations"][strIterCnt_l]["relative_gap"] = (masterDataPtr_l->_bestub - masterDataPtr_l->_lb) / masterDataPtr_l->_bestub;
                _output["iterations"][strIterCnt_l]["investment_cost"] = masterDataPtr_l->_invest_cost;
                _output["iterations"][strIterCnt_l]["operational_cost"] = masterDataPtr_l->_operational_cost;
                _output["iterations"][strIterCnt_l]["overall_cost"] = masterDataPtr_l->_invest_cost + masterDataPtr_l->_operational_cost;

                Json::Value vectCandidates_l(Json::arrayValue);
                for (auto pairNameValue_l : masterDataPtr_l->get_point())
                {
                    Json::Value candidate_l;
                    candidate_l["name"] = pairNameValue_l.first;
                    candidate_l["invest"] = pairNameValue_l.second;
                    candidate_l["min"] = masterDataPtr_l->get_min_invest()[pairNameValue_l.first];
                    candidate_l["max"] = masterDataPtr_l->get_max_invest()[pairNameValue_l.first];
                    vectCandidates_l.append(candidate_l);
                }
                _output["iterations"][strIterCnt_l]["candidates"] = vectCandidates_l;
            }
        }

        // solution
        size_t bestItIndex_l = iterations_data.best_it - 1;
        _output["solution"]["iteration"] = iterations_data.best_it;
        if (bestItIndex_l >= 0 && bestItIndex_l < iterations_data.bendersTrace_p.size())
        {
            _output["solution"]["investment_cost"] = iterations_data.bendersTrace_p[bestItIndex_l].get()->_invest_cost;
            _output["solution"]["operational_cost"] = iterations_data.bendersTrace_p[bestItIndex_l].get()->_operational_cost;
            _output["solution"]["overall_cost"] = iterations_data.bendersTrace_p[bestItIndex_l].get()->_invest_cost + iterations_data.bendersTrace_p[bestItIndex_l].get()->_operational_cost;

            for (auto pairNameValue_l : iterations_data.bendersTrace_p[bestItIndex_l]->get_point())
            {
                _output["solution"]["values"][pairNameValue_l.first] = pairNameValue_l.second;
            }
        }

        double abs_gap_l = iterations_data.best_ub - iterations_data.lb;
        double rel_gap_l = abs_gap_l / iterations_data.best_ub;
        _output["solution"]["optimality_gap"] = abs_gap_l;
        _output["solution"]["relative_gap"] = rel_gap_l;
        if ((abs_gap_l <= iterations_data.min_abs_gap) || (rel_gap_l <= iterations_data.min_rel_gap))
        {
            _output["solution"]["problem_status"] = "OPTIMAL";
        }
        else if (iterations_data.max_iter != -1 && iterations_data.it > iterations_data.max_iter)
        {
            _output["solution"]["problem_status"] = "MAX ITERATIONS";
        }
        else
        {
            _output["solution"]["problem_status"] = "ERROR";
        }
    }

    void JsonWriter::update_solution(const SolutionData &solution_data)
    {
        _output["nbWeeks"] = solution_data.nbWeeks_p;

        _output["solution"]["investment_cost"] = solution_data.investCost_p;
        _output["solution"]["operational_cost"] = solution_data.operationalCost_p;
        _output["solution"]["overall_cost"] = solution_data.overallCost_p;
        _output["solution"]["lb"] = solution_data.lb_p;
        _output["solution"]["ub"] = solution_data.ub_p;
        _output["solution"]["optimality_gap"] = solution_data.ub_p - solution_data.lb_p;
        _output["solution"]["relative_gap"] = (solution_data.ub_p - solution_data.lb_p) / solution_data.ub_p;
        if (solution_data.optimality_p)
        {
            _output["solution"]["problem_status"] = "OPTIMAL";
        }
        else
        {
            _output["solution"]["problem_status"] = "ERROR";
        }
        for (auto pairNameValue_l : solution_data.solution_p)
        {
            _output["solution"]["values"][pairNameValue_l.first] = pairNameValue_l.second;
        }
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
        _output["duration"] = _time.getDuration();

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

    void JsonWriter::initialize(BendersOptions options)
    {
        _filename = options.JSON_FILE;
        write_failure();
        dump();

        write_options(options);
        updateBeginTime();
    }

    void JsonWriter::end_writing(const IterationsData &iterations_data)
    {
        updateEndTime();
        write_iteration(iterations_data);
        dump();
    }
}
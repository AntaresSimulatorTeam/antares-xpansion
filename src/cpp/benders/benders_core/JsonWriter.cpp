#include "JsonWriter.h"
#include "config.h"

JsonWriter::JsonWriter() : OutputWriter()
{
}

JsonWriter::~JsonWriter()
{
}

void JsonWriter::updateBeginTime()
{
    OutputWriter::updateBeginTime();
    _output["begin"] = getBegin();
}

void JsonWriter::updateEndTime()
{
    OutputWriter::updateEndTime();
    _output["end"] = getEnd();
}

void JsonWriter::write(BendersOptions const &bendersOptions_p)
{
// Options
#define BENDERS_OPTIONS_MACRO(name__, type__, default__) _output["options"][#name__] = bendersOptions_p.name__;
#include "BendersOptions.hxx"
#undef BENDERS_OPTIONS_MACRO
}

void JsonWriter::write(int const &nbWeeks_p, BendersTrace const &bendersTrace_p,
                       BendersData const &bendersData_p, double const &min_abs_gap, double const &min_rel_gap, double const &max_iter)
{
    _output["nbWeeks"] = nbWeeks_p;

    // Iterations
    size_t iterCnt_l(0);
    for (auto masterDataPtr_l : bendersTrace_p)
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
    size_t bestItIndex_l = bendersData_p.best_it - 1;
    _output["solution"]["iteration"] = bendersData_p.best_it;
    if (bestItIndex_l >= 0 && bestItIndex_l < bendersTrace_p.size())
    {
        _output["solution"]["investment_cost"] = bendersTrace_p[bestItIndex_l].get()->_invest_cost;
        _output["solution"]["operational_cost"] = bendersTrace_p[bestItIndex_l].get()->_operational_cost;
        _output["solution"]["overall_cost"] = bendersTrace_p[bestItIndex_l].get()->_invest_cost + bendersTrace_p[bestItIndex_l].get()->_operational_cost;

        for (auto pairNameValue_l : bendersTrace_p[bestItIndex_l]->get_point())
        {
            _output["solution"]["values"][pairNameValue_l.first] = pairNameValue_l.second;
        }
    }

    double abs_gap_l = bendersData_p.best_ub - bendersData_p.lb;
    double rel_gap_l = abs_gap_l / bendersData_p.best_ub;
    _output["solution"]["optimality_gap"] = abs_gap_l;
    _output["solution"]["relative_gap"] = rel_gap_l;
    if ((abs_gap_l <= min_abs_gap) || (rel_gap_l <= min_rel_gap))
    {
        _output["solution"]["problem_status"] = "OPTIMAL";
    }
    else if (max_iter != -1 && bendersData_p.it > max_iter)
    {
        _output["solution"]["problem_status"] = "MAX ITERATIONS";
    }
    else
    {
        _output["solution"]["problem_status"] = "ERROR";
    }
}

void JsonWriter::write(int nbWeeks_p,
                       double const &lb_p, double const &ub_p,
                       double const &investCost_p,
                       double const &operationalCost_p,
                       double const &overallCost_p,
                       Point const &solution_p,
                       bool const &optimality_p)
{
    _output["nbWeeks"] = nbWeeks_p;

    _output["solution"]["investment_cost"] = investCost_p;
    _output["solution"]["operational_cost"] = operationalCost_p;
    _output["solution"]["overall_cost"] = overallCost_p;
    _output["solution"]["lb"] = lb_p;
    _output["solution"]["ub"] = ub_p;
    _output["solution"]["optimality_gap"] = ub_p - lb_p;
    _output["solution"]["relative_gap"] = (ub_p - lb_p) / ub_p;
    if (optimality_p)
    {
        _output["solution"]["problem_status"] = "OPTIMAL";
    }
    else
    {
        _output["solution"]["problem_status"] = "ERROR";
    }
    for (auto pairNameValue_l : solution_p)
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
    _output["duration"] = getDuration();

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

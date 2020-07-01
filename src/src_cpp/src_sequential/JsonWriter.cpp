#include "JsonWriter.h"
#include "config.h"

namespace
{
    std::string timeToStr(std::time_t* time_p)
    {
        char buffer_l[100];
        strftime(buffer_l, sizeof(buffer_l), "%d-%m-%Y %H:%M:%S", std::localtime(time_p));
        std::string strTime_l(buffer_l);

        return strTime_l;
    }
}


JsonWriter::JsonWriter() :
    _beginTime(std::time(0)),
    _endTime(std::time(0))
{
}


JsonWriter::~JsonWriter() {
}

void JsonWriter::updateBeginTime()
{
    _beginTime = std::time(0);
    _output["begin"] = getBegin();
}

void JsonWriter::updateEndTime()
{
    _endTime =  std::time(0);
    _output["end"] = getEnd();
}

std::string JsonWriter::getBegin()
{
    return timeToStr(&_beginTime);
}

std::string JsonWriter::getEnd()
{
    return timeToStr(&_endTime);
}

double JsonWriter::getDuration()
{
    return std::difftime(_endTime, _beginTime);
}

void JsonWriter::write(BendersOptions const & bendersOptions_p)
{
    //Options
    #define BENDERS_OPTIONS_MACRO(name__, type__, default__) _output["options"][#name__] = bendersOptions_p.name__;
    #include "BendersOptions.hxx"
    #undef BENDERS_OPTIONS_MACRO
}


void JsonWriter::write(int const & nbWeeks_p, BendersTrace const & bendersTrace_p,
                       BendersData const & bendersData_p)
{
    _output["nbWeeks"] = nbWeeks_p;

    //Iterations
    size_t iterCnt_l(0);
    for(auto masterDataPtr_l : bendersTrace_p)
    {
        ++iterCnt_l;

        std::string strIterCnt_l(std::to_string(iterCnt_l));
        _output["iterations"][strIterCnt_l]["duration"] = masterDataPtr_l->_time;
        _output["iterations"][strIterCnt_l]["lb"] = masterDataPtr_l->_lb;
        _output["iterations"][strIterCnt_l]["ub"] = masterDataPtr_l->_ub;
        _output["iterations"][strIterCnt_l]["gap"] = masterDataPtr_l->_ub - masterDataPtr_l->_lb;
        _output["iterations"][strIterCnt_l]["relative_gap"] = (masterDataPtr_l->_ub - masterDataPtr_l->_lb) / masterDataPtr_l->_ub;
        _output["iterations"][strIterCnt_l]["investment_cost"] = masterDataPtr_l->_invest_cost;
        _output["iterations"][strIterCnt_l]["operational_cost"] = masterDataPtr_l->_operational_cost;
        _output["iterations"][strIterCnt_l]["overall_cost"] = masterDataPtr_l->_invest_cost + masterDataPtr_l->_operational_cost;


        Json::Value vectCandidates_l (Json::arrayValue);
        for(auto pairNameValue_l : masterDataPtr_l->get_point())
        {
            Json::Value candidate_l;
            candidate_l["name"] = pairNameValue_l.first;
            candidate_l["invest"] = pairNameValue_l.second;
            vectCandidates_l.append(candidate_l);
        }
        _output["iterations"][strIterCnt_l]["candidates"] = vectCandidates_l;
    }

    //solution
    size_t bestItIndex_l = bendersData_p.best_it - 1;
    _output["solution"]["iteration"] = bendersData_p.best_it;
    _output["solution"]["investment_cost"] = bendersTrace_p[bestItIndex_l].get()->_invest_cost;
    _output["solution"]["operational_cost"] = bendersTrace_p[bestItIndex_l].get()->_operational_cost;
    _output["solution"]["overall_cost"] = bendersTrace_p[bestItIndex_l].get()->_invest_cost + bendersTrace_p[bestItIndex_l].get()->_operational_cost;
    double gap_l = bendersTrace_p.back().get()->_bestub - bendersTrace_p.back().get()->_lb;
    _output["solution"]["gap"] = gap_l;
    _output["solution"]["optimality"] = (gap_l < 1e-03);
    for(auto pairNameValue_l : bendersTrace_p[bestItIndex_l]->get_point())
    {
        _output["solution"]["values"][pairNameValue_l.first] = pairNameValue_l.second;
    }
}

void JsonWriter::write(int nbWeeks_p, double const & lb_p, double const & ub_p, double const & investCost_p, Point const & solution_p, bool const & optimality_p)
{
    _output["nbWeeks"] = nbWeeks_p;

    _output["solution"]["investment_cost"] = investCost_p;
    _output["solution"]["lb"] = lb_p;
    _output["solution"]["ub"] = ub_p;
    _output["solution"]["gap"] = ub_p - lb_p;
    _output["solution"]["optimality"] = optimality_p;
    for(auto pairNameValue_l : solution_p)
    {
        _output["solution"]["values"][pairNameValue_l.first] = pairNameValue_l.second;
    }
}

void JsonWriter::dump(std::string const & filename_p)
{
    //Antares
    _output["antares"]["version"] = "unknown";
    _output["antares"]["name"] = PROJECT_VER;

    //Xpansion
    _output["antares_xpansion"]["version"] = PROJECT_VER;

    //Time
    _output["duration"] = getDuration();

    std::ofstream jsonOut_l(filename_p);
    if(jsonOut_l)
    {
        //Output
        jsonOut_l << _output << std::endl;
        jsonOut_l.close();
    }
    else
    {
		std::cout << "Impossible d'ouvrir le fichier json " << filename_p << std::endl;
	}
}
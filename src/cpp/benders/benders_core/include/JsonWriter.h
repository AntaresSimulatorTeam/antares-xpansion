
#pragma once

#include "OutputWriter.h"
#include "BendersOptions.h"
#include "Clock.h"
#include "Timer.h"

#include <json/writer.h>

inline void localtime_platform(const std::time_t &time_p, struct tm &local_time)
{
#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
    localtime_s(&local_time, &time_p);
#else // defined(__unix__) || (__APPLE__)
    localtime_r(&time_p, &local_time);
#endif
}
namespace clock_utils
{
    std::string timeToStr(const std::time_t &time_p);
}
namespace Output
{

    // string constantes
    const std::string
        ANTARES_C("antares"),
        VERSION_C("version"),
        ANTARES_XPANSION_C("antares_xpansion"),
        BEGIN_C("begin"),
        END_C("end"),
        DURATION_C("duration"),
        ITERATIONS_C("iterations"),
        BEST_UB_C("best_ub"),
        CANDIDATES_C("candidates"),
        INVEST_C("invest"),
        MAX_C("max"),
        MIN_C("min"),
        NAME_C("name"),
        INVESTMENT_COST_C("investment_cost"),
        LB_C("lb"),
        OPERATIONAL_COST_C("operational_cost"),
        OPTIMALITY_GAP_C("optimality_gap"),
        OVERALL_COST_C("overall_cost"),
        RELATIVE_GAP_C("relative_gap"),
        UB_C("ub"),
        NBWEEKS_C("nbWeeks"),
        OPTIONS_C("options"),
        SOLUTION_C("solution"),
        ITERATION_C("iteration"),
        PROBLEM_STATUS_C("problem_status"),
        STATUS_OPTIMAL_C("OPTIMAL"),
        STATUS_ERROR_C("ERROR"),
        VALUES_C("values"),
        STOPPING_CRITERION_C("stopping_criterion"),
        CRIT_ABSOLUTE_GAP_C("absolute gap"),
        CRIT_RELATIVE_GAP_C("relative gap"),
        CRIT_MAX_ITER_C("maximum iterations"),
        CRIT_TIMELIMIT_C("timelimit"),
        STOP_ERROR_C("error");
    /*!
     * \class JsonWriter
     * \brief JsonWriter class to describe the execuion session of an antares xpansion optimization in a json file
     */
    class JsonWriter : public OutputWriter
    {
    private:
        std::shared_ptr<Clock> _clock;
        std::string _filename;
        // attributes of the optimization execution
        Json::Value _output;

        /*!
         *  \brief updates the execution begin time
         */
        virtual void updateBeginTime();

        /*!
         *  \brief updates the end of execution time
         */
        virtual void updateEndTime();
        virtual void write_iterations(const IterationsData &iterations_data);

        /*!
         *  \brief write an a priori errored json output, overwritten if optimization ends
         */
        std::string criterion_to_string(const StoppingCriterion stopping_criterion) const;
        std::string status_from_criterion(const StoppingCriterion stopping_criterion) const;

    public:
        /*!
         *  \brief JsonWriter default constructor
         */
        JsonWriter() = delete;

        JsonWriter(std::shared_ptr<Clock> p_clock, const std::string &json_filename);

        /*!
         *  \brief destructor of class JsonWriter
         */
        virtual ~JsonWriter() = default;

        /*!
         *  \brief saves the options of the benders algorithm to be later written to the json file
         *
         *  \param bendersOptions_p : set of options used for the optimization
         */
        virtual void write_options(BendersOptions const &bendersOptions_p);
        virtual void update_solution(const SolutionData &solution_data);

        /*!
         *  \brief write the json data into a file
         */
        virtual void dump();

        /*!
         * \brief initialize outputs
         * \param options : set of options used for the optimization
         */
        void initialize(const BendersOptions &options);

        void end_writing(const IterationsData &iterations_data);
    };
}
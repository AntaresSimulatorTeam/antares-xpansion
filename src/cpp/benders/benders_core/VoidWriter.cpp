#include "VoidWriter.h"
#include "config.h"
namespace Output
{
    VoidWriter::VoidWriter()
    {
    }

    VoidWriter::~VoidWriter()
    {
    }
    void VoidWriter::updateBeginTime()
    {
    }

    void VoidWriter::updateEndTime()
    {
    }

    void VoidWriter::write_options(BendersOptions const &bendersOptions_p)
    {
    }

    void VoidWriter::write_iteration(const IterationsInfo &iterations_info)
    {
    }

    void VoidWriter::update_solution(int nbWeeks_p,
                                     double const &lb_p, double const &ub_p,
                                     double const &investCost_p,
                                     double const &operationalCost_p,
                                     double const &overallCost_p,
                                     Point const &solution_p,
                                     bool const &optimality_p)
    {
    }

    /*!
     *  \brief write a json output with a failure status in solution. If optimization process exits before it ends, this failure will be available as an output.
     *
     */
    void VoidWriter::write_failure()
    {
    }

    /*!
     *  \brief write the json data into a file
     */
    void VoidWriter::dump()
    {
    }
    void VoidWriter::initialize(BendersOptions options)
    {
    }

    void VoidWriter::end_writing(const IterationsInfo &iterations_info)
    {
    }
}
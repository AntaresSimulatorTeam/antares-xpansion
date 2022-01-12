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

    void VoidWriter::write_iteration(const IterationsData &iterations_data)
    {
    }

    void VoidWriter::update_solution(const SolutionData &solution_data)
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
    void VoidWriter::initialize(const BendersOptions &options)
    {
    }

    void VoidWriter::end_writing(const IterationsData &iterations_data)
    {
    }
}
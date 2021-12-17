#pragma once

#include "BendersBase.h"
#include "common.h"
#include "core/ILogger.h"

/*!
 * \class BendersSequential
 * \brief Class use run the benders algorithm in sequential
 */
class BendersSequential : public BendersBase
{
public:
	explicit BendersSequential(BendersOptions const &options, Logger &logger, Writer writer);
	virtual ~BendersSequential();
	virtual void launch();
	void build_cut();
	void initialise_problems(const CouplingMap &problem_list);

protected:
	virtual void free();
	virtual void run();
};

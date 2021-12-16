#pragma once

#include "BendersBase.h"
#include "common.h"
#include "core/ILogger.h"


/*!
* \class Benders
* \brief Class use run the benders algorithm in sequential
*/
class Benders: public BendersBase {
public:
    explicit Benders(BendersOptions const &options, Logger &logger);
	virtual ~Benders();

	virtual void free();

	void build_cut();
	void run( CouplingMap const &problem_list);
private:

    void doRun();
    void initialise_problems(const CouplingMap &problem_list);


};

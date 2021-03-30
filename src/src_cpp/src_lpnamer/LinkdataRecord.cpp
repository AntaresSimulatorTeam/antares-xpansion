#include "StudyUpdater.h"



LinkdataRecord::LinkdataRecord(bool modernAntaresVersion_p):
modernAntaresVersion_(modernAntaresVersion_p),
directCapacity_(0),
indirectCapacity_(0),
directHurdlesCost_(0),
indirectHurdlesCost_(0),
impedances_(0),
loopFlow_(0),
pShiftMin_(0),
pShiftMax_(0)
{
}


LinkdataRecord::LinkdataRecord(double directCapacity_p, double indirectCapacity_p,
    double directHurdlesCost_p, double indirectHurdlesCost_p, double impedances_p,
    double loopFlow_p, double pShiftMin_p, double pShiftMax_p):
modernAntaresVersion_(true),
directCapacity_(directCapacity_p),
indirectCapacity_(indirectCapacity_p),
directHurdlesCost_(directHurdlesCost_p),
indirectHurdlesCost_(indirectHurdlesCost_p),
impedances_(impedances_p),
loopFlow_(loopFlow_p),
pShiftMin_(pShiftMin_p),
pShiftMax_(pShiftMax_p)
{
}


LinkdataRecord::LinkdataRecord(double directCapacity_p, double indirectCapacity_p,
    double directHurdlesCost_p, double indirectHurdlesCost_p, double impedances_p):
modernAntaresVersion_(false),
directCapacity_(directCapacity_p),
indirectCapacity_(indirectCapacity_p),
directHurdlesCost_(directHurdlesCost_p),
indirectHurdlesCost_(indirectHurdlesCost_p),
impedances_(impedances_p),
loopFlow_(0),
pShiftMin_(0),
pShiftMax_(0)
{
}

void LinkdataRecord::reset()
{
    directCapacity_ = 0;
    indirectCapacity_ = 0;
    directHurdlesCost_ = 0;
    indirectHurdlesCost_ = 0;
    impedances_ = 0;
    loopFlow_ = 0;
    pShiftMin_ = 0;
    pShiftMax_ = 0;
}


void LinkdataRecord::fillFromRow(std::string const &  line_p)
{
    std::istringstream iss_l(line_p);

    iss_l >> directCapacity_;
    iss_l >> indirectCapacity_;
    iss_l >> directHurdlesCost_;
    iss_l >> indirectHurdlesCost_;
    iss_l >> impedances_;

    if(modernAntaresVersion_)
    {
        iss_l >> loopFlow_;
        iss_l >> pShiftMin_;
        iss_l >> pShiftMax_;
    }
}


void LinkdataRecord::updateCapacities(double directCapacity_p, double indirectCapacity_p)
{
    directCapacity_ = directCapacity_p;
    indirectCapacity_ = indirectCapacity_p;
}


std::string LinkdataRecord::to_row(std::string const & sep_p) const
{
    std::string row_l = std::to_string(directCapacity_) + sep_p + std::to_string(indirectCapacity_)
                    + sep_p + std::to_string(directHurdlesCost_) + sep_p + std::to_string(indirectHurdlesCost_) + sep_p + std::to_string(impedances_);
    if(modernAntaresVersion_)
    {
        row_l += sep_p + std::to_string(loopFlow_) + sep_p + std::to_string(pShiftMin_) + sep_p + std::to_string(pShiftMax_);
    }

    return row_l;
}

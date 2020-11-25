#include "StudyUpdater.h"



LinkdataRecord::LinkdataRecord(bool modernAntaresVersion_p):
modernAntaresVersion_(modernAntaresVersion_p),
directCapacity_(0),
indirectCapacity_(0),
field3_(0),
field4_(0),
field5_(0),
field6_(0),
field7_(0),
field8_(0)
{
}


LinkdataRecord::LinkdataRecord(double directCapacity_p, double indirectCapacity_p,
            int field3_p, int field4_p, int field5_p,
            int field6_p, int field7_p, int field8_p):
modernAntaresVersion_(true),
directCapacity_(directCapacity_p),
indirectCapacity_(indirectCapacity_p),
field3_(field3_p),
field4_(field4_p),
field5_(field5_p),
field6_(field6_p),
field7_(field7_p),
field8_(field8_p)
{
}


LinkdataRecord::LinkdataRecord(double directCapacity_p, double indirectCapacity_p,
            int field3_p, int field4_p, int field5_p):
modernAntaresVersion_(false),
directCapacity_(directCapacity_p),
indirectCapacity_(indirectCapacity_p),
field3_(field3_p),
field4_(field4_p),
field5_(field5_p),
field6_(0),
field7_(0),
field8_(0)
{
}

void LinkdataRecord::reset()
{
    directCapacity_ = 0;
    indirectCapacity_ = 0;
    field3_ = 0;
    field4_ = 0;
    field5_ = 0;
    field6_ = 0;
    field7_ = 0;
    field8_ = 0;
}


void LinkdataRecord::fillFromRow(std::string const &  line_p)
{
    std::istringstream iss_l(line_p);

    iss_l >> directCapacity_;
    iss_l >> indirectCapacity_;
    iss_l >> field3_;
    iss_l >> field4_;
    iss_l >> field5_;

    if(modernAntaresVersion_)
    {
        iss_l >> field6_;
        iss_l >> field7_;
        iss_l >> field8_;
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
                    + sep_p + std::to_string(field3_) + sep_p + std::to_string(field4_) + sep_p + std::to_string(field5_);
    if(modernAntaresVersion_)
    {
        row_l += sep_p + std::to_string(field6_) + sep_p + std::to_string(field7_) + sep_p + std::to_string(field8_);
    }

    return row_l;
}

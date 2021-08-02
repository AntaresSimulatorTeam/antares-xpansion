//

#ifndef ANTARESXPANSION_INTERCO_H
#define ANTARESXPANSION_INTERCO_H

#include "Candidate.h"


struct IntercoData {
    std::string name;
};

class Interco {

public:

    Interco(const std::string &name);

    std::string _name;
    LinkProfile _already_installed_profile;
    std::vector<Candidate> _candidateList;
};


#endif //ANTARESXPANSION_INTERCO_H

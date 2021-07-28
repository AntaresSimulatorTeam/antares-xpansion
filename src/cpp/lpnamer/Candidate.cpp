//

#include "Candidate.h"

std::set<std::string> Candidate::str_fields = std::set<std::string>({
                                                                             "name",
                                                                             "investment_type",
                                                                             "link",
                                                                             "linkor",
                                                                             "linkex",
                                                                             "link-profile",
                                                                             "already-installed-link-profile"
                                                                     });

std::set<std::string> Candidate::dbl_fields = std::set<std::string>({
                                                                             "annual-cost-per-mw",
                                                                             "max-investment",
                                                                             "unit-size",
                                                                             "max-units",
                                                                             "already-installed-capacity"
                                                                     });
#include "antares-xpansion/helpers/OptionsParser.h"

#include <iostream>

namespace po = boost::program_options;

void OptionsParser::Parse(unsigned int argc, const char* const* argv) {
  if (argc == 0) {
    throw OptionsParser::InvalidNumberOfArgumentsPassedToParser(argc, exe_name_,
                                                                LOGLOCATION);
  }
  if (argv == nullptr) {
    throw OptionsParser::NullArgumentsValues(exe_name_, LOGLOCATION);
  }
  po::variables_map opts;
  po::store(po::parse_command_line(argc, argv, desc_), opts);

  if (opts.count("help")) {
    std::cout << desc_ << std::endl;
    std::exit(0);
  }

  po::notify(opts);
}

#include "OptionsParser.h"

#include <iostream>
namespace po = boost::program_options;

void OptionsParser::Parse(int argc, char** argv) {
  po::variables_map opts;
  po::store(po::parse_command_line(argc, argv, desc_), opts);

  if (opts.count("help")) {
    std::cout << desc_ << std::endl;
    return std::exit(0);
  }

  po::notify(opts);
}

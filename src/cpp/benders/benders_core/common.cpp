#include "antares-xpansion/benders/benders_core/common.h"

/*!
 *  \brief Return the distance between two point using 2-norm
 *
 *  \param x0 : first point
 *
 *  \param x1 : second point
 */
double norm_point(Point const &x0, Point const &x1) {
  double result(0);
  for (auto &kvp : x0) {
    result += (x0.find(kvp.first)->second - x1.find(kvp.first)->second) *
              (x0.find(kvp.first)->second - x1.find(kvp.first)->second);
  }
  result = std::sqrt(result);
  return result;
}

/*!
 *  \brief How to call for the algorithm
 *
 *  \param argc : number of arguments in command line
 */
void usage(int argc) {
  if (argc < 2) {
    std::cerr << "Error: usage is : <exe> <option_file> " << std::endl;
    std::exit(1);
  }
}



Json::Value get_json_file_content(const std::filesystem::path &json_file) {
  std::ifstream input_file_l(json_file, std::ifstream::binary);

  Json::CharReaderBuilder builder_l;
  Json::Value ret;
  // json file content
  std::string errs;
  if (!parseFromStream(builder_l, input_file_l, &ret, &errs)) {
    std::cerr << std::endl
              << "Invalid Json file: " << json_file.string() << std::endl;
    std::cerr << errs << std::endl;
  }
  return ret;
}
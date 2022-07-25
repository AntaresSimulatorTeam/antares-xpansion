#include "common.h"

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
    std::cerr << "Error: usage is : <exe> <option_file> [True|False]" << std::endl;
    std::exit(1);
  }
  if (argc > 3) {
    std::cerr << "Error too many parameters: usage is : <exe> <option_file> [True|False]" << std::endl;
    std::exit(1);
  }
}

/*!
 *  \brief Build the input from the structure file
 *
 *	Function to build the map linking each problem name to its variables and
 *their id
 *
 *  \param root : root of the structure file
 *
 *  \param summary_name : name of the structure file
 *
 *  \param coupling_map : empty map to increment
 *
 *  \note The id in the coupling_map is that of the variable in the solver
 *responsible for the creation of the structure file.
 */
CouplingMap MPSUtils::build_input(const std::filesystem::path &structure_path) const {
  CouplingMap coupling_map;
  std::ifstream summary(structure_path, std::ios::in);
  if (!summary) {
    std::cout << "Cannot open file summary " << structure_path << std::endl;
    return coupling_map;
  }
  std::string line;

  while (std::getline(summary, line)) {
    std::stringstream buffer(line);
    std::string problem_name;
    std::string variable_name;
    int variable_id;
    buffer >> problem_name;
    buffer >> variable_name;
    buffer >> variable_id;
    coupling_map[problem_name][variable_name] = variable_id;
  }

  summary.close();
  return coupling_map;
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
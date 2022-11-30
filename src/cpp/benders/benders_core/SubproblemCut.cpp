#include "SubproblemCut.h"

/*!
 *  \brief Constructor of a slave cut handler from slave cut data
 *
 *  \param data : pointer to a slave cut data
 */
SubproblemCutDataHandler::SubproblemCutDataHandler(SubproblemCutDataPtr& data)
    : _data(data) {
  // get_subgradient().clear();
  get_int().resize(MAXINTEGER);
  get_dbl().resize(MAXDBL);
  get_str().resize(SubproblemCutStr::MAXSTR);
}

/*!
 *  \brief Get subgradient of a slave cut
 */
Point& SubproblemCutDataHandler::get_subgradient() {
  return _data->first.first.first;
}

/*!
 *  \brief Get int variable of a slave cut
 */
IntVector& SubproblemCutDataHandler::get_int() {
  return _data->first.first.second;
}

/*!
 *  \brief Get double variable of a slave cut
 */
DblVector& SubproblemCutDataHandler::get_dbl() { return _data->first.second; }

/*!
 *  \brief Get string variable of a slave cut
 */
StrVector& SubproblemCutDataHandler::get_str() { return _data->second; }

/*!
 *  \brief Get int variable of a slave cut (SIMPLEXITE, LPSTATUS)
 */
int& SubproblemCutDataHandler::get_int(SubproblemCutInt key) {
  return get_int()[key];
}

/*!
 *  \brief Get double variable of a slave cut (SLAVECOST, ALPHA_I,
 * SUBPROBLEM_TIMER)
 */
double& SubproblemCutDataHandler::get_dbl(SubproblemCutDbl key) {
  return get_dbl()[key];
}

/*!
 *  \brief Get string variable of a slave cut
 */
std::string& SubproblemCutDataHandler::get_str(SubproblemCutStr key) {
  return get_str()[key];
}

/*!
 *  \brief Get subgradient of a slave cut
 */
Point const& SubproblemCutDataHandler::get_subgradient() const {
  return _data->first.first.first;
}

/*!
 *  \brief Get int variable of a slave cut
 */
IntVector const& SubproblemCutDataHandler::get_int() const {
  return _data->first.first.second;
}

/*!
 *  \brief Get double variable of a slave cut
 */
DblVector const& SubproblemCutDataHandler::get_dbl() const {
  return _data->first.second;
}

/*!
 *  \brief Get string variable of a slave cut
 */
StrVector const& SubproblemCutDataHandler::get_str() const {
  return _data->second;
}

/*!
 *  \brief Get int variable of a slave cut (SIMPLEXITE, LPSTATUS)
 */
int SubproblemCutDataHandler::get_int(SubproblemCutInt key) const {
  return get_int()[key];
}

/*!
 *  \brief Get double variable of a slave cut (SLAVECOST, ALPHA_I,
 * SUBPROBLEM_TIMER)
 */
double SubproblemCutDataHandler::get_dbl(SubproblemCutDbl key) const {
  return get_dbl()[key];
}

/*!
 *  \brief Get string variable of a slave cut
 */
std::string const& SubproblemCutDataHandler::get_str(
    SubproblemCutStr key) const {
  return get_str()[key];
}

/*!
 *  \brief Comparator overloading of slave cut trimmer
 */
bool SubproblemCutTrimmer::operator<(SubproblemCutTrimmer const& other) const {
  Predicate point_comp;
  if (std::fabs(_const_cut - other._const_cut) < EPSILON_PREDICATE) {
    return point_comp(_data_cut->get_subgradient(),
                      other._data_cut->get_subgradient());
  } else {
    return (_const_cut < other._const_cut);
  }
}

/*!
 *  \brief Constructor of Slave cut trimmer from an handler and trial values
 */
SubproblemCutTrimmer::SubproblemCutTrimmer(SubproblemCutDataHandlerPtr& data,
                                           const Point& x_cut)
    : _data_cut(data), _x_cut(x_cut) {
  _const_cut = _data_cut->get_dbl(SUBPROBLEM_COST);
  for (auto const& [cand_name, cand_value] : _x_cut) {
    if (get_subgradient().find(cand_name) != get_subgradient().end()) {
      _const_cut -= get_subgradient().find(cand_name)->second * cand_value;
    }
  }
}

/*!
 *  \brief Get subgradient of a slave cut trimmer
 */
Point const& SubproblemCutTrimmer::get_subgradient() const {
  return _data_cut->get_subgradient();
}

/*!
 *  \brief Function to print a slave cut trimmer
 *
 *  \param stream : output stream
 */
void SubproblemCutTrimmer::print(std::ostream& stream) const {
  std::stringstream buffer;
  buffer << _const_cut << get_subgradient();
  stream << buffer.str();
}

/*!
 *  \brief Function to print a slave cut handler
 *
 *  \param stream : output stream
 */
void SubproblemCutDataHandler::print(std::ostream& stream) const {
  std::stringstream buffer;
  buffer << get_dbl(SUBPROBLEM_COST) << get_subgradient();
  stream << buffer.str();
  stream << " Simplexiter " << get_int(SIMPLEXITER) << " | ";
}

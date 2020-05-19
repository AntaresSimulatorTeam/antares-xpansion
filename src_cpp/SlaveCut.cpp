#include "SlaveCut.h"

/*!
*  \brief Constructor of a slave cut handler from slave cut data
*
*  \param data : pointer to a slave cut data
*/
SlaveCutDataHandler::SlaveCutDataHandler(SlaveCutDataPtr & data) :_data(data) {
	//get_subgradient().clear();
	get_int().resize(SlaveCutInt::MAXINT);
	get_dbl().resize(SlaveCutDbl::MAXDBL);
	get_str().resize(SlaveCutDbl::MAXDBL);
}

SlaveCutDataHandler::~SlaveCutDataHandler() {

}

/*!
*  \brief Get subgradient of a slave cut
*/
Point & SlaveCutDataHandler::get_subgradient() {
	return _data->first.first.first;
}

/*!
*  \brief Get int variable of a slave cut
*/
IntVector & SlaveCutDataHandler::get_int() {
	return _data->first.first.second;
}

/*!
*  \brief Get double variable of a slave cut
*/
DblVector & SlaveCutDataHandler::get_dbl() {
	return _data->first.second;
}

/*!
*  \brief Get string variable of a slave cut
*/
StrVector & SlaveCutDataHandler::get_str() {
	return _data->second;
}

/*!
*  \brief Get int variable of a slave cut (SIMPLEXITE, LPSTATUS)
*/
int & SlaveCutDataHandler::get_int(SlaveCutInt key) {
	return get_int()[key];
}

/*!
*  \brief Get double variable of a slave cut (SLAVECOST, ALPHA_I, SLAVE_TIMER)
*/
double & SlaveCutDataHandler::get_dbl(SlaveCutDbl  key) {
	return get_dbl()[key];
}

/*!
*  \brief Get string variable of a slave cut
*/
std::string & SlaveCutDataHandler::get_str(SlaveCutStr key) {
	return get_str()[key];
}

/*!
*  \brief Get subgradient of a slave cut
*/
Point const & SlaveCutDataHandler::get_subgradient() const {
	return _data->first.first.first;
}

/*!
*  \brief Get int variable of a slave cut
*/
IntVector const & SlaveCutDataHandler::get_int() const {
	return _data->first.first.second;
}

/*!
*  \brief Get double variable of a slave cut
*/
DblVector const & SlaveCutDataHandler::get_dbl() const {
	return _data->first.second;
}

/*!
*  \brief Get string variable of a slave cut
*/
StrVector const & SlaveCutDataHandler::get_str() const {
	return _data->second;
}

/*!
*  \brief Get int variable of a slave cut (SIMPLEXITE, LPSTATUS)
*/
int SlaveCutDataHandler::get_int(SlaveCutInt key)const {
	return get_int()[key];
}

/*!
*  \brief Get double variable of a slave cut (SLAVECOST, ALPHA_I, SLAVE_TIMER)
*/
double SlaveCutDataHandler::get_dbl(SlaveCutDbl  key) const {
	return get_dbl()[key];
}

/*!
*  \brief Get string variable of a slave cut
*/
std::string const & SlaveCutDataHandler::get_str(SlaveCutStr key) const {
	return get_str()[key];
}

/*!
*  \brief Comparator overloading of slave cut trimmer
*/
bool SlaveCutTrimmer::operator<(SlaveCutTrimmer const & other) const {
	Predicate point_comp;
	if (std::fabs(_const_cut - other._const_cut) < EPSILON_PREDICATE) {
		return point_comp(_data_cut->get_subgradient(), other._data_cut->get_subgradient());
	}
	else {
		return (_const_cut < other._const_cut);
	}
}

/*!
*  \brief Constructor of Slave cut trimmer from an handler and trial values
*/
SlaveCutTrimmer::SlaveCutTrimmer(SlaveCutDataHandlerPtr & data, Point & x0) : _data_cut(data), _x0(x0) {
	_const_cut = _data_cut->get_dbl(SLAVE_COST);
	for (auto const & kvp : _x0) {
		_const_cut -= get_subgradient().find(kvp.first)->second * kvp.second;
	}
}

/*!
*  \brief Get subgradient of a slave cut trimmer
*/
Point const & SlaveCutTrimmer::get_subgradient() const {
	return _data_cut->get_subgradient();
}

/*!
*  \brief Operator overloading of slave cut trimmer for stream output
*
*  \param stream : output stream
*/
std::ostream & operator<<(std::ostream & stream, SlaveCutTrimmer const & rhs) {
	rhs.print(stream);
	return stream;
}

/*!
*  \brief Function to print a slave cut trimmer
*
*  \param stream : output stream
*/
void SlaveCutTrimmer::print(std::ostream & stream)const {
	std::stringstream buffer;
	buffer << _const_cut << get_subgradient();
	stream << buffer.str();
}

/*!
*  \brief Stream output overloading of slave cut handler
*
*  \param stream : output stream
*/
std::ostream & operator<<(std::ostream & stream, SlaveCutDataHandler const & rhs) {
	rhs.print(stream);
	return stream;
}

/*!
*  \brief Function to print a slave cut handler
*
*  \param stream : output stream
*/
void SlaveCutDataHandler::print(std::ostream & stream)const {
	std::stringstream buffer;
	buffer << get_dbl(SLAVE_COST) << get_subgradient();
	stream << buffer.str();
	stream << " Simplexiter " << get_int(SIMPLEXITER) << " | ";
}

/*!
*  \brief Stream output overloading of a slave cut
*
*  \param stream : output stream
*/
std::ostream & operator<<(std::ostream & stream, SlaveCutData const & rhs) {
	std::stringstream buffer;
	buffer << rhs.first.second[SLAVE_COST] << rhs.first.first.first;
	stream << buffer.str();
	stream << " Simplexiter " << rhs.first.first.second[SIMPLEXITER] << " | ";
	return stream;
}

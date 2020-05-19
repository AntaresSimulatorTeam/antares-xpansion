#include "SimplexBasis.h"


SimplexBasisHandler::SimplexBasisHandler() {
}

SimplexBasisHandler::SimplexBasisHandler(SimplexBasisPtr & data) :_data(data) {
}

SimplexBasisHandler::~SimplexBasisHandler() {

}

/*!
*  \brief Stream output overloading for vector of IntVector
*
*  \param stream : stream output
*
*  \param rhs : vector of IntVector
*/
bool SimplexBasisHandler::operator<(SimplexBasisHandler const & other) const
{
	return((get_row() < other.get_row()) || ((get_row() == other.get_row()) && (get_col() < other.get_col())));
}

/*!
*  \brief Get basis status of the column in constraint matrix
*/
IntVector & SimplexBasisHandler::get_col() {
	return _data->second;
}

/*!
*  \brief Get basis status of each variable in the matrix
*/
IntVector & SimplexBasisHandler::get_row() {
	return _data->first;
}

/*!
*  \brief Get basis status of the column in constraint matrix
*/
IntVector const & SimplexBasisHandler::get_col() const {
	return _data->second;
}

/*!
*  \brief Get basis status of each variable in the matrix
*/
IntVector const & SimplexBasisHandler::get_row() const {
	return _data->first;
}

/*!
*  \brief Print function for simplex basis
*
*  \param stream : stream output
*/
void SimplexBasisHandler::print(std::ostream & stream)const {
	std::stringstream buffer;
	buffer << "Rows : (";
	for (int i(0); i < get_row().size(); i++) {
		buffer << get_row()[i] << " ";
	}
	buffer << ") || Cols : (";
	for (int i(0); i < get_col().size(); i++) {
		buffer << get_col()[i] << " ";
	}
	buffer << ")";
	stream << buffer.str();
}

/*!
*  \brief Stream output overloading for simplex basis
*
*  \param stream : stream output
*
*  \param rhs : simplex basis to print
*/
std::ostream & operator<<(std::ostream & stream, SimplexBasisHandler const & rhs) {
	rhs.print(stream);
	return stream;
}

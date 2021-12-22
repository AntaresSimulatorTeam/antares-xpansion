#pragma once


#include <list>
#include <string>
#include <set>
#include <map>

#include <vector>
#include <iostream>
#include <sstream>
#include <iomanip>
#include <fstream>
#include <cstdlib>


inline bool contains(std::string const & v1, std::string const & v2) {
	return v1.find(v2) != std::string::npos;
}
inline int to_int(std::string const & value) {
	std::stringstream buffer;
	buffer << value;
	int result;
	buffer >> result;
	return result;
}

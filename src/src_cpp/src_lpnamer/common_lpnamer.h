#pragma once


#include <list>
#include <string>
#include <set>
#include <map>

#include <vector>
//#include <experimental/filesystem>
#include <iostream>
#include <sstream>
#include <iomanip>
#include <fstream>
#include <cstdlib>

#if defined(WIN32) || defined(_WIN32) 
#define PATH_SEPARATOR "\\" 
#else 
#define PATH_SEPARATOR "/" 
#endif 


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

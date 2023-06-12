#pragma once
#include <sstream>
#include <string>

std::string RemovePrefixFromLineMessage(const std::string& msg);
std::string RemovePrefixFromMessage(std::stringstream& message);
//
// Created by marechaljas on 02/05/2022.
//

#ifndef ANTARESXPANSION_TESTS_CPP_LP_NAMER_PROBLEMCONSTRUCTIONTEST_CPP_PROBLEMNAMEPARSER_H_
#define ANTARESXPANSION_TESTS_CPP_LP_NAMER_PROBLEMCONSTRUCTIONTEST_CPP_PROBLEMNAMEPARSER_H_

#include <filesystem>
#include <cstring>

unsigned int MCYear(std::string problem_file_name);
unsigned int MCYear(std::filesystem::path file_path);


#endif  // ANTARESXPANSION_TESTS_CPP_LP_NAMER_PROBLEMCONSTRUCTIONTEST_CPP_PROBLEMNAMEPARSER_H_

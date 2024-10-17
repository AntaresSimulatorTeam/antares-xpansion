//
// Created by marechaljas on 02/08/22.
//

#pragma once
#include "antares-xpansion/xpansion_interfaces/OutputWriter.h"

class WriterNOOPStub : public Output::OutputWriter {
 public:
  void update_solution(const Output::SolutionData& solution_data) override {}
  void dump() override {}
  void initialize() override {}
  void end_writing(const Output::IterationsData& iterations_data) override {}
  void write_solver_name(const std::string& solver_name) override {}
  void write_master_name(const std::string& master_name) override {}
  void write_log_level(const int log_level) override {}
  void write_solution(const Output::SolutionData& solution) override {}

  void write_iteration(const Output::Iteration& iteration_data,
                       const size_t iteration_num) override {}
  void updateBeginTime() override {}
  void updateEndTime() override {}
  void write_nbweeks(const int nb_weeks) override {}
  void write_duration(const double duration) override {}
  std::string solution_status() const override { return std::string(); }
  void WriteProblem(const Output::ProblemData& problem_data) override {}
};
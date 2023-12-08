#include "BendersMathLogger.h"

// #ifdef _WIN32
// #include <Windows.h>
// #endif

HeadersManager::HeadersManager(HEADERSTYPE type) {
  if (type == HEADERSTYPE::SHORT) {
    ITERATION = "ITE";
    LB = "LB";
    UB = "UB";
    BESTUB = "BESTUB";
    ABSOLUTE_GAP = "AGAP";
    RELATIVE_GAP = "RGAP";
    MINSIMPLEX = "miSpx";
    MAXSIMPLEX = "MaSpx";
    NUMBER_OF_SUBPROBLEM_SOLVED = "NBSUBSOLVD";
    CUMULATIVE_NUMBER_OF_SUBPROBLEM_SOLVED = "CNBSBSOLVD";
    BENDERS_TIME = "BENDTime";
    TIMEMASTER = "MASTTime";
    SUB_PROBLEMS_TIME_CPU = "SUBCPUTIME";
    SUB_PROBLEMS_TIME_WALL = "SUBWATIME";
    TIME_NOT_DOING_MASTER_OR_SUB_PROBLEMS_WALL = "TNotMAoSUB";
  } else {
    ITERATION = "ITERATION";
    LB = "LB";
    UB = "UB";
    BESTUB = "BESTUB";
    ABSOLUTE_GAP = "ABSOLUTE GAP";
    RELATIVE_GAP = "RELATIVE GAP";
    MINSIMPLEX = "MINSIMPLEX";
    MAXSIMPLEX = "MAXSIMPLEX";
    NUMBER_OF_SUBPROBLEM_SOLVED = "NUMBER OF SUBPROBLEMS SOLVED";
    CUMULATIVE_NUMBER_OF_SUBPROBLEM_SOLVED =
        "CUMULATIVE NUMBER OF SUBPROBLEMS SOLVED";
    BENDERS_TIME = "BENDERS TIME";
    TIMEMASTER = "MASTER TIME";
    SUB_PROBLEMS_TIME_CPU = "SUB-PROBLEMS TIME (CPU)";
    SUB_PROBLEMS_TIME_WALL = "SUB-PROBLEMS TIME (WALL)";
    TIME_NOT_DOING_MASTER_OR_SUB_PROBLEMS_WALL =
        "TIME NOT DOING MASTER OR SUB-PROBLEMS (WALL)";
  }
}

LogDestination::LogDestination(std::ostream* stream, std::streamsize width)
    : stream_(stream), width_(width) {
  // _COORD coordinates;
  // coordinates.X = 1000;
  // coordinates.Y = 1000;

  // if (0 == SetConsoleScreenBufferSize(GetStdHandle(STD_OUTPUT_HANDLE),
  //                                     coordinates)) {
  //   std::cout << "could not resize the console screen\n";
  //   // return -1;
  // }
}
void MathLoggerDriver::add_logger(
    std::shared_ptr<MathLoggerImplementation> logger) {
  if (logger) {
    math_loggers_.push_back(logger);
  }
}

void MathLoggerDriver::Print(const CurrentIterationData& data) {
  for (auto logger : math_loggers_) {
    logger->Print(data);
  }
}

void MathLoggerDriver::write_header() {
  for (auto logger : math_loggers_) {
    logger->write_header();
  }
}

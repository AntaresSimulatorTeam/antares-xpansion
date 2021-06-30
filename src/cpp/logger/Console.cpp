//
// Created by s20217 on 20/04/2021.
//

#include <algorithm>
#include <iomanip>
#include <sstream>
#include <list>

#include "logger/Console.h"

namespace xpansion {
    namespace logger {

        Console::Console(const std::string& filename)
        {
            _file.open(filename);
            _userLog = std::make_unique<User>(_file);
        }

        Console::~Console()
        {
            _file.close();
        }

        void Console::display_message(const std::string& str)
        {
            _userLog->display_message(str);
        }

        void Console::log_at_initialization(const LogData& d)
        {
            _userLog->log_at_initialization(d);
        }

        void Console::log_iteration_candidates(const LogData& d)
        {
            _userLog->log_iteration_candidates(d);
        }

        void Console::log_master_solving_duration(double durationInSeconds)
        {
            _userLog->log_master_solving_duration(durationInSeconds);
        }

        void Console::log_subproblems_solving_duration(double durationInSeconds)
        {
            _userLog->log_subproblems_solving_duration(durationInSeconds);
        }

        void Console::log_at_iteration_end(const LogData& d)
        {
            _userLog->log_at_iteration_end(d);
        }

        void Console::log_at_ending(const LogData& d)
        {
            _userLog->log_at_ending(d);
        }

        void Console::log_total_duration(double durationInSeconds)
        {
            _userLog->log_total_duration(durationInSeconds);
        }

    } // namespace logger
} // namespace xpansion

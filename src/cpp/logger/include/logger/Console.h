//
// Created by s90365 on 29/06/2021.
//

#ifndef ANTARESXPANSION_CONSOLE_H
#define ANTARESXPANSION_CONSOLE_H

#include <ostream>
#include <fstream>

#include "benders_sequential_core/ILogger.h"
#include "logger/User.h"

namespace xpansion{
namespace logger {

    class Console : public ILogger {

    public:

        Console(const std::string& filename);
        ~Console();

        void display_message(const std::string& str) override;

        void log_at_initialization(const LogData& d) override;

        void log_iteration_candidates(const LogData& d) override;

        void log_master_solving_duration(double durationInSeconds) override;

        void log_subproblems_solving_duration(double durationInSeconds) override;

        void log_at_iteration_end(const LogData& d) override;

        void log_at_ending(const LogData& d) override;

        void log_total_duration(double durationInSeconds) override;

    private:

		std::ofstream 		    _file;
        std::unique_ptr<User>   _userLog;

    };

} // namespace logger
} // namespace xpansion

#endif //ANTARESXPANSION_CONSOLE_H

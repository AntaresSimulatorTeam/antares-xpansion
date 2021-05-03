//
// Created by s20217 on 20/04/2021.
//

#ifndef ANTARESXPANSION_USER_H
#define ANTARESXPANSION_USER_H

#include <ostream>

#include "benders_sequential_core/ILogger.h"

namespace xpansion{
namespace logger {

    class User : public  ILogger {

    public:

        User(std::ostream& stream);

        void display_message(const std::string& str) override;

        void log_at_initialization(const LogData &d) override;

        void log_iteration_candidates(const LogData &d) override;

        void log_master_solving_duration(double durationInSeconds) override;

        void log_subproblems_solving_duration(double durationInSeconds) override;

        void log_at_iteration_end(const LogData &d) override;

        void log_at_ending(const LogData &d) override;

        void log_total_duration(double durationInSeconds) override;

    private:

        std::ostream& _stream;


    };

} // namespace logger
} // namespace xpansion

#endif //ANTARESXPANSION_USER_H

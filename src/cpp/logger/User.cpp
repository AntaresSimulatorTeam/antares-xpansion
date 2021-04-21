//
// Created by s20217 on 20/04/2021.
//

#include "logger/User.h"


namespace xpansion{
namespace logger {

    User::User(std::ostream& stream)
    :_stream(stream)
    {
    }

    void User::log_at_initialization(const LogData &) {

    }

    void User::log_at_iteration(const LogData &d) {
        _stream << "it = " << d.it << std::endl;
    }

    void User::log_at_ending(const LogData &d) {

    }

} // namespace logger
} // namespace xpansion
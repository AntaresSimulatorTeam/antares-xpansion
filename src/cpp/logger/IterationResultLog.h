//
// Created by s20217 on 21/04/2021.
//

#ifndef ANTARESXPANSION_ITERATIONRESULTLOG_H
#define ANTARESXPANSION_ITERATIONRESULTLOG_H

namespace xpansion{
namespace logger {

    class IterationResultLog {

    public:

        static std::string Log_at_iteration_end(const LogData &d);
    };

} // namespace logger
} // namespace xpansion

#endif //ANTARESXPANSION_ITERATIONRESULTLOG_H

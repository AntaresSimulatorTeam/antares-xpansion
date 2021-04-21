//
// Created by s20217 on 21/04/2021.
//

#ifndef ANTARESXPANSION_CANDIDATELOG_H
#define ANTARESXPANSION_CANDIDATELOG_H

namespace xpansion{
namespace logger {

    class CandidateLog {

    public :

        static std::string Log_at_iteration_start(const LogData &d);
    };

} // namespace logger
} // namespace xpansion

#endif //ANTARESXPANSION_CANDIDATELOG_H

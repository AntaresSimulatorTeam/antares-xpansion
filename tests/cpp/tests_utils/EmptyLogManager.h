#include "antares-xpansion/multisolver_interface/SolverAbstract.h"

class EmptyLogManager: public SolverLogManager
{
public:
    EmptyLogManager& operator=(const EmptyLogManager& other)
    {
        return *this;
    }

    ~EmptyLogManager() = default;
};
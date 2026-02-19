#include <cassert>
#include <iostream>

#include "antares-xpansion/benders/adapters/BendersBaseAdapter.h"
#include "antares-xpansion/benders/benders_core/BendersBase.h"
#include "antares-xpansion/benders/benders_core/common.h"
#include "antares-xpansion/benders/factories/LoggerFactories.h"
#include "antares-xpansion/xpansion_interfaces/ILogger.h"

class DummyBenders: public BendersBase
{
public:
    DummyBenders():
        BendersBase(BendersBaseOptions(SolverBaseOptions()),
                    build_void_logger(),
                    std::shared_ptr<Output::OutputWriter>(),
                    std::shared_ptr<MathLoggerDriver>())
    {
    }

    void launch() override
    {
    }

    std::string BendersName() const override
    {
        return "Dummy";
    }

    void InitializeProblems() override
    {
    }

protected:
    void Run() override
    {
    }

    bool shouldParallelize() const override
    {
        return false;
    }
};

int main()
{
    DummyBenders dummy;
    BendersBaseAdapter adapter(dummy);
    assert(adapter.BendersName() == "Dummy");
    std::cout << "Adapter smoke test OK\n";
    return 0;
}

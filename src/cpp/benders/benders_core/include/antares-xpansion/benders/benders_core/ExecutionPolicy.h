#pragma once

#include <execution>

/**
 * std execution policies don't share a base type so we can't just select
 * them in place in the foreach. This function allows the selection of policy
 * via template deduction.
 **/
template<class lambda>
auto selectPolicy(lambda f, bool shouldParallelize)
{
    if (shouldParallelize)
    {
        return f(std::execution::par_unseq);
    }
    else
    {
        return f(std::execution::seq);
    }
}

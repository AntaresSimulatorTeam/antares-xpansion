#pragma once

#include <algorithm>
#include <cassert>
#include <functional>
#include <vector>

class Interpolator
{
public:
    /// @brief function that lineraly interpolates a double from a list of x and y coordonates
    /// @param x coordinates
    /// @param y values
    /// @return a function taking an double and returning the linear interpolation of this double
    static std::function<double(double)> linearInterpolation(const std::vector<double>& x,
                                                             const std::vector<double>& y)
    {
        return [x, y](double xi) -> double
        {
            assert(std::is_sorted(x.begin(), x.end()));
            if (xi <= x.front())
            {
                return y.front();
            }
            if (xi >= x.back())
            {
                return y.back();
            }

            for (size_t i = 1; i < x.size(); ++i)
            {
                if (xi < x[i])
                {
                    double slope = (y[i] - y[i - 1]) / (x[i] - x[i - 1]);
                    return y[i - 1] + slope * (xi - x[i - 1]);
                }
            }

            return y.back();
        };
    }
};

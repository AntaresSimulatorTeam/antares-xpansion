#pragma once
#include <vector>

template <typename T>
struct AddVectors {
  void operator()(const std::vector<T>& a, const std::vector<T>& b) const {
    if (a.size() == b.size()) {
      std::transform(a.begin(), a.end(), b.begin(), a.begin(), std::plus<T>());
    }
  }
};
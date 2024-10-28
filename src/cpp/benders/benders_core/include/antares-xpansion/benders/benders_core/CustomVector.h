#pragma once
#include <vector>

template <typename T>
void AddVectors(std::vector<T>& a, const std::vector<T>& b) {
  if (a.size() == b.size()) {
    std::transform(a.begin(), a.end(), b.begin(), a.begin(), std::plus<T>());
  }
}

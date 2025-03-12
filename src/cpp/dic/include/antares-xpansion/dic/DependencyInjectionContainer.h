#pragma once
#include <string>
#include <unordered_map>
#include <any>

namespace Xpansion {
class DependencyInjectionContainer {
  DependencyInjectionContainer() = default;
  DependencyInjectionContainer(DependencyInjectionContainer const &) = delete;
  void operator=(DependencyInjectionContainer const &) = delete;
  DependencyInjectionContainer(DependencyInjectionContainer &&) = delete;
  DependencyInjectionContainer &operator=(DependencyInjectionContainer &&) = delete;
public:
static auto &Instance() {
  static DependencyInjectionContainer instance;
  return instance;
}
  template<class T>
  void  Register(const std::string &key, const T &value) {
    container[key] = value;
  }
  template<class T>
  auto get(const std::string &key) -> T {
    return std::any_cast<T>(container[key]);
  }

  std::unordered_map<std::string, std::any> container;
};
} // Xpansion

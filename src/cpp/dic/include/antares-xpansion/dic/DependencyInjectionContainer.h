#pragma once
#include <string>
#include <unordered_map>
#include <memory>
#include <boost/any/unique_any.hpp>

namespace Xpansion {
class DependencyInjectionContainer {
  DependencyInjectionContainer() = default;
public:
  DependencyInjectionContainer(DependencyInjectionContainer const &) = delete;
  void operator=(DependencyInjectionContainer const &) = delete;
  DependencyInjectionContainer(DependencyInjectionContainer &&) = delete;
  DependencyInjectionContainer &operator=(DependencyInjectionContainer &&) = delete;

static auto &Instance() {
  static DependencyInjectionContainer instance;
  return instance;
}
  template<class T>
  void  Register(const std::string &key, const T& value) {
    container[key] = value;
  }

    template<class T>
void  Register(const std::string &key, T&& value) {
    container.insert( {key, std::forward<T>(value)} );
}

    template<class T, class... Args>
        void Make(const std::string &key, Args &&... args)
{
    auto tmp = std::make_unique<T>(std::forward<Args>(args)...);
        container.insert({key, std::move(tmp)});
}

    template<class T>
  auto get(const std::string &key) -> T& {
    return boost::any_cast<T&>(container[key]);
}

  std::unordered_map<std::string, boost::anys::unique_any> container;
};
} // Xpansion

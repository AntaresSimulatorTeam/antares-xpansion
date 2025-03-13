#pragma once
#include <memory>
#include <string>
#include <unordered_map>

#include <boost/any/unique_any.hpp>

namespace Xpansion
{

/**
 * @brief Dependency Injection Container (DIC)
 *
 * Thread-safe singleton class that manages global application dependencies
 * by storing and retrieving them by name.
 *
 * Features:
 * - Meyers singleton pattern implementation
 * - Thread-safe initialization
 * - Container operations are not thread-safe
 *
 * @warning Container operations are not thread-safe
 */
class DependencyInjectionContainer
{
    DependencyInjectionContainer() = default;

public:
    DependencyInjectionContainer(const DependencyInjectionContainer&) = delete;
    void operator=(const DependencyInjectionContainer&) = delete;
    DependencyInjectionContainer(DependencyInjectionContainer&&) = delete;
    DependencyInjectionContainer& operator=(DependencyInjectionContainer&&) = delete;

    /**
     * @brief Get the unique container instance
     * @return Reference to the unique instance
     */
    static auto& Instance()
    {
        static DependencyInjectionContainer instance;
        return instance;
    }

    /**
     * @brief Register a dependency by move
     *
     * Especially useful for non-copyable types like std::unique_ptr
     *
     * @tparam T Type of the dependency
     * @param key Unique identifier for the dependency
     * @param value Dependency value to move
     */
    template<class T>
    void Register(const std::string& key, const T& value)
    {
        container[key] = value;
    }

    /**
     * Register a dependency
     * Useful for owning objects like std::unique_ptr
     * @tparam T any type even non-copyable or non-movable
     * @param key name of the dependency
     * @param value dependency
     */
    template<class T>
    void Register(const std::string& key, T&& value)
    {
        if (container.find(key) != container.end())
        {
            throw std::invalid_argument("Key already exists");
        }
        container.insert({key, std::forward<T>(value)});
    }

    /**
     * Create a dependency
     * @tparam T Any constructable type
     * @tparam Args T constructor args
     * @param key name of the dependency
     * @param args T constructor args
     */
    template<class T, class... Args>
    void Make(const std::string& key, Args&&... args)
    {
        if (container.find(key) != container.end())
        {
            throw std::invalid_argument("Key already exists");
        }
        auto tmp = std::make_unique<T>(std::forward<Args>(args)...);
        container.insert({key, std::move(tmp)});
    }

    /**
     *
     * @tparam T Dependency type
     * @param key name of a dependency
     * @return Reference to a depenbdency
     */
    template<class T>
    auto get(const std::string& key) -> T&
    {
        return boost::any_cast<T&>(container[key]);
    }

    std::unordered_map<std::string, boost::anys::unique_any> container;
};
} // namespace Xpansion

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
     * @throws std::invalid_argument If key already exists
     */
    template<class T>
    void Register(const std::string& key, const T& value)
    {
        if (container.find(key) != container.end())
        {
            throw std::invalid_argument("Dependency already exists");
        }
        container[key] = value;
    }

    /**
     * @brief Create and register a new dependency instance
     *
     * @tparam T Type to construct
     * @tparam Args Constructor argument types
     * @param key Unique identifier for the dependency
     * @param args Arguments forwarded to T's constructor
     * @throws std::invalid_argument If key already exists
     */
    template<class T>
    void Register(const std::string& key, T&& value)
    {
        if (container.find(key) != container.end())
        {
            throw std::invalid_argument("Dependency already exists");
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
            throw std::invalid_argument("Dependency already exists");
        }
        auto tmp = std::make_unique<T>(std::forward<Args>(args)...);
        container.insert({key, std::move(tmp)});
    }

    /**
     * @brief Retrieve a registered dependency
     *
     * @tparam T Expected dependency type
     * @param key Dependency identifier
     * @return T& Reference to the dependency
     * @throws boost::bad_any_cast If requested type doesn't match stored type
     * @throws std::out_of_range If key doesn't exist
     */
    template<class T>
    auto get(const std::string& key) -> T&
    {
        if (container.find(key) == container.end())
        {
            throw std::out_of_range("Dependency not found");
        }
        return boost::any_cast<T&>(container[key]);
    }

    std::unordered_map<std::string, boost::anys::unique_any> container;
};
} // namespace Xpansion

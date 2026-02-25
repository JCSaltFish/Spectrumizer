/**
 * @file ScopeGuard.hpp
 * @brief Defines a ScopeGuard class for executing a function upon scope exit.
 */

#pragma once

#include "utilsCommon.h"

/**
 * @brief A guard that executes a given function when it goes out of scope.
 *
 * This class is useful for ensuring that cleanup code is executed when a scope is exited,
 * even if the exit is due to an exception being thrown.
 */
class ScopeGuard {
public:
    explicit ScopeGuard(std::function<void()> onExit) : m_onExit(onExit) {};
    ~ScopeGuard() { if (m_onExit) m_onExit(); };
    ScopeGuard(const ScopeGuard&) = delete;
    ScopeGuard& operator=(const ScopeGuard&) = delete;
    ScopeGuard(ScopeGuard&& other) noexcept :
        m_onExit(std::move(other.m_onExit)) {
        other.m_onExit = nullptr;
    };
    ScopeGuard& operator=(ScopeGuard&& other) noexcept {
        if (this != &other) {
            m_onExit = std::move(other.m_onExit);
            other.m_onExit = nullptr;
        }
        return *this;
    };

private:
    std::function<void()> m_onExit; // Function to call on scope exit
};

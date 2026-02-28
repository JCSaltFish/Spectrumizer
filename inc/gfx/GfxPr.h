/**
 * @file GfxPr.h
 * @brief Private header for the GFX module.
 */

#pragma once

#include "GfxPub.h"

/**
 * @brief Scope guard class.
 */
class GfxScopeGuard {
public:
    explicit GfxScopeGuard(std::function<void()> onExit) : m_onExit(onExit) {};
    ~GfxScopeGuard() { if (m_onExit) m_onExit(); };
    GfxScopeGuard(const GfxScopeGuard&) = delete;
    GfxScopeGuard& operator=(const GfxScopeGuard&) = delete;
    GfxScopeGuard(GfxScopeGuard&& other) noexcept :
        m_onExit(std::move(other.m_onExit)) {
        other.m_onExit = nullptr;
    };
    GfxScopeGuard& operator=(GfxScopeGuard&& other) noexcept {
        if (this != &other) {
            m_onExit = std::move(other.m_onExit);
            other.m_onExit = nullptr;
        }
        return *this;
    };

private:
    std::function<void()> m_onExit; // Function to call on scope exit
};

/**
 * @brief Graphics pipeline state controller class.
 */
class GfxPipelineStateController {
public:
    /**
     * @brief Get the current pipeline state cache for a given state machine.
     * @param stateMachine The state machine to get the cache for.
     * @return The current pipeline state cache.
     */
    static const GfxPipelineStateCache& getStateCache(GfxPipelineStateMachine stateMachine);
    /**
     * @brief Bind a pipeline to the current state machine.
     * @param stateMachine The state machine to bind the pipeline to.
     * @param pipeline The pipeline to bind.
     */
    static void bindPipeline(GfxPipelineStateMachine stateMachine, GfxPipeline pipeline);
    /**
     * @brief Compare the current pipeline state with another state.
     * @param state The current pipeline state.
     * @param stateMachine The state machine to compare against.
     * @param other The other pipeline state cache to compare with.
     * @return True if the states are equal, false otherwise.
     */
    static bool compareState(
        GfxPipelineState state,
        GfxPipelineStateMachine stateMachine,
        const GfxPipelineStateCache& other
    );
    /**
     * @brief Cache the current pipeline state to a given pipeline.
     * @param stateMachine The state machine to cache the state from.
     * @param pipeline The pipeline to cache the state to.
     */
    static void cachePipelineState(GfxPipelineStateMachine stateMachine, GfxPipeline pipeline);
    /**
     * @brief Update the pipeline state for a given pipeline and state.
     * @param pipeline The pipeline to update.
     * @param state The state to update.
     * @param stateCache The current state cache to update.
     */
    static void updatePipelineState(
        GfxPipeline pipeline,
        GfxPipelineState state,
        GfxPipelineStateCache stateCache
    );
};

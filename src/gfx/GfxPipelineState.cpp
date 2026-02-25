/**
 * @file GfxPipelineState.cpp
 * @brief Implementation of GfxPipelineStateController for managing pipeline states.
 */

#include "gfx/GfxPr.h"

bool GfxPipelineStateController::compareState(
    GfxPipelineState state,
    GfxPipelineStateMachine stateMachine,
    const GfxPipelineStateCache& other
) {
    int result = 0;
    switch (state) {
    case GfxPipelineState::VIEWPORT:
        result = std::memcmp(
            &stateMachine->m_stateCache.viewport,
            &other.viewport,
            sizeof(GfxViewport));
        return result == 0;
    case GfxPipelineState::SCISSOR:
        result = std::memcmp(
            &stateMachine->m_stateCache.scissor,
            &other.scissor,
            sizeof(GfxRect)
        );
        return result == 0;
    case GfxPipelineState::LINE_WIDTH:
        return stateMachine->m_stateCache.lineWidth == other.lineWidth;
    case GfxPipelineState::DEPTH_BIAS:
        result = std::memcmp(
            &stateMachine->m_stateCache.depthBiasParams,
            &other.depthBiasParams,
            sizeof(GfxDepthBiasParams)
        );
        return result == 0;
    case GfxPipelineState::BLEND_CONSTANTS:
        return false;
    case GfxPipelineState::STENCIL_COMPARE_MASK:
    case GfxPipelineState::STENCIL_WRITE_MASK:
    case GfxPipelineState::STENCIL_REFERENCE:
    case GfxPipelineState::STENCIL_OP:
        result = std::memcmp(
            &stateMachine->m_stateCache.frontFaceStencilOpParams,
            &other.frontFaceStencilOpParams,
            sizeof(GfxStencilOpParams)
        );
        result |= std::memcmp(
            &stateMachine->m_stateCache.backFaceStencilOpParams,
            &other.backFaceStencilOpParams,
            sizeof(GfxStencilOpParams)
        );
        return result == 0;
    case GfxPipelineState::CULL_MODE:
        return stateMachine->m_stateCache.cullMode == other.cullMode;
    case GfxPipelineState::FRONT_FACE:
        return stateMachine->m_stateCache.frontFace == other.frontFace;
    case GfxPipelineState::PRIMITIVE_TOPOLOGY:
        return stateMachine->m_stateCache.primitiveTopo == other.primitiveTopo;
    case GfxPipelineState::DEPTH_TEST_ENABLE:
        return stateMachine->m_stateCache.depthTestEnabled == other.depthTestEnabled;
    case GfxPipelineState::DEPTH_WRITE_ENABLE:
        return stateMachine->m_stateCache.depthWriteEnabled == other.depthWriteEnabled;
    case GfxPipelineState::DEPTH_COMPARE_OP:
        return stateMachine->m_stateCache.depthCompareOp == other.depthCompareOp;
    case GfxPipelineState::STENCIL_TEST_ENABLE:
        return stateMachine->m_stateCache.stencilTestEnabled == other.stencilTestEnabled;
    case GfxPipelineState::DEPTH_BIAS_ENABLE:
        return stateMachine->m_stateCache.depthBiasEnabled == other.depthBiasEnabled;
    case GfxPipelineState::PRIMITIVE_RESTART_ENABLE:
        return stateMachine->m_stateCache.primitiveRestartEnabled == other.primitiveRestartEnabled;
    case GfxPipelineState::LOGIC_OP:
        return stateMachine->m_stateCache.logicOp == other.logicOp;
    case GfxPipelineState::LOGIC_OP_ENABLE:
        return stateMachine->m_stateCache.logicOpEnabled == other.logicOpEnabled;
    case GfxPipelineState::COLOR_BLEND_ENABLE:
        return stateMachine->m_stateCache.colorBlendEnabled == other.colorBlendEnabled;
    case GfxPipelineState::COLOR_BLEND_EQUATION:
        result = std::memcmp(
            &stateMachine->m_stateCache.colorBlendEquation,
            &other.colorBlendEquation,
            sizeof(GfxBlendEquation)
        );
        return result == 0;
    case GfxPipelineState::COLOR_WRITE_MASK:
        return stateMachine->m_stateCache.colorWriteMask == other.colorWriteMask;
    case GfxPipelineState::LINE_SMOOTH_ENABLE:
        return stateMachine->m_stateCache.lineSmoothEnabled == other.lineSmoothEnabled;
    case GfxPipelineState::POLYGON_MODE:
        return stateMachine->m_stateCache.polygonMode == other.polygonMode;
    default:
        return false; // Unsupported state
    }
}

void GfxPipelineStateController::cachePipelineState(
    GfxPipelineStateMachine stateMachine,
    GfxPipeline pipeline
) {
    pipeline->m_stateCache = stateMachine->m_stateCache;
}

void GfxPipelineStateController::updatePipelineState(
    GfxPipeline pipeline,
    GfxPipelineState state,
    GfxPipelineStateCache stateCache
) {
    switch (state) {
    case GfxPipelineState::VIEWPORT:
        pipeline->m_stateCache.viewport = stateCache.viewport;
        break;
    case GfxPipelineState::SCISSOR:
        pipeline->m_stateCache.scissor = stateCache.scissor;
        break;
    case GfxPipelineState::LINE_WIDTH:
        pipeline->m_stateCache.lineWidth = stateCache.lineWidth;
        break;
    case GfxPipelineState::DEPTH_BIAS:
        pipeline->m_stateCache.depthBiasParams = stateCache.depthBiasParams;
        break;
    case GfxPipelineState::BLEND_CONSTANTS:
        std::memcpy(
            pipeline->m_stateCache.blendConstants,
            stateCache.blendConstants,
            sizeof(float) * 4
        );
        break;
    case GfxPipelineState::STENCIL_COMPARE_MASK:
    case GfxPipelineState::STENCIL_WRITE_MASK:
    case GfxPipelineState::STENCIL_REFERENCE:
    case GfxPipelineState::STENCIL_OP:
        pipeline->m_stateCache.frontFaceStencilOpParams =
            stateCache.frontFaceStencilOpParams;
        pipeline->m_stateCache.backFaceStencilOpParams =
            stateCache.backFaceStencilOpParams;
        break;
    case GfxPipelineState::CULL_MODE:
        pipeline->m_stateCache.cullMode = stateCache.cullMode;
        break;
    case GfxPipelineState::FRONT_FACE:
        pipeline->m_stateCache.frontFace = stateCache.frontFace;
        break;
    case GfxPipelineState::PRIMITIVE_TOPOLOGY:
        pipeline->m_stateCache.primitiveTopo = stateCache.primitiveTopo;
        break;
    case GfxPipelineState::DEPTH_TEST_ENABLE:
        pipeline->m_stateCache.depthTestEnabled = stateCache.depthTestEnabled;
        break;
    case GfxPipelineState::DEPTH_WRITE_ENABLE:
        pipeline->m_stateCache.depthWriteEnabled = stateCache.depthWriteEnabled;
        break;
    case GfxPipelineState::DEPTH_COMPARE_OP:
        pipeline->m_stateCache.depthCompareOp = stateCache.depthCompareOp;
        break;
    case GfxPipelineState::STENCIL_TEST_ENABLE:
        pipeline->m_stateCache.stencilTestEnabled = stateCache.stencilTestEnabled;
        break;
    case GfxPipelineState::DEPTH_BIAS_ENABLE:
        pipeline->m_stateCache.depthBiasEnabled = stateCache.depthBiasEnabled;
        break;
    case GfxPipelineState::PRIMITIVE_RESTART_ENABLE:
        pipeline->m_stateCache.primitiveRestartEnabled =
            stateCache.primitiveRestartEnabled;
        break;
    case GfxPipelineState::LOGIC_OP:
        pipeline->m_stateCache.logicOp = stateCache.logicOp;
        break;
    case GfxPipelineState::LOGIC_OP_ENABLE:
        pipeline->m_stateCache.logicOpEnabled = stateCache.logicOpEnabled;
        break;
    case GfxPipelineState::COLOR_BLEND_ENABLE:
        pipeline->m_stateCache.colorBlendEnabled = stateCache.colorBlendEnabled;
        break;
    case GfxPipelineState::COLOR_BLEND_EQUATION:
        pipeline->m_stateCache.colorBlendEquation =
            stateCache.colorBlendEquation;
        break;
    case GfxPipelineState::COLOR_WRITE_MASK:
        pipeline->m_stateCache.colorWriteMask = stateCache.colorWriteMask;
        break;
    case GfxPipelineState::LINE_SMOOTH_ENABLE:
        pipeline->m_stateCache.lineSmoothEnabled = stateCache.lineSmoothEnabled;
        break;
    case GfxPipelineState::POLYGON_MODE:
        pipeline->m_stateCache.polygonMode = stateCache.polygonMode;
        break;
    default:
        // Unsupported state, do nothing
        break;
    }
}

const GfxPipelineStateCache& GfxPipelineStateController::getStateCache(
    GfxPipelineStateMachine stateMachine
) {
    return stateMachine->m_stateCache;
}

void GfxPipelineStateController::bindPipeline(
    GfxPipelineStateMachine stateMachine,
    GfxPipeline pipeline
) {
    if (stateMachine->m_currentBindingPipeline != pipeline)
        stateMachine->m_currentBindingPipeline = pipeline;

    for (const auto& state : pipeline->getDynamicStates()) {
        if (!compareState(state, stateMachine, pipeline->m_stateCache)) {
            switch (state) {
            case GfxPipelineState::VIEWPORT:
                stateMachine->setViewport(stateMachine->m_stateCache.viewport);
                break;
            case GfxPipelineState::SCISSOR:
                stateMachine->setScissor(stateMachine->m_stateCache.scissor);
                break;
            case GfxPipelineState::LINE_WIDTH:
                stateMachine->setLineWidth(stateMachine->m_stateCache.lineWidth);
                break;
            case GfxPipelineState::DEPTH_BIAS:
                stateMachine->setDepthBiasParams(stateMachine->m_stateCache.depthBiasParams);
                break;
            case GfxPipelineState::BLEND_CONSTANTS:
                stateMachine->setBlendConstants(stateMachine->m_stateCache.blendConstants);
                break;
            case GfxPipelineState::STENCIL_COMPARE_MASK:
            case GfxPipelineState::STENCIL_WRITE_MASK:
            case GfxPipelineState::STENCIL_REFERENCE:
            case GfxPipelineState::STENCIL_OP:
                stateMachine->setStencilOpParams(
                    GfxFaceSide::FRONT,
                    stateMachine->m_stateCache.frontFaceStencilOpParams
                );
                stateMachine->setStencilOpParams(
                    GfxFaceSide::BACK,
                    stateMachine->m_stateCache.backFaceStencilOpParams
                );
                break;
            case GfxPipelineState::CULL_MODE:
                stateMachine->setCullMode(stateMachine->m_stateCache.cullMode);
                break;
            case GfxPipelineState::FRONT_FACE:
                stateMachine->setFrontFace(stateMachine->m_stateCache.frontFace);
                break;
            case GfxPipelineState::PRIMITIVE_TOPOLOGY:
                stateMachine->setPrimitiveTopo(stateMachine->m_stateCache.primitiveTopo);
                break;
            case GfxPipelineState::DEPTH_TEST_ENABLE:
                stateMachine->setDepthTestEnabled(stateMachine->m_stateCache.depthTestEnabled);
                break;
            case GfxPipelineState::DEPTH_WRITE_ENABLE:
                stateMachine->setDepthWriteEnabled(stateMachine->m_stateCache.depthWriteEnabled);
                break;
            case GfxPipelineState::DEPTH_COMPARE_OP:
                stateMachine->setDepthCompareOp(stateMachine->m_stateCache.depthCompareOp);
                break;
            case GfxPipelineState::STENCIL_TEST_ENABLE:
                stateMachine->setStencilTestEnabled(stateMachine->m_stateCache.stencilTestEnabled);
                break;
            case GfxPipelineState::DEPTH_BIAS_ENABLE:
                stateMachine->setDepthBiasEnabled(stateMachine->m_stateCache.depthBiasEnabled);
                break;
            case GfxPipelineState::PRIMITIVE_RESTART_ENABLE:
                stateMachine->setPrimitiveRestartEnabled(
                    stateMachine->m_stateCache.primitiveRestartEnabled
                );
                break;
            case GfxPipelineState::LOGIC_OP:
                stateMachine->setLogicOp(stateMachine->m_stateCache.logicOp);
                break;
            case GfxPipelineState::LOGIC_OP_ENABLE:
                stateMachine->setLogicOpEnabled(stateMachine->m_stateCache.logicOpEnabled);
                break;
            case GfxPipelineState::COLOR_BLEND_ENABLE:
                stateMachine->setColorBlendEnabled(stateMachine->m_stateCache.colorBlendEnabled);
                break;
            case GfxPipelineState::COLOR_BLEND_EQUATION:
                stateMachine->setColorBlendEquation(stateMachine->m_stateCache.colorBlendEquation);
                break;
            case GfxPipelineState::COLOR_WRITE_MASK:
                stateMachine->setColorWriteMask(stateMachine->m_stateCache.colorWriteMask);
                break;
            case GfxPipelineState::LINE_SMOOTH_ENABLE:
                stateMachine->setLineSmoothEnabled(stateMachine->m_stateCache.lineSmoothEnabled);
                break;
            case GfxPipelineState::POLYGON_MODE:
                stateMachine->setPolygonMode(stateMachine->m_stateCache.polygonMode);
                break;
            default:
                // Unsupported state, do nothing
                break;
            }
        }
    }

    pipeline->m_stateCache = stateMachine->m_stateCache;
}

GfxPipeline GfxPipelineStateMachine_T::getCurrentBindingPipeline() const {
    return m_currentBindingPipeline;
}

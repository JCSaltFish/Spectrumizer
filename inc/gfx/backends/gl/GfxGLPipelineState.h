/**
 * @file GfxGLPipelineState.h
 * @brief OpenGL implementation of the GfxPipelineStateMachine interface.
 */

#pragma once

#include <glad/glad.h>

#include "gfx/GfxPr.h"

/**
 * @brief OpenGL implementation of GfxPipelineStateMachine.
 */
class GfxGLPipelineStateMachine : public GfxPipelineStateMachine_T {
public:
    void setViewport(const GfxViewport& viewport) override;
    void setScissor(const GfxRect& scissor) override;
    void setLineWidth(float lineWidth) override;
    void setLineSmoothEnabled(bool enabled) override;
    void setBlendConstants(const float blendConstants[4]) override;
    void setColorBlendEnabled(bool enabled) override;
    void setColorBlendEquation(const GfxBlendEquation& equation) override;
    void setColorWriteMask(unsigned int mask) override;
    void setDepthBiasEnabled(bool enabled) override;
    void setDepthBiasParams(const GfxDepthBiasParams& params) override;
    void setDepthTestEnabled(bool enabled) override;
    void setDepthWriteEnabled(bool enabled) override;
    void setDepthCompareOp(GfxCompareOp op) override;
    void setStencilTestEnabled(bool enabled) override;
    void setStencilOpParams(GfxFaceSide face, const GfxStencilOpParams& params) override;
    void setCullMode(GfxFaceSide mode) override;
    void setFrontFace(GfxFrontFace frontFace) override;
    void setPrimitiveTopo(GfxPrimitiveTopo topo) override;
    void setPrimitiveRestartEnabled(bool enabled) override;
    void setLogicOpEnabled(bool enabled) override;
    void setLogicOp(GfxLogicOp op) override;
    void setPolygonMode(GfxPolygonMode mode) override;
};

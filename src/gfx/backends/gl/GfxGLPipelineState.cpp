/**
 * @file GfxGLPipelineState.cpp
 * @brief OpenGL implementation of the GfxPipelineStateMachine interface.
 */

#include "gfx/backends/gl/GfxGLPipelineState.h"

#include "gfx/backends/gl/GfxGLTypeConverter.h"

void GfxGLPipelineStateMachine::setViewport(const GfxViewport& viewport) {
    m_stateCache.viewport = viewport;
    glViewport(
        static_cast<GLint>(viewport.x),
        static_cast<GLint>(viewport.y),
        static_cast<GLsizei>(viewport.width),
        static_cast<GLsizei>(viewport.height)
    );
    glDepthRange(viewport.minDepth, viewport.maxDepth);
}

void GfxGLPipelineStateMachine::setScissor(const GfxRect& scissor) {
    m_stateCache.scissor = scissor;
    glScissor(
        static_cast<GLint>(scissor.x),
        static_cast<GLint>(scissor.y),
        static_cast<GLsizei>(scissor.width),
        static_cast<GLsizei>(scissor.height)
    );
}

void GfxGLPipelineStateMachine::setLineWidth(float lineWidth) {
    m_stateCache.lineWidth = lineWidth;
    glLineWidth(lineWidth);
}

void GfxGLPipelineStateMachine::setLineSmoothEnabled(bool enabled) {
    m_stateCache.lineSmoothEnabled = enabled;
    if (enabled)
        glEnable(GL_LINE_SMOOTH);
    else
        glDisable(GL_LINE_SMOOTH);
}

void GfxGLPipelineStateMachine::setBlendConstants(const float blendConstants[4]) {
    m_stateCache.blendConstants[0] = blendConstants[0];
    m_stateCache.blendConstants[1] = blendConstants[1];
    m_stateCache.blendConstants[2] = blendConstants[2];
    m_stateCache.blendConstants[3] = blendConstants[3];
    glBlendColor(blendConstants[0], blendConstants[1], blendConstants[2], blendConstants[3]);
}

void GfxGLPipelineStateMachine::setColorBlendEnabled(bool enabled) {
    m_stateCache.colorBlendEnabled = enabled;
    if (enabled)
        glEnable(GL_BLEND);
    else
        glDisable(GL_BLEND);
}

void GfxGLPipelineStateMachine::setColorBlendEquation(const GfxBlendEquation& equation) {
    m_stateCache.colorBlendEquation = equation;
    glBlendEquationSeparate(
        GfxGLTypeConverter::toGLBlendOp(equation.colorBlendOp),
        GfxGLTypeConverter::toGLBlendOp(equation.alphaBlendOp)
    );
    glBlendFuncSeparate(
        GfxGLTypeConverter::toGLBlendFactor(equation.srcColorFactor),
        GfxGLTypeConverter::toGLBlendFactor(equation.dstColorFactor),
        GfxGLTypeConverter::toGLBlendFactor(equation.srcAlphaFactor),
        GfxGLTypeConverter::toGLBlendFactor(equation.dstAlphaFactor)
    );
}

void GfxGLPipelineStateMachine::setColorWriteMask(unsigned int mask) {
    m_stateCache.colorWriteMask = mask;
    glColorMask(
        (mask & GFX_COLOR_R) != 0,
        (mask & GFX_COLOR_G) != 0,
        (mask & GFX_COLOR_B) != 0,
        (mask & GFX_COLOR_A) != 0
    );
}

void GfxGLPipelineStateMachine::setDepthBiasEnabled(bool enabled) {
    m_stateCache.depthBiasEnabled = enabled;
    if (enabled)
        glEnable(GL_POLYGON_OFFSET_FILL);
    else
        glDisable(GL_POLYGON_OFFSET_FILL);
}

void GfxGLPipelineStateMachine::setDepthBiasParams(const GfxDepthBiasParams& params) {
    m_stateCache.depthBiasParams = params;
    glPolygonOffset(params.constantFactor, params.slopeFactor / 2.0f);
}

void GfxGLPipelineStateMachine::setDepthTestEnabled(bool enabled) {
    m_stateCache.depthTestEnabled = enabled;
    if (enabled)
        glEnable(GL_DEPTH_TEST);
    else
        glDisable(GL_DEPTH_TEST);
    glDepthMask(enabled ? GL_TRUE : GL_FALSE);
}

void GfxGLPipelineStateMachine::setDepthWriteEnabled(bool enabled) {
    m_stateCache.depthWriteEnabled = enabled;
    if (enabled)
        glDepthMask(GL_TRUE);
    else
        glDepthMask(GL_FALSE);
}

void GfxGLPipelineStateMachine::setDepthCompareOp(GfxCompareOp op) {
    m_stateCache.depthCompareOp = op;
    glDepthFunc(GfxGLTypeConverter::toGLCompareOp(op));
}

void GfxGLPipelineStateMachine::setStencilTestEnabled(bool enabled) {
    m_stateCache.stencilTestEnabled = enabled;
    if (enabled)
        glEnable(GL_STENCIL_TEST);
    else
        glDisable(GL_STENCIL_TEST);
}

void GfxGLPipelineStateMachine::setStencilOpParams(
    GfxFaceSide face,
    const GfxStencilOpParams& params
) {
    if (face == GfxFaceSide::FRONT)
        m_stateCache.frontFaceStencilOpParams = params;
    else if (face == GfxFaceSide::BACK)
        m_stateCache.backFaceStencilOpParams = params;
    else {
        m_stateCache.frontFaceStencilOpParams = params;
        m_stateCache.backFaceStencilOpParams = params;
    }

    GLenum glFace = (face == GfxFaceSide::FRONT) ? GL_FRONT : GL_BACK;
    glStencilOpSeparate(
        glFace,
        GfxGLTypeConverter::toGLStencilOp(params.failOp),
        GfxGLTypeConverter::toGLStencilOp(params.depthFailOp),
        GfxGLTypeConverter::toGLStencilOp(params.passOp)
    );
    glStencilFuncSeparate(
        glFace,
        GfxGLTypeConverter::toGLCompareOp(params.compareOp),
        params.reference,
        params.compareMask
    );
}

void GfxGLPipelineStateMachine::setCullMode(GfxFaceSide mode) {
    m_stateCache.cullMode = mode;
    if (mode == GfxFaceSide::NONE)
        glDisable(GL_CULL_FACE);
    else {
        glEnable(GL_CULL_FACE);
        glCullFace(GfxGLTypeConverter::toGLFaceSide(mode));
    }
}

void GfxGLPipelineStateMachine::setFrontFace(GfxFrontFace frontFace) {
    m_stateCache.frontFace = frontFace;
    if (frontFace == GfxFrontFace::COUNTER_CLOCKWISE)
        glFrontFace(GL_CCW);
    else
        glFrontFace(GL_CW);
}

void GfxGLPipelineStateMachine::setPrimitiveTopo(GfxPrimitiveTopo topo) {
    m_stateCache.primitiveTopo = topo;
}

void GfxGLPipelineStateMachine::setPrimitiveRestartEnabled(bool enabled) {
    m_stateCache.primitiveRestartEnabled = enabled;
    if (enabled)
        glEnable(GL_PRIMITIVE_RESTART);
    else
        glDisable(GL_PRIMITIVE_RESTART);
}

void GfxGLPipelineStateMachine::setLogicOpEnabled(bool enabled) {
    m_stateCache.logicOpEnabled = enabled;
    if (enabled)
        glEnable(GL_COLOR_LOGIC_OP);
    else
        glDisable(GL_COLOR_LOGIC_OP);
}

void GfxGLPipelineStateMachine::setLogicOp(GfxLogicOp op) {
    m_stateCache.logicOp = op;
    glLogicOp(GfxGLTypeConverter::toGLLogicOp(op));
}

void GfxGLPipelineStateMachine::setPolygonMode(GfxPolygonMode mode) {
    m_stateCache.polygonMode = mode;
    if (mode == GfxPolygonMode::FILL)
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    else if (mode == GfxPolygonMode::LINE)
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    else if (mode == GfxPolygonMode::POINT)
        glPolygonMode(GL_FRONT_AND_BACK, GL_POINT);
}

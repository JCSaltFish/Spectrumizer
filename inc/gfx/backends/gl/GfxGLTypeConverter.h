/**
 * @file GfxGLTypeConverter.h
 * @brief OpenGL type conversion utilities for Gfx types.
 */

#pragma once

#include <glad/glad.h>

#include "gfx/GfxPub.h"

/**
 * @brief Converts Gfx types to OpenGL types.
 */
class GfxGLTypeConverter {
public:
    /**
     * @brief Converts GfxFormat to OpenGL internal format.
     * @param format The GfxFormat to convert.
     * @return The corresponding OpenGL format.
     */
    static GLenum toGLInternalFormat(GfxFormat format);
    /**
     * @brief Converts GfxFormat to OpenGL format.
     * @param format The GfxFormat to convert.
     * @return The corresponding OpenGL format.
     */
    static GLenum toGLFormat(GfxFormat format);
    /**
     * @brief Converts GfxFormat to OpenGL type.
     * @param format The GfxFormat to convert.
     * @return The corresponding OpenGL type.
     */
    static GLenum toGLType(GfxFormat format);
    /**
     * @brief Gets the number of components for a given GfxFormat.
     * @param format The GfxFormat to check.
     * @return Number of components (e.g., 4 for RGBA).
     */
    static int toGLTypeSize(GfxFormat format);
    /**
     * @brief Converts GfxImageFilterMode to OpenGL filter mode.
     * @param filterMode The GfxImageFilterMode to convert.
     * @return The corresponding OpenGL filter mode.
     */
    static GLenum toGLFilterMode(GfxImageFilterMode filterMode);
    /**
     * @brief Converts GfxImageWrapMode to OpenGL wrap mode.
     * @param wrapMode The GfxImageWrapMode to convert.
     * @return The corresponding OpenGL wrap mode.
     */
    static GLenum toGLWrapMode(GfxImageWrapMode wrapMode);
    /**
     * @brief Converts GfxShaderStage to OpenGL shader type.
     * @param stage The GfxShaderStage to convert.
     * @return The corresponding OpenGL shader type.
     */
    static GLenum toGLShaderType(GfxShaderStage stage);
    /**
     * @brief Converts GfxBlendFactor to OpenGL blend factor.
     * @param factor The GfxBlendFactor to convert.
     * @return The corresponding OpenGL blend factor.
     */
    static GLenum toGLBlendFactor(GfxBlendFactor factor);
    /**
     * @brief Converts GfxBlendOp to OpenGL blend operation.
     * @param op The GfxBlendOp to convert.
     * @return The corresponding OpenGL blend operation.
     */
    static GLenum toGLBlendOp(GfxBlendOp op);
    /**
     * @brief Converts GfxCompareOp to OpenGL compare operation.
     * @param op The GfxCompareOp to convert.
     * @return The corresponding OpenGL compare operation.
     */
    static GLenum toGLCompareOp(GfxCompareOp op);
    /**
     * @brief Converts GfxFaceSide to OpenGL face side.
     * @param side The GfxFaceSide to convert.
     * @return The corresponding OpenGL face side.
     */
    static GLenum toGLFaceSide(GfxFaceSide side);
    /**
     * @brief Converts GfxStencilOp to OpenGL stencil operation.
     * @param op The GfxStencilOp to convert.
     * @return The corresponding OpenGL stencil operation.
     */
    static GLenum toGLStencilOp(GfxStencilOp op);
    /**
     * @brief Converts GfxFrontFace to OpenGL front face.
     * @param frontFace The GfxFrontFace to convert.
     * @return The corresponding OpenGL front face.
     */
    static GLenum toGLFrontFace(GfxFrontFace frontFace);
    /**
     * @brief Converts GfxPrimitiveTopo to OpenGL draw mode.
     * @param topo The GfxPrimitiveTopo to convert.
     * @return The corresponding OpenGL draw mode.
     */
    static GLenum toGLDrawMode(GfxPrimitiveTopo topo);
    /**
     * @brief Converts GfxLogicOp to OpenGL logic operation.
     * @param op The GfxLogicOp to convert.
     * @return The corresponding OpenGL logic operation.
     */
    static GLenum toGLLogicOp(GfxLogicOp op);
    /**
     * @brief Converts GfxPolygonMode to OpenGL polygon mode.
     * @param mode The GfxPolygonMode to convert.
     * @return The corresponding OpenGL polygon mode.
     */
    static GLenum toGLPolygonMode(GfxPolygonMode mode);
};

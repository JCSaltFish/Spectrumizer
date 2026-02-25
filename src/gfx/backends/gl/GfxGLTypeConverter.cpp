/**
 * @file GfxGLTypeConverter.cpp
 * @brief OpenGL type conversion utilities for Gfx types.
 */

#include <gfx/backends/gl/GfxGLTypeConverter.h>

GLenum GfxGLTypeConverter::toGLInternalFormat(GfxFormat format) {
    switch (format) {
    case GfxFormat::UNDEFINED:
        return GL_NONE;
    case GfxFormat::R32_SFLOAT:
        return GL_R32F;
    case GfxFormat::R32G32_SFLOAT:
        return GL_RG32F;
    case GfxFormat::R32G32B32_SFLOAT:
        return GL_RGB32F;
    case GfxFormat::R32G32B32A32_SFLOAT:
        return GL_RGBA32F;
    case GfxFormat::R8_UNORM:
        return GL_R8;
    case GfxFormat::R8G8B8A8_UNORM:
        return GL_RGBA8;
    case GfxFormat::R8_SNORM:
        return GL_R8_SNORM;
    case GfxFormat::R8G8B8A8_SNORM:
        return GL_RGBA8_SNORM;
    case GfxFormat::D32_SFLOAT:
        return GL_DEPTH_COMPONENT32F;
    case GfxFormat::D24_UNORM_S8_UINT:
        return GL_DEPTH24_STENCIL8;
    case GfxFormat::R32_UINT:
        return GL_R32UI;
    case GfxFormat::R32G32_UINT:
        return GL_RG32UI;
    case GfxFormat::R32_SINT:
        return GL_R32I;
    case GfxFormat::R32G32_SINT:
        return GL_RG32I;
    default:
        return GL_NONE; // Unsupported format
    }
}

GLenum GfxGLTypeConverter::toGLFormat(GfxFormat format) {
    switch (format) {
    case GfxFormat::R32_SFLOAT:
    case GfxFormat::R32_UINT:
    case GfxFormat::R32_SINT:
        return GL_RED;
    case GfxFormat::R32G32_SFLOAT:
    case GfxFormat::R32G32_UINT:
    case GfxFormat::R32G32_SINT:
        return GL_RG;
    case GfxFormat::R32G32B32_SFLOAT:
        return GL_RGB;
    case GfxFormat::R32G32B32A32_SFLOAT:
        return GL_RGBA;
    case GfxFormat::R8_UNORM:
    case GfxFormat::R8_SNORM:
        return GL_RED;
    case GfxFormat::R8G8B8A8_UNORM:
    case GfxFormat::R8G8B8A8_SNORM:
        return GL_RGBA;
    case GfxFormat::D32_SFLOAT:
        return GL_DEPTH_COMPONENT;
    case GfxFormat::D24_UNORM_S8_UINT:
        return GL_DEPTH_STENCIL;
    default:
        return GL_NONE; // Unsupported format
    }
}

GLenum GfxGLTypeConverter::toGLType(GfxFormat format) {
    switch (format) {
    case GfxFormat::R32_SFLOAT:
    case GfxFormat::R32G32_SFLOAT:
    case GfxFormat::R32G32B32_SFLOAT:
    case GfxFormat::R32G32B32A32_SFLOAT:
    case GfxFormat::D32_SFLOAT:
        return GL_FLOAT;
    case GfxFormat::R8_UNORM:
    case GfxFormat::R8G8B8A8_UNORM:
        return GL_UNSIGNED_BYTE;
    case GfxFormat::R8_SNORM:
    case GfxFormat::R8G8B8A8_SNORM:
        return GL_BYTE;
    case GfxFormat::D24_UNORM_S8_UINT:
        return GL_UNSIGNED_INT_24_8;
    case GfxFormat::R32_UINT:
    case GfxFormat::R32G32_UINT:
        return GL_UNSIGNED_INT;
    case GfxFormat::R32_SINT:
    case GfxFormat::R32G32_SINT:
        return GL_INT;
    default:
        return GL_NONE; // Unsupported format
    }
}

int GfxGLTypeConverter::toGLTypeSize(GfxFormat format) {
    switch (format) {
    case GfxFormat::R32_SFLOAT:
    case GfxFormat::R32_UINT:
    case GfxFormat::R32_SINT:
    case GfxFormat::R8_UNORM:
    case GfxFormat::R8_SNORM:
    case GfxFormat::D32_SFLOAT:
        return 1;
    case GfxFormat::R32G32_SFLOAT:
    case GfxFormat::R32G32_UINT:
    case GfxFormat::R32G32_SINT:
        return 2;
    case GfxFormat::R32G32B32_SFLOAT:
        return 3;
    case GfxFormat::R32G32B32A32_SFLOAT:
    case GfxFormat::R8G8B8A8_UNORM:
    case GfxFormat::R8G8B8A8_SNORM:
        return 4;
    default:
        return 0; // Unsupported format
    }
}

GLenum GfxGLTypeConverter::toGLFilterMode(GfxImageFilterMode filterMode) {
    switch (filterMode) {
    case GfxImageFilterMode::NEAREST:
        return GL_NEAREST;
    case GfxImageFilterMode::LINEAR:
        return GL_LINEAR;
    default:
        return GL_NONE; // Unsupported filter mode
    }
}

GLenum GfxGLTypeConverter::toGLWrapMode(GfxImageWrapMode wrapMode) {
    switch (wrapMode) {
    case GfxImageWrapMode::REPEAT:
        return GL_REPEAT;
    case GfxImageWrapMode::MIRRORED_REPEAT:
        return GL_MIRRORED_REPEAT;
    case GfxImageWrapMode::CLAMP_TO_EDGE:
        return GL_CLAMP_TO_EDGE;
    case GfxImageWrapMode::CLAMP_TO_BORDER:
        return GL_CLAMP_TO_BORDER;
    default:
        return GL_NONE; // Unsupported wrap mode
    }
}

GLenum GfxGLTypeConverter::toGLShaderType(GfxShaderStage stage) {
    switch (stage) {
    case GfxShaderStage::VERTEX:
        return GL_VERTEX_SHADER;
    case GfxShaderStage::FRAGMENT:
        return GL_FRAGMENT_SHADER;
    case GfxShaderStage::GEOMETRY:
        return GL_GEOMETRY_SHADER;
    case GfxShaderStage::TESS_CTRL:
        return GL_TESS_CONTROL_SHADER;
    case GfxShaderStage::TESS_EVAL:
        return GL_TESS_EVALUATION_SHADER;
    case GfxShaderStage::COMPUTE:
        return GL_COMPUTE_SHADER;
    default:
        return GL_NONE; // Unsupported shader stage
    }
}

GLenum GfxGLTypeConverter::toGLBlendFactor(GfxBlendFactor factor) {
    switch (factor) {
    case GfxBlendFactor::ZERO:
        return GL_ZERO;
    case GfxBlendFactor::ONE:
        return GL_ONE;
    case GfxBlendFactor::SRC_COLOR:
        return GL_SRC_COLOR;
    case GfxBlendFactor::ONE_MINUS_SRC_COLOR:
        return GL_ONE_MINUS_SRC_COLOR;
    case GfxBlendFactor::DST_COLOR:
        return GL_DST_COLOR;
    case GfxBlendFactor::ONE_MINUS_DST_COLOR:
        return GL_ONE_MINUS_DST_COLOR;
    case GfxBlendFactor::SRC_ALPHA:
        return GL_SRC_ALPHA;
    case GfxBlendFactor::ONE_MINUS_SRC_ALPHA:
        return GL_ONE_MINUS_SRC_ALPHA;
    case GfxBlendFactor::DST_ALPHA:
        return GL_DST_ALPHA;
    case GfxBlendFactor::ONE_MINUS_DST_ALPHA:
        return GL_ONE_MINUS_DST_ALPHA;
    case GfxBlendFactor::CONSTANT_COLOR:
        return GL_CONSTANT_COLOR;
    case GfxBlendFactor::ONE_MINUS_CONSTANT_COLOR:
        return GL_ONE_MINUS_CONSTANT_COLOR;
    case GfxBlendFactor::CONSTANT_ALPHA:
        return GL_CONSTANT_ALPHA;
    case GfxBlendFactor::ONE_MINUS_CONSTANT_ALPHA:
        return GL_ONE_MINUS_CONSTANT_ALPHA;
    case GfxBlendFactor::SRC1_COLOR:
        return GL_SRC1_COLOR;
    case GfxBlendFactor::ONE_MINUS_SRC1_COLOR:
        return GL_ONE_MINUS_SRC1_COLOR;
    case GfxBlendFactor::SRC1_ALPHA:
        return GL_SRC1_ALPHA;
    case GfxBlendFactor::ONE_MINUS_SRC1_ALPHA:
        return GL_ONE_MINUS_SRC1_ALPHA;
    case GfxBlendFactor::SRC_ALPHA_SATURATE:
        return GL_SRC_ALPHA_SATURATE;
    default:
        return GL_NONE; // Unsupported blend factor
    }
}

GLenum GfxGLTypeConverter::toGLBlendOp(GfxBlendOp op) {
    switch (op) {
    case GfxBlendOp::ADD:
        return GL_FUNC_ADD;
    case GfxBlendOp::SUBTRACT:
        return GL_FUNC_SUBTRACT;
    case GfxBlendOp::REVERSE_SUBTRACT:
        return GL_FUNC_REVERSE_SUBTRACT;
    case GfxBlendOp::MIN:
        return GL_MIN;
    case GfxBlendOp::MAX:
        return GL_MAX;
    default:
        return GL_NONE; // Unsupported blend operation
    }
}

GLenum GfxGLTypeConverter::toGLCompareOp(GfxCompareOp op) {
    switch (op) {
    case GfxCompareOp::NEVER:
        return GL_NEVER;
    case GfxCompareOp::LESS:
        return GL_LESS;
    case GfxCompareOp::EQUAL:
        return GL_EQUAL;
    case GfxCompareOp::LESS_OR_EQUAL:
        return GL_LEQUAL;
    case GfxCompareOp::GREATER:
        return GL_GREATER;
    case GfxCompareOp::NOT_EQUAL:
        return GL_NOTEQUAL;
    case GfxCompareOp::GREATER_OR_EQUAL:
        return GL_GEQUAL;
    case GfxCompareOp::ALWAYS:
        return GL_ALWAYS;
    default:
        return GL_NONE; // Unsupported compare operation
    }
}

GLenum GfxGLTypeConverter::toGLFaceSide(GfxFaceSide side) {
    switch (side) {
    case GfxFaceSide::NONE:
        return GL_NONE;
    case GfxFaceSide::FRONT:
        return GL_FRONT;
    case GfxFaceSide::BACK:
        return GL_BACK;
    case GfxFaceSide::FRONT_AND_BACK:
        return GL_FRONT_AND_BACK;
    default:
        return GL_NONE; // Unsupported face side
    }
}

GLenum GfxGLTypeConverter::toGLStencilOp(GfxStencilOp op) {
    switch (op) {
    case GfxStencilOp::KEEP:
        return GL_KEEP;
    case GfxStencilOp::ZERO:
        return GL_ZERO;
    case GfxStencilOp::REPLACE:
        return GL_REPLACE;
    case GfxStencilOp::INCREMENT_AND_CLAMP:
        return GL_INCR;
    case GfxStencilOp::DECREMENT_AND_CLAMP:
        return GL_DECR;
    case GfxStencilOp::INVERT:
        return GL_INVERT;
    case GfxStencilOp::INCREMENT_AND_WRAP:
        return GL_INCR_WRAP;
    case GfxStencilOp::DECREMENT_AND_WRAP:
        return GL_DECR_WRAP;
    default:
        return GL_NONE; // Unsupported stencil operation
    }
}

GLenum GfxGLTypeConverter::toGLFrontFace(GfxFrontFace frontFace) {
    switch (frontFace) {
    case GfxFrontFace::COUNTER_CLOCKWISE:
        return GL_CCW;
    case GfxFrontFace::CLOCKWISE:
        return GL_CW;
    default:
        return GL_NONE; // Unsupported front face
    }
}

GLenum GfxGLTypeConverter::toGLDrawMode(GfxPrimitiveTopo topo) {
    switch (topo) {
    case GfxPrimitiveTopo::POINT_LIST:
        return GL_POINTS;
    case GfxPrimitiveTopo::LINE_LIST:
        return GL_LINES;
    case GfxPrimitiveTopo::LINE_STRIP:
        return GL_LINE_STRIP;
    case GfxPrimitiveTopo::TRIANGLE_LIST:
        return GL_TRIANGLES;
    case GfxPrimitiveTopo::TRIANGLE_STRIP:
        return GL_TRIANGLE_STRIP;
    case GfxPrimitiveTopo::TRIANGLE_FAN:
        return GL_TRIANGLE_FAN;
    case GfxPrimitiveTopo::PATCH_LIST:
        return GL_PATCHES;
    default:
        return GL_NONE; // Unsupported primitive topology
    }
}

GLenum GfxGLTypeConverter::toGLLogicOp(GfxLogicOp op) {
    switch (op) {
    case GfxLogicOp::CLEAR:
        return GL_CLEAR;
    case GfxLogicOp::SET:
        return GL_SET;
    case GfxLogicOp::COPY:
        return GL_COPY;
    case GfxLogicOp::COPY_INVERTED:
        return GL_COPY_INVERTED;
    case GfxLogicOp::NO_OP:
        return GL_NOOP;
    case GfxLogicOp::INVERT:
        return GL_INVERT;
    case GfxLogicOp::AND:
        return GL_AND;
    case GfxLogicOp::NAND:
        return GL_NAND;
    case GfxLogicOp::OR:
        return GL_OR;
    case GfxLogicOp::NOR:
        return GL_NOR;
    case GfxLogicOp::XOR:
        return GL_XOR;
    case GfxLogicOp::EQUIV:
        return GL_EQUIV;
    default:
        return GL_NONE; // Unsupported logic operation
    }
}

GLenum GfxGLTypeConverter::toGLPolygonMode(GfxPolygonMode mode) {
    switch (mode) {
    case GfxPolygonMode::FILL:
        return GL_FILL;
    case GfxPolygonMode::LINE:
        return GL_LINE;
    case GfxPolygonMode::POINT:
        return GL_POINT;
    default:
        return GL_NONE; // Unsupported polygon mode
    }
}

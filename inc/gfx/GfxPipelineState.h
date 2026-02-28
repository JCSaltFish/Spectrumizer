/**
 * @file GfxPipelineState.h
 * @brief Defines the graphics pipeline state machine and its associated structures and enums.
 */

#pragma once

#include "GfxCommon.h"

/**
 * @brief Enum representing various graphics pipeline states.
 */
enum class GfxPipelineState {
    VIEWPORT,
    SCISSOR,
    LINE_WIDTH,
    DEPTH_BIAS,
    BLEND_CONSTANTS,
    STENCIL_COMPARE_MASK,
    STENCIL_WRITE_MASK,
    STENCIL_REFERENCE,
    CULL_MODE,
    FRONT_FACE,
    PRIMITIVE_TOPOLOGY,
    DEPTH_TEST_ENABLE,
    DEPTH_WRITE_ENABLE,
    DEPTH_COMPARE_OP,
    STENCIL_TEST_ENABLE,
    STENCIL_OP,
    DEPTH_BIAS_ENABLE,
    PRIMITIVE_RESTART_ENABLE,
    LOGIC_OP,
    POLYGON_MODE,
    LOGIC_OP_ENABLE,
    COLOR_BLEND_ENABLE,
    COLOR_BLEND_EQUATION,
    COLOR_WRITE_MASK,
    LINE_SMOOTH_ENABLE,
};

/**
 * @brief Structure representing a viewport in the graphics pipeline.
 */
struct GfxViewport {
    int x = 0; // X coordinate of the viewport
    int y = 0; // Y coordinate of the viewport
    int width = 1; // Width of the viewport
    int height = 1; // Height of the viewport
    float minDepth = 0.0f; // Minimum depth of the viewport
    float maxDepth = 1.0f; // Maximum depth of the viewport
};

/**
 * @brief Enum representing various blend factors used in the graphics pipeline.
 * @note These are used to control how colors are blended together.
 */
enum class GfxBlendFactor {
    ZERO,
    ONE,
    SRC_COLOR,
    ONE_MINUS_SRC_COLOR,
    DST_COLOR,
    ONE_MINUS_DST_COLOR,
    SRC_ALPHA,
    ONE_MINUS_SRC_ALPHA,
    DST_ALPHA,
    ONE_MINUS_DST_ALPHA,
    CONSTANT_COLOR,
    ONE_MINUS_CONSTANT_COLOR,
    CONSTANT_ALPHA,
    ONE_MINUS_CONSTANT_ALPHA,
    SRC_ALPHA_SATURATE,
    SRC1_COLOR,
    ONE_MINUS_SRC1_COLOR,
    SRC1_ALPHA,
    ONE_MINUS_SRC1_ALPHA,
};
/**
 * @brief Enum representing various blend operations used in the graphics pipeline.
 * @note These operations define how source and destination colors are combined.
 */
enum class GfxBlendOp {
    ADD,
    SUBTRACT,
    REVERSE_SUBTRACT,
    MIN,
    MAX,
};
/**
 * @brief Structure representing blend equations for color and alpha blending.
 * @note This structure defines how source and destination colors are blended together.
 */
struct GfxBlendEquation {
    GfxBlendFactor srcColorFactor = GfxBlendFactor::ONE; // Source color blend factor
    GfxBlendFactor dstColorFactor = GfxBlendFactor::ZERO; // Destination color blend factor
    GfxBlendOp colorBlendOp = GfxBlendOp::ADD; // Color blend operation
    GfxBlendFactor srcAlphaFactor = GfxBlendFactor::ONE; // Source alpha blend factor
    GfxBlendFactor dstAlphaFactor = GfxBlendFactor::ZERO; // Destination alpha blend factor
    GfxBlendOp alphaBlendOp = GfxBlendOp::ADD; // Alpha blend operation
};

constexpr unsigned int GFX_COLOR_R = 1 << 0;
constexpr unsigned int GFX_COLOR_G = 1 << 1;
constexpr unsigned int GFX_COLOR_B = 1 << 2;
constexpr unsigned int GFX_COLOR_A = 1 << 3;

/**
 * @brief Structure representing depth bias parameters in the graphics pipeline.
 * @note These parameters are used to adjust the depth values during rendering.
 */
struct GfxDepthBiasParams {
    float constantFactor = 0.0f; // Constant bias factor
    float clamp = 0.0f; // Maximum depth bias value
    float slopeFactor = 0.0f; // Slope-scaled depth bias factor
};

/**
 * @brief Enum representing the comparison operations used in the graphics pipeline.
 * @note These operations are used for depth and stencil testing.
 */
enum class GfxCompareOp {
    NEVER,
    LESS,
    EQUAL,
    LESS_OR_EQUAL,
    GREATER,
    NOT_EQUAL,
    GREATER_OR_EQUAL,
    ALWAYS,
};

/**
 * @brief Enum representing the sides of a face in the graphics pipeline.
 * @note This is used for culling and stencil operations.
 */
enum class GfxFaceSide {
    NONE,
    FRONT,
    BACK,
    FRONT_AND_BACK,
};

/**
 * @brief Enum representing stencil operations in the graphics pipeline.
 * @note These operations define how stencil values are modified during rendering.
 */
enum class GfxStencilOp {
    KEEP,
    ZERO,
    REPLACE,
    INCREMENT_AND_CLAMP,
    DECREMENT_AND_CLAMP,
    INVERT,
    INCREMENT_AND_WRAP,
    DECREMENT_AND_WRAP,
};
/**
 * @brief Structure representing stencil operation parameters in the graphics pipeline.
 * @note This structure defines how stencil values are compared and modified.
 */
struct GfxStencilOpParams {
    GfxStencilOp failOp = GfxStencilOp::KEEP; // Operation when stencil test fails
    GfxStencilOp passOp = GfxStencilOp::KEEP; // Operation when stencil test passes
    GfxStencilOp depthFailOp = GfxStencilOp::KEEP; // Operation when depth test fails
    GfxCompareOp compareOp = GfxCompareOp::ALWAYS; // Comparison operation for stencil test
    unsigned int compareMask = 0xFFFFFFFF; // Mask for stencil comparison
    unsigned int writeMask = 0xFFFFFFFF; // Mask for stencil writing
    unsigned int reference = 0; // Reference value for stencil comparison
};

/**
 * @brief Enum representing the front face orientation in the graphics pipeline.
 * @note This is used to determine the winding order of vertices for face culling.
 */
enum class GfxFrontFace {
    COUNTER_CLOCKWISE,
    CLOCKWISE,
};

/**
 * @brief Enum representing the primitive topology used in the graphics pipeline.
 * @note This defines how vertices are assembled into geometric primitives.
 */
enum class GfxPrimitiveTopo {
    POINT_LIST,
    LINE_LIST,
    LINE_STRIP,
    TRIANGLE_LIST,
    TRIANGLE_STRIP,
    TRIANGLE_FAN,
    LINE_LIST_WITH_ADJACENCY,
    LINE_STRIP_WITH_ADJACENCY,
    TRIANGLE_LIST_WITH_ADJACENCY,
    TRIANGLE_STRIP_WITH_ADJACENCY,
    PATCH_LIST,
};

/**
 * @brief Enum representing logical operations used in the graphics pipeline.
 * @note These operations are applied to the color buffer during rendering.
 */
enum class GfxLogicOp {
    CLEAR,
    AND,
    AND_REVERSE,
    COPY,
    AND_INVERTED,
    NO_OP,
    XOR,
    OR,
    NOR,
    EQUIV,
    INVERT,
    OR_REVERSE,
    COPY_INVERTED,
    OR_INVERTED,
    NAND,
    SET
};

/**
 * @brief Enum representing polygon modes used in the graphics pipeline.
 * @note These modes define how polygons are rendered (filled, outlined, or as points).
 */
enum class GfxPolygonMode {
    FILL,
    LINE,
    POINT,
};

/**
 * @brief Structure representing the cache of graphics pipeline states.
 * @note This structure holds various settings that can be dynamically changed during rendering.
 */
struct GfxPipelineStateCache {
    GfxViewport viewport = {}; // Viewport settings for rendering
    GfxRect scissor = {}; // Scissor rectangle for clipping
    float lineWidth = 1.0f; // Width of lines when rendering
    bool lineSmoothEnabled = false; // Whether line smoothing is enabled
    float blendConstants[4] = { 0.0f, 0.0f, 0.0f, 0.0f }; // Blend constants for color blending
    bool colorBlendEnabled = false; // Whether color blending is enabled
    GfxBlendEquation colorBlendEquation = {}; // Color blend equation for blending colors
    unsigned int colorWriteMask = 0xF; // Color write mask for color attachments (R, G, B, A)
    bool depthBiasEnabled = false; // Whether depth bias is enabled
    GfxDepthBiasParams depthBiasParams = {}; // Depth bias parameters for adjusting depth values
    bool depthTestEnabled = false; // Whether depth testing is enabled
    bool depthWriteEnabled = true; // Whether depth writing is enabled
    GfxCompareOp depthCompareOp = GfxCompareOp::LESS; // Depth comparison operation for depth testing
    bool stencilTestEnabled = false; // Whether stencil testing is enabled
    GfxStencilOpParams frontFaceStencilOpParams = {}; // Stencil operation parameters for front faces
    GfxStencilOpParams backFaceStencilOpParams = {}; // Stencil operation parameters for back faces
    GfxFaceSide cullMode = GfxFaceSide::NONE; // Culling mode for face culling
    GfxFrontFace frontFace = GfxFrontFace::COUNTER_CLOCKWISE; // Front face orientation
    GfxPrimitiveTopo primitiveTopo = GfxPrimitiveTopo::TRIANGLE_LIST; // Primitive topology
    bool primitiveRestartEnabled = false; // Whether primitive restart is enabled
    bool logicOpEnabled = false; // Whether logical operations are enabled
    GfxLogicOp logicOp = GfxLogicOp::NO_OP; // Logical operation to apply to the color buffer
    GfxPolygonMode polygonMode = GfxPolygonMode::FILL; // Polygon mode for rendering
};

class GfxPipeline_T;
using GfxPipeline = std::shared_ptr<GfxPipeline_T>;
class GfxPipelineStateController;

/**
 * @brief Abstract base class for the graphics pipeline state machine.
 * @note This class defines the interface for managing various graphics pipeline states.
 */
class GfxPipelineStateMachine_T {
    friend class GfxPipelineStateController;

public:
    /**
     * @breif Get the currently bound graphics pipeline.
     * @return The currently bound graphics pipeline.
     */
    GfxPipeline getCurrentBindingPipeline() const;

    /**
     * @brief Set the viewport for the graphics pipeline.
     * @param viewport The viewport to set.
     */
    virtual void setViewport(const GfxViewport& viewport) = 0;
    /**
     * @brief Set the scissor rectangle for the graphics pipeline.
     * @param scissor The scissor rectangle to set.
     */
    virtual void setScissor(const GfxRect& scissor) = 0;
    /**
     * @brief Set the line width for the graphics pipeline.
     * @param lineWidth The width of lines to set.
     */
    virtual void setLineWidth(float lineWidth) = 0;
    /**
     * @brief Enable or disable line smoothing for the graphics pipeline.
     * @param enabled Whether to enable line smoothing.
     */
    virtual void setLineSmoothEnabled(bool enabled) = 0;
    /**
     * @brief Set the blend constants for the graphics pipeline.
     * @param blendConstants Array of four float values representing blend constants.
     */
    virtual void setBlendConstants(const float blendConstants[4]) = 0;
    /**
     * @brief Enable or disable color blending for the graphics pipeline.
     * @param enabled Whether to enable color blending.
     */
    virtual void setColorBlendEnabled(bool enabled) = 0;
    /**
     * @brief Set the color blend equation for the graphics pipeline.
     * @param equation The blend equation to set.
     */
    virtual void setColorBlendEquation(const GfxBlendEquation& equation) = 0;
    /**
     * @brief Set the color write mask for the graphics pipeline.
     * @param mask Bitmask representing which color channels to write (R, G, B, A).
     */
    virtual void setColorWriteMask(unsigned int mask) = 0;
    /**
     * @brief Enable or disable depth bias for the graphics pipeline.
     * @param enabled Whether to enable depth bias.
     */
    virtual void setDepthBiasEnabled(bool enabled) = 0;
    /**
     * @brief Set the depth bias parameters for the graphics pipeline.
     * @param params The depth bias parameters to set.
     */
    virtual void setDepthBiasParams(const GfxDepthBiasParams& params) = 0;
    /**
     * @brief Enable or disable depth testing for the graphics pipeline.
     * @param enabled Whether to enable depth testing.
     */
    virtual void setDepthTestEnabled(bool enabled) = 0;
    /**
     * @brief Enable or disable depth writing for the graphics pipeline.
     * @param enabled Whether to enable depth writing.
     */
    virtual void setDepthWriteEnabled(bool enabled) = 0;
    /**
     * @brief Set the depth comparison operation for the graphics pipeline.
     * @param op The comparison operation to set.
     */
    virtual void setDepthCompareOp(GfxCompareOp op) = 0;
    /**
     * @brief Enable or disable stencil testing for the graphics pipeline.
     * @param enabled Whether to enable stencil testing.
     */
    virtual void setStencilTestEnabled(bool enabled) = 0;
    /**
     * @brief Set the stencil operation parameters for a specific face side.
     * @param face The face side (front, back, or both) to set the parameters for.
     * @param params The stencil operation parameters to set.
     */
    virtual void setStencilOpParams(GfxFaceSide face, const GfxStencilOpParams& params) = 0;
    /**
     * @brief Set the culling mode for the graphics pipeline.
     * @param mode The culling mode to set (none, front, back, or both).
     */
    virtual void setCullMode(GfxFaceSide mode) = 0;
    /**
     * @brief Set the front face orientation for the graphics pipeline.
     * @param frontFace The front face orientation to set (counter-clockwise or clockwise).
     */
    virtual void setFrontFace(GfxFrontFace frontFace) = 0;
    /**
     * @brief Set the primitive topology for the graphics pipeline.
     * @param topo The primitive topology to set (e.g., triangle list, line strip).
     */
    virtual void setPrimitiveTopo(GfxPrimitiveTopo topo) = 0;
    /**
     * @brief Enable or disable primitive restart for the graphics pipeline.
     * @param enabled Whether to enable primitive restart.
     */
    virtual void setPrimitiveRestartEnabled(bool enabled) = 0;
    /**
     * @brief Enable or disable logical operations for the graphics pipeline.
     * @param enabled Whether to enable logical operations.
     */
    virtual void setLogicOpEnabled(bool enabled) = 0;
    /**
     * @brief Set the logical operation for the graphics pipeline.
     * @param op The logical operation to set (e.g., clear, and, or).
     */
    virtual void setLogicOp(GfxLogicOp op) = 0;
    /**
     * @brief Set the polygon mode for the graphics pipeline.
     * @param mode The polygon mode to set (fill, line, or point).
     */
    virtual void setPolygonMode(GfxPolygonMode mode) = 0;

protected:
    GfxPipelineStateCache m_stateCache; // Current pipeline state cache
    GfxPipeline m_currentBindingPipeline = nullptr; // Currently bound pipeline
};
using GfxPipelineStateMachine = std::shared_ptr<GfxPipelineStateMachine_T>;

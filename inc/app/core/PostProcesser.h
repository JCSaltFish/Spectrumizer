/**
 * @file PostProcesser.h
 * @brief Header file for the PostProcesser class.
 */

#pragma once

#include "gfx/GfxPub.h"

/**
 * @brief Class for post-processing rendered images.
 */
class PostProcesser {
public:
    explicit PostProcesser(GfxRenderer& renderer) : m_renderer(renderer) {};

    /**
     * @brief Initialize the post-processor.
     * @return 0 on success, non-zero on failure.
     */
    int init();
    /**
     * @brief Terminate the post-processor and release resources.
     */
    void term();

    /**
     * @brief Initialize the post-processor for a new frame.
     * @param width Width of the frame.
     * @param height Height of the frame.
     * @param inputImages Array of input images (e.g., front and back buffers).
     * @return 0 on success, non-zero on failure.
     */
    int initFrame(int width, int height, const std::array<GfxBuffer, 2>& inputImages);

    /**
     * @brief Set the input image for post-processing.
     * @param image Input image to be processed.
     */
    void setInputImage(GfxBuffer& image);
    /**
     * @brief Get the output image after post-processing.
     * @return Output image, or a default texture if the output image is not available.
     */
    GfxImage getOutputImage() const;

    /**
     * @brief Set the display channel for the output image.
     * @param channel The display channel to set.
     */
    void setDisplayChannel(int channel);

    /** 
     * @brief Render a frame using the post-processor.
     * @return 0 on success, non-zero on failure.
     */
    int renderFrame();

private:
    GfxRenderer m_renderer = nullptr; // Reference to the graphics renderer

    GfxFramebuffer m_framebuffer = nullptr; // Framebuffer for rendering
    std::array<GfxBuffer, 2> m_inputImages = {}; // Input images for post-processing
    GfxBuffer m_currentInputImage = nullptr; // Current input image for post-processing
    GfxImage m_outputImage = nullptr; // Output image after post-processing
    GfxRenderPass m_renderPass = nullptr; // Render pass for post-processing
    GfxPipeline m_pipeline = nullptr; // Graphics pipeline for post-processing
    // Descriptor set bindings for post-processing
    std::array<GfxDescriptorSetBinding, 2> m_descriptorSetBindings = {};

    GfxBuffer m_uboParams = nullptr; // Uniform buffer for post-processing parameters
    /**
     * @brief Struct for storing uniform parameters for post-processing.
     */
    struct UParams {
        int channel = 0; // Display channel
        int resX = 0; // Horizontal resolution
        int resY = 0; // Vertical resolution
    };

    GfxShader m_vertexShader = nullptr; // Vertex shader
    GfxShader m_fragmentShader = nullptr; // Fragment shader

    GfxDescriptor b_radiances = {}; // Descriptor for input radiances
    GfxDescriptor u_params = {}; // Descriptor for parameters

    bool frameInitiated = false; // Flag indicating if the frame has been initiated

    int m_resolutionX = 0; // Horizontal resolution
    int m_resolutionY = 0; // Vertical resolution

    int m_dispChannel = 0; // Display channel
};

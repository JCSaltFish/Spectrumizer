/**
 * @file PostProcesser.cpp
 * @brief Implementation file for the PostProcesser class.
 */

#include "app/core/PostProcesser.h"

#include "app/AppTextureManager.h"
#include "utils/Logger.hpp"
#include "res/ShaderStrings.hpp"

int PostProcesser::init() {
    if (!m_renderer) {
        Logger() << "Renderer is null in Previewer::init";
        return 1;
    }

    // Load shaders
    try {
        m_vertexShader = m_renderer->createShader(
            GfxShaderStage::VERTEX,
            ShaderStrings::QUAD_VERT
        );
    } catch (GfxShaderException& e) {
        Logger() << "Failed to create vertex shader in PostProcesser::init: " << e.what();
        return 1;
    }
    try {
        m_fragmentShader = m_renderer->createShader(
            GfxShaderStage::FRAGMENT,
            ShaderStrings::QUAD_FRAG
        );
    } catch (GfxShaderException& e) {
        Logger() << "Failed to create fragment shader in PostProcesser::init: " << e.what();
        return 1;
    }

    // Initialize descriptors and UBOs
    b_radiances.binding = 0;
    b_radiances.type = GfxDescriptorType::STORAGE_BUFFER;
    b_radiances.stages.set(GfxShaderStage::FRAGMENT);

    u_params.binding = 1;
    u_params.type = GfxDescriptorType::UNIFORM_BUFFER;
    u_params.stages.set(GfxShaderStage::FRAGMENT);
    m_uboParams = m_renderer->createBuffer(
        sizeof(UParams),
        GfxBufferUsage::UNIFORM_BUFFER,
        GfxBufferProp::DYNAMIC
    );
    if (m_uboParams == nullptr) {
        Logger() << "Failed to create UBO for parameters in PostProcesser::init";
        return 1;
    }

    return 0;
}

void PostProcesser::term() {
    if (!m_renderer)
        return;

    m_renderer->waitDeviceIdle();

    m_renderer->destroyShader(m_vertexShader);
    m_vertexShader = nullptr;
    m_renderer->destroyShader(m_fragmentShader);
    m_fragmentShader = nullptr;

    m_renderer->destroyBuffer(m_uboParams);
    m_uboParams = nullptr;

    if (frameInitiated) {
        m_renderer->destroyImage(m_outputImage);
        m_outputImage = nullptr;
        m_renderer->destroyFramebuffer(m_framebuffer);
        m_framebuffer = nullptr;
        m_renderer->destroyRenderPass(m_renderPass);
        m_renderPass = nullptr;
        m_renderer->destroyPipeline(m_pipeline);
        m_pipeline = nullptr;

        m_renderer->destroyDescriptorSetBinding(m_descriptorSetBindings[0]);
        m_renderer->destroyDescriptorSetBinding(m_descriptorSetBindings[1]);
        m_descriptorSetBindings = {};

        frameInitiated = false;
    }

    m_resolutionX = 0;
    m_resolutionY = 0;
}

int PostProcesser::initFrame(
    int width,
    int height,
    const std::array<GfxBuffer, 2>& inputImages
) {
    m_resolutionX = width;
    m_resolutionY = height;
    m_inputImages = inputImages;

    m_renderer->waitDeviceIdle();

    // Create render pass
    GfxAttachment colorAttachment = {};
    colorAttachment.format = GfxFormat::R8G8B8A8_UNORM;
    colorAttachment.usages.set(GfxImageUsage::COLOR_ATTACHMENT);

    if (m_renderPass)
        m_renderer->destroyRenderPass(m_renderPass);
    m_renderPass = m_renderer->createRenderPass({ colorAttachment });
    if (!m_renderPass) {
        Logger() << "Failed to create render pass in PostProcesser::initFrame";
        return 1;
    }

    // Create pipeline
    std::vector<GfxPipelineState> dynamicStates = {
        GfxPipelineState::VIEWPORT,
        GfxPipelineState::SCISSOR,
        GfxPipelineState::PRIMITIVE_TOPOLOGY,
    };

    if (m_pipeline)
        m_renderer->destroyPipeline(m_pipeline);
    m_pipeline = m_renderer->createPipeline(
        { m_vertexShader, m_fragmentShader },
        { { b_radiances, u_params } },
        {},
        dynamicStates,
        m_renderPass
    );
    if (!m_pipeline) {
        Logger() << "Failed to create pipeline in PostProcesser::initFrame";
        return 1;
    }

    // Create descriptor set bindings
    if (m_descriptorSetBindings[0])
        m_renderer->destroyDescriptorSetBinding(m_descriptorSetBindings[0]);
    m_descriptorSetBindings[0] = m_renderer->createDescriptorSetBinding(
        m_pipeline,
        0,
        {
            { b_radiances, m_inputImages[0] },
            { u_params, m_uboParams }
        }
    );
    if (m_descriptorSetBindings[1])
        m_renderer->destroyDescriptorSetBinding(m_descriptorSetBindings[1]);
    m_descriptorSetBindings[1] = m_renderer->createDescriptorSetBinding(
        m_pipeline,
        0,
        {
            { b_radiances, m_inputImages[1] },
            { u_params, m_uboParams }
        }
    );

    // Create output image
    if (m_outputImage)
        m_renderer->destroyImage(m_outputImage);
    GfxImageInfo colorInfo = {};
    colorInfo.width = width;
    colorInfo.height = height;
    colorInfo.format = GfxFormat::R8G8B8A8_UNORM;
    colorInfo.usages.set(GfxImageUsage::COLOR_ATTACHMENT);
    colorInfo.usages.set(GfxImageUsage::SAMPLED_TEXTURE);
    m_outputImage = m_renderer->createImage(colorInfo);
    if (!m_outputImage) {
        Logger() << "Failed to create output image in PostProcesser::initFrame";
        return 1;
    }

    // Framebuffer
    if (m_framebuffer)
        m_renderer->destroyFramebuffer(m_framebuffer);
    m_framebuffer = m_renderer->createFramebuffer(m_renderPass, { m_outputImage });
    if (!m_framebuffer) {
        Logger() << "Failed to create framebuffer in PostProcesser::initFrame";
        return 1;
    }

    frameInitiated = true;
    return 0;
}

void PostProcesser::setInputImage(GfxBuffer& image) {
    m_currentInputImage = image;
}

GfxImage PostProcesser::getOutputImage() const {
    if (m_outputImage)
        return m_outputImage;
    else
        return AppTextureManager::instance().getDefaultTexture();
}

void PostProcesser::setDisplayChannel(int channel) {
    m_dispChannel = channel;
}

int PostProcesser::renderFrame() {
    if (!frameInitiated)
        return 1;

    m_renderer->beginRenderPass(m_framebuffer);
    m_renderer->bindPipeline(m_pipeline);

    m_renderer->getPipelineStateMachine()->setViewport({ 0, 0, m_resolutionX, m_resolutionY });
    m_renderer->getPipelineStateMachine()->setScissor({ 0, 0, m_resolutionX, m_resolutionY });

    m_renderer->clearColorAttachment(0, { 0.0f, 0.0f, 0.0f, 1.0f });

    m_renderer->getPipelineStateMachine()->setPrimitiveTopo(GfxPrimitiveTopo::TRIANGLE_STRIP);

    UParams u_params = {};
    u_params.channel = m_dispChannel;
    u_params.resX = m_resolutionX;
    u_params.resY = m_resolutionY;
    if (m_renderer->updateBufferData(m_uboParams, 0, sizeof(UParams), &u_params))
        return 1;

    if (m_currentInputImage == m_inputImages[0])
        m_renderer->bindDescriptorSetBinding(m_descriptorSetBindings[0]);
    else
        m_renderer->bindDescriptorSetBinding(m_descriptorSetBindings[1]);

    m_renderer->draw(4);

    m_renderer->endRenderPass();
    return 0;
}

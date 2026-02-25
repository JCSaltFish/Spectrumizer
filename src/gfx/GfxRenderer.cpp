/**
 * @file GfxRenderer.cpp
 * @brief Implementation of the GfxRenderer class.
 */

#include "gfx/GfxPr.h"

#include "gfx/backends/gl/GfxGLRenderer.h"
#include "gfx/backends/vulkan/GfxVulkanRenderer.h"

GfxBackend GfxRendererInterface::getBackend() const {
    return m_backend;
}

GfxPipelineStateMachine GfxRendererInterface::getPipelineStateMachine() const {
    return m_pipelineStateMachine;
}

GfxRendererFactory::~GfxRendererFactory() {
    if (m_initialized[GfxBackend::OpenGL]) {
        GfxGLRenderer::termGlobal();
        m_initialized[GfxBackend::OpenGL] = false;
    }
    if (m_initialized[GfxBackend::Vulkan]) {
        GfxVulkanRenderer::termGlobal();
        m_initialized[GfxBackend::Vulkan] = false;
    }
}

int GfxRendererFactory::initGlobal(GfxBackend backend, const GfxRendererConfig& config) {
    switch (backend) {
    case GfxBackend::OpenGL:
    {
        if (m_initialized[GfxBackend::OpenGL])
            return 0; // OpenGL backend already initialized
        m_initialized[GfxBackend::OpenGL] = !GfxGLRenderer::initGlobal(config);
        return m_initialized[GfxBackend::OpenGL] ? 0 : 1;
    }
    case GfxBackend::Vulkan:
    {
        if (m_initialized[GfxBackend::Vulkan])
            return 0; // Vulkan backend already initialized
        m_initialized[GfxBackend::Vulkan] = !GfxVulkanRenderer::initGlobal(config);
        return m_initialized[GfxBackend::Vulkan] ? 0 : 1;
    }
    default:
        return 1; // Unsupported backend
    }
}

void GfxRendererFactory::termGlobal(GfxBackend backend) {
    switch (backend) {
    case GfxBackend::OpenGL:
    {
        if (m_initialized[GfxBackend::OpenGL]) {
            GfxGLRenderer::termGlobal();
            m_initialized[GfxBackend::OpenGL] = false;
        }
        break;
    }
    case GfxBackend::Vulkan:
    {
        if (m_initialized[GfxBackend::Vulkan]) {
            GfxVulkanRenderer::termGlobal();
            m_initialized[GfxBackend::Vulkan] = false;
        }
        break;
    }
    default:
        break; // Unsupported backend
    }
}

GfxRenderer GfxRendererFactory::create(GfxBackend backend) {
    switch (backend) {
    case GfxBackend::OpenGL:
    {
        if (!m_initialized[GfxBackend::OpenGL])
            return nullptr; // OpenGL backend not initialized
        return std::shared_ptr<GfxGLRenderer>(new GfxGLRenderer());
    }
    case GfxBackend::Vulkan:
    {
        if (!m_initialized[GfxBackend::Vulkan])
            return nullptr; // Vulkan backend not initialized
        return std::shared_ptr<GfxVulkanRenderer>(new GfxVulkanRenderer());
    }
    default:
        return nullptr; // Unsupported backend
    }
}

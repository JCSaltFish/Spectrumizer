/**
 * @file GfxGLRenderer.cpp
 * @brief OpenGL implementation of the GfxRenderer interface.
 */

#include "gfx/backends/gl/GfxGLRenderer.h"

#include "gfx/backends/gl/GfxGLTypeConverter.h"
#include "gfx/backends/gl/GfxGLPipelineState.h"

#ifdef _DEBUG
#include <iostream>
#define ENABLE_DEBUG_OUTPUT
#endif // _DEBUG

std::mutex GfxGLRenderer::s_mutex; // Mutex for global OpenGL renderer

GfxGLRenderer::GfxGLRenderer() {
    m_backend = GfxBackend::OpenGL;
    m_pipelineStateMachine = std::make_shared<GfxGLPipelineStateMachine>();
}

#ifdef ENABLE_DEBUG_OUTPUT
static void GLAPIENTRY debugCallback(
    GLenum source,
    GLenum type,
    GLuint id,
    GLenum severity,
    GLsizei length,
    const GLchar* message,
    const void* userParam
) {
    // Ignore non-significant error/warning codes
    if (id == 131169 || id == 131185 || id == 131218 || id == 131204)
        return;
    std::cerr << "---------------" << std::endl;
    std::cerr << "Debug message (" << id << "): " << message << std::endl;
    switch (source) {
    case GL_DEBUG_SOURCE_API:
        std::cerr << "Source: API";
        break;
    case GL_DEBUG_SOURCE_WINDOW_SYSTEM:
        std::cerr << "Source: Window System";
        break;
    case GL_DEBUG_SOURCE_SHADER_COMPILER:
        std::cerr << "Source: Shader Compiler";
        break;
    case GL_DEBUG_SOURCE_THIRD_PARTY:
        std::cerr << "Source: Third Party";
        break;
    case GL_DEBUG_SOURCE_APPLICATION:
        std::cerr << "Source: Application";
        break;
    case GL_DEBUG_SOURCE_OTHER:
        std::cerr << "Source: Other";
        break;
    }
    std::cerr << std::endl;
    switch (type) {
    case GL_DEBUG_TYPE_ERROR:
        std::cerr << "Type: Error";
        break;
    case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR:
        std::cerr << "Type: Deprecated Behaviour";
        break;
    case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:
        std::cerr << "Type: Undefined Behaviour";
        break;
    case GL_DEBUG_TYPE_PORTABILITY:
        std::cerr << "Type: Portability";
        break;
    case GL_DEBUG_TYPE_PERFORMANCE:
        std::cerr << "Type: Performance";
        break;
    case GL_DEBUG_TYPE_MARKER:
        std::cerr << "Type: Marker";
        break;
    case GL_DEBUG_TYPE_PUSH_GROUP:
        std::cerr << "Type: Push Group";
        break;
    case GL_DEBUG_TYPE_POP_GROUP:
        std::cerr << "Type: Pop Group";
        break;
    case GL_DEBUG_TYPE_OTHER:
        std::cerr << "Type: Other";
        break;
    }
    std::cerr << std::endl;
    switch (severity) {
    case GL_DEBUG_SEVERITY_HIGH:
        std::cerr << "Severity: high";
        break;
    case GL_DEBUG_SEVERITY_MEDIUM:
        std::cerr << "Severity: medium";
        break;
    case GL_DEBUG_SEVERITY_LOW:
        std::cerr << "Severity: low";
        break;
    case GL_DEBUG_SEVERITY_NOTIFICATION:
        std::cerr << "Severity: notification";
        break;
    }
    std::cerr << std::endl;
    std::cerr << std::endl;
}
#endif // ENABLE_DEBUG_OUTPUT

int GfxGLRenderer::initGlobal(const GfxRendererConfig& config) {
    const GfxGLRendererConfig* glConfig = std::get_if<GfxGLRendererConfig>(&config);
    if (glConfig == nullptr || glConfig->proc == nullptr)
        return 1; // Invalid configuration
    std::lock_guard<std::mutex> lock(s_mutex);
    if (gladLoadGLLoader(reinterpret_cast<GLADloadproc>(glConfig->proc)) == 0)
        return 1; // Failed to initialize GLAD
    glEnable(GL_MULTISAMPLE);
#ifdef ENABLE_DEBUG_OUTPUT
    glEnable(GL_DEBUG_OUTPUT);
    glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
    glDebugMessageCallback(debugCallback, nullptr);
    glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, nullptr, GL_TRUE);
#endif // ENABLE_DEBUG_OUTPUT
    return 0;
}

void GfxGLRenderer::waitDeviceIdle() const {
    glFinish();
}

int GfxGLRenderer::initForImGui(const std::function<void(void*)>& initFunc) {
    initFunc(const_cast<void*>(static_cast<const void*>("#version 450")));
    return 0;
}

void GfxGLRenderer::termForImGui(const std::function<void()>& termFunc) {
    termFunc();
}

void GfxGLRenderer::renderForImGui(const std::function<void(void*)>& renderFunc) const {
    renderFunc(nullptr);
}

GfxRendererInterface::ImageInfo GfxGLRenderer::getImageInfo(const GfxImage& image) const {
    std::shared_ptr<GfxGLImage> glImage = std::static_pointer_cast<GfxGLImage>(image);
    return glImage->m_texture;
}

GfxImage GfxGLRenderer::createImage(const GfxImageInfo& info) const {
    GfxGLImage* image_ptr = new GfxGLImage(info);
    GfxImage image(
        image_ptr,
        [this](GfxImage_T* img) {
            this->destroyImage(GfxImage(img, [](GfxImage_T*) {}));
        }
    );
    std::shared_ptr<GfxGLImage> glImage = std::static_pointer_cast<GfxGLImage>(image);

    glImage->m_samples = info.samples;
    GLenum target = (info.samples > 1) ? GL_TEXTURE_2D_MULTISAMPLE : GL_TEXTURE_2D;

    glGenTextures(1, &glImage->m_texture);
    glBindTexture(target, glImage->m_texture);
    if (info.samples > 1) {
        glTexImage2DMultisample(
            target,
            info.samples,
            GfxGLTypeConverter::toGLInternalFormat(info.format),
            info.width,
            info.height,
            GL_TRUE // Fixed sample locations
        );
        glBindTexture(target, 0);
    } else {
        glTexImage2D(
            target,
            0,
            GfxGLTypeConverter::toGLInternalFormat(info.format),
            info.width,
            info.height,
            0,
            GfxGLTypeConverter::toGLFormat(info.format),
            GfxGLTypeConverter::toGLType(info.format),
            nullptr
        );

        GLuint minFilter = GfxGLTypeConverter::toGLFilterMode(info.minFilter);
        if (info.maxLod - info.minLod > 0) {
            if (minFilter == GL_LINEAR && info.mipmapFilter == GfxImageFilterMode::LINEAR)
                minFilter = GL_LINEAR_MIPMAP_LINEAR;
            else if (minFilter == GL_NEAREST && info.mipmapFilter == GfxImageFilterMode::LINEAR)
                minFilter = GL_NEAREST_MIPMAP_LINEAR;
            else if (minFilter == GL_LINEAR && info.mipmapFilter == GfxImageFilterMode::NEAREST)
                minFilter = GL_LINEAR_MIPMAP_NEAREST;
            else if (minFilter == GL_NEAREST && info.mipmapFilter == GfxImageFilterMode::NEAREST)
                minFilter = GL_NEAREST_MIPMAP_NEAREST;
        }
        GLuint magFilter = GfxGLTypeConverter::toGLFilterMode(info.magFilter);
        GLuint wrapS = GfxGLTypeConverter::toGLWrapMode(info.wrapS);
        GLuint wrapT = GfxGLTypeConverter::toGLWrapMode(info.wrapT);
        glTexParameteri(target, GL_TEXTURE_MIN_FILTER, minFilter);
        glTexParameteri(target, GL_TEXTURE_MAG_FILTER, magFilter);
        glTexParameteri(target, GL_TEXTURE_WRAP_S, wrapS);
        glTexParameteri(target, GL_TEXTURE_WRAP_T, wrapT);
        glTexParameteri(target, GL_TEXTURE_BASE_LEVEL, info.minLod);
        glTexParameteri(target, GL_TEXTURE_MAX_LEVEL, info.maxLod);
        glTexParameterf(target, GL_TEXTURE_LOD_BIAS, info.lodBias);
    }
    glBindTexture(target, 0);

    return image;
}

int GfxGLRenderer::setImageData(const GfxImage& image, void* data) const {
    std::shared_ptr<GfxGLImage> glImage = std::static_pointer_cast<GfxGLImage>(image);
    GLenum target = (glImage->m_samples > 1) ? GL_TEXTURE_2D_MULTISAMPLE : GL_TEXTURE_2D;
    glBindTexture(target, glImage->m_texture);
    glTexSubImage2D(
        target,
        0,
        0,
        0,
        glImage->getWidth(),
        glImage->getHeight(),
        GfxGLTypeConverter::toGLFormat(glImage->getFormat()),
        GfxGLTypeConverter::toGLType(glImage->getFormat()),
        data
    );
    glGenerateMipmap(target);
    glBindTexture(target, 0);
    return 0;
}

int GfxGLRenderer::getImageData(const GfxImage& image, void* data) const {
    std::shared_ptr<GfxGLImage> glImage = std::static_pointer_cast<GfxGLImage>(image);
    GLenum target = (glImage->m_samples > 1) ? GL_TEXTURE_2D_MULTISAMPLE : GL_TEXTURE_2D;
    glBindTexture(target, glImage->m_texture);
    glGetTexImage(
        target,
        0,
        GfxGLTypeConverter::toGLFormat(glImage->getFormat()),
        GfxGLTypeConverter::toGLType(glImage->getFormat()),
        data
    );
    glBindTexture(target, 0);
    return 0;
}

int GfxGLRenderer::generateMipmaps(const GfxImage& image) const {
    std::shared_ptr<GfxGLImage> glImage = std::static_pointer_cast<GfxGLImage>(image);
    GLenum target = (glImage->m_samples > 1) ? GL_TEXTURE_2D_MULTISAMPLE : GL_TEXTURE_2D;
    glBindTexture(target, glImage->m_texture);
    glGenerateMipmap(target);
    glBindTexture(target, 0);
    return 0;
}

void GfxGLRenderer::copyImage(const GfxImage& src, const GfxImage& dst, int width, int height) {
    std::shared_ptr<GfxGLImage> glImageSrc = std::static_pointer_cast<GfxGLImage>(src);
    std::shared_ptr<GfxGLImage> glImageDst = std::static_pointer_cast<GfxGLImage>(dst);

    GLuint fboSrc = 0;
    glGenFramebuffers(1, &fboSrc);
    glBindFramebuffer(GL_READ_FRAMEBUFFER, fboSrc);
    GLenum targetSrc = (glImageSrc->m_samples > 1) ? GL_TEXTURE_2D_MULTISAMPLE : GL_TEXTURE_2D;
    glFramebufferTexture2D(
        GL_READ_FRAMEBUFFER,
        GL_COLOR_ATTACHMENT0,
        targetSrc,
        glImageSrc->m_texture,
        0
    );

    GLuint fboDst = 0;
    glGenFramebuffers(1, &fboDst);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fboDst);
    GLenum targetDst = (glImageDst->m_samples > 1) ? GL_TEXTURE_2D_MULTISAMPLE : GL_TEXTURE_2D;
    glFramebufferTexture2D(
        GL_DRAW_FRAMEBUFFER,
        GL_COLOR_ATTACHMENT0,
        targetDst,
        glImageDst->m_texture,
        0
    );

    glBlitFramebuffer(
        0,
        0,
        width,
        height,
        0,
        0,
        width,
        height,
        GL_COLOR_BUFFER_BIT,
        GL_NEAREST
    );

    glDeleteFramebuffers(1, &fboSrc);
    glDeleteFramebuffers(1, &fboDst);
}

void GfxGLRenderer::destroyImage(const GfxImage& image) const {
    std::shared_ptr<GfxGLImage> glImage = std::static_pointer_cast<GfxGLImage>(image);
    if (glImage->m_texture != 0) {
        glDeleteTextures(1, &glImage->m_texture);
        glImage->m_texture = 0;
    }
}

GfxRenderPass GfxGLRenderer::createRenderPass(
    const std::vector<GfxAttachment>& colorAttachments,
    const GfxAttachment& depthAttachment
) const {
    GfxRenderPass renderPass =
        std::make_shared<GfxGLRenderPass>(colorAttachments, depthAttachment);
    std::shared_ptr<GfxGLRenderPass> glRenderPass =
        std::static_pointer_cast<GfxGLRenderPass>(renderPass);

    glGenFramebuffers(1, &glRenderPass->m_fbo);
    for (int i = 0; i < renderPass->getColorAttachments().size(); i++) {
        GfxAttachment colorAttachment = renderPass->getColorAttachments()[i];
        if (glRenderPass->m_samples == 0)
            glRenderPass->m_samples = colorAttachment.samples;
        else if (glRenderPass->m_samples != colorAttachment.samples)
            return nullptr; // Error: Inconsistent sample counts in color attachments
    }
    glBindFramebuffer(GL_FRAMEBUFFER, glRenderPass->m_fbo);
    std::vector<GLenum> drawBuffers{};
    drawBuffers.reserve(colorAttachments.size());
    for (int i = 0; i < colorAttachments.size(); i++)
        drawBuffers.push_back(GL_COLOR_ATTACHMENT0 + i);
    glDrawBuffers(static_cast<GLsizei>(drawBuffers.size()), drawBuffers.data());
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    if (depthAttachment.usages.check(GfxImageUsage::DEPTH_ATTACHMENT)) {
        if (glRenderPass->m_samples == 0)
            glRenderPass->m_samples = depthAttachment.samples;
        else if (glRenderPass->m_samples != depthAttachment.samples)
            return nullptr; // Error: Inconsistent sample counts in depth attachment
    }

    if (glRenderPass->m_samples > 1) {
        glGenFramebuffers(1, &glRenderPass->m_msaaFbo);
        glBindFramebuffer(GL_FRAMEBUFFER, glRenderPass->m_msaaFbo);
        glDrawBuffers(static_cast<GLsizei>(drawBuffers.size()), drawBuffers.data());
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

    return renderPass;
}

void GfxGLRenderer::destroyRenderPass(const GfxRenderPass& renderPass) const {
    std::shared_ptr<GfxGLRenderPass> glRenderPass =
        std::static_pointer_cast<GfxGLRenderPass>(renderPass);
    if (glRenderPass->m_fbo != 0) {
        glDeleteFramebuffers(1, &glRenderPass->m_fbo);
        glRenderPass->m_fbo = 0;
    }
    if (glRenderPass->m_msaaFbo != 0) {
        glDeleteFramebuffers(1, &glRenderPass->m_msaaFbo);
        glRenderPass->m_msaaFbo = 0;
    }
}

GfxFramebuffer GfxGLRenderer::createFramebuffer
(
    const GfxRenderPass& renderPass,
    const std::vector<GfxImage>& colorImages,
    const GfxImage& depthImage,
    const std::vector<GfxImage>& colorResolveImages
) const {
    GfxFramebuffer framebuffer = std::make_shared<GfxFramebuffer_T>(
            renderPass,
            colorImages,
            depthImage,
            colorResolveImages
        );
    std::shared_ptr<GfxGLRenderPass> glRenderPass =
        std::static_pointer_cast<GfxGLRenderPass>(renderPass);

    if (colorImages.size() != renderPass->getColorAttachments().size())
        return nullptr; // Error: Mismatched number of color images

    if (depthImage) {
        GfxAttachment depthAttachment = renderPass->getDepthAttachment();
        if (!depthAttachment.usages.check(GfxImageUsage::DEPTH_ATTACHMENT))
            return nullptr; // Error: Render pass has no depth attachment
    }

    if (glRenderPass->m_samples > 1) {
        if (colorResolveImages.size() != renderPass->getColorAttachments().size())
            return nullptr; // Error: Mismatched number of resolve images
    }

    return framebuffer;
}

int GfxGLRenderer::readFramebufferColorAttachmentPixels(
    const GfxFramebuffer& framebuffer,
    int index,
    const GfxRect& rect,
    void* pixels
) const {
    if (index < 0 || index >= framebuffer->getColorImages().size())
        return 1; // Error: Invalid color attachment index

    std::shared_ptr<GfxGLRenderPass> glRenderPass =
        std::static_pointer_cast<GfxGLRenderPass>(framebuffer->getRenderPass());
    std::shared_ptr<GfxGLImage> glImage =
        std::static_pointer_cast<GfxGLImage>(framebuffer->getColorImages()[index]);
    if (glImage->m_samples > 1) {
        glImage =
            std::static_pointer_cast<GfxGLImage>(framebuffer->getColorResolveImages()[index]);
    }

    glBindFramebuffer(GL_FRAMEBUFFER, glRenderPass->m_fbo);
    glFramebufferTexture2D(
        GL_FRAMEBUFFER,
        GL_COLOR_ATTACHMENT0 + index,
        GL_TEXTURE_2D,
        glImage->m_texture,
        glRenderPass->getColorAttachments()[index].level
    );
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        return 1; // Error: Incomplete framebuffer
    }

    glReadBuffer(GL_COLOR_ATTACHMENT0 + index);
    glReadPixels(
        rect.x,
        rect.y,
        rect.width,
        rect.height,
        GfxGLTypeConverter::toGLFormat(glImage->getFormat()),
        GfxGLTypeConverter::toGLType(glImage->getFormat()),
        pixels
    );

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    return 0;
}

GfxBuffer GfxGLRenderer::createBuffer(
    int size,
    GfxBufferUsage usage,
    GfxBufferProp prop
) const {
    GfxBuffer buffer = std::make_shared<GfxGLBuffer>(size, usage, prop);
    std::shared_ptr<GfxGLBuffer> glBuffer = std::static_pointer_cast<GfxGLBuffer>(buffer);
    glGenBuffers(1, &glBuffer->m_buffer);
    return buffer;
}

int GfxGLRenderer::setBufferData(const GfxBuffer& buffer, int size, const void* data) const {
    std::shared_ptr<GfxGLBuffer> glBuffer = std::static_pointer_cast<GfxGLBuffer>(buffer);

    GLenum usage =
        buffer->getProp() == GfxBufferProp::STATIC ? GL_STATIC_DRAW : GL_DYNAMIC_DRAW;

    GLenum target = GL_ARRAY_BUFFER;
    if (buffer->getUsage() == GfxBufferUsage::UNIFORM_BUFFER)
        target = GL_UNIFORM_BUFFER;
    else if (buffer->getUsage() == GfxBufferUsage::STORAGE_BUFFER)
        target = GL_SHADER_STORAGE_BUFFER;

    glBindBuffer(target, glBuffer->m_buffer);
    glBufferData(target, size, data, usage);

    return 0;
}

int GfxGLRenderer::updateBufferData(
    const GfxBuffer& buffer,
    int offset,
    int size,
    const void* data
) const {
    std::shared_ptr<GfxGLBuffer> glBuffer = std::static_pointer_cast<GfxGLBuffer>(buffer);

    GLenum usage =
        buffer->getProp() == GfxBufferProp::STATIC ? GL_STATIC_DRAW : GL_DYNAMIC_DRAW;

    GLenum target = GL_ARRAY_BUFFER;
    if (buffer->getUsage() == GfxBufferUsage::UNIFORM_BUFFER)
        target = GL_UNIFORM_BUFFER;
    else if (buffer->getUsage() == GfxBufferUsage::STORAGE_BUFFER)
        target = GL_SHADER_STORAGE_BUFFER;

    glBindBuffer(target, glBuffer->m_buffer);
    glBufferData(target, buffer->getSize(), nullptr, usage);
    glBufferSubData(target, offset, size, data);

    return 0;
}

void GfxGLRenderer::destroyBuffer(const GfxBuffer& buffer) const {
    std::shared_ptr<GfxGLBuffer> glBuffer = std::static_pointer_cast<GfxGLBuffer>(buffer);
    if (glBuffer->m_buffer != 0) {
        glDeleteBuffers(1, &glBuffer->m_buffer);
        glBuffer->m_buffer = 0;
    }
}

int GfxGLRenderer::readBufferData(
    const GfxBuffer& buffer,
    int offset,
    int size,
    void* data
) const {
    std::shared_ptr<GfxGLBuffer> glBuffer = std::static_pointer_cast<GfxGLBuffer>(buffer);
    GLenum target = GL_ARRAY_BUFFER;
    if (buffer->getUsage() == GfxBufferUsage::UNIFORM_BUFFER)
        target = GL_UNIFORM_BUFFER;
    else if (buffer->getUsage() == GfxBufferUsage::STORAGE_BUFFER)
        target = GL_SHADER_STORAGE_BUFFER;
    glBindBuffer(target, glBuffer->m_buffer);
    glGetBufferSubData(target, offset, size, data);
    return 0;
}

int GfxGLRenderer::copyBuffer(
    const GfxBuffer& src,
    const GfxBuffer& dst,
    int srcOffset,
    int dstOffset,
    int size
) const {
    std::shared_ptr<GfxGLBuffer> glBufferSrc = std::static_pointer_cast<GfxGLBuffer>(src);
    std::shared_ptr<GfxGLBuffer> glBufferDst = std::static_pointer_cast<GfxGLBuffer>(dst);
    GLenum targetSrc = GL_ARRAY_BUFFER;
    if (src->getUsage() == GfxBufferUsage::UNIFORM_BUFFER)
        targetSrc = GL_UNIFORM_BUFFER;
    else if (src->getUsage() == GfxBufferUsage::STORAGE_BUFFER)
        targetSrc = GL_SHADER_STORAGE_BUFFER;
    GLenum targetDst = GL_ARRAY_BUFFER;
    if (dst->getUsage() == GfxBufferUsage::UNIFORM_BUFFER)
        targetDst = GL_UNIFORM_BUFFER;
    else if (dst->getUsage() == GfxBufferUsage::STORAGE_BUFFER)
        targetDst = GL_SHADER_STORAGE_BUFFER;
    glBindBuffer(GL_COPY_READ_BUFFER, glBufferSrc->m_buffer);
    glBindBuffer(GL_COPY_WRITE_BUFFER, glBufferDst->m_buffer);
    glCopyBufferSubData(GL_COPY_READ_BUFFER, GL_COPY_WRITE_BUFFER, srcOffset, dstOffset, size);
    return 0;
}

GfxVAO GfxGLRenderer::createVAO(
    const GfxVertexDesc& vertexDesc,
    const GfxBuffer& vertexBuffer,
    const GfxBuffer& indexBuffer
) const {
    GfxVAO vao = std::make_shared<GfxGLVAO>(vertexDesc, vertexBuffer, indexBuffer);
    std::shared_ptr<GfxGLVAO> glVAO = std::static_pointer_cast<GfxGLVAO>(vao);
    std::shared_ptr<GfxGLBuffer> glVertexBuffer =
        std::static_pointer_cast<GfxGLBuffer>(vertexBuffer);
    std::shared_ptr<GfxGLBuffer> glIndexBuffer =
        std::static_pointer_cast<GfxGLBuffer>(indexBuffer);

    glGenVertexArrays(1, &glVAO->m_vao);
    glBindVertexArray(glVAO->m_vao);
    if (vertexBuffer) {
        glBindBuffer(GL_ARRAY_BUFFER, glVertexBuffer->m_buffer);
        for (const auto& attr : vertexDesc.attributes) {
            glEnableVertexAttribArray(attr.location);
            glVertexAttribPointer(
                attr.location,
                GfxGLTypeConverter::toGLTypeSize(attr.format),
                GfxGLTypeConverter::toGLType(attr.format),
                GL_FALSE,
                vertexDesc.stride,
                reinterpret_cast<void*>(static_cast<uintptr_t>(attr.offset))
            );
        }
    }
    if (indexBuffer)
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, glIndexBuffer->m_buffer);
    glBindVertexArray(0);

    return vao;
}

void GfxGLRenderer::destroyVAO(const GfxVAO& vao) const {
    std::shared_ptr<GfxGLVAO> glVAO = std::static_pointer_cast<GfxGLVAO>(vao);
    if (glVAO->m_vao != 0) {
        glDeleteVertexArrays(1, &glVAO->m_vao);
        glVAO->m_vao = 0;
    }
}

GfxShader GfxGLRenderer::createShader(
    GfxShaderStage stage,
    const std::string& source
) const {
    GfxShader shader = std::make_shared<GfxGLShader>(stage);
    std::shared_ptr<GfxGLShader> glShader = std::static_pointer_cast<GfxGLShader>(shader);

    glShader->m_shader = glCreateShader(GfxGLTypeConverter::toGLShaderType(stage));
    const char* src = source.c_str();
    glShaderSource(glShader->m_shader, 1, &src, nullptr);
    glCompileShader(glShader->m_shader);

    GLint success = 0;
    glGetShaderiv(glShader->m_shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        GLint logLength = 0;
        glGetShaderiv(glShader->m_shader, GL_INFO_LOG_LENGTH, &logLength);
        std::string infoLog(logLength, ' ');
        glGetShaderInfoLog(glShader->m_shader, logLength, nullptr, infoLog.data());
        glDeleteShader(glShader->m_shader);
        throw GfxShaderException(infoLog);
    }

    return shader;
}

void GfxGLRenderer::destroyShader(const GfxShader& shader) const {
    std::shared_ptr<GfxGLShader> glShader = std::static_pointer_cast<GfxGLShader>(shader);
    if (glShader->m_shader != 0) {
        glDeleteShader(glShader->m_shader);
        glShader->m_shader = 0;
    }
}

GfxPipeline GfxGLRenderer::createPipeline(
    const std::vector<GfxShader>& shaders,
    const std::vector<GfxDescriptorSet>& descriptorSets,
    const GfxVertexDesc& vertexDesc,
    const std::vector<GfxPipelineState>& dynamicStates,
    const GfxRenderPass& renderPass
) const {
    GfxPipeline pipeline = std::make_shared<GfxGLPipeline>(
            renderPass,
            descriptorSets,
            dynamicStates
        );
    std::shared_ptr<GfxGLPipeline> glPipeline =
        std::static_pointer_cast<GfxGLPipeline>(pipeline);

    glPipeline->m_program = glCreateProgram();
    for (const auto& shader : shaders) {
        std::shared_ptr<GfxGLShader> glShader =
            std::static_pointer_cast<GfxGLShader>(shader);
        glAttachShader(glPipeline->m_program, glShader->m_shader);
    }
    glLinkProgram(glPipeline->m_program);

    return pipeline;
}

void GfxGLRenderer::destroyPipeline(const GfxPipeline& pipeline) const {
    std::shared_ptr<GfxGLPipeline> glPipeline =
        std::static_pointer_cast<GfxGLPipeline>(pipeline);
    if (glPipeline->m_program != 0) {
        glDeleteProgram(glPipeline->m_program);
        glPipeline->m_program = 0;
    }
}

GfxDescriptorSetBinding GfxGLRenderer::createDescriptorSetBinding(
    const GfxPipeline& pipeline,
    int descriptorSetIndex,
    const std::vector<GfxDescriptorBinding>& bindings
) const {
    GfxDescriptorSetBinding descriptorSetBinding =
        std::make_shared<GfxGLDescriptorSetBinding>(pipeline, descriptorSetIndex, bindings);
    std::shared_ptr<GfxGLDescriptorSetBinding> glDescriptorSetBinding =
        std::static_pointer_cast<GfxGLDescriptorSetBinding>(descriptorSetBinding);

    for (const auto& binding : bindings) {
        if (binding.descriptor.type == GfxDescriptorType::SAMPLERS) {
            const std::vector<GfxImage>* images =
                std::get_if<std::vector<GfxImage>>(&binding.resource);
            if (images == nullptr || images->empty())
                continue; // Invalid binding, skip it
            GLuint ssbo = 0;
            glGenBuffers(1, &ssbo);
            glDescriptorSetBinding->m_bindingSamplesSsboMap[binding.descriptor.binding] = ssbo;
            std::vector<GLuint64> handles;
            handles.reserve(images->size());
            for (const auto& image : *images) {
                std::shared_ptr<GfxGLImage> glImage =
                    std::static_pointer_cast<GfxGLImage>(image);
                GLuint64 handle = glGetTextureHandleARB(glImage->m_texture);
                glMakeTextureHandleResidentARB(handle);
                handles.push_back(handle);
            }
            glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo);
            glBufferData(
                GL_SHADER_STORAGE_BUFFER,
                static_cast<GLsizeiptr>(handles.size() * sizeof(GLuint64)),
                handles.data(),
                GL_STATIC_DRAW
            );
        }
    }
    return descriptorSetBinding;
}

void GfxGLRenderer::destroyDescriptorSetBinding(GfxDescriptorSetBinding& binding) const {
    std::shared_ptr<GfxGLDescriptorSetBinding> glDescriptorSetBinding =
        std::static_pointer_cast<GfxGLDescriptorSetBinding>(binding);

    for (const auto& descriptorBinding : binding->getDescriptorBindings()) {
        if (descriptorBinding.descriptor.type == GfxDescriptorType::SAMPLERS) {
            const std::vector<GfxImage>* images =
                std::get_if<std::vector<GfxImage>>(&descriptorBinding.resource);
            if (images == nullptr || images->empty())
                continue; // Invalid binding, skip it
            for (const auto& image : *images) {
                std::shared_ptr<GfxGLImage> glImage =
                    std::static_pointer_cast<GfxGLImage>(image);
                GLuint64 handle = glGetTextureHandleARB(glImage->m_texture);
                glMakeTextureHandleNonResidentARB(handle);
            }
            int bindingPoint = descriptorBinding.descriptor.binding;
            GLuint ssbo = glDescriptorSetBinding->m_bindingSamplesSsboMap[bindingPoint];
            if (ssbo != 0)
                glDeleteBuffers(1, &ssbo);
        }
    }
    glDescriptorSetBinding->m_bindingSamplesSsboMap.clear();
}

int GfxGLRenderer::beginRenderPass(const GfxFramebuffer& framebuffer) {
    if (!framebuffer) {
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        m_currentFramebuffer = nullptr;
        return 0;
    }

    std::shared_ptr<GfxGLRenderPass> glRenderPass =
        std::static_pointer_cast<GfxGLRenderPass>(framebuffer->getRenderPass());

    const bool isMsaa = glRenderPass->m_samples > 1;

    GLuint drawFbo = isMsaa ? glRenderPass->m_msaaFbo : glRenderPass->m_fbo;

    glBindFramebuffer(GL_FRAMEBUFFER, drawFbo);

    GLenum colorTarget = isMsaa ?
        GL_TEXTURE_2D_MULTISAMPLE :
        GL_TEXTURE_2D;

    for (int i = 0; i < framebuffer->getColorImages().size(); i++) {
        std::shared_ptr<GfxGLImage> glImage =
            std::static_pointer_cast<GfxGLImage>(framebuffer->getColorImages()[i]);

        glFramebufferTexture2D(
            GL_FRAMEBUFFER,
            GL_COLOR_ATTACHMENT0 + i,
            colorTarget,
            glImage->m_texture,
            glRenderPass->getColorAttachments()[i].level
        );
    }

    if (framebuffer->getDepthImage()) {
        std::shared_ptr<GfxGLImage> glDepth =
            std::static_pointer_cast<GfxGLImage>(framebuffer->getDepthImage());

        glFramebufferTexture2D(
            GL_FRAMEBUFFER,
            GL_DEPTH_ATTACHMENT,
            colorTarget,
            glDepth->m_texture,
            glRenderPass->getDepthAttachment().level
        );
    }

    m_currentFramebuffer = framebuffer;
    return 0;
}

void GfxGLRenderer::endRenderPass() {
    if (m_currentFramebuffer) {
        std::shared_ptr<GfxGLRenderPass> glRenderPass =
            std::static_pointer_cast<GfxGLRenderPass>(m_currentFramebuffer->getRenderPass());
        if (glRenderPass->m_samples > 1) {
            glBindFramebuffer(GL_READ_FRAMEBUFFER, glRenderPass->m_msaaFbo);
            glBindFramebuffer(GL_DRAW_FRAMEBUFFER, glRenderPass->m_fbo);
            for (int i = 0; i < m_currentFramebuffer->getColorResolveImages().size(); i++) {
                GfxImage image = m_currentFramebuffer->getColorResolveImages()[i];
                std::shared_ptr<GfxGLImage> glImage =
                    std::static_pointer_cast<GfxGLImage>(image);

                glFramebufferTexture2D(
                    GL_DRAW_FRAMEBUFFER,
                    GL_COLOR_ATTACHMENT0 + i,
                    GL_TEXTURE_2D,
                    glImage->m_texture,
                    glRenderPass->getColorAttachments()[i].level
                );
            }
            for (int i = 0; i < m_currentFramebuffer->getColorImages().size(); i++) {
                glReadBuffer(GL_COLOR_ATTACHMENT0 + i);
                glDrawBuffer(GL_COLOR_ATTACHMENT0 + i);
                GfxImage image = m_currentFramebuffer->getColorResolveImages()[i];
                glBlitFramebuffer(
                    0,
                    0,
                    image->getWidth(),
                    image->getHeight(),
                    0,
                    0,
                    image->getWidth(),
                    image->getHeight(),
                    GL_COLOR_BUFFER_BIT,
                    GL_LINEAR
                );
            }
            glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
            glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
        }
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    m_currentFramebuffer = nullptr;
}

void GfxGLRenderer::bindPipeline(const GfxPipeline& pipeline) {
    std::shared_ptr<GfxGLPipeline> glPipeline =
        std::static_pointer_cast<GfxGLPipeline>(pipeline);
    glUseProgram(glPipeline->m_program);
}

void GfxGLRenderer::bindVAO(const GfxVAO& vao) {
    if (vao == nullptr) {
        glBindVertexArray(0);
        return;
    }
    std::shared_ptr<GfxGLVAO> glVAO = std::static_pointer_cast<GfxGLVAO>(vao);
    glBindVertexArray(glVAO->m_vao);
}

void GfxGLRenderer::clearColorAttachment(int index, const std::array<float, 4>& value) {
    glClearBufferfv(GL_COLOR, index, value.data());
}

void GfxGLRenderer::clearDepthAttachment(float value) {
    glClearBufferfv(GL_DEPTH, 0, &value);
}

void GfxGLRenderer::clearStencilAttachment(int value) {
    unsigned int stencil = static_cast<unsigned int>(value);
    glClearBufferuiv(GL_STENCIL, 0, &stencil);
}

void GfxGLRenderer::bindDescriptorSetBinding(const GfxDescriptorSetBinding& binding) {
    for (const auto& binding : binding->getDescriptorBindings()) {
        if (binding.descriptor.type == GfxDescriptorType::SAMPLER) {
            const GfxImage* image = std::get_if<GfxImage>(&binding.resource);
            if (image == nullptr)
                continue; // Invalid binding, skip it
            glActiveTexture(GL_TEXTURE0 + binding.descriptor.binding);
            std::shared_ptr<GfxGLImage> glImage =
                std::static_pointer_cast<GfxGLImage>(*image);
            glBindTexture(GL_TEXTURE_2D, glImage->m_texture);
        } else if (binding.descriptor.type == GfxDescriptorType::STORAGE_IMAGE) {
            const GfxImage* image = std::get_if<GfxImage>(&binding.resource);
            if (image == nullptr)
                continue; // Invalid binding, skip it
            std::shared_ptr<GfxGLImage> glImage =
                std::static_pointer_cast<GfxGLImage>(*image);
            glBindImageTexture(
                binding.descriptor.binding,
                glImage->m_texture,
                0, // level
                GL_FALSE, // layered
                0, // layer
                GL_READ_WRITE,
                GfxGLTypeConverter::toGLInternalFormat(glImage->getFormat())
            );
        } else {
            const GfxBuffer* buffer = std::get_if<GfxBuffer>(&binding.resource);
            if (buffer == nullptr)
                continue; // Invalid binding, skip it
            std::shared_ptr<GfxGLBuffer> glBuffer =
                std::static_pointer_cast<GfxGLBuffer>(*buffer);
            // Bindless texture descriptor is also a storage buffer holding texture handles
            GLenum bufferType = GL_SHADER_STORAGE_BUFFER;
            if (binding.descriptor.type == GfxDescriptorType::UNIFORM_BUFFER)
                bufferType = GL_UNIFORM_BUFFER;
            glBindBufferBase(bufferType, binding.descriptor.binding, glBuffer->m_buffer);
        }
    }
}

void GfxGLRenderer::draw(int nVertices, int nInstances, int firstVertex, int firstInstance) {
    GfxPrimitiveTopo topo =
        GfxPipelineStateController::getStateCache(m_pipelineStateMachine).primitiveTopo;
    GLenum mode = GfxGLTypeConverter::toGLDrawMode(topo);
    glDrawArraysInstancedBaseInstance(
        mode,
        firstVertex,
        nVertices,
        nInstances,
        firstInstance
    );
}

void GfxGLRenderer::drawIndexed(
    int nIndices,
    int nInstances,
    int firstIndex,
    int vertexOffset,
    int firstInstance
) {
    GfxPrimitiveTopo topo =
        GfxPipelineStateController::getStateCache(m_pipelineStateMachine).primitiveTopo;
    GLenum mode = GfxGLTypeConverter::toGLDrawMode(topo);
    glDrawElementsInstancedBaseVertexBaseInstance(
        mode,
        nIndices,
        GL_UNSIGNED_INT,
        reinterpret_cast<void*>(firstIndex * sizeof(unsigned int)),
        nInstances,
        vertexOffset,
        firstInstance
    );
}

void GfxGLRenderer::drawIndirect(const GfxBuffer& buffer, int offset, int drawCount, int stride) {
    std::shared_ptr<GfxGLBuffer> glBuffer = std::static_pointer_cast<GfxGLBuffer>(buffer);
    glBindBuffer(GL_DRAW_INDIRECT_BUFFER, glBuffer->m_buffer);
    GfxPrimitiveTopo topo =
        GfxPipelineStateController::getStateCache(m_pipelineStateMachine).primitiveTopo;
    GLenum mode = GfxGLTypeConverter::toGLDrawMode(topo);
    glMultiDrawArraysIndirect(
        mode,
        reinterpret_cast<void*>(static_cast<uintptr_t>(offset)),
        drawCount,
        stride
    );
    glBindBuffer(GL_DRAW_INDIRECT_BUFFER, 0);
}

void GfxGLRenderer::drawIndexedIndirect(
    const GfxBuffer& buffer,
    int offset,
    int drawCount,
    int stride
) {
    std::shared_ptr<GfxGLBuffer> glBuffer = std::static_pointer_cast<GfxGLBuffer>(buffer);
    glBindBuffer(GL_DRAW_INDIRECT_BUFFER, glBuffer->m_buffer);
    GfxPrimitiveTopo topo =
        GfxPipelineStateController::getStateCache(m_pipelineStateMachine).primitiveTopo;
    GLenum mode = GfxGLTypeConverter::toGLDrawMode(topo);
    glMultiDrawElementsIndirect(
        mode,
        GL_UNSIGNED_INT,
        reinterpret_cast<void*>(static_cast<uintptr_t>(offset)),
        drawCount,
        stride
    );
    glBindBuffer(GL_DRAW_INDIRECT_BUFFER, 0);
}

void GfxGLRenderer::dispatchCompute(int nGroupsX, int nGroupsY, int nGroupsZ) {
    glDispatchCompute(nGroupsX, nGroupsY, nGroupsZ);
}

void GfxGLRenderer::dispatchComputeIndirect(const GfxBuffer& buffer, int offset) {
    std::shared_ptr<GfxGLBuffer> glBuffer = std::static_pointer_cast<GfxGLBuffer>(buffer);
    glBindBuffer(GL_DISPATCH_INDIRECT_BUFFER, glBuffer->m_buffer);
    glDispatchComputeIndirect(offset);
    glBindBuffer(GL_DISPATCH_INDIRECT_BUFFER, 0);
}

void GfxGLRenderer::memoryBarrier() {
    glMemoryBarrier(GL_ALL_BARRIER_BITS);
}

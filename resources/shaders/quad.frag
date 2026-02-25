/**
 * @file quad.frag
 * @brief Fragment shader for rendering a textured quad.
 */

#version 450

layout(location = 0) in vec2 v_texCoord; // Texture coordinate

layout(binding = 0) buffer Radiances {
    float radiances[];
} b_radiances;

/**
 * @brief Uniform buffer for shader parameters.
 */
layout(binding = 1) uniform Params {
    int channel; // Channel selection
    int resX; // Horizontal resolution
    int resY; // Vertical resolution
} u_params; // Shader parameters

layout(location = 0) out vec4 o_fragColor; // Final fragment color

void main() {
    int pixelX = int(v_texCoord.x * u_params.resX);
    int pixelY = int(v_texCoord.y * u_params.resY);
    pixelX = clamp(pixelX, 0, u_params.resX - 1);
    pixelY = clamp(pixelY, 0, u_params.resY - 1);

    int waveBlockSize = u_params.resX * u_params.resY;
    int bufferIndex = u_params.channel * waveBlockSize + pixelY * u_params.resX + pixelX;
    float radiance = b_radiances.radiances[bufferIndex];

    o_fragColor = vec4(radiance, radiance, radiance, 1.0);
}

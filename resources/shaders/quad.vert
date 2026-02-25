/**
 * @file quad.vert
 * @brief Vertex shader for rendering a textured quad.
 */

#version 450

#ifdef VULKAN
#define gl_VertexID gl_VertexIndex
#endif

layout(location = 0) out vec2 v_texCoord; // Output texture coordinates

// Predefined vertex positions for a full-screen quad
const vec4 g_vertices[4] = vec4[](
    vec4(-1.0, -1.0, 0.0, 1.0),
    vec4( 1.0, -1.0, 0.0, 1.0),
    vec4(-1.0,  1.0, 0.0, 1.0),
    vec4( 1.0,  1.0, 0.0, 1.0)
);

void main() {
    gl_Position = g_vertices[gl_VertexID];
    v_texCoord = (gl_Position.xy + vec2(1.0)) * 0.5;
}

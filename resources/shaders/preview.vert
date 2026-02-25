/**
 * @file preview.vert
 * @brief Vertex shader for the previewer.
 */

#version 450

layout(location = 0) in vec3 i_pos; // Vertex position
layout(location = 1) in vec3 i_normal; // Vertex normal
layout(location = 2) in vec3 i_tangent; // Vertex tangent
layout(location = 3) in vec2 i_texCoord; // Vertex texture coordinate

/**
 * @brief Uniform struct for model, view, and projection matrices.
 */
 #ifdef VULKAN
layout(set = 1, binding = 0)
#else
layout(binding = 0)
#endif
uniform Xform {
    mat4 proj; // Projection matrix
    mat4 view; // View matrix
    mat4 model; // Model matrix
} u_xform; // Transformation matrices

layout(location = 0) out vec3 v_posW; // World space position
layout(location = 1) out vec3 v_normalW; // World space normal
layout(location = 2) out vec3 v_tangentW; // World space tangent
layout(location = 3) out vec2 v_texCoord; // Texture coordinate

void main() {
    v_posW = vec3(u_xform.model * vec4(i_pos, 1.0));
    v_normalW = normalize(mat3(u_xform.model) * i_normal);
    v_tangentW = normalize(mat3(u_xform.model) * i_tangent);
    v_texCoord = i_texCoord;

    gl_Position = u_xform.proj * u_xform.view * vec4(v_posW, 1.0);
}

/**
 * @file preview.frag
 * @brief Fragment shader for the previewer.
 */

#version 450

layout(location = 0) in vec3 v_posW; // World space position
layout(location = 1) in vec3 v_normalW; // World space normal
layout(location = 2) in vec3 v_tangentW; // World space tangent
layout(location = 3) in vec2 v_texCoord; // Texture coordinate

/**
 * @brief Uniform struct for camera position.
 */
#ifdef VULKAN
layout(set = 2, binding = 1)
#else
layout(binding = 1)
#endif
uniform Camera {
    vec3 posW; // Camera world position
} u_camera; // Camera data

/**
 * @brief Uniform struct for material properties.
 */
layout(binding = 2) uniform Material {
    vec4 diffuse; // Diffuse color (or highlight color)
    float roughness; // Roughness
    uint flags; // Material flags
} u_material; // Material data
const uint MATERIAL_NORMAL_MAP = 1 << 0; // Has normal texture
const uint MATERIAL_ROUGHNESS_MAP = 1 << 1; // Has roughness texture
const uint MATERIAL_INTENSITY_MAP = 1 << 2; // Has intensity texture (not used)
const uint MATERIAL_HIGHLIGHT = 1 << 3; // Highlighted material

layout(binding = 3) uniform sampler2D u_normalTex; // Normal texture
layout(binding = 4) uniform sampler2D u_roughnessTex; // Roughness texture
layout(binding = 5) uniform sampler2D u_intensityTex; // Intensity texture

/**
 * @brief Uniform struct for object picking information.
 */
layout(binding = 6) uniform PickInfo {
    uint modelID; // Model ID
    uint meshID; // Mesh ID
} u_pickInfo; // Picking information

layout(location = 0) out vec4 o_fragColor; // Final fragment color
layout(location = 1) out vec4 o_pickColor; // Picking color for color pick buffer

void main() {
    // Light direction in world space (camera as light source)
    vec3 l = normalize(u_camera.posW - v_posW);
    // Normal in world space
    vec3 n = normalize(v_normalW);
    if (dot(n, l) < 0.0)
        n = -n;
    if ((u_material.flags & MATERIAL_NORMAL_MAP) != 0) {
        vec3 bitangentW = normalize(cross(v_normalW, v_tangentW));
        mat3 tbn = mat3(v_tangentW, bitangentW, v_normalW);
        vec3 nt = normalize(texture(u_normalTex, v_texCoord).xyz * 2.0 - 1.0);
        n = tbn * nt;
    }

    float roughness = u_material.roughness;
    if ((u_material.flags & MATERIAL_ROUGHNESS_MAP) != 0)
        roughness = texture(u_roughnessTex, v_texCoord).r;

    vec3 diffuse = u_material.diffuse.rgb;
    if ((u_material.flags & MATERIAL_INTENSITY_MAP) != 0)
        diffuse = texture(u_intensityTex, v_texCoord).rgb;
    diffuse *= max(dot(n, l), 0.0);

    vec3 specular = vec3(1.0);
    float specularFact = pow(max(dot(n, l), 0.0), 128.0 * (1.0 - roughness));
    specularFact *= max(dot(n, l), 0.0);
    specular *= specularFact;

    vec3 shade = (diffuse + specular) * 0.5;
    if ((u_material.flags & MATERIAL_HIGHLIGHT) != 0)
        shade = diffuse;

    o_fragColor = vec4(shade, 1.0);

    o_pickColor = vec4(u_pickInfo.modelID, u_pickInfo.meshID, 1.0, 1.0);
}

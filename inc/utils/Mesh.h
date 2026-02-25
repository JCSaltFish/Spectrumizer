/**
 * @file Mesh.h
 * @brief Header file defining structures for representing 3D mesh data.
 */

#pragma once

#include "Math.h"

namespace Mesh {

/**
 * @brief Represents a vertex in a 3D mesh.
 */
struct Vertex {
    Math::Vec3 pos; // Position of the vertex in 3D space
    Math::Vec3 normal; // Normal vector
    Math::Vec3 tangent; // Tangent vector
    Math::Vec2 texCoord; // Texture coordinates
};

/**
 * @brief Represents a submesh within a mesh, containing indices of vertices.
 */
struct SubMesh {
    std::string name; // Name of the submesh
    std::vector<uint32_t> indices; // Indices of vertices in this submesh
};

/**
 * @brief Represents a mesh containing vertices and submeshes.
 */
struct Mesh {
    std::string name; // Name of the mesh
    std::vector<Vertex> vertices; // List of vertices in the mesh
    std::vector<SubMesh> submeshes; // List of submeshes in the mesh
};

/**
 * @brief Represents a 3D model containing multiple meshes.
 */
struct Model {
    std::string name; // Name of the model
    std::vector<Mesh> meshes; // List of meshes in the model
};

} // namespace Mesh

namespace MeshLoader {

/**
 * @brief Loads a 3D model from an OBJ file.
 * @param filename The name of the OBJ file to load.
 * @param model The Mesh::Model object to populate with the loaded data.
 * @return An integer indicating success (0) or failure (non-zero).
 */
int loadOBJ(const std::string& filename, Mesh::Model& model);
/**
 * @brief Retrieves information about a 3D model from an OBJ file without fully loading it.
 * @param filename The name of the OBJ file to inspect.
 * @param model The Mesh::Model object to populate with the retrieved information.
 * @return An integer indicating success (0) or failure (non-zero).
 */
int getInfoFromOBJ(const std::string& filename, Mesh::Model& model);

} // namespace MeshLoader

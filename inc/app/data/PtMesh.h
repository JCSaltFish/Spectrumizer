/**
 * @file PtMesh.h
 * @brief Declaration of the PtMesh class representing a mesh within a 3D model.
 */

#pragma once

#include "db/DbPub.h"

 /**
  * @brief Represents a mesh within a 3D model.
  */
class PtMesh {
    friend class DbTypeRegistry;
    /* OBJECT TYPE INFO */
private:
    static void serialize(DbSerializer& serializer, const PtMesh& mesh);
    static void deserialize(DbSerializer& serializer, PtMesh& mesh);
    static void migrate(int oldVersion, PtMesh& mesh);
public:
    static constexpr const char* TYPE_NAME = "PtMesh";
    static constexpr int VERSION = 1;

    /* OBJECT FIELDS */
private:
    DB::ID m_modelId = 0; // ID of the parent model
    std::string m_name; // Name of the mesh
    DB::ID m_materialId = 0; // ID of the associated material

    /* OBJECT METHODS */
private:
    /**
     * @brief Get a const pointer to the PtMesh object from a handle.
     * @param hMesh Handle to the mesh object.
     * @return Pointer to the PtMesh object, or nullptr if invalid.
     */
    static const PtMesh* view(const DbObjHandle& hMesh);
public:
    /**
     * @brief Get the parent model of the mesh.
     * @param hMesh Handle to the mesh object.
     * @return Handle to the parent model, or invalid handle if none.
     */
    static DbObjHandle getModel(const DbObjHandle& hMesh);
    /**
     * @brief Set the parent model of the mesh.
     * @param hMesh Handle to the mesh object.
     * @param hModel Handle to the new parent model object.
     * @return Result code indicating success or failure.
     */
    static DB::Result setModel(const DbObjHandle& hMesh, const DbObjHandle& hModel);
    /**
     * @brief Get the name of the mesh.
     * @param hMesh Handle to the mesh object.
     * @return Name of the mesh, or empty string if invalid.
     */
    static std::string getName(const DbObjHandle& hMesh);
    /**
     * @brief Set the name of the mesh.
     * @param hMesh Handle to the mesh object.
     * @param name New name for the mesh.
     * @return Result code indicating success or failure.
     */
    static DB::Result setName(const DbObjHandle& hMesh, const std::string& name);
    /**
     * @brief Get the associated material of the mesh.
     * @param hMesh Handle to the mesh object.
     * @return Handle to the associated material, or invalid handle if none.
     */
    static DbObjHandle getMaterial(const DbObjHandle& hMesh);
    /**
     * @brief Set the associated material of the mesh.
     *
     * This will delete the previous material if it exists. Only use this for assigning
     * material for a new mesh. To change properties of the existing material, retrieve it
     * with getMaterial().
     *
     * @param hMesh Handle to the mesh object.
     * @param hMaterial Handle to the new material object.
     * @return Result code indicating success or failure.
     */
    static DB::Result setMaterial(const DbObjHandle& hMesh, const DbObjHandle& hMaterial);
    /**
     * @brief Copy the mesh object to another database.
     * @param hMesh Handle to the mesh object to copy.
     * @param dst Shared pointer to the destination database.
     * @return Handle to the copied mesh in the destination database.
     */
    static DbObjHandle copy(const DbObjHandle& hMesh, std::shared_ptr<DB>& dst);
};

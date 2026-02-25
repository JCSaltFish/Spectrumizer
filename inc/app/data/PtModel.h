/**
 * @file PtModel.h
 * @brief Declaration of the PtModel class representing a 3D model.
 */

#pragma once

#include "db/DbPub.h"
#include "utils/Math.h"

/**
 * @brief Represents a 3D model with associated meshes and transformation properties.
 */
class PtModel {
    friend class DbTypeRegistry;
    /* OBJECT TYPE INFO */
private:
    static void serialize(DbSerializer& serializer, const PtModel& model);
    static void deserialize(DbSerializer& serializer, PtModel& model);
    static void migrate(int oldVersion, PtModel& model);
public:
    static constexpr const char* TYPE_NAME = "PtModel";
    static constexpr int VERSION = 1;

    /* OBJECT FIELDS */
private:
    std::string m_name; // Name of the model
    DbFilePath m_filePath; // File path of the model
    std::vector<DB::ID> m_meshes; // List of mesh IDs associated with the model
    Math::Vec3 m_location; // Position of the model in 3D space
    Math::Vec3 m_rotation; // Rotation of the model (Euler angles)
    Math::Vec3 m_scale = Math::Vec3(1.0f); // Scale of the model in 3D space

    /* OBJECT METHODS */
private:
    /**
     * @brief Get a const pointer to the PtModel object from a handle.
     * @param hModel Handle to the model object.
     * @return Pointer to the PtModel object, or nullptr if invalid.
     */
    static const PtModel* view(const DbObjHandle& hModel);
public:
    /**
     * @brief Get the name of the model.
     * @param hModel Handle to the model object.
     * @return Name of the model, or empty string if invalid.
     */
    static std::string getName(const DbObjHandle& hModel);
    /**
     * @brief Set the name of the model.
     * @param hModel Handle to the model object.
     * @param name New name for the model.
     * @return Result code indicating success or failure.
     */
    static DB::Result setName(const DbObjHandle& hModel, const std::string& name);
    /**
     * @brief Get the file path of the model.
     * @param hModel Handle to the model object.
     * @return File path of the model, or empty string if invalid.
     */
    static std::string getFilePath(const DbObjHandle& hModel);
    /**
     * @brief Set the file path of the model.
     * @param hModel Handle to the model object.
     * @param path New file path for the model.
     * @return Result code indicating success or failure.
     */
    static DB::Result setFilePath(const DbObjHandle& hModel, const std::string& path);
    /**
     * @brief Get the meshes associated with the model.
     * @param hModel Handle to the model object.
     * @return Vector of handles to the associated meshes.
     */
    static std::vector<DbObjHandle> getMeshes(const DbObjHandle& hModel);
    /**
     * @brief Set the meshes associated with the model.
     *
     * This will delete any existing mesh associations (along with their materials)
     * and replace them with the provided list. Only use this for assigning meshes for a new model.
     *
     * @param hModel Handle to the model object.
     * @param hMeshes Vector of handles to the meshes to associate with the model.
     * @return Result code indicating success or failure.
     */
    static DB::Result setMeshes(const DbObjHandle& hModel, const std::vector<DbObjHandle>& hMeshes);
    /**
     * @brief Add a mesh to the model.
     * @param hModel Handle to the model object.
     * @param hMesh Handle to the mesh object to add.
     * @return Result code indicating success or failure.
     */
    static Math::Vec3 getLocation(const DbObjHandle& hModel);
    /**
     * @brief Set the location of the model.
     * @param hModel Handle to the model object.
     * @param location New location as a Vec3.
     * @return Result code indicating success or failure.
     */
    static DB::Result setLocation(const DbObjHandle& hModel, const Math::Vec3& location);
    /**
     * @brief Get the rotation of the model.
     * @param hModel Handle to the model object.
     * @return Rotation as a Vec3 (Euler angles), or zero vector if invalid.
     */
    static Math::Vec3 getRotation(const DbObjHandle& hModel);
    /**
     * @brief Set the rotation of the model.
     * @param hModel Handle to the model object.
     * @param rotation New rotation as a Vec3 (Euler angles).
     * @return Result code indicating success or failure.
     */
    static DB::Result setRotation(const DbObjHandle& hModel, const Math::Vec3& rotation);
    /**
     * @brief Get the scale of the model.
     * @param hModel Handle to the model object.
     * @return Scale as a Vec3, or (1,1,1) if invalid.
     */
    static Math::Vec3 getScale(const DbObjHandle& hModel);
    /**
     * @brief Set the scale of the model.
     * @param hModel Handle to the model object.
     * @param scale New scale as a Vec3.
     * @return Result code indicating success or failure.
     */
    static DB::Result setScale(const DbObjHandle& hModel, const Math::Vec3& scale);
    /**
     * @brief Copy the model to another database.
     *
     * This will also copy all associated meshes and their materials.
     *
     * @param hModel Handle to the model object to copy.
     * @param dst Shared pointer to the destination database.
     * @return Handle to the copied model in the destination database.
     */
    static DbObjHandle copy(const DbObjHandle& hModel, std::shared_ptr<DB>& dst);
};

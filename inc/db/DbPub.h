/*
 * Copyright 2025 Jed Wang
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

 /**
  * @file DbPub.h
  * @brief Public header file for the DB module.
  */

#pragma once

#include "DbSerializer.h"

  /**
   * @brief Registry for database object types.
   */
class DbTypeRegistry
{
    friend class DB;

private:
    DbTypeRegistry() = default;

public:
    /**
     * @brief Information about a registered type.
     */
    struct TypeInfo
    {
        // Version of the type for migration
        uint32_t version = 0;
        // Unique name of the type
        std::string typeName = {};
        // Serialization function
        std::function<void(DbSerializer&, const std::any&)> serialize = nullptr;
        // Deserialization function
        std::function<void(DbSerializer&, std::any&)> deserialize = nullptr;
        // Migration function
        std::function<void(int, std::any&)> migrate = nullptr;
    };

    DbTypeRegistry(const DbTypeRegistry&) = delete;
    DbTypeRegistry& operator=(const DbTypeRegistry&) = delete;

    static DbTypeRegistry& instance()
    {
        static DbTypeRegistry registry;
        return registry;
    };

    /**
     * @brief Register a type with the registry.
     * @tparam T The type to register.
     */
    template<typename T>
    void registerType()
    {
        std::unique_lock lock(m_mutex);
        if (m_registry.find(T::TYPE_NAME) != m_registry.end())
            return; // Already registered
        TypeInfo info;
        info.version = T::VERSION;
        info.typeName = T::TYPE_NAME;
        info.serialize = [](DbSerializer& serializer, const std::any& obj)
            {
                T::serialize(serializer, std::any_cast<const T&>(obj));
            };
        info.deserialize = [](DbSerializer& serializer, std::any& obj)
            {
                if (!obj.has_value())
                    obj = T{};
                T::deserialize(serializer, std::any_cast<T&>(obj));
            };
        info.migrate = [](int oldVersion, std::any& obj)
            {
                if (obj.has_value())
                    T::migrate(oldVersion, std::any_cast<T&>(obj));
            };
        m_registry.emplace(T::TYPE_NAME, std::move(info));
        m_nameLookup[typeid(T)] = T::TYPE_NAME;
    };
    /**
     * @brief Check if a type is registered.
     * @tparam T The type to check.
     * @return True if the type is registered, false otherwise.
     */
    template<typename T>
    bool isRegistered() const
    {
        std::shared_lock lock(m_mutex);
        return m_nameLookup.find(typeid(T)) != m_nameLookup.end();
    };

    /**
     * @brief Get type information by type name.
     * @param typeName The name of the type.
     * @return Pointer to TypeInfo if found, nullptr otherwise.
     */
    const TypeInfo* getTypeInfo(const std::string& typeName) const;
    /**
     * @brief Get type information by std::type_index.
     * @param typeIndex The type index of the type.
     * @return Pointer to TypeInfo if found, nullptr otherwise.
     */
    const TypeInfo* getTypeInfo(const std::type_index& typeIndex) const;

private:
    std::unordered_map<std::string, TypeInfo> m_registry; // Map of type name to TypeInfo
    std::unordered_map<std::type_index, std::string> m_nameLookup; // Map of type_index to type name
    mutable std::shared_mutex m_mutex; // Mutex for thread-safe access
};

class DbObjHandle;
/**
 * @brief The database.
 */
class DB
{
    friend class DbObjHandle;

public:
    /**
     * @brief Result codes for database operations.
     */
    enum class Result
    {
        SUCCESS = 0,
        FAILURE = 1,
        INVALID_HANDLE,
        OBJECT_NOT_FOUND,
        UNKONWN_TYPE,
        FILE_OPEN_ERROR,
        FILE_FORMAT_ERROR,
        FILE_VERSION_ERROR,
    };

    using ID = uint32_t;

    DB() = default;
    DB(const std::vector<uint8_t>& magic, int version) :
        m_magic(magic),
        m_version(version)
    {
    };
    ~DB() = default;

private:
    /**
     * @brief Entry representing an object in the database.
     */
    struct ObjectEntry
    {
        ID id = 0; // Unique ID (generation + index)
        std::string typeName = {}; // Type name of the object
        bool alive = false; // Whether the object is alive
        std::any data = {}; // The actual object data
    };

    /**
     * @brief Operation types for transactions.
     */
    enum class OpType
    {
        CREATE,
        MODIFY,
        DELETE,
    };
    /**
     * @brief Operation record for transactions.
     */
    struct Op
    {
        OpType type = OpType::CREATE; // Type of operation
        ID objId = 0; // ID of the object
        std::string typeName = {}; // Type name of the object
        std::any oldData = {}; // Data before the operation
        std::any newData = {}; // Data after the operation
        bool oldAlive = false; // Whether the object was alive before
        bool newAlive = false; // Whether the object is alive after
    };
    using TxnRecord = std::vector<Op>;

public:
    /**
     * @brief Create a new object in the database.
     * @tparam T The type of the object to create.
     * @param obj The object to create.
     * @return A handle to the created object, or an invalid handle if creation failed.
     */
    template<typename T>
    DbObjHandle objCreate(const T& obj);
    /**
     * @brief Delete an object from the database.
     * @tparam T The type of the object to delete.
     * @param handle Handle to the object to delete.
     * @return DB::Result indicating success or failure.
     */
    template<typename T>
    Result objDelete(const DbObjHandle& handle);
    /**
     * @brief Modify an existing object in the database.
     * @tparam T The type of the object to modify.
     * @param handle Handle to the object to modify.
     * @param newData The new data for the object.
     * @return DB::Result indicating success or failure.
     */
    template<typename T>
    Result objModify(const DbObjHandle& handle, const T& newData);
    /**
     * @brief Retrieve an object from the database.
     * @tparam T The type of the object to retrieve.
     * @param handle Handle to the object to retrieve.
     * @return Pointer to the object if found, nullptr otherwise.
     */
    template<typename T>
    const T* objGet(const DbObjHandle& handle) const;

    /**
     * @brief Set the root object of the database.
     * @param handle Handle to the root object.
     */
    void setRootObject(const DbObjHandle& handle);
    /**
     * @brief Get the root object of the database.
     * @return Handle to the root object, or an invalid handle if not set.
     */
    DbObjHandle getRootObject() const;

    /**
     * @brief Load the database state from a file.
     * @param filename The path to the file to load from.
     * @return DB::Result indicating success or failure.
     */
    Result loadFromFile(const std::string& filename);
    /**
     * @brief Save the current database state to a file.
     * @param filename The path to the file to save to.
     * @return DB::Result indicating success or failure.
     */
    Result saveToFile(const std::string& filename);

    /**
     * @brief Begin a transaction.
     */
    void beginTxn();
    /**
     * @brief Commit the current transaction.
     */
    void commitTxn();
    /**
     * @brief Rollback the current transaction.
     */
    void rollbackTxn();
    /**
     * @brief Undo the last committed transaction.
     * @param dirtyObjects[out] Set to store handles of modified objects.
     * @return DB::Result indicating success or failure.
     */
    Result undo(std::unordered_set<DbObjHandle>& dirtyObjects);
    /**
     * @brief Redo the last undone transaction.
     * @param dirtyObjects[out] Set to store handles of modified objects.
     * @return DB::Result indicating success or failure.
     */
    Result redo(std::unordered_set<DbObjHandle>& dirtyObjects);
    /**
     * @brief Check if an undo operation is possible.
     * @return True if undo is possible, false otherwise.
     */
    bool canUndo() const;
    /**
     * @brief Check if a redo operation is possible.
     * @return True if redo is possible, false otherwise.
     */
    bool canRedo() const;
    /**
     * @brief Set the maximum size of the undo stack.
     * @param size The maximum number of transactions to keep in the undo stack.
     */
    void setMaxUndoStackSize(size_t size);
    /**
     * @brief Get the maximum size of the undo stack.
     * @return The maximum number of transactions in the undo stack.
     */
    size_t getMaxUndoStackSize() const;
    /**
     * @brief Check if the database has unsaved modifications.
     * @return True if there are unsaved modifications, false otherwise.
     */
    bool isModified() const;

private:
    /**
     * @brief Undo a single operation.
     * @param op The operation to undo.
     * @return DB::Result indicating success or failure.
     */
    Result undoOp(const Op& op);
    /**
     * @brief Redo a single operation.
     * @param op The operation to redo.
     * @return DB::Result indicating success or failure.
     */
    Result redoOp(const Op& op);
    /**
     * @brief Rebuild the list of free indices based on current objects.
     */
    void rebuildFreeIndices();

private:
    std::vector<uint8_t> m_magic{ 'D', 'B' }; // File magic number
    uint32_t m_version = 0; // File version

    mutable std::shared_mutex m_mutex; // Mutex for thread-safe access

    std::vector<ObjectEntry> m_objects{}; // List of all objects
    std::unordered_set<uint32_t> m_freeIndices{}; // List of free indices
    std::vector<uint32_t> m_gens{}; // Generation counters for each index
    ID m_rootObjId = -1; // ID of the root object

    bool m_inTxn = false; // Whether a transaction is in progress
    TxnRecord m_currentTxn{}; // Current transaction being recorded
    std::unordered_map<ID, ObjectEntry> m_txnWorkspace{}; // Workspace for current transaction
    std::deque<TxnRecord> m_undoStack{}; // Stack of undo transactions
    std::deque<TxnRecord> m_redoStack{}; // Stack of redo transactions

    uint32_t m_modifyCount = 0; // Count of transactions since last save
    size_t m_maxUndoStackSize = 100; // Maximum size of undo stack
};

namespace DbUtils
{

/**
 * @brief Guard for database transactions using RAII.
 */
class TxnGuard
{
public:
    explicit TxnGuard(std::shared_ptr<DB> db) : m_db(db)
    {
        m_db->beginTxn();
    };
    ~TxnGuard()
    {
        if (!m_committed && m_db)
            m_db->rollbackTxn();
    };
    TxnGuard(const TxnGuard&) = delete;
    TxnGuard& operator=(const TxnGuard&) = delete;

public:
    /**
     * @brief Commit the transaction.
     */
    void commit()
    {
        m_db->commitTxn();
        m_committed = true;
    };

private:
    std::shared_ptr<DB> m_db = nullptr; // Pointer to the database
    bool m_committed = false; // Whether the transaction has been committed
};

/**
 * @brief Execute a function within a database transaction.
 * @tparam Fn The type of the function to execute.
 * @tparam Args The types of the function arguments.
 * @param db Shared pointer to the database.
 * @param fn The function returns DB::Result to execute.
 * @param args Arguments to pass to the function.
 * @return DB::Result indicating success or failure.
 */
template<typename Fn, typename... Args>
DB::Result txnFn(std::shared_ptr<DB>& db, Fn&& fn, Args&&... args)
{
    TxnGuard txnGuard(db);
    DB::Result result = std::forward<Fn>(fn)(std::forward<Args>(args)...);
    if (result == DB::Result::SUCCESS)
        txnGuard.commit();
    return result;
}

} // namespace DbUtils

/**
 * @brief Handle to a database object.
 */
class DbObjHandle
{
public:
    DbObjHandle(DB* db = nullptr, DB::ID id = 0) : m_db(db), m_id(id) {};

    bool operator==(const DbObjHandle& other) const;

    /**
     * @brief Check if the handle is valid.
     * @return True if valid, false otherwise.
     */
    bool isValid() const;
    /**
     * @brief Get the ID of the object.
     * @return The object ID.
     */
    DB::ID getID() const;
    /**
     * @brief Get the type name of the object.
     * @return The type name, or empty string if invalid.
     */
    const std::string getType() const;
    /**
     * @brief Get the database instance.
     * @return Pointer to the database.
     */
    DB* getDB() const;

private:
    DB* m_db; // Pointer to the database
    DB::ID m_id; // Object ID
};

template<>
struct std::hash<DbObjHandle>
{
    std::size_t operator()(const DbObjHandle& handle) const noexcept
    {
        std::size_t h1 = std::hash<DB*>{}(handle.getDB());
        std::size_t h2 = std::hash<uint32_t>{}(handle.getID());
        return h1 ^ (h2 << 1);
    }
};

template<typename T>
DbObjHandle DB::objCreate(const T& obj)
{
    std::unique_lock lock(m_mutex);

    if (!DbTypeRegistry::instance().isRegistered<T>())
        return DbObjHandle();

    const DbTypeRegistry::TypeInfo* typeInfo =
        DbTypeRegistry::instance().getTypeInfo(typeid(T));
    if (!typeInfo)
        return DbObjHandle();

    ObjectEntry entry;
    uint32_t index = -1;
    if (!m_freeIndices.empty())
    {
        index = *m_freeIndices.begin();
        m_freeIndices.erase(m_freeIndices.begin());
        m_gens[index]++;
    }
    else
    {
        index = static_cast<uint32_t>(m_objects.size());
        m_objects.push_back(ObjectEntry{});
        m_gens.push_back(0);
    }
    entry.id = (m_gens[index] << 16) | index;
    entry.typeName = typeInfo->typeName;
    entry.alive = true;
    entry.data = obj;

    if (m_inTxn)
    {
        // Save "before" (non-existent) into workspace
        ObjectEntry origEntry{};
        origEntry.id = entry.id;
        origEntry.typeName = entry.typeName;
        origEntry.alive = false;
        m_txnWorkspace[entry.id] = origEntry;

        Op op;
        op.type = OpType::CREATE;
        op.objId = entry.id;
        op.typeName = entry.typeName;
        op.oldAlive = false;
        op.newAlive = true;
        // oldData empty, newData is the created data
        op.newData = entry.data;
        m_currentTxn.push_back(std::move(op));
    }

    ID id = entry.id;
    m_objects[index] = std::move(entry);
    return DbObjHandle(this, id);
}

template<typename T>
DB::Result DB::objDelete(const DbObjHandle& handle)
{
    std::unique_lock lock(m_mutex);

    uint32_t index = handle.getID() & 0xFFFF;
    if (index >= m_objects.size())
        return Result::INVALID_HANDLE;

    ObjectEntry& entry = m_objects[index];
    if (!entry.alive || entry.id != handle.getID())
        return Result::OBJECT_NOT_FOUND;

    const DbTypeRegistry::TypeInfo* typeInfo =
        DbTypeRegistry::instance().getTypeInfo(typeid(T));
    if (!typeInfo)
        return Result::UNKONWN_TYPE;

    if (m_inTxn)
    {
        m_txnWorkspace[entry.id] = entry;

        Op op;
        op.type = OpType::DELETE;
        op.objId = entry.id;
        op.typeName = entry.typeName;
        op.oldAlive = true;
        op.newAlive = false;
        op.oldData = entry.data;
        m_currentTxn.push_back(std::move(op));
    }

    entry.alive = false;
    //entry.typeName = {};
    entry.data.reset();
    m_freeIndices.insert(index);
    return Result::SUCCESS;
}

template<typename T>
DB::Result DB::objModify(const DbObjHandle& handle, const T& newData)
{
    std::unique_lock lock(m_mutex);

    uint32_t index = handle.getID() & 0xFFFF;
    uint32_t gen = handle.getID() >> 16;
    if (index >= m_objects.size() || gen != m_gens[index])
        return Result::INVALID_HANDLE;

    ObjectEntry& entry = m_objects[index];
    if (!entry.alive || entry.id != handle.getID())
        return Result::OBJECT_NOT_FOUND;

    const DbTypeRegistry::TypeInfo* typeInfo =
        DbTypeRegistry::instance().getTypeInfo(typeid(T));
    if (!typeInfo)
        return Result::UNKONWN_TYPE;

    const std::any oldAny = entry.data; // capture BEFORE
    if (m_inTxn)
    {
        if (m_txnWorkspace.find(entry.id) == m_txnWorkspace.end())
            m_txnWorkspace[entry.id] = entry;

        Op op;
        op.type = OpType::MODIFY;
        op.objId = entry.id;
        op.typeName = entry.typeName;
        op.oldAlive = true;
        op.newAlive = true;
        op.oldData = oldAny; // BEFORE
        // newData will be set after we assign entry.data
        // but we already have newData in 'newData' param (newData T)
        op.newData = newData;   // AFTER
        m_currentTxn.push_back(std::move(op));
    }

    entry.data = newData; // assign AFTER we've captured oldAny
    return Result::SUCCESS;
}

template<typename T>
const T* DB::objGet(const DbObjHandle& handle) const
{
    std::shared_lock lock(m_mutex);

    uint32_t index = handle.getID() & 0xFFFF;
    uint32_t gen = handle.getID() >> 16;
    if (index >= m_objects.size() || gen != m_gens[index])
        return nullptr;

    const ObjectEntry& entry = m_objects[index];
    if (!entry.alive || entry.id != handle.getID())
        return nullptr;
    const DbTypeRegistry::TypeInfo* typeInfo =
        DbTypeRegistry::instance().getTypeInfo(typeid(T));
    if (!typeInfo)
        return nullptr;

    try
    {
        return std::any_cast<T>(&entry.data);
    }
    catch (const std::bad_any_cast&) {}
    return nullptr;
}

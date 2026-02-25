/**
 * @file DB.cpp
 * @brief Implementation of the DB module.
 */

#include "db/DbPr.h"

const DbTypeRegistry::TypeInfo* DbTypeRegistry::getTypeInfo(
    const std::string& typeName
) const {
    std::shared_lock lock(m_mutex);
    auto it = m_registry.find(typeName);
    if (it != m_registry.end())
        return &it->second;
    return nullptr;
};

const DbTypeRegistry::TypeInfo* DbTypeRegistry::getTypeInfo(
    const std::type_index& typeIndex
) const {
    std::shared_lock lock(m_mutex);
    auto it = m_nameLookup.find(typeIndex);
    if (it == m_nameLookup.end())
        return nullptr;
    return getTypeInfo(it->second);
};

bool DbObjHandle::operator==(const DbObjHandle& other) const {
    return m_db == other.m_db && m_id == other.m_id;
}

bool DbObjHandle::isValid() const {
    if (!m_db || m_id < 0)
        return false;
    uint32_t index = m_id & 0xFFFF;
    uint32_t gen = m_id >> 16;
    if (index >= m_db->m_objects.size() || gen != m_db->m_gens[index])
        return false;
    const DB::ObjectEntry& entry = m_db->m_objects[index];
    if (!entry.alive || entry.id != m_id)
        return false;
    return true;
}

DB::ID DbObjHandle::getID() const {
    return m_id;
};

const std::string DbObjHandle::getType() const {
    if (!m_db || m_id < 0)
        return {};
    uint32_t index = m_id & 0xFFFF;
    uint32_t gen = m_id >> 16;
    if (index >= m_db->m_objects.size() || gen != m_db->m_gens[index])
        return {};
    const DB::ObjectEntry& entry = m_db->m_objects[index];
    if (entry.id != m_id)
        return {};
    return entry.typeName;
}

DB* DbObjHandle::getDB() const {
    return m_db;
}

void DB::setRootObject(const DbObjHandle& handle) {
    std::unique_lock lock(m_mutex);
    m_rootObjId = handle.getID();
}

DbObjHandle DB::getRootObject() const {
    std::shared_lock lock(m_mutex);
    if (m_rootObjId < 0)
        return DbObjHandle();
    uint32_t index = m_rootObjId & 0xFFFF;
    uint32_t gen = m_rootObjId >> 16;
    if (index >= m_objects.size() || gen != m_gens[index])
        return DbObjHandle();
    const ObjectEntry& entry = m_objects[index];
    if (!entry.alive || entry.id != m_rootObjId)
        return DbObjHandle();
    return DbObjHandle(const_cast<DB*>(this), m_rootObjId);
}

DB::Result DB::loadFromFile(const std::string& filename) {
    std::unique_lock lock(m_mutex);

    std::ifstream file(filename, std::ios::binary);
    if (!file.is_open())
        return Result::FILE_OPEN_ERROR;

    auto readInt = [](std::ifstream* file, uint32_t& value) {
        uint32_t netValue = 0;
        file->read(reinterpret_cast<char*>(&netValue), sizeof(netValue));
#ifdef _WIN32
        value = _byteswap_ulong(netValue);
#else
        value = __builtin_bswap32(netValue);
#endif
        };

    // Header
    std::vector<uint8_t> fileMagic(m_magic.size());
    file.read(reinterpret_cast<char*>(fileMagic.data()), fileMagic.size());
    if (fileMagic != m_magic)
        return Result::FILE_FORMAT_ERROR; // Invalid magic

    // Version
    uint32_t fileVersion = 0;
    readInt(&file, fileVersion);
    if (fileVersion > m_version)
        return Result::FILE_VERSION_ERROR; // Unsupported version

    // Root object ID
    readInt(&file, m_rootObjId);

    // Object count
    uint32_t objCount = 0;
    readInt(&file, objCount);
    m_objects.clear();
    m_objects.resize(objCount);
    m_freeIndices.clear();
    m_gens.clear();
    m_gens.resize(objCount, 0);

    // Objects
    for (uint32_t i = 0; i < objCount; ++i) {
        ObjectEntry entry;

        readInt(&file, entry.id);
        uint32_t typeNameLen = 0;
        readInt(&file, typeNameLen);
        std::string typeName(typeNameLen, '\0');
        file.read(typeName.data(), typeNameLen);
        entry.typeName = std::move(typeName);
        entry.alive = true;

        const DbTypeRegistry::TypeInfo* typeInfo =
            DbTypeRegistry::instance().getTypeInfo(entry.typeName);
        if (!typeInfo) {
            // Unknown type, skip
            uint32_t dataSize = 0;
            readInt(&file, dataSize);
            file.seekg(dataSize, std::ios::cur);
            uint32_t objectVersion = 0;
            readInt(&file, objectVersion);
            continue;
        }

        uint32_t dataSize = 0;
        readInt(&file, dataSize);
        if (dataSize > 0) {
            std::vector<char> dataBuf(dataSize);
            file.read(dataBuf.data(), dataSize);
            std::stringstream dataStream(
                std::string(dataBuf.data(), dataSize),
                std::ios::in | std::ios::binary
            );
            DbSerializer serializer(DbSerializer::SerializationMode::READ, dataStream, filename);
            typeInfo->deserialize(serializer, entry.data);
        }

        uint32_t objVersion = 0;
        readInt(&file, objVersion);
        if (objVersion < typeInfo->version && typeInfo->migrate)
            typeInfo->migrate(objVersion, entry.data);

        uint32_t index = entry.id & 0xFFFF;
        if (index >= m_objects.size()) {
            m_objects.resize(static_cast<size_t>(index) + 1);
            m_gens.resize(static_cast<size_t>(index) + 1, 0);
        }
        m_objects[index] = std::move(entry);
        m_gens[index] = entry.id >> 16;
    }

    // Rebuild free indices
    rebuildFreeIndices();

    // Clear transaction history
    m_undoStack.clear();
    m_redoStack.clear();
    // Reset modify count
    m_modifyCount = 0;

    file.close();
    return Result::SUCCESS;
}

DB::Result DB::saveToFile(const std::string& filename) {
    std::unique_lock lock(m_mutex);

    // Write to a temporary file first
    std::string tmpFilename = DbFileUtils::createTempFile(filename);
    std::ofstream file(tmpFilename, std::ios::binary | std::ios::trunc);
    if (!file.is_open())
        return Result::FILE_OPEN_ERROR;

    auto writeInt = [](std::ofstream* file, uint32_t value) {
        uint32_t netValue = 0;
#ifdef _WIN32
        netValue = _byteswap_ulong(value);
#else
        netValue = __builtin_bswap32(value);
#endif
        file->write(reinterpret_cast<const char*>(&netValue), sizeof(netValue));
        };

    // Header
    file.write(reinterpret_cast<const char*>(m_magic.data()), m_magic.size());
    writeInt(&file, m_version);

    // Root object ID
    writeInt(&file, m_rootObjId);

    // Object count
    uint32_t objCount = 0;
    for (const auto& entry : m_objects) {
        if (entry.alive)
            objCount++;
    }
    writeInt(&file, objCount);

    // Objects
    for (const auto& entry : m_objects) {
        if (!entry.alive)
            continue;

        writeInt(&file, entry.id);
        uint32_t typeNameLen = static_cast<uint32_t>(entry.typeName.size());
        writeInt(&file, typeNameLen);
        file.write(entry.typeName.data(), typeNameLen);

        const DbTypeRegistry::TypeInfo* typeInfo =
            DbTypeRegistry::instance().getTypeInfo(entry.typeName);
        if (!typeInfo) {
            // Unknown type, skip
            uint32_t dataSize = 0;
            writeInt(&file, dataSize);
            uint32_t objectVersion = 0;
            writeInt(&file, objectVersion);
            continue;
        }

        std::stringstream dataStream(std::ios::binary | std::ios::out);
        DbSerializer serializer(DbSerializer::SerializationMode::WRITE, dataStream, filename);
        typeInfo->serialize(serializer, entry.data);
        std::string dataStr = dataStream.str();
        uint32_t dataSize = static_cast<uint32_t>(dataStr.size());
        writeInt(&file, dataSize);
        if (dataSize > 0)
            file.write(dataStr.data(), dataSize);

        uint32_t objectVersion = typeInfo->version;
        writeInt(&file, objectVersion);
    }

    file.close();
    // Replace original file with temp file
    if (DbFileUtils::replaceFile(filename, tmpFilename))
        return Result::FAILURE;

    // Clear transaction history
    m_undoStack.clear();
    m_redoStack.clear();
    // Reset modify count
    m_modifyCount = 0;

    return Result::SUCCESS;
}

void DB::beginTxn() {
    std::unique_lock lock(m_mutex);
    if (m_inTxn)
        return; // Already in a transaction
    m_currentTxn = TxnRecord{};
    m_inTxn = true;
}

void DB::commitTxn() {
    std::unique_lock lock(m_mutex);
    if (!m_inTxn)
        return; // Not in a transaction

    if (m_currentTxn.empty()) {
        // No operations recorded, nothing to commit
        m_inTxn = false;
        return;
    }

    if (m_undoStack.size() >= m_maxUndoStackSize)
        m_undoStack.erase(m_undoStack.begin());

    m_undoStack.push_back(std::move(m_currentTxn));
    m_redoStack.clear();
    m_txnWorkspace.clear();
    m_inTxn = false;

    m_modifyCount++;
}

void DB::rollbackTxn() {
    std::unique_lock lock(m_mutex);
    if (!m_inTxn)
        return; // Not in a transaction
    // Revert changes using the workspace
    for (const auto& [objId, entry] : m_txnWorkspace) {
        uint32_t index = objId & 0xFFFF;
        if (index < m_objects.size()) {
            m_objects[index] = entry;
            m_gens[index] = objId >> 16;
        }
    }
    rebuildFreeIndices();
    m_txnWorkspace.clear();
    m_inTxn = false;
}

DB::Result DB::undo(std::unordered_set<DbObjHandle>& dirtyObjects) {
    std::unique_lock lock(m_mutex);
    if (m_inTxn || m_undoStack.empty())
        return Result::FAILURE; // Cannot undo
    const TxnRecord& lastTxn = m_undoStack.back();
    dirtyObjects.clear();
    for (auto it = lastTxn.rbegin(); it != lastTxn.rend(); ++it) {
        const Op& op = *it;
        if (undoOp(op) != Result::SUCCESS)
            return Result::FAILURE; // Undo operation failed
        dirtyObjects.insert(DbObjHandle(this, op.objId));
    }
    m_redoStack.push_back(lastTxn);
    m_undoStack.pop_back();
    m_modifyCount--;
    return Result::SUCCESS;
}

DB::Result DB::redo(std::unordered_set<DbObjHandle>& dirtyObjects) {
    std::unique_lock lock(m_mutex);
    if (m_inTxn || m_redoStack.empty())
        return Result::FAILURE; // Cannot redo
    const TxnRecord& lastRedo = m_redoStack.back();
    dirtyObjects.clear();
    for (const Op& op : lastRedo) {
        if (redoOp(op) != Result::SUCCESS)
            return Result::FAILURE; // Redo operation failed
        dirtyObjects.insert(DbObjHandle(this, op.objId));
    }
    m_undoStack.push_back(lastRedo);
    m_redoStack.pop_back();
    m_modifyCount++;
    return Result::SUCCESS;
}

bool DB::canUndo() const {
    return !m_undoStack.empty();
}

bool DB::canRedo() const {
    return !m_redoStack.empty();
}

void DB::setMaxUndoStackSize(size_t size) {
    std::unique_lock lock(m_mutex);
    m_maxUndoStackSize = size;
    while (m_undoStack.size() > m_maxUndoStackSize)
        m_undoStack.pop_front();
}

size_t DB::getMaxUndoStackSize() const {
    return m_maxUndoStackSize;
}

bool DB::isModified() const {
    return m_modifyCount > 0;
}

DB::Result DB::undoOp(const Op& op) {
    uint32_t index = op.objId & 0xFFFF;
    uint32_t gen = op.objId >> 16;
    // Ensure the index and generation are valid
    if (index >= m_objects.size()) {
        m_objects.resize(static_cast<size_t>(index) + 1);
        m_gens.resize(static_cast<size_t>(index) + 1, 0);
    }

    ObjectEntry& entry = m_objects[index];
    const DbTypeRegistry::TypeInfo* typeInfo =
        DbTypeRegistry::instance().getTypeInfo(op.typeName);
    if (!typeInfo)
        return Result::UNKONWN_TYPE;

    // restore "old" side
    if (op.oldAlive) {
        entry.id = op.objId;
        entry.typeName = op.typeName;
        entry.alive = true;
        entry.data = op.oldData;

        if (m_gens[index] < gen)
            m_gens[index] = gen;
        m_freeIndices.erase(index);
    } else {
        entry.alive = false;
        entry.data.reset();
        m_freeIndices.insert(index);
    }

    return Result::SUCCESS;
}

DB::Result DB::redoOp(const Op& op) {
    uint32_t index = op.objId & 0xFFFF;
    uint32_t gen = op.objId >> 16;
    // Ensure the index and generation are valid
    if (index >= m_objects.size()) {
        m_objects.resize(static_cast<size_t>(index) + 1);
        m_gens.resize(static_cast<size_t>(index) + 1, 0);
    }

    ObjectEntry& entry = m_objects[index];
    const DbTypeRegistry::TypeInfo* typeInfo =
        DbTypeRegistry::instance().getTypeInfo(op.typeName);
    if (!typeInfo)
        return Result::UNKONWN_TYPE;

    // apply "new" side
    if (op.newAlive) {
        entry.id = op.objId;
        entry.typeName = op.typeName;
        entry.alive = true;
        entry.data = op.newData;

        if (m_gens[index] < gen)
            m_gens[index] = gen;
        m_freeIndices.erase(index);
    } else {
        entry.alive = false;
        entry.data.reset();
        //entry.typeName.clear();
        m_freeIndices.insert(index);
    }

    return Result::SUCCESS;
}

void DB::rebuildFreeIndices() {
    m_freeIndices.clear();
    for (uint32_t i = 0; i < m_objects.size(); ++i) {
        if (!m_objects[i].alive)
            m_freeIndices.insert(i);
    }
}

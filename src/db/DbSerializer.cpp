/**
 * @file DbSerializer.cpp
 * @brief Implementation of the DbSerializer class for serializing and deserializing.
 */

#include "db/DbSerializer.h"

#include "db/DbPr.h"

void DbSerializer::serialize(bool value) {
    if (!m_stream || m_mode != SerializationMode::WRITE)
        return;
    m_stream->write(reinterpret_cast<const char*>(&value), sizeof(value));
}

void DbSerializer::serialize(int8_t value) {
    if (!m_stream || m_mode != SerializationMode::WRITE)
        return;
    m_stream->write(reinterpret_cast<const char*>(&value), sizeof(value));
}

void DbSerializer::serialize(uint8_t value) {
    if (!m_stream || m_mode != SerializationMode::WRITE)
        return;
    m_stream->write(reinterpret_cast<const char*>(&value), sizeof(value));
}

void DbSerializer::serialize(int16_t value) {
    if (!m_stream || m_mode != SerializationMode::WRITE)
        return;
    uint16_t netValue = htons(static_cast<uint16_t>(value));
    m_stream->write(reinterpret_cast<const char*>(&netValue), sizeof(netValue));
}

void DbSerializer::serialize(uint16_t value) {
    if (!m_stream || m_mode != SerializationMode::WRITE)
        return;
    uint16_t netValue = htons(value);
    m_stream->write(reinterpret_cast<const char*>(&netValue), sizeof(netValue));
}

void DbSerializer::serialize(int32_t value) {
    if (!m_stream || m_mode != SerializationMode::WRITE)
        return;
    uint32_t netValue = htonl(static_cast<uint32_t>(value));
    m_stream->write(reinterpret_cast<const char*>(&netValue), sizeof(netValue));
}

void DbSerializer::serialize(uint32_t value) {
    if (!m_stream || m_mode != SerializationMode::WRITE)
        return;
    uint32_t netValue = htonl(value);
    m_stream->write(reinterpret_cast<const char*>(&netValue), sizeof(netValue));
}

void DbSerializer::serialize(int64_t value) {
    if (!m_stream || m_mode != SerializationMode::WRITE)
        return;
    uint64_t netValue = htonll(static_cast<uint64_t>(value));
    m_stream->write(reinterpret_cast<const char*>(&netValue), sizeof(netValue));
}

void DbSerializer::serialize(uint64_t value) {
    if (!m_stream || m_mode != SerializationMode::WRITE)
        return;
    uint64_t netValue = htonll(value);
    m_stream->write(reinterpret_cast<const char*>(&netValue), sizeof(netValue));
}

void DbSerializer::serialize(float value) {
    if (!m_stream || m_mode != SerializationMode::WRITE)
        return;
    static_assert(sizeof(float) == sizeof(uint32_t), "float size is not 4 bytes");
    uint32_t intValue;
    std::memcpy(&intValue, &value, sizeof(value));
    uint32_t netValue = htonl(intValue);
    m_stream->write(reinterpret_cast<const char*>(&netValue), sizeof(netValue));
}

void DbSerializer::serialize(double value) {
    if (!m_stream || m_mode != SerializationMode::WRITE)
        return;
    static_assert(sizeof(double) == sizeof(uint64_t), "double size is not 8 bytes");
    uint64_t intValue;
    std::memcpy(&intValue, &value, sizeof(value));
    uint64_t netValue = htonll(intValue);
    m_stream->write(reinterpret_cast<const char*>(&netValue), sizeof(netValue));
}

void DbSerializer::serialize(const std::string& value) {
    if (!m_stream || m_mode != SerializationMode::WRITE)
        return;
    uint32_t size = static_cast<uint32_t>(value.size());
    serialize(size);
    m_stream->write(value.data(), size);
}

void DbSerializer::serialize(const DbFilePath& value) {
    if (!m_stream || m_mode != SerializationMode::WRITE)
        return;
    std::string relPath = value.path;
    if (!relPath.empty())
        relPath = DbFileUtils::getRelativePath(m_currentPath, value.path);
    serialize(relPath);
}

void DbSerializer::deserialize(bool& value) {
    if (!m_stream || m_mode != SerializationMode::READ)
        return;
    m_stream->read(reinterpret_cast<char*>(&value), sizeof(value));
}

void DbSerializer::deserialize(int8_t& value) {
    if (!m_stream || m_mode != SerializationMode::READ)
        return;
    m_stream->read(reinterpret_cast<char*>(&value), sizeof(value));
}

void DbSerializer::deserialize(uint8_t& value) {
    if (!m_stream || m_mode != SerializationMode::READ)
        return;
    m_stream->read(reinterpret_cast<char*>(&value), sizeof(value));
}

void DbSerializer::deserialize(int16_t& value) {
    if (!m_stream || m_mode != SerializationMode::READ)
        return;
    uint16_t netValue = 0;
    m_stream->read(reinterpret_cast<char*>(&netValue), sizeof(netValue));
    value = static_cast<int16_t>(ntohs(netValue));
}

void DbSerializer::deserialize(uint16_t& value) {
    if (!m_stream || m_mode != SerializationMode::READ)
        return;
    uint16_t netValue = 0;
    m_stream->read(reinterpret_cast<char*>(&netValue), sizeof(netValue));
    value = ntohs(netValue);
}

void DbSerializer::deserialize(int32_t& value) {
    if (!m_stream || m_mode != SerializationMode::READ)
        return;
    uint32_t netValue = 0;
    m_stream->read(reinterpret_cast<char*>(&netValue), sizeof(netValue));
    value = static_cast<int32_t>(ntohl(netValue));
}

void DbSerializer::deserialize(uint32_t& value) {
    if (!m_stream || m_mode != SerializationMode::READ)
        return;
    uint32_t netValue = 0;
    m_stream->read(reinterpret_cast<char*>(&netValue), sizeof(netValue));
    value = ntohl(netValue);
}

void DbSerializer::deserialize(int64_t& value) {
    if (!m_stream || m_mode != SerializationMode::READ)
        return;
    uint64_t netValue = 0;
    m_stream->read(reinterpret_cast<char*>(&netValue), sizeof(netValue));
    value = static_cast<int64_t>(ntohll(netValue));
}

void DbSerializer::deserialize(uint64_t& value) {
    if (!m_stream || m_mode != SerializationMode::READ)
        return;
    uint64_t netValue = 0;
    m_stream->read(reinterpret_cast<char*>(&netValue), sizeof(netValue));
    value = ntohll(netValue);
}

void DbSerializer::deserialize(float& value) {
    if (!m_stream || m_mode != SerializationMode::READ)
        return;
    static_assert(sizeof(float) == sizeof(uint32_t), "float size is not 4 bytes");
    uint32_t netValue = 0;
    m_stream->read(reinterpret_cast<char*>(&netValue), sizeof(netValue));
    uint32_t intValue = ntohl(netValue);
    std::memcpy(&value, &intValue, sizeof(value));
}

void DbSerializer::deserialize(double& value) {
    if (!m_stream || m_mode != SerializationMode::READ)
        return;
    static_assert(sizeof(double) == sizeof(uint64_t), "double size is not 8 bytes");
    uint64_t netValue = 0;
    m_stream->read(reinterpret_cast<char*>(&netValue), sizeof(netValue));
    uint64_t intValue = ntohll(netValue);
    std::memcpy(&value, &intValue, sizeof(value));
}

void DbSerializer::deserialize(std::string& value) {
    if (!m_stream || m_mode != SerializationMode::READ)
        return;
    uint32_t size = 0;
    deserialize(size);
    value.resize(size);
    m_stream->read(value.data(), size);
}

void DbSerializer::deserialize(DbFilePath& value) {
    if (!m_stream || m_mode != SerializationMode::READ)
        return;
    std::string relPath;
    deserialize(relPath);
    value.path = relPath;
    if (!relPath.empty())
        value.path = DbFileUtils::getAbsolutePath(m_currentPath, relPath);
}

uint16_t DbSerializer::htons(uint16_t hostshort) {
#ifdef _WIN32
    return _byteswap_ushort(hostshort);
#else
    return __builtin_bswap16(hostshort);
#endif
}

uint16_t DbSerializer::ntohs(uint16_t netshort) {
    return htons(netshort);
}

uint32_t DbSerializer::htonl(uint32_t hostlong) {
#ifdef _WIN32
    return _byteswap_ulong(hostlong);
#else
    return __builtin_bswap32(hostlong);
#endif
}

uint32_t DbSerializer::ntohl(uint32_t netlong) {
    return htonl(netlong);
}

uint64_t DbSerializer::htonll(uint64_t hostlonglong) {
#ifdef _WIN32
    return _byteswap_uint64(hostlonglong);
#else
    return __builtin_bswap64(hostlonglong);
#endif
}

uint64_t DbSerializer::ntohll(uint64_t netlonglong) {
    return htonll(netlonglong);
}

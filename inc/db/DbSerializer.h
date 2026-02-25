/**
 * @file DbSerializer.h
 * @brief Declaration of the DbSerializer class for serializing and deserializing data.
 */

#pragma once

#include "DbCommon.h"

struct DbFilePath {
    std::string path;
};

/**
 * @brief Class for serializing and deserializing various data types to/from a binary stream.
 */
class DbSerializer {
public:
    enum class SerializationMode {
        READ,
        WRITE,
    };

    DbSerializer(SerializationMode mode, std::iostream& stream, const std::string& path) :
        m_mode(mode),
        m_stream(&stream),
        m_currentPath(path) {};

    void serialize(bool value);
    void serialize(int8_t value);
    void serialize(uint8_t value);
    void serialize(int16_t value);
    void serialize(uint16_t value);
    void serialize(int32_t value);
    void serialize(uint32_t value);
    void serialize(int64_t value);
    void serialize(uint64_t value);
    void serialize(float value);
    void serialize(double value);
    void serialize(const std::string& value);
    void serialize(const DbFilePath& value);

    void deserialize(bool& value);
    void deserialize(int8_t& value);
    void deserialize(uint8_t& value);
    void deserialize(int16_t& value);
    void deserialize(uint16_t& value);
    void deserialize(int32_t& value);
    void deserialize(uint32_t& value);
    void deserialize(int64_t& value);
    void deserialize(uint64_t& value);
    void deserialize(float& value);
    void deserialize(double& value);
    void deserialize(std::string& value);
    void deserialize(DbFilePath& value);

    template<typename T>
    void serialize(const std::vector<T>& vec) {
        uint32_t size = static_cast<uint32_t>(vec.size());
        serialize(size);
        for (const auto& item : vec)
            serialize(item);
    };
    template<typename K, typename V>
    void serialize(const std::map<K, V>& m) {
        uint32_t size = static_cast<uint32_t>(m.size());
        serialize(size);
        for (const auto& [key, value] : m) {
            serialize(key);
            serialize(value);
        }
    };
    template<typename K, typename V>
    void serialize(const std::unordered_map<K, V>& m) {
        uint32_t size = static_cast<uint32_t>(m.size());
        serialize(size);
        for (const auto& [key, value] : m) {
            serialize(key);
            serialize(value);
        }
    };
    template<typename T>
    void serialize(const std::set<T>& s) {
        uint32_t size = static_cast<uint32_t>(s.size());
        serialize(size);
        for (const auto& item : s)
            serialize(item);
    };
    template<typename T>
    void serialize(const std::unordered_set<T>& s) {
        uint32_t size = static_cast<uint32_t>(s.size());
        serialize(size);
        for (const auto& item : s)
            serialize(item);
    };

    template<typename T>
    void deserialize(std::vector<T>& vec) {
        uint32_t size;
        deserialize(size);
        vec.resize(size);
        for (auto& item : vec)
            deserialize(item);
    };
    template<typename K, typename V>
    void deserialize(std::map<K, V>& m) {
        uint32_t size;
        deserialize(size);
        for (uint32_t i = 0; i < size; ++i) {
            K key{};
            V value{};
            deserialize(key);
            deserialize(value);
            m.emplace(std::move(key), std::move(value));
        }
    };
    template<typename K, typename V>
    void deserialize(std::unordered_map<K, V>& m) {
        uint32_t size;
        deserialize(size);
        for (uint32_t i = 0; i < size; ++i) {
            K key{};
            V value{};
            deserialize(key);
            deserialize(value);
            m.emplace(std::move(key), std::move(value));
        }
    };
    template<typename T>
    void deserialize(std::set<T>& s) {
        uint32_t size;
        deserialize(size);
        for (uint32_t i = 0; i < size; ++i) {
            T item{};
            deserialize(item);
            s.emplace(std::move(item));
        }
    };
    template<typename T>
    void deserialize(std::unordered_set<T>& s) {
        uint32_t size;
        deserialize(size);
        for (uint32_t i = 0; i < size; ++i) {
            T item{};
            deserialize(item);
            s.emplace(std::move(item));
        }
    };

private:
    static uint16_t htons(uint16_t hostshort);
    static uint16_t ntohs(uint16_t netshort);
    static uint32_t htonl(uint32_t hostlong);
    static uint32_t ntohl(uint32_t netlong);
    static uint64_t htonll(uint64_t hostlonglong);
    static uint64_t ntohll(uint64_t netlonglong);

private:
    SerializationMode m_mode;
    std::iostream* m_stream;
    std::string m_currentPath;
};

/**
 * @file Mesh.cpp
 * @brief Implementation of functions for loading and managing 3D mesh data.
 */

#include "utils/Mesh.h"

namespace MeshParser {

/**
 * @brief Class for parsing OBJ files and populating a Mesh::Model.
 */
class OBJParser {
public:
    OBJParser(Mesh::Model& model, std::ifstream& ifs) : m_model(model), m_ifs(ifs) {};

    /**
     * @brief Parse the OBJ file and populate the model.
     */
    void parseFull() {
        std::string line;
        while (std::getline(m_ifs, line)) {
            if (line.empty() || line[0] == '#')
                continue;

            std::istringstream iss(line);
            std::string head;
            iss >> head;

            if (head == "v")
                parseVertex(iss);
            else if (head == "vt")
                parseTexCoord(iss);
            else if (head == "vn")
                parseNormal(iss);
            else if (head == "o")
                parseObject(iss);
            else if (head == "g")
                parseGroup(iss);
            else if (head == "s")
                parseSmoothing(iss);
            else if (head == "f")
                parseFace(iss);
        }

        processTriangles();
        finalizeVertices();
    }

    /**
     * @brief Parse the OBJ file to retrieve only model information (names of objects and groups).
     */
    void parseInfoOnly() {
        std::string line;
        while (std::getline(m_ifs, line)) {
            if (line.empty() || line[0] == '#')
                continue;

            std::istringstream iss(line);
            std::string head;
            iss >> head;

            if (head == "o")
                parseObject(iss);
            else if (head == "g")
                parseGroup(iss);
        }

        if (m_model.meshes.empty()) {
            m_model.meshes.push_back({ "" });
            m_model.meshes[0].submeshes.push_back({ "" });
        }
    }

private:
    /**
     * @brief Vertex structure for parsing OBJ files.
     */
    struct Vtx {
        int idx_p = -1; // index of position in the vertex list
        int idx_t = -1; // index of texture coordinate in the texcoord list
        int idx_n = -1; // index of normal in the normal list
    };
    /**
     * @brief Triangle structure for storing parsed triangles.
     * @note Contains vertex indices, smoothing group, and mesh/submesh indices.
     */
    struct Tri {
        Vtx a; // first vertex of the triangle
        Vtx b; // second vertex of the triangle
        Vtx c; // third vertex of the triangle

        int smooth_group; // smoothing group ID (0 means "off")
        size_t idx_mesh; // index of the mesh in the model
        size_t idx_submesh; // index of the submesh in the mesh
    };

    /**
     * @brief Key for welding vertices in the OBJ file.
     * @note Only share by position and UV within the same smoothing group (no normal).
     */
    struct WeldKey {
        int idx_p; // position index
        int idx_t; // texcoord index (-1 if none)
        int smooth_group; // smoothing group id (0 means "off")

        bool operator==(const WeldKey& o) const noexcept {
            return idx_p == o.idx_p && idx_t == o.idx_t && smooth_group == o.smooth_group;
        }
    };
    /**
     * @brief Hash function for WeldKey.
     * @note Used in unordered_map to store shared vertices.
     */
    struct WeldKeyHash {
        size_t operator()(const WeldKey& k) const noexcept {
            size_t h = std::hash<int>()(k.idx_p);
            h ^= std::hash<int>()(k.idx_t) << 1;
            h ^= std::hash<int>()(k.smooth_group) << 2;
            return h;
        }
    };
    /**
     * @brief Accumulation structure for normals and tangents.
     * @note Used to accumulate normals and tangents for shared vertices.
     */
    struct Accum {
        Math::Vec3 n = {}; // normal accumulation
        Math::Vec3 t = {}; // tangent accumulation
        bool has_n = false; // has any normal
        bool has_t = false; // has any tangent
    };
    /**
     * @brief Bucket structure for shared vertices.
     */
    struct Bucket {
        std::unordered_map<WeldKey, unsigned int, WeldKeyHash> map; // key to vertex index
        std::vector<Accum> acc; // align with vertex list
    };

    /**
     * @brief Parse a vertex line from the OBJ file.
     * @param iss Input string stream containing the vertex data.
     */
    void parseVertex(std::istringstream& iss) {
        Math::Vec3 p;
        iss >> p.x >> p.y >> p.z;
        m_positions.push_back(p);
    }

    /**
     * @brief Parse a texture coordinate line from the OBJ file.
     * @param iss Input string stream containing the texture coordinate data.
     */
    void parseTexCoord(std::istringstream& iss) {
        Math::Vec2 t;
        iss >> t.x >> t.y;
        m_texcoords.push_back(t);
    }

    /**
     * @brief Parse a normal line from the OBJ file.
     * @param iss Input string stream containing the normal data.
     */
    void parseNormal(std::istringstream& iss) {
        Math::Vec3 n;
        iss >> n.x >> n.y >> n.z;
        m_normalsIn.push_back(n);
    }

    /**
     * @brief Parse an object line from the OBJ file.
     * @param iss Input string stream containing the object name.
     */
    void parseObject(std::istringstream& iss) {
        std::string name;
        std::getline(iss, name);
        if (!name.empty() && isspace(name[0]))
            name.erase(0, 1);

        if (m_model.meshes.empty()) {
            m_model.meshes.push_back({ name });
            m_curMesh = 0;
        } else if (m_model.meshes.size() == 1 && m_model.meshes[0].name.empty()) {
            m_model.meshes[0].name = name;
            m_curMesh = 0;
        } else {
            m_model.meshes.push_back({ name });
            m_curMesh = m_model.meshes.size() - 1;
        }

        if (m_model.meshes[m_curMesh].submeshes.empty())
            m_model.meshes[m_curMesh].submeshes.push_back({ "" });
        m_curSubMesh = 0;
    }

    /**
     * @brief Parse a group line from the OBJ file.
     * @param iss Input string stream containing the group name.
     */
    void parseGroup(std::istringstream& iss) {
        std::string gname;
        iss >> gname;
        if (gname.empty())
            gname = "";

        if (m_model.meshes.empty()) {
            m_model.meshes.push_back({ "" });
            m_curMesh = 0;
        }
        if (m_model.meshes[m_curMesh].submeshes.empty()) {
            m_model.meshes[m_curMesh].submeshes.push_back({ "" });
            m_curSubMesh = 0;
        }

        auto& currentSubmeshes = m_model.meshes[m_curMesh].submeshes;
        if (currentSubmeshes[m_curSubMesh].indices.empty() && m_curSubMesh == 0)
            currentSubmeshes[m_curSubMesh].name = gname;
        else {
            currentSubmeshes.push_back({ gname });
            m_curSubMesh = currentSubmeshes.size() - 1;
        }
    }

    /**
     * @brief Parse a smoothing group line from the OBJ file.
     * @param iss Input string stream containing the smoothing group data.
     * @note "off" or "0" means no smoothing, otherwise it is an integer ID.
     */
    void parseSmoothing(std::istringstream& iss) {
        std::string tok;
        iss >> tok;
        if (tok == "off" || tok == "0")
            m_currentSmoothGroup = 0;
        else {
            try {
                m_currentSmoothGroup = std::stoi(tok);
            } catch (...) {
                m_currentSmoothGroup = 0;
            }
        }
    }

    /**
     * @brief Parse a face line from the OBJ file.
     * @param iss Input string stream containing the face data.
     * @note Faces are defined by vertex indices, which may include texture and normal indices.
     */
    void parseFace(std::istringstream& iss) {
        if (m_curMesh == -1) {
            m_model.meshes.push_back({ "" });
            m_model.meshes[0].submeshes.push_back({ "" });
            m_curMesh = 0;
            m_curSubMesh = 0;
        }

        std::vector<Vtx> corners;
        std::string tok;

        while (iss >> tok) {
            Vtx vtx = parseVertexToken(tok);
            if (vtx.idx_p >= 0)
                corners.push_back(vtx);
        }

        if (corners.size() < 3)
            return;

        // fan triangulation
        for (size_t i = 1; i + 1 < corners.size(); ++i) {
            m_tris.push_back
            (
                {
                    corners[0],
                    corners[i],
                    corners[i + 1],
                    m_currentSmoothGroup,
                    m_curMesh,
                    m_curSubMesh
                }
            );
        }
    }

    /**
     * @brief Parse a vertex token from the OBJ file.
     * @param tok The token representing a vertex.
     * @return Parsed vertex structure with indices for position, texture, and normal.
     */
    Vtx parseVertexToken(const std::string& tok) {
        Vtx vtx;
        size_t p1 = tok.find('/'), p2 = tok.find('/', p1 + 1);

        auto parse = [](const std::string& s) {
            return s.empty() ? std::numeric_limits<int>::min() : std::stoi(s);
            };

        if (p1 == std::string::npos)
            vtx.idx_p = parse(tok);
        else {
            std::string sp = tok.substr(0, p1);
            std::string st = (p2 == std::string::npos) ?
                tok.substr(p1 + 1) : tok.substr(p1 + 1, p2 - p1 - 1);
            std::string sn = (p2 == std::string::npos) ? "" : tok.substr(p2 + 1);

            vtx.idx_p = parse(sp);
            vtx.idx_t = parse(st);
            vtx.idx_n = parse(sn);
        }

        vtx.idx_p = fixIndex(vtx.idx_p, m_positions.size());
        vtx.idx_t = fixIndex(vtx.idx_t, m_texcoords.size());
        vtx.idx_n = fixIndex(vtx.idx_n, m_normalsIn.size());

        return vtx;
    }

    /**
     * @brief Fix the index to be within the valid range.
     * @param idx The index to fix.
     * @param count The total number of elements in the list.
     * @return A valid index or -1 if the index is invalid.
     */
    int fixIndex(int idx, int count) {
        if (idx == std::numeric_limits<int>::min())
            return -1;
        if (idx > 0)
            return idx - 1;
        if (idx < 0)
            return count + idx;
        return -1;
    }

    /**
     * @brief Traverse all triangles first, generate vertices (shared/unshared),
     *        and accumulate normals and tangents.
     */
    void processTriangles() {
        m_buckets.resize(m_model.meshes.size());

        for (const auto& tri : m_tris) {
            auto& mesh = m_model.meshes[tri.idx_mesh];
            auto& sub = mesh.submeshes[tri.idx_submesh];

            if (tri.smooth_group == 0)
                processNonSmoothTriangle(tri, mesh, sub);
            else
                processSmoothTriangle(tri, mesh, sub);
        }
    }

    /**
     * @brief Process unshared vertices: no welding, just emit.
     * @param tri Triangle data containing vertex indices and smoothing group.
     * @param mesh The mesh to which the triangle belongs.
     * @param sub The submesh to which the triangle belongs.
     */
    void processNonSmoothTriangle(const Tri& tri, Mesh::Mesh& mesh, Mesh::SubMesh& sub) {
        Math::Vec3 fn, ft;
        computeFaceGeometry(tri, fn, ft);

        unsigned int i0 = emitVertex(mesh, tri.a, fn, ft);
        unsigned int i1 = emitVertex(mesh, tri.b, fn, ft);
        unsigned int i2 = emitVertex(mesh, tri.c, fn, ft);

        sub.indices.insert(sub.indices.end(), { i0, i1, i2 });
    }

    /**
     * @brief Process Shared vertices: weld by position and UV, accumulate normals and tangents.
     * @param tri Triangle data containing vertex indices and smoothing group.
     * @param mesh The mesh to which the triangle belongs.
     * @param sub The submesh to which the triangle belongs.
     */
    void processSmoothTriangle(const Tri& tri, Mesh::Mesh& mesh, Mesh::SubMesh& sub) {
        auto& bucket = m_buckets[tri.idx_mesh];
        Math::Vec3 fn, ft;
        computeFaceGeometry(tri, fn, ft);

        unsigned int i0 = bindVertex(bucket, tri.a, tri.smooth_group, mesh);
        unsigned int i1 = bindVertex(bucket, tri.b, tri.smooth_group, mesh);
        unsigned int i2 = bindVertex(bucket, tri.c, tri.smooth_group, mesh);

        accumulateVertex(bucket, i0, tri.a, fn, ft);
        accumulateVertex(bucket, i1, tri.b, fn, ft);
        accumulateVertex(bucket, i2, tri.c, fn, ft);

        sub.indices.insert(sub.indices.end(), { i0, i1, i2 });
    }

    /**
     * @brief Compute the face normal and tangent for a triangle.
     * @param tri The triangle data containing vertex indices.
     * @param fn Output face normal.
     * @param ft Output face tangent.
     */
    void computeFaceGeometry(const Tri& tri, Math::Vec3& fn, Math::Vec3& ft) {
        using namespace Math;

        Vec3 p0 = m_positions[tri.a.idx_p];
        Vec3 p1 = m_positions[tri.b.idx_p];
        Vec3 p2 = m_positions[tri.c.idx_p];
        fn = normalize(cross(p1 - p0, p2 - p0));

        Vec2 uv0 = (tri.a.idx_t >= 0) ? m_texcoords[tri.a.idx_t] : Vec2();
        Vec2 uv1 = (tri.b.idx_t >= 0) ? m_texcoords[tri.b.idx_t] : Vec2();
        Vec2 uv2 = (tri.c.idx_t >= 0) ? m_texcoords[tri.c.idx_t] : Vec2();

        Vec3 e1 = p1 - p0, e2 = p2 - p0;
        float du1 = uv1.x - uv0.x, dv1 = uv1.y - uv0.y;
        float du2 = uv2.x - uv0.x, dv2 = uv2.y - uv0.y;
        float denom = du1 * dv2 - du2 * dv1;
        // Compute the face tangent, if UVs are degenerate, use an arbitrary tangent
        if (std::fabs(denom) < std::numeric_limits<float>::epsilon())
            ft = buildArbitraryTangent(fn);
        else
            ft = (e1 * dv2 - e2 * dv1) * (1.0f / denom);

        if (dot(ft, ft) < std::numeric_limits<float>::epsilon())
            ft = buildArbitraryTangent(fn);
    }

    /**
     * @brief Build an arbitrary tangent vector based on the normal.
     * @param n The normal vector.
     * @return A normalized tangent vector orthogonal to the normal.
     * @note If the normal is aligned with the Z-axis, use Y-axis as a reference.
     */
    static Math::Vec3 buildArbitraryTangent(const Math::Vec3& n) {
        using namespace Math;
        Vec3 a = (std::fabs(n.z) < (1.0f - std::numeric_limits<float>::epsilon())) ?
            Vec3(0.0f, 0.0f, 1.0f) : Vec3(0.0f, 1.0f, 0.0f);
        Vec3 t = cross(a, n);
        if (dot(t, t) < std::numeric_limits<float>::epsilon())
            return Vec3(1.0f, 0.0f, 0.0f);
        return normalize(t);
    }

    /**
     * @brief Emit a vertex into the mesh.
     * @param mesh The mesh to which the vertex belongs.
     * @param vtx The vertex data containing indices for position, texture, and normal.
     * @param fn The face normal for the vertex.
     * @param ft The face tangent for the vertex.
     * @return The index of the emitted vertex in the mesh.
     */
    unsigned int emitVertex(
        Mesh::Mesh& mesh,
        const Vtx& vtx,
        const Math::Vec3& fn,
        const Math::Vec3& ft
    ) {
        using namespace Math;

        Mesh::Vertex v{};
        v.pos = m_positions[vtx.idx_p];
        v.texCoord = (vtx.idx_t >= 0) ? m_texcoords[vtx.idx_t] : Vec2();
        // Normal: prefer vertex normal, otherwise use face normal
        v.normal = (vtx.idx_n >= 0) ? normalize(m_normalsIn[vtx.idx_n]) : fn;

        // Tangent: if the face tangent is valid, perform orthogonalization,
        // otherwise build from the normal
        Vec3 t = ft;
        if (dot(t, t) < std::numeric_limits<float>::epsilon())
            t = buildArbitraryTangent(v.normal);
        else
            t = normalize(t - v.normal * dot(t, v.normal));

        v.tangent = t;

        unsigned int out = mesh.vertices.size();
        mesh.vertices.push_back(v);
        return out;
    }

    /**
     * @brief Bind a vertex to the shared bucket, welding by position and UV.
     * @param bucket The bucket to which the vertex belongs.
     * @param vtx The vertex data containing indices for position, texture, and normal.
     * @param smoothGroup The smoothing group ID for the vertex.
     * @return The index of the bound vertex in the mesh.
     */
    unsigned int bindVertex(Bucket& bucket, const Vtx& vtx, int smoothGroup, Mesh::Mesh& mesh) {
        WeldKey k{ vtx.idx_p, vtx.idx_t, smoothGroup };
        auto it = bucket.map.find(k);
        if (it != bucket.map.end())
            return it->second;

        Mesh::Vertex v{};
        v.pos = m_positions[vtx.idx_p];
        v.texCoord = (vtx.idx_t >= 0) ? m_texcoords[vtx.idx_t] : Math::Vec2();
        v.normal = Math::Vec3();
        v.tangent = Math::Vec3();

        unsigned int idx = mesh.vertices.size();
        mesh.vertices.push_back(v);
        bucket.map[k] = idx;
        if (bucket.acc.size() <= idx)
            bucket.acc.resize(mesh.vertices.size());

        return idx;
    }

    /**
     * @brief Accumulate vertex normals and tangents into the shared bucket.
     * @note Prioritize explicit vertex normals, otherwise use face normal.
     *       Tangents always accumulate face tangents.
     * @param bucket The bucket to which the vertex belongs.
     * @param idx The index of the vertex in the bucket.
     * @param vtx The vertex data containing indices for position, texture, and normal.
     * @param fn Face normal for the vertex.
     * @param ft Face tangent for the vertex.
     */
    void accumulateVertex(
        Bucket& bucket,
        unsigned int idx,
        const Vtx& vtx,
        const Math::Vec3& fn,
        const Math::Vec3& ft
    ) {
        if (idx >= bucket.acc.size())
            bucket.acc.resize(static_cast<size_t>(idx) + 1);
        Accum& a = bucket.acc[idx];
        if (vtx.idx_n >= 0) {
            a.n = a.n + Math::normalize(m_normalsIn[vtx.idx_n]);
            a.has_n = true;
        } else {
            a.n = a.n + fn;
            a.has_n = true;
        }
        a.t = a.t + ft;
        a.has_t = true;
    }

    /**
     * @brief Normalize the accumulation result of the shared bucked
     *        and perform tangent orthogonalization.
     */
    void finalizeVertices() {
        using namespace Math;
        for (size_t mi = 0; mi < m_model.meshes.size(); ++mi) {
            auto& mesh = m_model.meshes[mi];
            auto& bucket = m_buckets[mi];

            for (size_t i = 0; i < bucket.acc.size() && i < mesh.vertices.size(); ++i) {
                auto& v = mesh.vertices[i];
                auto& a = bucket.acc[i];

                Vec3 n = a.has_n ? a.n : Vec3(0.0f, 1.0f, 0.0f);
                if (dot(n, n) < std::numeric_limits<float>::epsilon())
                    n = Vec3(0.0f, 1.0f, 0.0f);
                else
                    n = normalize(n);
                v.normal = n;

                Vec3 t = a.has_t ? a.t : buildArbitraryTangent(n);
                // Normalize and orthogonalize
                t = t - n * dot(t, n);
                if (dot(t, t) < std::numeric_limits<float>::epsilon())
                    t = buildArbitraryTangent(n);
                else
                    t = normalize(t);
                v.tangent = t;
            }
        }
    }

private:
    Mesh::Model& m_model; // The model to populate with parsed data
    std::ifstream& m_ifs; // Input file stream for reading the OBJ file
    std::vector<Math::Vec3> m_positions; // List of vertex positions
    std::vector<Math::Vec3> m_normalsIn; // List of input normals
    std::vector<Math::Vec2> m_texcoords; // List of texture coordinates
    size_t m_curMesh = -1; // Current mesh index in the model
    size_t m_curSubMesh = -1; // Current submesh index in the current mesh
    int m_currentSmoothGroup = 0; // Current smoothing group ID
    std::vector<Tri> m_tris; // List of triangles parsed from the OBJ file
    std::vector<Bucket> m_buckets; // Buckets for shared vertices, indexed by mesh
};

} // namespace MeshParser

int MeshLoader::loadOBJ(const std::string& filename, Mesh::Model& model) {
    model.meshes.clear();
    std::filesystem::path filePath(filename);
    model.name = filePath.stem().string();

    std::ifstream ifs(filename);
    if (!ifs.is_open())
        return 1; // Failed to open file

    MeshParser::OBJParser parser(model, ifs);
    parser.parseFull();

    if (!model.meshes.empty()) {
        for (const auto& mesh : model.meshes) {
            if (mesh.vertices.empty())
                return 1; // Invalid mesh with no vertices
        }
    } else
        return 1; // No meshes found

    return 0;
}

int MeshLoader::getInfoFromOBJ(const std::string& filename, Mesh::Model& model) {
    model.meshes.clear();
    std::filesystem::path filePath(filename);
    model.name = filePath.stem().string();

    std::ifstream ifs(filename);
    if (!ifs.is_open())
        return 1; // Failed to open file

    MeshParser::OBJParser parser(model, ifs);
    parser.parseInfoOnly();

    return 0;
}

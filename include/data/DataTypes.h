#pragma once

#include <vector>
#include <string>
#include <memory>
#include <glm/glm.hpp>

namespace mviz {

/**
 * 点云数据结构
 */
struct PointCloudData {
    std::vector<glm::vec3> points;
    std::vector<glm::vec3> colors;  // RGB颜色，每个点一个
    float pointSize = 1.0f;
    
    PointCloudData() = default;
    
    void clear() {
        points.clear();
        colors.clear();
    }
    
    size_t size() const {
        return points.size();
    }
    
    bool empty() const {
        return points.empty();
    }
};

/**
 * 球体数据结构
 */
struct SphereData {
    glm::vec3 center{0.0f};
    float radius = 1.0f;
    glm::vec3 color{1.0f, 1.0f, 1.0f};
    bool wireframe = false;
};

/**
 * 线条数据结构
 */
struct LineData {
    std::vector<glm::vec3> points;
    std::vector<glm::vec3> colors;
    float lineWidth = 1.0f;
    bool loop = false;  // 是否闭合
    
    LineData() = default;
    
    void clear() {
        points.clear();
        colors.clear();
    }
    
    size_t size() const {
        return points.size();
    }
    
    bool empty() const {
        return points.empty();
    }
};

/**
 * 网格数据结构
 */
struct MeshData {
    std::vector<glm::vec3> vertices;
    std::vector<glm::vec3> normals;
    std::vector<glm::vec2> uvs;
    std::vector<unsigned int> indices;
    glm::vec3 color{1.0f, 1.0f, 1.0f};
    bool wireframe = false;
    
    MeshData() = default;
    
    void clear() {
        vertices.clear();
        normals.clear();
        uvs.clear();
        indices.clear();
    }
    
    size_t vertexCount() const {
        return vertices.size();
    }
    
    size_t indexCount() const {
        return indices.size();
    }
    
    bool empty() const {
        return vertices.empty();
    }
};

} // namespace mviz 
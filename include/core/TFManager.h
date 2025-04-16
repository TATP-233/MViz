#pragma once

#include <string>
#include <map>
#include <memory>
#include <vector>
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

namespace mviz {

// 表示一个坐标系变换
struct Transform {
    glm::vec3 translation;
    glm::quat rotation;
    
    // 默认构造函数
    Transform() 
        : translation(0.0f, 0.0f, 0.0f)
        , rotation(1.0f, 0.0f, 0.0f, 0.0f) // 单位四元数
    {}
    
    // 带参数构造函数
    Transform(const glm::vec3& t, const glm::quat& r)
        : translation(t)
        , rotation(r)
    {}
    
    // 转换为4x4矩阵
    glm::mat4 toMat4() const;
    
    // 变换组合（右乘）
    Transform operator*(const Transform& other) const;
    
    // 反转变换
    Transform inverse() const;
};

// 表示TF树中的一个节点
class TransformNode {
public:
    TransformNode(const std::string& name);
    ~TransformNode();
    
    // 获取节点名称
    const std::string& getName() const { return m_name; }
    
    // 设置父节点
    void setParent(TransformNode* parent, const Transform& transform);
    
    // 获取父节点
    TransformNode* getParent() const { return m_parent; }
    
    // 添加子节点
    void addChild(TransformNode* child);
    
    // 删除子节点
    void removeChild(const std::string& name);
    
    // 获取相对于父节点的变换
    const Transform& getTransform() const { return m_transform; }
    
    // 设置相对于父节点的变换
    void setTransform(const Transform& transform);
    
    // 获取子节点列表
    const std::vector<TransformNode*>& getChildren() const { return m_children; }
    
private:
    std::string m_name;
    TransformNode* m_parent;
    Transform m_transform;
    std::vector<TransformNode*> m_children;
};

// TF管理器类
class TFManager {
public:
    TFManager();
    ~TFManager();
    
    // 添加或更新一个变换（从parent_frame到child_frame）
    void addTransform(const std::string& parent_frame, const std::string& child_frame, 
                     const Transform& transform);
    
    // 移除一个变换节点
    void removeTransform(const std::string& frame);
    
    // 查找从source_frame到target_frame的变换
    bool lookupTransform(const std::string& target_frame, const std::string& source_frame,
                        Transform& transform) const;
    
    // 获取所有坐标系名称
    std::vector<std::string> getAllFrameNames() const;
    
    // 获取指定坐标系的位置（在世界坐标系下）
    glm::vec3 getFramePosition(const std::string& frame) const;
    
    // 获取图可视化的数据：起点、终点和标签
    void getConnectionsForRendering(std::vector<std::pair<glm::vec3, glm::vec3>>& connections) const;
    
private:
    // 查找节点，如果不存在则创建
    TransformNode* findOrCreateNode(const std::string& name);
    
    // 查找节点
    TransformNode* findNode(const std::string& name) const;
    
    // 计算从source到target的路径，返回沿路径的变换序列和方向
    bool findTransformPath(const TransformNode* target, const TransformNode* source,
                          std::vector<std::pair<const TransformNode*, bool>>& path) const;
    
    // 世界坐标系节点
    TransformNode* m_worldNode;
    
    // 所有坐标系节点的映射表
    std::map<std::string, std::unique_ptr<TransformNode>> m_nodes;
};

} // namespace mviz 
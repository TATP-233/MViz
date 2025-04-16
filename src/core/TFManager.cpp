#include "core/TFManager.h"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/matrix_decompose.hpp>
#include <algorithm>
#include <iostream>
#include <queue>

namespace mviz {

//-------------------- Transform 实现 --------------------

glm::mat4 Transform::toMat4() const {
    // 创建从旋转四元数得到的旋转矩阵
    glm::mat4 rotMat = glm::mat4_cast(rotation);
    
    // 设置平移部分
    rotMat[3][0] = translation.x;
    rotMat[3][1] = translation.y;
    rotMat[3][2] = translation.z;
    
    return rotMat;
}

Transform Transform::operator*(const Transform& other) const {
    // 组合变换
    Transform result;
    result.rotation = rotation * other.rotation;
    result.translation = translation + rotation * other.translation;
    return result;
}

Transform Transform::inverse() const {
    // 反转变换
    Transform result;
    result.rotation = glm::inverse(rotation);
    result.translation = -(result.rotation * translation);
    return result;
}

//-------------------- TransformNode 实现 --------------------

TransformNode::TransformNode(const std::string& name)
    : m_name(name)
    , m_parent(nullptr)
{
}

TransformNode::~TransformNode() {
    // 清理工作 - 在析构时不需要删除子节点，因为它们由TFManager管理
}

void TransformNode::setParent(TransformNode* parent, const Transform& transform) {
    // 如果有旧的父节点，从它的子节点列表中移除自己
    if (m_parent) {
        m_parent->removeChild(m_name);
    }
    
    m_parent = parent;
    m_transform = transform;
    
    // 如果有新的父节点，将自己添加到它的子节点列表中
    if (m_parent) {
        m_parent->addChild(this);
    }
}

void TransformNode::addChild(TransformNode* child) {
    // 检查是否已经是子节点
    auto it = std::find(m_children.begin(), m_children.end(), child);
    if (it == m_children.end()) {
        m_children.push_back(child);
    }
}

void TransformNode::removeChild(const std::string& name) {
    m_children.erase(
        std::remove_if(m_children.begin(), m_children.end(),
            [&name](const TransformNode* node) { return node->getName() == name; }),
        m_children.end()
    );
}

void TransformNode::setTransform(const Transform& transform) {
    m_transform = transform;
}

//-------------------- TFManager 实现 --------------------

TFManager::TFManager() {
    // 创建世界坐标系节点
    m_worldNode = findOrCreateNode("world");
}

TFManager::~TFManager() {
    // 清理节点
    m_nodes.clear();
    m_worldNode = nullptr;
}

void TFManager::addTransform(const std::string& parent_frame, const std::string& child_frame, 
                            const Transform& transform) {
    // 查找或创建父节点和子节点
    TransformNode* parentNode = findOrCreateNode(parent_frame);
    TransformNode* childNode = findOrCreateNode(child_frame);
    
    // 设置子节点的父节点和变换关系
    childNode->setParent(parentNode, transform);
}

void TFManager::removeTransform(const std::string& frame) {
    // 不能删除世界坐标系
    if (frame == "world") {
        return;
    }
    
    auto it = m_nodes.find(frame);
    if (it != m_nodes.end()) {
        TransformNode* node = it->second.get();
        
        // 获取父节点和子节点
        TransformNode* parent = node->getParent();
        const auto& children = node->getChildren();
        
        // 将子节点重新连接到父节点
        if (parent) {
            for (TransformNode* child : children) {
                // 计算子节点相对于父节点的新变换
                Transform parentToNode = node->getTransform();
                Transform nodeToChild = child->getTransform();
                Transform parentToChild = parentToNode * nodeToChild;
                
                // 更新子节点的父节点和变换
                child->setParent(parent, parentToChild);
            }
            
            // 从父节点的子节点列表中移除自己
            parent->removeChild(frame);
        } else {
            // 如果没有父节点（这应该只可能是世界坐标系），将子节点变成独立节点
            for (TransformNode* child : children) {
                child->setParent(nullptr, child->getTransform());
            }
        }
        
        // 从节点映射表中移除
        m_nodes.erase(it);
    }
}

bool TFManager::lookupTransform(const std::string& target_frame, const std::string& source_frame,
                               Transform& transform) const {
    // 特殊情况：源和目标是同一个坐标系
    if (target_frame == source_frame) {
        transform = Transform(); // 单位变换
        return true;
    }
    
    // 查找源和目标节点
    const TransformNode* sourceNode = findNode(source_frame);
    const TransformNode* targetNode = findNode(target_frame);
    
    if (!sourceNode || !targetNode) {
        return false; // 未找到节点
    }
    
    // 查找从source到target的路径
    std::vector<std::pair<const TransformNode*, bool>> path;
    if (!findTransformPath(targetNode, sourceNode, path)) {
        return false; // 无法找到路径
    }
    
    // 沿路径计算最终变换
    transform = Transform(); // 初始为单位变换
    
    for (const auto& [node, inverse] : path) {
        if (inverse) {
            // 如果方向是反向的，使用逆变换
            transform = transform * node->getTransform().inverse();
        } else {
            // 正向使用常规变换
            transform = transform * node->getTransform();
        }
    }
    
    return true;
}

std::vector<std::string> TFManager::getAllFrameNames() const {
    std::vector<std::string> names;
    names.reserve(m_nodes.size());
    
    for (const auto& [name, _] : m_nodes) {
        names.push_back(name);
    }
    
    return names;
}

glm::vec3 TFManager::getFramePosition(const std::string& frame) const {
    Transform worldToFrame;
    if (lookupTransform("world", frame, worldToFrame)) {
        // 逆向变换，从frame到world
        Transform frameToWorld = worldToFrame.inverse();
        return frameToWorld.translation;
    }
    
    // 如果找不到变换，返回原点
    return glm::vec3(0.0f, 0.0f, 0.0f);
}

void TFManager::getConnectionsForRendering(std::vector<std::pair<glm::vec3, glm::vec3>>& connections) const {
    connections.clear();
    
    // 遍历所有节点
    for (const auto& [name, node] : m_nodes) {
        const TransformNode* childNode = node.get();
        const TransformNode* parentNode = childNode->getParent();
        
        if (parentNode) {
            // 计算子节点和父节点在世界坐标系下的位置
            glm::vec3 childPos = getFramePosition(childNode->getName());
            glm::vec3 parentPos = getFramePosition(parentNode->getName());
            
            // 添加连接线
            connections.emplace_back(parentPos, childPos);
        }
    }
}

TransformNode* TFManager::findOrCreateNode(const std::string& name) {
    auto it = m_nodes.find(name);
    if (it != m_nodes.end()) {
        return it->second.get();
    } else {
        // 创建新节点
        auto [newIt, inserted] = m_nodes.emplace(name, std::make_unique<TransformNode>(name));
        return newIt->second.get();
    }
}

TransformNode* TFManager::findNode(const std::string& name) const {
    auto it = m_nodes.find(name);
    return (it != m_nodes.end()) ? it->second.get() : nullptr;
}

bool TFManager::findTransformPath(const TransformNode* target, const TransformNode* source,
                                 std::vector<std::pair<const TransformNode*, bool>>& path) const {
    // 如果源或目标节点有一个不存在，返回失败
    if (!target || !source) {
        return false;
    }
    
    // 特殊情况：源和目标是同一个节点
    if (target == source) {
        return true; // 路径为空，单位变换
    }
    
    // 使用广度优先搜索查找最短路径
    std::queue<const TransformNode*> queue;
    std::map<const TransformNode*, const TransformNode*> cameFrom;
    std::map<const TransformNode*, bool> isInverse; // 表示边的方向
    
    queue.push(source);
    cameFrom[source] = nullptr;
    
    while (!queue.empty()) {
        const TransformNode* current = queue.front();
        queue.pop();
        
        // 检查是否找到目标
        if (current == target) {
            // 重建路径
            path.clear();
            const TransformNode* pathNode = current;
            
            while (pathNode != source) {
                const TransformNode* fromNode = cameFrom[pathNode];
                bool inverse = isInverse[pathNode];
                
                if (inverse) {
                    // 反向边：fromNode是子节点，pathNode是父节点
                    path.emplace_back(fromNode, true);
                } else {
                    // 正向边：fromNode是父节点，pathNode是子节点
                    path.emplace_back(pathNode, false);
                }
                
                pathNode = fromNode;
            }
            
            // 反转路径，使其从源到目标
            std::reverse(path.begin(), path.end());
            return true;
        }
        
        // 向上探索：检查父节点
        if (current->getParent() && cameFrom.find(current->getParent()) == cameFrom.end()) {
            queue.push(current->getParent());
            cameFrom[current->getParent()] = current;
            isInverse[current->getParent()] = true; // 反向：从子节点到父节点
        }
        
        // 向下探索：检查子节点
        for (const TransformNode* child : current->getChildren()) {
            if (cameFrom.find(child) == cameFrom.end()) {
                queue.push(child);
                cameFrom[child] = current;
                isInverse[child] = false; // 正向：从父节点到子节点
            }
        }
    }
    
    // 如果遍历完整个图仍未找到目标，返回失败
    return false;
}

} // namespace mviz 
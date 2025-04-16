#pragma once

#include <memory>
#include <map>
#include <string>
#include <vector>
#include <glm/glm.hpp>
#include "core/TFManager.h"

namespace mviz {

// 前向声明
class Renderer;
class Camera;

// 可视化对象基类
class VisualObject {
public:
    using SharedPtr = std::shared_ptr<VisualObject>;
    
    VisualObject(const std::string& name, const std::string& frame_id);
    virtual ~VisualObject() = default;
    
    // 获取和设置属性
    const std::string& getName() const { return m_name; }
    const std::string& getFrameId() const { return m_frame_id; }
    bool isVisible() const { return m_visible; }
    void setVisible(bool visible) { m_visible = visible; }
    
    // 更新对象的变换和状态
    virtual void update(TFManager& tf_manager, const std::string& reference_frame);
    
    // 绘制对象
    virtual void draw(Renderer& renderer, const glm::mat4& view_projection_matrix) = 0;
    
protected:
    std::string m_name;        // 对象名称
    std::string m_frame_id;    // 对象所在的坐标系
    bool m_visible;            // 是否可见
    glm::mat4 m_model_matrix;  // 模型矩阵
};

// 坐标轴可视化对象
class AxesVisual : public VisualObject {
public:
    AxesVisual(const std::string& name, const std::string& frame_id, float size = 1.0f);
    
    void update(TFManager& tf_manager, const std::string& reference_frame) override;
    void draw(Renderer& renderer, const glm::mat4& view_projection_matrix) override;
    
private:
    float m_size;  // 坐标轴大小
};

// 场景管理器类
class SceneManager {
public:
    SceneManager();
    ~SceneManager();
    
    // 初始化
    bool initialize();
    
    // 设置引用
    void setRenderer(std::shared_ptr<Renderer> renderer);
    void setCamera(std::shared_ptr<Camera> camera);
    
    // 添加和移除可视化对象
    void addVisualObject(const VisualObject::SharedPtr& object);
    void removeVisualObject(const std::string& name);
    VisualObject::SharedPtr getVisualObject(const std::string& name);
    
    // 创建示例TF数据
    void createDemoTFs();
    
    // 设置参考坐标系
    void setReferenceFrame(const std::string& frame);
    const std::string& getReferenceFrame() const { return m_reference_frame; }
    
    // 获取可用帧名称列表
    std::vector<std::string> getAvailableFrames() const;
    
    // 获取可视化对象列表
    const std::map<std::string, VisualObject::SharedPtr>& getVisualObjects() const { return m_visual_objects; }
    
    // 更新场景状态
    void update();
    
    // 渲染场景
    void render();
    
    // 获取TF管理器
    TFManager& getTFManager() { return m_tf_manager; }
    
private:
    // 对象映射表（按名称索引）
    std::map<std::string, VisualObject::SharedPtr> m_visual_objects;
    
    // TF管理器
    TFManager m_tf_manager;
    
    // 当前参考坐标系
    std::string m_reference_frame;
    
    // 渲染器和相机引用
    std::shared_ptr<Renderer> m_renderer;
    std::shared_ptr<Camera> m_camera;
    
    // 世界坐标轴
    VisualObject::SharedPtr m_world_axes;
};

} // namespace mviz 
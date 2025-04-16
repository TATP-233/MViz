#pragma once

#include "core/SceneManager.h"
#include "data/DataTypes.h"
#include <glad/glad.h>
#include <memory>

namespace mviz {

/**
 * 点云可视化对象类
 */
class PointCloudVisual : public VisualObject {
public:
    /**
     * 构造函数
     * @param name 对象名称
     * @param frame_id 坐标系ID
     */
    PointCloudVisual(const std::string& name, const std::string& frame_id);
    
    /**
     * 析构函数
     */
    ~PointCloudVisual() override;
    
    /**
     * 更新点云数据
     * @param pointCloud 点云数据
     */
    void setPointCloud(const PointCloudData& pointCloud);
    
    /**
     * 获取点云数据
     * @return 点云数据的常引用
     */
    const PointCloudData& getPointCloud() const { return m_pointCloudData; }
    
    /**
     * 设置点的大小
     * @param size 点的大小
     */
    void setPointSize(float size);
    
    /**
     * 获取点的大小
     * @return 点的大小
     */
    float getPointSize() const { return m_pointCloudData.pointSize; }
    
    /**
     * 更新对象变换矩阵
     * @param tf_manager TF管理器
     * @param reference_frame 参考坐标系
     */
    void update(TFManager& tf_manager, const std::string& reference_frame) override;
    
    /**
     * 绘制点云
     * @param renderer 渲染器
     * @param view_projection_matrix 视图投影矩阵
     */
    void draw(Renderer& renderer, const glm::mat4& view_projection_matrix) override;
    
private:
    // 点云数据
    PointCloudData m_pointCloudData;
    
    // OpenGL缓冲对象
    GLuint m_vao; // 顶点数组对象
    GLuint m_vbo; // 顶点缓冲对象
    
    // 是否需要更新缓冲区
    bool m_needBufferUpdate;
    
    // 点的数量
    size_t m_pointCount;
    
    // 初始化OpenGL资源
    void initializeGLResources();
    
    // 更新OpenGL缓冲区
    void updateBuffers();
    
    // 清理OpenGL资源
    void cleanupGLResources();
};

} // namespace mviz 
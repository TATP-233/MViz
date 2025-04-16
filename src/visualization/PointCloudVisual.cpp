#include "visualization/PointCloudVisual.h"
#include "rendering/Renderer.h"
#include <glm/gtc/type_ptr.hpp>
#include <iostream>

namespace mviz {

PointCloudVisual::PointCloudVisual(const std::string& name, const std::string& frame_id)
    : VisualObject(name, frame_id)
    , m_vao(0)
    , m_vbo(0)
    , m_needBufferUpdate(true)
    , m_pointCount(0)
{
    // 初始化OpenGL资源
    initializeGLResources();
}

PointCloudVisual::~PointCloudVisual() {
    // 清理OpenGL资源
    cleanupGLResources();
}

void PointCloudVisual::setPointCloud(const PointCloudData& pointCloud) {
    // 更新点云数据
    m_pointCloudData = pointCloud;
    m_needBufferUpdate = true;
}

void PointCloudVisual::setPointSize(float size) {
    // 更新点的大小
    if (size > 0) {
        m_pointCloudData.pointSize = size;
    }
}

void PointCloudVisual::update(TFManager& tf_manager, const std::string& reference_frame) {
    // 调用基类的update方法更新模型矩阵
    VisualObject::update(tf_manager, reference_frame);
    
    // 如果需要，更新缓冲区
    if (m_needBufferUpdate) {
        updateBuffers();
        m_needBufferUpdate = false;
    }
}

void PointCloudVisual::draw(Renderer& renderer, const glm::mat4& view_projection_matrix) {
    // 如果不可见或者没有点，则不绘制
    if (!m_visible || m_pointCount == 0) {
        return;
    }
    
    // 记住当前着色器类型，以便稍后恢复
    auto currentShader = renderer.getActiveShader();
    
    // 切换到点云着色器
    renderer.useShader(Renderer::ShaderType::POINT_CLOUD);
    
    // 获取当前渲染器的着色器
    auto shader = renderer.getActiveShader();
    if (!shader) {
        std::cerr << "Error: No active shader for point cloud rendering" << std::endl;
        return;
    }
    
    // 使用着色器
    shader->use();
    
    // 设置着色器统一变量
    shader->setMat4("model", m_model_matrix);
    shader->setMat4("view_projection", view_projection_matrix);
    shader->setFloat("point_size", m_pointCloudData.pointSize);
    
    // 绑定VAO并绘制点
    glBindVertexArray(m_vao);
    glDrawArrays(GL_POINTS, 0, m_pointCount);
    glBindVertexArray(0);
    
    // 恢复到基本着色器
    renderer.useShader(Renderer::ShaderType::BASIC);
}

void PointCloudVisual::initializeGLResources() {
    // 创建VAO和VBO
    glGenVertexArrays(1, &m_vao);
    glGenBuffers(1, &m_vbo);
    
    // 绑定VAO
    glBindVertexArray(m_vao);
    
    // 绑定VBO
    glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
    
    // 设置顶点属性
    // 顶点位置: vec3
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    
    // 顶点颜色: vec3
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    
    // 解绑VAO和VBO
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

void PointCloudVisual::updateBuffers() {
    // 如果没有点，则不更新
    if (m_pointCloudData.empty()) {
        m_pointCount = 0;
        return;
    }
    
    // 确保颜色数组和点数组大小一致
    if (m_pointCloudData.colors.size() < m_pointCloudData.points.size()) {
        // 如果颜色不足，使用默认颜色（白色）填充
        m_pointCloudData.colors.resize(m_pointCloudData.points.size(), glm::vec3(1.0f, 1.0f, 1.0f));
    }
    
    // 创建顶点数据数组（位置+颜色）
    std::vector<float> vertices;
    vertices.reserve(m_pointCloudData.points.size() * 6); // 每个点6个浮点数：xyz位置和rgb颜色
    
    for (size_t i = 0; i < m_pointCloudData.points.size(); ++i) {
        // 添加位置
        vertices.push_back(m_pointCloudData.points[i].x);
        vertices.push_back(m_pointCloudData.points[i].y);
        vertices.push_back(m_pointCloudData.points[i].z);
        
        // 添加颜色
        vertices.push_back(m_pointCloudData.colors[i].r);
        vertices.push_back(m_pointCloudData.colors[i].g);
        vertices.push_back(m_pointCloudData.colors[i].b);
    }
    
    // 更新点数
    m_pointCount = m_pointCloudData.points.size();
    
    // 绑定VAO和VBO
    glBindVertexArray(m_vao);
    glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
    
    // 更新VBO数据
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);
    
    // 解绑
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

void PointCloudVisual::cleanupGLResources() {
    // 删除VBO和VAO
    if (m_vbo != 0) {
        glDeleteBuffers(1, &m_vbo);
        m_vbo = 0;
    }
    
    if (m_vao != 0) {
        glDeleteVertexArrays(1, &m_vao);
        m_vao = 0;
    }
    
    m_pointCount = 0;
}

} // namespace mviz 
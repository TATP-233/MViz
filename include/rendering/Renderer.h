#pragma once

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <memory>
#include <string>
#include <vector>
#include "core/Camera.h"
#include "core/TFManager.h"
#include "rendering/Shader.h"

namespace mviz {

class Renderer {
public:
    Renderer();
    ~Renderer();

    // 初始化渲染器
    bool initialize();
    
    // 设置着色器和相机
    void setShader(const std::shared_ptr<Shader>& shader);
    void setCamera(const Camera* camera);
    void setTFManager(const TFManager* tfManager);
    
    // 创建和绘制基础场景元素
    void createCoordinateAxes(float size = 1.0f);
    void drawCoordinateAxes();
    
    void createGroundGrid(float size = 10.0f, float step = 1.0f);
    void drawGroundGrid();
    
    // 创建和绘制TF树
    void createTFVisualization();
    void drawTFVisualization();
    
    // 清除帧缓冲
    void clear();
    
private:
    // 着色器和相机
    std::shared_ptr<Shader> m_shader;
    const Camera* m_camera;
    const TFManager* m_tfManager;
    
    // 坐标轴VAO, VBO
    unsigned int m_axesVAO, m_axesVBO;
    int m_axesVertexCount;
    
    // 地面网格VAO, VBO
    unsigned int m_gridVAO, m_gridVBO;
    int m_gridVertexCount;
    
    // TF连接线VAO, VBO
    unsigned int m_tfLinesVAO, m_tfLinesVBO;
    int m_tfLinesVertexCount;
    
    // TF坐标系VAO, VBO
    struct TFFrameVisual {
        unsigned int vao;
        unsigned int vbo;
        int vertexCount;
        std::string name;
        glm::vec3 position;
    };
    std::vector<TFFrameVisual> m_tfFrames;
    
    // 初始化OpenGL状态
    void setupOpenGLState();
    
    // 更新TF可视化数据
    void updateTFVisualData();
};

} // namespace mviz 
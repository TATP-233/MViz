#pragma once

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <memory>
#include <string>
#include <vector>
#include <unordered_map>
#include <glm/gtc/matrix_transform.hpp>
#include "core/Camera.h"
#include "core/TFManager.h"
#include "core/SceneManager.h"
#include "rendering/Shader.h"
#include "rendering/TextRenderer.h"

namespace mviz {

class Renderer {
public:
    Renderer();
    ~Renderer();

    // 初始化渲染器
    bool initialize();
    
    // 设置着色器和相机
    void setShader(const std::shared_ptr<Shader>& shader);
    void setCamera(const Camera* camera) { m_camera = camera; }
    void setTFManager(const TFManager* tfManager) { m_tfManager = tfManager; }
    void setSceneManager(const SceneManager* sceneManager) { m_sceneManager = sceneManager; }
    
    // 创建和绘制基础场景元素
    void createCoordinateAxes(float size = 1.0f);
    void drawCoordinateAxes();
    
    void createGroundGrid(float size = 10.0f, float step = 1.0f);
    void drawGroundGrid(const std::string& referenceFrame = "world");
    
    // 创建和绘制TF树
    void createTFVisualization();
    void drawTFVisualization();
    
    // 设置坐标系名称可见性和文本大小
    void setFrameLabelsVisible(bool visible) { m_showFrameLabels = visible; }
    bool isFrameLabelsVisible() const { return m_showFrameLabels; }
    
    void setFrameLabelsSize(float size) { m_frameLabelSize = size; }
    float getFrameLabelsSize() const { return m_frameLabelSize; }
    
    // 设置坐标轴线粗细
    void setAxisThickness(float thickness) { m_axisThickness = thickness; }
    float getAxisThickness() const { return m_axisThickness; }
    
    // 清除帧缓冲
    void clear();
    
    void init();
    void drawAxes();
    void setBackgroundColor(float r, float g, float b);

private:
    // 着色器和相机
    std::shared_ptr<Shader> m_shader;
    const Camera* m_camera;
    const TFManager* m_tfManager;
    const SceneManager* m_sceneManager;
    
    // 文本渲染器
    std::shared_ptr<TextRenderer> m_textRenderer;
    
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
    
    // 坐标系标签设置
    bool m_showFrameLabels;
    float m_frameLabelSize;
    
    // 坐标轴粗细
    float m_axisThickness;
    
    // 初始化OpenGL状态
    void setupOpenGLState();
    
    // 更新TF可视化数据
    void updateTFVisualData();
    
    // 绘制3D文本标签
    void renderText(const std::string& text, const glm::vec3& position, const glm::vec3& color);

    // 帧数据结构
    struct TFFrame {
        std::string name;
        GLuint vao;
        GLuint vbo;
        GLuint vertexCount;
        glm::vec3 position;
    };
};

} // namespace mviz 
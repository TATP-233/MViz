#include "rendering/Renderer.h"
#include <vector>
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>

namespace mviz {

Renderer::Renderer()
    : m_camera(nullptr)
    , m_tfManager(nullptr)
    , m_sceneManager(nullptr)
    , m_axesVAO(0)
    , m_axesVBO(0)
    , m_axesVertexCount(0)
    , m_gridVAO(0)
    , m_gridVBO(0)
    , m_gridVertexCount(0)
    , m_tfLinesVAO(0)
    , m_tfLinesVBO(0)
    , m_tfLinesVertexCount(0)
    , m_showFrameLabels(true)
    , m_frameLabelSize(1.0f)
    , m_axisThickness(1.0f)
{
}

Renderer::~Renderer() {
    // 清理VAO和VBO
    if (m_axesVAO) {
        glDeleteVertexArrays(1, &m_axesVAO);
        glDeleteBuffers(1, &m_axesVBO);
    }
    
    if (m_gridVAO) {
        glDeleteVertexArrays(1, &m_gridVAO);
        glDeleteBuffers(1, &m_gridVBO);
    }
    
    if (m_tfLinesVAO) {
        glDeleteVertexArrays(1, &m_tfLinesVAO);
        glDeleteBuffers(1, &m_tfLinesVBO);
    }
    
    // 清理TF坐标系可视化
    for (auto& frame : m_tfFrames) {
        glDeleteVertexArrays(1, &frame.vao);
        glDeleteBuffers(1, &frame.vbo);
    }
    m_tfFrames.clear();
}

bool Renderer::initialize() {
    // 设置OpenGL状态
    setupOpenGLState();
    
    // 创建基础场景元素
    createCoordinateAxes();
    createGroundGrid();
    createTFVisualization();
    
    // 创建文本渲染器
    m_textRenderer = std::make_shared<TextRenderer>();
    
    // 尝试不同的字体加载路径
    std::vector<std::string> fontPaths = {
        "fonts/Helvetica.ttc",               // 相对路径
        "./fonts/Helvetica.ttc",             // 当前目录相对路径
        "../fonts/Helvetica.ttc",            // 上级目录
        "../../fonts/Helvetica.ttc",         // 更上级目录
    };
    
    bool fontLoaded = false;
    for (const auto& path : fontPaths) {
        std::cout << "尝试加载字体: " << path << std::endl;
        if (m_textRenderer->initialize(path, 32)) {
            std::cout << "成功加载字体: " << path << std::endl;
            fontLoaded = true;
            break;
        }
    }
    
    if (!fontLoaded) {
        std::cerr << "警告: 无法加载任何字体文件，将使用点标记代替文本" << std::endl;
    }
    
    return true;
}

void Renderer::setShader(const std::shared_ptr<Shader>& shader) {
    m_shader = shader;
}

void Renderer::createCoordinateAxes(float size) {
    // 创建三个轴线的顶点数据
    // X轴: 红色, Y轴: 绿色, Z轴: 蓝色
    std::vector<float> axesVertices = {
        // 位置              // 颜色
        0.0f, 0.0f, 0.0f,   1.0f, 0.0f, 0.0f, // X轴起点
        size, 0.0f, 0.0f,   1.0f, 0.0f, 0.0f, // X轴终点
        
        0.0f, 0.0f, 0.0f,   0.0f, 1.0f, 0.0f, // Y轴起点
        0.0f, size, 0.0f,   0.0f, 1.0f, 0.0f, // Y轴终点
        
        0.0f, 0.0f, 0.0f,   0.0f, 0.0f, 1.0f, // Z轴起点
        0.0f, 0.0f, size,   0.0f, 0.0f, 1.0f  // Z轴终点
    };
    
    m_axesVertexCount = axesVertices.size() / 6; // 每个顶点有6个浮点数
    
    // 创建VAO和VBO
    glGenVertexArrays(1, &m_axesVAO);
    glGenBuffers(1, &m_axesVBO);
    
    // 绑定VAO
    glBindVertexArray(m_axesVAO);
    
    // 绑定VBO并填充数据
    glBindBuffer(GL_ARRAY_BUFFER, m_axesVBO);
    glBufferData(GL_ARRAY_BUFFER, axesVertices.size() * sizeof(float), axesVertices.data(), GL_STATIC_DRAW);
    
    // 设置顶点属性指针
    // 位置属性
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    
    // 颜色属性
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    
    // 解绑VBO和VAO
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

void Renderer::drawCoordinateAxes() {
    if (!m_shader || !m_camera || !m_axesVAO) {
        return;
    }
    
    // 使用着色器
    m_shader->use();
    
    // 设置模型、视图和投影矩阵
    glm::mat4 model = glm::mat4(1.0f);
    m_shader->setMat4("model", model);
    m_shader->setMat4("view", m_camera->getViewMatrix());
    m_shader->setMat4("projection", m_camera->getProjectionMatrix());
    
    // 设置线宽
    glLineWidth(m_axisThickness);
    
    // 绘制坐标轴
    glBindVertexArray(m_axesVAO);
    glDrawArrays(GL_LINES, 0, m_axesVertexCount);
    glBindVertexArray(0);
    
    // 恢复默认线宽
    glLineWidth(1.0f);
}

void Renderer::createGroundGrid(float size, float step) {
    // 创建地面网格的顶点数据
    std::vector<float> gridVertices;
    
    // 计算网格线数量
    int lineCount = static_cast<int>(size / step) * 2 + 1;
    float halfSize = size / 2.0f;
    
    // 创建平行于X轴的线
    for (int i = 0; i < lineCount; ++i) {
        float z = -halfSize + i * step;
        
        // 浅灰色，Z轴处的线为深灰色
        float color = (std::abs(z) < 0.001f) ? 0.5f : 0.3f;
        
        // 每条线的两个端点
        gridVertices.push_back(-halfSize); // x
        gridVertices.push_back(0.0f);      // y
        gridVertices.push_back(z);         // z
        gridVertices.push_back(color);     // r
        gridVertices.push_back(color);     // g
        gridVertices.push_back(color);     // b
        
        gridVertices.push_back(halfSize);  // x
        gridVertices.push_back(0.0f);      // y
        gridVertices.push_back(z);         // z
        gridVertices.push_back(color);     // r
        gridVertices.push_back(color);     // g
        gridVertices.push_back(color);     // b
    }
    
    // 创建平行于Z轴的线
    for (int i = 0; i < lineCount; ++i) {
        float x = -halfSize + i * step;
        
        // 浅灰色，X轴处的线为深灰色
        float color = (std::abs(x) < 0.001f) ? 0.5f : 0.3f;
        
        // 每条线的两个端点
        gridVertices.push_back(x);         // x
        gridVertices.push_back(0.0f);      // y
        gridVertices.push_back(-halfSize); // z
        gridVertices.push_back(color);     // r
        gridVertices.push_back(color);     // g
        gridVertices.push_back(color);     // b
        
        gridVertices.push_back(x);         // x
        gridVertices.push_back(0.0f);      // y
        gridVertices.push_back(halfSize);  // z
        gridVertices.push_back(color);     // r
        gridVertices.push_back(color);     // g
        gridVertices.push_back(color);     // b
    }
    
    m_gridVertexCount = gridVertices.size() / 6; // 每个顶点有6个浮点数
    
    // 创建VAO和VBO
    glGenVertexArrays(1, &m_gridVAO);
    glGenBuffers(1, &m_gridVBO);
    
    // 绑定VAO
    glBindVertexArray(m_gridVAO);
    
    // 绑定VBO并填充数据
    glBindBuffer(GL_ARRAY_BUFFER, m_gridVBO);
    glBufferData(GL_ARRAY_BUFFER, gridVertices.size() * sizeof(float), gridVertices.data(), GL_STATIC_DRAW);
    
    // 设置顶点属性指针
    // 位置属性
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    
    // 颜色属性
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    
    // 解绑VBO和VAO
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

void Renderer::drawGroundGrid(const std::string& referenceFrame) {
    if (!m_shader || !m_camera) {
        return;
    }
    
    // 使用着色器
    m_shader->use();
    
    // 如果指定了特定的参考坐标系，获取从世界坐标系到该坐标系的变换
    glm::mat4 model = glm::mat4(1.0f);
    
    if (m_tfManager && referenceFrame != "world") {
        Transform worldToRef;
        if (m_tfManager->lookupTransform("world", referenceFrame, worldToRef)) {
            // 创建变换矩阵 - 先旋转后平移
            glm::mat4 rotMat = glm::mat4_cast(worldToRef.rotation);
            model = glm::translate(glm::mat4(1.0f), worldToRef.translation) * rotMat;
        }
    }
    
    // 设置模型、视图和投影矩阵
    m_shader->setMat4("model", model);
    m_shader->setMat4("view", m_camera->getViewMatrix());
    m_shader->setMat4("projection", m_camera->getProjectionMatrix());
    
    // 临时禁用深度写入，确保网格不会遮挡其他对象
    glDepthMask(GL_FALSE);
    
    // 绑定网格VAO并绘制
    glBindVertexArray(m_gridVAO);
    glDrawArrays(GL_LINES, 0, m_gridVertexCount);
    glBindVertexArray(0);
    
    // 恢复深度写入
    glDepthMask(GL_TRUE);
}

void Renderer::createTFVisualization() {
    // 初始化TF连接线的VAO和VBO
    glGenVertexArrays(1, &m_tfLinesVAO);
    glGenBuffers(1, &m_tfLinesVBO);
    
    // 绑定VAO
    glBindVertexArray(m_tfLinesVAO);
    
    // 绑定VBO（先不填充数据，在updateTFVisualData中更新）
    glBindBuffer(GL_ARRAY_BUFFER, m_tfLinesVBO);
    
    // 设置顶点属性指针
    // 位置属性
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    
    // 颜色属性
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    
    // 解绑VBO和VAO
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

void Renderer::updateTFVisualData() {
    if (!m_tfManager) {
        return;
    }
    
    // 获取TF坐标系之间的连接
    std::vector<std::pair<glm::vec3, glm::vec3>> connections;
    m_tfManager->getConnectionsForRendering(connections);
    
    // 准备TF连接线的顶点数据
    std::vector<float> tfLinesVertices;
    tfLinesVertices.reserve(connections.size() * 12); // 每条线有2个点，每个点有6个float
    
    for (const auto& [parentPos, childPos] : connections) {
        // 父节点（黄色）
        tfLinesVertices.push_back(parentPos.x);
        tfLinesVertices.push_back(parentPos.y);
        tfLinesVertices.push_back(parentPos.z);
        tfLinesVertices.push_back(1.0f); // 黄色
        tfLinesVertices.push_back(1.0f);
        tfLinesVertices.push_back(0.0f);
        
        // 子节点（黄色）
        tfLinesVertices.push_back(childPos.x);
        tfLinesVertices.push_back(childPos.y);
        tfLinesVertices.push_back(childPos.z);
        tfLinesVertices.push_back(1.0f); // 黄色
        tfLinesVertices.push_back(1.0f);
        tfLinesVertices.push_back(0.0f);
    }
    
    m_tfLinesVertexCount = tfLinesVertices.size() / 6;
    
    // 更新VBO数据
    glBindBuffer(GL_ARRAY_BUFFER, m_tfLinesVBO);
    glBufferData(GL_ARRAY_BUFFER, tfLinesVertices.size() * sizeof(float), tfLinesVertices.data(), GL_DYNAMIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    
    // 更新TF坐标系可视化
    // 清除旧的TF坐标系
    for (auto& frame : m_tfFrames) {
        glDeleteVertexArrays(1, &frame.vao);
        glDeleteBuffers(1, &frame.vbo);
    }
    m_tfFrames.clear();
    
    // 获取所有坐标系名称
    std::vector<std::string> frameNames = m_tfManager->getAllFrameNames();
    
    // 为每个坐标系创建一个小的坐标轴
    float axisSize = 0.2f; // 小坐标轴的大小
    
    for (const auto& name : frameNames) {
        TFFrameVisual frameVisual;
        frameVisual.name = name;
        frameVisual.position = m_tfManager->getFramePosition(name);
        
        // 使用相同的代码创建坐标轴，但应用偏移
        std::vector<float> axesVertices = {
            // 位置                                                       // 颜色
            0.0f, 0.0f, 0.0f,                                           1.0f, 0.0f, 0.0f, // X轴起点
            axisSize, 0.0f, 0.0f,                                       1.0f, 0.0f, 0.0f, // X轴终点
            
            0.0f, 0.0f, 0.0f,                                           0.0f, 1.0f, 0.0f, // Y轴起点
            0.0f, axisSize, 0.0f,                                       0.0f, 1.0f, 0.0f, // Y轴终点
            
            0.0f, 0.0f, 0.0f,                                           0.0f, 0.0f, 1.0f, // Z轴起点
            0.0f, 0.0f, axisSize,                                       0.0f, 0.0f, 1.0f  // Z轴终点
        };
        
        frameVisual.vertexCount = axesVertices.size() / 6;
        
        // 创建VAO和VBO
        glGenVertexArrays(1, &frameVisual.vao);
        glGenBuffers(1, &frameVisual.vbo);
        
        // 绑定VAO
        glBindVertexArray(frameVisual.vao);
        
        // 绑定VBO并填充数据
        glBindBuffer(GL_ARRAY_BUFFER, frameVisual.vbo);
        glBufferData(GL_ARRAY_BUFFER, axesVertices.size() * sizeof(float), axesVertices.data(), GL_STATIC_DRAW);
        
        // 设置顶点属性指针
        // 位置属性
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);
        
        // 颜色属性
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
        glEnableVertexAttribArray(1);
        
        // 解绑VBO和VAO
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);
        
        // 添加到TF坐标系列表
        m_tfFrames.push_back(frameVisual);
    }
}

void Renderer::drawTFVisualization() {
    if (!m_shader || !m_camera || !m_tfManager) {
        return;
    }
    
    // 更新TF可视化数据
    updateTFVisualData();
    
    // 使用着色器
    m_shader->use();
    
    // 设置视图和投影矩阵（模型矩阵会为每个元素单独设置）
    m_shader->setMat4("view", m_camera->getViewMatrix());
    m_shader->setMat4("projection", m_camera->getProjectionMatrix());
    
    // 绘制TF连接线
    if (m_tfLinesVertexCount > 0) {
        glm::mat4 model = glm::mat4(1.0f);
        m_shader->setMat4("model", model);
        
        // 连接线使用细一点的线
        glLineWidth(m_axisThickness * 0.7f);
        
        glBindVertexArray(m_tfLinesVAO);
        glDrawArrays(GL_LINES, 0, m_tfLinesVertexCount);
        glBindVertexArray(0);
    }
    
    // 绘制TF坐标系
    for (const auto& frame : m_tfFrames) {
        // 检查坐标系是否应该可见，或者是否应该显示标签
        // 如果是world坐标系，或者用户启用了显示标签，我们就显示坐标系
        bool isVisible = !m_sceneManager || m_sceneManager->isFrameVisible(frame.name);
        bool shouldDrawFrame = isVisible || (m_showFrameLabels && frame.name != "world");
        
        if (!shouldDrawFrame) {
            continue;  // 跳过不可见的坐标系
        }
        
        // 设置模型矩阵，偏移到坐标系位置
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, frame.position);
        
        // 查找从世界坐标系到当前坐标系的变换（如果可用，则应用旋转）
        Transform worldToFrame;
        if (m_tfManager->lookupTransform("world", frame.name, worldToFrame)) {
            // 应用旋转（四元数转矩阵）
            glm::mat4 rotMat = glm::mat4_cast(worldToFrame.rotation);
            // 只提取旋转部分，不要应用平移（因为我们已经使用frame.position平移了）
            model[0] = rotMat[0];
            model[1] = rotMat[1];
            model[2] = rotMat[2];
        }
        
        m_shader->setMat4("model", model);
        
        // 如果坐标系应该可见，则绘制坐标轴
        if (isVisible) {
            // 设置坐标轴线宽
            glLineWidth(m_axisThickness);
            
            glBindVertexArray(frame.vao);
            glDrawArrays(GL_LINES, 0, frame.vertexCount);
            glBindVertexArray(0);
        }
        
        // 如果启用了标签显示，绘制坐标系名称（世界坐标系除外，它已经通过网格显示）
        if (m_showFrameLabels && frame.name != "world") {
            // 计算标签位置，略微偏移以便可见
            glm::vec3 labelPos = frame.position + glm::vec3(0.0f, 0.2f * m_frameLabelSize, 0.0f);
            
            // 绘制文本
            renderText(frame.name, labelPos, glm::vec3(1.0f, 1.0f, 1.0f));
        }
    }
    
    // 恢复默认线宽
    glLineWidth(1.0f);
}

void Renderer::clear() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void Renderer::setupOpenGLState() {
    // 启用深度测试
    glEnable(GL_DEPTH_TEST);
    
    // 启用背面剔除
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    
    // 启用混合
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    // 设置背景颜色
    glClearColor(0.1f, 0.1f, 0.2f, 1.0f);
}

void Renderer::renderText(const std::string& text, const glm::vec3& position, const glm::vec3& color) {
    if (m_textRenderer && m_camera) {
        // 使用文本渲染器绘制3D文本
        m_textRenderer->renderText3D(text, position, m_frameLabelSize * 0.005f, color, m_camera->getViewMatrix(), m_camera->getProjectionMatrix());
    } else if (m_shader && m_camera) {
        // 回退方法：使用点标记
        // 使用已有的着色器
        m_shader->use();
        
        // 创建一个小方块在文本位置
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, position);
        model = glm::scale(model, glm::vec3(0.05f * m_frameLabelSize));
        
        m_shader->setMat4("model", model);
        m_shader->setMat4("view", m_camera->getViewMatrix());
        m_shader->setMat4("projection", m_camera->getProjectionMatrix());
        
        // 使用坐标轴VAO绘制一个点
        glPointSize(5.0f * m_frameLabelSize);
        glBindVertexArray(m_axesVAO);
        glDrawArrays(GL_POINTS, 0, 1);
        glBindVertexArray(0);
        glPointSize(1.0f);
    }
}

} // namespace mviz 
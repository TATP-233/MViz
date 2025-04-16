#pragma once

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <map>
#include <string>
#include <memory>
#include "rendering/Shader.h"

namespace mviz {

// 字符结构体
struct Character {
    unsigned int TextureID;  // 字形纹理ID
    glm::ivec2   Size;       // 字形大小
    glm::ivec2   Bearing;    // 基线到字形左侧/顶部的偏移
    unsigned int Advance;    // 水平偏移量
};

class TextRenderer {
public:
    // 构造函数
    TextRenderer();
    ~TextRenderer();

    // 初始化字体
    bool initialize(const std::string& fontPath, unsigned int fontSize = 24);
    
    // 渲染文本
    void renderText(const std::string& text, float x, float y, float scale, const glm::vec3& color);
    
    // 渲染3D空间中的文本
    void renderText3D(const std::string& text, const glm::vec3& position, float scale, const glm::vec3& color, 
                    const glm::mat4& view, const glm::mat4& projection);

private:
    // 着色器
    std::shared_ptr<Shader> m_shader;
    
    // 字符映射表
    std::map<char, Character> m_characters;
    
    // VAO和VBO
    unsigned int m_VAO, m_VBO;
    
    // 初始化OpenGL资源
    void setupOpenGLResources();
    
    // 创建文本着色器
    void createTextShader();
};

} // namespace mviz 
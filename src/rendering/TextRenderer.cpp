#include "rendering/TextRenderer.h"
#include <iostream>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <ft2build.h>
#include FT_FREETYPE_H

namespace mviz {

TextRenderer::TextRenderer()
    : m_VAO(0)
    , m_VBO(0)
{
}

TextRenderer::~TextRenderer() {
    // 清理字符纹理
    for (auto& pair : m_characters) {
        glDeleteTextures(1, &pair.second.TextureID);
    }

    // 清理VAO和VBO
    if (m_VAO) {
        glDeleteVertexArrays(1, &m_VAO);
    }
    if (m_VBO) {
        glDeleteBuffers(1, &m_VBO);
    }
}

bool TextRenderer::initialize(const std::string& fontPath, unsigned int fontSize) {
    // 创建着色器
    createTextShader();
    
    // 初始化OpenGL资源
    setupOpenGLResources();
    
    // 加载字体
    FT_Library ft;
    if (FT_Init_FreeType(&ft)) {
        std::cerr << "ERROR::FREETYPE: Could not init FreeType Library" << std::endl;
        return false;
    }

    FT_Face face;
    if (FT_New_Face(ft, fontPath.c_str(), 0, &face)) {
        std::cerr << "ERROR::FREETYPE: Failed to load font at " << fontPath << std::endl;
        FT_Done_FreeType(ft);
        return false;
    }

    // 设置字体大小
    FT_Set_Pixel_Sizes(face, 0, fontSize);

    // 禁用字节对齐限制
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    // 加载ASCII字符集 (0-127)
    for (unsigned char c = 0; c < 128; c++) {
        // 加载字符的字形
        if (FT_Load_Char(face, c, FT_LOAD_RENDER)) {
            std::cerr << "ERROR::FREETYPE: Failed to load Glyph for character: " << c << std::endl;
            continue;
        }

        // 生成纹理
        unsigned int texture;
        glGenTextures(1, &texture);
        glBindTexture(GL_TEXTURE_2D, texture);
        glTexImage2D(
            GL_TEXTURE_2D,
            0,
            GL_RED,
            face->glyph->bitmap.width,
            face->glyph->bitmap.rows,
            0,
            GL_RED,
            GL_UNSIGNED_BYTE,
            face->glyph->bitmap.buffer
        );

        // 设置纹理参数
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        // 存储字符
        Character character = {
            texture,
            glm::ivec2(face->glyph->bitmap.width, face->glyph->bitmap.rows),
            glm::ivec2(face->glyph->bitmap_left, face->glyph->bitmap_top),
            static_cast<unsigned int>(face->glyph->advance.x)
        };
        m_characters.insert(std::pair<char, Character>(c, character));
    }

    // 清理FreeType资源
    FT_Done_Face(face);
    FT_Done_FreeType(ft);

    return true;
}

void TextRenderer::setupOpenGLResources() {
    // 创建VAO和VBO
    glGenVertexArrays(1, &m_VAO);
    glGenBuffers(1, &m_VBO);
    
    glBindVertexArray(m_VAO);
    glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
    
    // 动态VBO，每次渲染文本时都会更新
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 6 * 4, NULL, GL_DYNAMIC_DRAW);
    
    // 位置和纹理坐标
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), 0);
    
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

void TextRenderer::createTextShader() {
    // 创建文本着色器
    m_shader = std::make_shared<Shader>("shaders/text.vert", "shaders/text.frag");
}

void TextRenderer::renderText(const std::string& text, float x, float y, float scale, const glm::vec3& color) {
    // 使用着色器
    m_shader->use();
    
    // 配置着色器
    m_shader->setVec3("textColor", color);
    m_shader->setBool("is3D", false);
    
    // 创建正射投影矩阵 (屏幕空间)
    int width, height;
    // 获取视口尺寸
    GLint viewport[4];
    glGetIntegerv(GL_VIEWPORT, viewport);
    width = viewport[2];
    height = viewport[3];
    
    glm::mat4 projection = glm::ortho(0.0f, static_cast<float>(width), 0.0f, static_cast<float>(height));
    m_shader->setMat4("projection", projection);
    
    // 激活纹理单元
    glActiveTexture(GL_TEXTURE0);
    glBindVertexArray(m_VAO);
    
    // 遍历文本中的每个字符
    float xpos = x;
    for (char c : text) {
        Character ch = m_characters[c];
        
        float w = ch.Size.x * scale;
        float h = ch.Size.y * scale;
        
        // 计算字符的位置
        float xpos_offset = xpos + ch.Bearing.x * scale;
        float ypos_offset = y - (ch.Size.y - ch.Bearing.y) * scale;
        
        // 更新VBO
        float vertices[6][4] = {
            { xpos_offset,     ypos_offset + h,   0.0f, 0.0f },            
            { xpos_offset,     ypos_offset,       0.0f, 1.0f },
            { xpos_offset + w, ypos_offset,       1.0f, 1.0f },
            
            { xpos_offset,     ypos_offset + h,   0.0f, 0.0f },
            { xpos_offset + w, ypos_offset,       1.0f, 1.0f },
            { xpos_offset + w, ypos_offset + h,   1.0f, 0.0f }           
        };
        
        // 渲染字形纹理
        glBindTexture(GL_TEXTURE_2D, ch.TextureID);
        
        // 更新VBO内存
        glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        
        // 渲染四边形
        glDrawArrays(GL_TRIANGLES, 0, 6);
        
        // 更新位置到下一个字形
        xpos += (ch.Advance >> 6) * scale; // 位移是以1/64像素为单位的
    }
    
    // 重置状态
    glBindVertexArray(0);
    glBindTexture(GL_TEXTURE_2D, 0);
}

void TextRenderer::renderText3D(const std::string& text, const glm::vec3& position, float scale, const glm::vec3& color, 
                               const glm::mat4& view, const glm::mat4& projection) {
    // 使用着色器
    m_shader->use();
    
    // 配置着色器
    m_shader->setVec3("textColor", color);
    m_shader->setBool("is3D", true);
    m_shader->setMat4("view", view);
    m_shader->setMat4("projection", projection);
    
    // 设置模型矩阵
    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, position);
    
    // 计算面向相机的旋转矩阵（billboard效果）
    glm::vec3 cameraRight = glm::vec3(view[0][0], view[1][0], view[2][0]);
    glm::vec3 cameraUp = glm::vec3(view[0][1], view[1][1], view[2][1]);
    
    model[0] = glm::vec4(cameraRight, 0.0f);
    model[1] = glm::vec4(cameraUp, 0.0f);
    
    m_shader->setMat4("model", model);
    
    // 计算文本总宽度，以便居中
    float textWidth = 0.0f;
    for (char c : text) {
        Character ch = m_characters[c];
        textWidth += (ch.Advance >> 6) * scale;
    }
    
    // 激活纹理单元
    glActiveTexture(GL_TEXTURE0);
    glBindVertexArray(m_VAO);
    
    // 启用混合
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    // 暂时禁用深度测试，确保文本始终可见
    glDisable(GL_DEPTH_TEST);
    
    // 遍历文本中的每个字符
    float xpos = -textWidth / 2.0f; // 居中
    for (char c : text) {
        Character ch = m_characters[c];
        
        float w = ch.Size.x * scale;
        float h = ch.Size.y * scale;
        
        // 计算字符的位置
        float xpos_offset = xpos + ch.Bearing.x * scale;
        float ypos_offset = -(ch.Size.y - ch.Bearing.y) * scale;
        
        // 更新VBO
        float vertices[6][4] = {
            { xpos_offset,     ypos_offset + h,   0.0f, 0.0f },            
            { xpos_offset,     ypos_offset,       0.0f, 1.0f },
            { xpos_offset + w, ypos_offset,       1.0f, 1.0f },
            
            { xpos_offset,     ypos_offset + h,   0.0f, 0.0f },
            { xpos_offset + w, ypos_offset,       1.0f, 1.0f },
            { xpos_offset + w, ypos_offset + h,   1.0f, 0.0f }           
        };
        
        // 渲染字形纹理
        glBindTexture(GL_TEXTURE_2D, ch.TextureID);
        
        // 更新VBO内存
        glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        
        // 渲染四边形
        glDrawArrays(GL_TRIANGLES, 0, 6);
        
        // 更新位置到下一个字形
        xpos += (ch.Advance >> 6) * scale; // 位移是以1/64像素为单位的
    }
    
    // 恢复深度测试
    glEnable(GL_DEPTH_TEST);
    
    // 重置状态
    glBindVertexArray(0);
    glBindTexture(GL_TEXTURE_2D, 0);
}

} // namespace mviz 
#pragma once

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <string>
#include <fstream>
#include <sstream>
#include <iostream>

namespace mviz {

class Shader {
public:
    // 着色器程序ID
    unsigned int ID;

    // 构造函数，读取并构建着色器
    Shader(const std::string& vertexPath, const std::string& fragmentPath);
    ~Shader();

    // 激活着色器程序
    void use() const;

    // uniform工具函数
    void setBool(const std::string& name, bool value) const;
    void setInt(const std::string& name, int value) const;
    void setFloat(const std::string& name, float value) const;
    void setVec2(const std::string& name, const glm::vec2& value) const;
    void setVec3(const std::string& name, const glm::vec3& value) const;
    void setVec4(const std::string& name, const glm::vec4& value) const;
    void setMat2(const std::string& name, const glm::mat2& mat) const;
    void setMat3(const std::string& name, const glm::mat3& mat) const;
    void setMat4(const std::string& name, const glm::mat4& mat) const;

private:
    // 检查着色器编译/链接错误的工具函数
    void checkCompileErrors(unsigned int shader, const std::string& type);
};

} // namespace mviz 
#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace mviz {

class Camera {
public:
    // 相机类型枚举
    enum class Mode {
        ORBIT,  // 绕目标点旋转
        FPS     // 第一人称视角
    };

    // 构造函数
    Camera(const glm::vec3& position = glm::vec3(0.0f, 5.0f, 10.0f),
           const glm::vec3& target = glm::vec3(0.0f, 0.0f, 0.0f),
           const glm::vec3& up = glm::vec3(0.0f, 1.0f, 0.0f));

    // 获取视图和投影矩阵
    glm::mat4 getViewMatrix() const;
    glm::mat4 getProjectionMatrix() const;

    // 相机移动方法
    void setPosition(const glm::vec3& position);
    void setTarget(const glm::vec3& target);
    void setUpVector(const glm::vec3& up);
    
    // 设置投影参数
    void setPerspective(float fov, float aspectRatio, float nearPlane, float farPlane);
    
    // 鼠标交互方法
    void processMouseMove(float xoffset, float yoffset, bool constrainPitch = true);
    void processMouseScroll(float yoffset);
    void processMouseDrag(float xoffset, float yoffset, bool isRightButton);
    
    // 相机模式切换
    void setMode(Mode mode);
    Mode getMode() const;
    
    // 获取相机属性
    glm::vec3 getPosition() const { return m_position; }
    glm::vec3 getTarget() const { return m_target; }
    glm::vec3 getUpVector() const { return m_up; }
    glm::vec3 getFrontVector() const;
    glm::vec3 getRightVector() const;
    
    // 重置相机
    void reset();

private:
    // 相机属性
    glm::vec3 m_position;
    glm::vec3 m_target;
    glm::vec3 m_up;
    glm::vec3 m_front;
    glm::vec3 m_right;
    glm::vec3 m_worldUp;
    
    // 欧拉角
    float m_yaw;
    float m_pitch;
    
    // 相机选项
    float m_movementSpeed;
    float m_mouseSensitivity;
    float m_zoomSensitivity;
    float m_distance;
    
    // 投影参数
    float m_fov;
    float m_aspectRatio;
    float m_nearPlane;
    float m_farPlane;
    
    // 相机模式
    Mode m_mode;
    
    // 内部计算方法
    void updateCameraVectors();
    void calculateOrbitPosition();
};

} // namespace mviz 
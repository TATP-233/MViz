#include "core/Camera.h"
#include <algorithm>
#include <cmath>
#include <glm/gtc/quaternion.hpp>

namespace mviz {

// 默认相机参数
constexpr float DEFAULT_YAW = -90.0f;
constexpr float DEFAULT_PITCH = -30.0f;
constexpr float DEFAULT_SPEED = 2.5f;
constexpr float DEFAULT_SENSITIVITY = 0.1f;
constexpr float DEFAULT_ZOOM_SENSITIVITY = 0.1f;
constexpr float DEFAULT_FOV = 45.0f;
constexpr float DEFAULT_NEAR_PLANE = 0.1f;
constexpr float DEFAULT_FAR_PLANE = 1000.0f;
constexpr float MIN_FOV = 1.0f;
constexpr float MAX_FOV = 90.0f;
constexpr float MIN_PITCH = -89.0f;
constexpr float MAX_PITCH = 89.0f;
constexpr float MIN_DISTANCE = 0.1f;
constexpr float MAX_DISTANCE = 100.0f;

Camera::Camera(const glm::vec3& position, const glm::vec3& target, const glm::vec3& up)
    : m_position(position)
    , m_target(target)
    , m_worldUp(up)
    , m_yaw(DEFAULT_YAW)
    , m_pitch(DEFAULT_PITCH)
    , m_movementSpeed(DEFAULT_SPEED)
    , m_mouseSensitivity(DEFAULT_SENSITIVITY)
    , m_zoomSensitivity(DEFAULT_ZOOM_SENSITIVITY)
    , m_fov(DEFAULT_FOV)
    , m_aspectRatio(1.0f)
    , m_nearPlane(DEFAULT_NEAR_PLANE)
    , m_farPlane(DEFAULT_FAR_PLANE)
    , m_mode(Mode::ORBIT)
{
    // 计算相机与目标点之间的距离
    m_distance = glm::length(m_position - m_target);
    
    // 更新相机向量
    updateCameraVectors();
}

glm::mat4 Camera::getViewMatrix() const {
    return glm::lookAt(m_position, m_target, m_up);
}

glm::mat4 Camera::getProjectionMatrix() const {
    return glm::perspective(glm::radians(m_fov), m_aspectRatio, m_nearPlane, m_farPlane);
}

void Camera::setPosition(const glm::vec3& position) {
    m_position = position;
    m_distance = glm::length(m_position - m_target);
    updateCameraVectors();
}

void Camera::setTarget(const glm::vec3& target) {
    m_target = target;
    m_distance = glm::length(m_position - m_target);
    updateCameraVectors();
}

void Camera::setUpVector(const glm::vec3& up) {
    m_worldUp = up;
    updateCameraVectors();
}

void Camera::setPerspective(float fov, float aspectRatio, float nearPlane, float farPlane) {
    m_fov = fov;
    m_aspectRatio = aspectRatio;
    m_nearPlane = nearPlane;
    m_farPlane = farPlane;
}

void Camera::processMouseMove(float xoffset, float yoffset, bool constrainPitch) {
    // 仅在FPS模式下更新欧拉角
    if (m_mode == Mode::FPS) {
        xoffset *= m_mouseSensitivity;
        yoffset *= m_mouseSensitivity;

        m_yaw += xoffset;
        m_pitch += yoffset;

        // 防止万向节锁 (Gimbal Lock)
        if (constrainPitch) {
            m_pitch = std::clamp(m_pitch, MIN_PITCH, MAX_PITCH);
        }

        // 更新前、右、上向量
        updateCameraVectors();
    }
}

void Camera::processMouseScroll(float yoffset) {
    // 缩放功能 - 两种模式都支持
    if (m_mode == Mode::ORBIT) {
        // Orbit模式下调整视点与目标的距离
        m_distance -= yoffset * m_zoomSensitivity * m_distance * 0.1f;
        m_distance = std::clamp(m_distance, MIN_DISTANCE, MAX_DISTANCE);
        calculateOrbitPosition();
    } else {
        // FPS模式下调整FOV
        m_fov -= yoffset * m_zoomSensitivity * 2.0f;
        m_fov = std::clamp(m_fov, MIN_FOV, MAX_FOV);
    }
}

void Camera::processMouseDrag(float xoffset, float yoffset, bool isRightButton) {
    if (m_mode == Mode::ORBIT) {
        if (!isRightButton) {
            // 左键拖拽 - 旋转
            xoffset *= m_mouseSensitivity;
            yoffset *= m_mouseSensitivity;

            m_yaw += xoffset;
            m_pitch += yoffset;
            
            // 限制俯仰角
            m_pitch = std::clamp(m_pitch, MIN_PITCH, MAX_PITCH);
            
            // 根据欧拉角重新计算位置
            calculateOrbitPosition();
        } else {
            // 右键拖拽 - 平移相机和目标点
            float panSpeed = m_distance * 0.001f;
            glm::vec3 right = glm::normalize(glm::cross(m_front, m_worldUp));
            glm::vec3 up = glm::normalize(glm::cross(right, m_front));
            
            m_target -= right * xoffset * panSpeed * m_movementSpeed;
            m_target += up * yoffset * panSpeed * m_movementSpeed;
            
            // 更新相机位置
            calculateOrbitPosition();
        }
    }
}

void Camera::setMode(Mode mode) {
    m_mode = mode;
    
    if (m_mode == Mode::ORBIT) {
        // 如果切换到轨道模式，确保有有效的目标点
        calculateOrbitPosition();
    }
}

Camera::Mode Camera::getMode() const {
    return m_mode;
}

glm::vec3 Camera::getFrontVector() const {
    return m_front;
}

glm::vec3 Camera::getRightVector() const {
    return m_right;
}

void Camera::reset() {
    // 重置到默认状态
    m_position = glm::vec3(0.0f, 5.0f, 10.0f);
    m_target = glm::vec3(0.0f, 0.0f, 0.0f);
    m_worldUp = glm::vec3(0.0f, 1.0f, 0.0f);
    m_yaw = DEFAULT_YAW;
    m_pitch = DEFAULT_PITCH;
    m_distance = glm::length(m_position - m_target);
    m_fov = DEFAULT_FOV;
    
    updateCameraVectors();
}

void Camera::updateCameraVectors() {
    // 计算前向量
    m_front.x = cos(glm::radians(m_yaw)) * cos(glm::radians(m_pitch));
    m_front.y = sin(glm::radians(m_pitch));
    m_front.z = sin(glm::radians(m_yaw)) * cos(glm::radians(m_pitch));
    m_front = glm::normalize(m_front);
    
    // 重新计算右向量和上向量
    m_right = glm::normalize(glm::cross(m_front, m_worldUp));
    m_up = glm::normalize(glm::cross(m_right, m_front));
    
    if (m_mode == Mode::FPS) {
        // FPS模式下，前向量决定相机朝向，目标点跟随相机
        m_target = m_position + m_front;
    }
}

void Camera::calculateOrbitPosition() {
    // 根据目标点、距离和欧拉角计算相机位置
    m_front.x = cos(glm::radians(m_yaw)) * cos(glm::radians(m_pitch));
    m_front.y = sin(glm::radians(m_pitch));
    m_front.z = sin(glm::radians(m_yaw)) * cos(glm::radians(m_pitch));
    m_front = glm::normalize(m_front);
    
    // 更新相机位置
    m_position = m_target - m_front * m_distance;
    
    // 更新其他方向向量
    m_right = glm::normalize(glm::cross(m_front, m_worldUp));
    m_up = glm::normalize(glm::cross(m_right, m_front));
}

} // namespace mviz 
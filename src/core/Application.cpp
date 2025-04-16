#include "core/Application.h"
#include <iostream>
#include <filesystem>

// 添加必要的头文件
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "rendering/Shader.h"
#include "rendering/Renderer.h"
#include "core/Camera.h"
#include "core/TFManager.h"
#include "core/SceneManager.h"
#include "ui/UIManager.h"

namespace mviz {

// 初始化静态变量，用于回调函数访问当前实例
static Application* currentInstance = nullptr;

Application::Application(int width, int height, const std::string& title)
    : m_width(width)
    , m_height(height)
    , m_title(title)
    , m_window(nullptr)
    , m_initialized(false)
    , m_camera(std::make_shared<Camera>())
    , m_renderer(nullptr)
    , m_shader(nullptr)
    , m_firstMouse(true)
    , m_leftMousePressed(false)
    , m_rightMousePressed(false)
    , m_lastMouseX(0.0)
    , m_lastMouseY(0.0)
    , m_isRunning(true)
{
    // 存储当前实例指针
    currentInstance = this;
    
    // 创建TF管理器
    m_tfManager = std::make_shared<TFManager>();
    
    // 创建场景管理器
    m_sceneManager = std::make_shared<SceneManager>();
    
    // 创建UI管理器
    m_uiManager = std::make_shared<UIManager>();
}

Application::~Application() {
    // 清理资源
    m_uiManager.reset();
    m_sceneManager.reset();
    m_renderer.reset();
    m_shader.reset();

    if (m_window) {
        glfwDestroyWindow(m_window);
    }
    glfwTerminate();
}

bool Application::initialize() {
    // 设置GLFW错误回调
    glfwSetErrorCallback(errorCallback);

    // 初始化GLFW
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW" << std::endl;
        return false;
    }

    // 配置GLFW
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    
    // Mac特定配置
#ifdef __APPLE__
    // 启用高DPI支持 - 告诉GLFW启用Retina/高DPI模式
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_COCOA_RETINA_FRAMEBUFFER, GLFW_TRUE);
#endif

    // 创建窗口
    m_window = glfwCreateWindow(m_width, m_height, m_title.c_str(), nullptr, nullptr);
    if (!m_window) {
        std::cerr << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return false;
    }

    // 设置当前上下文
    glfwMakeContextCurrent(m_window);
    
    // 获取帧缓冲区大小以适应高DPI显示器
    int framebufferWidth, framebufferHeight;
    glfwGetFramebufferSize(m_window, &framebufferWidth, &framebufferHeight);
    
    // 设置回调
    setupCallbacks();

    // 初始化GLAD
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cerr << "Failed to initialize GLAD" << std::endl;
        return false;
    }

    // 设置视口 - 使用帧缓冲区大小而不是窗口大小
    glViewport(0, 0, framebufferWidth, framebufferHeight);
    
    // 设置相机投影参数 - 使用帧缓冲区宽高比
    float aspectRatio = static_cast<float>(framebufferWidth) / static_cast<float>(framebufferHeight);
    m_camera->setPerspective(45.0f, aspectRatio, 0.1f, 1000.0f);
    
    // 初始化渲染资源
    if (!initializeRenderingResources()) {
        std::cerr << "Failed to initialize rendering resources" << std::endl;
        return false;
    }
    
    // 初始化场景管理器
    m_sceneManager->setRenderer(m_renderer);
    m_sceneManager->setCamera(m_camera);
    if (!m_sceneManager->initialize()) {
        std::cerr << "Failed to initialize scene manager" << std::endl;
        return false;
    }
    
    // 创建示例TF数据
    m_sceneManager->createDemoTFs();
    
    // 初始化UI
    if (!initializeUI()) {
        std::cerr << "Failed to initialize UI" << std::endl;
        return false;
    }

    m_initialized = true;
    return true;
}

bool Application::initializeRenderingResources() {
    // 创建着色器
    try {
        std::filesystem::path currentPath = std::filesystem::current_path();
        std::string vertexShaderPath = (currentPath / "shaders/basic.vert").string();
        std::string fragmentShaderPath = (currentPath / "shaders/basic.frag").string();
        
        m_shader = std::make_shared<Shader>(vertexShaderPath, fragmentShaderPath);
    } catch (const std::exception& e) {
        std::cerr << "Failed to create shader: " << e.what() << std::endl;
        return false;
    }
    
    // 创建渲染器
    m_renderer = std::make_shared<Renderer>();
    if (!m_renderer->initialize()) {
        std::cerr << "Failed to initialize renderer" << std::endl;
        return false;
    }
    
    // 设置渲染器的着色器和相机
    m_renderer->setShader(m_shader);
    m_renderer->setCamera(m_camera.get());
    
    return true;
}

bool Application::initializeUI() {
    // 初始化UI管理器
    if (!m_uiManager->initialize(m_window)) {
        std::cerr << "Failed to initialize UI manager" << std::endl;
        return false;
    }
    
    return true;
}

void Application::run() {
    if (!m_initialized) {
        std::cerr << "Application not initialized" << std::endl;
        return;
    }

    // 主循环
    while (!glfwWindowShouldClose(m_window)) {
        // 处理输入
        processInput();

        // 清除缓冲区
        glClearColor(0.1f, 0.1f, 0.2f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        
        // 更新场景
        m_sceneManager->update();
        
        // 开始新一帧的UI渲染
        m_uiManager->newFrame();
        
        // 更新UI内容
        m_uiManager->update(*m_sceneManager);
        
        // 渲染场景
        m_sceneManager->render();
        
        // 渲染UI
        m_uiManager->render();
        
        // 交换缓冲区并处理事件
        glfwSwapBuffers(m_window);
        glfwPollEvents();
    }
}

void Application::framebufferSizeCallback(GLFWwindow* window, int width, int height) {
    // 设置视口尺寸为帧缓冲区尺寸
    glViewport(0, 0, width, height);
    
    // 更新当前实例中的相机宽高比
    if (currentInstance) {
        currentInstance->m_width = width;
        currentInstance->m_height = height;
        // 使用实际帧缓冲区尺寸的宽高比
        float aspectRatio = static_cast<float>(width) / static_cast<float>(height);
        currentInstance->m_camera->setPerspective(
            45.0f,
            aspectRatio,
            0.1f,
            1000.0f
        );
    }
}

void Application::errorCallback(int error, const char* description) {
    std::cerr << "GLFW Error " << error << ": " << description << std::endl;
}

void Application::keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    // 这里可以处理键盘输入
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, true);
    }
    
    // 切换相机模式
    if (key == GLFW_KEY_F && action == GLFW_PRESS && currentInstance) {
        Camera::Mode currentMode = currentInstance->m_camera->getMode();
        Camera::Mode newMode = (currentMode == Camera::Mode::ORBIT) ? 
                                Camera::Mode::FPS : Camera::Mode::ORBIT;
        currentInstance->m_camera->setMode(newMode);
    }
    
    // 重置相机
    if (key == GLFW_KEY_R && action == GLFW_PRESS && currentInstance) {
        currentInstance->m_camera->reset();
    }
}

void Application::mouseButtonCallback(GLFWwindow* window, int button, int action, int mods) {
    if (currentInstance) {
        currentInstance->processMouseButton(button, action, mods);
    }
}

void Application::cursorPosCallback(GLFWwindow* window, double xpos, double ypos) {
    if (currentInstance) {
        currentInstance->processMouseMovement(xpos, ypos);
    }
}

void Application::scrollCallback(GLFWwindow* window, double xoffset, double yoffset) {
    if (currentInstance) {
        currentInstance->processMouseScroll(yoffset);
    }
}

void Application::processInput() {
    // 处理键盘输入
    if (glfwGetKey(m_window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
        glfwSetWindowShouldClose(m_window, true);
    }
    
    // 根据相机模式添加更多的键盘控制
    if (m_camera->getMode() == Camera::Mode::FPS) {
        // FPS模式的WASD移动
        float cameraSpeed = 0.05f;
        
        if (glfwGetKey(m_window, GLFW_KEY_W) == GLFW_PRESS) {
            glm::vec3 pos = m_camera->getPosition();
            pos += m_camera->getFrontVector() * cameraSpeed;
            m_camera->setPosition(pos);
        }
        
        if (glfwGetKey(m_window, GLFW_KEY_S) == GLFW_PRESS) {
            glm::vec3 pos = m_camera->getPosition();
            pos -= m_camera->getFrontVector() * cameraSpeed;
            m_camera->setPosition(pos);
        }
        
        if (glfwGetKey(m_window, GLFW_KEY_A) == GLFW_PRESS) {
            glm::vec3 pos = m_camera->getPosition();
            pos -= m_camera->getRightVector() * cameraSpeed;
            m_camera->setPosition(pos);
        }
        
        if (glfwGetKey(m_window, GLFW_KEY_D) == GLFW_PRESS) {
            glm::vec3 pos = m_camera->getPosition();
            pos += m_camera->getRightVector() * cameraSpeed;
            m_camera->setPosition(pos);
        }
    }
}

void Application::processMouseMovement(double xpos, double ypos) {
    // 检查鼠标是否在ImGui窗口上
    if (m_uiManager && m_uiManager->isMouseOverUI()) {
        return; // 如果鼠标在UI上，则不处理相机移动
    }
    
    if (m_firstMouse) {
        m_lastMouseX = xpos;
        m_lastMouseY = ypos;
        m_firstMouse = false;
        return;
    }
    
    double xoffset = xpos - m_lastMouseX;
    double yoffset = m_lastMouseY - ypos; // 反转Y轴
    
    m_lastMouseX = xpos;
    m_lastMouseY = ypos;
    
    // 根据鼠标按键状态处理不同的操作
    if (m_leftMousePressed) {
        m_camera->processMouseDrag(xoffset, yoffset, false);
    } else if (m_rightMousePressed) {
        m_camera->processMouseDrag(xoffset, -yoffset, true);
    } else {
        // 处理FPS模式下的自由移动
        if (m_camera->getMode() == Camera::Mode::FPS) {
            m_camera->processMouseMove(xoffset, yoffset);
        }
    }
}

void Application::processMouseButton(int button, int action, int mods) {
    // 检查鼠标是否在ImGui窗口上
    if (m_uiManager && m_uiManager->isMouseOverUI()) {
        return; // 如果鼠标在UI上，则不处理鼠标按键事件
    }
    
    if (button == GLFW_MOUSE_BUTTON_LEFT) {
        m_leftMousePressed = (action == GLFW_PRESS);
        
        // 如果按下鼠标，获取焦点
        if (m_leftMousePressed) {
            glfwSetInputMode(m_window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        }
    } else if (button == GLFW_MOUSE_BUTTON_RIGHT) {
        m_rightMousePressed = (action == GLFW_PRESS);
        
        // 如果按下鼠标，获取焦点
        if (m_rightMousePressed) {
            glfwSetInputMode(m_window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        }
    }
    
    // 如果所有按键都释放
    if (!m_leftMousePressed && !m_rightMousePressed && m_camera->getMode() == Camera::Mode::FPS) {
        // 在FPS模式下，默认捕获鼠标
        glfwSetInputMode(m_window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    }
}

void Application::processMouseScroll(double yoffset) {
    // 检查鼠标是否在ImGui窗口上
    if (m_uiManager && m_uiManager->isMouseOverUI()) {
        return; // 如果鼠标在UI上，则不处理滚轮事件
    }
    
    m_camera->processMouseScroll(yoffset);
}

void Application::setupCallbacks() {
    // 设置窗口用户指针
    glfwSetWindowUserPointer(m_window, this);
    
    // 设置回调函数
    glfwSetFramebufferSizeCallback(m_window, framebufferSizeCallback);
    glfwSetKeyCallback(m_window, keyCallback);
    glfwSetMouseButtonCallback(m_window, mouseButtonCallback);
    glfwSetCursorPosCallback(m_window, cursorPosCallback);
    glfwSetScrollCallback(m_window, scrollCallback);
    
    // 配置鼠标行为 - 默认为正常模式
    glfwSetInputMode(m_window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
}

void Application::addTransform(const std::string& parent_frame, const std::string& child_frame, const Transform& transform) {
    m_sceneManager->getTFManager().addTransform(parent_frame, child_frame, transform);
}

bool Application::lookupTransform(const std::string& target_frame, const std::string& source_frame, Transform& transform) const {
    return m_sceneManager->getTFManager().lookupTransform(target_frame, source_frame, transform);
}

} // namespace mviz 
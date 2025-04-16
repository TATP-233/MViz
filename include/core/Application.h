#pragma once

// 系统头文件
#include <string>
#include <memory>
#include <vector>

// 前向声明
struct GLFWwindow; // 避免直接包含GLFW头文件

namespace mviz {

// 内部类型前向声明
class TFManager;
class Shader;
class Renderer;
class Camera;
class SceneManager;
struct Transform;

class Application {
public:
    // 默认构造函数
    Application();
    
    // 带参数构造函数
    Application(int width, int height, const std::string& title);
    
    // 析构函数
    ~Application();

    // 禁用拷贝构造函数和赋值操作符
    Application(const Application&) = delete;
    Application& operator=(const Application&) = delete;

    // 获取应用程序的单例实例
    static Application& get();

    // 初始化应用程序
    bool initialize();

    // 运行应用程序的主循环
    void run();

    // 关闭应用程序
    void shutdown();

    // 处理窗口大小改变
    void onWindowResize(int width, int height);

    // 获取窗口句柄
    GLFWwindow* getWindow() const { return m_window; }

    // 获取窗口尺寸
    void getWindowSize(int& width, int& height) const;

    // 示例TF操作方法
    void createDemoTFs();
    void addTransform(const std::string& parentFrame, const std::string& childFrame, const Transform& transform);
    bool lookupTransform(const std::string& targetFrame, const std::string& sourceFrame, Transform& transform) const;

    // 获取场景管理器
    SceneManager* getSceneManager() const { return m_sceneManager.get(); }

private:
    // 初始化OpenGL
    bool initializeOpenGL();
    
    // 初始化渲染资源
    bool initializeRenderingResources();

    // 处理输入
    void processInput();

    // 处理鼠标移动
    void processMouseMovement(double xpos, double ypos);
    
    // 处理鼠标按钮
    void processMouseButton(int button, int action, int mods);
    
    // 处理鼠标滚轮
    void processMouseScroll(double yoffset);
    
    // 设置回调函数
    void setupCallbacks();

    // 更新场景
    void update(float deltaTime);

    // 渲染场景
    void render();

    // GLFW回调函数
    static void framebufferSizeCallback(GLFWwindow* window, int width, int height);
    static void errorCallback(int error, const char* description);
    static void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
    static void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods);
    static void cursorPosCallback(GLFWwindow* window, double xpos, double ypos);
    static void scrollCallback(GLFWwindow* window, double xoffset, double yoffset);
    
    // 静态当前实例指针
    static Application* s_instance;

    // 窗口相关
    GLFWwindow* m_window;
    int m_width;
    int m_height;
    std::string m_title;

    // 相机
    std::shared_ptr<Camera> m_camera;

    // 渲染器
    std::shared_ptr<Renderer> m_renderer;

    // 着色器
    std::shared_ptr<Shader> m_shader;
    
    // 场景管理器 (新增)
    std::shared_ptr<SceneManager> m_sceneManager;
    
    // TF管理器 (可能会通过SceneManager使用，但暂时保留向后兼容性)
    std::shared_ptr<TFManager> m_tfManager;

    // 计时相关
    float m_deltaTime;
    float m_lastFrameTime;

    // 应用程序状态
    bool m_isRunning;
    bool m_initialized;
    
    // 鼠标相关状态
    bool m_firstMouse;
    bool m_leftMousePressed;
    bool m_rightMousePressed;
    double m_lastMouseX;
    double m_lastMouseY;
};

} // namespace mviz 
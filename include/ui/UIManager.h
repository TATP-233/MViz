#pragma once

#include <memory>
#include <string>
#include <vector>

// 前向声明
struct GLFWwindow;
namespace mviz {
    class SceneManager;
}

namespace mviz {

class UIManager {
public:
    UIManager();
    ~UIManager();

    // 初始化ImGui
    bool initialize(GLFWwindow* window);

    // 开始新一帧的UI渲染
    void newFrame();

    // 渲染UI
    void render();

    // 清理资源
    void shutdown();

    // 更新UI内容 - 场景管理器引用用于获取坐标系和可视化对象
    void update(SceneManager& sceneManager);
    
    // 检查鼠标是否悬停在UI上
    bool isMouseOverUI() const;

private:
    // 渲染左侧控制面板
    void renderControlPanel(SceneManager& sceneManager);

    // 渲染参考坐标系选择器
    void renderReferenceFrameSelector(SceneManager& sceneManager);

    // 渲染坐标系统设置
    void renderCoordinateSystemSettings(SceneManager& sceneManager);

    // 渲染可视化对象列表
    void renderVisualObjectList(SceneManager& sceneManager);

    // 状态变量
    bool m_initialized;
    std::string m_selectedReferenceFrame;
    bool m_showDemoWindow;
};

} // namespace mviz 
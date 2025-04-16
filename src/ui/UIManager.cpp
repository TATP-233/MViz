#include "ui/UIManager.h"
#include "core/SceneManager.h"
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <GLFW/glfw3.h>
#include <iostream>

namespace mviz {

UIManager::UIManager()
    : m_initialized(false)
    , m_selectedReferenceFrame("world")
    , m_showDemoWindow(false)
{
}

UIManager::~UIManager() {
    shutdown();
}

bool UIManager::initialize(GLFWwindow* window) {
    // 检查输入有效性
    if (!window) {
        std::cerr << "Error: UIManager::initialize - Invalid window pointer" << std::endl;
        return false;
    }

    // 初始化ImGui上下文
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;  // 启用键盘控制

    // 设置ImGui风格
    ImGui::StyleColorsDark();

    // 初始化平台/渲染器绑定
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330 core");

    m_initialized = true;
    return true;
}

void UIManager::newFrame() {
    if (!m_initialized) return;

    // 开始新一帧
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
}

void UIManager::render() {
    if (!m_initialized) return;

    // 渲染ImGui
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void UIManager::shutdown() {
    if (!m_initialized) return;

    // 清理ImGui资源
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    m_initialized = false;
}

bool UIManager::isMouseOverUI() const {
    if (!m_initialized) return false;
    
    // 使用ImGui的IO来检查鼠标是否在UI上
    return ImGui::GetIO().WantCaptureMouse;
}

void UIManager::update(SceneManager& sceneManager) {
    if (!m_initialized) return;

    // 显示ImGui演示窗口（开发调试用，最终版本可以移除）
    if (m_showDemoWindow) {
        ImGui::ShowDemoWindow(&m_showDemoWindow);
    }

    // 渲染控制面板
    renderControlPanel(sceneManager);
}

void UIManager::renderControlPanel(SceneManager& sceneManager) {
    // 创建左侧控制面板
    ImGui::SetNextWindowPos(ImVec2(10, 10), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(250, 500), ImGuiCond_FirstUseEver);
    
    if (ImGui::Begin("MViz Control Panel")) {
        ImGui::Text("MViz - 3D Visualization Tool");
        ImGui::Separator();

        // 添加演示窗口切换选项（开发调试用）
        ImGui::Checkbox("Show ImGui Demo Window", &m_showDemoWindow);
        ImGui::Separator();

        // 渲染参考坐标系选择器
        renderReferenceFrameSelector(sceneManager);
        ImGui::Separator();

        // 渲染坐标系显示设置
        renderCoordinateSystemSettings(sceneManager);
        ImGui::Separator();

        // 渲染可视化对象列表
        renderVisualObjectList(sceneManager);
    }
    ImGui::End();
}

void UIManager::renderReferenceFrameSelector(SceneManager& sceneManager) {
    ImGui::Text("Reference Frame");
    
    // 获取所有可用的坐标系
    std::vector<std::string> availableFrames = sceneManager.getAvailableFrames();
    
    // 如果没有可用的坐标系，显示提示信息
    if (availableFrames.empty()) {
        ImGui::TextColored(ImVec4(1.0f, 0.5f, 0.5f, 1.0f), "No available frames");
        return;
    }
    
    // 获取当前选中的参考坐标系
    m_selectedReferenceFrame = sceneManager.getReferenceFrame();
    
    // 创建一个组合框用于选择参考坐标系
    if (ImGui::BeginCombo("##ReferenceFrame", m_selectedReferenceFrame.c_str())) {
        for (const auto& frame : availableFrames) {
            bool isSelected = (m_selectedReferenceFrame == frame);
            if (ImGui::Selectable(frame.c_str(), isSelected)) {
                m_selectedReferenceFrame = frame;
                sceneManager.setReferenceFrame(m_selectedReferenceFrame);
            }
            
            if (isSelected) {
                ImGui::SetItemDefaultFocus();
            }
        }
        ImGui::EndCombo();
    }
}

void UIManager::renderCoordinateSystemSettings(SceneManager& sceneManager) {
    if (ImGui::CollapsingHeader("Coordinate System Settings", ImGuiTreeNodeFlags_DefaultOpen)) {
        // 获取当前设置
        bool showLabels = sceneManager.getShowFrameLabels();
        float labelSize = sceneManager.getFrameLabelSize();
        
        // 显示标签的切换开关
        if (ImGui::Checkbox("Show Frame Labels", &showLabels)) {
            sceneManager.setShowFrameLabels(showLabels);
        }
        
        // 标签大小的滑动条
        if (ImGui::SliderFloat("Label Size", &labelSize, 0.5f, 3.0f, "%.1f")) {
            sceneManager.setFrameLabelSize(labelSize);
        }
        
        // 轴线粗细的滑动条
        float axisThickness = sceneManager.getAxisThickness();
        if (ImGui::SliderFloat("Axis Thickness", &axisThickness, 1.0f, 5.0f, "%.1f")) {
            sceneManager.setAxisThickness(axisThickness);
        }
    }
}

void UIManager::renderVisualObjectList(SceneManager& sceneManager) {
    ImGui::Text("Visualization Objects");
    
    // 获取所有可视化对象
    auto& visualObjects = sceneManager.getVisualObjects();
    
    // 如果没有可视化对象，显示提示信息
    if (visualObjects.empty()) {
        ImGui::TextColored(ImVec4(1.0f, 0.5f, 0.5f, 1.0f), "No visualization objects");
        return;
    }
    
    // 创建分组折叠面板
    if (ImGui::CollapsingHeader("Coordinate Frames", ImGuiTreeNodeFlags_DefaultOpen)) {
        for (auto& [name, object] : visualObjects) {
            // 仅显示坐标系类型的对象
            if (name.find("_axes") != std::string::npos) {
                bool isVisible = object->isVisible();
                if (ImGui::Checkbox(name.c_str(), &isVisible)) {
                    object->setVisible(isVisible);
                }
            }
        }
    }
    
    // 为未来扩展添加其他类型的可视化对象
    if (ImGui::CollapsingHeader("Point Clouds", ImGuiTreeNodeFlags_DefaultOpen)) {
        ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), "No point clouds available");
    }
    
    if (ImGui::CollapsingHeader("Geometric Primitives", ImGuiTreeNodeFlags_DefaultOpen)) {
        ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), "No primitives available");
    }
}

} // namespace mviz 
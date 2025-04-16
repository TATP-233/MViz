#include "core/SceneManager.h"
#include "rendering/Renderer.h"
#include "core/Camera.h"
#include "visualization/PointCloudVisual.h"
#include "data/DataTypes.h"
#include <iostream>
#include <glm/gtc/matrix_transform.hpp>
#include <random>

namespace mviz {

//-------------------- VisualObject 实现 --------------------

VisualObject::VisualObject(const std::string& name, const std::string& frame_id)
    : m_name(name)
    , m_frame_id(frame_id)
    , m_visible(true)
    , m_model_matrix(1.0f) // 初始化为单位矩阵
{
}

void VisualObject::update(TFManager& tf_manager, const std::string& reference_frame) {
    // 查找从对象坐标系到参考坐标系的变换
    Transform transform;
    bool success = tf_manager.lookupTransform(reference_frame, m_frame_id, transform);
    
    if (success) {
        // 如果找到变换，更新模型矩阵
        m_model_matrix = transform.toMat4();
    } else {
        // 如果找不到变换，设置为单位矩阵并输出警告
        m_model_matrix = glm::mat4(1.0f);
        
        // 特殊情况：如果frame_id就是reference_frame，不需要警告
        if (m_frame_id != reference_frame) {
            std::cerr << "Warning: Could not find transform from '" << m_frame_id 
                      << "' to '" << reference_frame << "'" << std::endl;
        }
    }
}

//-------------------- AxesVisual 实现 --------------------

AxesVisual::AxesVisual(const std::string& name, const std::string& frame_id, float size)
    : VisualObject(name, frame_id)
    , m_size(size)
{
}

void AxesVisual::update(TFManager& tf_manager, const std::string& reference_frame) {
    // 调用基类的update方法更新模型矩阵
    VisualObject::update(tf_manager, reference_frame);
}

void AxesVisual::draw(Renderer& renderer, const glm::mat4& view_projection_matrix) {
    if (!m_visible) return;
    
    // 使用渲染器绘制坐标轴
    renderer.drawCoordinateAxes();
}

//-------------------- SceneManager 实现 --------------------

SceneManager::SceneManager()
    : m_reference_frame("world")
{
}

SceneManager::~SceneManager() {
    // 清理资源
    m_visual_objects.clear();
    m_world_axes.reset();
}

bool SceneManager::initialize() {
    // 创建世界坐标轴
    m_world_axes = std::make_shared<AxesVisual>("world_axes", "world", 1.0f);
    m_visual_objects["world_axes"] = m_world_axes;
    
    return true;
}

void SceneManager::setRenderer(std::shared_ptr<Renderer> renderer) {
    m_renderer = renderer;
    
    // 将TF管理器设置到渲染器中
    if (m_renderer) {
        m_renderer->setTFManager(&m_tf_manager);
        m_renderer->setSceneManager(this);
    }
}

void SceneManager::setCamera(std::shared_ptr<Camera> camera) {
    m_camera = camera;
}

void SceneManager::addVisualObject(const VisualObject::SharedPtr& object) {
    if (!object) return;
    
    // 添加到对象映射表
    m_visual_objects[object->getName()] = object;
}

void SceneManager::removeVisualObject(const std::string& name) {
    auto it = m_visual_objects.find(name);
    if (it != m_visual_objects.end()) {
        m_visual_objects.erase(it);
    }
}

VisualObject::SharedPtr SceneManager::getVisualObject(const std::string& name) {
    auto it = m_visual_objects.find(name);
    if (it != m_visual_objects.end()) {
        return it->second;
    }
    return nullptr;
}

void SceneManager::createDemoTFs() {
    // 创建示例TF树
    // 定义变换: world -> base_link
    Transform world_to_base;
    world_to_base.translation = glm::vec3(0.0f, 0.3f, 0.0f);
    world_to_base.rotation = glm::quat(1.0f, 0.0f, 0.0f, 0.0f); // 单位四元数
    m_tf_manager.addTransform("world", "base_link", world_to_base);
    
    // 定义变换: base_link -> sensor
    Transform base_to_sensor;
    base_to_sensor.translation = glm::vec3(1.0f, 0.5f, 0.0f);
    base_to_sensor.rotation = glm::quat(0.707f, 0.0f, 0.707f, 0.0f); // 单位四元数   
    m_tf_manager.addTransform("base_link", "sensor", base_to_sensor);
    
    // 定义变换: base_link -> left_wheel
    Transform base_to_left_wheel;
    base_to_left_wheel.translation = glm::vec3(0.0f, -0.3f, -0.5f);
    base_to_left_wheel.rotation = glm::quat(0.0f, 0.0f, 0.0f, 1.0f); // 单位四元数
    m_tf_manager.addTransform("base_link", "left_wheel", base_to_left_wheel);
    
    // 定义变换: base_link -> right_wheel
    Transform base_to_right_wheel;
    base_to_right_wheel.translation = glm::vec3(0.0f, -0.3f, 0.5f);
    base_to_right_wheel.rotation = glm::quat(0.707f, 0.0f, 0.0f, 0.707f); // 单位四元数
    m_tf_manager.addTransform("base_link", "right_wheel", base_to_right_wheel);
    
    // 为每个坐标系创建可视化对象
    addVisualObject(std::make_shared<AxesVisual>("base_link_axes", "base_link", 0.5f));
    addVisualObject(std::make_shared<AxesVisual>("sensor_axes", "sensor", 0.3f));
    addVisualObject(std::make_shared<AxesVisual>("left_wheel_axes", "left_wheel", 0.2f));
    addVisualObject(std::make_shared<AxesVisual>("right_wheel_axes", "right_wheel", 0.2f));
}

void SceneManager::setReferenceFrame(const std::string& frame) {
    m_reference_frame = frame;
}

std::vector<std::string> SceneManager::getAvailableFrames() const {
    return m_tf_manager.getAllFrameNames();
}

void SceneManager::update() {
    // 更新所有可视化对象
    for (auto& [name, object] : m_visual_objects) {
        if (object) {
            object->update(m_tf_manager, m_reference_frame);
        }
    }
    
    // 更新渲染器中的TF可视化数据
    if (m_renderer) {
        m_renderer->createTFVisualization();
    }
}

void SceneManager::render() {
    if (!m_renderer || !m_camera) return;
    
    // 确保使用基本着色器
    m_renderer->useShader(Renderer::ShaderType::BASIC);
    
    // 获取相机的视图矩阵和投影矩阵
    glm::mat4 view = m_camera->getViewMatrix();
    glm::mat4 projection = m_camera->getProjectionMatrix();
    glm::mat4 view_projection = projection * view;
    
    // 绘制地面网格
    m_renderer->drawGroundGrid(m_reference_frame);
    
    // 绘制TF连接线
    m_renderer->drawTFVisualization();
    
    // 绘制所有可视化对象
    for (auto& [name, object] : m_visual_objects) {
        if (object && object->isVisible()) {
            object->draw(*m_renderer, view_projection);
        }
    }
}

// 坐标系可视化设置方法实现
void SceneManager::setShowFrameLabels(bool show) {
    if (m_renderer) {
        m_renderer->setFrameLabelsVisible(show);
    }
}

bool SceneManager::getShowFrameLabels() const {
    return m_renderer ? m_renderer->isFrameLabelsVisible() : true;
}

void SceneManager::setFrameLabelSize(float size) {
    if (m_renderer) {
        m_renderer->setFrameLabelsSize(size);
    }
}

float SceneManager::getFrameLabelSize() const {
    return m_renderer ? m_renderer->getFrameLabelsSize() : 1.0f;
}

void SceneManager::setAxisThickness(float thickness) {
    if (m_renderer) {
        m_renderer->setAxisThickness(thickness);
    }
}

float SceneManager::getAxisThickness() const {
    return m_renderer ? m_renderer->getAxisThickness() : 1.0f;
}

bool SceneManager::isFrameVisible(const std::string& frame_name) const {
    // 查找对应名称的坐标系可视化对象
    std::string axesName = frame_name + "_axes";
    auto it = m_visual_objects.find(axesName);
    if (it != m_visual_objects.end()) {
        return it->second->isVisible();
    }
    
    // 如果找不到对应的可视化对象，默认为可见
    return true;
}

void SceneManager::createDemoPointCloud() {
    // 创建一个新的点云可视化对象
    auto pointCloudVisual = std::make_shared<PointCloudVisual>("demo_point_cloud", "sensor");
    
    // 生成示例点云数据
    PointCloudData pointCloud;
    
    // 使用随机数生成器创建一些随机点
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<float> posDist(-1.0f, 1.0f); // 位置范围
    std::uniform_real_distribution<float> colorDist(0.0f, 1.0f); // 颜色范围
    
    // 设置点的数量
    const int numPoints = 1000;
    
    // 生成随机点
    for (int i = 0; i < numPoints; ++i) {
        // 生成随机点位置（球形分布）
        float r = 0.7f * std::pow(posDist(gen), 2); // 距离原点的距离（二次方分布使点更集中在中心）
        float theta = posDist(gen) * glm::pi<float>(); // 天顶角
        float phi = posDist(gen) * glm::pi<float>() * 2.0f; // 方位角
        
        float x = r * std::sin(theta) * std::cos(phi);
        float y = r * std::sin(theta) * std::sin(phi);
        float z = r * std::cos(theta);
        
        // 添加点
        pointCloud.points.push_back(glm::vec3(x, y, z));
        
        // 添加随机颜色
        pointCloud.colors.push_back(glm::vec3(
            colorDist(gen),
            colorDist(gen),
            colorDist(gen)
        ));
    }
    
    // 设置点的大小
    pointCloud.pointSize = 2.0f;
    
    // 将点云数据设置到可视化对象中
    pointCloudVisual->setPointCloud(pointCloud);
    
    // 将点云可视化对象添加到场景中
    addVisualObject(pointCloudVisual);
    
    std::cout << "Created demo point cloud with " << numPoints << " points" << std::endl;
}

} // namespace mviz 
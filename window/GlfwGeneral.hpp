//
// Created by Jhong on 2025/4/27.
//

#include "../myVulkan/VKBase.h"
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#pragma comment(lib, "glfw3.lib") //链接编译所需的静态库

//窗口的指针，全局变量自动初始化为NULL
GLFWwindow* pWindow;
//显示器信息的指针
GLFWmonitor* pMonitor;
//窗口标题
const char* windowTitle = "EasyVK";
//窗口状态
bool isFullScreen;
//窗口尺寸
VkExtent2D windowSize;

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);
void MakeWindowFullScreen();
void MakeWindowWindowed(VkOffset2D position, VkExtent2D size);

//初始化窗口
bool InitializeWindow(VkExtent2D size, bool fullScreen = false, bool isResizable = true, bool limitFrameRate = true) {
    using namespace myVulkan;
    //初始化glfw
    if(!glfwInit())
    {
        std::cout << std::format("[ InitializeWindow ] ERROR\nFailed to initialize GLFW!\n");
        return false;
    }

    //创建监视器
    pMonitor = glfwGetPrimaryMonitor();

    glfwWindowHint(GLFW_CLIENT_API,GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE,isResizable);

    //创建窗口
    pWindow = glfwCreateWindow(size.width,size.height,windowTitle, nullptr, nullptr);
    if(!pWindow)
    {
        std::cout << std::format("[ InitializeWindow ]\nFailed to create a glfw window!\n");
        glfwTerminate();
        return false;
    }
    //获取窗口信息
    windowSize = size;
    ::isFullScreen = fullScreen;
    if(fullScreen) MakeWindowFullScreen();

    //注册键盘回调函数
    glfwSetKeyCallback(pWindow,key_callback);

    //获取实例级别扩展
#ifdef _WIN32
    graphicsBase::Base().AddInstanceExtension(VK_KHR_SURFACE_EXTENSION_NAME);
    graphicsBase::Base().AddInstanceExtension(VK_KHR_WIN32_SURFACE_EXTENSION_NAME);
#else
    uint32_t extensionCount = 0;
    const char** extensionNames;
    extensionNames = glfwGetRequiredInstanceExtensions(&extensionCount);
    if (!extensionNames) {
        std::cout << std::format("[ InitializeWindow ]\nVulkan is not available on this machine!\n");
        glfwTerminate();
        return false;
    }
    for (size_t i = 0; i < extensionCount; i++)
        graphicsBase::Base().AddInstanceExtension(extensionNames[i]);
#endif

    //获取设备级别扩展
    graphicsBase::Base().AddDeviceExtension(VK_KHR_SWAPCHAIN_EXTENSION_NAME);

    //首先创建Vulkan实例
    graphicsBase::Base().UseLatestApiVersion();
    if(graphicsBase::Base().CreateInstance())
        return false;

    //创建Window Surface
    VkSurfaceKHR surface = VK_NULL_HANDLE;
    if(auto result = glfwCreateWindowSurface(graphicsBase::Base().Instance(),pWindow, nullptr,&surface)){
        std::cout << std::format("[ InitializeWindow ] ERROR\nFailed to create a window surface!\nError code: {}\n", int32_t(result));
        glfwTerminate();
        return false;
    }
    graphicsBase::Base().Surface(surface);

    //获取设备
    if (//获取物理设备，并使用列表中的第一个物理设备，这里不考虑以下任意函数失败后更换物理设备的情况
            graphicsBase::Base().GetPhysicalDevices() ||
            //一个true一个false，暂时不需要计算用的队列
            graphicsBase::Base().DeterminePhysicalDevice(0, true, false) ||
            //创建逻辑设备
            graphicsBase::Base().CreateDevice())
        return false;

    //创建交换链
    if (graphicsBase::Base().CreateSwapchain(limitFrameRate))
        return false;

    return true;
}

//终止窗口
void TerminateWindow() {
    using namespace myVulkan;
    //应该确保Vulkan没有与窗口系统的呈现引擎进行交互
    graphicsBase::Base().WaitIdle();
    glfwTerminate();
}

//在标题显示帧率
void TitleFPS()
{
    static double time0 = glfwGetTime();
    static double time1;
    static double dt;
    static int dframe = -1;
    static std::stringstream info;
    time1 = glfwGetTime();
    dframe++;
    if((dt=time1-time0)>=1)
    {
        info.precision(1);
        info << windowTitle << "    " << std::fixed << dframe / dt << " FPS";
        glfwSetWindowTitle(pWindow,info.str().c_str());
        info.str("");
        time0=time1;
        dframe=0;
    }
}

//全屏功能函数
void MakeWindowFullScreen() {
    const GLFWvidmode* pMode = glfwGetVideoMode(pMonitor);
    glfwSetWindowMonitor(pWindow, pMonitor, 0, 0, pMode->width, pMode->height, pMode->refreshRate);
}

//窗口化功能函数
void MakeWindowWindowed(VkOffset2D position, VkExtent2D size) {
    const GLFWvidmode* pMode = glfwGetVideoMode(pMonitor);
    glfwSetWindowMonitor(pWindow, nullptr, position.x, position.y, size.width, size.height, pMode->refreshRate);
}

//IO部分
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if (key == GLFW_KEY_F && action == GLFW_PRESS)
    {
        if(isFullScreen)
        {
            MakeWindowWindowed({0,0},windowSize);
            isFullScreen = false;

        }
        else
        {
            MakeWindowFullScreen();
            isFullScreen = true;
        }
    }
}
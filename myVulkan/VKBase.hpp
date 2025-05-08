#pragma once
#include "EasyVKStart.h"

namespace myVulkan {
////重定义结果类型result_t
//情况1：根据函数返回值确定是否抛异常
#ifdef VK_RESULT_THROW
    class result_t {
    VkResult result;
public:
    static void(*callback_throw)(VkResult);
    result_t(VkResult result) :result(result) {}
    result_t(result_t&& other) noexcept :result(other.result) { other.result = VK_SUCCESS; }
    ~result_t() noexcept(false) {
        if (uint32_t(result) < VK_RESULT_MAX_ENUM)
            return;
        if (callback_throw)
            callback_throw(result);
        throw result;
    }
    operator VkResult() {
        VkResult result = this->result;
        this->result = VK_SUCCESS;
        return result;
    }
};
inline void(*result_t::callback_throw)(VkResult);

//情况2：若抛弃函数返回值，让编译器发出警告
#elif defined VK_RESULT_NODISCARD
    struct [[nodiscard]] result_t {
    VkResult result;
    result_t(VkResult result) :result(result) {}
    operator VkResult() const { return result; }
};
//在本文件中关闭弃值提醒（因为我懒得做处理）
#pragma warning(disable:4834)
#pragma warning(disable:6031)

//情况3：默认
#else
    using result_t = VkResult;
#endif

////封装Vulkan对象
//析构
#define DestroyHandleBy(Func) if (handle) { Func(graphicsBase::Base().Device(), handle, nullptr); handle = VK_NULL_HANDLE; }
//移动构造
#define MoveHandle handle = other.handle; other.handle = VK_NULL_HANDLE;
//移动赋值
#define DefineMoveAssignmentOperator(type) type& operator=(type&& other) { this->~type(); MoveHandle; return *this; }
//转换函数
#define DefineHandleTypeOperator operator decltype(handle)() const { return handle; }
//句柄取址
#define DefineAddressFunction const auto Address() const { return &handle; }

//作用为放置于块内，其下部分只执行一次，其上部分可多次执行
#define ExecuteOnce(...) { static bool executed = false; if (executed) return __VA_ARGS__; executed = true; }

    //定义错误输出，可自定义错误输出流
    inline auto& outStream = std::cout;//不是constexpr，因为std::cout具有外部链接

    template<typename T>
    class arrayRef {
        T* const pArray = nullptr;
        size_t count = 0;
    public:
        //从空参数构造，count为0
        arrayRef() = default;
        //从单个对象构造，count为1
        arrayRef(T& data) :pArray(&data), count(1) {}
        //从顶级数组构造
        template<size_t elementCount>
        arrayRef(T(&data)[elementCount]) : pArray(data), count(elementCount) {}
        //从指针和元素个数构造
        arrayRef(T* pData, size_t elementCount) :pArray(pData), count(elementCount) {}
        //复制构造，若T带const修饰，兼容从对应的无const修饰版本的arrayRef构造
        //24.01.07 修正因复制粘贴产生的typo：从pArray(&other)改为pArray(other.Pointer())
        arrayRef(const arrayRef<std::remove_const_t<T>>& other) :pArray(other.Pointer()), count(other.Count()) {}
        //Getter
        T* Pointer() const { return pArray; }
        size_t Count() const { return count; }
        //Const Function
        T& operator[](size_t index) const { return pArray[index]; }
        T* begin() const { return pArray; }
        T* end() const { return pArray + count; }
        //Non-const Function
        //禁止复制/移动赋值
        arrayRef& operator=(const arrayRef&) = delete;
    };

    //默认的交换链图像尺寸
    constexpr VkExtent2D defaultWindowSize = { 1280, 720 };

    class graphicsBasePlus;

    class graphicsBase {
    private:
        static graphicsBase singleton;

        graphicsBase(); // Declaration only
        graphicsBase(graphicsBase&&) = delete;
        ~graphicsBase(); // Declaration only

        //api版本
        uint32_t apiVersion = VK_API_VERSION_1_0;

        //Vulkan实例
        VkInstance instance = VK_NULL_HANDLE;
        std::vector<const char*> instanceLayers;
        std::vector<const char*> instanceExtensions;

        //Debug Messenger
        VkDebugUtilsMessengerEXT debugMessenger = VK_NULL_HANDLE;

        //window surface
        VkSurfaceKHR surface = VK_NULL_HANDLE;

        //设备
        VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
        VkPhysicalDeviceProperties physicalDeviceProperties{};
        VkPhysicalDeviceMemoryProperties physicalDeviceMemoryProperties{};
        std::vector<VkPhysicalDevice> availablePhysicalDevices;
        std::vector<const char*> deviceExtensions;

        //图形队列
        uint32_t queueFamilyIndex_graphics = VK_QUEUE_FAMILY_IGNORED;
        VkQueue queue_graphics = VK_NULL_HANDLE;

        //显示队列
        uint32_t queueFamilyIndex_presentation  = VK_QUEUE_FAMILY_IGNORED;
        VkQueue queue_presentation = VK_NULL_HANDLE;

        //计算队列
        uint32_t queueFamilyIndex_compute  = VK_QUEUE_FAMILY_IGNORED;
        VkQueue queue_compute = VK_NULL_HANDLE;

        VkDevice device = VK_NULL_HANDLE;

        std::vector <VkSurfaceFormatKHR> availableSurfaceFormats;

        //交换链
        VkSwapchainKHR swapchain = VK_NULL_HANDLE;
        std::vector <VkImage> swapchainImages;
        std::vector <VkImageView> swapchainImageViews;
        //保存交换链的创建信息以便重建交换链
        VkSwapchainCreateInfoKHR swapchainCreateInfo = {};
        //增删交换链回调函数
        std::vector<std::function<void()>> callbacks_createSwapchain;
        std::vector<std::function<void()>> callbacks_destroySwapchain;
        //增删（逻辑）设备回调函数
        std::vector<std::function<void()>> callbacks_createDevice;
        std::vector<std::function<void()>> callbacks_destroyDevice;
        //当前取得的交换链图像索引
        uint32_t currentImageIndex = 0;

        //pimpl定义的加类
        graphicsBasePlus* pPlus = nullptr;//=nullptr可以省略，因为是单例类的成员，自动零初始化

        //向层或拓展容器添加字符串指针
        static void AddLayerOrExtension(std::vector<const char*>&container,const char* name);

        result_t CreateDebugMessenger();

        //该函数被DeterminePhysicalDevice(...)调用，用于检查物理设备是否满足所需的队列族类型，并将对应的队列族索引返回到queueFamilyIndices，执行成功时直接将索引写入相应成员变量
        result_t GetQueueFamilyIndices(VkPhysicalDevice physicalDevice, bool enableGraphicsQueue, bool enableComputeQueue, uint32_t (&queueFamilyIndices)[3]);

        //该函数被CreateSwapchain(...)和RecreateSwapchain()调用
        result_t CreateSwapchain_Internal();

    public:
        static graphicsBase& Base();

        // Getters
        VkInstance Instance() const;
        VkDebugUtilsMessengerEXT DebugMessenger() const;
        VkSurfaceKHR Surface() const;
        VkPhysicalDevice PhysicalDevice() const;
        const VkPhysicalDeviceProperties& PhysicalDeviceProperties() const;
        const VkPhysicalDeviceMemoryProperties& PhysicalDeviceMemoryProperties() const;
        VkPhysicalDevice AvailablePhysicalDevice(uint32_t index) const;
        uint32_t AvailablePhysicalDeviceCount() const;
        VkDevice Device() const;
        uint32_t QueueFamilyIndex_Graphics() const;
        uint32_t QueueFamilyIndex_Presentation() const;
        uint32_t QueueFamilyIndex_Compute() const;
        VkQueue Queue_Graphics() const;
        VkQueue Queue_Presentation() const;
        VkQueue Queue_Compute() const;
        const VkFormat& AvailableSurfaceFormat(uint32_t index) const;
        const VkColorSpaceKHR& AvailableSurfaceColorSpace(uint32_t index) const;
        uint32_t AvailableSurfaceFormatCount() const;
        VkSwapchainKHR Swapchain() const;
        VkImage SwapchainImage(uint32_t index) const;
        VkImageView SwapchainImageView(uint32_t index) const;
        uint32_t SwapchainImageCount() const;
        const VkSwapchainCreateInfoKHR& SwapchainCreateInfo() const;
        uint32_t ApiVersion() const;
        uint32_t CurrentImageIndex() const;
        //*pPlus的Getter
        static graphicsBasePlus& Plus() { return *singleton.pPlus; }

        //*pPlus的Setter，只允许设置pPlus一次
        static void Plus(graphicsBasePlus& plus) { if (!singleton.pPlus) singleton.pPlus = &plus; }

        // Instance related
        void AddInstanceLayer(const char* layerName);
        void AddInstanceExtension(const char* extensionName);
        result_t CreateInstance(VkInstanceCreateFlags flags = 0);
        result_t CheckInstanceLayers(arrayRef<const char*> layersToCheck);
        result_t CheckInstanceExtensions(arrayRef<const char*> extensionsToCheck, const char* layerName = nullptr) const;

        // Surface related
        void Surface(VkSurfaceKHR s);

        // Device related
        void AddDeviceExtension(const char* extensionName);
        result_t GetPhysicalDevices();
        result_t DeterminePhysicalDevice(uint32_t deviceIndex = 0, bool enableGraphicsQueue = true, bool enableComputeQueue = true);
        void AddCallback_CreateDevice(std::function<void()> function);
        void AddCallback_DestroyDevice(std::function<void()> function);
        result_t WaitIdle() const;
        result_t RecreateDevice(VkDeviceCreateFlags flags = 0);
        result_t CreateDevice(VkDeviceCreateFlags flags = 0);
        result_t CheckDeviceExtensions(arrayRef<const char*> extensionsToCheck, const char* layerName = nullptr) const;
        void DeviceExtensions(const std::vector<const char*>& extensionNames);

        // Surface format related
        result_t GetSurfaceFormats();
        result_t SetSurfaceFormat(VkSurfaceFormatKHR surfaceFormat);

        // Swapchain related
        void AddCallback_CreateSwapchain(std::function<void()> function);
        void AddCallback_DestroySwapchain(std::function<void()> function);
        result_t CreateSwapchain(bool limitFrameRate = true, VkSwapchainCreateFlagsKHR flags = 0);
        result_t RecreateSwapchain();
        result_t AcquireNextImage(VkSemaphore presentCompleteSemaphore, VkFence fence = VK_NULL_HANDLE);

        //use api
        VkResult UseLatestApiVersion();

        result_t SwapImage(VkSemaphore semaphore_imageIsAvailable);

        result_t SubmitCommandBuffer_Graphics(VkSubmitInfo &submitInfo, VkFence fence) const;

        result_t SubmitCommandBuffer_Graphics(VkCommandBuffer commandBuffer, VkSemaphore semaphore_imageIsAvailable,
                                              VkSemaphore semaphore_renderingIsOver, VkFence fence,
                                              VkPipelineStageFlags waitDstStage_imageIsAvailable) const;

        result_t SubmitCommandBuffer_Graphics(VkCommandBuffer commandBuffer, VkFence fence) const;

        result_t SubmitCommandBuffer_Compute(VkSubmitInfo &submitInfo, VkFence fence) const;

        result_t SubmitCommandBuffer_Compute(VkCommandBuffer commandBuffer, VkFence fence) const;

        result_t PresentImage(VkPresentInfoKHR &presentInfo);

        result_t PresentImage(VkSemaphore semaphore_renderingIsOver);

        void CmdTransferImageOwnership(VkCommandBuffer commandBuffer) const;

        result_t SubmitCommandBuffer_Presentation(VkCommandBuffer commandBuffer, VkSemaphore semaphore_renderingIsOver,
                                                  VkSemaphore semaphore_ownershipIsTransfered, VkFence fence) const;
    }; // end class graphicsBase

} // namespace myVulkan
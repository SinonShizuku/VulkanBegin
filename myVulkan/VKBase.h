//
// Created by Jhong on 2025/4/27.
//

#pragma once
#include "VKBase.hpp"

//定义myVulkan命名空间
namespace myVulkan {

    graphicsBase graphicsBase::singleton;

    graphicsBase::graphicsBase() = default;
    graphicsBase::~graphicsBase(){
        if (!instance)
            return;

        //销毁交换链和逻辑设备
        if (device) {
            WaitIdle(); //等待逻辑设备空闲
            if (swapchain) {
                for (auto& i : callbacks_destroySwapchain)
                    i();
                for (auto& i : swapchainImageViews)
                    if (i)
                        vkDestroyImageView(device, i, nullptr);
                vkDestroySwapchainKHR(device, swapchain, nullptr);
            }
            for (auto& i : callbacks_destroyDevice)
                i();
            vkDestroyDevice(device, nullptr);
        }

        //销毁surface
        if (surface)
            vkDestroySurfaceKHR(instance, surface, nullptr);

        //销毁debugMessenger
        if (debugMessenger) {
            PFN_vkDestroyDebugUtilsMessengerEXT DestroyDebugUtilsMessenger =
                    reinterpret_cast<PFN_vkDestroyDebugUtilsMessengerEXT>(vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT"));
            if (DestroyDebugUtilsMessenger)
                DestroyDebugUtilsMessenger(instance, debugMessenger, nullptr);
        }

        //销毁Vulkan实例
        vkDestroyInstance(instance, nullptr);
    }

    //向层或拓展容器添加字符串指针
    void graphicsBase::AddLayerOrExtension(std::vector<const char*>&container,const char* name){
        for(auto&i:container)
        {
            if(!strcmp(name,i)) return;
        }
        container.push_back(name);
    }

    result_t graphicsBase::CreateDebugMessenger()
    {
        //定义lambda回调函数
        static PFN_vkDebugUtilsMessengerCallbackEXT DebugUtilsMessengerCallback = [](
                VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
                VkDebugUtilsMessageTypeFlagsEXT messageTypes,
                const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
                void* pUserData)->VkBool32 {
            outStream << std::format("{}\n\n", pCallbackData->pMessage);
            return VK_FALSE;
        };

        //dubug messenger创建信息
        VkDebugUtilsMessengerCreateInfoEXT debugUtilsMessengerCreateInfo  = {
                .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT,
                .messageSeverity =
                        VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
                        VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT,
                .messageType =
                        VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
                        VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
                        VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT,
                .pfnUserCallback = DebugUtilsMessengerCallback
        };

        //显式通过vkGetInstanceProcAddr加载vkCreateDebugUtilsMessengerEXT函数
        PFN_vkCreateDebugUtilsMessengerEXT vkCreateDebugUtilsMessenger =
                reinterpret_cast<PFN_vkCreateDebugUtilsMessengerEXT>(vkGetInstanceProcAddr(instance,"vkCreateDebugUtilsMessengerEXT"));
        if(vkCreateDebugUtilsMessenger){
            result_t result = vkCreateDebugUtilsMessenger(instance,&debugUtilsMessengerCreateInfo, nullptr,&debugMessenger);
            if(result)
                outStream << std::format("[ graphicsBase ] ERROR\nFailed to create a debug messenger!\nError code: {}\n", int32_t(result));
            return result;
        }
        outStream << std::format("[ graphicsBase ] ERROR\nFailed to get the function pointer of vkCreateDebugUtilsMessengerEXT!\n");
        return VK_RESULT_MAX_ENUM; //因没有合适的错误代码而返回VK_RESULT_MAX_ENUM，即INT32_MAX

    }

    //该函数被DeterminePhysicalDevice(...)调用，用于检查物理设备是否满足所需的队列族类型，并将对应的队列族索引返回到queueFamilyIndices，执行成功时直接将索引写入相应成员变量
    result_t graphicsBase::GetQueueFamilyIndices(VkPhysicalDevice physicalDevice, bool enableGraphicsQueue, bool enableComputeQueue, uint32_t (&queueFamilyIndices)[3]) {
        uint32_t queueFamilyCount = 0;
        vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, nullptr);
        if (!queueFamilyCount)
            return VK_RESULT_MAX_ENUM;
        std::vector<VkQueueFamilyProperties> queueFamilyPropertieses(queueFamilyCount);
        vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, queueFamilyPropertieses.data());
        auto& [ig, ip, ic] = queueFamilyIndices;
        ig = ip = ic = VK_QUEUE_FAMILY_IGNORED;
        for (uint32_t i = 0; i < queueFamilyCount; i++) {
            //这三个VkBool32变量指示是否可获取（指应该被获取且能获取）相应队列族索引
            VkBool32
                //只在enableGraphicsQueue为true时获取支持图形操作的队列族的索引
                supportGraphics = enableGraphicsQueue && queueFamilyPropertieses[i].queueFlags & VK_QUEUE_GRAPHICS_BIT,
                supportPresentation = false,
                //只在enableComputeQueue为true时获取支持计算的队列族的索引
                supportCompute = enableComputeQueue && queueFamilyPropertieses[i].queueFlags & VK_QUEUE_COMPUTE_BIT;
                //只在创建了window surface时获取支持呈现的队列族的索引
            if (surface)
                if (VkResult result = vkGetPhysicalDeviceSurfaceSupportKHR(physicalDevice, i, surface, &supportPresentation)) {
                    std::cout << std::format("[ graphicsBase ] ERROR\nFailed to determine if the queue family supports presentation!\nError code: {}\n", int32_t(result));
                    return result;
                }
            //若某队列族同时支持图形操作和计算
            if (supportGraphics && supportCompute) {
                //若需要呈现，最好是三个队列族索引全部相同
                if (supportPresentation) {
                    ig = ip = ic = i;
                    break;
                }
                //除非ig和ic都已取得且相同，否则将它们的值覆写为i，以确保两个队列族索引相同
                if (ig != ic ||
                    ig == VK_QUEUE_FAMILY_IGNORED)
                    ig = ic = i;
                //如果不需要呈现，那么已经可以break了
                if (!surface)
                    break;
            }
            //若任何一个队列族索引可以被取得但尚未被取得，将其值覆写为i
            if (supportGraphics &&
                ig == VK_QUEUE_FAMILY_IGNORED)
                ig = i;
            if (supportPresentation &&
                ip == VK_QUEUE_FAMILY_IGNORED)
                ip = i;
            if (supportCompute &&
                ic == VK_QUEUE_FAMILY_IGNORED)
                ic = i;
        }
        if (ig == VK_QUEUE_FAMILY_IGNORED && enableGraphicsQueue ||
            ip == VK_QUEUE_FAMILY_IGNORED && surface ||
            ic == VK_QUEUE_FAMILY_IGNORED && enableComputeQueue)
            return VK_RESULT_MAX_ENUM;
        queueFamilyIndex_graphics = ig;
        queueFamilyIndex_presentation = ip;
        queueFamilyIndex_compute = ic;
        return VK_SUCCESS;
    }

    //该函数被CreateSwapchain(...)和RecreateSwapchain()调用
    result_t graphicsBase::CreateSwapchain_Internal() {
        //根据已有信息创建交换链
        if (result_t result = vkCreateSwapchainKHR(device, &swapchainCreateInfo, nullptr, &swapchain)) {
            outStream << std::format("[ graphicsBase ] ERROR\nFailed to create a swapchain!\nError code: {}\n", int32_t(result));
            return result;
        }
        //获取交换链图像
        uint32_t swapchainImageCount;
        if (result_t result = vkGetSwapchainImagesKHR(device, swapchain, &swapchainImageCount, nullptr)) {
            outStream << std::format("[ graphicsBase ] ERROR\nFailed to get the count of swapchain images!\nError code: {}\n", int32_t(result));
            return result;
        }
        swapchainImages.resize(swapchainImageCount);
        if (result_t result = vkGetSwapchainImagesKHR(device, swapchain, &swapchainImageCount, swapchainImages.data())) {
            outStream << std::format("[ graphicsBase ] ERROR\nFailed to get swapchain images!\nError code: {}\n", int32_t(result));
            return result;
        }
        //创建image view
        swapchainImageViews.resize(swapchainImageCount);
        VkImageViewCreateInfo imageViewCreateInfo = {
                .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
                .viewType = VK_IMAGE_VIEW_TYPE_2D,
                .format = swapchainCreateInfo.imageFormat,
                //.components = {},//四个成员皆为VK_COMPONENT_SWIZZLE_IDENTITY
                .subresourceRange = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 }
        };
        for (size_t i = 0; i < swapchainImageCount; i++) {
            imageViewCreateInfo.image = swapchainImages[i];
            if (result_t result = vkCreateImageView(device, &imageViewCreateInfo, nullptr, &swapchainImageViews[i])) {
                outStream << std::format("[ graphicsBase ] ERROR\nFailed to create a swapchain image view!\nError code: {}\n", int32_t(result));
                return result;
            }
        }
        return VK_SUCCESS;
    }

    //Static Function
    graphicsBase& graphicsBase::Base()
    {
        return singleton;
    }
    //Getter
    VkInstance graphicsBase::Instance() const {
        return instance;
    }
    VkDebugUtilsMessengerEXT graphicsBase::DebugMessenger() const {
        return debugMessenger;
    }
    VkSurfaceKHR graphicsBase::Surface() const {
        return surface;
    }
    VkPhysicalDevice graphicsBase::PhysicalDevice() const {
        return physicalDevice;
    }
    const VkPhysicalDeviceProperties& graphicsBase::PhysicalDeviceProperties() const {
        return physicalDeviceProperties;
    }
    const VkPhysicalDeviceMemoryProperties& graphicsBase::PhysicalDeviceMemoryProperties() const {
        return physicalDeviceMemoryProperties;
    }
    VkPhysicalDevice graphicsBase::AvailablePhysicalDevice(uint32_t index) const {
        return availablePhysicalDevices[index];
    }
    uint32_t graphicsBase::AvailablePhysicalDeviceCount() const {
        return (uint32_t)availablePhysicalDevices.size();
    }
    VkDevice graphicsBase::Device() const {
        return device;
    }
    uint32_t graphicsBase::QueueFamilyIndex_Graphics() const {
        return queueFamilyIndex_graphics;
    }
    uint32_t graphicsBase::QueueFamilyIndex_Presentation() const {
        return queueFamilyIndex_presentation;
    }
    uint32_t graphicsBase::QueueFamilyIndex_Compute() const {
        return queueFamilyIndex_compute;
    }
    VkQueue graphicsBase::Queue_Graphics() const {
        return queue_graphics;
    }
    VkQueue graphicsBase::Queue_Presentation() const {
        return queue_presentation;
    }
    VkQueue graphicsBase::Queue_Compute() const {
        return queue_compute;
    }
    const VkFormat& graphicsBase::AvailableSurfaceFormat(uint32_t index) const {
        return availableSurfaceFormats[index].format;
    }
    const VkColorSpaceKHR& graphicsBase::AvailableSurfaceColorSpace(uint32_t index) const {
        return availableSurfaceFormats[index].colorSpace;
    }
    uint32_t graphicsBase::AvailableSurfaceFormatCount() const {
        return (uint32_t)availableSurfaceFormats.size();
    }
    VkSwapchainKHR graphicsBase::Swapchain() const {
        return swapchain;
    }
    VkImage graphicsBase::SwapchainImage(uint32_t index) const {
        return swapchainImages[index];
    }
    VkImageView graphicsBase::SwapchainImageView(uint32_t index) const {
        return swapchainImageViews[index];
    }
    uint32_t graphicsBase::SwapchainImageCount() const {
        return (uint32_t)swapchainImageViews.size();
    }
    const VkSwapchainCreateInfoKHR& graphicsBase::SwapchainCreateInfo() const {
        return swapchainCreateInfo;
    }
    uint32_t graphicsBase::ApiVersion() const {
        return apiVersion;
    }
    uint32_t graphicsBase::CurrentImageIndex() const { return currentImageIndex; }

    //Setter
    //Instance related
    void graphicsBase::AddInstanceLayer(const char* layerName) {
        AddLayerOrExtension(instanceLayers, layerName);
    }
    void graphicsBase::AddInstanceExtension(const char* extensionName) {
        AddLayerOrExtension(instanceExtensions, extensionName);
    }
    //该函数仅被CreateInstance(...)调用
    result_t graphicsBase::CreateInstance(VkInstanceCreateFlags flags) {
        if constexpr (ENABLE_DEBUG_MESSENGER)
            AddInstanceLayer("VK_LAYER_KHRONOS_validation"),
            AddInstanceExtension(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);

        //创建应用程序信息，不关键
        VkApplicationInfo applicationInfo = {
                .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
                .apiVersion = apiVersion
        };

        //Vulkan实例创建信息
        VkInstanceCreateInfo instanceCreateInfo = {
                .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
                .flags = flags,
                .pApplicationInfo = &applicationInfo,
                .enabledLayerCount = (uint32_t)instanceLayers.size(),
                .ppEnabledLayerNames = instanceLayers.data(),
                .enabledExtensionCount = (uint32_t)instanceExtensions.size(),
                .ppEnabledExtensionNames = instanceExtensions.data()
        };

        //根据info创建Vulkan实例
        if (result_t result = vkCreateInstance(&instanceCreateInfo, nullptr, &instance)) {
            outStream << std::format("[ graphicsBase ] ERROR\nFailed to create an instance!\nError code: {}\n", int32_t(result));
            return result;
        }

        outStream << std::format(
            "Vulkan API Version: {}.{}.{}\n",
            VK_VERSION_MAJOR(apiVersion),
            VK_VERSION_MINOR(apiVersion),
            VK_VERSION_PATCH(apiVersion)
        );

        //创建debug messenger
        if constexpr (ENABLE_DEBUG_MESSENGER)
            return CreateDebugMessenger();
        return VK_SUCCESS;
    }

    //创建实例失败后
    result_t graphicsBase::CheckInstanceLayers(arrayRef<const char*> layersToCheck) {
        uint32_t layerCount;
        std::vector<VkLayerProperties> availableLayers;
        if (VkResult result = vkEnumerateInstanceLayerProperties(&layerCount, nullptr)) {
            std::cout << std::format("[ graphicsBase ] ERROR\nFailed to get the count of instance layers!\n");
            return result;
        }
        if (layerCount) {
            availableLayers.resize(layerCount);
            if (VkResult result = vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data())) {
                std::cout << std::format("[ graphicsBase ] ERROR\nFailed to enumerate instance layer properties!\nError code: {}\n", int32_t(result));
                return result;
            }
            for (auto& i : layersToCheck) {
                bool found = false;
                for (auto& j : availableLayers)
                    if (!strcmp(i, j.layerName)) {
                        found = true;
                        break;
                    }
                if (!found)
                    i = nullptr;
            }
        }
        else
            for (auto& i : layersToCheck)
                i = nullptr;
        //一切顺利则返回VK_SUCCESS
        return VK_SUCCESS;
    }

    result_t graphicsBase::CheckInstanceExtensions(arrayRef<const char*> extensionsToCheck, const char* layerName) const {
        uint32_t extensionCount;
        std::vector<VkExtensionProperties> availableExtensions;
        if (VkResult result = vkEnumerateInstanceExtensionProperties(layerName, &extensionCount, nullptr)) {
            layerName ?
            std::cout << std::format("[ graphicsBase ] ERROR\nFailed to get the count of instance extensions!\nLayer name:{}\n", layerName) :
            std::cout << std::format("[ graphicsBase ] ERROR\nFailed to get the count of instance extensions!\n");
            return result;
        }
        if (extensionCount) {
            availableExtensions.resize(extensionCount);
            if (VkResult result = vkEnumerateInstanceExtensionProperties(layerName, &extensionCount, availableExtensions.data())) {
                std::cout << std::format("[ graphicsBase ] ERROR\nFailed to enumerate instance extension properties!\nError code: {}\n", int32_t(result));
                return result;
            }
            for (auto& i : extensionsToCheck) {
                bool found = false;
                for (auto& j : availableExtensions)
                    if (!strcmp(i, j.extensionName)) {
                        found = true;
                        break;
                    }
                if (!found)
                    i = nullptr;
            }
        }
        else
            for (auto& i : extensionsToCheck)
                i = nullptr;
        return VK_SUCCESS;
    }
    //Surface related
    void graphicsBase::Surface(VkSurfaceKHR s) {
        if (surface)
            vkDestroySurfaceKHR(instance, surface, nullptr);
        surface = s;
    }
    //Device related
    void graphicsBase::AddDeviceExtension(const char* extensionName) {
        AddLayerOrExtension(deviceExtensions, extensionName);
    }
    result_t graphicsBase::GetPhysicalDevices() {
        uint32_t physicalDeviceCount;
        if (result_t result = vkEnumeratePhysicalDevices(instance, &physicalDeviceCount, nullptr)) {
            outStream << std::format("[ graphicsBase ] ERROR\nFailed to get the count of physical devices!\nError code: {}\n", int32_t(result));
            return result;
        }
        if (!physicalDeviceCount)
            return VK_ERROR_INITIALIZATION_FAILED;
        availablePhysicalDevices.resize(physicalDeviceCount);
        if (result_t result = vkEnumeratePhysicalDevices(instance, &physicalDeviceCount, availablePhysicalDevices.data())) {
            outStream << std::format("[ graphicsBase ] ERROR\nFailed to enumerate physical devices!\nError code: {}\n", int32_t(result));
            return result;
        }
        return VK_SUCCESS;
    }

    result_t graphicsBase::DeterminePhysicalDevice(uint32_t deviceIndex, bool enableGraphicsQueue, bool enableComputeQueue) {
        //定义一个特殊值用于标记一个队列族索引已被找过但未找到
        static constexpr uint32_t notFound = INT32_MAX;//== VK_QUEUE_FAMILY_IGNORED & INT32_MAX
        //定义队列族索引组合的结构体
        struct queueFamilyIndexCombination {
            uint32_t graphics = VK_QUEUE_FAMILY_IGNORED;
            uint32_t presentation = VK_QUEUE_FAMILY_IGNORED;
            uint32_t compute = VK_QUEUE_FAMILY_IGNORED;
        };
        //queueFamilyIndexCombinations用于为各个物理设备保存一份队列族索引组合
        static std::vector<queueFamilyIndexCombination> queueFamilyIndexCombinations(availablePhysicalDevices.size());
        auto& [ig, ip, ic] = queueFamilyIndexCombinations[deviceIndex];

        //如果有任何队列族索引已被找过但未找到，返回VK_RESULT_MAX_ENUM
        if (ig == notFound && enableGraphicsQueue ||
            ip == notFound && surface ||
            ic == notFound && enableComputeQueue)
            return VK_RESULT_MAX_ENUM;

        //如果有任何队列族索引应被获取但还未被找过
        if (ig == VK_QUEUE_FAMILY_IGNORED && enableGraphicsQueue ||
            ip == VK_QUEUE_FAMILY_IGNORED && surface ||
            ic == VK_QUEUE_FAMILY_IGNORED && enableComputeQueue) {
            uint32_t indices[3];
            VkResult result = GetQueueFamilyIndices(availablePhysicalDevices[deviceIndex], enableGraphicsQueue, enableComputeQueue, indices);
            //若GetQueueFamilyIndices(...)返回VK_SUCCESS或VK_RESULT_MAX_ENUM（vkGetPhysicalDeviceSurfaceSupportKHR(...)执行成功但没找齐所需队列族），
            //说明对所需队列族索引已有结论，保存结果到queueFamilyIndexCombinations[deviceIndex]中相应变量
            //应被获取的索引若仍为VK_QUEUE_FAMILY_IGNORED，说明未找到相应队列族，VK_QUEUE_FAMILY_IGNORED（~0u）与INT32_MAX做位与得到的数值等于notFound
            if (result == VK_SUCCESS ||
                result == VK_RESULT_MAX_ENUM) {
                if (enableGraphicsQueue)
                    ig = indices[0] & INT32_MAX;
                if (surface)
                    ip = indices[1] & INT32_MAX;
                if (enableComputeQueue)
                    ic = indices[2] & INT32_MAX;
            }
            //如果GetQueueFamilyIndices(...)执行失败，return
            if (result)
                return result;
        }

        //若以上两个if分支皆不执行，则说明所需的队列族索引皆已被获取，从queueFamilyIndexCombinations[deviceIndex]中取得索引
        else {
            queueFamilyIndex_graphics = enableGraphicsQueue ? ig : VK_QUEUE_FAMILY_IGNORED;
            queueFamilyIndex_presentation = surface ? ip : VK_QUEUE_FAMILY_IGNORED;
            queueFamilyIndex_compute = enableComputeQueue ? ic : VK_QUEUE_FAMILY_IGNORED;
        }
        physicalDevice = availablePhysicalDevices[deviceIndex];
        return VK_SUCCESS;
    }
    void graphicsBase::AddCallback_CreateDevice(std::function<void()> function) {
        callbacks_createDevice.push_back(function);
    }
    void graphicsBase::AddCallback_DestroyDevice(std::function<void()> function) {
        callbacks_destroyDevice.push_back(function);
    }
    result_t graphicsBase::WaitIdle() const {
        if (!device) return VK_SUCCESS;
        if (result_t result = vkDeviceWaitIdle(device)) {
            outStream << std::format("[ graphicsBase ] ERROR\nFailed to wait for the logical device to become idle!\nError code: {}\n", int32_t(result));
            return result;
        }
        return VK_SUCCESS;
    }
    result_t graphicsBase::RecreateDevice(VkDeviceCreateFlags flags) {
        if (VkResult result = WaitIdle())
            return result;
        if (swapchain) {
            for (auto& i : callbacks_destroySwapchain)
                i();
            for (auto& i : swapchainImageViews)
                if (i)
                    vkDestroyImageView(device, i, nullptr);
            swapchainImageViews.resize(0);
            vkDestroySwapchainKHR(device, swapchain, nullptr);
            swapchain = VK_NULL_HANDLE;
            swapchainCreateInfo = {};
        }
        for (auto& i : callbacks_destroyDevice)
            i();
        if (device)
            vkDestroyDevice(device, nullptr),
            device = VK_NULL_HANDLE;
        return CreateDevice(flags);

    }
    result_t graphicsBase::CreateDevice(VkDeviceCreateFlags flags) {
			float queuePriority = 1.f;
			VkDeviceQueueCreateInfo queueCreateInfos[3] = {
				{
					.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
					.queueCount = 1,
					.pQueuePriorities = &queuePriority },
				{
					.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
					.queueCount = 1,
					.pQueuePriorities = &queuePriority },
				{
					.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
					.queueCount = 1,
					.pQueuePriorities = &queuePriority } };
			uint32_t queueCreateInfoCount = 0;
			if (queueFamilyIndex_graphics != VK_QUEUE_FAMILY_IGNORED)
				queueCreateInfos[queueCreateInfoCount++].queueFamilyIndex = queueFamilyIndex_graphics;
			if (queueFamilyIndex_presentation != VK_QUEUE_FAMILY_IGNORED &&
				queueFamilyIndex_presentation != queueFamilyIndex_graphics)
				queueCreateInfos[queueCreateInfoCount++].queueFamilyIndex = queueFamilyIndex_presentation;
			if (queueFamilyIndex_compute != VK_QUEUE_FAMILY_IGNORED &&
				queueFamilyIndex_compute != queueFamilyIndex_graphics &&
				queueFamilyIndex_compute != queueFamilyIndex_presentation)
				queueCreateInfos[queueCreateInfoCount++].queueFamilyIndex = queueFamilyIndex_compute;
			VkPhysicalDeviceFeatures physicalDeviceFeatures;
			vkGetPhysicalDeviceFeatures(physicalDevice, &physicalDeviceFeatures);
			VkDeviceCreateInfo deviceCreateInfo = {
				.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
				.flags = flags,
				.queueCreateInfoCount = queueCreateInfoCount,
				.pQueueCreateInfos = queueCreateInfos,
				.enabledExtensionCount = uint32_t(deviceExtensions.size()),
				.ppEnabledExtensionNames = deviceExtensions.data(),
				.pEnabledFeatures = &physicalDeviceFeatures
			};
			if (VkResult result = vkCreateDevice(physicalDevice, &deviceCreateInfo, nullptr, &device)) {
				outStream << std::format("[ graphicsBase ] ERROR\nFailed to create a vulkan logical device!\nError code: {}\n", int32_t(result));
				return result;
			}
			if (queueFamilyIndex_graphics != VK_QUEUE_FAMILY_IGNORED)
				vkGetDeviceQueue(device, queueFamilyIndex_graphics, 0, &queue_graphics);
			if (queueFamilyIndex_presentation != VK_QUEUE_FAMILY_IGNORED)
				vkGetDeviceQueue(device, queueFamilyIndex_presentation, 0, &queue_presentation);
			if (queueFamilyIndex_compute != VK_QUEUE_FAMILY_IGNORED)
				vkGetDeviceQueue(device, queueFamilyIndex_compute, 0, &queue_compute);
			vkGetPhysicalDeviceProperties(physicalDevice, &physicalDeviceProperties);
			vkGetPhysicalDeviceMemoryProperties(physicalDevice, &physicalDeviceMemoryProperties);
			outStream << std::format("Renderer: {}\n", physicalDeviceProperties.deviceName);
			return VK_SUCCESS;
	}

    result_t graphicsBase::CheckDeviceExtensions(arrayRef<const char*> extensionsToCheck, const char* layerName) const {
			uint32_t extensionCount;
			std::vector<VkExtensionProperties> availableExtensions;
			if (VkResult result = vkEnumerateDeviceExtensionProperties(physicalDevice, layerName, &extensionCount, nullptr)) {
				layerName ?
					outStream << std::format("[ graphicsBase ] ERROR\nFailed to get the count of device extensions!\nLayer name:{}\n", layerName) :
					outStream << std::format("[ graphicsBase ] ERROR\nFailed to get the count of device extensions!\n");
				return result;
			}
			if (extensionCount) {
				availableExtensions.resize(extensionCount);
				if (VkResult result = vkEnumerateDeviceExtensionProperties(physicalDevice, layerName, &extensionCount, availableExtensions.data())) {
					outStream << std::format("[ graphicsBase ] ERROR\nFailed to enumerate device extension properties!\nError code: {}\n", int32_t(result));
					return result;
				}
				for (auto& i : extensionsToCheck) {
					bool found = false;
					for (auto& j : availableExtensions)
						if (!strcmp(i, j.extensionName)) {
							found = true;
							break;
						}
					if (!found)
						i = nullptr;//If a required extension isn't available, set it to nullptr
				}
			}
			else
				for (auto& i : extensionsToCheck)
					i = nullptr;
			return VK_SUCCESS;
	}
    void graphicsBase::DeviceExtensions(const std::vector<const char*>& extensionNames) {
        deviceExtensions = extensionNames;
    }

    //Surface format related
    result_t graphicsBase::GetSurfaceFormats() {
        if (!physicalDevice || !surface) return VK_RESULT_MAX_ENUM;
        uint32_t formatCount;
        if (result_t result = vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &formatCount, nullptr)) {
            outStream << std::format("[ graphicsBase ] ERROR\nFailed to get the count of surface formats!\nError code: {}\n", int32_t(result));
            return result;
        }
        if (!formatCount) return VK_RESULT_MAX_ENUM;
        availableSurfaceFormats.resize(formatCount);
        if (result_t result = vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &formatCount, availableSurfaceFormats.data())) {
            outStream << std::format("[ graphicsBase ] ERROR\nFailed to get surface formats!\nError code: {}\n", int32_t(result));
            return result;
        }
        return VK_SUCCESS;
    }
    result_t graphicsBase::SetSurfaceFormat(VkSurfaceFormatKHR surfaceFormat) {
        bool formatIsAvailable = false;
        if (!surfaceFormat.format) {
            for (auto& i : availableSurfaceFormats)
                if (i.colorSpace == surfaceFormat.colorSpace) {
                    swapchainCreateInfo.imageFormat = i.format;
                    swapchainCreateInfo.imageColorSpace = i.colorSpace;
                    formatIsAvailable = true;
                    break;
                }
        }
        else
            for (auto& i : availableSurfaceFormats)
                if (i.format == surfaceFormat.format &&
                    i.colorSpace == surfaceFormat.colorSpace) {
                    swapchainCreateInfo.imageFormat = i.format;
                    swapchainCreateInfo.imageColorSpace = i.colorSpace;
                    formatIsAvailable = true;
                    break;
                }
        if (!formatIsAvailable)
            return VK_ERROR_FORMAT_NOT_SUPPORTED;
        if (swapchain)
            return RecreateSwapchain();
        return VK_SUCCESS;
    }

    //Swapchain related
    void graphicsBase::AddCallback_CreateSwapchain(std::function<void()> function) {
        callbacks_createSwapchain.push_back(function);
    }
    void graphicsBase::AddCallback_DestroySwapchain(std::function<void()> function) {
        callbacks_destroySwapchain.push_back(function);
    }

    //设置交换链信息，并调用CreateSwapchain_Internal()创建交换链
    result_t graphicsBase::CreateSwapchain(bool limitFrameRate, VkSwapchainCreateFlagsKHR flags) {
        //查询物理设备duisurface的支持
        VkSurfaceCapabilitiesKHR surfaceCapabilities = {};
        if (VkResult result = vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, surface, &surfaceCapabilities)) {
            outStream << std::format("[ graphicsBase ] ERROR\nFailed to get physical device surface capabilities!\nError code: {}\n", int32_t(result));
            return result;
        }
        //设置 image count
        swapchainCreateInfo.minImageCount = surfaceCapabilities.minImageCount + (surfaceCapabilities.maxImageCount > surfaceCapabilities.minImageCount);
        //设置 image extent
        swapchainCreateInfo.imageExtent =
                surfaceCapabilities.currentExtent.width == -1 ?
                VkExtent2D{
                        glm::clamp(defaultWindowSize.width, surfaceCapabilities.minImageExtent.width, surfaceCapabilities.maxImageExtent.width),
                        glm::clamp(defaultWindowSize.height, surfaceCapabilities.minImageExtent.height, surfaceCapabilities.maxImageExtent.height) } :
                surfaceCapabilities.currentExtent;
        //设置图像视点数
        swapchainCreateInfo.imageArrayLayers = 1;
        //设置 transformation
        swapchainCreateInfo.preTransform = surfaceCapabilities.currentTransform;
        //设置透明通道方式
        if (surfaceCapabilities.supportedCompositeAlpha & VK_COMPOSITE_ALPHA_INHERIT_BIT_KHR)//表示透明度的处理方式由应用程序的其他部分（Vulkan以外的部分）指定
            swapchainCreateInfo.compositeAlpha = VK_COMPOSITE_ALPHA_INHERIT_BIT_KHR;
        else
            for (size_t i = 0; i < 4; i++)
                if (surfaceCapabilities.supportedCompositeAlpha & 1 << i) {
                    swapchainCreateInfo.compositeAlpha = VkCompositeAlphaFlagBitsKHR(surfaceCapabilities.supportedCompositeAlpha & 1 << i);
                    break;
                }
        //设置图像用途
        //图像必须被用作颜色附件（VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT），最好还能被用作数据传送的目标
        swapchainCreateInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
        if (surfaceCapabilities.supportedUsageFlags & VK_IMAGE_USAGE_TRANSFER_SRC_BIT)
            swapchainCreateInfo.imageUsage |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
        if (surfaceCapabilities.supportedUsageFlags & VK_IMAGE_USAGE_TRANSFER_DST_BIT)
            swapchainCreateInfo.imageUsage |= VK_IMAGE_USAGE_TRANSFER_DST_BIT;
        else
            outStream << std::format("[ graphicsBase ] WARNING\nVK_IMAGE_USAGE_TRANSFER_DST_BIT isn't supported!\n");

        //获取图像格式与色彩空间
        if (!availableSurfaceFormats.size())
            if (VkResult result = GetSurfaceFormats())
                return result;

        //设置默认格式
        if (!swapchainCreateInfo.imageFormat)
            if (SetSurfaceFormat({ VK_FORMAT_R8G8B8A8_UNORM, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR }) &&
                SetSurfaceFormat({ VK_FORMAT_B8G8R8A8_UNORM, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR })) {
                //如果找不到上述图像格式和色彩空间的组合，那只能有什么用什么，采用availableSurfaceFormats中的第一组
                swapchainCreateInfo.imageFormat = availableSurfaceFormats[0].format;
                swapchainCreateInfo.imageColorSpace = availableSurfaceFormats[0].colorSpace;
                outStream << std::format("[ graphicsBase ] WARNING\nFailed to select a four-component UNORM surface format!\n");
            }

        //指定呈现方式
        uint32_t surfacePresentModeCount;
        if (VkResult result = vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, &surfacePresentModeCount, nullptr)) {
            outStream << std::format("[ graphicsBase ] ERROR\nFailed to get the count of surface present modes!\nError code: {}\n", int32_t(result));
            return result;
        }
        if (!surfacePresentModeCount)
            outStream << std::format("[ graphicsBase ] ERROR\nFailed to find any surface present mode!\n"),
                    abort();
        std::vector<VkPresentModeKHR> surfacePresentModes(surfacePresentModeCount);
        if (VkResult result = vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, &surfacePresentModeCount, surfacePresentModes.data())) {
            outStream << std::format("[ graphicsBase ] ERROR\nFailed to get surface present modes!\nError code: {}\n", int32_t(result));
            return result;
        }
        //默认FIFO，未限制帧率则mailbox防止画面撕裂
        swapchainCreateInfo.presentMode = VK_PRESENT_MODE_FIFO_KHR;
        if (!limitFrameRate)
            for (size_t i = 0; i < surfacePresentModeCount; i++)
                if (surfacePresentModes[i] == VK_PRESENT_MODE_MAILBOX_KHR) {
                    swapchainCreateInfo.presentMode = VK_PRESENT_MODE_MAILBOX_KHR;
                    break;
                }

        //剩余参数
        swapchainCreateInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
        swapchainCreateInfo.flags = flags;
        swapchainCreateInfo.surface = surface;
        swapchainCreateInfo.imageArrayLayers = 1;
        swapchainCreateInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        swapchainCreateInfo.clipped = VK_TRUE;

        if (VkResult result = CreateSwapchain_Internal())
            return result;
        for (auto& i : callbacks_createSwapchain)
            i();
        return VK_SUCCESS;
    }
    VkResult graphicsBase::RecreateSwapchain() {
        VkSurfaceCapabilitiesKHR surfaceCapabilities = {};
        if (VkResult result = vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, surface, &surfaceCapabilities)) {
            std::cout << std::format("[ graphicsBase ] ERROR\nFailed to get physical device surface capabilities!\nError code: {}\n", int32_t(result));
            return result;
        }
        if (surfaceCapabilities.currentExtent.width == 0 ||
            surfaceCapabilities.currentExtent.height == 0)
            return VK_SUBOPTIMAL_KHR;
        swapchainCreateInfo.imageExtent = surfaceCapabilities.currentExtent;
        swapchainCreateInfo.oldSwapchain = swapchain;
        VkResult result = vkQueueWaitIdle(queue_graphics);
        if (result == VK_SUCCESS &&
            queue_graphics != queue_presentation)
            result = vkQueueWaitIdle(queue_presentation);
        if (result) {
            std::cout << std::format("[ graphicsBase ] ERROR\nFailed to wait for the queue to be idle!\nError code: {}\n", int32_t(result));
            return result;
        }
        //销毁旧交换链相关对象
        for (auto& i : callbacks_destroySwapchain)
            i();
        for (auto& i : swapchainImageViews)
            if (i)
                vkDestroyImageView(device, i, nullptr);
        swapchainImageViews.resize(0);
        //创建新交换链及与之相关的对象
        if (VkResult result = CreateSwapchain_Internal())
            return result;
        for (auto& i : callbacks_createSwapchain)
            i();
        return VK_SUCCESS;
    }

    //获取交换链图像索引到currentImageIndex，以及在需要重建交换链时调用RecreateSwapchain()、重建交换链后销毁旧交换链
    result_t graphicsBase::SwapImage(VkSemaphore semaphore_imageIsAvailable) {
        //销毁旧交换链（若存在）
        if (swapchainCreateInfo.oldSwapchain &&
            swapchainCreateInfo.oldSwapchain != swapchain) {
            vkDestroySwapchainKHR(device, swapchainCreateInfo.oldSwapchain, nullptr);
            swapchainCreateInfo.oldSwapchain = VK_NULL_HANDLE;
        }
        //获取交换链图像索引
        while (VkResult result = vkAcquireNextImageKHR(device, swapchain, UINT64_MAX, semaphore_imageIsAvailable, VK_NULL_HANDLE, &currentImageIndex))
            switch (result) {
                case VK_SUBOPTIMAL_KHR:
                case VK_ERROR_OUT_OF_DATE_KHR:
                    if (VkResult result = RecreateSwapchain())
                        return result;
                    break; //注意重建交换链后仍需要获取图像，通过break递归，再次执行while的条件判定语句
                default:
                    outStream << std::format("[ graphicsBase ] ERROR\nFailed to acquire the next image!\nError code: {}\n", int32_t(result));
                    return result;
            }
        return VK_SUCCESS;
    }

    VkResult graphicsBase::UseLatestApiVersion() {
        if (vkGetInstanceProcAddr(VK_NULL_HANDLE, "vkEnumerateInstanceVersion"))
            return vkEnumerateInstanceVersion(&apiVersion);
        return VK_SUCCESS;
    }

    //用于将命令缓冲区提交到用于图形的队列
    result_t graphicsBase::SubmitCommandBuffer_Graphics(VkSubmitInfo& submitInfo, VkFence fence = VK_NULL_HANDLE) const {
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        VkResult result = vkQueueSubmit(queue_graphics, 1, &submitInfo, fence);
        if (result)
            outStream << std::format("[ graphicsBase ] ERROR\nFailed to submit the command buffer!\nError code: {}\n", int32_t(result));
        return result;
    }

    //重构SubmitCommandBuffer_Graphics函数，处理渲染循环中将命令缓冲区提交到图形队列的常见情形
    result_t graphicsBase::SubmitCommandBuffer_Graphics(VkCommandBuffer commandBuffer,
                                                        VkSemaphore semaphore_imageIsAvailable = VK_NULL_HANDLE, VkSemaphore semaphore_renderingIsOver = VK_NULL_HANDLE, VkFence fence = VK_NULL_HANDLE,
                                                        VkPipelineStageFlags waitDstStage_imageIsAvailable = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT) const{
        VkSubmitInfo submitInfo = {
                .commandBufferCount = 1,
                .pCommandBuffers = &commandBuffer
        };

        //如果图像可用信号量不为空，则设置提交信息结构体中的信号量数量、等待信号量和等待阶段
        if(semaphore_imageIsAvailable){
            submitInfo.waitSemaphoreCount=1;
            submitInfo.pWaitSemaphores = &semaphore_imageIsAvailable;
            submitInfo.pWaitDstStageMask = &waitDstStage_imageIsAvailable;
        }
        //如果渲染完成信号量不为空，则设置提交信息结构体中的信号量数量和信号量
        if(semaphore_renderingIsOver){
            submitInfo.signalSemaphoreCount = 1;
            submitInfo.pSignalSemaphores = &semaphore_renderingIsOver;
        }
        return SubmitCommandBuffer_Graphics(submitInfo, fence);
    }

    //将命令缓冲区提交到用于图形的队列，且只使用栅栏
    result_t graphicsBase::SubmitCommandBuffer_Graphics(VkCommandBuffer commandBuffer, VkFence fence = VK_NULL_HANDLE) const{
        VkSubmitInfo submitInfo = {
                .commandBufferCount = 1,
                .pCommandBuffers = &commandBuffer
        };
        return SubmitCommandBuffer_Graphics(submitInfo, fence);
    }

    //用于将命令缓冲区提交到用于计算的队列
    result_t graphicsBase::SubmitCommandBuffer_Compute(VkSubmitInfo& submitInfo, VkFence fence = VK_NULL_HANDLE) const{
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        VkResult result = vkQueueSubmit(queue_compute, 1, &submitInfo, fence);
        if(result){
            outStream <<std::format("[ graphicsBase ] ERROR\nFailed to submit the command buffer!\nError code: {}\n", int32_t(result));
        }
        return result;
    }

    //将命令缓冲区提交到用于计算的队列，且只使用栅栏
    result_t graphicsBase::SubmitCommandBuffer_Compute(VkCommandBuffer commandBuffer, VkFence fence = VK_NULL_HANDLE) const{
        VkSubmitInfo submitInfo = {
                .commandBufferCount = 1,
                .pCommandBuffers = &commandBuffer
        };
        return SubmitCommandBuffer_Compute(submitInfo, fence);
    }

    result_t graphicsBase::PresentImage(VkPresentInfoKHR& presentInfo){
        presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
        switch(VkResult result = vkQueuePresentKHR(queue_presentation, &presentInfo)){
            case VK_SUCCESS:
                return VK_SUCCESS;
            // 如果返回值为VK_SUBOPTIMAL_KHR，则表示设备不支持请求的格式，但仍然可以继续使用
            case VK_SUBOPTIMAL_KHR:
            case VK_ERROR_OUT_OF_DATE_KHR:
                return RecreateSwapchain();
            default:
                outStream << std::format("[ graphicsBase ] ERROR\nFailed to queue the image for presentation!\nError code: {}\n", int32_t(result));
                return result;
        }
    }

    //该函数用于在渲染循环中呈现图像的常见情形
    result_t graphicsBase::PresentImage(VkSemaphore semaphore_renderingIsOver = VK_NULL_HANDLE) {
        VkPresentInfoKHR presentInfo = {
                .swapchainCount = 1,
                .pSwapchains = &swapchain,
                .pImageIndices = &currentImageIndex
        };
        if (semaphore_renderingIsOver)
            presentInfo.waitSemaphoreCount = 1,
                    presentInfo.pWaitSemaphores = &semaphore_renderingIsOver;
        return PresentImage(presentInfo);
    }

    //在图形队列和呈现队列之间传输图像的所有权
    void graphicsBase::CmdTransferImageOwnership(VkCommandBuffer commandBuffer) const {
        // 创建一个VkImageMemoryBarrier结构体，用于在图形队列和呈现队列之间传输图像的所有权
        VkImageMemoryBarrier imageMemoryBarrier_g2p = {
                .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER, // 结构体类型
                .srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT, // 源访问掩码
                .dstAccessMask = 0, // 目标访问掩码
                .oldLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR, // 旧布局
                .newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR, // 新布局
                .srcQueueFamilyIndex = queueFamilyIndex_graphics, // 源队列族索引
                .dstQueueFamilyIndex = queueFamilyIndex_presentation, // 目标队列族索引
                .image = swapchainImages[currentImageIndex], // 图像
                .subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1} // 子资源范围
        };
        // 使用vkCmdPipelineBarrier函数在图形队列和呈现队列之间传输图像的所有权
        vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
                             VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, 0,
                             0, nullptr, 0, nullptr, 1, &imageMemoryBarrier_g2p);
    }

    //提交命令缓冲区到队列
    result_t graphicsBase::SubmitCommandBuffer_Presentation(VkCommandBuffer commandBuffer,
                                              VkSemaphore semaphore_renderingIsOver = VK_NULL_HANDLE, VkSemaphore semaphore_ownershipIsTransfered = VK_NULL_HANDLE, VkFence fence = VK_NULL_HANDLE) const {
        static constexpr VkPipelineStageFlags waitDstStage = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;
        VkSubmitInfo submitInfo = {
                .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
                .commandBufferCount = 1,
                .pCommandBuffers = &commandBuffer
        };
        if (semaphore_renderingIsOver)
            submitInfo.waitSemaphoreCount = 1,
            submitInfo.pWaitSemaphores = &semaphore_renderingIsOver,
            submitInfo.pWaitDstStageMask = &waitDstStage;
        if (semaphore_ownershipIsTransfered)
            submitInfo.signalSemaphoreCount = 1,
                    submitInfo.pSignalSemaphores = &semaphore_ownershipIsTransfered;
        VkResult result = vkQueueSubmit(queue_presentation, 1, &submitInfo, fence);
        if (result)
            outStream << std::format("[ graphicsBase ] ERROR\nFailed to submit the presentation command buffer!\nError code: {}\n", int32_t(result));
        return result;
    }
} // namespace myVulkan

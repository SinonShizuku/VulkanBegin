//
// Created by Jhong on 2025/4/27.
//
#pragma once
//可能会用上的C++标准库
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <stack>
#include <map>
#include <unordered_map>
#include <span>
#include <memory>
#include <functional>
#include <concepts>
#include <format>
#include <chrono>
#include <numeric>
#include <numbers>
#include <cstring>
#include <stdexcept>
#include <thread>
#include <filesystem>

//GLM
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
//如果你惯用左手坐标系，在此定义GLM_FORCE_LEFT_HANDED
#include <glm1_0/glm.hpp>
#include <glm1_0/gtc/matrix_transform.hpp>

#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>

//Vulkan
#ifdef _WIN32                        //考虑平台是Windows的情况（请自行解决其他平台上的差异）
#define VK_USE_PLATFORM_WIN32_KHR    //在包含vulkan.h前定义该宏，会一并包含vulkan_win32.h和windows.h
#define NOMINMAX                     //定义该宏可避免windows.h中的min和max两个宏与标准库中的函数名冲突
#pragma comment(lib, "vulkan-1.lib") //链接编译所需的静态存根库
#endif
#include <vulkan/vulkan.h>

#ifndef NDEBUG
#define ENABLE_DEBUG_MESSENGER true
#else
#define ENABLE_DEBUG_MESSENGER false
#endif

template<std::signed_integral T>
constexpr bool Between_Closed(T min, T num, T max) {
    return ((num - min) | (max - num)) >= 0;
}

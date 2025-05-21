cmake_minimum_required(VERSION 3.28)
project(vulkan_learning)

set(CMAKE_CXX_STANDARD 20)

add_executable(vulkan_learning main.hpp
        myVulkan/EasyVKStart.h
        myVulkan/VKBase.h
        window/GlfwGeneral.hpp
        myVulkan/VKBase+.h
        myVulkan/VKUtils.h
        myVulkan/VKBase.hpp
        myVulkan/EasyVulkan.hpp
        myVulkan/VKFormat.h
        scene.h
        DrawLine.hpp
        main.cpp
)

target_link_libraries(vulkan_learning glfw vulkan)

cmake_minimum_required(VERSION 3.30)

project(Vulking)

set(Vulking_VERSION_MAJOR 0)
set(Vulking_VERSION_MINOR 0)
set(Vulking_VERSION_PATCH 1)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED true)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address -g")
# add_compile_options(-ftrivial-auto-var-init=pattern -ftrivial-auto-var-init-stop-after=100000)


file(GLOB_RECURSE SRC_FILES "src/*.cpp")

add_executable(vulking ${SRC_FILES})

find_package(Vulkan REQUIRED)
find_package(glfw3 REQUIRED)
find_package(glm REQUIRED)

target_include_directories(vulking PRIVATE "includes")
target_link_libraries(vulking Vulkan::Vulkan)
target_link_libraries(vulking glfw)
target_link_libraries(vulking glm::glm)
# add_dependencies(Vulking copy_assets)

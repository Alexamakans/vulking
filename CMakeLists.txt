cmake_minimum_required(VERSION 3.30)

project(Vulking)
add_compile_definitions(PROJECT_ROOT="${CMAKE_SOURCE_DIR}/")

set(Vulking_VERSION_MAJOR 0)
set(Vulking_VERSION_MINOR 0)
set(Vulking_VERSION_PATCH 1)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)

# Set default build type if not specified
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Debug)
endif()

set(CMAKE_CXX_FLAGS_DEBUG "-g")

# Engine source
file(GLOB_RECURSE ENGINE_SRC "src/*.cpp")
list(FILTER ENGINE_SRC EXCLUDE REGEX ".*test_.*\\.cpp$") # Exclude test_*.cpp
                                                         # from engine

# Create vulkinglib (the engine)
add_library(vulkinglib ${ENGINE_SRC})
target_include_directories(
  vulkinglib PUBLIC $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/includes>
                    $<INSTALL_INTERFACE:include>)

# Link external dependencies to the engine
find_package(Vulkan REQUIRED)
find_package(glfw3 REQUIRED)
find_package(glm REQUIRED)

target_link_libraries(vulkinglib PUBLIC Vulkan::Vulkan glfw glm::glm)

# ===========================
# === User project target ===
# ===========================

file(GLOB_RECURSE USER_SRC_FILES "user_src/*.cpp")

add_executable(vulking_user ${USER_SRC_FILES})
target_link_libraries(vulking_user PRIVATE vulkinglib)

# Allow the user project to also include headers using <vulking/...>
target_include_directories(vulking_user PRIVATE ${CMAKE_SOURCE_DIR}/includes)

# Copy assets
add_custom_command(
  TARGET vulking_user
  POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy_directory "${CMAKE_SOURCE_DIR}/assets"
          "$<TARGET_FILE_DIR:vulking_user>/assets"
  COMMENT "Copying assets directory to build output")

# =====================
# === Engine tests ====
# =====================

file(GLOB_RECURSE TEST_SRC_FILES "src/test_*.cpp")

add_executable(tests ${TEST_SRC_FILES})
include(FetchContent)
FetchContent_Declare(
  catch2 URL https://github.com/catchorg/Catch2/archive/refs/tags/v3.8.1.zip)
FetchContent_MakeAvailable(catch2)

target_link_libraries(tests PRIVATE Catch2::Catch2WithMain vulkinglib)
enable_testing()
include(Catch)
catch_discover_tests(tests)

cmake_minimum_required(VERSION 3.30)

project(Vulking)

set(Vulking_VERSION_MAJOR 0)
set(Vulking_VERSION_MINOR 0)
set(Vulking_VERSION_PATCH 1)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED true)

# Set default build type if not specified
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Debug)
endif()

# Always add debug info in Debug builds
set(CMAKE_CXX_FLAGS_DEBUG "-g")

# set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address -g")

# add_compile_options(-ftrivial-auto-var-init=pattern
# -ftrivial-auto-var-init-stop-after=100000)

set(SRC_DIR "src2")

file(GLOB_RECURSE SRC_FILES "${SRC_DIR}/*.cpp")
foreach(file ${SRC_FILES})
  if(file MATCHES ".*test_.*\\.cpp$")
    list(REMOVE_ITEM SRC_FILES ${file})
  endif()
endforeach()

find_package(Vulkan REQUIRED)
find_package(glfw3 REQUIRED)
find_package(glm REQUIRED)

add_library(vulkinglib ${SRC_FILES})
target_include_directories(vulkinglib PUBLIC "includes")
target_link_libraries(vulkinglib Vulkan::Vulkan glfw glm::glm)

add_executable(vulking ${SRC_FILES})
target_link_libraries(vulking vulkinglib)

add_custom_command(
  TARGET vulking
  POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy_directory "${CMAKE_SOURCE_DIR}/assets"
          "$<TARGET_FILE_DIR:vulking>/assets"
  COMMENT "Copying assets directory to build output")

file(GLOB_RECURSE TEST_SRC_FILES "${SRC_DIR}/test_*.cpp")
add_executable(tests ${TEST_SRC_FILES})
include(FetchContent)
FetchContent_Declare(
  catch2 URL https://github.com/catchorg/Catch2/archive/refs/tags/v3.8.1.zip)
FetchContent_MakeAvailable(catch2)
target_include_directories(tests PRIVATE ${catch2_SOURCE_DIR}/src)
target_link_libraries(tests Catch2::Catch2WithMain vulkinglib)
enable_testing()
include(CTest)
include(Catch)
catch_discover_tests(tests)

cmake_minimum_required(VERSION 3.28)

project(Mengled
    VERSION 0.1.0
    DESCRIPTION "Skyrim like 2D game"
    LANGUAGES C CXX
)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

if(CMAKE_SOURCE_DIR STREQUAL CMAKE_BINARY_DIR)
    message(FATAL_ERROR "In-source builds are not allowed. Use: cmake -B build")
endif()

if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_BUILD_TYPE Debug CACHE STRING "Choose the type of build." FORCE)
endif()

include(FetchContent)

# --------------------------------------------------------------------------------------
# Raylib
# --------------------------------------------------------------------------------------

#Disable extras
set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)
set(BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(BUILD_GAMES OFF CACHE BOOL "" FORCE)

FetchContent_Declare(
    raylib
    GIT_REPOSITORY https://github.com/raysan5/raylib
    GIT_TAG 6.0
)

FetchContent_MakeAvailable(raylib)

# --------------------------------------------------------------------------------------
# EnTT (ECS) - header-only, no build
# --------------------------------------------------------------------------------------

FetchContent_Declare(
    entt
    GIT_REPOSITORY https://github.com/skypjack/entt.git
    GIT_TAG v3.14.0
)

FetchContent_GetProperties(entt)
if(NOT entt_POPULATED)
    FetchContent_Populate(entt)
endif()

add_library(EnTT INTERFACE)
add_library(EnTT::EnTT ALIAS EnTT)

target_include_directories(EnTT INTERFACE
    ${entt_SOURCE_DIR}/src
)

# --------------------------------------------------------------------------------------
# ImGui
# --------------------------------------------------------------------------------------
FetchContent_Declare(
    imgui
    GIT_REPOSITORY https://github.com/ocornut/imgui.git
    GIT_TAG v1.92.1
)

FetchContent_GetProperties(imgui)
if(NOT imgui_POPULATED)
    FetchContent_MakeAvailable(imgui)
endif()

add_library(imgui STATIC
    ${imgui_SOURCE_DIR}/imgui.cpp
    ${imgui_SOURCE_DIR}/imgui_draw.cpp
    ${imgui_SOURCE_DIR}/imgui_tables.cpp
    ${imgui_SOURCE_DIR}/imgui_widgets.cpp
    ${imgui_SOURCE_DIR}/imgui_demo.cpp
    ${imgui_SOURCE_DIR}/misc/cpp/imgui_stdlib.cpp
)

target_include_directories(imgui PUBLIC
    ${imgui_SOURCE_DIR}
    ${imgui_SOURCE_DIR}/backends
)

target_link_libraries(imgui PUBLIC raylib)

# --------------------------------------------------------------------------------------
# rlImGui (high-level ImGui integration for raylib)
# --------------------------------------------------------------------------------------

FetchContent_Declare(
    rlimgui
    GIT_REPOSITORY https://github.com/raylib-extras/rlImGui.git
    GIT_TAG main
)

FetchContent_GetProperties(rlimgui)
if(NOT rlimgui_POPULATED)
    FetchContent_MakeAvailable(rlimgui)
endif()

add_library(rlImGui STATIC
    ${rlimgui_SOURCE_DIR}/rlImGui.cpp
)

target_include_directories(rlImGui PUBLIC
    ${rlimgui_SOURCE_DIR}
)

target_link_libraries(rlImGui PUBLIC imgui raylib)


# --------------------------------------------------------------------------------------
# Native File Dialog Extended (nfd)
# --------------------------------------------------------------------------------------

set(NFD_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(NFD_INSTALL OFF CACHE BOOL "" FORCE)

FetchContent_Declare(
    nfd
    GIT_REPOSITORY https://github.com/btzy/nativefiledialog-extended.git
    GIT_TAG v1.3.0
)

FetchContent_GetProperties(nfd)
if(NOT nfd_POPULATED)
    FetchContent_MakeAvailable(nfd)
endif()

# --------------------------------------------------------------------------------------
# Executable
# --------------------------------------------------------------------------------------

file(GLOB_RECURSE PROJECT_SOURCES CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp"
)

file(GLOB_RECURSE PROJECT_HEADERS CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.hpp"
)

add_executable(${PROJECT_NAME}
    ${PROJECT_SOURCES}
    ${PROJECT_HEADERS}
)

target_link_libraries(${PROJECT_NAME} PRIVATE
    raylib
    rlImGui
    nfd
    EnTT::EnTT
)

if(UNIX AND NOT APPLE)
    # Required by some GLFW setups
    find_package(X11 REQUIRED)
    target_link_libraries(${PROJECT_NAME} PRIVATE X11)
endif()

target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

# Organize files in VS filters using the src tree
source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}/src" FILES ${PROJECT_SOURCES} ${PROJECT_HEADERS})

# Expose project version to C++ code as APP_VERSION
target_compile_definitions(${PROJECT_NAME} PRIVATE
    APP_VERSION=\"${PROJECT_VERSION}\"
)

if(MSVC)
    target_compile_options(${PROJECT_NAME} PRIVATE /W4 /permissive- /EHsc)
else()
    target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra -Wpedantic)
endif()

if(MSVC)
    # Run with working directory set to the folder containing the .exe
    set_property(TARGET ${PROJECT_NAME} PROPERTY VS_DEBUGGER_WORKING_DIRECTORY "$<TARGET_FILE_DIR:${PROJECT_NAME}>")
endif()

# For release builds on MSVC, build as a windowed app (no console)
if(MSVC)
    set_target_properties(${PROJECT_NAME} PROPERTIES
        LINK_FLAGS_RELEASE "/SUBSYSTEM:WINDOWS /ENTRY:mainCRTStartup"
    )
endif()

# Set startup project for Visual Studio
set_property(DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}" PROPERTY VS_STARTUP_PROJECT ${PROJECT_NAME})

# set the source directory of resources
set(RES_DIR ${CMAKE_CURRENT_SOURCE_DIR}/res)

# Copy the contents of the res folder to the build directory if there is one
if(EXISTS ${RES_DIR})
  message(STATUS "Resource folder fond: ${RES_DIR}")
  add_custom_command(
    TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory ${RES_DIR} $<TARGET_FILE_DIR:${PROJECT_NAME}>
    COMMENT "Copying resources to the build directory..."
  )
else()
  # Skipping copying resources
  message(STATUS "Resource folder not found: ${RES_DIR}. Skipping sesource copy.")
endif()

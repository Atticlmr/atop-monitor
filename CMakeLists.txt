cmake_minimum_required(VERSION 3.14)
project(atop-monitor VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# FTXUI路径（可通过环境变量FTXUI_DIR覆盖）
set(FTXUI_DIR "$ENV{FTXUI_DIR}" CACHE PATH "Path to FTXUI source directory")
if(NOT FTXUI_DIR)
    set(FTXUI_DIR "/tmp/FTXUI")
endif()

# 源文件
set(SOURCES
    src/main.cpp
    src/cpu_monitor.cpp
    src/memory_monitor.cpp
    src/gpu_monitor.cpp
    src/ui/dashboard.cpp
)

# 可执行文件
add_executable(atop-monitor ${SOURCES})

target_include_directories(atop-monitor PRIVATE 
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${FTXUI_DIR}/include
)

target_link_libraries(atop-monitor PRIVATE
    ${FTXUI_DIR}/build/libftxui-component.a
    ${FTXUI_DIR}/build/libftxui-dom.a
    ${FTXUI_DIR}/build/libftxui-screen.a
    pthread
    dl
)

# 可选NVML
find_library(NVML_LIBRARY nvidia-ml 
    PATHS /usr/lib/x86_64-linux-gnu /usr/lib64
    NO_DEFAULT_PATH)

if(NVML_LIBRARY)
    message(STATUS "NVML found: ${NVML_LIBRARY}")
    target_link_libraries(atop-monitor PRIVATE ${NVML_LIBRARY})
    target_compile_definitions(atop-monitor PRIVATE HAS_NVML)
else()
    message(WARNING "NVML not found, GPU monitoring will be disabled")
endif()

# 安装配置
include(GNUInstallDirs)

install(TARGETS atop-monitor
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

# CPack配置
set(CPACK_PACKAGE_NAME "atop-monitor")
set(CPACK_PACKAGE_VERSION "${PROJECT_VERSION}")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "A terminal-based system monitor with CPU, Memory, and GPU tracking")
set(CPACK_PACKAGE_VENDOR "atop-monitor")
set(CPACK_GENERATOR "DEB;RPM;TGZ")

include(CPack)

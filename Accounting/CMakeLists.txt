cmake_minimum_required(VERSION 3.10)
project(Accounting VERSION 1.0.0 LANGUAGES CXX)

# 设置 C++ 标准
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 设置输出目录
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# 包含目录
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

# 优先使用仓库内的 nlohmann 目录（如果存在），以便在离线或受限网络环境下构建。
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/include/nlohmann")
    message(STATUS "Using bundled nlohmann/json from ${CMAKE_CURRENT_SOURCE_DIR}/nlohmann")
    # 创建一个 INTERFACE 库并把本地包含目录暴露出去，并提供一个带命名空间的别名
    add_library(nlohmann_json INTERFACE)
    target_include_directories(nlohmann_json INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/nlohmann)
    add_library(nlohmann_json::nlohmann_json ALIAS nlohmann_json)
else()
    # 查找系统安装的 nlohmann_json
    find_package(nlohmann_json 3.2.0 QUIET)

    # 如果未找到 nlohmann_json，尝试从 FetchContent 下载
    if(NOT nlohmann_json_FOUND)
        include(FetchContent)
        FetchContent_Declare(nlohmann_json
            URL https://github.com/nlohmann/json/releases/download/v3.11.2/json.tar.xz
            DOWNLOAD_NO_PROGRESS FALSE
        )
        FetchContent_MakeAvailable(nlohmann_json)
    endif()
endif()

# 核心库源文件
set(CORE_SOURCES
    src/core/account_manager.cc
)

# 管理器源文件
set(MANAGER_SOURCES
    src/managers/user_manager.cc
    src/managers/bill_manager.cc
    src/managers/budget_manager.cc
    src/managers/category_manager.cc
    src/managers/report_manager.cc
)

# 模型源文件
set(MODEL_SOURCES
    src/models/user.cc
    src/models/bill.cc
    src/models/budget.cc
    src/models/category.cc
    src/models/query_criteria.cc
    src/models/report.cc
    src/models/bill_data.cc
    src/models/budget_data.cc
)

# 存储源文件
set(STORAGE_SOURCES
    src/storage/json_storage.cc
)

# CLI 源文件
set(CLI_SOURCES
    src/cli/cli.cc
)

# 创建静态库
add_library(accounting_lib STATIC
    ${CORE_SOURCES}
    ${MANAGER_SOURCES}
    ${MODEL_SOURCES}
    ${STORAGE_SOURCES}
    ${CLI_SOURCES}
)

# 链接依赖
target_link_libraries(accounting_lib PUBLIC nlohmann_json::nlohmann_json)

# 为库设置包含目录
target_include_directories(accounting_lib PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# 创建可执行程序 - 自动化测试版本
add_executable(accounting_app src/main.cc)

# 链接库
target_link_libraries(accounting_app PRIVATE accounting_lib)

# 创建可执行程序 - 交互式 CLI 版本
add_executable(accounting_cli src/cli_main.cc)

# 链接库
target_link_libraries(accounting_cli PRIVATE accounting_lib)

# 设置编译器选项
if(MSVC)
    # Visual Studio 编译器选项
    target_compile_options(accounting_app PRIVATE /W4)
    target_compile_options(accounting_cli PRIVATE /W4)
    target_compile_options(accounting_lib PRIVATE /W4)
else()
    # GCC/Clang 编译器选项
    target_compile_options(accounting_app PRIVATE -Wall -Wextra -Wpedantic)
    target_compile_options(accounting_cli PRIVATE -Wall -Wextra -Wpedantic)
    target_compile_options(accounting_lib PRIVATE -Wall -Wextra -Wpedantic)
endif()

# 打印配置信息
message(STATUS "Project: Accounting")
message(STATUS "C++ Standard: ${CMAKE_CXX_STANDARD}")
message(STATUS "Build Type: ${CMAKE_BUILD_TYPE}")
message(STATUS "Source Directory: ${CMAKE_CURRENT_SOURCE_DIR}")
message(STATUS "Binary Directory: ${CMAKE_CURRENT_BINARY_DIR}")

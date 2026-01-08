cmake_minimum_required(VERSION 3.10)
project(mylayout C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

# LayX library tests
add_executable(test_layx
    test_layx.c
    layx.c
)

# Layout patterns test
add_executable(test_layout_patterns
    test_layout_patterns.c
    layx.c
)

# Default values test
add_executable(test_defaults
    test_defaults.c
    layx.c
)

# 设置包含目录
target_include_directories(test_layx PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_include_directories(test_layout_patterns PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_include_directories(test_defaults PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

# 设置编译选项
if(MSVC)
    target_compile_options(test_layx PRIVATE /W4)
    target_compile_options(test_layout_patterns PRIVATE /W4)
    target_compile_options(test_defaults PRIVATE /W4)
else()
    target_compile_options(test_layx PRIVATE -Wall -Wextra)
    target_compile_options(test_layout_patterns PRIVATE -Wall -Wextra)
    target_compile_options(test_defaults PRIVATE -Wall -Wextra)
endif()
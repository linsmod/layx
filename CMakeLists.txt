cmake_minimum_required(VERSION 3.10)
project(mylayout C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

# Create LayX library
add_library(layx layx.c scroll_utils.c)
target_include_directories(layx PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

# LayX library tests
add_executable(test_layx
    test_layx.c
)
target_link_libraries(test_layx layx)

# Layout patterns test
add_executable(test_layout_patterns
    test_layout_patterns.c
)
target_link_libraries(test_layout_patterns layx)

# Default values test
add_executable(test_defaults
    test_defaults.c
)
target_link_libraries(test_defaults layx)

# Block margin test
add_executable(test_block_margin
    test_block_margin.c
)
target_link_libraries(test_block_margin layx)

# Flex margin test
add_executable(test_flex_margin
    test_flex_margin.c
)
target_link_libraries(test_flex_margin layx)

# Scroll test
add_executable(test_scroll
    test_scroll.c
)
target_link_libraries(test_scroll layx)

# scroll_max test
add_executable(test_scroll_max
    test_scroll_max.c
)
target_link_libraries(test_scroll_max layx)

# Display types test
add_executable(test_display_types
    test_display_types.c
)
target_link_libraries(test_display_types layx)

# Hit test test
add_executable(test_hit_test
    test_hit_test.c
)
target_link_libraries(test_hit_test layx)

# Margin merge test
add_executable(test_margin_merge
    test_margin_merge.c
)
target_link_libraries(test_margin_merge layx)

# Destroy item test
add_executable(test_destroy
    test_destroy.c
)
target_link_libraries(test_destroy layx)

# Last child margin test
add_executable(test_last_child_margin
    test_last_child_margin.c
)
target_link_libraries(test_last_child_margin layx)

# Last child margin advanced test
add_executable(test_last_child_margin_advanced
    test_last_child_margin_advanced.c
)
target_link_libraries(test_last_child_margin_advanced layx)

# Multiple layout runs test
add_executable(test_multiple_layout_runs
    test_multiple_layout_runs.c
)
target_link_libraries(test_multiple_layout_runs layx)

# 设置包含目录
target_include_directories(test_layx PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_include_directories(test_layout_patterns PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_include_directories(test_defaults PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_include_directories(test_block_margin PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_include_directories(test_flex_margin PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_include_directories(test_scroll PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_include_directories(test_scroll_max PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_include_directories(test_display_types PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_include_directories(test_hit_test PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_include_directories(test_margin_merge PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_include_directories(test_destroy PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_include_directories(test_last_child_margin PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_include_directories(test_last_child_margin_advanced PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_include_directories(test_multiple_layout_runs PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

# 设置编译选项
if(MSVC)
    target_compile_options(test_layx PRIVATE /W4)
    target_compile_options(test_layout_patterns PRIVATE /W4)
    target_compile_options(test_defaults PRIVATE /W4)
    target_compile_options(test_block_margin PRIVATE /W4)
    target_compile_options(test_flex_margin PRIVATE /W4)
    target_compile_options(test_scroll PRIVATE /W4)
    target_compile_options(test_scroll_max PRIVATE /W4)
    target_compile_options(test_display_types PRIVATE /W4)
else()
    target_compile_options(test_layx PRIVATE -Wall -Wextra)
    target_compile_options(test_layout_patterns PRIVATE -Wall -Wextra)
    target_compile_options(test_defaults PRIVATE -Wall -Wextra)
    target_compile_options(test_block_margin PRIVATE -Wall -Wextra)
    target_compile_options(test_flex_margin PRIVATE -Wall -Wextra)
    target_compile_options(test_scroll PRIVATE -Wall -Wextra)
    target_compile_options(test_scroll_max PRIVATE -Wall -Wextra)
    target_compile_options(test_display_types PRIVATE -Wall -Wextra)
    target_compile_options(test_hit_test PRIVATE -Wall -Wextra)
    target_compile_options(test_margin_merge PRIVATE -Wall -Wextra)
    target_compile_options(test_destroy PRIVATE -Wall -Wextra)
    target_compile_options(test_last_child_margin PRIVATE -Wall -Wextra)
    target_compile_options(test_last_child_margin_advanced PRIVATE -Wall -Wextra)
    target_compile_options(test_multiple_layout_runs PRIVATE -Wall -Wextra)
endif()

# 使用生成器表达式来获取可执行文件的完整路径
add_custom_target(tests
    COMMAND echo "Running test_layx..."
    COMMAND $<TARGET_FILE:test_layx>
    COMMAND echo "Running test_layout_patterns..."
    COMMAND $<TARGET_FILE:test_layout_patterns>
    COMMAND echo "Running test_defaults..."
    COMMAND $<TARGET_FILE:test_defaults>
    COMMAND echo "Running test_block_margin..."
    COMMAND $<TARGET_FILE:test_block_margin>
    COMMAND echo "Running test_flex_margin..."
    COMMAND $<TARGET_FILE:test_flex_margin>
    COMMAND echo "Running test_scroll..."
    COMMAND $<TARGET_FILE:test_scroll>
    COMMAND echo "Running test_scroll_max..."
    COMMAND $<TARGET_FILE:test_scroll_max>
    COMMAND echo "Running test_display_types..."
    COMMAND $<TARGET_FILE:test_display_types>
    COMMAND echo "Running test_hit_test..."
    COMMAND $<TARGET_FILE:test_hit_test>
    COMMAND echo "Running test_margin_merge..."
    COMMAND $<TARGET_FILE:test_margin_merge>
    COMMAND echo "Running test_destroy..."
    COMMAND $<TARGET_FILE:test_destroy>
    COMMAND echo "Running test_multiple_layout_runs..."
    COMMAND $<TARGET_FILE:test_multiple_layout_runs>
    DEPENDS test_layx test_layout_patterns test_defaults test_block_margin test_flex_margin test_scroll test_scroll_max test_display_types test_hit_test test_margin_merge test_destroy test_multiple_layout_runs
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    COMMENT "Running all layout tests"
)

# 可选：如果你想要在错误时停止测试，可以使用 &&
# 这样当一个测试失败时，整个测试就会停止
# 例如：COMMAND echo "Running test_layx..." && $<TARGET_FILE:test_layx>
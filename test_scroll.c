/**
 * @file test_scroll.c
 * @brief 测试滚动功能
 */

#include <stdio.h>
#include <math.h>
#include "layx.h"

// 测试计数
static int tests_passed = 0;
static int tests_failed = 0;

// 辅助函数：检查两个浮点数是否近似相等
static int float_equals(float a, float b, float epsilon) {
    return fabsf(a - b) < epsilon;
}

// 辅助函数：测试断言
#define TEST_ASSERT(condition, message) \
    do { \
        if (condition) { \
            printf("  ✓ %s\n", message); \
            tests_passed++; \
        } else { \
            printf("  ✗ %s\n", message); \
            tests_failed++; \
        } \
    } while(0)

// 辅助函数：打印向量内容
void print_vec2(const char* name, layx_vec2 vec) {
#if defined(__GNUC__) || defined(__clang__)
    printf("%s: (%.2f, %.2f)\n", name, vec[0], vec[1]);
#else
    printf("%s: (%.2f, %.2f)\n", name, vec.xy[0], vec.xy[1]);
#endif
}

/**
 * 测试1: 内容超出容器时自动显示水平滚动条 (overflow: auto)
 */
void test_auto_horizontal_scrollbar(void) {
    printf("\n=== Test 1: Auto Horizontal Scrollbar ===\n");

    layx_context ctx;
    layx_init_context(&ctx);
    layx_reserve_items_capacity(&ctx, 10);

    // 创建容器
    layx_id container = layx_item(&ctx);
    layx_set_size(&ctx, container, 200, 150);
    layx_set_padding(&ctx, container, 10);
    layx_set_overflow(&ctx, container, LAYX_OVERFLOW_AUTO);

    // 创建子项使内容超出容器宽度
    layx_id child1 = layx_item(&ctx);
    layx_set_size(&ctx, child1, 300, 50);
    layx_push(&ctx, container, child1);

    // 运行布局
    layx_run_context(&ctx);

    // 验证水平滚动条应该显示
    int has_h_scroll = layx_has_horizontal_scrollbar(&ctx, container);
    TEST_ASSERT(has_h_scroll, "水平滚动条应显示（内容宽度310 > 客户区宽度180）");

    // 验证垂直滚动条不应该显示
    int has_v_scroll = layx_has_vertical_scrollbar(&ctx, container);
    TEST_ASSERT(!has_v_scroll, "垂直滚动条不应显示（内容高度50 < 客户区高度130）");

    // 验证内容尺寸
    layx_vec2 content_size;
    layx_get_content_size(&ctx, container, &content_size);
    TEST_ASSERT(content_size[0] > 180.0f, "内容宽度应大于客户区宽度");
    TEST_ASSERT(content_size[0] >= 300.0f, "内容宽度应至少包含子项宽度");

    layx_destroy_context(&ctx);
}

/**
 * 测试2: 内容超出容器时自动显示垂直滚动条 (overflow: auto)
 */
void test_auto_vertical_scrollbar(void) {
    printf("\n=== Test 2: Auto Vertical Scrollbar ===\n");

    layx_context ctx;
    layx_init_context(&ctx);
    layx_reserve_items_capacity(&ctx, 10);

    // 创建容器
    layx_id container = layx_item(&ctx);
    layx_set_size(&ctx, container, 200, 150);
    layx_set_padding(&ctx, container, 10);
    layx_set_overflow(&ctx, container, LAYX_OVERFLOW_AUTO);
    layx_set_display(&ctx, container, LAYX_DISPLAY_FLEX);
    layx_set_flex_direction(&ctx, container, LAYX_FLEX_DIRECTION_COLUMN);

    // 创建子项使内容超出容器高度
    layx_id child1 = layx_item(&ctx);
    layx_set_size(&ctx, child1, 100, 50);
    layx_push(&ctx, container, child1);

    layx_id child2 = layx_item(&ctx);
    layx_set_size(&ctx, child2, 100, 50);
    layx_push(&ctx, container, child2);

    layx_id child3 = layx_item(&ctx);
    layx_set_size(&ctx, child3, 100, 50);
    layx_push(&ctx, container, child3);

    layx_id child4 = layx_item(&ctx);
    layx_set_size(&ctx, child4, 100, 50);
    layx_push(&ctx, container, child4);

    // 运行布局
    layx_run_context(&ctx);

    // 验证垂直滚动条应该显示
    int has_v_scroll = layx_has_vertical_scrollbar(&ctx, container);
    TEST_ASSERT(has_v_scroll, "垂直滚动条应显示（内容高度200 > 客户区高度130）");

    // 验证水平滚动条不应该显示
    int has_h_scroll = layx_has_horizontal_scrollbar(&ctx, container);
    TEST_ASSERT(!has_h_scroll, "水平滚动条不应显示（内容宽度100 < 客户区宽度180）");

    layx_destroy_context(&ctx);
}

/**
 * 测试3: overflow: scroll 总是显示滚动条
 */
void test_overflow_scroll_always(void) {
    printf("\n=== Test 3: Overflow Scroll (Always Show) ===\n");

    layx_context ctx;
    layx_init_context(&ctx);

    layx_id container = layx_item(&ctx);
    layx_set_size(&ctx, container, 200, 200);
    layx_set_overflow(&ctx, container, LAYX_OVERFLOW_SCROLL);

    // 运行布局
    layx_run_context(&ctx);

    // overflow: scroll 应该总是显示滚动条
    int has_h_scroll = layx_has_horizontal_scrollbar(&ctx, container);
    int has_v_scroll = layx_has_vertical_scrollbar(&ctx, container);
    TEST_ASSERT(has_h_scroll || has_v_scroll, "overflow:scroll 应该显示滚动条");

    layx_destroy_context(&ctx);
}

/**
 * 测试4: overflow: hidden 不显示滚动条
 */
void test_overflow_hidden_no_scrollbars(void) {
    printf("\n=== Test 4: Overflow Hidden (No Scrollbars) ===\n");

    layx_context ctx;
    layx_init_context(&ctx);
    layx_reserve_items_capacity(&ctx, 10);

    layx_id container = layx_item(&ctx);
    layx_set_size(&ctx, container, 200, 150);
    layx_set_overflow(&ctx, container, LAYX_OVERFLOW_HIDDEN);
    layx_set_display(&ctx, container, LAYX_DISPLAY_FLEX);
    layx_set_flex_direction(&ctx, container, LAYX_FLEX_DIRECTION_COLUMN);

    // 添加大子项使内容超出
    layx_id child = layx_item(&ctx);
    layx_set_size(&ctx, child, 300, 300);
    layx_push(&ctx, container, child);

    // 运行布局
    layx_run_context(&ctx);

    // overflow: hidden 不应该显示滚动条
    int has_h_scroll = layx_has_horizontal_scrollbar(&ctx, container);
    int has_v_scroll = layx_has_vertical_scrollbar(&ctx, container);
    TEST_ASSERT(!has_h_scroll, "overflow:hidden 不应显示水平滚动条");
    TEST_ASSERT(!has_v_scroll, "overflow:hidden 不应显示垂直滚动条");

    layx_destroy_context(&ctx);
}

/**
 * 测试5: overflow: visible 不显示滚动条
 */
void test_overflow_visible_no_scrollbars(void) {
    printf("\n=== Test 5: Overflow Visible (No Scrollbars) ===\n");

    layx_context ctx;
    layx_init_context(&ctx);
    layx_reserve_items_capacity(&ctx, 10);

    layx_id container = layx_item(&ctx);
    layx_set_size(&ctx, container, 200, 150);
    layx_set_overflow(&ctx, container, LAYX_OVERFLOW_VISIBLE);
    layx_set_display(&ctx, container, LAYX_DISPLAY_FLEX);
    layx_set_flex_direction(&ctx, container, LAYX_FLEX_DIRECTION_COLUMN);

    // 添加大子项使内容超出
    layx_id child = layx_item(&ctx);
    layx_set_size(&ctx, child, 300, 300);
    layx_push(&ctx, container, child);

    // 运行布局
    layx_run_context(&ctx);

    // overflow: visible 不应该显示滚动条
    int has_h_scroll = layx_has_horizontal_scrollbar(&ctx, container);
    int has_v_scroll = layx_has_vertical_scrollbar(&ctx, container);
    TEST_ASSERT(!has_h_scroll, "overflow:visible 不应显示水平滚动条");
    TEST_ASSERT(!has_v_scroll, "overflow:visible 不应显示垂直滚动条");

    layx_destroy_context(&ctx);
}

/**
 * 测试6: scroll_to 函数
 */
void test_scroll_to(void) {
    printf("\n=== Test 6: Scroll To Function ===\n");

    layx_context ctx;
    layx_init_context(&ctx);
    layx_reserve_items_capacity(&ctx, 10);

    layx_id container = layx_item(&ctx);
    layx_set_size(&ctx, container, 200, 150);
    layx_set_overflow(&ctx, container, LAYX_OVERFLOW_AUTO);
    layx_set_display(&ctx, container, LAYX_DISPLAY_FLEX);
    layx_set_flex_direction(&ctx, container, LAYX_FLEX_DIRECTION_COLUMN);

    // 添加大子项
    layx_id child = layx_item(&ctx);
    layx_set_size(&ctx, child, 300, 300);
    layx_push(&ctx, container, child);

    layx_run_context(&ctx);

    // 滚动到指定位置
    layx_scroll_to(&ctx, container, 50.0f, 25.0f);

    // 验证滚动偏移量
    layx_vec2 scroll_offset;
    layx_get_scroll_offset(&ctx, container, &scroll_offset);
    TEST_ASSERT(float_equals(scroll_offset[0], 50.0f, 0.1f), "scroll_to(50, 25) 应设置scrollX为50");
    TEST_ASSERT(float_equals(scroll_offset[1], 25.0f, 0.1f), "scroll_to(50, 25) 应设置scrollY为25");

    layx_destroy_context(&ctx);
}

/**
 * 测试7: scroll_by 函数
 */
void test_scroll_by(void) {
    printf("\n=== Test 7: Scroll By Function ===\n");

    layx_context ctx;
    layx_init_context(&ctx);
    layx_reserve_items_capacity(&ctx, 10);

    layx_id container = layx_item(&ctx);
    layx_set_size(&ctx, container, 200, 150);
    layx_set_overflow(&ctx, container, LAYX_OVERFLOW_AUTO);
    layx_set_display(&ctx, container, LAYX_DISPLAY_FLEX);
    layx_set_flex_direction(&ctx, container, LAYX_FLEX_DIRECTION_COLUMN);

    // 添加大子项
    layx_id child = layx_item(&ctx);
    layx_set_size(&ctx, child, 300, 300);
    layx_push(&ctx, container, child);

    layx_run_context(&ctx);

    // 先滚动到某个位置
    layx_scroll_to(&ctx, container, 30.0f, 20.0f);

    // 再相对滚动
    layx_scroll_by(&ctx, container, 10.0f, 5.0f);

    // 验证滚动偏移量
    layx_vec2 scroll_offset;
    layx_get_scroll_offset(&ctx, container, &scroll_offset);
    TEST_ASSERT(float_equals(scroll_offset[0], 40.0f, 0.1f), "scroll_by(10, 5) 应使scrollX变为40");
    TEST_ASSERT(float_equals(scroll_offset[1], 25.0f, 0.1f), "scroll_by(10, 5) 应使scrollY变为25");

    layx_destroy_context(&ctx);
}

/**
 * 测试8: 滚动范围限制
 */
void test_scroll_range_clamping(void) {
    printf("\n=== Test 8: Scroll Range Clamping ===\n");

    layx_context ctx;
    layx_init_context(&ctx);
    layx_reserve_items_capacity(&ctx, 10);

    layx_id container = layx_item(&ctx);
    layx_set_size(&ctx, container, 200, 150);
    layx_set_overflow(&ctx, container, LAYX_OVERFLOW_AUTO);
    layx_set_display(&ctx, container, LAYX_DISPLAY_FLEX);
    layx_set_flex_direction(&ctx, container, LAYX_FLEX_DIRECTION_COLUMN);

    // 添加大子项
    layx_id child = layx_item(&ctx);
    layx_set_size(&ctx, child, 300, 300);
    layx_push(&ctx, container, child);

    layx_run_context(&ctx);

    // 获取最大滚动范围
    layx_vec2 scroll_max;
    layx_get_scroll_max(&ctx, container, &scroll_max);
    TEST_ASSERT(scroll_max[0] > 0.0f, "最大滚动X应大于0");
    TEST_ASSERT(scroll_max[1] > 0.0f, "最大滚动Y应大于0");

    // 尝试滚动超出范围
    layx_scroll_to(&ctx, container, 9999.0f, 9999.0f);

    // 验证滚动位置被限制在最大范围内
    layx_vec2 scroll_offset;
    layx_get_scroll_offset(&ctx, container, &scroll_offset);
    TEST_ASSERT(scroll_offset[0] <= scroll_max[0] + 1.0f, "滚动X应被限制在最大范围内");
    TEST_ASSERT(scroll_offset[1] <= scroll_max[1] + 1.0f, "滚动Y应被限制在最大范围内");

    // 尝试滚动到负值
    layx_scroll_to(&ctx, container, -10.0f, -10.0f);

    // 验证滚动位置不应为负
    layx_get_scroll_offset(&ctx, container, &scroll_offset);
    TEST_ASSERT(scroll_offset[0] >= 0.0f, "滚动X不应为负");
    TEST_ASSERT(scroll_offset[1] >= 0.0f, "滚动Y不应为负");

    layx_destroy_context(&ctx);
}

/**
 * 测试9: 获取内容尺寸
 */
void test_get_content_size(void) {
    printf("\n=== Test 9: Get Content Size ===\n");

    layx_context ctx;
    layx_init_context(&ctx);
    layx_reserve_items_capacity(&ctx, 10);

    layx_id container = layx_item(&ctx);
    layx_set_size(&ctx, container, 200, 150);
    layx_set_padding(&ctx, container, 10);
    layx_set_overflow(&ctx, container, LAYX_OVERFLOW_AUTO);
    layx_set_display(&ctx, container, LAYX_DISPLAY_FLEX);
    layx_set_flex_direction(&ctx, container, LAYX_FLEX_DIRECTION_COLUMN);
    layx_set_display(&ctx, container, LAYX_DISPLAY_FLEX);
    layx_set_flex_direction(&ctx, container, LAYX_FLEX_DIRECTION_COLUMN);

    // 添加子项
    layx_id child1 = layx_item(&ctx);
    layx_set_size(&ctx, child1, 100, 50);
    layx_push(&ctx, container, child1);

    layx_id child2 = layx_item(&ctx);
    layx_set_size(&ctx, child2, 100, 50);
    layx_push(&ctx, container, child2);

    layx_run_context(&ctx);

    // 获取内容尺寸
    layx_vec2 content_size;
    layx_get_content_size(&ctx, container, &content_size);
    TEST_ASSERT(content_size[0] > 0.0f, "内容宽度应大于0");
    TEST_ASSERT(content_size[1] > 0.0f, "内容高度应大于0");

    layx_destroy_context(&ctx);
}

/**
 * 测试10: 容器布局正确性（padding不影响滚动条检测）
 */
void test_container_layout_with_padding(void) {
    printf("\n=== Test 10: Container Layout with Padding ===\n");

    layx_context ctx;
    layx_init_context(&ctx);
    layx_reserve_items_capacity(&ctx, 10);

    layx_id container = layx_item(&ctx);
    layx_set_size(&ctx, container, 200, 150);
    layx_set_padding(&ctx, container, 10);
    layx_set_overflow(&ctx, container, LAYX_OVERFLOW_AUTO);

    // 添加超出容器的子项
    layx_id child = layx_item(&ctx);
    layx_set_size(&ctx, child, 300, 50);
    layx_push(&ctx, container, child);

    layx_run_context(&ctx);

    // 验证容器布局正确
    layx_vec4 rect = layx_get_rect(&ctx, container);
    TEST_ASSERT(float_equals(rect[0], 0.0f, 0.1f), "容器X位置应为0");
    TEST_ASSERT(float_equals(rect[1], 0.0f, 0.1f), "容器Y位置应为0");
    TEST_ASSERT(float_equals(rect[2], 200.0f, 0.1f), "容器宽度应为200");
    TEST_ASSERT(float_equals(rect[3], 150.0f, 0.1f), "容器高度应为150");

    // 验证滚动条仍然正确检测
    int has_h_scroll = layx_has_horizontal_scrollbar(&ctx, container);
    TEST_ASSERT(has_h_scroll, "即使有padding，水平滚动条也应正确显示");

    layx_destroy_context(&ctx);
}

int main(void) {
    printf("===========================================\n");
    printf("   LAYX Scroll Functionality Test Suite\n");
    printf("===========================================\n");

    // 运行所有测试
    test_auto_horizontal_scrollbar();
    test_auto_vertical_scrollbar();
    test_overflow_scroll_always();
    test_overflow_hidden_no_scrollbars();
    test_overflow_visible_no_scrollbars();
    test_scroll_to();
    test_scroll_by();
    test_scroll_range_clamping();
    test_get_content_size();
    test_container_layout_with_padding();

    // 输出测试总结
    printf("\n===========================================\n");
    printf("           Test Summary\n");
    printf("===========================================\n");
    printf("Tests Passed: %d\n", tests_passed);
    printf("Tests Failed: %d\n", tests_failed);
    printf("Total Tests:  %d\n", tests_passed + tests_failed);
    printf("===========================================\n");

    if (tests_failed == 0) {
        printf("✓ All tests passed!\n");
    } else {
        printf("✗ Some tests failed!\n");
    }

    return 0;
}
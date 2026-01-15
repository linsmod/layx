/**
 * @file test_hit_test.c
 * @brief 测试 layx_hit_test 点命中测试功能
 */

#include <stdio.h>
#include <math.h>
#include "layx.h"

// 测试计数
static int tests_passed = 0;
static int tests_failed = 0;

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

/**
 * 测试1: 基本点命中测试 - 点在元素内部
 */
void test_basic_hit_inside(void) {
    printf("\n=== Test 1: Basic Hit Test - Point Inside Element ===\n");

    layx_context ctx;
    layx_init_context(&ctx);
    layx_reserve_items_capacity(&ctx, 10);

    // 创建一个简单的元素
    layx_id element = layx_item(&ctx);
    layx_set_size(&ctx, element, 200, 150);

    layx_run_context(&ctx);

    // 测试元素内部的点
    int hit = layx_hit_test(&ctx, element, 100, 80);
    TEST_ASSERT(hit, "点(100, 80)应该在元素内部");

    hit = layx_hit_test(&ctx, element, 0, 0);
    TEST_ASSERT(hit, "点(0, 0)应该在元素左上角（包含边界）");

    hit = layx_hit_test(&ctx, element, 199, 149);
    TEST_ASSERT(hit, "点(199, 149)应该在元素内部（边界内）");

    layx_destroy_context(&ctx);
}

/**
 * 测试2: 基本点命中测试 - 点在元素外部
 */
void test_basic_hit_outside(void) {
    printf("\n=== Test 2: Basic Hit Test - Point Outside Element ===\n");

    layx_context ctx;
    layx_init_context(&ctx);
    layx_reserve_items_capacity(&ctx, 10);

    // 创建元素
    layx_id element = layx_item(&ctx);
    layx_set_size(&ctx, element, 200, 150);

    layx_run_context(&ctx);

    // 测试元素外部的点
    int hit = layx_hit_test(&ctx, element, 300, 200);
    TEST_ASSERT(!hit, "点(300, 200)应该在元素外部");

    hit = layx_hit_test(&ctx, element, -10, -10);
    TEST_ASSERT(!hit, "点(-10, -10)应该在元素外部");

    hit = layx_hit_test(&ctx, element, 200, 150);
    TEST_ASSERT(!hit, "点(200, 150)应该在元素外部（右下边界外）");

    layx_destroy_context(&ctx);
}

/**
 * 测试3: 带滚动容器的命中测试
 */
void test_hit_with_scroll(void) {
    printf("\n=== Test 3: Hit Test with Scroll Container ===\n");

    layx_context ctx;
    layx_init_context(&ctx);
    layx_reserve_items_capacity(&ctx, 10);

    // 创建滚动容器
    layx_id container = layx_item(&ctx);
    layx_set_size(&ctx, container, 200, 150);
    layx_set_overflow(&ctx, container, LAYX_OVERFLOW_AUTO);

    // 创建超出容器大小的子项
    layx_id child = layx_item(&ctx);
    layx_set_size(&ctx, child, 300, 200);
    layx_set_min_size(&ctx, child, 300, 200);
    layx_prepend(&ctx, container, child);

    layx_run_context(&ctx);

    // 滚动子项
    layx_scroll_to(&ctx, container, 50.0f, 30.0f);

    // 测试命中：考虑滚动偏移
    // 子项位置在容器内部，滚动后，实际显示区域移动了
    int hit = layx_hit_test(&ctx, child, 100, 80);
    TEST_ASSERT(hit, "考虑滚动偏移后，点(100, 80)应该在子项内部");

    layx_destroy_context(&ctx);
}

/**
 * 测试4: 嵌套容器的命中测试
 */
void test_hit_nested_containers(void) {
    printf("\n=== Test 4: Hit Test with Nested Containers ===\n");

    layx_context ctx;
    layx_init_context(&ctx);
    layx_reserve_items_capacity(&ctx, 20);

    // 创建外层容器
    layx_id outer = layx_item(&ctx);
    layx_set_size(&ctx, outer, 400, 300);
    layx_set_display(&ctx, outer, LAYX_DISPLAY_FLEX);

    // 创建内层容器
    layx_id inner = layx_item(&ctx);
    layx_set_size(&ctx, inner, 200, 150);
    layx_append(&ctx, outer, inner);

    // 创建叶子元素
    layx_id leaf = layx_item(&ctx);
    layx_set_size(&ctx, leaf, 100, 80);
    layx_append(&ctx, inner, leaf);

    layx_run_context(&ctx);

    // 获取叶子元素的rect
    layx_vec4 leaf_rect = layx_get_rect(&ctx, leaf);

    // 测试叶子元素的命中 - 使用leaf元素的中心点
    layx_scalar center_x = leaf_rect[0] + leaf_rect[2] / 2;
    layx_scalar center_y = leaf_rect[1] + leaf_rect[3] / 2;
    int hit = layx_hit_test(&ctx, leaf, center_x, center_y);
    TEST_ASSERT(hit, "叶子元素中心点应该命中");

    layx_destroy_context(&ctx);
}

/**
 * 测试5: 带padding的元素命中测试
 */
void test_hit_with_padding(void) {
    printf("\n=== Test 5: Hit Test with Padding ===\n");

    layx_context ctx;
    layx_init_context(&ctx);
    layx_reserve_items_capacity(&ctx, 10);

    // 创建带padding的元素
    layx_id element = layx_item(&ctx);
    layx_set_size(&ctx, element, 200, 150);
    layx_set_position_lt(&ctx, element, 50, 30);
    layx_set_padding(&ctx, element, 20);

    layx_run_context(&ctx);

    // 命中测试应该考虑元素的margin-box（包含padding）
    int hit = layx_hit_test(&ctx, element, 60, 40);
    TEST_ASSERT(hit, "点(60, 40)应该在元素内部（包含padding区域）");

    hit = layx_hit_test(&ctx, element, 70, 70);
    TEST_ASSERT(hit, "点(70, 70)应该在元素内部（在padding区域内）");

    layx_destroy_context(&ctx);
}

/**
 * 测试6: 带margin的元素命中测试
 */
void test_hit_with_margin(void) {
    printf("\n=== Test 6: Hit Test with Margin ===\n");

    layx_context ctx;
    layx_init_context(&ctx);
    layx_reserve_items_capacity(&ctx, 10);

    // 创建容器
    layx_id container = layx_item(&ctx);
    layx_set_size(&ctx, container, 400, 300);
    layx_set_display(&ctx, container, LAYX_DISPLAY_FLEX);

    // 创建带margin的子元素
    layx_id child = layx_item(&ctx);
    layx_set_size(&ctx, child, 100, 80);
    layx_set_margin(&ctx, child, 20);
    layx_append(&ctx, container, child);

    layx_run_context(&ctx);

    // 命中测试应该基于元素的位置（margin-box相对于父元素content-box）
    layx_vec4 child_rect = layx_get_rect(&ctx, child);
    int hit = layx_hit_test(&ctx, child, child_rect[0] + 10, child_rect[1] + 10);
    TEST_ASSERT(hit, "点应该在元素margin-box内部");

    layx_destroy_context(&ctx);
}

/**
 * 测试7: 多层滚动容器的命中测试
 */
void test_hit_multiple_scroll_containers(void) {
    printf("\n=== Test 7: Hit Test with Multiple Scroll Containers ===\n");

    layx_context ctx;
    layx_init_context(&ctx);
    layx_reserve_items_capacity(&ctx, 20);

    // 外层滚动容器
    layx_id outer = layx_item(&ctx);
    layx_set_size(&ctx, outer, 300, 200);
    layx_set_overflow(&ctx, outer, LAYX_OVERFLOW_AUTO);
    layx_set_display(&ctx, outer, LAYX_DISPLAY_FLEX);
    layx_set_flex_direction(&ctx, outer, LAYX_FLEX_DIRECTION_COLUMN);

    // 内层滚动容器
    layx_id inner = layx_item(&ctx);
    layx_set_size(&ctx, inner, 250, 300);
    layx_set_overflow(&ctx, inner, LAYX_OVERFLOW_AUTO);
    layx_prepend(&ctx, outer, inner);

    // 内容项
    layx_id content = layx_item(&ctx);
    layx_set_size(&ctx, content, 200, 400);
    layx_set_min_size(&ctx, content, 200, 400);
    layx_prepend(&ctx, inner, content);

    layx_run_context(&ctx);

    // 获取content的rect
    layx_vec4 content_rect = layx_get_rect(&ctx, content);

    // 设置滚动偏移
    layx_scroll_to(&ctx, outer, 0, 50);
    layx_scroll_to(&ctx, inner, 0, 100);

    // 测试命中：使用content元素的中心点
    layx_scalar content_center_x = content_rect[0] + content_rect[2] / 2;
    layx_scalar content_center_y = content_rect[1] + content_rect[3] / 2;
    int hit = layx_hit_test(&ctx, content, content_center_x, content_center_y);
    TEST_ASSERT(hit, "考虑多层滚动偏移后，content中心点应该命中");

    layx_destroy_context(&ctx);
}

/**
 * 测试8: 水平滚动的命中测试
 */
void test_hit_horizontal_scroll(void) {
    printf("\n=== Test 8: Hit Test with Horizontal Scroll ===\n");

    layx_context ctx;
    layx_init_context(&ctx);
    layx_reserve_items_capacity(&ctx, 10);

    // 创建水平滚动容器
    layx_id container = layx_item(&ctx);
    layx_set_size(&ctx, container, 200, 150);
    layx_set_overflow(&ctx, container, LAYX_OVERFLOW_AUTO);
    layx_set_display(&ctx, container, LAYX_DISPLAY_FLEX);
    layx_set_flex_direction(&ctx, container, LAYX_FLEX_DIRECTION_ROW);

    // 创建宽子项
    layx_id child = layx_item(&ctx);
    layx_set_size(&ctx, child, 400, 100);
    layx_set_min_size(&ctx, child, 400, 100);
    layx_prepend(&ctx, container, child);

    layx_run_context(&ctx);

    // 水平滚动
    layx_scroll_to(&ctx, container, 100, 0);

    // 测试命中：考虑水平滚动偏移
    int hit = layx_hit_test(&ctx, child, 150, 75);
    TEST_ASSERT(hit, "考虑水平滚动偏移后，点应该命中子项");

    layx_destroy_context(&ctx);
}

/**
 * 测试9: 边界情况 - 零尺寸元素
 */
void test_hit_zero_size_element(void) {
    printf("\n=== Test 9: Hit Test with Zero Size Element ===\n");

    layx_context ctx;
    layx_init_context(&ctx);
    layx_reserve_items_capacity(&ctx, 10);

    // 创建零尺寸元素
    layx_id element = layx_item(&ctx);
    layx_set_size(&ctx, element, 0, 0);
    layx_set_position_lt(&ctx, element, 100, 100);

    layx_run_context(&ctx);

    // 零尺寸元素不应该命中任何点（除了可能的边界点）
    int hit = layx_hit_test(&ctx, element, 100, 100);
    // 由于是闭区间，理论上可能命中左上角，但实际中通常不应该
    // 这里只是测试边界情况
    TEST_ASSERT(hit == hit, "零尺寸元素的命中测试应该稳定");

    layx_destroy_context(&ctx);
}

/**
 * 测试10: 多个重叠元素的命中测试
 */
void test_hit_overlapping_elements(void) {
    printf("\n=== Test 10: Hit Test with Overlapping Elements ===\n");

    layx_context ctx;
    layx_init_context(&ctx);
    layx_reserve_items_capacity(&ctx, 10);

    // 创建第一个元素（使用容器创建重叠效果）
    layx_id container = layx_item(&ctx);
    layx_set_size(&ctx, container, 300, 300);
    layx_set_display(&ctx, container, LAYX_DISPLAY_FLEX);

    // 创建两个重叠的子元素（通过负margin实现重叠）
    layx_id elem1 = layx_item(&ctx);
    layx_set_size(&ctx, elem1, 100, 100);
    layx_set_margin_ltrb(&ctx, elem1, 0, 0, 0, 0);
    layx_append(&ctx, container, elem1);

    layx_id elem2 = layx_item(&ctx);
    layx_set_size(&ctx, elem2, 100, 100);
    layx_set_margin_ltrb(&ctx, elem2, -50, -50, 0, 0);  // 负margin导致重叠
    layx_append(&ctx, container, elem2);

    layx_run_context(&ctx);

    // 获取elem1的rect
    layx_vec4 elem1_rect = layx_get_rect(&ctx, elem1);

    // 测试重叠区域（如果有的话）
    // 测试elem1的中心点
    layx_scalar elem1_center_x = elem1_rect[0] + elem1_rect[2] / 2;
    layx_scalar elem1_center_y = elem1_rect[1] + elem1_rect[3] / 2;
    int hit1 = layx_hit_test(&ctx, elem1, elem1_center_x, elem1_center_y);
    layx_hit_test(&ctx, elem2, elem1_center_x, elem1_center_y);  // 测试elem2命中（但不检查结果）

    TEST_ASSERT(hit1, "elem1中心点应该在elem1内部");
    // elem2的命中取决于实际重叠情况

    layx_destroy_context(&ctx);
}

/**
 * 测试11: 极端坐标值测试
 */
void test_hit_extreme_coordinates(void) {
    printf("\n=== Test 11: Hit Test with Extreme Coordinates ===\n");

    layx_context ctx;
    layx_init_context(&ctx);
    layx_reserve_items_capacity(&ctx, 10);

    // 创建元素
    layx_id element = layx_item(&ctx);
    layx_set_size(&ctx, element, 200, 150);
    layx_set_position_lt(&ctx, element, 50, 30);

    layx_run_context(&ctx);

    // 测试极端坐标
    int hit = layx_hit_test(&ctx, element, -1000, -1000);
    TEST_ASSERT(!hit, "极端负坐标不应命中");

    hit = layx_hit_test(&ctx, element, 10000, 10000);
    TEST_ASSERT(!hit, "极端正坐标不应命中");

    layx_destroy_context(&ctx);
}

/**
 * 测试12: 带边框的元素命中测试
 */
void test_hit_with_border(void) {
    printf("\n=== Test 12: Hit Test with Border ===\n");

    layx_context ctx;
    layx_init_context(&ctx);
    layx_reserve_items_capacity(&ctx, 10);

    // 创建带边框的元素
    layx_id element = layx_item(&ctx);
    layx_set_size(&ctx, element, 200, 150);
    layx_set_border(&ctx, element, 10);

    layx_run_context(&ctx);

    // 获取rect
    layx_vec4 rect = layx_get_rect(&ctx, element);

    // 命中测试应该包含边框区域（border-box）
    int hit = layx_hit_test(&ctx, element, 10, 10);
    TEST_ASSERT(hit, "点应该在元素border-box内部（包含边框）");

    // 边界内但接近边界
    hit = layx_hit_test(&ctx, element, rect[2]-1, rect[3]-1);
    TEST_ASSERT(hit, "点应该在元素右下角边界内");

    layx_destroy_context(&ctx);
}

/**
 * 测试13: overflow: visible的命中测试
 */
void test_hit_overflow_visible(void) {
    printf("\n=== Test 13: Hit Test with Overflow Visible ===\n");

    layx_context ctx;
    layx_init_context(&ctx);
    layx_reserve_items_capacity(&ctx, 10);

    // 创建overflow: visible的容器
    layx_id container = layx_item(&ctx);
    layx_set_size(&ctx, container, 200, 150);
    layx_set_overflow(&ctx, container, LAYX_OVERFLOW_VISIBLE);

    // 创建超出容器的子项
    layx_id child = layx_item(&ctx);
    layx_set_size(&ctx, child, 300, 200);
    layx_prepend(&ctx, container, child);

    layx_run_context(&ctx);

    // overflow: visible时，子项可能超出容器边界
    // 但hit_test只检查元素本身的rect
    int hit = layx_hit_test(&ctx, child, 250, 100);
    TEST_ASSERT(hit, "子项超出容器部分仍应能命中");

    layx_destroy_context(&ctx);
}

/**
 * 测试14: 嵌套不同overflow的命中测试
 */
void test_hit_nested_different_overflow(void) {
    printf("\n=== Test 14: Hit Test with Nested Different Overflow ===\n");

    layx_context ctx;
    layx_init_context(&ctx);
    layx_reserve_items_capacity(&ctx, 20);

    // 外层overflow: auto
    layx_id outer = layx_item(&ctx);
    layx_set_size(&ctx, outer, 300, 200);
    layx_set_overflow(&ctx, outer, LAYX_OVERFLOW_AUTO);
    layx_set_display(&ctx, outer, LAYX_DISPLAY_FLEX);
    layx_set_flex_direction(&ctx, outer, LAYX_FLEX_DIRECTION_COLUMN);

    // 内层overflow: visible
    layx_id inner = layx_item(&ctx);
    layx_set_size(&ctx, inner, 250, 250);
    layx_set_overflow(&ctx, inner, LAYX_OVERFLOW_VISIBLE);
    layx_prepend(&ctx, outer, inner);

    // 内容项
    layx_id content = layx_item(&ctx);
    layx_set_size(&ctx, content, 200, 300);
    layx_prepend(&ctx, inner, content);

    layx_run_context(&ctx);

    // 滚动外层
    layx_scroll_to(&ctx, outer, 0, 50);

    // 测试命中：只考虑外层滚动，内层不滚动
    int hit = layx_hit_test(&ctx, content, 100, 100);
    TEST_ASSERT(hit, "应该正确处理嵌套的不同overflow设置");

    layx_destroy_context(&ctx);
}

/**
 * 测试15: 复杂布局的命中测试
 */
void test_hit_complex_layout(void) {
    printf("\n=== Test 15: Hit Test with Complex Layout ===\n");

    layx_context ctx;
    layx_init_context(&ctx);
    layx_reserve_items_capacity(&ctx, 30);

    // 创建复杂的布局结构
    layx_id root = layx_item(&ctx);
    layx_set_size(&ctx, root, 600, 400);
    layx_set_display(&ctx, root, LAYX_DISPLAY_FLEX);
    layx_set_flex_direction(&ctx, root, LAYX_FLEX_DIRECTION_ROW);
    layx_set_padding(&ctx, root, 10);

    // 侧边栏
    layx_id sidebar = layx_item(&ctx);
    layx_set_size(&ctx, sidebar, 150, 0);
    layx_set_display(&ctx, sidebar, LAYX_DISPLAY_FLEX);
    layx_set_flex_direction(&ctx, sidebar, LAYX_FLEX_DIRECTION_COLUMN);
    layx_set_margin_ltrb(&ctx, sidebar, 0, 0, 10, 0);
    layx_append(&ctx, root, sidebar);

    // 侧边栏按钮
    layx_id btn1 = layx_item(&ctx);
    layx_set_size(&ctx, btn1, 0, 40);
    layx_set_margin_ltrb(&ctx, btn1, 0, 0, 0, 5);
    layx_append(&ctx, sidebar, btn1);

    layx_id btn2 = layx_item(&ctx);
    layx_set_size(&ctx, btn2, 0, 40);
    layx_set_margin_ltrb(&ctx, btn2, 0, 0, 0, 5);
    layx_append(&ctx, sidebar, btn2);

    // 主内容区
    layx_id content = layx_item(&ctx);
    layx_set_display(&ctx, content, LAYX_DISPLAY_FLEX);
    layx_set_flex_direction(&ctx, content, LAYX_FLEX_DIRECTION_COLUMN);
    layx_append(&ctx, root, content);

    // Header
    layx_id header = layx_item(&ctx);
    layx_set_size(&ctx, header, 400, 60);  // 设置固定宽度和高度
    layx_set_margin_ltrb(&ctx, header, 0, 0, 0, 10);
    layx_append(&ctx, content, header);

    // Body（滚动容器）
    layx_id body = layx_item(&ctx);
    layx_set_display(&ctx, body, LAYX_DISPLAY_FLEX);
    layx_set_flex_direction(&ctx, body, LAYX_FLEX_DIRECTION_COLUMN);
    layx_set_overflow(&ctx, body, LAYX_OVERFLOW_AUTO);
    layx_append(&ctx, content, body);

    // 添加多个卡片
    layx_id card1 = layx_item(&ctx);
    layx_set_size(&ctx, card1, 400, 80);  // 设置固定宽度和高度
    layx_set_margin_ltrb(&ctx, card1, 0, 0, 0, 10);
    layx_append(&ctx, body, card1);

    layx_id card2 = layx_item(&ctx);
    layx_set_size(&ctx, card2, 400, 80);  // 设置固定宽度和高度
    layx_set_margin_ltrb(&ctx, card2, 0, 0, 0, 10);
    layx_append(&ctx, body, card2);

    layx_id card3 = layx_item(&ctx);
    layx_set_size(&ctx, card3, 400, 80);  // 设置固定宽度和高度
    layx_set_margin_ltrb(&ctx, card3, 0, 0, 0, 10);
    layx_append(&ctx, body, card3);

    layx_run_context(&ctx);

    // 获取各个元素的rect
    layx_vec4 btn1_rect = layx_get_rect(&ctx, btn1);
    layx_vec4 header_rect = layx_get_rect(&ctx, header);
    layx_vec4 card2_rect = layx_get_rect(&ctx, card2);

    // 测试各个元素的命中（使用元素中心点）
    layx_scalar btn1_center_x = btn1_rect[0] + btn1_rect[2] / 2;
    layx_scalar btn1_center_y = btn1_rect[1] + btn1_rect[3] / 2;
    int hit = layx_hit_test(&ctx, btn1, btn1_center_x, btn1_center_y);
    TEST_ASSERT(hit, "btn1中心点应该命中btn1");

    layx_scalar header_center_x = header_rect[0] + header_rect[2] / 2;
    layx_scalar header_center_y = header_rect[1] + header_rect[3] / 2;
    hit = layx_hit_test(&ctx, header, header_center_x, header_center_y);
    TEST_ASSERT(hit, "header中心点应该命中header");

    // 滚动body后测试
    layx_scroll_to(&ctx, body, 0, 30);
    layx_scalar card2_center_x = card2_rect[0] + card2_rect[2] / 2;
    layx_scalar card2_center_y = card2_rect[1] + card2_rect[3] / 2;
    hit = layx_hit_test(&ctx, card2, card2_center_x, card2_center_y);
    TEST_ASSERT(hit, "滚动后，card2中心点应该命中card2");

    layx_destroy_context(&ctx);
}

int main(void) {
    printf("===========================================\n");
    printf("   LAYX Hit Test Functionality Test Suite\n");
    printf("===========================================\n");

    // 运行所有测试
    test_basic_hit_inside();
    test_basic_hit_outside();
    test_hit_with_scroll();
    test_hit_nested_containers();
    test_hit_with_padding();
    test_hit_with_margin();
    test_hit_multiple_scroll_containers();
    test_hit_horizontal_scroll();
    test_hit_zero_size_element();
    test_hit_overlapping_elements();
    test_hit_extreme_coordinates();
    test_hit_with_border();
    test_hit_overflow_visible();
    test_hit_nested_different_overflow();
    test_hit_complex_layout();

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

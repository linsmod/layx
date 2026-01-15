#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "layx.h"

// 测试计数
static int tests_passed = 0;
static int tests_failed = 0;

// 辅助函数：检查两个浮点数是否近似相等
static int float_equals(float a, float b, float epsilon) {
    return fabsf(a - b) < epsilon;
}

// 测试断言
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
 * 测试1: 基础布局结构创建
 */
void test_basic_layout_structure(void) {
    printf("\n=== Test 1: Basic Layout Structure ===");

    layx_context ctx;
    layx_init_context(&ctx);
    layx_reserve_items_capacity(&ctx, 20);

    // 创建根容器
    layx_id root = layx_item(&ctx);
    layx_set_size(&ctx, root, 600, 400);
    layx_set_display(&ctx, root, LAYX_DISPLAY_FLEX);
    layx_set_flex_direction(&ctx, root, LAYX_FLEX_DIRECTION_ROW);
    layx_set_padding(&ctx, root, 10);

    // 创建侧边栏
    layx_id sidebar = layx_item(&ctx);
    layx_set_size(&ctx, sidebar, 150, 0);
    layx_set_display(&ctx, sidebar, LAYX_DISPLAY_FLEX);
    layx_set_flex_direction(&ctx, sidebar, LAYX_FLEX_DIRECTION_COLUMN);
    layx_set_margin_ltrb(&ctx, sidebar, 0, 0, 10, 0);
    layx_append(&ctx, root, sidebar);

    // 创建主内容区
    layx_id content = layx_item(&ctx);
    layx_set_display(&ctx, content, LAYX_DISPLAY_FLEX);
    layx_set_flex_direction(&ctx, content, LAYX_FLEX_DIRECTION_COLUMN);
    layx_append(&ctx, root, content);

    // 运行布局
    layx_run_context(&ctx);

    // 验证根容器位置和大小
    layx_vec4 root_rect = layx_get_rect(&ctx, root);
    TEST_ASSERT(float_equals(root_rect[0], 0.0f, 0.1f), "根容器X位置应为0");
    TEST_ASSERT(float_equals(root_rect[1], 0.0f, 0.1f), "根容器Y位置应为0");
    TEST_ASSERT(float_equals(root_rect[2], 600.0f, 0.1f), "根容器宽度应为600");
    TEST_ASSERT(float_equals(root_rect[3], 400.0f, 0.1f), "根容器高度应为400");

    // 验证侧边栏位置
    layx_vec4 sidebar_rect = layx_get_rect(&ctx, sidebar);
    TEST_ASSERT(float_equals(sidebar_rect[0], 10.0f, 0.1f), "侧边栏X位置应为10(padding)");
    TEST_ASSERT(float_equals(sidebar_rect[1], 10.0f, 0.1f), "侧边栏Y位置应为10(padding)");
    TEST_ASSERT(float_equals(sidebar_rect[2], 150.0f, 0.1f), "侧边栏宽度应为150");

    // 验证侧边栏在根容器左边
    TEST_ASSERT(sidebar_rect[0] >= root_rect[0], "侧边栏应在根容器内部");
    TEST_ASSERT(sidebar_rect[0] + sidebar_rect[2] <= root_rect[0] + root_rect[2], "侧边栏应在根容器宽度范围内");

    layx_destroy_context(&ctx);
}

/**
 * 测试2: 嵌套布局
 */
void test_nested_layout(void) {
    printf("\n=== Test 2: Nested Layout ===");

    layx_context ctx;
    layx_init_context(&ctx);
    layx_reserve_items_capacity(&ctx, 20);

    // 创建根容器
    layx_id root = layx_item(&ctx);
    layx_set_size(&ctx, root, 600, 400);
    layx_set_display(&ctx, root, LAYX_DISPLAY_FLEX);
    layx_set_flex_direction(&ctx, root, LAYX_FLEX_DIRECTION_ROW);
    layx_set_padding(&ctx, root, 10);

    // 创建侧边栏
    layx_id sidebar = layx_item(&ctx);
    layx_set_size(&ctx, sidebar, 150, 0);
    layx_set_display(&ctx, sidebar, LAYX_DISPLAY_FLEX);
    layx_set_flex_direction(&ctx, sidebar, LAYX_FLEX_DIRECTION_COLUMN);
    layx_set_margin_ltrb(&ctx, sidebar, 0, 0, 10, 0);
    layx_append(&ctx, root, sidebar);

    // 创建侧边栏按钮
    layx_id btn1 = layx_item(&ctx);
    layx_set_size(&ctx, btn1, 0, 40);
    layx_set_margin_ltrb(&ctx, btn1, 0, 0, 0, 5);
    layx_append(&ctx, sidebar, btn1);

    layx_id btn2 = layx_item(&ctx);
    layx_set_size(&ctx, btn2, 0, 40);
    layx_set_margin_ltrb(&ctx, btn2, 0, 0, 0, 5);
    layx_append(&ctx, sidebar, btn2);

    layx_id btn3 = layx_item(&ctx);
    layx_set_size(&ctx, btn3, 0, 40);
    layx_append(&ctx, sidebar, btn3);

    // 创建主内容区
    layx_id content = layx_item(&ctx);
    layx_set_display(&ctx, content, LAYX_DISPLAY_FLEX);
    layx_set_flex_direction(&ctx, content, LAYX_FLEX_DIRECTION_COLUMN);
    layx_append(&ctx, root, content);

    // 创建header
    layx_id header = layx_item(&ctx);
    layx_set_size(&ctx, header, 0, 60);
    layx_set_margin_ltrb(&ctx, header, 0, 0, 0, 10);
    layx_append(&ctx, content, header);

    // 运行布局
    layx_run_context(&ctx);

    // 验证按钮1在侧边栏中
    layx_vec4 btn1_rect = layx_get_rect(&ctx, btn1);
    layx_vec4 sidebar_rect = layx_get_rect(&ctx, sidebar);
    TEST_ASSERT(btn1_rect[0] >= sidebar_rect[0], "按钮1应在侧边栏X范围内");
    TEST_ASSERT(btn1_rect[1] >= sidebar_rect[1], "按钮1应在侧边栏Y范围内");

    // 验证按钮1在按钮2上方
    layx_vec4 btn2_rect = layx_get_rect(&ctx, btn2);
    TEST_ASSERT(btn1_rect[1] < btn2_rect[1], "按钮1应在按钮2上方");

    // 验证按钮2在按钮3上方
    layx_vec4 btn3_rect = layx_get_rect(&ctx, btn3);
    TEST_ASSERT(btn2_rect[1] < btn3_rect[1], "按钮2应在按钮3上方");

    // 验证header在主内容区顶部
    layx_vec4 content_rect = layx_get_rect(&ctx, content);
    layx_vec4 header_rect = layx_get_rect(&ctx, header);
    TEST_ASSERT(header_rect[0] >= content_rect[0], "header应在内容区X范围内");
    TEST_ASSERT(header_rect[1] >= content_rect[1], "header应在内容区Y范围内");

    layx_destroy_context(&ctx);
}

/**
 * 测试3: 复杂布局（侧边栏+主内容区+卡片）
 */
void test_complex_layout(void) {
    printf("\n=== Test 3: Complex Layout ===");

    layx_context ctx;
    layx_init_context(&ctx);
    layx_reserve_items_capacity(&ctx, 20);

    // 创建根容器
    layx_id root = layx_item(&ctx);
    layx_set_size(&ctx, root, 600, 400);
    layx_set_display(&ctx, root, LAYX_DISPLAY_FLEX);
    layx_set_flex_direction(&ctx, root, LAYX_FLEX_DIRECTION_ROW);
    layx_set_padding(&ctx, root, 10);

    // 创建侧边栏
    layx_id sidebar = layx_item(&ctx);
    layx_set_size(&ctx, sidebar, 150, 0);
    layx_set_display(&ctx, sidebar, LAYX_DISPLAY_FLEX);
    layx_set_flex_direction(&ctx, sidebar, LAYX_FLEX_DIRECTION_COLUMN);
    layx_set_margin_ltrb(&ctx, sidebar, 0, 0, 10, 0);
    layx_append(&ctx, root, sidebar);

    // 创建3个侧边栏按钮
    layx_id btn1 = layx_item(&ctx);
    layx_set_size(&ctx, btn1, 0, 40);
    layx_set_margin_ltrb(&ctx, btn1, 0, 0, 0, 5);
    layx_append(&ctx, sidebar, btn1);

    layx_id btn2 = layx_item(&ctx);
    layx_set_size(&ctx, btn2, 0, 40);
    layx_set_margin_ltrb(&ctx, btn2, 0, 0, 0, 5);
    layx_append(&ctx, sidebar, btn2);

    layx_id btn3 = layx_item(&ctx);
    layx_set_size(&ctx, btn3, 0, 40);
    layx_append(&ctx, sidebar, btn3);

    // 创建主内容区
    layx_id content = layx_item(&ctx);
    layx_set_display(&ctx, content, LAYX_DISPLAY_FLEX);
    layx_set_flex_direction(&ctx, content, LAYX_FLEX_DIRECTION_COLUMN);
    layx_append(&ctx, root, content);

    // 创建header
    layx_id header = layx_item(&ctx);
    layx_set_size(&ctx, header, 0, 60);
    layx_set_margin_ltrb(&ctx, header, 0, 0, 0, 10);
    layx_append(&ctx, content, header);

    // 创建body
    layx_id body = layx_item(&ctx);
    layx_set_display(&ctx, body, LAYX_DISPLAY_FLEX);
    layx_set_flex_direction(&ctx, body, LAYX_FLEX_DIRECTION_COLUMN);
    layx_append(&ctx, content, body);

    // 创建2个卡片
    layx_id card1 = layx_item(&ctx);
    layx_set_size(&ctx, card1, 0, 100);
    layx_set_margin_ltrb(&ctx, card1, 0, 0, 0, 10);
    layx_append(&ctx, body, card1);

    layx_id card2 = layx_item(&ctx);
    layx_set_size(&ctx, card2, 0, 100);
    layx_set_margin_ltrb(&ctx, card2, 0, 0, 0, 10);
    layx_append(&ctx, body, card2);

    // 运行布局
    layx_run_context(&ctx);

    // 验证总item数量
    int total_items = layx_items_count(&ctx);
    TEST_ASSERT(total_items == 10, "总item数量应为10");

    // 验证所有元素都不超出根容器范围
    for (int i = 0; i < total_items; i++) {
        layx_vec4 rect = layx_get_rect(&ctx, i);
        TEST_ASSERT(rect[0] >= 0.0f, "所有元素X位置不应小于0");
        TEST_ASSERT(rect[1] >= 0.0f, "所有元素Y位置不应小于0");
    }

    // 验证卡片1在卡片2上方
    layx_vec4 card1_rect = layx_get_rect(&ctx, card1);
    layx_vec4 card2_rect = layx_get_rect(&ctx, card2);
    TEST_ASSERT(float_equals(card1_rect[0], card2_rect[0], 0.1f), "卡片1和卡片2应有相同X坐标");
    TEST_ASSERT(card1_rect[1] < card2_rect[1], "卡片1应在卡片2上方");

    layx_destroy_context(&ctx);
}

/**
 * 测试4: Padding正确性
 */
void test_padding_correctness(void) {
    printf("\n=== Test 4: Padding Correctness ===");

    layx_context ctx;
    layx_init_context(&ctx);
    layx_reserve_items_capacity(&ctx, 10);

    // 创建容器
    layx_id container = layx_item(&ctx);
    layx_set_size(&ctx, container, 600, 400);
    layx_set_display(&ctx, container, LAYX_DISPLAY_FLEX);
    layx_set_padding(&ctx, container, 10);

    // 创建子元素
    layx_id child = layx_item(&ctx);
    layx_set_size(&ctx, child, 100, 50);
    layx_append(&ctx, container, child);

    // 运行布局
    layx_run_context(&ctx);

    // 验证子元素位置考虑了padding
    layx_vec4 child_rect = layx_get_rect(&ctx, child);
    TEST_ASSERT(float_equals(child_rect[0], 10.0f, 0.1f), "子元素X位置应为padding值(10)");
    TEST_ASSERT(float_equals(child_rect[1], 10.0f, 0.1f), "子元素Y位置应为padding值(10)");

    layx_destroy_context(&ctx);
}

/**
 * 测试5: Margin正确性
 */
void test_margin_correctness(void) {
    printf("\n=== Test 5: Margin Correctness ===");

    layx_context ctx;
    layx_init_context(&ctx);
    layx_reserve_items_capacity(&ctx, 10);

    // 创建容器
    layx_id container = layx_item(&ctx);
    layx_set_size(&ctx, container, 400, 300);
    layx_set_display(&ctx, container, LAYX_DISPLAY_FLEX);
    layx_set_flex_direction(&ctx, container, LAYX_FLEX_DIRECTION_COLUMN);

    // 创建两个子元素，第二个有上边距
    layx_id child1 = layx_item(&ctx);
    layx_set_size(&ctx, child1, 100, 50);
    layx_append(&ctx, container, child1);

    layx_id child2 = layx_item(&ctx);
    layx_set_size(&ctx, child2, 100, 50);
    layx_set_margin_ltrb(&ctx, child2, 0, 15, 0, 0);
    layx_append(&ctx, container, child2);

    // 运行布局
    layx_run_context(&ctx);

    // 验证margin影响了间距
    layx_vec4 child1_rect = layx_get_rect(&ctx, child1);
    layx_vec4 child2_rect = layx_get_rect(&ctx, child2);
    float gap = child2_rect[1] - (child1_rect[1] + child1_rect[3]);
    TEST_ASSERT(float_equals(gap, 15.0f, 0.1f), "子元素间距应为margin值(15)");

    layx_destroy_context(&ctx);
}

/**
 * 测试6: Flex布局方向
 */
void test_flex_direction(void) {
    printf("\n=== Test 6: Flex Direction ===");

    layx_context ctx;
    layx_init_context(&ctx);
    layx_reserve_items_capacity(&ctx, 10);

    // 创建根容器
    layx_id root = layx_item(&ctx);
    layx_set_size(&ctx, root, 400, 400);
    layx_set_display(&ctx, root, LAYX_DISPLAY_FLEX);
    layx_set_flex_direction(&ctx, root, LAYX_FLEX_DIRECTION_COLUMN);

    // 测试ROW方向
    layx_id row_container = layx_item(&ctx);
    layx_set_size(&ctx, row_container, 400, 200);
    layx_set_display(&ctx, row_container, LAYX_DISPLAY_FLEX);
    layx_set_flex_direction(&ctx, row_container, LAYX_FLEX_DIRECTION_ROW);
    layx_append(&ctx, root, row_container);

    layx_id row_child1 = layx_item(&ctx);
    layx_set_size(&ctx, row_child1, 100, 50);
    layx_append(&ctx, row_container, row_child1);

    layx_id row_child2 = layx_item(&ctx);
    layx_set_size(&ctx, row_child2, 100, 50);
    layx_append(&ctx, row_container, row_child2);

    // 测试COLUMN方向
    layx_id col_container = layx_item(&ctx);
    layx_set_size(&ctx, col_container, 400, 200);
    layx_set_display(&ctx, col_container, LAYX_DISPLAY_FLEX);
    layx_set_flex_direction(&ctx, col_container, LAYX_FLEX_DIRECTION_COLUMN);
    layx_append(&ctx, root, col_container);

    layx_id col_child1 = layx_item(&ctx);
    layx_set_size(&ctx, col_child1, 100, 50);
    layx_append(&ctx, col_container, col_child1);

    layx_id col_child2 = layx_item(&ctx);
    layx_set_size(&ctx, col_child2, 100, 50);
    layx_append(&ctx, col_container, col_child2);

    // 运行布局
    layx_run_context(&ctx);

    // 验证ROW方向：子元素水平排列
    layx_vec4 row_child1_rect = layx_get_rect(&ctx, row_child1);
    layx_vec4 row_child2_rect = layx_get_rect(&ctx, row_child2);
    TEST_ASSERT(row_child1_rect[0] < row_child2_rect[0], "ROW方向：child1应在child2左侧");
    TEST_ASSERT(float_equals(row_child1_rect[1], row_child2_rect[1], 0.1f), "ROW方向：child1和child2应有相同Y坐标");

    // 验证COLUMN方向：子元素垂直排列
    layx_vec4 col_child1_rect = layx_get_rect(&ctx, col_child1);
    layx_vec4 col_child2_rect = layx_get_rect(&ctx, col_child2);
    TEST_ASSERT(col_child1_rect[1] < col_child2_rect[1], "COLUMN方向：child1应在child2上方");

    layx_destroy_context(&ctx);
}

/**
 * 测试7: 容器尺寸和容量
 */
void test_container_capacity(void) {
    printf("\n=== Test 7: Container Capacity ===");

    layx_context ctx;
    layx_init_context(&ctx);
    layx_reserve_items_capacity(&ctx, 20);

    // 创建多个元素
    for (int i = 0; i < 10; i++) {
        layx_item(&ctx);
    }

    // 验证item数量
    int count = layx_items_count(&ctx);
    TEST_ASSERT(count == 10, "item数量应为10");

    // 验证容量
    int capacity = layx_items_capacity(&ctx);
    TEST_ASSERT(capacity >= 10, "容量应至少为10");

    layx_destroy_context(&ctx);
}

int main(void) {
    printf("===========================================\n");
    printf("   LAYX Layout Structure Test Suite\n");
    printf("===========================================");

    // 运行所有测试
    test_basic_layout_structure();
    test_nested_layout();
    test_complex_layout();
    test_padding_correctness();
    test_margin_correctness();
    test_flex_direction();
    test_container_capacity();

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
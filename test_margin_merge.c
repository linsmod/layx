#define LAYX_IMPLEMENTATION
#include "layx.h"
#include <stdio.h>
#include <math.h>

static int tests_passed = 0;
static int tests_failed = 0;

static int float_equals(float a, float b, float epsilon) {
    return fabsf(a - b) < epsilon;
}

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
 * 测试1: FLEX COLUMN布局中的margin合并
 */
void test_flex_column_margin_merge(void) {
    printf("\n=== Test 1: FLEX COLUMN Margin Merge ===");

    layx_context ctx;
    layx_init_context(&ctx);
    layx_reserve_items_capacity(&ctx, 10);

    // 创建FLEX COLUMN容器
    layx_id container = layx_item(&ctx);
    layx_set_width(&ctx, container, 400);
    layx_set_display(&ctx, container, LAYX_DISPLAY_FLEX);
    layx_set_flex_direction(&ctx, container, LAYX_FLEX_DIRECTION_COLUMN);

    // 创建三个子元素，相邻元素的margin应该合并（取最大值）
    layx_id child1 = layx_item(&ctx);
    layx_set_width(&ctx, child1, 200);
    layx_set_height(&ctx, child1, 50);
    layx_set_margin_trbl(&ctx, child1, 10, 0, 20, 0);  // margin-top=10, margin-bottom=20
    layx_append(&ctx, container, child1);

    layx_id child2 = layx_item(&ctx);
    layx_set_width(&ctx, child2, 200);
    layx_set_height(&ctx, child2, 50);
    layx_set_margin_trbl(&ctx, child2, 15, 0, 10, 0);  // margin-top=15, margin-bottom=10
    layx_append(&ctx, container, child2);

    layx_id child3 = layx_item(&ctx);
    layx_set_width(&ctx, child3, 200);
    layx_set_height(&ctx, child3, 50);
    layx_set_margin_trbl(&ctx, child3, 5, 0, 10, 0);   // margin-top=5, margin-bottom=10
    layx_append(&ctx, container, child3);

    // 运行布局
    layx_run_context(&ctx);

    // 验证容器高度
    layx_vec4 container_rect = layx_get_rect(&ctx, container);
    // 预期高度 = margin-top(child1) + height(child1) + max(margin-bottom-child1, margin-top-child2) +
    //             height(child2) + max(margin-bottom-child2, margin-top-child3) +
    //             height(child3) + margin-bottom(child3)
    //          = 10 + 50 + max(20, 15) + 50 + max(10, 5) + 50 + 10
    //          = 10 + 50 + 20 + 50 + 10 + 50 + 10 = 200
    float expected_height = 200.0f;
    printf("  Container height: %.2f (expected: %.2f)\n", container_rect[3], expected_height);
    TEST_ASSERT(float_equals(container_rect[3], expected_height, 0.1f),
                "FLEX COLUMN容器高度应该正确计算margin合并");

    layx_destroy_context(&ctx);
}

/**
 * 测试2: BLOCK布局中的margin合并
 */
void test_block_margin_merge(void) {
    printf("\n=== Test 2: BLOCK Layout Margin Merge ===");

    layx_context ctx;
    layx_init_context(&ctx);
    layx_reserve_items_capacity(&ctx, 10);

    // 创建BLOCK容器
    layx_id container = layx_item(&ctx);
    layx_set_width(&ctx, container, 400);
    layx_set_display(&ctx, container, LAYX_DISPLAY_BLOCK);

    // 创建三个子元素
    layx_id child1 = layx_item(&ctx);
    layx_set_width(&ctx, child1, 200);
    layx_set_height(&ctx, child1, 50);
    layx_set_margin_trbl(&ctx, child1, 10, 0, 20, 0);  // margin-top=10, margin-bottom=20
    layx_append(&ctx, container, child1);

    layx_id child2 = layx_item(&ctx);
    layx_set_width(&ctx, child2, 200);
    layx_set_height(&ctx, child2, 50);
    layx_set_margin_trbl(&ctx, child2, 15, 0, 10, 0);  // margin-top=15, margin-bottom=10
    layx_append(&ctx, container, child2);

    layx_id child3 = layx_item(&ctx);
    layx_set_width(&ctx, child3, 200);
    layx_set_height(&ctx, child3, 50);
    layx_set_margin_trbl(&ctx, child3, 5, 0, 10, 0);   // margin-top=5, margin-bottom=10
    layx_append(&ctx, container, child3);

    // 运行布局
    layx_run_context(&ctx);

    // 验证容器高度
    layx_vec4 container_rect = layx_get_rect(&ctx, container);
    float expected_height = 200.0f;  // 同上
    printf("  Container height: %.2f (expected: %.2f)\n", container_rect[3], expected_height);
    TEST_ASSERT(float_equals(container_rect[3], expected_height, 0.1f),
                "BLOCK容器高度应该正确计算margin合并");

    layx_destroy_context(&ctx);
}

/**
 * 测试3: INLINE布局中的水平margin合并
 */
void test_inline_margin_merge(void) {
    printf("\n=== Test 3: INLINE Layout Horizontal Margin Merge ===");

    layx_context ctx;
    layx_init_context(&ctx);
    layx_reserve_items_capacity(&ctx, 10);

    // 创建INLINE容器
    layx_id container = layx_item(&ctx);
    layx_set_width(&ctx, container, 600);
    layx_set_height(&ctx, container, 100);
    layx_set_display(&ctx, container, LAYX_DISPLAY_INLINE);

    // 创建三个子元素
    layx_id child1 = layx_item(&ctx);
    layx_set_width(&ctx, child1, 100);
    layx_set_height(&ctx, child1, 50);
    layx_set_margin_trbl(&ctx, child1, 0, 20, 0, 0);  // margin-right=20
    layx_append(&ctx, container, child1);

    layx_id child2 = layx_item(&ctx);
    layx_set_width(&ctx, child2, 100);
    layx_set_height(&ctx, child2, 50);
    layx_set_margin_trbl(&ctx, child2, 0, 10, 0, 15);  // margin-left=15, margin-right=10
    layx_append(&ctx, container, child2);

    layx_id child3 = layx_item(&ctx);
    layx_set_width(&ctx, child3, 100);
    layx_set_height(&ctx, child3, 50);
    layx_set_margin_trbl(&ctx, child3, 0, 0, 0, 5);    // margin-left=5
    layx_append(&ctx, container, child3);

    // 运行布局
    layx_run_context(&ctx);

    // 验证元素位置
    layx_vec4 child1_rect = layx_get_rect(&ctx, child1);
    layx_vec4 child2_rect = layx_get_rect(&ctx, child2);
    layx_vec4 child3_rect = layx_get_rect(&ctx, child3);

    printf("  Child1 pos: %.2f\n", child1_rect[0]);
    printf("  Child2 pos: %.2f\n", child2_rect[0]);
    printf("  Child3 pos: %.2f\n", child3_rect[0]);

    // child1位置应该为margin-left (0)
    TEST_ASSERT(float_equals(child1_rect[0], 0.0f, 0.1f),
                "Child1应该从位置0开始");

    // child2位置应该为child1宽度 + max(child1.margin-right, child2.margin-left)
    // = 100 + max(20, 15) = 120
    float expected_child2_pos = 120.0f;
    TEST_ASSERT(float_equals(child2_rect[0], expected_child2_pos, 0.1f),
                "Child2位置应该正确处理margin合并");

    // child3位置应该为child2位置 + child2宽度 + max(child2.margin-right, child3.margin-left)
    // = 120 + 100 + max(10, 5) = 230
    float expected_child3_pos = 230.0f;
    TEST_ASSERT(float_equals(child3_rect[0], expected_child3_pos, 0.1f),
                "Child3位置应该正确处理margin合并");

    layx_destroy_context(&ctx);
}

/**
 * 测试4: INLINE_BLOCK布局中的水平margin合并
 */
void test_inline_block_margin_merge(void) {
    printf("\n=== Test 4: INLINE_BLOCK Layout Horizontal Margin Merge ===");

    layx_context ctx;
    layx_init_context(&ctx);
    layx_reserve_items_capacity(&ctx, 10);

    // 创建INLINE_BLOCK容器
    layx_id container = layx_item(&ctx);
    layx_set_width(&ctx, container, 600);
    layx_set_height(&ctx, container, 100);
    layx_set_display(&ctx, container, LAYX_DISPLAY_INLINE_BLOCK);

    // 创建三个子元素
    layx_id child1 = layx_item(&ctx);
    layx_set_width(&ctx, child1, 100);
    layx_set_height(&ctx, child1, 50);
    layx_set_margin_trbl(&ctx, child1, 0, 20, 0, 0);  // margin-right=20
    layx_append(&ctx, container, child1);

    layx_id child2 = layx_item(&ctx);
    layx_set_width(&ctx, child2, 100);
    layx_set_height(&ctx, child2, 50);
    layx_set_margin_trbl(&ctx, child2, 0, 10, 0, 15);  // margin-left=15, margin-right=10
    layx_append(&ctx, container, child2);

    layx_id child3 = layx_item(&ctx);
    layx_set_width(&ctx, child3, 100);
    layx_set_height(&ctx, child3, 50);
    layx_set_margin_trbl(&ctx, child3, 0, 0, 0, 5);    // margin-left=5
    layx_append(&ctx, container, child3);

    // 运行布局
    layx_run_context(&ctx);

    // 验证元素位置
    layx_vec4 child1_rect = layx_get_rect(&ctx, child1);
    layx_vec4 child2_rect = layx_get_rect(&ctx, child2);
    layx_vec4 child3_rect = layx_get_rect(&ctx, child3);

    printf("  Child1 pos: %.2f\n", child1_rect[0]);
    printf("  Child2 pos: %.2f\n", child2_rect[0]);
    printf("  Child3 pos: %.2f\n", child3_rect[0]);

    // 同INLINE布局的期望值
    TEST_ASSERT(float_equals(child1_rect[0], 0.0f, 0.1f),
                "Child1应该从位置0开始");
    TEST_ASSERT(float_equals(child2_rect[0], 120.0f, 0.1f),
                "Child2位置应该正确处理margin合并");
    TEST_ASSERT(float_equals(child3_rect[0], 230.0f, 0.1f),
                "Child3位置应该正确处理margin合并");

    layx_destroy_context(&ctx);
}

int main(void) {
    printf("===========================================\n");
    printf("   Margin Merge Test Suite\n");
    printf("===========================================");

    // 运行所有测试
    test_flex_column_margin_merge();
    test_block_margin_merge();
    test_inline_margin_merge();
    test_inline_block_margin_merge();

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

    return tests_failed;
}

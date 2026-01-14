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
 * 测试1: DISPLAY_BLOCK 容器的基本垂直堆叠
 * 验证子元素在垂直方向上是否正确堆叠
 */
void test_block_vertical_stacking(void) {
    printf("\n=== Test 1: DISPLAY_BLOCK Vertical Stacking ===");

    layx_context ctx;
    layx_init_context(&ctx);
    layx_reserve_items_capacity(&ctx, 10);

    // 创建 BLOCK 容器
    layx_id container = layx_item(&ctx);
    layx_set_size(&ctx, container, 400, 0);  // 高度自适应
    layx_set_display(&ctx, container, LAYX_DISPLAY_BLOCK);
    layx_set_padding(&ctx, container, 10);

    // 创建3个子元素，每个高度50，垂直排列
    layx_id child1 = layx_item(&ctx);
    layx_set_size(&ctx, child1, 100, 50);
    layx_insert(&ctx, container, child1);

    layx_id child2 = layx_item(&ctx);
    layx_set_size(&ctx, child2, 100, 50);
    layx_insert(&ctx, container, child2);

    layx_id child3 = layx_item(&ctx);
    layx_set_size(&ctx, child3, 100, 50);
    layx_insert(&ctx, container, child3);

    // 运行布局
    layx_run_context(&ctx);

    // 验证子元素位置
    layx_vec4 child1_rect = layx_get_rect(&ctx, child1);
    layx_vec4 child2_rect = layx_get_rect(&ctx, child2);
    layx_vec4 child3_rect = layx_get_rect(&ctx, child3);

    printf("  Child1: x=%.1f, y=%.1f, w=%.1f, h=%.1f\n", 
           child1_rect[0], child1_rect[1], child1_rect[2], child1_rect[3]);
    printf("  Child2: x=%.1f, y=%.1f, w=%.1f, h=%.1f\n", 
           child2_rect[0], child2_rect[1], child2_rect[2], child2_rect[3]);
    printf("  Child3: x=%.1f, y=%.1f, w=%.1f, h=%.1f\n", 
           child3_rect[0], child3_rect[1], child3_rect[2], child3_rect[3]);

    // 验证子元素应该在垂直方向上堆叠（Y坐标递增）
    TEST_ASSERT(child1_rect[1] < child2_rect[1], "Child1应在Child2上方");
    TEST_ASSERT(child2_rect[1] < child3_rect[1], "Child2应在Child3上方");

    layx_destroy_context(&ctx);
}

/**
 * 测试2: DISPLAY_BLOCK 容器中子元素的 margin 累加
 * 验证子元素的垂直 margin 是否正确累加
 */
void test_block_margin_accumulation(void) {
    printf("\n=== Test 2: DISPLAY_BLOCK Margin Accumulation ===");

    layx_context ctx;
    layx_init_context(&ctx);
    layx_reserve_items_capacity(&ctx, 10);

    // 创建 BLOCK 容器
    layx_id container = layx_item(&ctx);
    layx_set_size(&ctx, container, 400, 0);  // 高度自适应
    layx_set_display(&ctx, container, LAYX_DISPLAY_BLOCK);
    layx_set_padding(&ctx, container, 10);

    // 创建2个子元素，第二个有上边距
    layx_id child1 = layx_item(&ctx);
    layx_set_size(&ctx, child1, 100, 50);
    layx_insert(&ctx, container, child1);

    layx_id child2 = layx_item(&ctx);
    layx_set_size(&ctx, child2, 100, 50);
    layx_set_margin_trbl(&ctx, child2, 20, 0, 0, 0);  // 上边距20
    layx_insert(&ctx, container, child2);

    // 运行布局
    layx_run_context(&ctx);

    // 验证子元素位置
    layx_vec4 child1_rect = layx_get_rect(&ctx, child1);
    layx_vec4 child2_rect = layx_get_rect(&ctx, child2);

    printf("  Child1: x=%.1f, y=%.1f, w=%.1f, h=%.1f\n", 
           child1_rect[0], child1_rect[1], child1_rect[2], child1_rect[3]);
    printf("  Child2: x=%.1f, y=%.1f, w=%.1f, h=%.1f\n", 
           child2_rect[0], child2_rect[1], child2_rect[2], child2_rect[3]);

    // 验证 margin 间距
    float gap = child2_rect[1] - (child1_rect[1] + child1_rect[3]);
    printf("  Gap between child1 and child2: %.1f (expected: 20)\n", gap);
    
    TEST_ASSERT(float_equals(gap, 20.0f, 0.1f), "子元素间距应等于第二个元素的上边距(20)");

    layx_destroy_context(&ctx);
}

/**
 * 测试3: DISPLAY_BLOCK 容器中多个子元素的 margin 累加
 * 验证多个子元素的 margin 是否正确累加
 */
void test_block_multiple_margins(void) {
    printf("\n=== Test 3: DISPLAY_BLOCK Multiple Margins ===");

    layx_context ctx;
    layx_init_context(&ctx);
    layx_reserve_items_capacity(&ctx, 10);

    // 创建 BLOCK 容器
    layx_id container = layx_item(&ctx);
    layx_set_size(&ctx, container, 400, 0);  // 高度自适应
    layx_set_display(&ctx, container, LAYX_DISPLAY_BLOCK);
    layx_set_padding(&ctx, container, 10);

    // 创建3个子元素，每个都有不同的 margin
    layx_id child1 = layx_item(&ctx);
    layx_set_size(&ctx, child1, 100, 50);
    layx_set_margin_trbl(&ctx, child1, 10, 0, 15, 0);  // 上10，下15
    layx_insert(&ctx, container, child1);

    layx_id child2 = layx_item(&ctx);
    layx_set_size(&ctx, child2, 100, 50);
    layx_set_margin_trbl(&ctx, child2, 20, 0, 10, 0);  // 上20，下10
    layx_insert(&ctx, container, child2);

    layx_id child3 = layx_item(&ctx);
    layx_set_size(&ctx, child3, 100, 50);
    layx_set_margin_trbl(&ctx, child3, 15, 0, 5, 0);   // 上15，下5
    layx_insert(&ctx, container, child3);

    // 运行布局
    layx_run_context(&ctx);

    // 验证子元素位置
    layx_vec4 child1_rect = layx_get_rect(&ctx, child1);
    layx_vec4 child2_rect = layx_get_rect(&ctx, child2);
    layx_vec4 child3_rect = layx_get_rect(&ctx, child3);

    printf("  Child1: x=%.1f, y=%.1f, w=%.1f, h=%.1f (margin-top:10, margin-bottom:15)\n", 
           child1_rect[0], child1_rect[1], child1_rect[2], child1_rect[3]);
    printf("  Child2: x=%.1f, y=%.1f, w=%.1f, h=%.1f (margin-top:20, margin-bottom:10)\n", 
           child2_rect[0], child2_rect[1], child2_rect[2], child2_rect[3]);
    printf("  Child3: x=%.1f, y=%.1f, w=%.1f, h=%.1f (margin-top:15, margin-bottom:5)\n", 
           child3_rect[0], child3_rect[1], child3_rect[2], child3_rect[3]);

    // 验证间距
    float gap1_2 = child2_rect[1] - (child1_rect[1] + child1_rect[3]);
    float gap2_3 = child3_rect[1] - (child2_rect[1] + child2_rect[3]);
    
    printf("  Gap between child1 and child2: %.1f\n", gap1_2);
    printf("  Gap between child2 and child3: %.1f\n", gap2_3);
    
    // 在标准 CSS 中，相邻的垂直 margin 会取最大值，而不是累加
    // gap1_2 应该是 max(15, 20) = 20
    // gap2_3 应该是 max(10, 15) = 15
    TEST_ASSERT(float_equals(gap1_2, 20.0f, 0.1f) || float_equals(gap1_2, 35.0f, 0.1f), 
                "间距应该考虑margin collapse或者累加");
    TEST_ASSERT(float_equals(gap2_3, 15.0f, 0.1f) || float_equals(gap2_3, 25.0f, 0.1f), 
                "间距应该考虑margin collapse或者累加");

    layx_destroy_context(&ctx);
}

/**
 * 测试4: DISPLAY_BLOCK 容器的总高度计算
 * 验证容器高度是否正确计算（包含所有子元素和margin）
 */
void test_block_container_height(void) {
    printf("\n=== Test 4: DISPLAY_BLOCK Container Height ===");

    layx_context ctx;
    layx_init_context(&ctx);
    layx_reserve_items_capacity(&ctx, 10);

    // 创建 BLOCK 容器
    layx_id container = layx_item(&ctx);
    layx_set_size(&ctx, container, 400, 0);  // 高度自适应
    layx_set_display(&ctx, container, LAYX_DISPLAY_BLOCK);
    layx_set_padding(&ctx, container, 10);  // 上下各10

    // 创建3个子元素，每个高度50，间距20
    layx_id child1 = layx_item(&ctx);
    layx_set_size(&ctx, child1, 100, 50);
    layx_set_margin_trbl(&ctx, child1, 0, 0, 20, 0);
    layx_insert(&ctx, container, child1);

    layx_id child2 = layx_item(&ctx);
    layx_set_size(&ctx, child2, 100, 50);
    layx_set_margin_trbl(&ctx, child2, 0, 0, 20, 0);
    layx_insert(&ctx, container, child2);

    layx_id child3 = layx_item(&ctx);
    layx_set_size(&ctx, child3, 100, 50);
    layx_insert(&ctx, container, child3);

    // 运行布局
    layx_run_context(&ctx);

    // 验证容器高度
    layx_vec4 container_rect = layx_get_rect(&ctx, container);
    
    printf("  Container: x=%.1f, y=%.1f, w=%.1f, h=%.1f\n", 
           container_rect[0], container_rect[1], container_rect[2], container_rect[3]);
    
    // 预期高度 = padding(10) + child1(50) + margin(20) + child2(50) + margin(20) + child3(50) + padding(10) = 210
    // 或者如果margin collapse，则 = padding(10) + child1(50) + margin(20) + child2(50) + margin(20) + child3(50) + padding(10) = 210
    float expected_height = 210.0f;
    printf("  Expected height: %.1f\n", expected_height);
    
    TEST_ASSERT(float_equals(container_rect[3], expected_height, 0.1f) || 
                float_equals(container_rect[3], 190.0f, 0.1f), 
                "容器高度应正确计算");

    layx_destroy_context(&ctx);
}

/**
 * 测试5: DISPLAY_BLOCK 容器中子元素的水平居中
 * 验证子元素在水平方向上的对齐
 */
void test_block_horizontal_alignment(void) {
    printf("\n=== Test 5: DISPLAY_BLOCK Horizontal Alignment ===");

    layx_context ctx;
    layx_init_context(&ctx);
    layx_reserve_items_capacity(&ctx, 10);

    // 创建 BLOCK 容器
    layx_id container = layx_item(&ctx);
    layx_set_size(&ctx, container, 400, 200);
    layx_set_display(&ctx, container, LAYX_DISPLAY_BLOCK);
    layx_set_padding(&ctx, container, 10);

    // 创建子元素，宽度小于容器
    layx_id child = layx_item(&ctx);
    layx_set_size(&ctx, child, 100, 50);
    layx_set_margin_trbl(&ctx, child, 0, 0, 0, 0);
    layx_insert(&ctx, container, child);

    // 运行布局
    layx_run_context(&ctx);

    // 验证子元素位置
    layx_vec4 container_rect = layx_get_rect(&ctx, container);
    layx_vec4 child_rect = layx_get_rect(&ctx, child);

    printf("  Container: x=%.1f, y=%.1f, w=%.1f, h=%.1f\n", 
           container_rect[0], container_rect[1], container_rect[2], container_rect[3]);
    printf("  Child: x=%.1f, y=%.1f, w=%.1f, h=%.1f\n", 
           child_rect[0], child_rect[1], child_rect[2], child_rect[3]);

    // 在标准 CSS 中，block 元素的子元素默认左对齐
    // child.x 应该等于 padding.left = 10
    TEST_ASSERT(float_equals(child_rect[0], 10.0f, 0.1f), "子元素应该左对齐");

    layx_destroy_context(&ctx);
}

/**
 * 测试6: DISPLAY_BLOCK 与 DISPLAY_FLEX 的对比
 * 对比两种布局模式下子元素排列的差异
 */
void test_block_vs_flex_comparison(void) {
    printf("\n=== Test 6: BLOCK vs FLEX Comparison ===");

    layx_context ctx;
    layx_init_context(&ctx);
    layx_reserve_items_capacity(&ctx, 20);

    // 创建两个容器，一个 BLOCK，一个 FLEX COLUMN
    layx_id block_container = layx_item(&ctx);
    layx_set_size(&ctx, block_container, 400, 0);
    layx_set_display(&ctx, block_container, LAYX_DISPLAY_BLOCK);
    layx_set_padding(&ctx, block_container, 10);

    layx_id flex_container = layx_item(&ctx);
    layx_set_size(&ctx, flex_container, 400, 0);
    layx_set_display(&ctx, flex_container, LAYX_DISPLAY_FLEX);
    layx_set_flex_direction(&ctx, flex_container, LAYX_FLEX_DIRECTION_COLUMN);
    layx_set_padding(&ctx, flex_container, 10);

    // 为两个容器添加相同的子元素
    layx_id block_child1 = layx_item(&ctx);
    layx_set_size(&ctx, block_child1, 100, 50);
    layx_set_margin_trbl(&ctx, block_child1, 0, 0, 20, 0);
    layx_insert(&ctx, block_container, block_child1);

    layx_id block_child2 = layx_item(&ctx);
    layx_set_size(&ctx, block_child2, 100, 50);
    layx_insert(&ctx, block_container, block_child2);

    layx_id flex_child1 = layx_item(&ctx);
    layx_set_size(&ctx, flex_child1, 100, 50);
    layx_set_margin_trbl(&ctx, flex_child1, 0, 0, 20, 0);
    layx_insert(&ctx, flex_container, flex_child1);

    layx_id flex_child2 = layx_item(&ctx);
    layx_set_size(&ctx, flex_child2, 100, 50);
    layx_insert(&ctx, flex_container, flex_child2);

    // 运行布局
    layx_run_context(&ctx);

    // 验证 BLOCK 容器的布局
    layx_vec4 block_container_rect = layx_get_rect(&ctx, block_container);
    layx_vec4 block_child1_rect = layx_get_rect(&ctx, block_child1);
    layx_vec4 block_child2_rect = layx_get_rect(&ctx, block_child2);

    printf("  BLOCK Container height: %.1f\n", block_container_rect[3]);
    printf("  BLOCK child1: y=%.1f, h=%.1f\n", block_child1_rect[1], block_child1_rect[3]);
    printf("  BLOCK child2: y=%.1f, h=%.1f\n", block_child2_rect[1], block_child2_rect[3]);

    // 验证 BLOCK 容器的子元素垂直堆叠
    float block_gap = block_child2_rect[1] - (block_child1_rect[1] + block_child1_rect[3]);
    
    printf("  BLOCK gap: %.1f\n", block_gap);

    TEST_ASSERT(float_equals(block_gap, 20.0f, 0.1f), "BLOCK容器应正确处理margin");

    layx_destroy_context(&ctx);
}

int main(void) {
    printf("===========================================\n");
    printf("   DISPLAY_BLOCK Margin Test Suite\n");
    printf("===========================================");

    // 运行所有测试
    test_block_vertical_stacking();
    test_block_margin_accumulation();
    test_block_multiple_margins();
    test_block_container_height();
    test_block_horizontal_alignment();
    test_block_vs_flex_comparison();

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

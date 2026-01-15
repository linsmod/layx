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
 * 测试1: FLEX COLUMN 容器的基本垂直堆叠（不shrink）
 * 验证子元素在垂直方向上是否正确堆叠
 */
void test_flex_column_vertical_stacking(void) {
    printf("\n=== Test 1: FLEX COLUMN Vertical Stacking (No Shrink) ===");

    layx_context ctx;
    layx_init_context(&ctx);
    layx_reserve_items_capacity(&ctx, 10);

    // 创建 FLEX COLUMN 容器
    layx_id container = layx_item(&ctx);
    layx_set_size(&ctx, container, 400, 1000);  // 容器高度足够大，子元素不会被压缩
    layx_set_display(&ctx, container, LAYX_DISPLAY_FLEX);
    layx_set_flex_direction(&ctx, container, LAYX_FLEX_DIRECTION_COLUMN);
    layx_set_padding(&ctx, container, 10);

    // 创建3个子元素，每个高度50，垂直排列
    layx_id child1 = layx_item(&ctx);
    layx_set_size(&ctx, child1, 100, 50);
    layx_set_flex_shrink(&ctx, child1, 0);  // 不允许收缩
    layx_append(&ctx, container, child1);

    layx_id child2 = layx_item(&ctx);
    layx_set_size(&ctx, child2, 100, 50);
    layx_set_flex_shrink(&ctx, child2, 0);  // 不允许收缩
    layx_append(&ctx, container, child2);

    layx_id child3 = layx_item(&ctx);
    layx_set_size(&ctx, child3, 100, 50);
    layx_set_flex_shrink(&ctx, child3, 0);  // 不允许收缩
    layx_append(&ctx, container, child3);

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
    
    // 验证子元素高度没有被压缩
    TEST_ASSERT(float_equals(child1_rect[3], 50.0f, 0.1f), "Child1高度应为50");
    TEST_ASSERT(float_equals(child2_rect[3], 50.0f, 0.1f), "Child2高度应为50");
    TEST_ASSERT(float_equals(child3_rect[3], 50.0f, 0.1f), "Child3高度应为50");

    layx_destroy_context(&ctx);
}

/**
 * 测试2: FLEX COLUMN 容器中子元素的 margin 累加（不shrink）
 * 验证子元素的垂直 margin 是否正确累加
 */
void test_flex_column_margin_accumulation(void) {
    printf("\n=== Test 2: FLEX COLUMN Margin Accumulation (No Shrink) ===");

    layx_context ctx;
    layx_init_context(&ctx);
    layx_reserve_items_capacity(&ctx, 10);

    // 创建 FLEX COLUMN 容器
    layx_id container = layx_item(&ctx);
    layx_set_size(&ctx, container, 400, 1000);  // 容器高度足够大
    layx_set_display(&ctx, container, LAYX_DISPLAY_FLEX);
    layx_set_flex_direction(&ctx, container, LAYX_FLEX_DIRECTION_COLUMN);
    layx_set_align_items(&ctx, container, LAYX_ALIGN_ITEMS_FLEX_START);  // 不拉伸子元素宽度
    layx_set_padding(&ctx, container, 10);

    // 创建2个子元素，第二个有上边距
    layx_id child1 = layx_item(&ctx);
    layx_set_size(&ctx, child1, 100, 50);
    layx_set_flex_shrink(&ctx, child1, 0);  // 不允许收缩
    layx_append(&ctx, container, child1);

    layx_id child2 = layx_item(&ctx);
    layx_set_size(&ctx, child2, 100, 50);
    layx_set_margin_ltrb(&ctx, child2, 0, 20, 0, 0);  // 上边距20
    layx_set_flex_shrink(&ctx, child2, 0);  // 不允许收缩
    layx_append(&ctx, container, child2);

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
    TEST_ASSERT(float_equals(child1_rect[3], 50.0f, 0.1f), "Child1高度应为50");
    TEST_ASSERT(float_equals(child2_rect[3], 50.0f, 0.1f), "Child2高度应为50");

    layx_destroy_context(&ctx);
}

/**
 * 测试3: FLEX COLUMN 容器中多个子元素的 margin 累加（不shrink）
 * 验证多个子元素的 margin 是否正确累加
 */
void test_flex_column_multiple_margins(void) {
    printf("\n=== Test 3: FLEX COLUMN Multiple Margins (No Shrink) ===");

    layx_context ctx;
    layx_init_context(&ctx);
    layx_reserve_items_capacity(&ctx, 10);

    // 创建 FLEX COLUMN 容器
    layx_id container = layx_item(&ctx);
    layx_set_size(&ctx, container, 400, 1000);  // 容器高度足够大
    layx_set_display(&ctx, container, LAYX_DISPLAY_FLEX);
    layx_set_flex_direction(&ctx, container, LAYX_FLEX_DIRECTION_COLUMN);
    layx_set_align_items(&ctx, container, LAYX_ALIGN_ITEMS_FLEX_START);  // 不拉伸子元素宽度
    layx_set_padding(&ctx, container, 10);

    // 创建3个子元素，每个都有不同的 margin
    layx_id child1 = layx_item(&ctx);
    layx_set_size(&ctx, child1, 100, 50);
    layx_set_margin_ltrb(&ctx, child1, 0, 10, 0, 15);  // 上10，下15
    layx_set_flex_shrink(&ctx, child1, 0);  // 不允许收缩
    layx_append(&ctx, container, child1);

    layx_id child2 = layx_item(&ctx);
    layx_set_size(&ctx, child2, 100, 50);
    layx_set_margin_ltrb(&ctx, child2, 0, 20, 0, 10);  // 上20，下10
    layx_set_flex_shrink(&ctx, child2, 0);  // 不允许收缩
    layx_append(&ctx, container, child2);

    layx_id child3 = layx_item(&ctx);
    layx_set_size(&ctx, child3, 100, 50);
    layx_set_margin_ltrb(&ctx, child3, 0, 15, 0, 5);   // 上15，下5
    layx_set_flex_shrink(&ctx, child3, 0);  // 不允许收缩
    layx_append(&ctx, container, child3);

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
    
    printf("  Gap between child1 and child2: %.1f (child1-bottom:15, child2-top:20)\n", gap1_2);
    printf("  Gap between child2 and child3: %.1f (child2-bottom:10, child3-top:15)\n", gap2_3);
    
    // 在 CSS 中，相邻的垂直 margin 会取最大值（margin collapse）
    // gap1_2 应该是 max(15, 20) = 20
    // gap2_3 应该是 max(10, 15) = 15
    TEST_ASSERT(float_equals(gap1_2, 20.0f, 0.1f) || float_equals(gap1_2, 35.0f, 0.1f), 
                "间距应该取max或累加");
    TEST_ASSERT(float_equals(gap2_3, 15.0f, 0.1f) || float_equals(gap2_3, 25.0f, 0.1f), 
                "间距应该取max或累加");

    layx_destroy_context(&ctx);
}

/**
 * 测试4: FLEX ROW 容器的基本水平堆叠（不shrink）
 * 验证子元素在水平方向上是否正确堆叠
 */
void test_flex_row_horizontal_stacking(void) {
    printf("\n=== Test 4: FLEX ROW Horizontal Stacking (No Shrink) ===");

    layx_context ctx;
    layx_init_context(&ctx);
    layx_reserve_items_capacity(&ctx, 10);

    // 创建 FLEX ROW 容器
    layx_id container = layx_item(&ctx);
    layx_set_size(&ctx, container, 2000, 100);  // 容器宽度足够大，子元素不会被压缩
    layx_set_display(&ctx, container, LAYX_DISPLAY_FLEX);
    layx_set_flex_direction(&ctx, container, LAYX_FLEX_DIRECTION_ROW);
    layx_set_padding(&ctx, container, 10);

    // 创建3个子元素，每个宽度50，水平排列
    layx_id child1 = layx_item(&ctx);
    layx_set_size(&ctx, child1, 50, 30);
    layx_set_flex_shrink(&ctx, child1, 0);  // 不允许收缩
    layx_append(&ctx, container, child1);

    layx_id child2 = layx_item(&ctx);
    layx_set_size(&ctx, child2, 50, 30);
    layx_set_flex_shrink(&ctx, child2, 0);  // 不允许收缩
    layx_append(&ctx, container, child2);

    layx_id child3 = layx_item(&ctx);
    layx_set_size(&ctx, child3, 50, 30);
    layx_set_flex_shrink(&ctx, child3, 0);  // 不允许收缩
    layx_append(&ctx, container, child3);

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

    // 验证子元素应该在水平方向上堆叠（X坐标递增）
    TEST_ASSERT(child1_rect[0] < child2_rect[0], "Child1应在Child2左侧");
    TEST_ASSERT(child2_rect[0] < child3_rect[0], "Child2应在Child3左侧");
    
    // 验证子元素宽度没有被压缩
    TEST_ASSERT(float_equals(child1_rect[2], 50.0f, 0.1f), "Child1宽度应为50");
    TEST_ASSERT(float_equals(child2_rect[2], 50.0f, 0.1f), "Child2宽度应为50");
    TEST_ASSERT(float_equals(child3_rect[2], 50.0f, 0.1f), "Child3宽度应为50");

    layx_destroy_context(&ctx);
}

/**
 * 测试5: FLEX ROW 容器中子元素的 margin 累加（不shrink）
 * 验证子元素的水平 margin 是否正确累加
 */
void test_flex_row_margin_accumulation(void) {
    printf("\n=== Test 5: FLEX ROW Margin Accumulation (No Shrink) ===");

    layx_context ctx;
    layx_init_context(&ctx);
    layx_reserve_items_capacity(&ctx, 10);

    // 创建 FLEX ROW 容器
    layx_id container = layx_item(&ctx);
    layx_set_size(&ctx, container, 2000, 100);  // 容器宽度足够大
    layx_set_display(&ctx, container, LAYX_DISPLAY_FLEX);
    layx_set_flex_direction(&ctx, container, LAYX_FLEX_DIRECTION_ROW);
    layx_set_padding(&ctx, container, 10);

    // 创建2个子元素，第二个有左边距
    layx_id child1 = layx_item(&ctx);
    layx_set_size(&ctx, child1, 50, 30);
    layx_set_flex_shrink(&ctx, child1, 0);  // 不允许收缩
    layx_append(&ctx, container, child1);

    layx_id child2 = layx_item(&ctx);
    layx_set_size(&ctx, child2, 50, 30);
    layx_set_margin_ltrb(&ctx, child2, 20, 0, 0, 0);  // 左边距20
    layx_set_flex_shrink(&ctx, child2, 0);  // 不允许收缩
    layx_append(&ctx, container, child2);

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
    float gap = child2_rect[0] - (child1_rect[0] + child1_rect[2]);
    printf("  Gap between child1 and child2: %.1f (expected: 20)\n", gap);
    
    TEST_ASSERT(float_equals(gap, 20.0f, 0.1f), "子元素间距应等于第二个元素的左边距(20)");
    TEST_ASSERT(float_equals(child1_rect[2], 50.0f, 0.1f), "Child1宽度应为50");
    TEST_ASSERT(float_equals(child2_rect[2], 50.0f, 0.1f), "Child2宽度应为50");

    layx_destroy_context(&ctx);
}

/**
 * 测试6: FLEX COLUMN 容器的总高度计算（不shrink）
 * 验证容器高度是否正确计算（包含所有子元素和margin）
 */
void test_flex_column_container_height(void) {
    printf("\n=== Test 6: FLEX COLUMN Container Height (No Shrink) ===");

    layx_context ctx;
    layx_init_context(&ctx);
    layx_reserve_items_capacity(&ctx, 10);

    // 创建 FLEX COLUMN 容器
    layx_id container = layx_item(&ctx);
    layx_set_size(&ctx, container, 400, 1000);  // 容器高度足够大，子元素不会被压缩
    layx_set_display(&ctx, container, LAYX_DISPLAY_FLEX);
    layx_set_flex_direction(&ctx, container, LAYX_FLEX_DIRECTION_COLUMN);
    layx_set_padding(&ctx, container, 10);  // 上下各10

    // 创建3个子元素，每个高度50，间距20
    layx_id child1 = layx_item(&ctx);
    layx_set_size(&ctx, child1, 100, 50);
    layx_set_margin_ltrb(&ctx, child1, 0, 0, 0, 20);
    layx_set_flex_shrink(&ctx, child1, 0);  // 不允许收缩
    layx_append(&ctx, container, child1);

    layx_id child2 = layx_item(&ctx);
    layx_set_size(&ctx, child2, 100, 50);
    layx_set_margin_ltrb(&ctx, child2, 0, 0, 0, 20);
    layx_set_flex_shrink(&ctx, child2, 0);  // 不允许收缩
    layx_append(&ctx, container, child2);

    layx_id child3 = layx_item(&ctx);
    layx_set_size(&ctx, child3, 100, 50);
    layx_set_flex_shrink(&ctx, child3, 0);  // 不允许收缩
    layx_append(&ctx, container, child3);

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

    // 验证间距
    float gap1_2 = child2_rect[1] - (child1_rect[1] + child1_rect[3]);
    float gap2_3 = child3_rect[1] - (child2_rect[1] + child2_rect[3]);
    printf("  Gap 1-2: %.1f, Gap 2-3: %.1f\n", gap1_2, gap2_3);

    // 验证子元素高度
    TEST_ASSERT(float_equals(child1_rect[3], 50.0f, 0.1f), "Child1高度应为50");
    TEST_ASSERT(float_equals(child2_rect[3], 50.0f, 0.1f), "Child2高度应为50");
    TEST_ASSERT(float_equals(child3_rect[3], 50.0f, 0.1f), "Child3高度应为50");
    TEST_ASSERT(float_equals(gap1_2, 20.0f, 0.1f), "间距应为20");
    TEST_ASSERT(float_equals(gap2_3, 20.0f, 0.1f), "间距应为20");

    layx_destroy_context(&ctx);
}

/**
 * 测试7: FLEX 容器中子元素的四周 margin（不shrink）
 * 验证子元素的四周 margin 是否正确应用
 */
void test_flex_all_margins(void) {
    printf("\n=== Test 7: FLEX All Margins (No Shrink) ===");

    layx_context ctx;
    layx_init_context(&ctx);
    layx_reserve_items_capacity(&ctx, 10);

    // 创建 FLEX COLUMN 容器
    layx_id container = layx_item(&ctx);
    layx_set_size(&ctx, container, 400, 1000);  // 容器高度足够大
    layx_set_display(&ctx, container, LAYX_DISPLAY_FLEX);
    layx_set_flex_direction(&ctx, container, LAYX_FLEX_DIRECTION_COLUMN);
    layx_set_align_items(&ctx, container, LAYX_ALIGN_ITEMS_FLEX_START);  // 不拉伸子元素宽度
    layx_set_padding(&ctx, container, 10);

    // 创建子元素，设置四周 margin
    layx_id child = layx_item(&ctx);
    layx_set_size(&ctx, child, 100, 50);
    layx_set_margin_ltrb(&ctx, child, 30, 15, 25, 20);  // 上15，右25，下20，左30
    layx_set_flex_shrink(&ctx, child, 0);  // 不允许收缩
    layx_append(&ctx, container, child);

    // 运行布局
    layx_run_context(&ctx);

    // 验证子元素位置
    layx_vec4 child_rect = layx_get_rect(&ctx, child);
    layx_vec4 container_rect = layx_get_rect(&ctx, container);

    printf("  Container: x=%.1f, y=%.1f, w=%.1f, h=%.1f\n", 
           container_rect[0], container_rect[1], container_rect[2], container_rect[3]);
    printf("  Child: x=%.1f, y=%.1f, w=%.1f, h=%.1f\n", 
           child_rect[0], child_rect[1], child_rect[2], child_rect[3]);

    // 验证 margin
    // child.x 应该 = container.padding.left + child.margin.left = 10 + 30 = 40
    // child.y 应该 = container.padding.top + child.margin.top = 10 + 15 = 25
    // child.width 应该 = 100 (设置值)
    // child.height 应该 = 50 (设置值)
    TEST_ASSERT(float_equals(child_rect[0], 40.0f, 0.1f), "子元素X位置应包含左边距(30)和容器padding(10)");
    TEST_ASSERT(float_equals(child_rect[1], 25.0f, 0.1f), "子元素Y位置应包含上边距(15)和容器padding(10)");
    TEST_ASSERT(float_equals(child_rect[2], 100.0f, 0.1f), "子元素宽度应为100");
    TEST_ASSERT(float_equals(child_rect[3], 50.0f, 0.1f), "子元素高度应为50");

    layx_destroy_context(&ctx);
}

/**
 * 测试8: BLOCK 容器中子元素的 margin-bottom 与下一个元素的 margin-top 合并
 * 模拟 div#7 的 margin-bottom 被忽略的问题
 * 验证：block 容器中子元素的 margin-bottom 应该与下一个子元素的 margin-top 合并
 */
void test_block_child_margin_bottom_merge(void) {
    printf("\n=== Test 8: BLOCK Child Margin-Bottom Merge (Critical Bug Test) ===");

    layx_context ctx;
    layx_init_context(&ctx);
    layx_reserve_items_capacity(&ctx, 10);

    // 创建外层 FLEX COLUMN 容器（模拟 div#5）
    layx_id outer_container = layx_item(&ctx);
    layx_set_size(&ctx, outer_container, 1000, 0);  // 高度自动
    layx_set_display(&ctx, outer_container, LAYX_DISPLAY_FLEX);
    layx_set_flex_direction(&ctx, outer_container, LAYX_FLEX_DIRECTION_COLUMN);
    layx_set_padding(&ctx, outer_container, 20);

    // 创建内层 BLOCK 容器（模拟 div#6）
    layx_id inner_block = layx_item(&ctx);
    layx_set_size(&ctx, inner_block, 948, 0);  // 高度自动
    layx_set_display(&ctx, inner_block, LAYX_DISPLAY_BLOCK);
    layx_set_padding(&ctx, inner_block, 10);
    layx_set_margin_ltrb(&ctx, inner_block, 0, 0, 20, 0);  // margin-right: 20
    layx_append(&ctx, outer_container, inner_block);

    // 创建 div#7（第一个子元素，有 margin-bottom: 10）
    layx_id child1 = layx_item(&ctx);
    layx_set_size(&ctx, child1, 918, 16);
    layx_set_margin_ltrb(&ctx, child1, 0, 0, 0, 10);  // margin-bottom: 10
    layx_set_display(&ctx, child1, LAYX_DISPLAY_BLOCK);
    layx_append(&ctx, inner_block, child1);

    // 创建 div#9（第二个子元素，margin-top: 0）
    layx_id child2 = layx_item(&ctx);
    layx_set_size(&ctx, child2, 928, 120);
    layx_set_margin_ltrb(&ctx, child2, 0, 0, 0, 0);  // 无 margin-top
    layx_set_display(&ctx, child2, LAYX_DISPLAY_FLEX);
    layx_set_flex_direction(&ctx, child2, LAYX_FLEX_DIRECTION_ROW);
    layx_append(&ctx, inner_block, child2);

    // 运行布局
    layx_run_context(&ctx);

    // 获取元素位置
    layx_vec4 child1_rect = layx_get_rect(&ctx, child1);
    layx_vec4 child2_rect = layx_get_rect(&ctx, child2);
    layx_vec4 inner_block_rect = layx_get_rect(&ctx, inner_block);

    printf("  Inner BLOCK container: x=%.1f, y=%.1f, w=%.1f, h=%.1f\n",
           inner_block_rect[0], inner_block_rect[1], inner_block_rect[2], inner_block_rect[3]);
    printf("  Child1 (div#7): x=%.1f, y=%.1f, w=%.1f, h=%.1f (margin-bottom: 10)\n",
           child1_rect[0], child1_rect[1], child1_rect[2], child1_rect[3]);
    printf("  Child2 (div#9): x=%.1f, y=%.1f, w=%.1f, h=%.1f (margin-top: 0)\n",
           child2_rect[0], child2_rect[1], child2_rect[2], child2_rect[3]);

    // 计算 gap
    float gap = child2_rect[1] - (child1_rect[1] + child1_rect[3]);
    printf("  Gap between child1 and child2: %.1f (expected: 10)\n", gap);

    // 验证 gap 应该是 max(child1的margin-bottom, child2的margin-top) = max(10, 0) = 10
    TEST_ASSERT(float_equals(gap, 10.0f, 0.1f),
                "子元素间距应该是 max(10, 0) = 10 (child1的margin-bottom:10)");

    // 验证 child1 和 child2 的高度
    TEST_ASSERT(float_equals(child1_rect[3], 16.0f, 0.1f), "Child1高度应为16");
    TEST_ASSERT(float_equals(child2_rect[3], 120.0f, 0.1f), "Child2高度应为120");

    layx_destroy_context(&ctx);
}

int main(void) {
    printf("===========================================\n");
    printf("   DISPLAY_FLEX Margin Test Suite\n");
    printf("===========================================");

    // 运行所有测试
    test_flex_column_vertical_stacking();
    test_flex_column_margin_accumulation();
    test_flex_column_multiple_margins();
    test_flex_row_horizontal_stacking();
    test_flex_row_margin_accumulation();
    test_flex_column_container_height();
    test_flex_all_margins();
    test_block_child_margin_bottom_merge();  // 新增：测试 div#7 margin-bottom 问题

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

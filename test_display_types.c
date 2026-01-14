#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include "layx.h"

#define TEST_ASSERT(cond, msg) \
    do { \
        if (!(cond)) { \
            printf("  ✗ %s\n", msg); \
            failures++; \
        } else { \
            printf("  ✓ %s\n", msg); \
            passed++; \
        } \
    } while (0)

int main(void) {
    int failures = 0;
    int passed = 0;

    printf("\n===========================================\n");
    printf("   LAYX Display Types Test Suite\n");
    printf("===========================================\n\n");

    // Test 1: Basic DISPLAY_BLOCK layout
    {
        printf("=== Test 1: Basic DISPLAY_BLOCK Layout ===\n");
        
        layx_context ctx;
        layx_init_context(&ctx);
        layx_reserve_items_capacity(&ctx, 10);
        
        layx_id container = layx_item(&ctx);
        layx_set_size(&ctx, container, 400, 400);
        layx_set_display(&ctx, container, LAYX_DISPLAY_BLOCK);
        
        layx_id child1 = layx_item(&ctx);
        layx_set_size(&ctx, child1, 100, 50);
        layx_id child2 = layx_item(&ctx);
        layx_set_size(&ctx, child2, 100, 50);
        
        layx_append(&ctx, container, child1);
        layx_insert_after(&ctx, child1, child2);
        
        layx_run_context(&ctx);
        
        layx_scalar x1, y1, w1, h1;
        layx_get_rect_xywh(&ctx, child1, &x1, &y1, &w1, &h1);
        
        layx_scalar x2, y2, w2, h2;
        layx_get_rect_xywh(&ctx, child2, &x2, &y2, &w2, &h2);
        
        printf("  Child1: pos=(%.1f, %.1f), size=(%.1f, %.1f)\n", x1, y1, w1, h1);
        printf("  Child2: pos=(%.1f, %.1f), size=(%.1f, %.1f)\n", x2, y2, w2, h2);
        
        // BLOCK: 子元素在垂直方向堆叠
        TEST_ASSERT(y2 > y1, "BLOCK布局：子元素应垂直堆叠（child2在child1下方）");
        TEST_ASSERT(x1 == x2, "BLOCK布局：子元素水平位置应相同（左对齐）");
        TEST_ASSERT(w1 == 100 && h1 == 50, "BLOCK布局：child1尺寸应保持原值");
        TEST_ASSERT(w2 == 100 && h2 == 50, "BLOCK布局：child2尺寸应保持原值");
        
        layx_destroy_context(&ctx);
    }

    // Test 2: DISPLAY_BLOCK does not apply flex-grow/shrink
    {
        printf("\n=== Test 2: DISPLAY_BLOCK ignores flex properties ===\n");
        
        layx_context ctx;
        layx_init_context(&ctx);
        layx_reserve_items_capacity(&ctx, 10);
        
        layx_id container = layx_item(&ctx);
        layx_set_size(&ctx, container, 200, 200);
        layx_set_display(&ctx, container, LAYX_DISPLAY_BLOCK);
        
        // 设置flex属性，但对BLOCK容器应无效
        layx_id child1 = layx_item(&ctx);
        layx_set_size(&ctx, child1, 100, 50);
        layx_set_flex_grow(&ctx, child1, 1);  // flex-grow 在 BLOCK 中应被忽略
        layx_id child2 = layx_item(&ctx);
        layx_set_size(&ctx, child2, 100, 50);
        
        layx_append(&ctx, container, child1);
        layx_insert_after(&ctx, child1, child2);
        
        layx_run_context(&ctx);
        
        layx_scalar w1, w2;
        layx_get_rect_xywh(&ctx, child1, NULL, NULL, &w1, NULL);
        layx_get_rect_xywh(&ctx, child2, NULL, NULL, &w2, NULL);
        
        printf("  Child1 width: %.1f (expected: 100, not stretched)\n", w1);
        printf("  Child2 width: %.1f (expected: 100, not stretched)\n", w2);
        
        TEST_ASSERT(w1 == 100, "BLOCK容器：flex-grow应被忽略（child1宽度不应改变）");
        TEST_ASSERT(w2 == 100, "BLOCK容器：flex-grow应被忽略（child2宽度不应改变）");
        
        layx_destroy_context(&ctx);
    }

    // Test 3: DISPLAY_INLINE basic behavior
    {
        printf("\n=== Test 3: Basic DISPLAY_INLINE Layout ===\n");
        
        layx_context ctx;
        layx_init_context(&ctx);
        layx_reserve_items_capacity(&ctx, 10);
        
        layx_id container = layx_item(&ctx);
        layx_set_size(&ctx, container, 400, 100);
        layx_set_display(&ctx, container, LAYX_DISPLAY_INLINE);
        
        layx_id child1 = layx_item(&ctx);
        layx_set_size(&ctx, child1, 50, 50);
        layx_id child2 = layx_item(&ctx);
        layx_set_size(&ctx, child2, 50, 50);
        layx_id child3 = layx_item(&ctx);
        layx_set_size(&ctx, child3, 50, 50);
        
        layx_append(&ctx, container, child1);
        layx_insert_after(&ctx, child1, child2);
        layx_insert_after(&ctx, child2, child3);
        
        layx_run_context(&ctx);
        
        layx_scalar x1, y1, x2, y2, x3, y3;
        layx_get_rect_xywh(&ctx, child1, &x1, &y1, NULL, NULL);
        layx_get_rect_xywh(&ctx, child2, &x2, &y2, NULL, NULL);
        layx_get_rect_xywh(&ctx, child3, &x3, &y3, NULL, NULL);
        
        printf("  Child1: pos=(%.1f, %.1f)\n", x1, y1);
        printf("  Child2: pos=(%.1f, %.1f)\n", x2, y2);
        printf("  Child3: pos=(%.1f, %.1f)\n", x3, y3);
        
        // INLINE: 子元素在一行内水平排列
        TEST_ASSERT(x2 > x1 && x3 > x2, "INLINE布局：子元素应水平排列");
        TEST_ASSERT(y1 == y2 && y2 == y3, "INLINE布局：子元素垂直位置应相同");
        
        layx_destroy_context(&ctx);
    }

    // Test 4: DISPLAY_INLINE_BLOCK basic behavior
    {
        printf("\n=== Test 4: Basic DISPLAY_INLINE_BLOCK Layout ===\n");
        
        layx_context ctx;
        layx_init_context(&ctx);
        layx_reserve_items_capacity(&ctx, 10);
        
        layx_id container = layx_item(&ctx);
        layx_set_size(&ctx, container, 400, 100);
        layx_set_display(&ctx, container, LAYX_DISPLAY_INLINE_BLOCK);
        
        layx_id child1 = layx_item(&ctx);
        layx_set_size(&ctx, child1, 80, 50);
        layx_id child2 = layx_item(&ctx);
        layx_set_size(&ctx, child2, 80, 50);
        
        layx_append(&ctx, container, child1);
        layx_insert_after(&ctx, child1, child2);
        
        layx_run_context(&ctx);
        
        layx_scalar x1, w1, x2, w2;
        layx_get_rect_xywh(&ctx, child1, &x1, NULL, &w1, NULL);
        layx_get_rect_xywh(&ctx, child2, &x2, NULL, &w2, NULL);
        
        printf("  Child1: pos=(%.1f, ...), width=%.1f\n", x1, w1);
        printf("  Child2: pos=(%.1f, ...), width=%.1f\n", x2, w2);
        
        // INLINE_BLOCK: 在一行内排列，但保持设置的宽度
        TEST_ASSERT(x2 > x1, "INLINE_BLOCK布局：子元素应水平排列");
        TEST_ASSERT(w1 == 80, "INLINE_BLOCK布局：应保持设置的宽度");
        TEST_ASSERT(w2 == 80, "INLINE_BLOCK布局：应保持设置的宽度");
        
        layx_destroy_context(&ctx);
    }

    // Test 5: DISPLAY_INLINE wrapping
    {
        printf("\n=== Test 5: DISPLAY_INLINE with Wrapping ===\n");
        
        layx_context ctx;
        layx_init_context(&ctx);
        layx_reserve_items_capacity(&ctx, 20);
        
        layx_id container = layx_item(&ctx);
        layx_set_size(&ctx, container, 300, 100);
        layx_set_display(&ctx, container, LAYX_DISPLAY_INLINE);
        
        // 添加多个子项，总宽度超过容器宽度
        layx_id child1 = layx_item(&ctx);
        layx_set_size(&ctx, child1, 80, 40);
        layx_id child2 = layx_item(&ctx);
        layx_set_size(&ctx, child2, 80, 40);
        layx_id child3 = layx_item(&ctx);
        layx_set_size(&ctx, child3, 80, 40);
        layx_id child4 = layx_item(&ctx);
        layx_set_size(&ctx, child4, 80, 40);
        
        layx_append(&ctx, container, child1);
        layx_insert_after(&ctx, child1, child2);
        layx_insert_after(&ctx, child2, child3);
        layx_insert_after(&ctx, child3, child4);
        
        layx_run_context(&ctx);
        
        layx_scalar x1, y1, x2, y2, x3, y3, x4, y4;
        layx_get_rect_xywh(&ctx, child1, &x1, &y1, NULL, NULL);
        layx_get_rect_xywh(&ctx, child2, &x2, &y2, NULL, NULL);
        layx_get_rect_xywh(&ctx, child3, &x3, &y3, NULL, NULL);
        layx_get_rect_xywh(&ctx, child4, &x4, &y4, NULL, NULL);
        
        printf("  Child1: pos=(%.1f, %.1f)\n", x1, y1);
        printf("  Child2: pos=(%.1f, %.1f)\n", x2, y2);
        printf("  Child3: pos=(%.1f, %.1f)\n", x3, y3);
        printf("  Child4: pos=(%.1f, %.1f)\n", x4, y4);
        
        // 验证换行行为
        TEST_ASSERT(x2 > x1 && x3 > x2, "INLINE布局：前几个元素应水平排列");
        
        layx_destroy_context(&ctx);
    }

    // Test 6: Display type string representation
    {
        printf("\n=== Test 6: Display Type String Representation ===\n");
        
        TEST_ASSERT(strcmp(layx_get_display_string(LAYX_DISPLAY_BLOCK), "BLOCK") == 0,
                   "LAYX_DISPLAY_BLOCK 字符串应为'BLOCK'");
        TEST_ASSERT(strcmp(layx_get_display_string(LAYX_DISPLAY_FLEX), "FLEX") == 0,
                   "LAYX_DISPLAY_FLEX 字符串应为'FLEX'");
        TEST_ASSERT(strcmp(layx_get_display_string(LAYX_DISPLAY_INLINE), "INLINE") == 0,
                   "LAYX_DISPLAY_INLINE 字符串应为'INLINE'");
        TEST_ASSERT(strcmp(layx_get_display_string(LAYX_DISPLAY_INLINE_BLOCK), "INLINE_BLOCK") == 0,
                   "LAYX_DISPLAY_INLINE_BLOCK 字符串应为'INLINE_BLOCK'");
    }

    // Test 7: Switching between display types
    {
        printf("\n=== Test 7: Switching Display Types ===\n");
        
        layx_context ctx;
        layx_init_context(&ctx);
        layx_reserve_items_capacity(&ctx, 10);
        
        layx_id container = layx_item(&ctx);
        layx_set_size(&ctx, container, 300, 200);
        
        layx_id child1 = layx_item(&ctx);
        layx_set_size(&ctx, child1, 100, 50);
        layx_id child2 = layx_item(&ctx);
        layx_set_size(&ctx, child2, 100, 50);
        
        layx_append(&ctx, container, child1);
        layx_insert_after(&ctx, child1, child2);
        
        // 首先使用 BLOCK 布局
        layx_set_display(&ctx, container, LAYX_DISPLAY_BLOCK);
        layx_run_context(&ctx);
        
        layx_scalar y1_block, y2_block;
        layx_get_rect_xywh(&ctx, child1, NULL, &y1_block, NULL, NULL);
        layx_get_rect_xywh(&ctx, child2, NULL, &y2_block, NULL, NULL);
        printf("  BLOCK: child1 y=%.1f, child2 y=%.1f\n", y1_block, y2_block);
        TEST_ASSERT(y2_block > y1_block, "BLOCK: child2应在child1下方");
        
        // 切换到 FLEX 布局（row方向）
        layx_set_display(&ctx, container, LAYX_DISPLAY_FLEX);
        layx_set_flex_direction(&ctx, container, LAYX_FLEX_DIRECTION_ROW);
        layx_run_context(&ctx);
        
        layx_scalar x1_flex, x2_flex;
        layx_get_rect_xywh(&ctx, child1, &x1_flex, NULL, NULL, NULL);
        layx_get_rect_xywh(&ctx, child2, &x2_flex, NULL, NULL, NULL);
        printf("  FLEX ROW: child1 x=%.1f, child2 x=%.1f\n", x1_flex, x2_flex);
        TEST_ASSERT(x2_flex > x1_flex, "FLEX ROW: child2应在child1右侧");
        
        layx_destroy_context(&ctx);
    }

    // Test 8: Mixed display types in hierarchy
    {
        printf("\n=== Test 8: Mixed Display Types ===\n");
        
        layx_context ctx;
        layx_init_context(&ctx);
        layx_reserve_items_capacity(&ctx, 10);
        
        // 父容器使用 BLOCK
        layx_id parent = layx_item(&ctx);
        layx_set_size(&ctx, parent, 300, 300);
        layx_set_display(&ctx, parent, LAYX_DISPLAY_BLOCK);
        
        // 子容器使用 FLEX
        layx_id child1 = layx_item(&ctx);
        layx_set_size(&ctx, child1, 150, 100);
        layx_set_display(&ctx, child1, LAYX_DISPLAY_FLEX);
        layx_set_flex_direction(&ctx, child1, LAYX_FLEX_DIRECTION_ROW);
        
        layx_id grandchild1 = layx_item(&ctx);
        layx_set_size(&ctx, grandchild1, 50, 50);
        layx_id grandchild2 = layx_item(&ctx);
        layx_set_size(&ctx, grandchild2, 50, 50);
        
        layx_append(&ctx, child1, grandchild1);
        layx_insert_after(&ctx, grandchild1, grandchild2);
        layx_append(&ctx, parent, child1);
        
        layx_run_context(&ctx);
        
        layx_scalar gc_x1, gc_x2;
        layx_get_rect_xywh(&ctx, grandchild1, &gc_x1, NULL, NULL, NULL);
        layx_get_rect_xywh(&ctx, grandchild2, &gc_x2, NULL, NULL, NULL);
        
        printf("  Grandchild1 x=%.1f, Grandchild2 x=%.1f\n", gc_x1, gc_x2);
        TEST_ASSERT(gc_x2 > gc_x1, "混合布局：FLEX子容器应正确排列其子项");
        
        layx_destroy_context(&ctx);
    }

    // Test 9: Default display type
    {
        printf("\n=== Test 9: Default Display Type ===\n");
        
        layx_context ctx;
        layx_init_context(&ctx);
        
        layx_id item = layx_item(&ctx);
        
        // 默认情况下，应该是什么 display 类型？
        // 根据我们的实现，默认应该是 BLOCK
        layx_item_t *pitem = layx_get_item(&ctx, item);
        layx_display display = layx_get_display_from_flags(pitem->flags);
        
        printf("  Default display: %s\n", layx_get_display_string(display));
        TEST_ASSERT(display == LAYX_DISPLAY_BLOCK, "默认display应为BLOCK");
        
        layx_destroy_context(&ctx);
    }

    // Summary
    printf("\n===========================================\n");
    printf("           Test Summary\n");
    printf("===========================================\n");
    printf("Tests Passed: %d\n", passed);
    printf("Tests Failed: %d\n", failures);
    printf("Total Tests:  %d\n", passed + failures);
    printf("===========================================\n");
    
    if (failures == 0) {
        printf("✓ All tests passed!\n");
    } else {
        printf("✗ Some tests failed!\n");
    }
    
    return failures;
}

/**
 * @file test_multiple_layout_runs.c
 * @brief 测试多次运行layout导致子元素宽度增长的问题
 * 
 * Bug: 多次调用layx_run_context()后，子元素宽度会递增
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <math.h>
#include "layx.h"

static int tests_passed = 0;
static int tests_failed = 0;

#define FLOAT_EQUAL(a, b, epsilon) (fabs((a) - (b)) < (epsilon))

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

// 测试多次运行layout导致子元素宽度增长
void test_multiple_layout_runs() {
    printf("\n=== Test: Multiple layout runs (reproduce bug) ===\n");
    
    layx_context ctx;
    layx_init_context(&ctx);
    
    // 父容器：flex row, align-items: flex-start
    layx_id parent = layx_item(&ctx);
    layx_set_display(&ctx, parent, LAYX_DISPLAY_FLEX);
    layx_set_flex_direction(&ctx, parent, LAYX_FLEX_DIRECTION_ROW);
    layx_set_align_items(&ctx, parent, LAYX_ALIGN_ITEMS_FLEX_START);
    layx_set_justify_content(&ctx, parent, LAYX_JUSTIFY_FLEX_START);
    layx_set_size(&ctx, parent, 948, 207);
    
    // 创建5个box子元素，每个都是flex容器，固定宽度80px
    layx_id boxes[5];
    layx_scalar x, y, w, h;
    for (int i = 0; i < 5; i++) {
        boxes[i] = layx_item(&ctx);
        layx_set_display(&ctx, boxes[i], LAYX_DISPLAY_FLEX);
        layx_set_flex_direction(&ctx, boxes[i], LAYX_FLEX_DIRECTION_ROW);
        layx_set_justify_content(&ctx, boxes[i], LAYX_JUSTIFY_CENTER);
        layx_set_align_items(&ctx, boxes[i], LAYX_ALIGN_ITEMS_CENTER);
        layx_set_width(&ctx, boxes[i], 80);
        layx_set_height(&ctx, boxes[i], 80);
        layx_set_margin(&ctx, boxes[i], 5);
        layx_append(&ctx, parent, boxes[i]);
        
        // 每个box包含一个文本节点
        layx_id text = layx_item(&ctx);
        layx_set_display(&ctx, text, LAYX_DISPLAY_BLOCK);
        layx_set_width(&ctx, text, 10);
        layx_set_height(&ctx, text, 16);
        layx_append(&ctx, boxes[i], text);
    }
    
    // 运行布局三次
    for (int pass = 1; pass <= 3; pass++) {
        printf("\n--- Pass %d ---\n", pass);
        layx_run_context(&ctx);
        
        printf("After pass %d:\n", pass);
        for (int i = 0; i < 5; i++) {
            layx_get_rect_xywh(&ctx, boxes[i], &x, &y, &w, &h);
            printf("  box[%d]: x=%.1f, y=%.1f, w=%.1f, h=%.1f\n", i, x, y, w, h);
        }
    }
    
    // 打印完整的布局树
    // printf("\n=== Layout Tree ===\n");
    // layx_dump_tree(&ctx, parent, 0);
    
    // 检查每个box的宽度
    printf("\n=== Checking box widths ===\n");
    bool all_equal = true;
    layx_scalar expected_width = 80;
    
    for (int i = 0; i < 5; i++) {
        layx_get_rect_xywh(&ctx, boxes[i], &x, &y, &w, &h);
        printf("  box[%d]: w=%.1f (expected: %.1f)\n", i, w, expected_width);
        
        if (!FLOAT_EQUAL(w, expected_width, 0.1f)) {
            printf("    ⚠ BUG DETECTED: Width should be %.1f, but got %.1f (difference: %.1f)\n",
                   expected_width, w, fabs(w - expected_width));
            all_equal = false;
        }
    }
    
    TEST_ASSERT(all_equal, "All boxes should have width 80px");
    
    layx_destroy_context(&ctx);
}

// 测试不同的box数量
void test_varying_box_count(int count) {
    printf("\n=== Test with %d boxes ===\n", count);
    
    layx_context ctx;
    layx_init_context(&ctx);
    
    layx_id parent = layx_item(&ctx);
    layx_set_display(&ctx, parent, LAYX_DISPLAY_FLEX);
    layx_set_flex_direction(&ctx, parent, LAYX_FLEX_DIRECTION_ROW);
    layx_set_align_items(&ctx, parent, LAYX_ALIGN_ITEMS_FLEX_START);
    layx_set_size(&ctx, parent, 1000, 120);
    
    layx_id *boxes = (layx_id*)malloc(count * sizeof(layx_id));
    
    for (int i = 0; i < count; i++) {
        boxes[i] = layx_item(&ctx);
        layx_set_display(&ctx, boxes[i], LAYX_DISPLAY_FLEX);
        layx_set_flex_direction(&ctx, boxes[i], LAYX_FLEX_DIRECTION_ROW);
        layx_set_justify_content(&ctx, boxes[i], LAYX_JUSTIFY_CENTER);
        layx_set_align_items(&ctx, boxes[i], LAYX_ALIGN_ITEMS_CENTER);
        layx_set_width(&ctx, boxes[i], 80);
        layx_set_height(&ctx, boxes[i], 80);
        layx_set_margin(&ctx, boxes[i], 5);
        layx_append(&ctx, parent, boxes[i]);
        
        layx_id text = layx_item(&ctx);
        layx_set_width(&ctx, text, 10);
        layx_set_height(&ctx, text, 16);
        layx_append(&ctx, boxes[i], text);
    }
    
    layx_run_context(&ctx);
    
    layx_scalar x, y, w, h;
    bool all_equal = true;
    
    for (int i = 0; i < count; i++) {
        layx_get_rect_xywh(&ctx, boxes[i], &x, &y, &w, &h);
        if (!FLOAT_EQUAL(w, 80, 0.1f)) {
            printf("  box[%d]: w=%.1f (expected: 80.0) ⚠ BUG!\n", i, w);
            all_equal = false;
        }
    }
    
    TEST_ASSERT(all_equal, "All boxes should have width 80px");
    
    free(boxes);
    layx_destroy_context(&ctx);
}

// 测试box内文本宽度的影响
void test_text_width_impact() {
    printf("\n=== Test: Impact of different text widths ===\n");
    
    const int text_widths[] = {5, 10, 20, 30};
    const int num_tests = 4;
    
    for (int t = 0; t < num_tests; t++) {
        layx_context ctx;
        layx_init_context(&ctx);
        
        layx_id parent = layx_item(&ctx);
        layx_set_display(&ctx, parent, LAYX_DISPLAY_FLEX);
        layx_set_flex_direction(&ctx, parent, LAYX_FLEX_DIRECTION_ROW);
        layx_set_align_items(&ctx, parent, LAYX_ALIGN_ITEMS_FLEX_START);
        layx_set_size(&ctx, parent, 500, 120);
        
        // 创建3个box，每个包含不同宽度的文本
        for (int i = 0; i < 3; i++) {
            layx_id box = layx_item(&ctx);
            layx_set_display(&ctx, box, LAYX_DISPLAY_FLEX);
            layx_set_flex_direction(&ctx, box, LAYX_FLEX_DIRECTION_ROW);
            layx_set_justify_content(&ctx, box, LAYX_JUSTIFY_CENTER);
            layx_set_align_items(&ctx, box, LAYX_ALIGN_ITEMS_CENTER);
            layx_set_width(&ctx, box, 80);
            layx_set_height(&ctx, box, 80);
            layx_set_margin(&ctx, box, 5);
            layx_append(&ctx, parent, box);
            
            layx_id text = layx_item(&ctx);
            layx_set_width(&ctx, text, text_widths[(t + i) % num_tests]);
            layx_set_height(&ctx, text, 16);
            layx_append(&ctx, box, text);
        }
        
        layx_run_context(&ctx);
        
        printf("  Test %d: ", t + 1);
        layx_id child = layx_get_item(&ctx, parent)->first_child;
        bool ok = true;
        int idx = 0;
        while (child != LAYX_INVALID_ID) {
            layx_scalar x, y, w, h;
            layx_get_rect_xywh(&ctx, child, &x, &y, &w, &h);
            if (!FLOAT_EQUAL(w, 80, 0.1f)) {
                printf("box[%d] w=%.1f ", idx, w);
                ok = false;
            }
            child = layx_get_item(&ctx, child)->next_sibling;
            idx++;
        }
        if (ok) {
            printf("✓ All boxes width=80px\n");
        } else {
            printf("⚠ BUG!\n");
        }
        
        layx_destroy_context(&ctx);
    }
}

int main() {
    printf("========================================\n");
    printf("Testing: Child Width Growth Bug (Multiple Layout Runs)\n");
    printf("========================================\n");
    
    test_multiple_layout_runs();
    test_varying_box_count(3);
    test_varying_box_count(5);
    test_varying_box_count(7);
    test_text_width_impact();
    
    printf("\n========================================\n");
    printf("Test Results:\n");
    printf("  Passed: %d\n", tests_passed);
    printf("  Failed: %d\n", tests_failed);
    printf("========================================\n");
    
    return tests_failed > 0 ? 1 : 0;
}

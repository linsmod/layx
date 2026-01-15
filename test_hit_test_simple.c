/**
 * @file test_hit_test_simple.c
 * @brief 简化的hit test测试
 */

#include <stdio.h>
#include "layx.h"

int main(void) {
    printf("Step 1: Init context\n");
    layx_context ctx;
    layx_init_context(&ctx);
    
    printf("Step 2: Reserve capacity\n");
    layx_reserve_items_capacity(&ctx, 10);
    
    printf("Step 3: Create element\n");
    layx_id element = layx_item(&ctx);
    printf("  element ID = %u\n", element);
    
    printf("Step 4: Set size\n");
    layx_set_size(&ctx, element, 200, 150);
    
    printf("Step 5: Run layout\n");
    layx_run_context(&ctx);
    printf("  Layout completed\n");
    
    printf("Step 6: Get rect\n");
    layx_vec4 rect = layx_get_rect(&ctx, element);
    printf("  rect: x=%.1f y=%.1f w=%.1f h=%.1f\n", rect[0], rect[1], rect[2], rect[3]);
    
    printf("Step 7: Hit test\n");
    int hit = layx_hit_test(&ctx, element, 100, 80);
    printf("  Hit test result: %d\n", hit);
    
    printf("Step 8: Destroy context\n");
    layx_destroy_context(&ctx);
    
    printf("All steps completed!\n");
    return 0;
}

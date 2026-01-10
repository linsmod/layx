#include "layx.h"
#include <stdio.h>

void print_vec2(const char* name, layx_vec2 vec) {
#if defined(__GNUC__) || defined(__clang__)
    printf("%s: (%.2f, %.2f)\n", name, vec[0], vec[1]);
#else
    printf("%s: (%.2f, %.2f)\n", name, vec.xy[0], vec.xy[1]);
#endif
}

int main() {
    printf("=== Debug layx_run_item Order ===\n\n");
    
    layx_context ctx;
    layx_init_context(&ctx);
    layx_reserve_items_capacity(&ctx, 10);
    
    layx_id container = layx_item(&ctx);
    layx_set_size(&ctx, container, 200, 150);
    layx_set_padding(&ctx, container, 10);
    layx_set_overflow(&ctx, container, LAYX_OVERFLOW_AUTO);
    layx_set_display(&ctx, container, LAYX_DISPLAY_FLEX);
    layx_set_flex_direction(&ctx, container, LAYX_FLEX_DIRECTION_COLUMN);
    
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
    
    // Step 1: 初始化后
    printf("Step 1: After init and push\n");
    layx_item_t *pitem = layx_get_item(&ctx, container);
    printf("  content_size: (%.2f, %.2f)\n", pitem->content_size[0], pitem->content_size[1]);
    printf("  first_child: %d\n", pitem->first_child);
    
    // Step 2: 第一次 calc_size
    printf("\nStep 2: After first calc_size\n");
    layx_calc_size(&ctx, container, 0);
    printf("  content_size: (%.2f, %.2f)\n", pitem->content_size[0], pitem->content_size[1]);
    
    // Step 3: 第一次 arrange
    printf("\nStep 3: After first arrange\n");
    layx_arrange(&ctx, container, 0);
    printf("  content_size: (%.2f, %.2f)\n", pitem->content_size[0], pitem->content_size[1]);
    
    // 手动计算应该的内容尺寸
    layx_scalar max_x = 0.0f, max_y = 0.0f;
    layx_id child = pitem->first_child;
    while (child != LAYX_INVALID_ID) {
        layx_vec4 rect = layx_get_rect(&ctx, child);
        layx_scalar bottom = rect[1] + rect[3];
        if (bottom > max_y) max_y = bottom;
        child = layx_next_sibling(&ctx, child);
    }
    printf("  Expected content_size.y (based on child positions): %.2f\n", max_y);
    
    // Step 4: calculate_content_size
    printf("\nStep 4: After calculate_content_size\n");
    layx_calculate_content_size(&ctx, container);
    printf("  content_size: (%.2f, %.2f)\n", pitem->content_size[0], pitem->content_size[1]);
    
    // Step 5: detect_scrollbars
    printf("\nStep 5: After detect_scrollbars\n");
    layx_detect_scrollbars(&ctx, container);
    printf("  content_size: (%.2f, %.2f)\n", pitem->content_size[0], pitem->content_size[1]);
    printf("  scroll_max: (%.2f, %.2f)\n", pitem->scroll_max[0], pitem->scroll_max[1]);
    
    // Step 6: 第二次 calc_size
    printf("\nStep 6: After second calc_size\n");
    layx_calc_size(&ctx, container, 1);
    printf("  content_size: (%.2f, %.2f)\n", pitem->content_size[0], pitem->content_size[1]);
    
    // Step 7: 第二次 arrange
    printf("\nStep 7: After second arrange\n");
    layx_arrange(&ctx, container, 1);
    printf("  content_size: (%.2f, %.2f)\n", pitem->content_size[0], pitem->content_size[1]);
    
    layx_destroy_context(&ctx);
    return 0;
}

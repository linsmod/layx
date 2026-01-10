#include "layx.h"
#include <stdio.h>

void print_vec2(const char* name, layx_vec2 vec) {
#if defined(__GNUC__) || defined(__clang__)
    printf("%s: (%.2f, %.2f)\n", name, vec[0], vec[1]);
#else
    printf("%s: (%.2f, %.2f)\n", name, vec.xy[0], vec.xy[1]);
#endif
}

void print_rect(layx_context *ctx, layx_id item, const char* label) {
    layx_vec4 rect = layx_get_rect(ctx, item);
    printf("  %s: pos=(%.1f, %.1f) size=(%.1f, %.1f)\n",
           label, rect[0], rect[1], rect[2], rect[3]);
}

int main() {
    printf("=== Debug Content Size Calculation ===\n\n");
    
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
    
    // 运行布局
    layx_run_context(&ctx);
    
    // 手动计算内容尺寸
    layx_item_t *pitem = layx_get_item(&ctx, container);
    layx_scalar max_x = 0.0f;
    layx_scalar max_y = 0.0f;
    layx_id child = pitem->first_child;
    
    printf("Manual calculation of content_size:\n");
    while (child != LAYX_INVALID_ID) {
        layx_vec4 child_rect = layx_get_rect(&ctx, child);
        layx_scalar child_right = child_rect[0] + child_rect[2];
        layx_scalar child_bottom = child_rect[1] + child_rect[3];
        
        printf("  child %d: right=%.2f, bottom=%.2f\n", child, child_right, child_bottom);
        
        if (child_right > max_x) max_x = child_right;
        if (child_bottom > max_y) max_y = child_bottom;
        
        child = layx_next_sibling(&ctx, child);
    }
    
    printf("  Calculated: (%.2f, %.2f)\n", max_x, max_y);
    
    // 获取存储的内容尺寸
    layx_vec2 content_size;
    layx_get_content_size(&ctx, container, &content_size);
    printf("  Stored: "); print_vec2("", content_size);
    
    // 检查是否有子项
    printf("\nFirst child: %d\n", pitem->first_child);
    printf("Next sibling of child1: %d\n", layx_next_sibling(&ctx, child1));
    
    layx_destroy_context(&ctx);
    return 0;
}

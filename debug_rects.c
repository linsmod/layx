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
    printf("=== Debug Child Rects ===\n\n");
    
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
    
    layx_run_context(&ctx);
    
    printf("Container:\n");
    layx_vec2 content_size;
    layx_get_content_size(&ctx, container, &content_size);
    printf("  content_size: "); print_vec2("", content_size);
    print_rect(&ctx, container, "rect");
    
    printf("\nChildren:\n");
    print_rect(&ctx, child1, "child1");
    print_rect(&ctx, child2, "child2");
    print_rect(&ctx, child3, "child3");
    print_rect(&ctx, child4, "child4");
    
    layx_destroy_context(&ctx);
    return 0;
}

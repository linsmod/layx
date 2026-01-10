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
    printf("=== Debug Scroll Tests ===\n\n");
    
    // Test 2: Vertical scroll
    {
        printf("Test 2: Vertical Scroll\n");
        layx_context ctx;
        layx_init_context(&ctx);
        layx_reserve_items_capacity(&ctx, 10);
        
        layx_id container = layx_item(&ctx);
        layx_set_size(&ctx, container, 200, 150);
        layx_set_padding(&ctx, container, 10);
        layx_set_overflow(&ctx, container, LAYX_OVERFLOW_AUTO);
        
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
        
        layx_vec2 content_size;
        layx_get_content_size(&ctx, container, &content_size);
        printf("  content_size: "); print_vec2("", content_size);
        printf("  client height: %.2f\n", 150 - 10 - 10);
        printf("  has_v_scroll: %d\n", layx_has_vertical_scrollbar(&ctx, container));
        
        layx_destroy_context(&ctx);
    }
    
    // Test 6: Scroll To
    {
        printf("\nTest 6: Scroll To\n");
        layx_context ctx;
        layx_init_context(&ctx);
        layx_reserve_items_capacity(&ctx, 10);
        
        layx_id container = layx_item(&ctx);
        layx_set_size(&ctx, container, 200, 150);
        layx_set_overflow(&ctx, container, LAYX_OVERFLOW_AUTO);
        
        layx_id child = layx_item(&ctx);
        layx_set_size(&ctx, child, 300, 300);
        layx_push(&ctx, container, child);
        
        layx_run_context(&ctx);
        
        layx_vec2 content_size, scroll_max;
        layx_get_content_size(&ctx, container, &content_size);
        layx_get_scroll_max(&ctx, container, &scroll_max);
        printf("  content_size: "); print_vec2("", content_size);
        printf("  scroll_max: "); print_vec2("", scroll_max);
        
        layx_scroll_to(&ctx, container, 50.0f, 25.0f);
        layx_vec2 scroll_offset;
        layx_get_scroll_offset(&ctx, container, &scroll_offset);
        printf("  scroll_offset after scroll_to(50,25): "); print_vec2("", scroll_offset);
        
        layx_destroy_context(&ctx);
    }
    
    return 0;
}

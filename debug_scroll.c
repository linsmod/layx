#include "layx.h"
#include <stdio.h>

// 辅助函数：打印向量内容
void print_vec2(const char* name, layx_vec2 vec) {
#if defined(__GNUC__) || defined(__clang__)
    printf("%s: (%.2f, %.2f)\n", name, vec[0], vec[1]);
#else
    printf("%s: (%.2f, %.2f)\n", name, vec.xy[0], vec.xy[1]);
#endif
}

int main() {
    printf("=== LAYX Scroll Debug ===\n\n");
    
    // 初始化上下文
    layx_context ctx;
    layx_init_context(&ctx);
    layx_reserve_items_capacity(&ctx, 5);
    
    // 创建容器
    layx_id container = layx_item(&ctx);
    printf("After creating container (id=%d)\n", container);
    layx_set_size(&ctx, container, 200, 150);
    layx_set_padding(&ctx, container, 10);
    
    // 设置overflow为auto以启用滚动条
    layx_set_overflow(&ctx, container, LAYX_OVERFLOW_AUTO);
    printf("Set overflow to AUTO\n");
    
    // 创建一些子项使内容超出容器大小
    for (int i = 0; i < 5; i++) {
        layx_id child = layx_item(&ctx);
        layx_set_size(&ctx, child, 300, 50);  // 宽度超出容器
        layx_push(&ctx, container, child);
    }
    
    // 第一次运行布局
    printf("\nFirst layx_run_context:\n");
    layx_run_context(&ctx);
    
    // 调试信息
    layx_vec2 content_size;
    layx_get_content_size(&ctx, container, &content_size);
    printf("  content_size=(%.2f, %.2f)\n", content_size[0], content_size[1]);
    
    layx_scalar client_width = 200 - 10 - 10; // 宽度减去左右padding
    layx_scalar client_height = 150 - 10 - 10; // 高度减去上下padding
    printf("  client_width=%.2f, client_height=%.2f\n", client_width, client_height);
    
    // 检查滚动条状态
    int has_v_scroll = layx_has_vertical_scrollbar(&ctx, container);
    int has_h_scroll = layx_has_horizontal_scrollbar(&ctx, container);
    printf("  has_v_scroll=%d, has_h_scroll=%d\n", has_v_scroll, has_h_scroll);
    
    // 检查item的overflow值
    layx_item_t *pitem = layx_get_item(&ctx, container);
    printf("  overflow_x=%d (AUTO=%d), overflow_y=%d (AUTO=%d)\n", 
           pitem->overflow_x, LAYX_OVERFLOW_AUTO, 
           pitem->overflow_y, LAYX_OVERFLOW_AUTO);
    
    printf("\nExpected: needs_h_scroll = (310 > 180) = true\n");
    printf("Actual: needs_h_scroll = %d\n", has_h_scroll);
    
    // 清理
    layx_destroy_context(&ctx);
    
    printf("\nDebug completed!\n");
    return 0;
}

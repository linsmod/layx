#include <stdio.h>
#include "layx.h"

void debug_content_size(void) {
    layx_context ctx;
    layx_init_context(&ctx);
    layx_reserve_items_capacity(&ctx, 10);

    // 创建简单的容器
    layx_id container = layx_item(&ctx);
    layx_set_size(&ctx, container, 200, 150);
    layx_set_overflow(&ctx, container, LAYX_OVERFLOW_AUTO);
    layx_set_display(&ctx, container, LAYX_DISPLAY_FLEX);
    layx_set_flex_direction(&ctx, container, LAYX_FLEX_DIRECTION_COLUMN);

    // 添加子项
    layx_id child = layx_item(&ctx);
    layx_set_size(&ctx, child, 300, 50);
    layx_push(&ctx, container, child);

    printf("=== Before layx_run_context ===\n");
    layx_run_context(&ctx);

    printf("\n=== After layx_run_context ===\n");
    layx_item_t *pcontainer = layx_get_item(&ctx, container);
    layx_vec4 container_rect = layx_get_rect(&ctx, container);
    layx_vec4 child_rect = layx_get_rect(&ctx, child);

    printf("Container size: %.2fx%.2f\n", pcontainer->size[0], pcontainer->size[1]);
    printf("Container padding: %.2f,%.2f,%.2f,%.2f\n", pcontainer->padding[0], pcontainer->padding[1], pcontainer->padding[2], pcontainer->padding[3]);
    printf("Container content_size: %.2fx%.2f\n", pcontainer->content_size[0], pcontainer->content_size[1]);
    printf("Container rect: %.2f,%.2f,%.2fx%.2f\n", container_rect[0], container_rect[1], container_rect[2], container_rect[3]);
    printf("Child rect: %.2f,%.2f,%.2fx%.2f\n", child_rect[0], child_rect[1], child_rect[2], child_rect[3]);

    layx_vec2 content_size;
    layx_get_content_size(&ctx, container, &content_size);
    printf("Content size (via getter): %.2fx%.2f\n", content_size[0], content_size[1]);

    layx_destroy_context(&ctx);
}

int main(void) {
    debug_content_size();
    return 0;
}

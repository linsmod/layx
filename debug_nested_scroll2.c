#include <stdio.h>
#include "layx.h"

void debug_nested_scroll(void) {
    layx_context ctx;
    layx_init_context(&ctx);
    layx_reserve_items_capacity(&ctx, 10);

    // 外层容器
    layx_id outer = layx_item(&ctx);
    layx_set_size(&ctx, outer, 400, 300);
    layx_set_overflow(&ctx, outer, LAYX_OVERFLOW_AUTO);
    layx_set_display(&ctx, outer, LAYX_DISPLAY_FLEX);
    layx_set_flex_direction(&ctx, outer, LAYX_FLEX_DIRECTION_COLUMN);

    // 内层容器
    layx_id inner = layx_item(&ctx);
    layx_set_size(&ctx, inner, 500, 200);
    layx_set_overflow(&ctx, inner, LAYX_OVERFLOW_AUTO);
    layx_set_display(&ctx, inner, LAYX_DISPLAY_FLEX);
    layx_set_flex_direction(&ctx, inner, LAYX_FLEX_DIRECTION_COLUMN);
    layx_push(&ctx, outer, inner);

    // 内容项
    layx_id content = layx_item(&ctx);
    layx_set_size(&ctx, content, 800, 600);
    layx_push(&ctx, inner, content);

    layx_run_context(&ctx);

    // 输出调试信息
    printf("=== Outer Container ===\n");
    layx_item_t *pouter = layx_get_item(&ctx, outer);
    printf("Size: %.2fx%.2f\n", pouter->size[0], pouter->size[1]);
    printf("Padding: %.2f,%.2f,%.2f,%.2f\n", pouter->padding[0], pouter->padding[1], pouter->padding[2], pouter->padding[3]);
    printf("Border: %.2f,%.2f,%.2f,%.2f\n", pouter->border[0], pouter->border[1], pouter->border[2], pouter->border[3]);
    layx_vec2 outer_content;
    layx_get_content_size(&ctx, outer, &outer_content);
    printf("Content size: %.2fx%.2f\n", outer_content[0], outer_content[1]);
    layx_vec4 outer_rect = layx_get_rect(&ctx, outer);
    printf("Rect: %.2f,%.2f,%.2fx%.2f\n", outer_rect[0], outer_rect[1], outer_rect[2], outer_rect[3]);
    printf("Has horizontal scrollbar: %d\n", layx_has_horizontal_scrollbar(&ctx, outer));
    printf("Has vertical scrollbar: %d\n", layx_has_vertical_scrollbar(&ctx, outer));

    printf("\n=== Inner Container ===\n");
    layx_item_t *pinner = layx_get_item(&ctx, inner);
    printf("Size: %.2fx%.2f\n", pinner->size[0], pinner->size[1]);
    printf("Padding: %.2f,%.2f,%.2f,%.2f\n", pinner->padding[0], pinner->padding[1], pinner->padding[2], pinner->padding[3]);
    printf("Border: %.2f,%.2f,%.2f,%.2f\n", pinner->border[0], pinner->border[1], pinner->border[2], pinner->border[3]);
    layx_vec2 inner_content;
    layx_get_content_size(&ctx, inner, &inner_content);
    printf("Content size: %.2fx%.2f\n", inner_content[0], inner_content[1]);
    layx_vec4 inner_rect = layx_get_rect(&ctx, inner);
    printf("Rect: %.2f,%.2f,%.2fx%.2f\n", inner_rect[0], inner_rect[1], inner_rect[2], inner_rect[3]);
    printf("Has horizontal scrollbar: %d\n", layx_has_horizontal_scrollbar(&ctx, inner));
    printf("Has vertical scrollbar: %d\n", layx_has_vertical_scrollbar(&ctx, inner));

    printf("\n=== Content Item ===\n");
    layx_item_t *pcontent = layx_get_item(&ctx, content);
    printf("Size: %.2fx%.2f\n", pcontent->size[0], pcontent->size[1]);
    layx_vec4 content_rect = layx_get_rect(&ctx, content);
    printf("Rect: %.2f,%.2f,%.2fx%.2f\n", content_rect[0], content_rect[1], content_rect[2], content_rect[3]);

    printf("\n=== First child of inner ===\n");
    layx_id first_child = layx_first_child(&ctx, inner);
    printf("First child ID: %u (content item ID: %u)\n", first_child, content);
    if (first_child != LAYX_INVALID_ID) {
        layx_vec4 first_child_rect = layx_get_rect(&ctx, first_child);
        printf("First child rect: %.2f,%.2f,%.2fx%.2f\n", first_child_rect[0], first_child_rect[1], first_child_rect[2], first_child_rect[3]);
    }

    layx_destroy_context(&ctx);
}

int main(void) {
    debug_nested_scroll();
    return 0;
}

#define LAYX_IMPLEMENTATION
#include "layx.h"
#include <stdio.h>
#include <assert.h>

void test_scrollbar_dimensions(layx_context *ctx, layx_id container_id, const char* test_name) {
    printf("\n=== %s ===\n", test_name);
    
    layx_item_t *item = layx_get_item(ctx, container_id);
    layx_vec2 content_size = item->content_size;
    layx_vec2 scroll_max = item->scroll_max;
    
    printf("内容尺寸: %.2f x %.2f\n", content_size[0], content_size[1]);
    printf("滚动最大值: %.2f x %.2f\n", scroll_max[0], scroll_max[1]);
    printf("是否有垂直滚动条: %s\n", 
           (item->has_scrollbars & LAYX_HAS_VSCROLL) ? "是" : "否");
    printf("是否有水平滚动条: %s\n", 
           (item->has_scrollbars & LAYX_HAS_HSCROLL) ? "是" : "否");
    
    // 验证CSS规范关系
    float client_w = layx_get_client_width(ctx, container_id);
    float client_h = layx_get_client_height(ctx, container_id);
    float scroll_w = layx_get_scroll_width(ctx, container_id);
    float scroll_h = layx_get_scroll_height(ctx, container_id);
    
    printf("client: %.2f x %.2f\n", client_w, client_h);
    printf("scroll: %.2f x %.2f\n", scroll_w, scroll_h);
    
    // CSS规范要求
    if (scroll_w > client_w) {
        assert(item->has_scrollbars & LAYX_HAS_HSCROLL);
        printf("✓ 水平滚动条正确显示\n");
    }
    
    if (scroll_h > client_h) {
        assert(item->has_scrollbars & LAYX_HAS_VSCROLL);
        printf("✓ 垂直滚动条正确显示\n");
    }
    
    if (scroll_w <= client_w) {
        assert(!(item->has_scrollbars & LAYX_HAS_HSCROLL));
        printf("✓ 无水平滚动条（正确）\n");
    }
    
    if (scroll_h <= client_h) {
        assert(!(item->has_scrollbars & LAYX_HAS_VSCROLL));
        printf("✓ 无垂直滚动条（正确）\n");
    }
}

int main() {
    layx_context ctx;
    layx_init_context(&ctx);
    layx_reserve_items_capacity(&ctx, 20);

    printf("🧪 CSS规范兼容性测试\n");
    printf("====================\n");

    // 测试1: 垂直滚动
    layx_id container1 = layx_item(&ctx);
    layx_set_width(&ctx, container1, 400);
    layx_set_height(&ctx, container1, 300);
    layx_set_border(&ctx, container1, 10);
    layx_set_padding(&ctx, container1, 20);
    layx_set_overflow(&ctx, container1, LAYX_OVERFLOW_SCROLL);

    for (int i = 0; i < 4; i++) {
        layx_id child = layx_item(&ctx);
        layx_set_width(&ctx, child, 350);
        layx_set_height(&ctx, child, 100);
        layx_set_margin(&ctx, child, 10);
        layx_push(&ctx, container1, child);
    }

    // 测试2: 水平滚动
    layx_id container2 = layx_item(&ctx);
    layx_set_width(&ctx, container2, 200);
    layx_set_height(&ctx, container2, 150);
    layx_set_overflow_x(&ctx, container2, LAYX_OVERFLOW_SCROLL);
    layx_set_overflow_y(&ctx, container2, LAYX_OVERFLOW_HIDDEN);

    layx_id wide_child = layx_item(&ctx);
    layx_set_width(&ctx, wide_child, 500);
    layx_set_height(&ctx, wide_child, 100);
    layx_push(&ctx, container2, wide_child);

    // 测试3: 无滚动条
    layx_id container3 = layx_item(&ctx);
    layx_set_width(&ctx, container3, 400);
    layx_set_height(&ctx, container3, 300);
    layx_set_overflow(&ctx, container3, LAYX_OVERFLOW_HIDDEN);

    layx_id small_child = layx_item(&ctx);
    layx_set_width(&ctx, small_child, 100);
    layx_set_height(&ctx, small_child, 50);
    layx_push(&ctx, container3, small_child);

    // 运行布局
    layx_run_context(&ctx);

    // 执行测试
    test_scrollbar_dimensions(&ctx, container1, "垂直滚动测试");
    test_scrollbar_dimensions(&ctx, container2, "水平滚动测试");
    test_scrollbar_dimensions(&ctx, container3, "无滚动条测试");

    // 测试Web标准API
    printf("\n🔍 Web标准API验证\n");
    printf("==================\n");
    
    printf("offsetWidth/Height = 视口尺寸 (包含border+padding+内容)\n");
    printf("clientWidth/Height = 内容+padding (无border, 无滚动条)\n");
    printf("scrollWidth/Height = 实际内容尺寸\n");
    
    for (int i = 1; i <= 3; i++) {
        layx_id id = (i == 1) ? container1 : (i == 2) ? container2 : container3;
        printf("\n容器 %d:\n", i);
        printf("  offset: %.2f x %.2f\n", 
               layx_get_offset_width(&ctx, id), 
               layx_get_offset_height(&ctx, id));
        printf("  client: %.2f x %.2f\n", 
               layx_get_client_width(&ctx, id), 
               layx_get_client_height(&ctx, id));
        printf("  scroll: %.2f x %.2f\n", 
               layx_get_scroll_width(&ctx, id), 
               layx_get_scroll_height(&ctx, id));
    }

    layx_destroy_context(&ctx);
    printf("\n✅ 所有测试通过！\n");
    return 0;
}
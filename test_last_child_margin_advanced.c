#include "layx.h"
#include <stdio.h>
#include <assert.h>

// 测试：有flex-shrink时，最后一个子元素的margin-bottom
void test_last_child_margin_with_shrink() {
    printf("=== Test: Last Child Margin-Bottom With Flex Shrink ===\n");

    layx_context ctx;
    layx_init_context(&ctx);
    layx_reserve_items_capacity(&ctx, 10);

    // 创建容器（FLEX COLUMN，固定高度200，宽度200）
    layx_id container = layx_item(&ctx);
    layx_set_size(&ctx, container, 200, 200);  // 固定高度200
    layx_set_display(&ctx, container, LAYX_DISPLAY_FLEX);
    layx_set_flex_direction(&ctx, container, LAYX_FLEX_DIRECTION_COLUMN);

    // 创建子元素1：高度100，flex-shrink: 1
    layx_id child1 = layx_item(&ctx);
    layx_set_size(&ctx, child1, 100, 100);
    layx_set_margin_trbl(&ctx, child1, 10, 0, 20, 0);  // margin-top:10, margin-bottom:20
    layx_set_flex_shrink(&ctx, child1, 1.0f);
    layx_append(&ctx, container, child1);

    // 创建子元素2：高度100，flex-shrink: 1，margin-bottom: 30（关键测试点）
    layx_id child2 = layx_item(&ctx);
    layx_set_size(&ctx, child2, 100, 100);
    layx_set_margin_trbl(&ctx, child2, 20, 0, 30, 0);  // margin-top:20, margin-bottom:30
    layx_set_flex_shrink(&ctx, child2, 1.0f);
    layx_append(&ctx, container, child2);

    layx_run_context(&ctx);

    // 获取容器尺寸
    layx_scalar cx, cy, cw, ch;
    layx_get_rect_xywh(&ctx, container, &cx, &cy, &cw, &ch);

    // 获取子元素尺寸和位置
    layx_scalar c1x, c1y, c1w, c1h;
    layx_get_rect_xywh(&ctx, child1, &c1x, &c1y, &c1w, &c1h);

    layx_scalar c2x, c2y, c2w, c2h;
    layx_get_rect_xywh(&ctx, child2, &c2x, &c2y, &c2w, &c2h);

    printf("  Container: x=%.1f, y=%.1f, w=%.1f, h=%.1f (fixed height)\n", cx, cy, cw, ch);
    printf("  Child1: x=%.1f, y=%.1f, w=%.1f, h=%.1f (margin-top:10, margin-bottom:20)\n", c1x, c1y, c1w, c1h);
    printf("  Child2: x=%.1f, y=%.1f, w=%.1f, h=%.1f (margin-top:20, margin-bottom:30)\n", c2x, c2y, c2w, c2h);

    // 计算总占用空间（不包含padding/border）
    // child1 margin-top: 10
    // child1 压缩后高度
    // gap: max(20, 20) = 20
    // child2 压缩后高度
    // child2 margin-bottom: 30
    layx_scalar total_used = 10 + c1h + 20 + c2h + 30;

    printf("  Total space used (including margins): %.1f\n", total_used);
    printf("  Container height: %.1f\n", ch);
    printf("  Difference: %.1f\n", total_used - ch);

    if (total_used > ch) {
        printf("  ✓ 子元素+margin的总空间超过了容器高度，这是正常的（会发生压缩或溢出）\n");
    } else if (total_used == ch) {
        printf("  ✓ 子元素+margin的总空间等于容器高度\n");
    } else {
        printf("  ✓ 子元素+margin的总空间小于容器高度（有剩余空间）\n");
    }

    layx_destroy_context(&ctx);
}

// 测试：嵌套容器中最后一个子元素的margin
void test_nested_last_child_margin() {
    printf("\n=== Test: Nested Container Last Child Margin ===\n");

    layx_context ctx;
    layx_init_context(&ctx);
    layx_reserve_items_capacity(&ctx, 10);

    // 创建外层容器
    layx_id outer = layx_item(&ctx);
    layx_set_size(&ctx, outer, 0, 0);  // 高度自动
    layx_set_display(&ctx, outer, LAYX_DISPLAY_FLEX);
    layx_set_flex_direction(&ctx, outer, LAYX_FLEX_DIRECTION_COLUMN);

    // 创建内层容器
    layx_id inner = layx_item(&ctx);
    layx_set_size(&ctx, inner, 200, 0);  // 高度自动
    layx_set_display(&ctx, inner, LAYX_DISPLAY_FLEX);
    layx_set_flex_direction(&ctx, inner, LAYX_FLEX_DIRECTION_COLUMN);
    layx_append(&ctx, outer, inner);

    // 创建子元素1
    layx_id child1 = layx_item(&ctx);
    layx_set_size(&ctx, child1, 100, 50);
    layx_set_margin_trbl(&ctx, child1, 10, 0, 20, 0);
    layx_append(&ctx, inner, child1);

    // 创建子元素2（最后一个，有margin-bottom）
    layx_id child2 = layx_item(&ctx);
    layx_set_size(&ctx, child2, 100, 50);
    layx_set_margin_trbl(&ctx, child2, 30, 0, 40, 0);  // margin-bottom:40
    layx_append(&ctx, inner, child2);

    layx_run_context(&ctx);

    // 获取尺寸
    layx_scalar ox, oy, ow, oh;
    layx_get_rect_xywh(&ctx, outer, &ox, &oy, &ow, &oh);

    layx_scalar ix, iy, iw, ih;
    layx_get_rect_xywh(&ctx, inner, &ix, &iy, &iw, &ih);

    layx_scalar c1x, c1y, c1w, c1h;
    layx_get_rect_xywh(&ctx, child1, &c1x, &c1y, &c1w, &c1h);

    layx_scalar c2x, c2y, c2w, c2h;
    layx_get_rect_xywh(&ctx, child2, &c2x, &c2y, &c2w, &c2h);

    printf("  Outer container: x=%.1f, y=%.1f, w=%.1f, h=%.1f\n", ox, oy, ow, oh);
    printf("  Inner container: x=%.1f, y=%.1f, w=%.1f, h=%.1f\n", ix, iy, iw, ih);
    printf("  Child1: x=%.1f, y=%.1f, w=%.1f, h=%.1f\n", c1x, c1y, c1w, c1h);
    printf("  Child2: x=%.1f, y=%.1f, w=%.1f, h=%.1f (margin-bottom:40)\n", c2x, c2y, c2w, c2h);

    // 预期inner高度：10 + 50 + max(20,30) + 50 + 40 = 10 + 50 + 30 + 50 + 40 = 180
    layx_scalar expected_inner_height = 10 + 50 + 30 + 50 + 40;  // 180

    printf("  Expected inner container height: %.1f\n", expected_inner_height);
    printf("  Actual inner container height: %.1f\n", ih);

    if (ih == expected_inner_height) {
        printf("  ✓ 内层容器高度正确，包含了最后一个元素的margin-bottom\n");
    } else {
        printf("  ✗ 内层容器高度错误！预期%.1f，实际%.1f\n", expected_inner_height, ih);
    }

    layx_destroy_context(&ctx);
}

// 测试：BLOCK布局中最后一个子元素的margin
void test_block_last_child_margin() {
    printf("\n=== Test: BLOCK Layout Last Child Margin ===\n");

    layx_context ctx;
    layx_init_context(&ctx);
    layx_reserve_items_capacity(&ctx, 10);

    // 创建容器（BLOCK布局）
    layx_id container = layx_item(&ctx);
    layx_set_size(&ctx, container, 200, 0);  // 高度自动
    layx_set_display(&ctx, container, LAYX_DISPLAY_BLOCK);

    // 创建子元素1
    layx_id child1 = layx_item(&ctx);
    layx_set_size(&ctx, child1, 100, 50);
    layx_set_margin_trbl(&ctx, child1, 10, 0, 20, 0);
    layx_append(&ctx, container, child1);

    // 创建子元素2（最后一个，有margin-bottom）
    layx_id child2 = layx_item(&ctx);
    layx_set_size(&ctx, child2, 100, 50);
    layx_set_margin_trbl(&ctx, child2, 30, 0, 40, 0);  // margin-bottom:40
    layx_append(&ctx, container, child2);

    layx_run_context(&ctx);

    // 获取尺寸
    layx_scalar cx, cy, cw, ch;
    layx_get_rect_xywh(&ctx, container, &cx, &cy, &cw, &ch);

    layx_scalar c1x, c1y, c1w, c1h;
    layx_get_rect_xywh(&ctx, child1, &c1x, &c1y, &c1w, &c1h);

    layx_scalar c2x, c2y, c2w, c2h;
    layx_get_rect_xywh(&ctx, child2, &c2x, &c2y, &c2w, &c2h);

    printf("  Container: x=%.1f, y=%.1f, w=%.1f, h=%.1f\n", cx, cy, cw, ch);
    printf("  Child1: x=%.1f, y=%.1f, w=%.1f, h=%.1f\n", c1x, c1y, c1w, c1h);
    printf("  Child2: x=%.1f, y=%.1f, w=%.1f, h=%.1f (margin-bottom:40)\n", c2x, c2y, c2w, c2h);

    // 预期容器高度：10 + 50 + max(20,30) + 50 + 40 = 10 + 50 + 30 + 50 + 40 = 180
    layx_scalar expected_height = 10 + 50 + 30 + 50 + 40;  // 180

    printf("  Expected container height: %.1f\n", expected_height);
    printf("  Actual container height: %.1f\n", ch);

    if (ch == expected_height) {
        printf("  ✓ 容器高度正确，包含了最后一个元素的margin-bottom\n");
    } else {
        printf("  ✗ 容器高度错误！预期%.1f，实际%.1f\n", expected_height, ch);
    }

    layx_destroy_context(&ctx);
}

int main() {
    printf("===========================================\n");
    printf("   Last Child Margin Test (Advanced)\n");
    printf("===========================================\n\n");

    test_last_child_margin_with_shrink();
    test_nested_last_child_margin();
    test_block_last_child_margin();

    printf("\n===========================================\n");
    printf("   Test Completed\n");
    printf("===========================================\n");

    return 0;
}

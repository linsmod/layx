#include "layx.h"
#include <stdio.h>
#include <assert.h>

// 测试：最后一个子元素的margin-bottom应该影响容器高度
void test_last_child_margin_bottom() {
    printf("=== Test: Last Child Margin-Bottom Affects Container Height ===\n");

    layx_context ctx;
    layx_init_context(&ctx);
    layx_reserve_items_capacity(&ctx, 10);

    // 创建容器（FLEX COLUMN，固定宽度，高度由内容决定）
    layx_id container = layx_item(&ctx);
    layx_set_size(&ctx, container, 200, 0);  // 固定宽度200，高度自动
    layx_set_display(&ctx, container, LAYX_DISPLAY_FLEX);
    layx_set_flex_direction(&ctx, container, LAYX_FLEX_DIRECTION_COLUMN);

    // 创建子元素1：高度50，margin-bottom: 0
    layx_id child1 = layx_item(&ctx);
    layx_set_size(&ctx, child1, 100, 50);
    layx_set_margin_trbl(&ctx, child1, 10, 0, 0, 0);  // margin-top:10
    layx_append(&ctx, container, child1);

    // 创建子元素2：高度50，margin-bottom: 30（关键测试点）
    layx_id child2 = layx_item(&ctx);
    layx_set_size(&ctx, child2, 100, 50);
    layx_set_margin_trbl(&ctx, child2, 20, 0, 30, 0);  // margin-top:20, margin-bottom:30
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

    // 验证：容器高度应该包含最后一个元素的margin-bottom
    // 预期：
    //   child1: y=10 (margin-top), h=50
    //   child2: y=50+10+20=80 (child1高度+margin-bottom+child2的margin-top), h=50
    //   容器高度: child2.y + child2.h + child2的margin-bottom = 80 + 50 + 30 = 160
    // 但还要加上第一个元素的margin-top=10
    // 所以容器高度应该是: 160 + 10 = 170？不对，应该重新计算

    printf("  Container: x=%.1f, y=%.1f, w=%.1f, h=%.1f\n", cx, cy, cw, ch);
    printf("  Child1: x=%.1f, y=%.1f, w=%.1f, h=%.1f (margin-top:10, margin-bottom:0)\n", c1x, c1y, c1w, c1h);
    printf("  Child2: x=%.1f, y=%.1f, w=%.1f, h=%.1f (margin-top:20, margin-bottom:30)\n", c2x, c2y, c2w, c2h);

    // 重新计算预期容器高度
    // child1的margin-top: 10
    // child1的高度: 50
    // child1和child2之间的gap: max(child1的margin-bottom, child2的margin-top) = max(0, 20) = 20
    // child2的高度: 50
    // child2的margin-bottom: 30
    // 总高度: 10 + 50 + 20 + 50 + 30 = 160

    layx_scalar expected_height = 10 + 50 + 20 + 50 + 30;  // 160

    printf("  Expected container height: %.1f\n", expected_height);
    printf("  Actual container height: %.1f\n", ch);

    if (ch == expected_height) {
        printf("  ✓ 容器高度正确，包含了最后一个元素的margin-bottom\n");
    } else {
        printf("  ✗ 容器高度错误！预期%.1f，实际%.1f，差异%.1f\n", expected_height, ch, ch - expected_height);
        printf("  说明：最后一个元素的margin-bottom没有正确影响容器高度\n");
    }

    layx_destroy_context(&ctx);
}

// 测试：FLEX ROW最后一个子元素的margin-right
void test_last_child_margin_right() {
    printf("\n=== Test: Last Child Margin-Right Affects Container Width ===\n");

    layx_context ctx;
    layx_init_context(&ctx);
    layx_reserve_items_capacity(&ctx, 10);

    // 创建容器（FLEX ROW，固定高度，宽度由内容决定）
    layx_id container = layx_item(&ctx);
    layx_set_size(&ctx, container, 0, 100);  // 高度固定100，宽度自动
    layx_set_display(&ctx, container, LAYX_DISPLAY_FLEX);
    layx_set_flex_direction(&ctx, container, LAYX_FLEX_DIRECTION_ROW);

    // 创建子元素1：宽度50，margin-right: 0
    layx_id child1 = layx_item(&ctx);
    layx_set_size(&ctx, child1, 50, 30);
    layx_set_margin_trbl(&ctx, child1, 0, 0, 0, 10);  // margin-left:10
    layx_append(&ctx, container, child1);

    // 创建子元素2：宽度50，margin-right: 30（关键测试点）
    layx_id child2 = layx_item(&ctx);
    layx_set_size(&ctx, child2, 50, 30);
    layx_set_margin_trbl(&ctx, child2, 0, 30, 0, 20);  // margin-left:20, margin-right:30
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

    printf("  Container: x=%.1f, y=%.1f, w=%.1f, h=%.1f\n", cx, cy, cw, ch);
    printf("  Child1: x=%.1f, y=%.1f, w=%.1f, h=%.1f (margin-left:10, margin-right:0)\n", c1x, c1y, c1w, c1h);
    printf("  Child2: x=%.1f, y=%.1f, w=%.1f, h=%.1f (margin-left:20, margin-right:30)\n", c2x, c2y, c2w, c2h);

    // 预期容器宽度：
    // child1的margin-left: 10
    // child1的宽度: 50
    // child1和child2之间的gap: max(child1的margin-right, child2的margin-left) = max(0, 20) = 20
    // child2的宽度: 50
    // child2的margin-right: 30
    // 总宽度: 10 + 50 + 20 + 50 + 30 = 160

    layx_scalar expected_width = 10 + 50 + 20 + 50 + 30;  // 160

    printf("  Expected container width: %.1f\n", expected_width);
    printf("  Actual container width: %.1f\n", cw);

    if (cw == expected_width) {
        printf("  ✓ 容器宽度正确，包含了最后一个元素的margin-right\n");
    } else {
        printf("  ✗ 容器宽度错误！预期%.1f，实际%.1f，差异%.1f\n", expected_width, cw, cw - expected_width);
        printf("  说明：最后一个元素的margin-right没有正确影响容器宽度\n");
    }

    layx_destroy_context(&ctx);
}

int main() {
    printf("===========================================\n");
    printf("   Last Child Margin Test\n");
    printf("===========================================\n\n");

    test_last_child_margin_bottom();
    test_last_child_margin_right();

    printf("\n===========================================\n");
    printf("   Test Completed\n");
    printf("===========================================\n");

    return 0;
}

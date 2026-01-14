/**
 * @file test_defaults.c
 * @brief 测试未设置属性时的默认值是否符合CSS Flexbox规范
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "layx.h"

// 测试计数
static int tests_passed = 0;
static int tests_failed = 0;

// 测试辅助宏
#define TEST_ASSERT(condition, message) \
    do { \
        if (condition) { \
            printf("  ✓ %s\n", message); \
            tests_passed++; \
        } else { \
            printf("  ✗ %s\n", message); \
            tests_failed++; \
        } \
    } while(0)

// 提取属性值
static layx_flex_direction get_flex_direction(const layx_item_t *item) {
    return (layx_flex_direction)(item->flags & LAYX_FLEX_DIRECTION_MASK);
}

static layx_flex_wrap get_flex_wrap(const layx_item_t *item) {
    return (layx_flex_wrap)(item->flags & LAYX_FLEX_WRAP_MASK);
}

static layx_justify_content get_justify_content(const layx_item_t *item) {
    return (layx_justify_content)(item->flags & LAYX_JUSTIFY_CONTENT_MASK);
}

static layx_align_items get_align_items(const layx_item_t *item) {
    return (layx_align_items)(item->flags & LAYX_ALIGN_ITEMS_MASK);
}

static layx_align_content get_align_content(const layx_item_t *item) {
    return (layx_align_content)(item->flags & LAYX_ALIGN_CONTENT_MASK);
}

static layx_align_self get_align_self(const layx_item_t *item) {
    return (layx_align_self)(item->flags & LAYX_ALIGN_SELF_MASK);
}

static bool has_fixed_width(const layx_item_t *item) {
    return (item->flags & LAYX_SIZE_FIXED_WIDTH) != 0;
}

static bool has_fixed_height(const layx_item_t *item) {
    return (item->flags & LAYX_SIZE_FIXED_HEIGHT) != 0;
}

static bool is_flex_container_test(const layx_item_t *item) {
    return layx_is_flex_container(item->flags);
}

/**
 * CSS Flexbox 默认值规范：
 * 
 * 容器属性：
 * - display: flex (需要显式设置)
 * - flex-direction: row
 * - flex-wrap: nowrap
 * - justify-content: flex-start
 * - align-items: stretch
 * - align-content: stretch
 * 
 * Item属性：
 * - flex-grow: 0
 * - flex-shrink: 1
 * - flex-basis: auto
 * - align-self: auto (继承父容器)
 */

void test_container_defaults(void) {
    printf("\n=== Test: Flex容器默认值 ===\n");
    
    layx_context ctx;
    layx_init_context(&ctx);
    
    layx_id container = layx_item(&ctx);
    layx_item_t *item = layx_get_item(&ctx, container);
    
    // CSS: flex-direction默认为row
    layx_flex_direction dir = get_flex_direction(item);
    TEST_ASSERT(dir == LAYX_FLEX_DIRECTION_ROW, 
                "flex-direction默认值为ROW (CSS规范: row)");
    
    // CSS: flex-wrap默认为nowrap
    layx_flex_wrap wrap = get_flex_wrap(item);
    TEST_ASSERT(wrap == LAYX_FLEX_WRAP_NOWRAP,
                "flex-wrap默认值为NOWRAP (CSS规范: nowrap)");
    
    // CSS: justify-content默认为flex-start
    layx_justify_content justify = get_justify_content(item);
    TEST_ASSERT(justify == LAYX_JUSTIFY_FLEX_START,
                "justify-content默认值为FLEX_START (CSS规范: flex-start)");
    
    // CSS: align-items默认为stretch
    layx_align_items align = get_align_items(item);
    TEST_ASSERT(align == LAYX_ALIGN_ITEMS_STRETCH,
                "align-items默认值为STRETCH (CSS规范: stretch)");
    
    // CSS: align-content默认为stretch
    layx_align_content content = get_align_content(item);
    TEST_ASSERT(content == LAYX_ALIGN_CONTENT_STRETCH,
                "align-content默认值为STRETCH (CSS规范: stretch)");
    
    // CSS: align-self默认为auto (在layx中表示为0，未设置)
    layx_align_self self = get_align_self(item);
    TEST_ASSERT(self == 0,
                "align-self默认为auto (未设置，继承父容器的align-items)");
    
    // 默认不是flex容器
    TEST_ASSERT(!is_flex_container_test(item),
                "默认display不是flex (需要显式设置)");
    
    // flex-grow默认为0
    TEST_ASSERT(item->flex_grow == 0.0f,
                "flex-grow默认值为0 (CSS规范: 0)");
    
    // flex-shrink默认为1
    TEST_ASSERT(item->flex_shrink == 1.0f,
                "flex-shrink默认值为1 (CSS规范: 1)");
    
    // flex-basis默认为auto (在layx中通过size=0表示)
    TEST_ASSERT(item->flex_basis == 0.0f,
                "flex-basis默认为auto (layx实现: 0)");
    
    // 默认size为0
    TEST_ASSERT(item->size[0] == 0.0f && item->size[1] == 0.0f,
                "width/height默认为auto (layx实现: 0)");
    
    // 默认没有固定宽度/高度标志
    TEST_ASSERT(!has_fixed_width(item),
                "默认没有固定宽度标志");
    TEST_ASSERT(!has_fixed_height(item),
                "默认没有固定高度标志");
    
    // 默认margin/padding/border为0
    TEST_ASSERT(item->margins[0] == 0.0f && item->margins[1] == 0.0f && 
                item->margins[2] == 0.0f && item->margins[3] == 0.0f,
                "margin默认为0 (CSS规范: 0)");
    TEST_ASSERT(item->padding[0] == 0.0f && item->padding[1] == 0.0f && 
                item->padding[2] == 0.0f && item->padding[3] == 0.0f,
                "padding默认为0 (CSS规范: 0)");
    TEST_ASSERT(item->border[0] == 0.0f && item->border[1] == 0.0f && 
                item->border[2] == 0.0f && item->border[3] == 0.0f,
                "border默认为0 (CSS规范: 0)");
    
    layx_destroy_context(&ctx);
}

void test_item_defaults(void) {
    printf("\n=== Test: Flex Item默认值 ===\n");
    
    layx_context ctx;
    layx_init_context(&ctx);
    
    layx_id item = layx_item(&ctx);
    layx_item_t *pitem = layx_get_item(&ctx, item);
    
    // CSS: item的所有对齐属性默认为auto/未设置
    TEST_ASSERT(get_align_items(pitem) == LAYX_ALIGN_ITEMS_STRETCH,
                "item的align-items默认为STRETCH");
    TEST_ASSERT(get_align_content(pitem) == LAYX_ALIGN_CONTENT_STRETCH,
                "item的align-content默认为STRETCH");
    TEST_ASSERT(get_align_self(pitem) == 0,
                "item的align-self默认为auto (未设置)");
    
    // CSS: flex-grow默认为0
    TEST_ASSERT(pitem->flex_grow == 0.0f,
                "item的flex-grow默认为0");
    
    // CSS: flex-shrink默认为1
    TEST_ASSERT(pitem->flex_shrink == 1.0f,
                "item的flex-shrink默认为1");
    
    // CSS: flex-basis默认为auto
    TEST_ASSERT(pitem->flex_basis == 0.0f,
                "item的flex-basis默认为auto (layx实现: 0)");
    
    layx_destroy_context(&ctx);
}

void test_flex_container_defaults(void) {
    printf("\n=== Test: 设置为Flex容器后的默认值 ===\n");
    
    layx_context ctx;
    layx_init_context(&ctx);
    
    layx_id container = layx_item(&ctx);
    layx_set_display(&ctx, container, LAYX_DISPLAY_FLEX);
    
    layx_item_t *item = layx_get_item(&ctx, container);
    
    // 设置为flex容器后，其他默认值保持不变
    TEST_ASSERT(get_flex_direction(item) == LAYX_FLEX_DIRECTION_ROW,
                "flex-container的flex-direction仍为ROW");
    TEST_ASSERT(get_flex_wrap(item) == LAYX_FLEX_WRAP_NOWRAP,
                "flex-container的flex-wrap仍为NOWRAP");
    TEST_ASSERT(get_justify_content(item) == LAYX_JUSTIFY_FLEX_START,
                "flex-container的justify-content仍为FLEX_START");
    TEST_ASSERT(get_align_items(item) == LAYX_ALIGN_ITEMS_STRETCH,
                "flex-container的align-items仍为STRETCH");
    
    layx_destroy_context(&ctx);
}

void test_size_defaults(void) {
    printf("\n=== Test: 尺寸相关默认值 ===\n");
    
    layx_context ctx;
    layx_init_context(&ctx);
    
    layx_id item = layx_item(&ctx);
    layx_item_t *pitem = layx_get_item(&ctx, item);
    
    // CSS: width/height默认为auto
    TEST_ASSERT(pitem->size[0] == 0.0f && pitem->size[1] == 0.0f,
                "width/height默认为auto (layx: 0表示auto)");
    
    // CSS: min-width/min-height默认为0
    TEST_ASSERT(pitem->min_size[0] == 0.0f && pitem->min_size[1] == 0.0f,
                "min-width/min-height默认为0");
    
    // CSS: max-width/max-height默认为none
    TEST_ASSERT(pitem->max_size[0] == 0.0f && pitem->max_size[1] == 0.0f,
                "max-width/max-height默认为none (layx: 0表示none)");
    
    // 设置固定尺寸后的标志
    layx_set_size(&ctx, item, 100, 200);
    pitem = layx_get_item(&ctx, item);
    TEST_ASSERT(has_fixed_width(pitem) && has_fixed_height(pitem),
                "设置size后，fixed标志被正确设置");
    TEST_ASSERT(pitem->size[0] == 100.0f && pitem->size[1] == 200.0f,
                "size值被正确保存");
    
    layx_destroy_context(&ctx);
}

void test_flex_properties_defaults(void) {
    printf("\n=== Test: Flex属性默认值 ===\n");
    
    layx_context ctx;
    layx_init_context(&ctx);
    
    layx_id item = layx_item(&ctx);
    layx_item_t *pitem = layx_get_item(&ctx, item);
    
    // CSS: flex-grow默认为0
    TEST_ASSERT(pitem->flex_grow == 0.0f,
                "flex-grow默认值为0");
    
    // CSS: flex-shrink默认为1
    TEST_ASSERT(pitem->flex_shrink == 1.0f,
                "flex-shrink默认值为1");
    
    // CSS: flex-basis默认为auto
    TEST_ASSERT(pitem->flex_basis == 0.0f,
                "flex-basis默认为auto (layx: 0)");
    
    // 设置flex-grow后的状态
    layx_set_flex_grow(&ctx, item, 2.0f);
    pitem = layx_get_item(&ctx, item);
    TEST_ASSERT(pitem->flex_grow == 2.0f,
                "flex-grow可以被正确设置");
    
    // CSS flex简写: flex: 0 1 auto
    // 在layx中需要分别设置
    layx_set_flex_properties(&ctx, item, 1, 1, 0);
    pitem = layx_get_item(&ctx, item);
    TEST_ASSERT(pitem->flex_grow == 1.0f && 
                pitem->flex_shrink == 1.0f && 
                pitem->flex_basis == 0.0f,
                "flex属性可以被正确设置");
    
    layx_destroy_context(&ctx);
}

void test_margin_padding_border_defaults(void) {
    printf("\n=== Test: Margin/Padding/Border默认值 ===\n");
    
    layx_context ctx;
    layx_init_context(&ctx);
    
    layx_id item = layx_item(&ctx);
    layx_item_t *pitem = layx_get_item(&ctx, item);
    
    // CSS: margin默认为0
    TEST_ASSERT(pitem->margins[0] == 0.0f && pitem->margins[1] == 0.0f && 
                pitem->margins[2] == 0.0f && pitem->margins[3] == 0.0f,
                "margin默认为0");
    
    // CSS: padding默认为0
    TEST_ASSERT(pitem->padding[0] == 0.0f && pitem->padding[1] == 0.0f && 
                pitem->padding[2] == 0.0f && pitem->padding[3] == 0.0f,
                "padding默认为0");
    
    // CSS: border默认为0
    TEST_ASSERT(pitem->border[0] == 0.0f && pitem->border[1] == 0.0f && 
                pitem->border[2] == 0.0f && pitem->border[3] == 0.0f,
                "border默认为0");
    
    // 设置margin后的状态
    layx_set_margin_ltrb(&ctx, item, 10, 20, 30, 40);
    pitem = layx_get_item(&ctx, item);
    TEST_ASSERT(pitem->margins[0] == 10.0f && pitem->margins[1] == 20.0f &&
                pitem->margins[2] == 30.0f && pitem->margins[3] == 40.0f,
                "margin可以被正确设置 (left, top, right, bottom)");
    
    // 设置padding后的状态
    layx_set_padding_ltrb(&ctx, item, 5, 10, 15, 20);
    pitem = layx_get_item(&ctx, item);
    TEST_ASSERT(pitem->padding[0] == 5.0f && pitem->padding[1] == 10.0f &&
                pitem->padding[2] == 15.0f && pitem->padding[3] == 20.0f,
                "padding可以被正确设置");
    
    // 设置border后的状态
    layx_set_border_ltrb(&ctx, item, 2, 3, 4, 5);
    pitem = layx_get_item(&ctx, item);
    TEST_ASSERT(pitem->border[0] == 2.0f && pitem->border[1] == 3.0f &&
                pitem->border[2] == 4.0f && pitem->border[3] == 5.0f,
                "border可以被正确设置");
    
    layx_destroy_context(&ctx);
}

void test_direction_defaults(void) {
    printf("\n=== Test: Flex方向默认值 ===\n");
    
    layx_context ctx;
    layx_init_context(&ctx);
    
    layx_id item = layx_item(&ctx);
    layx_item_t *pitem = layx_get_item(&ctx, item);
    
    // CSS: flex-direction默认为row
    layx_flex_direction dir = get_flex_direction(pitem);
    TEST_ASSERT(dir == LAYX_FLEX_DIRECTION_ROW,
                "flex-direction默认为ROW");
    
    // 测试其他方向
    layx_set_flex_direction(&ctx, item, LAYX_FLEX_DIRECTION_COLUMN);
    pitem = layx_get_item(&ctx, item);
    dir = get_flex_direction(pitem);
    TEST_ASSERT(dir == LAYX_FLEX_DIRECTION_COLUMN,
                "flex-direction可以设置为COLUMN");
    
    layx_set_flex_direction(&ctx, item, LAYX_FLEX_DIRECTION_ROW_REVERSE);
    pitem = layx_get_item(&ctx, item);
    dir = get_flex_direction(pitem);
    TEST_ASSERT(dir == LAYX_FLEX_DIRECTION_ROW_REVERSE,
                "flex-direction可以设置为ROW_REVERSE");
    
    layx_set_flex_direction(&ctx, item, LAYX_FLEX_DIRECTION_COLUMN_REVERSE);
    pitem = layx_get_item(&ctx, item);
    dir = get_flex_direction(pitem);
    TEST_ASSERT(dir == LAYX_FLEX_DIRECTION_COLUMN_REVERSE,
                "flex-direction可以设置为COLUMN_REVERSE");
    
    layx_destroy_context(&ctx);
}

void test_wrap_defaults(void) {
    printf("\n=== Test: Flex Wrap默认值 ===\n");
    
    layx_context ctx;
    layx_init_context(&ctx);
    
    layx_id item = layx_item(&ctx);
    layx_item_t *pitem = layx_get_item(&ctx, item);
    
    // CSS: flex-wrap默认为nowrap
    layx_flex_wrap wrap = get_flex_wrap(pitem);
    TEST_ASSERT(wrap == LAYX_FLEX_WRAP_NOWRAP,
                "flex-wrap默认为NOWRAP");
    
    // 测试其他wrap值
    layx_set_flex_wrap(&ctx, item, LAYX_FLEX_WRAP_WRAP);
    pitem = layx_get_item(&ctx, item);
    wrap = get_flex_wrap(pitem);
    TEST_ASSERT(wrap == LAYX_FLEX_WRAP_WRAP,
                "flex-wrap可以设置为WRAP");
    
    layx_set_flex_wrap(&ctx, item, LAYX_FLEX_WRAP_WRAP_REVERSE);
    pitem = layx_get_item(&ctx, item);
    wrap = get_flex_wrap(pitem);
    TEST_ASSERT(wrap == LAYX_FLEX_WRAP_WRAP_REVERSE,
                "flex-wrap可以设置为WRAP_REVERSE");
    
    layx_destroy_context(&ctx);
}

void test_alignment_defaults(void) {
    printf("\n=== Test: 对齐属性默认值 ===\n");
    
    layx_context ctx;
    layx_init_context(&ctx);
    
    layx_id container = layx_item(&ctx);
    layx_set_display(&ctx, container, LAYX_DISPLAY_FLEX);
    
    layx_id item = layx_item(&ctx);
    layx_append(&ctx, container, item);
    
    layx_item_t *pcontainer = layx_get_item(&ctx, container);
    layx_item_t *pitem = layx_get_item(&ctx, item);
    
    // CSS: justify-content默认为flex-start
    layx_justify_content justify = get_justify_content(pcontainer);
    TEST_ASSERT(justify == LAYX_JUSTIFY_FLEX_START,
                "justify-content默认为FLEX_START");
    
    // CSS: align-items默认为stretch
    layx_align_items align = get_align_items(pcontainer);
    TEST_ASSERT(align == LAYX_ALIGN_ITEMS_STRETCH,
                "align-items默认为STRETCH");
    
    // CSS: align-content默认为stretch
    layx_align_content content = get_align_content(pcontainer);
    TEST_ASSERT(content == LAYX_ALIGN_CONTENT_STRETCH,
                "align-content默认为STRETCH");
    
    // CSS: align-self默认为auto (未设置，继承父容器)
    layx_align_self self = get_align_self(pitem);
    TEST_ASSERT(self == 0,
                "align-self默认为auto (未设置)");
    
    // 设置align-self后应该覆盖容器的align-items
    layx_set_align_self(&ctx, item, LAYX_ALIGN_SELF_CENTER);
    pitem = layx_get_item(&ctx, item);
    self = get_align_self(pitem);
    TEST_ASSERT(self == LAYX_ALIGN_SELF_CENTER,
                "align-self可以覆盖父容器的align-items");
    
    layx_destroy_context(&ctx);
}

void test_complete_defaults(void) {
    printf("\n=== Test: 完整默认值检查 ===\n");
    
    layx_context ctx;
    layx_init_context(&ctx);
    
    // 创建一个完整的flex布局
    layx_id root = layx_item(&ctx);
    layx_set_size(&ctx, root, 800, 600);
    layx_set_display(&ctx, root, LAYX_DISPLAY_FLEX);
    
    layx_id container = layx_item(&ctx);
    layx_append(&ctx, root, container);
    
    layx_id item1 = layx_item(&ctx);
    layx_set_size(&ctx, item1, 100, 100);
    layx_append(&ctx, container, item1);
    
    layx_id item2 = layx_item(&ctx);
    layx_append(&ctx, container, item2);
    
    // 运行布局
    layx_run_context(&ctx);
    
    // 检查默认布局行为
    layx_item_t *proot = layx_get_item(&ctx, root);
    layx_item_t *pcontainer = layx_get_item(&ctx, container);
    (void)pcontainer; // 未使用，避免警告
    layx_item_t *pitem1 = layx_get_item(&ctx, item1);
    layx_item_t *pitem2 = layx_get_item(&ctx, item2);
    (void)pitem1; // 未使用，避免警告
    (void)pitem2; // 未使用，避免警告
    
    // root默认使用row方向
    TEST_ASSERT(get_flex_direction(proot) == LAYX_FLEX_DIRECTION_ROW,
                "root使用默认的row方向");
    
    // container继承默认方向
    TEST_ASSERT(get_flex_direction(pcontainer) == LAYX_FLEX_DIRECTION_ROW,
                "container继承默认的row方向");
    
    // justify-content默认为flex-start，item应该从左边开始
    layx_vec4 rect1 = ctx.rects[item1];
    layx_vec4 rect2 = ctx.rects[item2];
    TEST_ASSERT(rect1[0] >= 0.0f && rect2[0] >= rect1[0],
                "item使用默认的flex-start对齐");
    
    layx_destroy_context(&ctx);
}

void test_flags_initialization(void) {
    printf("\n=== Test: Flags初始化 ===\n");
    
    layx_context ctx;
    layx_init_context(&ctx);
    
    layx_id item = layx_item(&ctx);
    layx_item_t *pitem = layx_get_item(&ctx, item);
    
    // 新创建的item的flags应该是干净的（只有可能设置的属性）
    uint32_t flags = pitem->flags;
    
    // 检查flags的各个部分
    TEST_ASSERT((flags & LAYX_FLEX_DIRECTION_MASK) == LAYX_FLEX_DIRECTION_ROW,
                "flags中flex-direction部分正确");
    TEST_ASSERT((flags & LAYX_FLEX_WRAP_MASK) == LAYX_FLEX_WRAP_NOWRAP,
                "flags中flex-wrap部分正确");
    TEST_ASSERT((flags & LAYX_JUSTIFY_CONTENT_MASK) == LAYX_JUSTIFY_FLEX_START,
                "flags中justify-content部分正确");
    TEST_ASSERT((flags & LAYX_ALIGN_ITEMS_MASK) == LAYX_ALIGN_ITEMS_STRETCH,
                "flags中align-items部分正确");
    TEST_ASSERT((flags & LAYX_ALIGN_CONTENT_MASK) == LAYX_ALIGN_CONTENT_STRETCH,
                "flags中align-content部分正确");
    TEST_ASSERT((flags & LAYX_ALIGN_SELF_MASK) == 0,
                "flags中align-self未设置（auto）");
    
    layx_destroy_context(&ctx);
}

static void test_getter_functions(void) {
    printf("\n=== Test: Box Model Getter函数 ===\n");
    
    layx_context ctx;
    layx_id root, container, item;
    layx_scalar l, t, r, b;
    
    layx_init_context(&ctx);
    layx_reserve_items_capacity(&ctx, 10);
    root = layx_item(&ctx);
    container = layx_item(&ctx);
    item = layx_item(&ctx);
    layx_prepend(&ctx, root, container);
    layx_prepend(&ctx, container, item);
    
    // 测试默认值为0
    layx_get_margin_ltrb(&ctx, item, &l, &t, &r, &b);
    TEST_ASSERT(l == 0 && t == 0 && r == 0 && b == 0, "默认margin getter返回0");
    layx_get_padding_ltrb(&ctx, item, &l, &t, &r, &b);
    TEST_ASSERT(l == 0 && t == 0 && r == 0 && b == 0, "默认padding getter返回0");
    layx_get_border_ltrb(&ctx, item, &l, &t, &r, &b);
    TEST_ASSERT(l == 0 && t == 0 && r == 0 && b == 0, "默认border getter返回0");
    
    // 测试设置后返回值正确
    layx_set_margin_ltrb(&ctx, item, 10, 20, 30, 40);
    layx_get_margin_ltrb(&ctx, item, &l, &t, &r, &b);
    TEST_ASSERT(l == 10 && t == 20 && r == 30 && b == 40, "margin getter正确返回ltrb值(10,20,30,40)");
    
    layx_set_padding_ltrb(&ctx, item, 5, 15, 25, 35);
    layx_get_padding_ltrb(&ctx, item, &l, &t, &r, &b);
    TEST_ASSERT(l == 5 && t == 15 && r == 25 && b == 35, "padding getter正确返回ltrb值(5,15,25,35)");
    
    layx_set_border_ltrb(&ctx, item, 1, 2, 3, 4);
    layx_get_border_ltrb(&ctx, item, &l, &t, &r, &b);
    TEST_ASSERT(l == 1 && t == 2 && r == 3 && b == 4, "border getter正确返回ltrb值(1,2,3,4)");
    
    // 测试混合设置
    layx_set_margin_left(&ctx, item, 100);
    layx_set_margin_top(&ctx, item, 200);
    layx_get_margin_ltrb(&ctx, item, &l, &t, &r, &b);
    TEST_ASSERT(l == 100 && t == 200 && r == 30 && b == 40, "混合设置后margin getter正确返回(100,200,30,40)");
    
    layx_destroy_context(&ctx);
    printf("  \u2713 margin/padding/border getter函数测试完成\n");
}

int main(void) {
    printf("===========================================\n");
    printf("   LayX CSS规范默认值测试套件\n");
    printf("===========================================\n");
    
    // 运行所有测试
    test_container_defaults();
    test_item_defaults();
    test_flex_container_defaults();
    test_size_defaults();
    test_flex_properties_defaults();
    test_margin_padding_border_defaults();
    test_direction_defaults();
    test_wrap_defaults();
    test_alignment_defaults();
    test_complete_defaults();
    test_flags_initialization();
    test_getter_functions();
    
    // 输出测试结果
    printf("\n===========================================\n");
    printf("           测试总结\n");
    printf("===========================================\n");
    printf("通过测试: %d\n", tests_passed);
    printf("失败测试: %d\n", tests_failed);
    printf("总测试数: %d\n", tests_passed + tests_failed);
    printf("===========================================\n");
    
    if (tests_failed == 0) {
        printf("✓ 所有测试通过！\n");
    } else {
        printf("✗ 有测试失败！\n");
    }
    
    return 0;
}
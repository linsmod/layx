#include "layx.h"
#include <string.h>
#include <stdio.h>

// 简单的 MAX 宏
#define LAYX_MAX(a, b) ((a) > (b) ? (a) : (b))

// 初始化项目的滚动相关字段
void layx_init_scroll_fields(layx_context *ctx, layx_id item) {
    LAYX_ASSERT(ctx != NULL && item != LAYX_INVALID_ID);
    layx_item_t *pitem = layx_get_item(ctx, item);
    
    // 初始化滚动偏移量为0
    pitem->scroll_offset[0] = 0.0f;  // scrollLeft
    pitem->scroll_offset[1] = 0.0f;  // scrollTop
    
    // 初始化滚动最大值为0（需要先设置内容尺寸后更新）
    pitem->scroll_max[0] = 0.0f;     // maxScrollLeft
    pitem->scroll_max[1] = 0.0f;     // maxScrollTop
    
    // 初始化内容尺寸为项目尺寸（如果没有子项，内容尺寸等于项目尺寸）
    pitem->content_size[0] = pitem->size[0];
    pitem->content_size[1] = pitem->size[1];
    
    // 初始化滚动条标志
    pitem->has_scrollbars = 0;
    
    // 清除滚动条标志位
    pitem->flags &= ~(LAYX_HAS_VSCROLL | LAYX_HAS_HSCROLL);
}

// 设置overflow属性
void layx_set_overflow_x(layx_context *ctx, layx_id item, layx_overflow overflow) {
    LAYX_ASSERT(ctx != NULL && item != LAYX_INVALID_ID);
    layx_item_t *pitem = layx_get_item(ctx, item);
    pitem->overflow_x = overflow;
}

void layx_set_overflow_y(layx_context *ctx, layx_id item, layx_overflow overflow) {
    LAYX_ASSERT(ctx != NULL && item != LAYX_INVALID_ID);
    layx_item_t *pitem = layx_get_item(ctx, item);
    pitem->overflow_y = overflow;
}

void layx_set_overflow(layx_context *ctx, layx_id item, layx_overflow overflow) {
    layx_set_overflow_x(ctx, item, overflow);
    layx_set_overflow_y(ctx, item, overflow);
}

// 获取overflow属性的字符串表示
const char* layx_get_overflow_string(layx_overflow overflow) {
    switch (overflow) {
        case LAYX_OVERFLOW_VISIBLE: return "visible";
        case LAYX_OVERFLOW_HIDDEN: return "hidden";
        case LAYX_OVERFLOW_SCROLL: return "scroll";
        case LAYX_OVERFLOW_AUTO: return "auto";
        default: return "unknown";
    }
}

// 滚动操作函数
void layx_scroll_to(layx_context *ctx, layx_id item, layx_scalar x, layx_scalar y) {
    LAYX_ASSERT(ctx != NULL && item != LAYX_INVALID_ID);
    layx_item_t *pitem = layx_get_item(ctx, item);
    
    pitem->scroll_offset[0] = x;
    pitem->scroll_offset[1] = y;
    
    // 限制滚动范围
    if (pitem->scroll_offset[0] < 0.0f) pitem->scroll_offset[0] = 0.0f;
    if (pitem->scroll_offset[1] < 0.0f) pitem->scroll_offset[1] = 0.0f;
    if (pitem->scroll_offset[0] > pitem->scroll_max[0]) pitem->scroll_offset[0] = pitem->scroll_max[0];
    if (pitem->scroll_offset[1] > pitem->scroll_max[1]) pitem->scroll_offset[1] = pitem->scroll_max[1];
}

void layx_scroll_by(layx_context *ctx, layx_id item, layx_scalar dx, layx_scalar dy) {
    LAYX_ASSERT(ctx != NULL && item != LAYX_INVALID_ID);
    layx_item_t *pitem = layx_get_item(ctx, item);
    
    layx_scroll_to(ctx, item, 
                   pitem->scroll_offset[0] + dx, 
                   pitem->scroll_offset[1] + dy);
}

// 获取可见区域的内容（考虑滚动偏移）
void layx_get_visible_content_rect(layx_context *ctx, layx_id item, 
                                  layx_scalar *visible_left, layx_scalar *visible_top,
                                  layx_scalar *visible_right, layx_scalar *visible_bottom) {
    LAYX_ASSERT(ctx != NULL && item != LAYX_INVALID_ID);
    layx_item_t *pitem = layx_get_item(ctx, item);
    
    layx_scalar client_width = pitem->size[0] - 
                               pitem->padding_trbl[TRBL_LEFT] - pitem->padding_trbl[TRBL_RIGHT] -
                               pitem->border_trbl[TRBL_LEFT] - pitem->border_trbl[TRBL_RIGHT];
    layx_scalar client_height = pitem->size[1] - 
                                pitem->padding_trbl[TRBL_TOP] - pitem->padding_trbl[TRBL_BOTTOM] -
                                pitem->border_trbl[TRBL_TOP] - pitem->border_trbl[TRBL_BOTTOM];
    
    *visible_left = pitem->scroll_offset[0];
    *visible_top = pitem->scroll_offset[1];
    *visible_right = pitem->scroll_offset[0] + client_width;
    *visible_bottom = pitem->scroll_offset[1] + client_height;
}

// 辅助函数实现
int layx_has_vertical_scrollbar(layx_context *ctx, layx_id item) {
    if (ctx == NULL || item == LAYX_INVALID_ID) return 0;
    layx_item_t *pitem = layx_get_item(ctx, item);
    return (pitem->has_scrollbars & 1) != 0;
}

int layx_has_horizontal_scrollbar(layx_context *ctx, layx_id item) {
    if (ctx == NULL || item == LAYX_INVALID_ID) return 0;
    layx_item_t *pitem = layx_get_item(ctx, item);
    return (pitem->has_scrollbars & 2) != 0;
}

// 获取滚动偏移量
void layx_get_scroll_offset(layx_context *ctx, layx_id item, layx_vec2 *offset) {
    LAYX_ASSERT(ctx != NULL && item != LAYX_INVALID_ID);
    layx_item_t *pitem = layx_get_item(ctx, item);
    *offset = pitem->scroll_offset;
}

void layx_get_scroll_offset_xy(layx_context *ctx, layx_id item, layx_scalar *x, layx_scalar *y) {
    LAYX_ASSERT(ctx != NULL && item != LAYX_INVALID_ID);
    layx_item_t *pitem = layx_get_item(ctx, item);
    *x = pitem->scroll_offset[0];
    *y = pitem->scroll_offset[1];
}
// 获取最大滚动范围
void layx_get_scroll_max(layx_context *ctx, layx_id item, layx_vec2 *max) {
    LAYX_ASSERT(ctx != NULL && item != LAYX_INVALID_ID);
    layx_item_t *pitem = layx_get_item(ctx, item);
    *max = pitem->scroll_max;
}

// 获取内容尺寸
void layx_get_content_size(layx_context *ctx, layx_id item, layx_vec2 *size) {
    LAYX_ASSERT(ctx != NULL && item != LAYX_INVALID_ID);
    layx_item_t *pitem = layx_get_item(ctx, item);
    *size = pitem->content_size;
}
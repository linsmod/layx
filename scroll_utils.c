#include "layx.h"
#include <string.h>

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
    
    // 初始化overflow属性为visible
    pitem->overflow_x = LAYX_OVERFLOW_VISIBLE;
    pitem->overflow_y = LAYX_OVERFLOW_VISIBLE;
    
    // 初始化滚动条标志
    pitem->has_scrollbars = 0;
    
    // 清除滚动条标志位
    pitem->flags &= ~(LAYX_HAS_VSCROLL | LAYX_HAS_HSCROLL);
}

// 计算内容实际尺寸（遍历所有子项）
void layx_calculate_content_size(layx_context *ctx, layx_id item) {
    LAYX_ASSERT(ctx != NULL && item != LAYX_INVALID_ID);
    layx_item_t *pitem = layx_get_item(ctx, item);
    
    // 如果没有子项，内容尺寸等于padding+border后的可用空间
    if (pitem->first_child == LAYX_INVALID_ID) {
        layx_scalar client_width = pitem->size[0] - 
                                   pitem->padding[0] - pitem->padding[2] -
                                   pitem->border[0] - pitem->border[2];
        layx_scalar client_height = pitem->size[1] - 
                                    pitem->padding[1] - pitem->padding[3] -
                                    pitem->border[1] - pitem->border[3];
        
        pitem->content_size[0] = client_width;
        pitem->content_size[1] = client_height;
        return;
    }
    
    // 有子项时，计算所有子项的边界框
    layx_scalar max_x = 0.0f;
    layx_scalar max_y = 0.0f;
    layx_id child = pitem->first_child;
    
    while (child != LAYX_INVALID_ID) {
        layx_vec4 child_rect = layx_get_rect(ctx, child);
        layx_scalar child_right = child_rect[0] + child_rect[2];  // x + width
        layx_scalar child_bottom = child_rect[1] + child_rect[3]; // y + height
        
        if (child_right > max_x) max_x = child_right;
        if (child_bottom > max_y) max_y = child_bottom;
        
        child = layx_next_sibling(ctx, child);
    }
    
    // 内容尺寸应该是最大子项边界 + padding（如果需要）
    // 这里我们简单地将内容尺寸设为包含所有子项的最小矩形
    pitem->content_size[0] = max_x;
    pitem->content_size[1] = max_y;
    
    // 确保内容尺寸不小于client area
    layx_scalar client_width = pitem->size[0] - 
                               pitem->padding[0] - pitem->padding[2] -
                               pitem->border[0] - pitem->border[2];
    layx_scalar client_height = pitem->size[1] - 
                                pitem->padding[1] - pitem->padding[3] -
                                pitem->border[1] - pitem->border[3];
    
    if (pitem->content_size[0] < client_width) {
        pitem->content_size[0] = client_width;
    }
    if (pitem->content_size[1] < client_height) {
        pitem->content_size[1] = client_height;
    }
}

// 检测是否需要滚动条（CSS标准行为：不占用布局空间）
void layx_detect_scrollbars(layx_context *ctx, layx_id item) {
    LAYX_ASSERT(ctx != NULL && item != LAYX_INVALID_ID);
    layx_item_t *pitem = layx_get_item(ctx, item);
    
    // 计算client area（不包括滚动条，因为滚动条不占用布局空间）
    layx_scalar client_width = pitem->size[0] - 
                               pitem->padding[0] - pitem->padding[2] -
                               pitem->border[0] - pitem->border[2];
    layx_scalar client_height = pitem->size[1] - 
                                pitem->padding[1] - pitem->padding[3] -
                                pitem->border[1] - pitem->border[3];
    
    // 检测水平和垂直滚动条需求
    int needs_v_scroll = 0;
    int needs_h_scroll = 0;
    
    // 检查垂直滚动条
    switch (pitem->overflow_y) {
        case LAYX_OVERFLOW_SCROLL:
            needs_v_scroll = 1;  // 总是显示
            break;
        case LAYX_OVERFLOW_AUTO:
            needs_v_scroll = (pitem->content_size[1] > client_height);
            break;
        case LAYX_OVERFLOW_HIDDEN:
            needs_v_scroll = 0;
            break;
        case LAYX_OVERFLOW_VISIBLE:
        default:
            needs_v_scroll = 0;
            break;
    }
    
    // 检查水平滚动条（注意：垂直滚动条可能会影响水平空间的可用性）
    switch (pitem->overflow_x) {
        case LAYX_OVERFLOW_SCROLL:
            needs_h_scroll = 1;  // 总是显示
            break;
        case LAYX_OVERFLOW_AUTO:
            needs_h_scroll = (pitem->content_size[0] > client_width);
            break;
        case LAYX_OVERFLOW_HIDDEN:
            needs_h_scroll = 0;
            break;
        case LAYX_OVERFLOW_VISIBLE:
        default:
            needs_h_scroll = 0;
            break;
    }
    
    // 浮动滚动条不参与布局计算，滚动条之间不相互影响
    
    // 更新滚动条标志位
    pitem->has_scrollbars = (needs_v_scroll ? 1 : 0) | (needs_h_scroll ? 2 : 0);
    
    // 更新flags中的滚动条标志
    if (needs_v_scroll) {
        pitem->flags |= LAYX_HAS_VSCROLL;
    } else {
        pitem->flags &= ~LAYX_HAS_VSCROLL;
    }
    
    if (needs_h_scroll) {
        pitem->flags |= LAYX_HAS_HSCROLL;
    } else {
        pitem->flags &= ~LAYX_HAS_HSCROLL;
    }
    
    // 计算滚动范围（最大滚动距离）
    // 注意：滚动条覆盖在内容上，所以client_width/height不变
    pitem->scroll_max[0] = (pitem->content_size[0] > client_width) ? 
                          (pitem->content_size[0] - client_width) : 0.0f;
    pitem->scroll_max[1] = (pitem->content_size[1] > client_height) ? 
                          (pitem->content_size[1] - client_height) : 0.0f;
    
    // 限制当前滚动位置不超过最大值
    if (pitem->scroll_offset[0] > pitem->scroll_max[0]) {
        pitem->scroll_offset[0] = pitem->scroll_max[0];
    }
    if (pitem->scroll_offset[1] > pitem->scroll_max[1]) {
        pitem->scroll_offset[1] = pitem->scroll_max[1];
    }
    
    // 确保滚动位置非负
    if (pitem->scroll_offset[0] < 0.0f) pitem->scroll_offset[0] = 0.0f;
    if (pitem->scroll_offset[1] < 0.0f) pitem->scroll_offset[1] = 0.0f;
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
                               pitem->padding[0] - pitem->padding[2] -
                               pitem->border[0] - pitem->border[2];
    layx_scalar client_height = pitem->size[1] - 
                                pitem->padding[1] - pitem->padding[3] -
                                pitem->border[1] - pitem->border[3];
    
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
layx_vec2 layx_get_scroll_offset(layx_context *ctx, layx_id item) {
    LAYX_ASSERT(ctx != NULL && item != LAYX_INVALID_ID);
    layx_item_t *pitem = layx_get_item(ctx, item);
    return pitem->scroll_offset;
}

// 获取最大滚动范围
layx_vec2 layx_get_scroll_max(layx_context *ctx, layx_id item) {
    LAYX_ASSERT(ctx != NULL && item != LAYX_INVALID_ID);
    layx_item_t *pitem = layx_get_item(ctx, item);
    return pitem->scroll_max;
}

// 获取内容尺寸
layx_vec2 layx_get_content_size(layx_context *ctx, layx_id item) {
    LAYX_ASSERT(ctx != NULL && item != LAYX_INVALID_ID);
    layx_item_t *pitem = layx_get_item(ctx, item);
    return pitem->content_size;
}
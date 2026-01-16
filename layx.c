#define LAYX_IMPLEMENTATION
#include "layx.h"
#include "scroll_utils.h"
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#ifndef LAYX_REALLOC
#define LAYX_REALLOC(_block, _size) realloc(_block, _size)
#define LAYX_FREE(_block) free(_block)
#endif

#ifndef LAYX_MEMSET
#define LAYX_MEMSET(_dst, _val, _size) memset(_dst, _val, _size)
#endif

#if defined(__GNUC__) || defined(__clang__)
#define LAYX_FORCE_INLINE __attribute__((always_inline)) inline
#ifdef __cplusplus
#define LAYX_RESTRICT __restrict
#else
#define LAYX_RESTRICT restrict
#endif
#elif defined(_MSC_VER)
#define LAYX_FORCE_INLINE __forceinline
#define LAYX_RESTRICT __restrict
#else
#define LAYX_FORCE_INLINE inline
#ifdef __cplusplus
#define LAYX_RESTRICT
#else
#define LAYX_RESTRICT restrict
#endif
#endif

// Math utilities
static LAYX_FORCE_INLINE layx_scalar layx_scalar_max(layx_scalar a, layx_scalar b)
{ return a > b ? a : b; }
static LAYX_FORCE_INLINE layx_scalar layx_scalar_min(layx_scalar a, layx_scalar b)
{ return a < b ? a : b; }
static LAYX_FORCE_INLINE float layx_float_max(float a, float b)
{ return a > b ? a : b; }
static LAYX_FORCE_INLINE float layx_float_min(float a, float b)
{ return a < b ? a : b; }

// Helper functions to convert new enums to internal flags
static LAYX_FORCE_INLINE uint32_t layx_display_to_flags(layx_display display) {
    return ((uint32_t)display & 0x3) << 2;  // Bits 2-3 for display type
}

static LAYX_FORCE_INLINE uint32_t layx_flex_direction_to_flags(layx_flex_direction direction) {
    return (uint32_t)direction & LAYX_FLEX_DIRECTION_MASK;
}

static LAYX_FORCE_INLINE uint32_t layx_flex_wrap_to_flags(layx_flex_wrap wrap) {
    return (uint32_t)wrap & LAYX_FLEX_WRAP_MASK;
}

static LAYX_FORCE_INLINE uint32_t layx_justify_content_to_flags(layx_justify_content justify) {
    return (uint32_t)justify;
}

static LAYX_FORCE_INLINE uint32_t layx_align_items_to_flags(layx_align_items align) {
    return (uint32_t)align;
}

static LAYX_FORCE_INLINE uint32_t layx_align_content_to_flags(layx_align_content align) {
    return (uint32_t)align;
}

static LAYX_FORCE_INLINE uint32_t layx_align_self_to_flags(layx_align_self align) {
    return (uint32_t)align;
}

// Helper to get direction dimension
static LAYX_FORCE_INLINE int layx_get_direction_dim(uint32_t flags) {
    layx_flex_direction dir = (layx_flex_direction)(flags & LAYX_FLEX_DIRECTION_MASK);
    return (dir == LAYX_FLEX_DIRECTION_ROW || dir == LAYX_FLEX_DIRECTION_ROW_REVERSE) ? 0 : 1;
}
typedef enum {
    DIM_WIDTH = 0,   // 水平维度（计算宽度）
    DIM_HEIGHT = 1   // 垂直维度（计算高度）
} layout_dim_t;
// Macros to extract TRBL index based on dimension
#define START_SIDE(dim) ((dim) == DIM_WIDTH ? TRBL_LEFT: TRBL_TOP) 
#define END_SIDE(dim) ((dim) == DIM_WIDTH ? TRBL_RIGHT: TRBL_BOTTOM)
#define POINT_DIM(dim) ((dim) == DIM_WIDTH ? XYWH_X : XYWH_Y)
#define SIZE_DIM(dim)    ((dim) == DIM_WIDTH ? XYWH_WIDTH : XYWH_HEIGHT)
// Context management
void layx_init_context(layx_context *ctx)
{
    ctx->capacity = 0;
    ctx->count = 0;
    ctx->items = NULL;
    ctx->rects = NULL;
    ctx->screen_to_local_fn = NULL;
    ctx->free_list_head = LAYX_INVALID_ID;
}

void layx_reserve_items_capacity(layx_context *ctx, layx_id count)
{
    if (count >= ctx->capacity) {
        ctx->capacity = count;
        const size_t item_size = sizeof(layx_item_t) + sizeof(layx_vec4);
        ctx->items = (layx_item_t*)LAYX_REALLOC(ctx->items, ctx->capacity * item_size);
        const layx_item_t *past_last = ctx->items + ctx->capacity;
        ctx->rects = (layx_vec4*)past_last;
    }
}

void layx_dump_tree(layx_context *layout_ctx, layx_id layout_id, int indent){
    layx_scalar l, t, r, b;
	layx_get_margin_trbl(layout_ctx, layout_id, &l, &t, &r, &b);

	layx_scalar x, y, width, height;
	layx_get_rect_xywh(layout_ctx, layout_id, &x, &y, &width, &height);

    layx_scalar padding_l, padding_t, padding_r, padding_b;
	layx_get_padding_trbl(layout_ctx, layout_id, &padding_l, &padding_t, &padding_r, &padding_b);

    layx_item_t *item = layx_get_item(layout_ctx, layout_id);
    const char* overflow_x_str = layx_get_overflow_string((layx_overflow)item->overflow_x);
    const char* overflow_y_str = layx_get_overflow_string((layx_overflow)item->overflow_y);

    LAYX_DEBUG_PRINT("%*s<lay_item_%d: xywh=[%.1f, %.1f, %.1f, %.1f] margin=[%.1f, %.1f, %.1f, %.1f] padding=[%.1f, %.1f, %.1f, %.1f]",
        indent, "", layout_id, x, y, width, height, l, t, r, b, padding_l, padding_t, padding_r, padding_b);
    LAYX_DEBUG_PRINT(" PROP=%s|overflow-x:%s|overflow-y:%s",layx_get_layout_properties_string(layout_ctx, layout_id), overflow_x_str, overflow_y_str);
    bool fixed_width = item->flags & LAYX_SIZE_FIXED_WIDTH;
    bool fixed_height = item->flags & LAYX_SIZE_FIXED_HEIGHT;
    LAYX_DEBUG_PRINT(" initial_w=%.1f initial_h=%.1f fixed_width:%s fixed_height=%s>\n",item->size[0],item->size[1], fixed_width ? "YES" : "NO", fixed_height ? "YES" : "NO");
    layx_id child = item->first_child;
    while (child != LAYX_INVALID_ID) {
        layx_dump_tree(layout_ctx, child, indent + 2);
        layx_item_t *child_item = layx_get_item(layout_ctx, child);
        child = child_item->next_sibling;
    }
}
void layx_destroy_context(layx_context *ctx)
{
    if (ctx->items != NULL) {
        LAYX_FREE(ctx->items);
        ctx->items = NULL;
        ctx->rects = NULL;
    }
}

void layx_reset_context(layx_context *ctx)
{
    ctx->count = 0;
    ctx->free_list_head = LAYX_INVALID_ID;
}

// Layout calculation declarations
static void layx_calc_size(layx_context *ctx, layx_id item, int dim);
static void layx_arrange(layx_context *ctx, layx_id item, int dim);

void layx_run_context(layx_context *ctx)
{
    LAYX_ASSERT(ctx != NULL);
    if (ctx->count > 0) {
        layx_run_item(ctx, 0);
    }
}


extern void layx_init_scroll_fields(layx_context *ctx, layx_id item);

// 计算并更新项目的滚动相关字段
static void layx_update_scroll_fields(layx_context *ctx, layx_id item) {
    layx_item_t *pitem = layx_get_item(ctx, item);
    layx_vec4 rect = ctx->rects[item];
    
    // 1. 初始化滚动字段
    pitem->scroll_offset[0] = 0.0f;
    pitem->scroll_offset[1] = 0.0f;
    pitem->has_scrollbars = 0;
    
    // 2. 计算客户区尺寸（内容区域，不包含 padding 和 border）
    layx_scalar client_width = rect[XYWH_WIDTH] - 
                               pitem->padding_trbl[TRBL_LEFT] - pitem->padding_trbl[TRBL_RIGHT] -
                               pitem->border_trbl[TRBL_LEFT] - pitem->border_trbl[TRBL_RIGHT];
    layx_scalar client_height = rect[XYWH_HEIGHT] - 
                                pitem->padding_trbl[TRBL_TOP] - pitem->padding_trbl[TRBL_BOTTOM] -
                                pitem->border_trbl[TRBL_TOP] - pitem->border_trbl[TRBL_BOTTOM];
    client_width = client_width > 0 ? client_width : 0;
    client_height = client_height > 0 ? client_height : 0;
    
    // 3. 计算内容尺寸（所有子元素的总占用空间）
    layx_scalar content_width = 0;
    layx_scalar content_height = 0;
    
    layx_id child = pitem->first_child;
    while (child != LAYX_INVALID_ID) {
        layx_item_t *pchild = layx_get_item(ctx, child);
        layx_vec4 child_rect = ctx->rects[child];
        
        // 子元素相对于父元素的绝对位置 + 尺寸 + margin
        layx_scalar child_right = child_rect[XYWH_X] + child_rect[XYWH_WIDTH] + pchild->margin_trbl[TRBL_RIGHT];
        layx_scalar child_bottom = child_rect[XYWH_Y] + child_rect[XYWH_HEIGHT] + pchild->margin_trbl[TRBL_BOTTOM];
        
        // 取最大值作为内容尺寸
        if (child_right > content_width) content_width = child_right;
        if (child_bottom > content_height) content_height = child_bottom;
        
        child = pchild->next_sibling;
    }
    
    // 如果没有子元素，内容尺寸等于客户区尺寸
    if (content_width == 0) content_width = client_width;
    if (content_height == 0) content_height = client_height;
    
    pitem->content_size[0] = content_width;
    pitem->content_size[1] = content_height;
    
    // 4. 计算最大滚动值
    // 对于 overflow:visible，scroll_max 始终为 0
    if (pitem->overflow_x == LAYX_OVERFLOW_VISIBLE) {
        pitem->scroll_max[0] = 0.0f;
    } else {
        pitem->scroll_max[0] = content_width - client_width;
        if (pitem->scroll_max[0] < 0.0f) pitem->scroll_max[0] = 0.0f;
    }
    
    if (pitem->overflow_y == LAYX_OVERFLOW_VISIBLE) {
        pitem->scroll_max[1] = 0.0f;
    } else {
        pitem->scroll_max[1] = content_height - client_height;
        if (pitem->scroll_max[1] < 0.0f) pitem->scroll_max[1] = 0.0f;
    }
    
    // 5. 设置滚动条标志
    // 水平滚动条
    int has_h_scroll = 0;
    if (pitem->overflow_x == LAYX_OVERFLOW_SCROLL) {
        has_h_scroll = 1;  // overflow:scroll 始终显示滚动条
    } else if (pitem->overflow_x == LAYX_OVERFLOW_AUTO) {
        has_h_scroll = (pitem->scroll_max[0] > 0.0f) ? 1 : 0;
    }
    
    // 垂直滚动条
    int has_v_scroll = 0;
    if (pitem->overflow_y == LAYX_OVERFLOW_SCROLL) {
        has_v_scroll = 1;  // overflow:scroll 始终显示滚动条
    } else if (pitem->overflow_y == LAYX_OVERFLOW_AUTO) {
        has_v_scroll = (pitem->scroll_max[1] > 0.0f) ? 1 : 0;
    }
    
    pitem->has_scrollbars = (has_v_scroll ? 1 : 0) | ((has_h_scroll ? 1 : 0) << 1);
    
    // 更新 flags
    if (has_v_scroll) {
        pitem->flags |= LAYX_HAS_VSCROLL;
    } else {
        pitem->flags &= ~LAYX_HAS_VSCROLL;
    }
    if (has_h_scroll) {
        pitem->flags |= LAYX_HAS_HSCROLL;
    } else {
        pitem->flags &= ~LAYX_HAS_HSCROLL;
    }
}

void layx_run_item(layx_context *ctx, layx_id item)
{
    LAYX_ASSERT(ctx != NULL);
    
    // 横向计算尺寸和排列
    layx_calc_size(ctx, item,0);
    layx_arrange(ctx, item,0);
    
    // 纵向计算尺寸和排列（会递归计算内容尺寸和滚动条）
    layx_calc_size(ctx, item,1);
    layx_arrange(ctx, item,1);
    
    // 计算滚动相关字段
    layx_update_scroll_fields(ctx, item);
}

void layx_clear_item_break(layx_context *ctx, layx_id item)
{
    LAYX_ASSERT(ctx != NULL);
    layx_item_t *pitem = layx_get_item(ctx, item);
    pitem->flags = pitem->flags & ~LAYX_BREAK;
}

layx_id layx_items_count(layx_context *ctx)
{
    LAYX_ASSERT(ctx != NULL);
    return ctx->count;
}

layx_id layx_items_capacity(layx_context *ctx)
{
    LAYX_ASSERT(ctx != NULL);
    return ctx->capacity;
}

layx_id layx_item(layx_context *ctx)
{
    layx_id idx;
    layx_item_t *item;
    
    // 优先从空闲链表分配
    if (ctx->free_list_head != LAYX_INVALID_ID) {
        idx = ctx->free_list_head;
        item = layx_get_item(ctx, idx);
        ctx->free_list_head = item->next_sibling;  // 从 free_list 中取出
        
        // 初始化 item 数据
        LAYX_MEMSET(item, 0, sizeof(layx_item_t));
        item->parent = LAYX_INVALID_ID;
        item->first_child = LAYX_INVALID_ID;
        item->next_sibling = LAYX_INVALID_ID;
        item->min_size[0] = 0; item->min_size[1] = 0;
        item->max_size[0] = 0; item->max_size[1] = 0;
        item->flex_grow = 0;
        item->flex_shrink = 1;
        item->flex_basis = 0;
        LAYX_MEMSET(&ctx->rects[idx], 0, sizeof(layx_vec4));
    } else {
        // 从数组末尾分配
        idx = ctx->count++;
        if (idx >= ctx->capacity) {
            ctx->capacity = ctx->capacity < 1 ? 32 : (ctx->capacity * 4);
            const size_t item_size = sizeof(layx_item_t) + sizeof(layx_vec4);
            ctx->items = (layx_item_t*)LAYX_REALLOC(ctx->items, ctx->capacity * item_size);
            const layx_item_t *past_last = ctx->items + ctx->capacity;
            ctx->rects = (layx_vec4*)past_last;
        }
        item = layx_get_item(ctx, idx);
        LAYX_MEMSET(item, 0, sizeof(layx_item_t));
        item->parent = LAYX_INVALID_ID;
        item->first_child = LAYX_INVALID_ID;
        item->next_sibling = LAYX_INVALID_ID;
        item->min_size[0] = 0; item->min_size[1] = 0;
        item->max_size[0] = 0; item->max_size[1] = 0;
        item->flex_grow = 0;
        item->flex_shrink = 1;  // CSS规范: flex-shrink默认为1
        item->flex_basis = 0;
        LAYX_MEMSET(&ctx->rects[idx], 0, sizeof(layx_vec4));
    }
    return idx;
}

static LAYX_FORCE_INLINE
void layx_insert_after_by_ptr(
        layx_item_t *LAYX_RESTRICT pearlier,
        layx_id later, layx_item_t *LAYX_RESTRICT plater)
{
    plater->next_sibling = pearlier->next_sibling;
    plater->flags |= LAYX_ITEM_INSERTED;
    pearlier->next_sibling = later;
}

layx_id layx_last_child(const layx_context *ctx, layx_id parent)
{
    layx_item_t *pparent = layx_get_item(ctx, parent);
    layx_id child = pparent->first_child;
    if (child == LAYX_INVALID_ID) return LAYX_INVALID_ID;
    layx_item_t *pchild = layx_get_item(ctx, child);
    layx_id result = child;
    for (;;) {
        layx_id next = pchild->next_sibling;
        if (next == LAYX_INVALID_ID) break;
        result = next;
        pchild = layx_get_item(ctx, next);
    }
    return result;
}

void layx_insert_after(layx_context *ctx, layx_id earlier, layx_id later)
{
    LAYX_ASSERT(later != 0);
    LAYX_ASSERT(earlier != later);
    layx_item_t *LAYX_RESTRICT pearlier = layx_get_item(ctx, earlier);
    layx_item_t *LAYX_RESTRICT plater = layx_get_item(ctx, later);
    plater->parent = pearlier->parent;  // 设置parent，与earlier的parent相同
    layx_insert_after_by_ptr(pearlier, later, plater);
}

int layx_is_inserted(layx_context *ctx, layx_id child){
    LAYX_ASSERT(child != 0);
    layx_item_t *LAYX_RESTRICT pchild = layx_get_item(ctx, child);
    return pchild->flags & LAYX_ITEM_INSERTED;
}

void layx_append(layx_context *ctx, layx_id parent, layx_id child)
{
    LAYX_ASSERT(child != 0);
    LAYX_ASSERT(parent != child);
    layx_item_t *LAYX_RESTRICT pparent = layx_get_item(ctx, parent);
    layx_item_t *LAYX_RESTRICT pchild = layx_get_item(ctx, child);
    LAYX_ASSERT(!(pchild->flags & LAYX_ITEM_INSERTED));
    pchild->parent = parent;  // 设置parent
    if (pparent->first_child == LAYX_INVALID_ID) {
        pparent->first_child = child;
        pchild->flags |= LAYX_ITEM_INSERTED;
    } else {
        layx_id next = pparent->first_child;
        layx_item_t *LAYX_RESTRICT pnext = layx_get_item(ctx, next);
        for (;;) {
            next = pnext->next_sibling;
            if (next == LAYX_INVALID_ID) break;
            pnext = layx_get_item(ctx, next);
        }
        layx_insert_after_by_ptr(pnext, child, pchild);
    }
}

void layx_prepend(layx_context *ctx, layx_id parent, layx_id new_child)
{
    LAYX_ASSERT(new_child != 0);
    LAYX_ASSERT(parent != new_child);
    layx_item_t *LAYX_RESTRICT pparent = layx_get_item(ctx, parent);
    layx_id old_child = pparent->first_child;
    layx_item_t *LAYX_RESTRICT pchild = layx_get_item(ctx, new_child);
    LAYX_ASSERT(!(pchild->flags & LAYX_ITEM_INSERTED));
    pchild->parent = parent;  // 设置parent
    pparent->first_child = new_child;
    pchild->flags |= LAYX_ITEM_INSERTED;
    pchild->next_sibling = old_child;
}

void layx_remove(layx_context *ctx, layx_id item)
{
    LAYX_ASSERT(ctx != NULL);
    LAYX_ASSERT(item != LAYX_INVALID_ID && item < ctx->count);
    
    layx_item_t *pitem = layx_get_item(ctx, item);
    layx_id parent_id = pitem->parent;
    
    // 如果元素未插入到任何父元素中，直接返回
    if (parent_id == LAYX_INVALID_ID) {
        return;
    }
    
    layx_item_t *pparent = layx_get_item(ctx, parent_id);
    
    // 从父元素的子节点链中移除
    if (pparent->first_child == item) {
        // 元素是父元素的第一个子节点
        pparent->first_child = pitem->next_sibling;
    } else {
        // 查找前一个兄弟节点
        layx_id prev_child = pparent->first_child;
        layx_item_t *pprev = NULL;
        
        while (prev_child != LAYX_INVALID_ID) {
            pprev = layx_get_item(ctx, prev_child);
            if (pprev->next_sibling == item) {
                // 找到了前一个兄弟节点，更新其 next_sibling
                pprev->next_sibling = pitem->next_sibling;
                break;
            }
            prev_child = pprev->next_sibling;
        }
    }
    
    // 清除插入标志和重置父元素引用
    pitem->flags &= ~LAYX_ITEM_INSERTED;
    pitem->parent = LAYX_INVALID_ID;
}

void layx_destroy_item(layx_context *ctx, layx_id item)
{
    LAYX_ASSERT(ctx != NULL);
    LAYX_ASSERT(item != LAYX_INVALID_ID && item < ctx->count);
    
    layx_item_t *pitem = layx_get_item(ctx, item);
    
    // 先从父元素中移除（如果有父元素）
    if (pitem->parent != LAYX_INVALID_ID) {
        layx_remove(ctx, item);
    }
    
    // 递归销毁所有子元素
    layx_id child = pitem->first_child;
    while (child != LAYX_INVALID_ID) {
        layx_item_t *pchild = layx_get_item(ctx, child);
        layx_id next_child = pchild->next_sibling;
        layx_destroy_item(ctx, child);
        child = next_child;
    }
    
    // 将 item 加入空闲链表
    pitem->first_child = LAYX_INVALID_ID;
    pitem->next_sibling = ctx->free_list_head;
    pitem->parent = LAYX_INVALID_ID;
    pitem->flags = 0;
    ctx->free_list_head = item;
}

// Display property
void layx_set_display(layx_context *ctx, layx_id item, layx_display display)
{
    layx_item_t *pitem = layx_get_item(ctx, item);
    uint32_t flags = pitem->flags;
    flags &= ~LAYX_DISPLAY_TYPE_MASK;
    flags |= layx_display_to_flags(display);
    pitem->flags = flags;
}

const char* layx_get_display_string(layx_display display) {
    switch (display) {
        case LAYX_DISPLAY_BLOCK: return "BLOCK";
        case LAYX_DISPLAY_FLEX: return "FLEX";
        case LAYX_DISPLAY_INLINE: return "INLINE";
        case LAYX_DISPLAY_INLINE_BLOCK: return "INLINE_BLOCK";
        default: return "UNKNOWN";
    }
}

// Flex properties
void layx_set_flex_direction(layx_context *ctx, layx_id item, layx_flex_direction direction)
{
    layx_item_t *pitem = layx_get_item(ctx, item);
    uint32_t flags = pitem->flags;
    flags &= ~LAYX_FLEX_DIRECTION_MASK;
    flags |= layx_flex_direction_to_flags(direction);
    pitem->flags = flags;
}

void layx_set_flex_wrap(layx_context *ctx, layx_id item, layx_flex_wrap wrap)
{
    layx_item_t *pitem = layx_get_item(ctx, item);
    uint32_t flags = pitem->flags;
    flags &= ~LAYX_FLEX_WRAP_MASK;
    flags |= layx_flex_wrap_to_flags(wrap);
    pitem->flags = flags;
}

void layx_set_justify_content(layx_context *ctx, layx_id item, layx_justify_content justify)
{
    layx_item_t *pitem = layx_get_item(ctx, item);
    uint32_t flags = pitem->flags;
    flags &= ~LAYX_JUSTIFY_CONTENT_MASK;
    flags |= layx_justify_content_to_flags(justify);
    pitem->flags = flags;
}

void layx_set_align_items(layx_context *ctx, layx_id item, layx_align_items align)
{
    layx_item_t *pitem = layx_get_item(ctx, item);
    uint32_t flags = pitem->flags;
    flags &= ~LAYX_ALIGN_ITEMS_MASK;
    flags |= layx_align_items_to_flags(align);
    pitem->flags = flags;
}

void layx_set_align_content(layx_context *ctx, layx_id item, layx_align_content align)
{
    layx_item_t *pitem = layx_get_item(ctx, item);
    uint32_t flags = pitem->flags;
    flags &= ~LAYX_ALIGN_CONTENT_MASK;
    flags |= layx_align_content_to_flags(align);
    pitem->flags = flags;
}

void layx_set_flex(layx_context *ctx, layx_id item,
                    layx_flex_direction direction,
                    layx_flex_wrap wrap,
                    layx_justify_content justify,
                    layx_align_items align_items,
                    layx_align_content align_content)
{
    layx_set_display(ctx, item, LAYX_DISPLAY_FLEX);
    layx_set_flex_direction(ctx, item, direction);
    layx_set_flex_wrap(ctx, item, wrap);
    layx_set_justify_content(ctx, item, justify);
    layx_set_align_items(ctx, item, align_items);
    layx_set_align_content(ctx, item, align_content);
}

// String getters
const char* layx_get_flex_direction_string(layx_flex_direction direction) {
    switch (direction) {
        case LAYX_FLEX_DIRECTION_ROW: return "ROW";
        case LAYX_FLEX_DIRECTION_COLUMN: return "COLUMN";
        case LAYX_FLEX_DIRECTION_ROW_REVERSE: return "ROW_REVERSE";
        case LAYX_FLEX_DIRECTION_COLUMN_REVERSE: return "COLUMN_REVERSE";
        default: return "UNKNOWN";
    }
}

const char* layx_get_flex_wrap_string(layx_flex_wrap wrap) {
    switch (wrap) {
        case LAYX_FLEX_WRAP_NOWRAP: return "NOWRAP";
        case LAYX_FLEX_WRAP_WRAP: return "WRAP";
        case LAYX_FLEX_WRAP_WRAP_REVERSE: return "WRAP_REVERSE";
        default: return "UNKNOWN";
    }
}

const char* layx_get_justify_content_string(layx_justify_content justify) {
    switch (justify) {
        case LAYX_JUSTIFY_FLEX_START: return "FLEX_START";
        case LAYX_JUSTIFY_CENTER: return "CENTER";
        case LAYX_JUSTIFY_FLEX_END: return "FLEX_END";
        case LAYX_JUSTIFY_SPACE_BETWEEN: return "SPACE_BETWEEN";
        case LAYX_JUSTIFY_SPACE_AROUND: return "SPACE_AROUND";
        case LAYX_JUSTIFY_SPACE_EVENLY: return "SPACE_EVENLY";
        default: return "UNKNOWN";
    }
}

const char* layx_get_align_items_string(layx_align_items align) {
    switch (align) {
        case LAYX_ALIGN_ITEMS_STRETCH: return "STRETCH";
        case LAYX_ALIGN_ITEMS_FLEX_START: return "FLEX_START";
        case LAYX_ALIGN_ITEMS_CENTER: return "CENTER";
        case LAYX_ALIGN_ITEMS_FLEX_END: return "FLEX_END";
        case LAYX_ALIGN_ITEMS_BASELINE: return "BASELINE";
        default: return "UNKNOWN";
    }
}

const char* layx_get_align_content_string(layx_align_content align) {
    switch (align) {
        case LAYX_ALIGN_CONTENT_STRETCH: return "STRETCH";
        case LAYX_ALIGN_CONTENT_FLEX_START: return "FLEX_START";
        case LAYX_ALIGN_CONTENT_CENTER: return "CENTER";
        case LAYX_ALIGN_CONTENT_FLEX_END: return "FLEX_END";
        case LAYX_ALIGN_CONTENT_SPACE_BETWEEN: return "SPACE_BETWEEN";
        case LAYX_ALIGN_CONTENT_SPACE_AROUND: return "SPACE_AROUND";
        default: return "UNKNOWN";
    }
}

// Size properties
void layx_set_width(layx_context *ctx, layx_id item, layx_scalar width)
{
    layx_item_t *pitem = layx_get_item(ctx, item);
    pitem->size[0] = width;
    uint32_t flags = pitem->flags;
    if (width == 0)
        flags &= ~LAYX_SIZE_FIXED_WIDTH;
    else
        flags |= LAYX_SIZE_FIXED_WIDTH;
    pitem->flags = flags;
}

void layx_set_height(layx_context *ctx, layx_id item, layx_scalar height)
{
    layx_item_t *pitem = layx_get_item(ctx, item);
    pitem->size[1] = height;
    uint32_t flags = pitem->flags;
    if (height == 0)
        flags &= ~LAYX_SIZE_FIXED_HEIGHT;
    else
        flags |= LAYX_SIZE_FIXED_HEIGHT;
    pitem->flags = flags;
}

void layx_set_min_width(layx_context *ctx, layx_id item, layx_scalar min_width)
{
    layx_item_t *pitem = layx_get_item(ctx, item);
    pitem->min_size[0] = min_width;
}

void layx_set_min_height(layx_context *ctx, layx_id item, layx_scalar min_height)
{
    layx_item_t *pitem = layx_get_item(ctx, item);
    pitem->min_size[1] = min_height;
}

void layx_set_min_size(layx_context *ctx, layx_id item, layx_scalar min_width, layx_scalar min_height)
{
    layx_set_min_width(ctx, item, min_width);
    layx_set_min_height(ctx, item, min_height);
}

void layx_set_max_width(layx_context *ctx, layx_id item, layx_scalar max_width)
{
    layx_item_t *pitem = layx_get_item(ctx, item);
    pitem->max_size[0] = max_width;
}

void layx_set_max_height(layx_context *ctx, layx_id item, layx_scalar max_height)
{
    layx_item_t *pitem = layx_get_item(ctx, item);
    pitem->max_size[1] = max_height;
}

void layx_set_position(layx_context *ctx, layx_id item, layx_scalar left, layx_scalar top, layx_scalar right, layx_scalar bottom)
{
    layx_item_t *pitem = layx_get_item(ctx, item);
    pitem->position[0] = left;
    pitem->position[1] = top;
    pitem->position[2] = right;
    pitem->position[3] = bottom;
}
void layx_set_position_lt(layx_context *ctx, layx_id item, layx_scalar left, layx_scalar top){
    layx_item_t *pitem = layx_get_item(ctx, item);
    pitem->position[0] = left;
    pitem->position[1] = top;
}

void layx_set_position_rb(layx_context *ctx, layx_id item, layx_scalar right, layx_scalar bottom){
    layx_item_t *pitem = layx_get_item(ctx, item);
    pitem->position[2] = right;
    pitem->position[3] = bottom;
}

void layx_get_position_ltrb(layx_context *ctx, layx_id item, layx_scalar *left, layx_scalar *top, layx_scalar *right, layx_scalar *bottom)
{
    layx_item_t *pitem = layx_get_item(ctx, item);
    *left = pitem->position[0];
    *top = pitem->position[1];
    *right = pitem->position[2];
    *bottom = pitem->position[3];
}

void layx_set_size(layx_context *ctx, layx_id item, layx_scalar width, layx_scalar height)
{
    layx_set_width(ctx, item, width);
    layx_set_height(ctx, item, height);
}

layx_vec2 layx_get_size(layx_context *ctx, layx_id item)
{
    layx_item_t *pitem = layx_get_item(ctx, item);
    return pitem->size;
}

// Flex item properties
void layx_set_flex_grow(layx_context *ctx, layx_id item, layx_scalar grow)
{
    layx_item_t *pitem = layx_get_item(ctx, item);
    pitem->flex_grow = grow;
}

void layx_set_flex_shrink(layx_context *ctx, layx_id item, layx_scalar shrink)
{
    layx_item_t *pitem = layx_get_item(ctx, item);
    pitem->flex_shrink = shrink;
}

void layx_set_flex_basis(layx_context *ctx, layx_id item, layx_scalar basis)
{
    layx_item_t *pitem = layx_get_item(ctx, item);
    pitem->flex_basis = basis;
}

void layx_set_flex_properties(layx_context *ctx, layx_id item,
                                 layx_scalar grow, layx_scalar shrink, layx_scalar basis)
{
    layx_set_flex_grow(ctx, item, grow);
    layx_set_flex_shrink(ctx, item, shrink);
    layx_set_flex_basis(ctx, item, basis);
}

// Align self
void layx_set_align_self(layx_context *ctx, layx_id item, layx_align_self align)
{
    layx_item_t *pitem = layx_get_item(ctx, item);
    uint32_t flags = pitem->flags;
    flags &= ~LAYX_ALIGN_SELF_MASK;
    flags |= layx_align_self_to_flags(align);
    pitem->flags = flags;
}

/* 生成单个方向的设置函数 */
#define LAYX_GEN_SIDE_SETTER(field, side, side_idx) \
void layx_set_##field##_##side(layx_context *ctx, layx_id item, layx_scalar value) \
{ \
    layx_item_t *pitem = layx_get_item(ctx, item); \
    pitem->field##_trbl[TRBL_##side_idx] = value; \
}

/* 生成完整的四个方向设置函数 */
#define LAYX_GEN_ALL_SIDES(field) \
    LAYX_GEN_SIDE_SETTER(field, top, TOP) \
    LAYX_GEN_SIDE_SETTER(field, right, RIGHT) \
    LAYX_GEN_SIDE_SETTER(field, bottom, BOTTOM) \
    LAYX_GEN_SIDE_SETTER(field, left, LEFT) \
    void layx_set_##field(layx_context *ctx, layx_id item, layx_scalar value) \
    { \
        layx_item_t *pitem = layx_get_item(ctx, item); \
        pitem->field##_trbl[TRBL_LEFT]   = value; \
        pitem->field##_trbl[TRBL_TOP]    = value; \
        pitem->field##_trbl[TRBL_RIGHT]  = value; \
        pitem->field##_trbl[TRBL_BOTTOM] = value; \
    } \
    void layx_set_##field##_trbl(layx_context *ctx, layx_id item, \
                               layx_scalar top, \
                               layx_scalar right, layx_scalar bottom,layx_scalar left) \
    { \
        layx_item_t *pitem = layx_get_item(ctx, item); \
        pitem->field##_trbl[TRBL_LEFT]   = left; \
        pitem->field##_trbl[TRBL_TOP]    = top; \
        pitem->field##_trbl[TRBL_RIGHT]  = right; \
        pitem->field##_trbl[TRBL_BOTTOM] = bottom; \
    }

/* 生成三组函数 */
LAYX_GEN_ALL_SIDES(margin)
LAYX_GEN_ALL_SIDES(border)
LAYX_GEN_ALL_SIDES(padding)


// Getters for box model
void layx_get_margin_trbl(layx_context *ctx, layx_id item, layx_scalar *top, layx_scalar *right, layx_scalar *bottom,layx_scalar *left)
{
    layx_item_t *pitem = layx_get_item(ctx, item);
    layx_vec4 margins = pitem->margin_trbl;
    *top = GET_TOP(margins);
    *right = GET_RIGHT(margins);
    *bottom = GET_BOTTOM(margins);
     *left = GET_LEFT(margins);
}

void layx_get_padding_trbl(layx_context *ctx, layx_id item,  layx_scalar *top, layx_scalar *right, layx_scalar *bottom,layx_scalar *left)
{
    layx_item_t *pitem = layx_get_item(ctx, item);
    layx_vec4 padding = pitem->padding_trbl;
    *top = GET_TOP(padding);
    *right = GET_RIGHT(padding);
    *bottom = GET_BOTTOM(padding);
    *left = GET_LEFT(padding);
}

void layx_get_border_trbl(layx_context *ctx, layx_id item, layx_scalar *top, layx_scalar *right, layx_scalar *bottom,layx_scalar *left)
{
    layx_item_t *pitem = layx_get_item(ctx, item);
    layx_vec4 border = pitem->border_trbl;
    *top = GET_TOP(border);
    *right = GET_RIGHT(border);
    *bottom = GET_BOTTOM(border);
    *left = GET_LEFT(border);
}

// Style application
void layx_style_reset(layx_style *style)
{
    LAYX_MEMSET(style, 0, sizeof(layx_style));
}

void layx_apply_style(layx_context *ctx, layx_id item, const layx_style *style)
{
    // Display
    if (style->display != 0) {
        layx_set_display(ctx, item, style->display);
    }
    
    // Flex properties
    if (style->flex_direction != 0) {
        layx_set_flex_direction(ctx, item, style->flex_direction);
    }
    if (style->flex_wrap != 0) {
        layx_set_flex_wrap(ctx, item, style->flex_wrap);
    }
    if (style->justify_content != 0) {
        layx_set_justify_content(ctx, item, style->justify_content);
    }
    if (style->align_items != 0) {
        layx_set_align_items(ctx, item, style->align_items);
    }
    if (style->align_content != 0) {
        layx_set_align_content(ctx, item, style->align_content);
    }
    
    // Size properties
    if (style->width != 0 || style->height != 0) {
        layx_set_size(ctx, item, style->width, style->height);
    }
    if (style->min_width != 0) {
        layx_set_min_width(ctx, item, style->min_width);
    }
    if (style->min_height != 0) {
        layx_set_min_height(ctx, item, style->min_height);
    }
    if (style->max_width != 0) {
        layx_set_max_width(ctx, item, style->max_width);
    }
    if (style->max_height != 0) {
        layx_set_max_height(ctx, item, style->max_height);
    }
    
    // Box model
    if (style->margin_top != 0 || style->margin_right != 0 || 
        style->margin_bottom != 0 || style->margin_left != 0) {
        layx_set_margin_trbl(ctx, item,
                             style->margin_top,
                             style->margin_right, style->margin_bottom,style->margin_left);
    }
    if (style->padding_top != 0 || style->padding_right != 0 || 
        style->padding_bottom != 0 || style->padding_left != 0) {
        layx_set_padding_trbl(ctx, item,style->padding_top,
                             style->padding_right, style->padding_bottom,
                             style->padding_left);
    }
    if (style->border_top != 0 || style->border_right != 0 || 
        style->border_bottom != 0 || style->border_left != 0) {
        layx_set_border_trbl(ctx, item,
                              style->border_top,
                             style->border_right, style->border_bottom,style->border_left);
    }
    
    // Flex item properties
    if (style->flex_grow != 0 || style->flex_shrink != 0 || style->flex_basis != 0) {
        layx_set_flex_properties(ctx, item, style->flex_grow, style->flex_shrink, style->flex_basis);
    }
    
    // Align self
    if (style->align_self != 0) {
        layx_set_align_self(ctx, item, style->align_self);
    }
}

layx_id layx_create_item_with_style(layx_context *ctx, const layx_style *style)
{
    layx_id item = layx_item(ctx);
    layx_apply_style(ctx, item, style);
    return item;
}

// Helper function to get internal space available for children
static LAYX_FORCE_INLINE
layx_scalar layx_get_internal_space(
        layx_context *ctx, layx_id item, int dim)
{
    layx_item_t *pitem = layx_get_item(ctx, item);
    layx_vec4 rect = ctx->rects[item];
    return rect[SIZE_DIM(dim)] - pitem->padding_trbl[START_SIDE(dim)] - pitem->border_trbl[START_SIDE(dim)] 
                           - pitem->padding_trbl[END_SIDE(dim)] - pitem->border_trbl[END_SIDE(dim)];
}

// Helper function to get offset where children should be positioned
// 当布局子元素时，用于获取父元素内容区域(content-box)的起始位置，用于作为子元素布局的基准点。
static LAYX_FORCE_INLINE
layx_scalar layx_get_content_offset(
        layx_context *ctx, layx_id item, int dim)
{
    layx_item_t *pitem = layx_get_item(ctx, item);
    layx_vec4 rect = ctx->rects[item]; // margin-boxing

    // dim 0 or 1: left or top
    return rect[POINT_DIM(dim)] + pitem->padding_trbl[START_SIDE(dim)] + pitem->border_trbl[START_SIDE(dim)];
}

// Helper to calculate overlayed size
static LAYX_FORCE_INLINE
layx_scalar layx_calc_overlayed_size(
        layx_context *ctx, layx_id item, int dim)
{
    layx_item_t *LAYX_RESTRICT pitem = layx_get_item(ctx, item);
    layx_scalar need_size = 0;
    layx_id child = pitem->first_child;
    while (child != LAYX_INVALID_ID) {
        layx_item_t *pchild = layx_get_item(ctx, child);
        if(IS_AUTO_SIZE(pchild, dim)) {
            // TODO
        }
        layx_vec4 rect = ctx->rects[child];
        // 只使用子元素的尺寸，不使用位置（位置在 arrange 阶段设置）
        layx_scalar child_size = rect[SIZE_DIM(dim)] + pchild->margin_trbl[START_SIDE(dim)] + pchild->margin_trbl[END_SIDE(dim)];
        need_size = layx_scalar_max(need_size, child_size);
        child = pchild->next_sibling;
    }
    return need_size;
}

// Helper to calculate stacked size
static LAYX_FORCE_INLINE
layx_scalar layx_calc_stacked_size(
        layx_context *ctx, layx_id item, int dim)
{
    layx_item_t *pitem = layx_get_item(ctx, item);
    layx_scalar need_size = 0;
    layx_id child = pitem->first_child;
    layx_id prev_child = LAYX_INVALID_ID;
    
    const uint32_t item_flags = pitem->flags;
    bool is_flex_container = layx_is_flex_container(item_flags);
    bool is_vertical = (dim == 1);
    
    while (child != LAYX_INVALID_ID) {
        layx_item_t *pchild = layx_get_item(ctx, child);
        layx_vec4 rect = ctx->rects[child];
        const layx_vec4 margins = pchild->margin_trbl;
        
        // 检查是否为inline元素
        bool is_inline = (pchild->flags & LAYX_DISPLAY_INLINE) != 0;
        
        if (is_inline) {
            // Inline元素特殊处理
            if (dim == 0) {
                // 水平方向：累加宽度和水平margin
                need_size += rect[SIZE_DIM(dim)] + margins[START_SIDE(dim)] + margins[END_SIDE(dim)];
            } else {
                // 垂直方向：inline元素不影响容器高度（由line-height决定）
                // 这里简单返回0或最小行高
                if (prev_child == LAYX_INVALID_ID) {
                    need_size = layx_scalar_max(need_size, rect[SIZE_DIM(dim)]);
                }
            }
        } else {
            // 块级元素处理（原逻辑）
            layx_scalar margin_start = margins[START_SIDE(dim)];
            layx_scalar margin_end = margins[END_SIDE(dim)];
            
            if (prev_child == LAYX_INVALID_ID) {
                need_size += margin_start;
            } else {
                layx_item_t *pprev = layx_get_item(ctx, prev_child);
                layx_scalar prev_margin_end = pprev->margin_trbl[END_SIDE(dim)];
                
                if (is_flex_container) {
                    need_size += prev_margin_end + margin_start;
                } else {
                    if (is_vertical) {
                        need_size += layx_scalar_max(prev_margin_end, margin_start);
                    } else {
                        need_size += prev_margin_end + margin_start;
                    }
                }
            }
            
            need_size += rect[SIZE_DIM(dim)];
            
            prev_child = child;
        }
        
        child = pchild->next_sibling;
    }
    
    // 最后一个元素的结束margin
    if (prev_child != LAYX_INVALID_ID) {
        layx_item_t *plast = layx_get_item(ctx, prev_child);
        bool is_inline = (plast->flags & LAYX_DISPLAY_INLINE) != 0;
        
        if (!is_inline || dim == 0) {  // inline元素只有水平margin
            need_size += plast->margin_trbl[END_SIDE(dim)];
        }
    }
    
    return need_size;
}

// Helper to calculate wrapped overlayed size
static LAYX_FORCE_INLINE
layx_scalar layx_calc_wrapped_overlayed_size(
        layx_context *ctx, layx_id item, int dim)
{
    layx_item_t *LAYX_RESTRICT pitem = layx_get_item(ctx, item);
    layx_scalar need_size = 0;
    layx_scalar need_size2 = 0;
    layx_id child = pitem->first_child;
    while (child != LAYX_INVALID_ID) {
        layx_item_t *pchild = layx_get_item(ctx, child);
        layx_vec4 rect = ctx->rects[child];
        if (pchild->flags & LAYX_BREAK) {
            need_size2 += need_size;
            need_size = 0;
        }
        // 只使用子元素的尺寸，不使用位置（位置在 arrange 阶段设置）
        layx_scalar child_size = rect[SIZE_DIM(dim)] + pchild->margin_trbl[START_SIDE(dim)] + pchild->margin_trbl[END_SIDE(dim)];
        need_size = layx_scalar_max(need_size, child_size);
        child = pchild->next_sibling;
    }
    return need_size2 + need_size;
}

// Helper to calculate wrapped stacked size
static LAYX_FORCE_INLINE
layx_scalar layx_calc_wrapped_stacked_size(
        layx_context *ctx, layx_id item, int dim)
{
    layx_item_t *LAYX_RESTRICT pitem = layx_get_item(ctx, item);
    layx_scalar need_size = 0;
    layx_scalar need_size2 = 0;
    layx_id child = pitem->first_child;
    layx_id prev_child = LAYX_INVALID_ID;  // 用于记录上一个子项，正确处理margin合并

    const uint32_t item_flags = pitem->flags;
    int is_flex_container = layx_is_flex_container(item_flags);

    while (child != LAYX_INVALID_ID) {
        layx_item_t *pchild = layx_get_item(ctx, child);
        layx_vec4 rect = ctx->rects[child];

        if (pchild->flags & LAYX_BREAK) {
            need_size2 = layx_scalar_max(need_size2, need_size);
            need_size = 0;
            prev_child = LAYX_INVALID_ID;  // 换行后重置prev_child
        }

        // 正确处理margin合并：
        // 对于 flex 容器：margin 不合并，直接相加
        // 对于 block 容器：margin 合并，取最大值
        if (prev_child == LAYX_INVALID_ID) {
            // 第一个子项（或换行后的第一个）：累加其起始margin
            need_size += pchild->margin_trbl[START_SIDE(dim)];
        } else {
            // 非第一个子项：累加与上一个子项之间的边距
            layx_item_t *pprev = layx_get_item(ctx, prev_child);
            layx_scalar gap;
            if (is_flex_container) {
                // Flex 容器：margin 不合并，相邻 margin 相加
                gap = pprev->margin_trbl[END_SIDE(dim)] + pchild->margin_trbl[START_SIDE(dim)];
            } else {
                // Block 容器：margin 合并，取最大值
                gap = layx_scalar_max(pprev->margin_trbl[END_SIDE(dim)], pchild->margin_trbl[START_SIDE(dim)]);
            }
            need_size += gap;
        }

        // 累加子项尺寸
        need_size += rect[SIZE_DIM(dim)];

        // 如果是最后一个子项或换行前的最后一个子项，累加其结束margin
        if (pchild->next_sibling == LAYX_INVALID_ID || 
            (layx_get_item(ctx, pchild->next_sibling)->flags & LAYX_BREAK)) {
            need_size += pchild->margin_trbl[END_SIDE(dim)];
        }
        
        prev_child = child;
        child = pchild->next_sibling;
    }
    return layx_scalar_max(need_size2, need_size);
}

// PHASE 1: Calculate size (first pass)
static void layx_calc_size(layx_context *ctx, layx_id item, int dim)
{
    layx_item_t *pitem = layx_get_item(ctx, item);
    uint32_t flags = pitem->flags;

    layx_id child = pitem->first_child;
    while (child != LAYX_INVALID_ID) {
        layx_calc_size(ctx, child, dim);
        child = layx_next_sibling(ctx, child);
    }

    ctx->rects[item][SIZE_DIM(dim)] = pitem->margin_trbl[START_SIDE(dim)];

        layx_scalar cal_size;
        layx_flex_direction direction = (layx_flex_direction)(flags & LAYX_FLEX_DIRECTION_MASK);
        layx_display display = layx_get_display_from_flags(flags);
        layx_flex_wrap wrap = (layx_flex_wrap)(flags & LAYX_FLEX_WRAP_MASK);
        
        if (display == LAYX_DISPLAY_FLEX) {
            bool is_wrapped = (wrap != LAYX_FLEX_WRAP_NOWRAP);
            bool is_row_direction = (direction == LAYX_FLEX_DIRECTION_ROW || direction == LAYX_FLEX_DIRECTION_ROW_REVERSE);
            
            if (is_wrapped) {
                if (is_row_direction) {
                    if (dim == 0)
                        cal_size = layx_calc_wrapped_stacked_size(ctx, item, 0);
                    else
                        cal_size = layx_calc_wrapped_overlayed_size(ctx, item, 1);
                } else {
                    if (dim == 1)
                        cal_size = layx_calc_wrapped_stacked_size(ctx, item,1);
                    else
                        cal_size = layx_calc_wrapped_overlayed_size(ctx, item, 0);
                }
            } else {
                if ((is_row_direction && dim == 0) || (!is_row_direction && dim == 1)) {
                    cal_size = layx_calc_stacked_size(ctx, item, dim);
                } else {
                    cal_size = layx_calc_overlayed_size(ctx, item, dim);
                }
            }
        } else if (display == LAYX_DISPLAY_BLOCK || display == LAYX_DISPLAY_INLINE_BLOCK) {
            // DISPLAY_BLOCK 和 DISPLAY_INLINE_BLOCK: 子元素在水平方向上叠加，在垂直方向上堆叠
            if (dim == 1) {
                // Y 轴（垂直方向）：使用 stacked（堆叠）
                cal_size = layx_calc_stacked_size(ctx, item, dim);
            } else {
                // X 轴（水平方向）：使用 overlay（叠加）
                cal_size = layx_calc_overlayed_size(ctx, item, dim);
            }
        } else if (display == LAYX_DISPLAY_INLINE) {
            // DISPLAY_INLINE: 宽度和高度都使用 overlayed size（取最大值）
            // inline元素的内容在同一行上，尺寸由最大的子元素决定
            cal_size = layx_calc_overlayed_size(ctx, item, dim);
        } else {
            // 默认情况，使用 block 布局
            if (dim == 1) {
                cal_size = layx_calc_stacked_size(ctx, item, dim);
            } else {
                cal_size = layx_calc_overlayed_size(ctx, item, dim);
            }
        }

        
    

    layx_scalar result_size;
    bool is_fixedsize = pitem->flags & (dim==0?LAYX_SIZE_FIXED_WIDTH:LAYX_SIZE_FIXED_HEIGHT);
    if(is_fixedsize && pitem->size[dim] > 0) {
        result_size = pitem->size[dim];
    }else {
        result_size = cal_size;
    }

    // Apply min/max size constraints
    if (dim == 0) { // width
        if (pitem->min_size[0] > 0 && result_size < pitem->min_size[0]) {
            result_size = pitem->min_size[0];
        }
        if (pitem->max_size[0] > 0 && result_size > pitem->max_size[0]) {
            result_size = pitem->max_size[0];
        }
    } else { // height
        if (pitem->min_size[1] > 0 && result_size < pitem->min_size[1]) {
            result_size = pitem->min_size[1];
        }
        if (pitem->max_size[1] > 0 && result_size > pitem->max_size[1]) {
            result_size = pitem->max_size[1];
        }
    }
    result_size += pitem->padding_trbl[START_SIDE(dim)] + pitem->border_trbl[START_SIDE(dim)] 
                 + pitem->padding_trbl[END_SIDE(dim)] + pitem->border_trbl[END_SIDE(dim)];

    ctx->rects[item][SIZE_DIM(dim)] = result_size;
    // DEBUG: 打印尺寸设置信息
    layx_id first_child = pitem->first_child;
    LAYX_DEBUG_PRINT("DEBUG: layx_calc_size(item=%d, dim=%d, has_child=%d, size[%.1f,%.1f]) -> rect[%d]=%.1f\n",
           item, dim, first_child != LAYX_INVALID_ID, pitem->size[0], pitem->size[1], SIZE_DIM(dim), result_size);
}

// Helper to arrange a single child in a flex container (with justify-content support)
static LAYX_FORCE_INLINE
void layx_arrange_flex_container_single_child(
            layx_context *ctx, layx_id item, int dim)
{
    layx_item_t *pitem = layx_get_item(ctx, item);
    const uint32_t item_flags = pitem->flags;
    layx_justify_content justify = (layx_justify_content)(item_flags & LAYX_JUSTIFY_CONTENT_MASK);
    layx_scalar space = layx_get_internal_space(ctx, item, dim);
    layx_scalar content_offset = layx_get_content_offset(ctx, item, dim);

    layx_id child = pitem->first_child;
    if (child != LAYX_INVALID_ID) {
        layx_item_t *pchild = layx_get_item(ctx, child);
        const layx_vec4 child_margins = pchild->margin_trbl;
        layx_vec4 child_rect = ctx->rects[child];
        
        // 获取子元素尺寸
        float child_width;
        uint32_t size_fixed_flag = (dim == 0) ? LAYX_SIZE_FIXED_WIDTH : LAYX_SIZE_FIXED_HEIGHT;
        if (pchild->flags & size_fixed_flag && pchild->size[dim] > 0) {
            child_width = (float)pchild->size[dim];
        } else {
            child_width = (float)child_rect[SIZE_DIM(dim)];
        }
        
        // 根据 justify-content 计算起点
        float start_pos = 0.0f;
        
        switch (justify) {
        case LAYX_JUSTIFY_FLEX_START:
            start_pos = 0.0f;
            break;
            
        case LAYX_JUSTIFY_FLEX_END: {
            float total_child_space = child_width + child_margins[START_SIDE(dim)] + child_margins[END_SIDE(dim)];
            start_pos = space - total_child_space;
            break;
        }
            
        case LAYX_JUSTIFY_CENTER: {
            // 使用直观的居中公式
            float total_child_space = child_width + child_margins[START_SIDE(dim)] + child_margins[END_SIDE(dim)];
            start_pos = (space - total_child_space) / 2.0f;
            break;
        }
            
        case LAYX_JUSTIFY_SPACE_AROUND:
        case LAYX_JUSTIFY_SPACE_BETWEEN:
        case LAYX_JUSTIFY_SPACE_EVENLY:
            // 单个子元素时，居中对齐
            float total_child_space = child_width + child_margins[START_SIDE(dim)] + child_margins[END_SIDE(dim)];
            start_pos = (space - total_child_space) / 2.0f;
            break;
        }
        
        // 计算最终位置
        float ix0 = (float)content_offset + start_pos + child_margins[START_SIDE(dim)];
        
        child_rect[POINT_DIM(dim)] = (layx_scalar)ix0;
        child_rect[SIZE_DIM(dim)] = (layx_scalar)child_width;
        ctx->rects[child] = child_rect;
    }
}

// Helper to arrange multiple children in a flex container (with flex-wrap and justify-content)
static LAYX_FORCE_INLINE
void layx_arrange_flex_container_multiple_children(
            layx_context *ctx, layx_id item, int dim, bool wrap)
{
    layx_item_t *pitem = layx_get_item(ctx, item);
    const uint32_t item_flags = pitem->flags;
    layx_scalar space = layx_get_internal_space(ctx, item, dim);
    layx_scalar content_offset = layx_get_content_offset(ctx, item, dim);
    float max_x2 = (float)(content_offset + space);

    layx_id start_child = pitem->first_child;
    while (start_child != LAYX_INVALID_ID) {
        layx_scalar used = 0;
        uint32_t count = 0;
        uint32_t total = 0;
        bool hardbreak = false;
        float total_shrink_factor = 0.0f;

        layx_id child = start_child;
        layx_id end_child = LAYX_INVALID_ID;
        while (child != LAYX_INVALID_ID) {
            layx_item_t *pchild = layx_get_item(ctx, child);
            const uint32_t child_flags = pchild->flags;
            const layx_vec4 child_margins = pchild->margin_trbl;
            layx_vec4 child_rect = ctx->rects[child];

            int has_flex_grow = (pchild->flex_grow > 0);

            layx_scalar extend = used;
            if (has_flex_grow) {
                ++count;
                extend += child_rect[SIZE_DIM(dim)];
            } else {
                extend += child_rect[SIZE_DIM(dim)];
                if (pchild->flex_shrink > 0.0f) {
                    total_shrink_factor += pchild->flex_shrink;
                }
            }

            if (wrap && (total && ((extend > space) || (child_flags & LAYX_BREAK)))) {
                end_child = child;
                hardbreak = (child_flags & LAYX_BREAK) == LAYX_BREAK;
                pchild->flags = child_flags | LAYX_BREAK;
                break;
            } else {
                used = extend;
                child = pchild->next_sibling;
            }
            ++total;
        }

        layx_scalar extra_space = space - used;
        float filler = 0.0f;
        float spacer = 0.0f;
        float extra_margin = 0.0f;

        if (extra_space > 0) {
            if (count > 0)
                filler = (float)extra_space / (float)count;
            else if (total > 0) {
                layx_justify_content justify = (layx_justify_content)(item_flags & LAYX_JUSTIFY_CONTENT_MASK);
                switch (justify) {
                case LAYX_JUSTIFY_SPACE_BETWEEN:
                    if (!wrap || ((end_child != LAYX_INVALID_ID) && !hardbreak)) {
                        if (total > 1) {
                            spacer = (float)extra_space / (float)(total - 1);
                        } else {
                            spacer = 0.0f;
                        }
                    }
                    break;
                case LAYX_JUSTIFY_FLEX_START:
                    break;
                case LAYX_JUSTIFY_FLEX_END:
                    extra_margin = extra_space;
                    break;
                case LAYX_JUSTIFY_CENTER:
                    extra_margin = extra_space / 2.0f;
                    break;
                case LAYX_JUSTIFY_SPACE_AROUND:
                    if (total > 0)
                        spacer = (float)extra_space / (float)total;
                    break;
                case LAYX_JUSTIFY_SPACE_EVENLY:
                    if (total > 0)
                        spacer = (float)extra_space / (float)(total + 1);
                    break;
                }
            }
        }

        float x = (float)content_offset + extra_margin;
        layx_id prev_child = LAYX_INVALID_ID;

        child = start_child;
        while (child != end_child) {
            if (spacer != 0 && child == start_child) {
                layx_justify_content justify = (layx_justify_content)(item_flags & LAYX_JUSTIFY_CONTENT_MASK);
                if (justify == LAYX_JUSTIFY_SPACE_AROUND) {
                    x += spacer / 2.0f;
                } else if (justify == LAYX_JUSTIFY_SPACE_EVENLY) {
                    x += spacer;
                }
            }

            layx_scalar ix0, ix1, x1;
            layx_item_t *pchild = layx_get_item(ctx, child);
            const layx_vec4 child_margins = pchild->margin_trbl;
            layx_vec4 child_rect = ctx->rects[child];
            child_rect[POINT_DIM(dim)] = 0;

            int has_flex_grow = (pchild->flex_grow > 0);

            // Flex 容器：margin 不合并，相邻 margin 相加
            if (prev_child == LAYX_INVALID_ID) {
                ix0 = (layx_scalar)(x + child_margins[START_SIDE(dim)]);
            } else {
                layx_item_t *pprev = layx_get_item(ctx, prev_child);
                layx_scalar gap = pprev->margin_trbl[END_SIDE(dim)] + child_margins[START_SIDE(dim)];
                ix0 = (layx_scalar)(x + gap);
            }

            if (has_flex_grow) {
                x1 = ix0 + filler;
            } else {
                layx_scalar child_size = (float)child_rect[SIZE_DIM(dim)];
                float constrained_size = (float)child_size;
                x1 = ix0 + constrained_size;
            }

            if (wrap)
                ix1 = (layx_scalar)layx_float_min(max_x2 - (float)child_margins[END_SIDE(dim)], x1);
            else
                ix1 = (layx_scalar)x1;

            child_rect[POINT_DIM(dim)] = ix0;
            child_rect[SIZE_DIM(dim)] = ix1 - ix0;
            ctx->rects[child] = child_rect;

            x = x1;
            prev_child = child;
            child = pchild->next_sibling;
            extra_margin = spacer;
        }

        start_child = end_child;
    }
}

// Helper to arrange a single child in a block container (with margin collapsing)
static LAYX_FORCE_INLINE
void layx_arrange_block_container_single_child(
            layx_context *ctx, layx_id item, int dim)
{
    layx_item_t *pitem = layx_get_item(ctx, item);
    layx_scalar space = layx_get_internal_space(ctx, item, dim);
    layx_scalar content_offset = layx_get_content_offset(ctx, item, dim);

    LAYX_DEBUG_PRINT("DEBUG layx_arrange_block_container_single_child(item=%d, dim=%d): space=%.1f, content_offset=%.1f\n", item, dim, space, content_offset);

    layx_id child = pitem->first_child;
    if (child != LAYX_INVALID_ID) {
        layx_item_t *pchild = layx_get_item(ctx, child);
        const layx_vec4 child_margins = pchild->margin_trbl;
        layx_vec4 child_rect = ctx->rects[child];
        child_rect[POINT_DIM(dim)] = 0;

        float x = (float)content_offset;
        layx_scalar ix0 = (layx_scalar)(x + child_margins[START_SIDE(dim)]);

        LAYX_DEBUG_PRINT("DEBUG set single child position: child=%d, x=%.1f, margin_start=%.1f, ix0=%.1f\n",
               child, x, child_margins[START_SIDE(dim)], ix0);

        float final_size = (float)child_rect[SIZE_DIM(dim)];
        child_rect[POINT_DIM(dim)] = ix0;
        child_rect[SIZE_DIM(dim)] = final_size;
        ctx->rects[child] = child_rect;
    }
}
static LAYX_FORCE_INLINE
void layx_arrange_block_container_multiple_children(
            layx_context *ctx, layx_id item, int dim, bool wrap)
{
    layx_item_t *pitem = layx_get_item(ctx, item);
    layx_scalar content_offset = layx_get_content_offset(ctx, item, dim);
    layx_scalar space = layx_get_internal_space(ctx, item, dim);
    
    // 只对垂直方向进行margin合并
    bool vertical = (dim == 1);  // 假设dim=1是垂直方向
    
    // 跟踪上一个元素的结束margin（用于合并）
    layx_scalar prev_margin_end = 0;
    
    layx_id child = pitem->first_child;
    layx_scalar current_y = content_offset;
    
    while (child != LAYX_INVALID_ID) {
        layx_item_t *pchild = layx_get_item(ctx, child);
        layx_vec4 child_rect = ctx->rects[child];
        const layx_vec4 margins = pchild->margin_trbl;
        
        // 计算当前元素的起点
        layx_scalar margin_start = margins[START_SIDE(dim)];
        layx_scalar margin_end = margins[END_SIDE(dim)];
        
        if (vertical) {
            // 垂直方向：处理margin合并
            if (child == pitem->first_child) {
                // 第一个子元素：可能与父元素的margin-top合并
                // 这里简化处理，实际更复杂
                current_y += margin_start;
            } else {
                // 与前一个元素的margin合并
                layx_scalar collapsed_margin = layx_scalar_max(prev_margin_end, margin_start);
                current_y += collapsed_margin - prev_margin_end;
            }
        } else {
            // 水平方向：不合并，直接相加
            current_y += margin_start;
        }
        
        // 设置子元素位置
        child_rect[POINT_DIM(dim)] = current_y;
        
        // 更新当前位置（加上子元素高度）
        current_y += child_rect[SIZE_DIM(dim)];
        
        if (vertical) {
            // 保存当前元素的结束margin（供下一个元素合并使用）
            prev_margin_end = margin_end;
        } else {
            // 水平方向：加上结束margin
            current_y += margin_end;
        }
        
        // 保存更新后的矩形
        ctx->rects[child] = child_rect;
        
        // 处理换行（如果启用）
        if (wrap) {
            // ... 换行逻辑
        }
        
        child = pchild->next_sibling;
    }
}

// Dispatcher function:根据容器类型和子元素数量调用相应的专用函数
static LAYX_FORCE_INLINE
void layx_arrange_stacked(layx_context *ctx, layx_id item, int dim, bool wrap)
{
    layx_item_t *pitem = layx_get_item(ctx, item);
    const uint32_t item_flags = pitem->flags;
    int is_flex_container = layx_is_flex_container(item_flags);

    // 判断子元素数量
    layx_id first_child = pitem->first_child;
    layx_id second_child = LAYX_INVALID_ID;
    if (first_child != LAYX_INVALID_ID) {
        layx_item_t *pfirst_child = layx_get_item(ctx, first_child);
        second_child = pfirst_child->next_sibling;
    }
    bool has_single_child = (second_child == LAYX_INVALID_ID);

    // 根据容器类型和子元素数量调用对应的函数
    if (is_flex_container) {
        if (has_single_child) {
            layx_arrange_flex_container_single_child(ctx, item, dim);
            LAYX_DEBUG_PRINT("DEBUG: layx_arrange_flex_container_single_child(item=%d, dim=%d) \n", item, dim);
        } else {
            layx_arrange_flex_container_multiple_children(ctx, item, dim, wrap);
            LAYX_DEBUG_PRINT("DEBUG: layx_arrange_flex_container_multiple_children(item=%d, dim=%d, wrap=%d) \n", item, dim, wrap);
        }
    } else {
        // Block容器（包括INLINE_BLOCK）
        if (has_single_child) {
            layx_arrange_block_container_single_child(ctx, item, dim);
            LAYX_DEBUG_PRINT("DEBUG: layx_arrange_block_container_single_child(item=%d, dim=%d) \n", item, dim);
        } else {
            layx_arrange_block_container_multiple_children(ctx, item, dim, wrap);
            LAYX_DEBUG_PRINT("DEBUG: layx_arrange_block_container_multiple_children(item=%d, dim=%d, wrap=%d) \n", item, dim, wrap);
        }
    }
}

static void layx_align_baseline(layx_context *ctx, layx_id container, int dim)
{
    layx_item_t *pcontainer = layx_get_item(ctx, container);
    float max_baseline = 0;
    
    // 第一步：收集所有子项的基线信息
    layx_id child = pcontainer->first_child;
    while (child != LAYX_INVALID_ID) {
        layx_item_t *pchild = layx_get_item(ctx, child);
        
        if (pchild->has_baseline) {
            max_baseline = layx_float_max(max_baseline, pchild->baseline);
        } else {
            // 对于没有基线的项目，使用默认值（如高度）
            max_baseline = layx_float_max(max_baseline, 
                                         ctx->rects[child][3] * 0.8f); // 80%高度
        }
        
        child = layx_next_sibling(ctx, child);
    }
    
    // 第二步：根据基线对齐调整位置
    child = pcontainer->first_child;
    while (child != LAYX_INVALID_ID) {
        layx_item_t *pchild = layx_get_item(ctx, child);
        layx_vec4 rect = ctx->rects[child];
        
        float child_baseline = pchild->has_baseline ? 
                              pchild->baseline : rect[3] * 0.8f;
        
        // 调整位置使基线对齐
        float adjustment = max_baseline - child_baseline;
        rect[1] += adjustment;  // 调整Y位置
        
        ctx->rects[child] = rect;
        child = layx_next_sibling(ctx, child);
    }
}
// Helper to arrange overlay items (cross-axis alignment)
static LAYX_FORCE_INLINE
void layx_arrange_overlay(layx_context *ctx, layx_id item, int dim)
{
    layx_item_t *pitem = layx_get_item(ctx, item);
    // For cross-axis alignment in flex layout, offset should be fixed (start of content-box)
    const layx_scalar offset = layx_get_content_offset(ctx, item, dim);
    const layx_scalar space = layx_get_internal_space(ctx, item, dim);

    LAYX_DEBUG_PRINT("DEBUG layx_arrange_overlay: item=%d, dim=%d=%d, offset=%.1f, space=%.1f\n",
                   item, dim, offset, space);

    // Get align-items for cross-axis alignment
    layx_align_items align_items = (layx_align_items)(pitem->flags & LAYX_ALIGN_ITEMS_MASK);

    layx_id child = pitem->first_child;
    while (child != LAYX_INVALID_ID) {
        layx_item_t *pchild = layx_get_item(ctx, child);
        const layx_vec4 child_margins = pchild->margin_trbl;
        layx_vec4 child_rect = ctx->rects[child];

        // Check if child has explicit align-self
        layx_align_self align_self = (layx_align_self)(pchild->flags & LAYX_ALIGN_SELF_MASK);

        // Apply alignment
        // First, check baseline (only for align-items, not for align-self)
        if (align_items == LAYX_ALIGN_ITEMS_BASELINE && align_self == LAYX_ALIGN_SELF_AUTO) {
            layx_align_baseline(ctx, item, dim);
            return;
        }

        // Determine alignment behavior based on align-self or align-items
        if (align_self != LAYX_ALIGN_SELF_AUTO) {
            // Child has explicit align-self - use it
            switch (align_self) {
                case LAYX_ALIGN_SELF_CENTER:
                    child_rect[POINT_DIM(dim)] = offset + child_margins[START_SIDE(dim)] + 
                        (space - child_margins[START_SIDE(dim)] - child_margins[END_SIDE(dim)] - child_rect[SIZE_DIM(dim)]) / 2;
                    LAYX_DEBUG_PRINT("DEBUG CENTER (align-self): item=%d, child=%d, offset=%.1f, space=%.1f, child_h=%.1f, margin_top=%.1f, margin_bottom=%.1f\n",
                                   item, child, offset, space, child_rect[SIZE_DIM(dim)], child_margins[START_SIDE(dim)], child_margins[END_SIDE(dim)]);
                    break;
                case LAYX_ALIGN_SELF_FLEX_END:
                    child_rect[POINT_DIM(dim)] = offset + space - child_margins[END_SIDE(dim)] - child_rect[SIZE_DIM(dim)];
                    break;
                case LAYX_ALIGN_SELF_STRETCH: {
                    bool has_fixed_size = (dim == 0) ? 
                        ((pchild->flags & LAYX_SIZE_FIXED_WIDTH) != 0) :
                        ((pchild->flags & LAYX_SIZE_FIXED_HEIGHT) != 0);
                    if (!has_fixed_size) {
                        child_rect[SIZE_DIM(dim)] = layx_scalar_max(0, space - child_margins[START_SIDE(dim)] - child_margins[END_SIDE(dim)]);
                    }
                    child_rect[POINT_DIM(dim)] = offset + child_margins[START_SIDE(dim)];
                    break;
                }
                case LAYX_ALIGN_SELF_FLEX_START:
                default:
                    child_rect[POINT_DIM(dim)] = offset + child_margins[START_SIDE(dim)];
                    break;
            }
        } else {
            // Child uses parent's align-items
            switch (align_items) {
                case LAYX_ALIGN_ITEMS_CENTER:
                    child_rect[POINT_DIM(dim)] = offset + child_margins[START_SIDE(dim)] + 
                        (space - child_margins[START_SIDE(dim)] - child_margins[END_SIDE(dim)] - child_rect[SIZE_DIM(dim)]) / 2;
                    LAYX_DEBUG_PRINT("DEBUG CENTER (align-items): item=%d, child=%d, offset=%.1f, space=%.1f, child_h=%.1f, margin_top=%.1f, margin_bottom=%.1f\n",
                                   item, child, offset, space, child_rect[SIZE_DIM(dim)], child_margins[START_SIDE(dim)], child_margins[END_SIDE(dim)]);
                    break;
                case LAYX_ALIGN_ITEMS_FLEX_END:
                    child_rect[POINT_DIM(dim)] = offset + space - child_margins[END_SIDE(dim)] - child_rect[SIZE_DIM(dim)];
                    break;
                case LAYX_ALIGN_ITEMS_STRETCH: {
                    bool has_fixed_size = (dim == 0) ? 
                        ((pchild->flags & LAYX_SIZE_FIXED_WIDTH) != 0) :
                        ((pchild->flags & LAYX_SIZE_FIXED_HEIGHT) != 0);
                    if (!has_fixed_size) {
                        child_rect[SIZE_DIM(dim)] = layx_scalar_max(0, space - child_margins[START_SIDE(dim)] - child_margins[END_SIDE(dim)]);
                    }
                    child_rect[POINT_DIM(dim)] = offset + child_margins[START_SIDE(dim)];
                    break;
                }
                case LAYX_ALIGN_ITEMS_FLEX_START:
                default:
                    child_rect[POINT_DIM(dim)] = offset + child_margins[START_SIDE(dim)];
                    break;
            }
        }
        ctx->rects[child] = child_rect;
        child = pchild->next_sibling;
    }
}
// Helper to arrange overlay squeezed range
static LAYX_FORCE_INLINE
void layx_arrange_overlay_squeezed_range(
        layx_context *ctx, int dim,
        layx_id start_item, layx_id end_item,
        layx_scalar offset, layx_scalar space)
{
    layx_id item = start_item;
    while (item != end_item) {
        layx_item_t *pitem = layx_get_item(ctx, item);
        const layx_vec4 margins = pitem->margin_trbl;
        layx_vec4 rect = ctx->rects[item];
        layx_scalar min_size = layx_scalar_max(0, space - rect[POINT_DIM(dim)] - margins[END_SIDE(dim)]);
        rect[SIZE_DIM(dim)] = layx_scalar_min(rect[SIZE_DIM(dim)], min_size);
        rect[POINT_DIM(dim)] += offset;
        ctx->rects[item] = rect;
        item = pitem->next_sibling;
    }
}

// Helper to arrange wrapped overlay squeezed
// This handles cross-axis alignment for wrapped flex layouts (flex-wrap)
static LAYX_FORCE_INLINE
layx_scalar layx_arrange_wrapped_overlay_squeezed(
        layx_context *ctx, layx_id item,int dim)
{
    layx_item_t *pitem = layx_get_item(ctx, item);
    layx_align_content align_content = (layx_align_content)(pitem->flags & LAYX_ALIGN_CONTENT_MASK);
    layx_align_items align_items = (layx_align_items)(pitem->flags & LAYX_ALIGN_ITEMS_MASK);
    
    layx_scalar offset = layx_get_content_offset(ctx, item, dim);
    const layx_scalar space = layx_get_internal_space(ctx, item, dim);
    
    // Collect all rows to apply align-content later
    struct {
        layx_id start_child;
        layx_id end_child;
        layx_scalar row_size;
        layx_scalar row_offset;
    } rows[32];
    int row_count = 0;
    
    layx_id child = pitem->first_child;
    layx_id start_child = child;
    layx_scalar need_size = 0;
    
    // Phase 1: Collect row information
    while (child != LAYX_INVALID_ID && row_count < 32) {
        layx_item_t *pchild = layx_get_item(ctx, child);
        if (pchild->flags & LAYX_BREAK || row_count == 0) {
            // Save previous row (except for first row)
            if (row_count > 0) {
                rows[row_count - 1].end_child = child;
                rows[row_count - 1].row_size = need_size;
            }
            
            // Start new row
            rows[row_count].start_child = child;
            rows[row_count].end_child = LAYX_INVALID_ID;
            rows[row_count].row_size = 0;
            rows[row_count].row_offset = offset + (row_count > 0 ? rows[row_count - 1].row_size : 0);
            
            need_size = 0;
        }
        
        const layx_vec4 rect = ctx->rects[child];
        layx_scalar child_size = rect[POINT_DIM(dim)] + rect[SIZE_DIM(dim)] + pchild->margin_trbl[END_SIDE(dim)];
        need_size = layx_scalar_max(need_size, child_size);
        
        child = pchild->next_sibling;
        
        // If this was the last child, save the final row
        if (child == LAYX_INVALID_ID) {
            rows[row_count].end_child = child;
            rows[row_count].row_size = need_size;
        }
        
        row_count++;
    }
    
    // Phase 2: Apply align-content to position rows
    layx_scalar total_rows_height = 0;
    for (int i = 0; i < row_count; i++) {
        total_rows_height += rows[i].row_size;
    }
    
    layx_scalar available_space_for_rows = space - total_rows_height;
    layx_scalar row_start_offset = offset;
    
    switch (align_content) {
        case LAYX_ALIGN_CONTENT_CENTER:
            row_start_offset = offset + available_space_for_rows / 2;
            break;
        case LAYX_ALIGN_CONTENT_FLEX_END:
            row_start_offset = offset + available_space_for_rows;
            break;
        case LAYX_ALIGN_CONTENT_SPACE_BETWEEN:
            // Space is distributed between rows, not before/after
            // Handled in the loop below
            break;
        case LAYX_ALIGN_CONTENT_SPACE_AROUND:
            // Space is distributed around rows
            // Handled in the loop below
            break;
        case LAYX_ALIGN_CONTENT_FLEX_START:
        case LAYX_ALIGN_CONTENT_STRETCH:
        default:
            // FLEX_START and STRETCH: no extra spacing
            break;
    }
    
    // Phase 3: Position rows with align-content spacing
    layx_scalar current_offset = row_start_offset;
    for (int i = 0; i < row_count; i++) {
        rows[i].row_offset = current_offset;
        
        // Calculate spacing for this row based on align-content
        if (align_content == LAYX_ALIGN_CONTENT_SPACE_BETWEEN) {
            if (i > 0) {
                layx_scalar gap = available_space_for_rows / (layx_scalar)(row_count - 1);
                current_offset += gap;
            }
            rows[i].row_offset = current_offset;
        } else if (align_content == LAYX_ALIGN_CONTENT_SPACE_AROUND) {
            layx_scalar gap = available_space_for_rows / (layx_scalar)row_count;
            current_offset += gap;
            rows[i].row_offset = current_offset;
        } else if (align_content == LAYX_ALIGN_CONTENT_STRETCH) {
            // Stretch rows to fill available space
            if (row_count > 0) {
                layx_scalar stretched_row_height = space / (layx_scalar)row_count;
                rows[i].row_size = layx_scalar_max(rows[i].row_size, stretched_row_height);
            }
        }
        
        current_offset += rows[i].row_size;
    }
    
    // Phase 4: Apply align-items/align-self to each row
    for (int i = 0; i < row_count; i++) {
        // Apply alignment to children in this row
        layx_arrange_overlay_squeezed_range(ctx, dim, rows[i].start_child, rows[i].end_child, rows[i].row_offset, rows[i].row_size);
    }
    
    return offset + total_rows_height + available_space_for_rows;
}

// 独立的 block 布局函数
// BLOCK: 元素独占一行，子元素在水平方向上叠加，在垂直方向上堆叠
static void layx_arrange_block(layx_context *ctx, layx_id item, int dim)
{
    layx_item_t *pitem = layx_get_item(ctx, item);

    if (dim == 1) {
        // Y 轴（垂直方向）：使用 stacked（堆叠）- 子元素从上到下排列
        layx_arrange_stacked(ctx, item,dim, false);
    } else {
        // X 轴（水平方向）：使用 overlay（叠加）- 子元素左对齐
        // 对于 BLOCK，子元素宽度由 padding-box 决定，需要考虑 margin
        const layx_scalar offset = layx_get_content_offset(ctx, item, dim);
        const layx_scalar space = layx_get_internal_space(ctx, item, dim);

        LAYX_DEBUG_PRINT("DEBUG layx_arrange_block(item=%d, dim=0): offset=%.1f, space=%.1f\n", item, offset, space);

        layx_id child = pitem->first_child;
        while (child != LAYX_INVALID_ID) {
            layx_item_t *pchild = layx_get_item(ctx, child);
            layx_vec4 child_rect = ctx->rects[child];
            const layx_vec4 child_margins = pchild->margin_trbl;
            const layx_vec4 child_padding = pchild->padding_trbl;
            const layx_vec4 child_border = pchild->border_trbl;

            // 子元素左对齐
            child_rect[POINT_DIM(dim)] = offset + child_margins[START_SIDE(dim)];

            // BLOCK 布局中，子元素宽度填充可用空间（减去左右 margin）
            // 但需要保留原有的 fixed width 如果设置了
            if (pchild->flags & LAYX_SIZE_FIXED_WIDTH) {
                // 如果子元素有固定宽度，保持原宽度
                // child_rect[2] 已经在 layx_calc_size 中设置了
                LAYX_DEBUG_PRINT("DEBUG layx_arrange_block: child %d has fixed width %.1f\n",
                       child, child_rect[2]);
            } else {
                // 如果子元素没有固定宽度，填充可用空间
                layx_scalar available_width = space - child_margins[START_SIDE(dim)] - child_margins[END_SIDE(dim)];
                child_rect[2] = available_width;
                LAYX_DEBUG_PRINT("DEBUG layx_arrange_block: child %d set width to %.1f (space=%.1f, margins=[%.1f,%.1f])\n",
                       child, available_width, space, child_margins[START_SIDE(dim)], child_margins[END_SIDE(dim)]);
            }

            ctx->rects[child] = child_rect;

            child = pchild->next_sibling;
        }
    }
}

// Inline 布局函数
// INLINE: 元素在一行内排列，宽度由内容决定
static void layx_arrange_inline(layx_context *ctx, layx_id item, int dim)
{
    layx_item_t *pitem = layx_get_item(ctx, item);

    // INLINE 元素的宽度由内容决定，不支持 flex 属性
    // 子元素在一行内排列，如果超出宽度则换行
    if (dim == 0) {
        // X 轴（水平方向）：子元素从左到右排列
        const layx_scalar offset = layx_get_content_offset(ctx, item,0);
        const layx_scalar space = layx_get_internal_space(ctx, item, 0);

        float x = (float)offset;
        float line_start = x;
        float max_line_width = 0.0f;
        layx_id prev_child = LAYX_INVALID_ID;  // 用于记录上一个子项，正确处理margin合并

        layx_id child = pitem->first_child;
        while (child != LAYX_INVALID_ID) {
            layx_item_t *pchild = layx_get_item(ctx, child);
            layx_vec4 child_rect = ctx->rects[child];
            const layx_vec4 child_margins = pchild->margin_trbl;

            // 计算子元素的占用宽度（包含margin）
            float child_content_width = (float)child_rect[2];

            // 正确处理margin合并：第一个元素或换行后的第一个元素使用左margin
            float margin_left = 0.0f;
            float margin_right = 0.0f;

            if (prev_child == LAYX_INVALID_ID) {
                // 第一个子项（或换行后的第一个）：使用完整的左margin
                margin_left = (float)child_margins[START_SIDE(dim)];
            } else {
                // 非第一个子项：与上一个子项之间的边距取最大值
                layx_item_t *pprev = layx_get_item(ctx, prev_child);
                margin_left = layx_float_max((float)pprev->margin_trbl[END_SIDE(dim)], (float)child_margins[START_SIDE(dim)]);
            }

            // 如果是最后一个子项，使用完整的右margin
            layx_id next_child = pchild->next_sibling;
            if (next_child == LAYX_INVALID_ID || (layx_get_item(ctx, next_child)->flags & LAYX_BREAK)) {
                margin_right = (float)child_margins[END_SIDE(dim)];
            }

            float child_total_width = child_content_width + margin_left + margin_right;

            // 检查是否需要换行
            if (x + child_total_width > offset + space && x > line_start) {
                x = (float)offset;  // 换行
                prev_child = LAYX_INVALID_ID;  // 换行后重置prev_child，重新计算margin
                // 换行后，当前元素成为新行的第一个元素，需要使用完整的左margin
                margin_left = (float)child_margins[START_SIDE(dim)];
                child_total_width = child_content_width + margin_left + margin_right;
            }

            // 设置子元素位置
            child_rect[0] = (layx_scalar)(x + margin_left);
            x += child_total_width;
            max_line_width = layx_float_max(max_line_width, x - offset);

            ctx->rects[child] = child_rect;
            prev_child = child;
            child = pchild->next_sibling;
        }
    } else {
        // Y 轴（垂直方向）：使用 overlay（叠加）
        layx_arrange_overlay(ctx, item, 1);
    }
}

// Inline-block 布局函数
// INLINE_BLOCK: 元素在一行内排列，但可以设置宽高
static void layx_arrange_inline_block(layx_context *ctx, layx_id item, int dim)
{
    layx_item_t *pitem = layx_get_item(ctx, item);

    // INLINE_BLOCK 元素在一行内排列，支持设置宽高
    if (dim == 0) {
        // X 轴（水平方向）：子元素从左到右排列
        const layx_scalar offset = layx_get_content_offset(ctx, item, 0);
        const layx_scalar space = layx_get_internal_space(ctx, item, 0);

        float x = (float)offset;
        float line_start = x;
        float max_line_width = 0.0f;
        layx_id prev_child = LAYX_INVALID_ID;  // 用于记录上一个子项，正确处理margin合并

        layx_id child = pitem->first_child;
        while (child != LAYX_INVALID_ID) {
            layx_item_t *pchild = layx_get_item(ctx, child);
            layx_vec4 child_rect = ctx->rects[child];
            const layx_vec4 child_margins = pchild->margin_trbl;

            // 计算子元素的占用宽度（包含margin）
            float child_content_width = (float)child_rect[2];

            // 正确处理margin合并：第一个元素或换行后的第一个元素使用左margin
            float margin_left = 0.0f;
            float margin_right = 0.0f;

            if (prev_child == LAYX_INVALID_ID) {
                // 第一个子项（或换行后的第一个）：使用完整的左margin
                margin_left = (float)child_margins[START_SIDE(dim)];
            } else {
                // 非第一个子项：与上一个子项之间的边距取最大值
                layx_item_t *pprev = layx_get_item(ctx, prev_child);
                margin_left = layx_float_max((float)pprev->margin_trbl[END_SIDE(dim)], (float)child_margins[START_SIDE(dim)]);
            }

            // 如果是最后一个子项，使用完整的右margin
            layx_id next_child = pchild->next_sibling;
            if (next_child == LAYX_INVALID_ID || (layx_get_item(ctx, next_child)->flags & LAYX_BREAK)) {
                margin_right = (float)child_margins[END_SIDE(dim)];
            }

            float child_total_width = child_content_width + margin_left + margin_right;

            // 检查是否需要换行
            if (x + child_total_width > offset + space && x > line_start) {
                x = (float)offset;  // 换行
                prev_child = LAYX_INVALID_ID;  // 换行后重置prev_child，重新计算margin
                // 换行后，当前元素成为新行的第一个元素，需要使用完整的左margin
                margin_left = (float)child_margins[START_SIDE(dim)];
                child_total_width = child_content_width + margin_left + margin_right;
            }

            // 设置子元素位置
            child_rect[0] = (layx_scalar)(x + margin_left);
            x += child_total_width;
            max_line_width = layx_float_max(max_line_width, x - offset);

            ctx->rects[child] = child_rect;
            prev_child = child;
            child = pchild->next_sibling;
        }
    } else {
        // Y 轴（垂直方向）：使用 overlay（叠加）
        layx_arrange_overlay(ctx, item, 1);
    }
}

// PHASE 2: Arrange items (second pass)
static void layx_arrange(layx_context *ctx, layx_id item, int dim)
{
    layx_item_t *pitem = layx_get_item(ctx, item);

    const uint32_t flags = pitem->flags;
    layx_display display = layx_get_display_from_flags(flags);
    layx_flex_direction direction = (layx_flex_direction)(flags & LAYX_FLEX_DIRECTION_MASK);
    layx_flex_wrap wrap = (layx_flex_wrap)(flags & LAYX_FLEX_WRAP_MASK);
    
    // 根据不同的 display 类型调用不同的布局函数
    switch (display) {
       
        case LAYX_DISPLAY_INLINE:
            layx_arrange_inline(ctx, item, dim);
            LAYX_DEBUG_PRINT("DEBUG: layx_arrange_inline(item=%d, dim=%d, display=%d) \n", item, dim, display);
            break;
        case LAYX_DISPLAY_INLINE_BLOCK:
            layx_arrange_inline_block(ctx, item,dim);
            LAYX_DEBUG_PRINT("DEBUG: layx_arrange_inline_block(item=%d, dim=%d, display=%d) \n", item, dim, display);
            break;
        case LAYX_DISPLAY_FLEX: {
            bool is_row_direction = (direction == LAYX_FLEX_DIRECTION_ROW || direction == LAYX_FLEX_DIRECTION_ROW_REVERSE);
            bool is_wrapped = (wrap != LAYX_FLEX_WRAP_NOWRAP);
            
            if (is_wrapped) {
                if (is_row_direction) {
                    if (dim == 0) {
                        layx_arrange_stacked(ctx, item, 0, true);
                    } else {
                        layx_arrange_wrapped_overlay_squeezed(ctx, item,1);
                    }
                } else {
                    if (dim == 1) {
                        layx_arrange_stacked(ctx, item, 1, true);
                        layx_arrange_wrapped_overlay_squeezed(ctx, item,0);
                    } else {
                        layx_arrange_wrapped_overlay_squeezed(ctx, item,0);
                    }
                }
            } else {
                if ((is_row_direction && dim == 0) || (!is_row_direction && dim == 1)) {
                    layx_arrange_stacked(ctx, item, dim, false);
                } else {
                    // Use layx_arrange_overlay for cross-axis alignment (align-items)
                    layx_arrange_overlay(ctx, item,dim);
                }
            }
            break;
        }
        default:
        case LAYX_DISPLAY_BLOCK:
           layx_arrange_block(ctx, item, dim);
           LAYX_DEBUG_PRINT("DEBUG: layx_arrange_block(item=%d, dim=%d, display=%d) \n", item, dim, display);
           break;
    }
    
    // 递归处理子项
    layx_id child = pitem->first_child;
    while (child != LAYX_INVALID_ID) {
        layx_arrange(ctx, child, dim);
        child = layx_next_sibling(ctx, child);
    }
}

// Debug functions
const char* layx_get_layout_properties_string(layx_context *ctx, layx_id item)
{
    layx_item_t *pitem = layx_get_item(ctx, item);
    uint32_t flags = pitem->flags;
    
    static char buf[256];
    int len = 0;
    
    layx_display display = layx_get_display_from_flags(flags);
    switch (display) {
        case LAYX_DISPLAY_BLOCK: len += snprintf(buf + len, sizeof(buf) - len, "display:BLOCK"); break;
        case LAYX_DISPLAY_FLEX: len += snprintf(buf + len, sizeof(buf) - len, "display:FLEX"); break;
        case LAYX_DISPLAY_INLINE_BLOCK: len += snprintf(buf + len, sizeof(buf) - len, "display:INLINE_BLOCK"); break;
        case LAYX_DISPLAY_INLINE: len += snprintf(buf + len, sizeof(buf) - len, "display:INLINE"); break;
        default: len += snprintf(buf + len, sizeof(buf) - len, "display:INVALID(%d)", display); break;
    }
    
    if (display == LAYX_DISPLAY_FLEX) {
        layx_flex_direction dir = (layx_flex_direction)(flags & LAYX_FLEX_DIRECTION_MASK);
        const char* dir_str = "UNKNOWN";
        switch (dir) {
            case LAYX_FLEX_DIRECTION_ROW: dir_str = "ROW"; break;
            case LAYX_FLEX_DIRECTION_COLUMN: dir_str = "COLUMN"; break;
            case LAYX_FLEX_DIRECTION_ROW_REVERSE: dir_str = "ROW_REVERSE"; break;
            case LAYX_FLEX_DIRECTION_COLUMN_REVERSE: dir_str = "COLUMN_REVERSE"; break;
            default: dir_str = "INVALID"; break;
        }
        len += snprintf(buf + len, sizeof(buf) - len, "|dir:%s", dir_str);
        
        layx_flex_wrap wrap = (layx_flex_wrap)(flags & LAYX_FLEX_WRAP_MASK);
        const char* wrap_str = "UNKNOWN";
        switch (wrap) {
            case LAYX_FLEX_WRAP_NOWRAP: wrap_str = "NOWRAP"; break;
            case LAYX_FLEX_WRAP_WRAP: wrap_str = "WRAP"; break;
            case LAYX_FLEX_WRAP_WRAP_REVERSE: wrap_str = "WRAP_REVERSE"; break;
            default: wrap_str = "INVALID"; break;
        }
        len += snprintf(buf + len, sizeof(buf) - len, "|wrap:%s", wrap_str);
        
        layx_justify_content justify = (layx_justify_content)(flags & LAYX_JUSTIFY_CONTENT_MASK);
        const char* justify_str = "UNKNOWN";
        switch (justify) {
            case LAYX_JUSTIFY_FLEX_START: justify_str = "FLEX_START"; break;
            case LAYX_JUSTIFY_CENTER: justify_str = "CENTER"; break;
            case LAYX_JUSTIFY_FLEX_END: justify_str = "FLEX_END"; break;
            case LAYX_JUSTIFY_SPACE_BETWEEN: justify_str = "SPACE_BETWEEN"; break;
            case LAYX_JUSTIFY_SPACE_AROUND: justify_str = "SPACE_AROUND"; break;
            case LAYX_JUSTIFY_SPACE_EVENLY: justify_str = "SPACE_EVENLY"; break;
            default: justify_str = "INVALID"; break;
        }
        len += snprintf(buf + len, sizeof(buf) - len, "|justify:%s", justify_str);
        
        layx_align_items align_items = (layx_align_items)(flags & LAYX_ALIGN_ITEMS_MASK);
        const char* align_items_str = "UNKNOWN";
        switch (align_items) {
            case LAYX_ALIGN_ITEMS_STRETCH: align_items_str = "STRETCH"; break;
            case LAYX_ALIGN_ITEMS_FLEX_START: align_items_str = "FLEX_START"; break;
            case LAYX_ALIGN_ITEMS_CENTER: align_items_str = "CENTER"; break;
            case LAYX_ALIGN_ITEMS_FLEX_END: align_items_str = "FLEX_END"; break;
            case LAYX_ALIGN_ITEMS_BASELINE: align_items_str = "BASELINE"; break;
            default: align_items_str = "INVALID"; break;
        }
        len += snprintf(buf + len, sizeof(buf) - len, "|align-items:%s", align_items_str);
        
        layx_align_content align_content = (layx_align_content)(flags & LAYX_ALIGN_CONTENT_MASK);
        const char* align_content_str = "UNKNOWN";
        switch (align_content) {
            case LAYX_ALIGN_CONTENT_STRETCH: align_content_str = "STRETCH"; break;
            case LAYX_ALIGN_CONTENT_FLEX_START: align_content_str = "FLEX_START"; break;
            case LAYX_ALIGN_CONTENT_CENTER: align_content_str = "CENTER"; break;
            case LAYX_ALIGN_CONTENT_FLEX_END: align_content_str = "FLEX_END"; break;
            case LAYX_ALIGN_CONTENT_SPACE_BETWEEN: align_content_str = "SPACE_BETWEEN"; break;
            case LAYX_ALIGN_CONTENT_SPACE_AROUND: align_content_str = "SPACE_AROUND"; break;
            default: align_content_str = "INVALID"; break;
        }
        len += snprintf(buf + len, sizeof(buf) - len, "|align-content:%s", align_content_str);
    }
    
    return buf;
}

const char* layx_get_item_alignment_string(layx_context *ctx, layx_id item)
{
    layx_item_t *pitem = layx_get_item(ctx, item);
    uint32_t flags = pitem->flags;

    static char buf[128];
    int len = 0;
    int first = 1;

    if (flags & LAYX_SIZE_FIXED_WIDTH) {
        len += snprintf(buf + len, sizeof(buf) - len, "%sWIDTH_FIXED", first ? "" : "|");
        first = 0;
    }
    if (flags & LAYX_SIZE_FIXED_HEIGHT) {
        len += snprintf(buf + len, sizeof(buf) - len, "%sHEIGHT_FIXED", first ? "" : "|");
        first = 0;
    }

    if (len == 0) return "default";
    return buf;
}

// Text measurement API functions
void layx_set_item_measure_callback(
    layx_context *ctx,
    layx_id item_id,
    layx_measure_text_fn fn,
    void *user_data
) {
    layx_item_t *pitem = layx_get_item(ctx, item_id);
    pitem->measure_text_fn = fn;
    pitem->measure_text_user_data = user_data;
}

// Web标准 API 实现

// clientWidth/clientHeight: 绘制区域（内容+内边距，无滚动条）
layx_scalar layx_get_client_width(layx_context *ctx, layx_id item) {
    layx_vec4 rect = ctx->rects[item];
    layx_item_t *pitem = layx_get_item(ctx, item);

    // 计算绘制区域宽度：rect宽度 - 边框宽度
    layx_scalar client_width = rect[2] - pitem->border_trbl[START_SIDE(DIM_WIDTH)] - pitem->border_trbl[END_SIDE(DIM_WIDTH)];

    // 如果有垂直滚动条，减去滚动条宽度
    if (pitem->has_scrollbars & LAYX_HAS_VSCROLL) {
        // 假设滚动条宽度为 15
        client_width -= 15;
    }

    return client_width > 0 ? client_width : 0;
}

layx_scalar layx_get_client_height(layx_context *ctx, layx_id item) {
    layx_vec4 rect = ctx->rects[item];
    layx_item_t *pitem = layx_get_item(ctx, item);

    // 计算绘制区域高度：rect高度 - 边框高度
    layx_scalar client_height = rect[3] - pitem->border_trbl[START_SIDE(DIM_HEIGHT)] - pitem->border_trbl[END_SIDE(DIM_HEIGHT)];
    
    // 如果有水平滚动条，减去滚动条高度
    if (pitem->has_scrollbars & LAYX_HAS_HSCROLL) {
        // 假设滚动条高度为 15
        client_height -= 15;
    }
    
    return client_height > 0 ? client_height : 0;
}
void layx_get_client_size(layx_context *ctx, layx_id item, layx_vec2 *size){
    (*size)[0] = layx_get_client_width(ctx, item);
    (*size)[1] = layx_get_client_height(ctx, item);
}
void layx_get_client_size_wh(layx_context *ctx, layx_id item, layx_scalar *width, layx_scalar *height){
    *width = layx_get_client_width(ctx, item);
    *height = layx_get_client_height(ctx, item);
}


// scrollWidth/scrollHeight: 内容区域（实际内容大小）
layx_scalar layx_get_scroll_width(layx_context *ctx, layx_id item) {
    layx_item_t *pitem = layx_get_item(ctx, item);
    return pitem->content_size[0];
}

layx_scalar layx_get_scroll_height(layx_context *ctx, layx_id item) {
    layx_item_t *pitem = layx_get_item(ctx, item);
    return pitem->content_size[1];
}

// offsetWidth/offsetHeight: 视口（边框+内边距+内容，无margin）
layx_scalar layx_get_offset_width(layx_context *ctx, layx_id item) {
    layx_vec4 rect = ctx->rects[item];
    // offsetWidth 就是 rect[2]（不包含margin）
    return rect[2];
}

layx_scalar layx_get_offset_height(layx_context *ctx, layx_id item) {
    layx_vec4 rect = ctx->rects[item];
    // offsetHeight 就是 rect[3]（不包含margin）
    return rect[3];
}
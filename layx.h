#ifndef LAYX_INCLUDE_HEADER
#define LAYX_INCLUDE_HEADER
#define LAYX_FLOAT 1

// Users of this library can define LAYX_IMPLEMENTATION in exactly one C or C++ file
// before you include layx.h.
//
// Your includes should look like this:
//   #define LAYX_IMPLEMENTATION
//   #include "layx.h"

#include <stdint.h>
#include <stdbool.h>

#ifndef LAYX_EXPORT
#define LAYX_EXPORT extern
#endif

// Users of this library can define LAYX_ASSERT if they would like to use an
// assert other than the one from assert.h.
#ifndef LAYX_ASSERT
#include <assert.h>
#define LAYX_ASSERT assert
#endif

// 'static inline' for things we always want inlined
#if defined(__GNUC__) || defined(__clang__)
#define LAYX_STATIC_INLINE __attribute__((always_inline)) static inline
#elif defined(_MSC_VER)
#define LAYX_STATIC_INLINE __forceinline static
#else
#define LAYX_STATIC_INLINE inline static
#endif

typedef uint32_t layx_id;
#if LAYX_FLOAT == 1
typedef float layx_scalar;
#else
typedef int16_t layx_scalar;
#endif

#define LAYX_INVALID_ID UINT32_MAX

// Text measurement callback type
// 注意：user_data 由调用端设置，通常包含字体和文本信息
typedef void (*layx_measure_text_fn)(
    void *user_data,
    int is_wrap,
    float wrap_width,
    float *out_width,
    float *out_height
);

// Vector types
#if defined(__GNUC__) || defined(__clang__)
#ifdef LAYX_FLOAT
typedef float layx_vec4 __attribute__ ((__vector_size__ (16), aligned(4)));
typedef float layx_vec2 __attribute__ ((__vector_size__ (8), aligned(4)));
#else
typedef int16_t layx_vec4 __attribute__ ((__vector_size__ (8), aligned(2)));
typedef int16_t layx_vec2 __attribute__ ((__vector_size__ (4), aligned(2)));
#endif
#elif defined(_MSC_VER)
struct layx_vec4 {
    layx_scalar xyzw[4];
    const layx_scalar& operator[](int index) const
    { return xyzw[index]; }
    layx_scalar& operator[](int index)
    { return xyzw[index]; }
};
struct layx_vec2 {
    layx_scalar xy[2];
    const layx_scalar& operator[](int index) const
    { return xy[index]; }
    layx_scalar& operator[](int index)
    { return xy[index]; }
};
#endif

// Item structure
typedef struct layx_item_t {
    uint32_t flags;
    layx_id first_child;
    layx_id next_sibling;
    layx_id parent;
    layx_vec4 margins;
    layx_vec4 padding;
    layx_vec4 border;
    layx_vec2 size;
    layx_vec2 min_size;
    layx_vec2 max_size;
    layx_vec4 position; // l t r b
    layx_scalar flex_grow;
    layx_scalar flex_shrink;
    layx_scalar flex_basis;
    
    // 滚动状态
    layx_vec2 scroll_offset;     // [0]=scrollLeft, [1]=scrollTop
    layx_vec2 scroll_max;        // [0]=maxScrollLeft, [1]=maxScrollTop
    
    // 内容实际尺寸（用于计算滚动范围）
    layx_vec2 content_size;      // [0]=contentWidth, [1]=contentHeight
    
    // 滚动条标志位
    uint8_t overflow_x;          // overflow-x 属性
    uint8_t overflow_y;          // overflow-y 属性
    uint8_t has_scrollbars;      // 标志位：是否有滚动条 (bit0=v, bit1=h)

    float baseline;          // 基线偏移（从项目顶部算起）
    uint8_t has_baseline;    // 是否有有效的基线信息
    
    // ============ 新增：文本测量相关字段 ============
    layx_measure_text_fn measure_text_fn;  // NULL 表示不是文本节点
    void *measure_text_user_data;          // 用户数据（通常指向 ui_component）
} layx_item_t;

// Context structure
typedef struct layx_context {
    layx_item_t *items;
    // rects是执行布局计算后缓存的计算结果。
    // 表示的元素margin-box相对于父元素content-box的偏移量
    // ✅ 包含 border
    // ✅ 包含 padding
    // ✅ 包含 content
    // ✅ 包含了该元素自身的margin
    layx_vec4 *rects;
    layx_id capacity;
    layx_id count;
} layx_context;

// Display property
typedef enum layx_display {
    LAYX_DISPLAY_BLOCK = 0,
    LAYX_DISPLAY_FLEX = 1,
    LAYX_DISPLAY_INLINE = 2,
    LAYX_DISPLAY_INLINE_BLOCK = 3
} layx_display;

// Flex direction
typedef enum layx_flex_direction {
    LAYX_FLEX_DIRECTION_ROW,
    LAYX_FLEX_DIRECTION_COLUMN,
    LAYX_FLEX_DIRECTION_ROW_REVERSE,
    LAYX_FLEX_DIRECTION_COLUMN_REVERSE
} layx_flex_direction;

// Flex wrap (bits 4-5)
// whether items wrap to the next row (only applies if combined width of items is greater than container's)
typedef enum layx_flex_wrap {
    LAYX_FLEX_WRAP_NOWRAP = 0 << 4,  // 0x0000
    LAYX_FLEX_WRAP_WRAP = 1 << 4,    // 0x0010
    LAYX_FLEX_WRAP_WRAP_REVERSE = 2 << 4  // 0x0020
} layx_flex_wrap;

// Justify content (bits 6-8)
// alignment along the x axis
typedef enum layx_justify_content {
    LAYX_JUSTIFY_FLEX_START = 0x000,
    LAYX_JUSTIFY_CENTER = 0x0040,
    LAYX_JUSTIFY_FLEX_END = 0x0080,
    LAYX_JUSTIFY_SPACE_BETWEEN = 0x00C0,
    LAYX_JUSTIFY_SPACE_AROUND = 0x0100,
    LAYX_JUSTIFY_SPACE_EVENLY = 0x0140
} layx_justify_content;

// Align items (bits 9-11)
// alignment along the y axis
typedef enum layx_align_items {
    LAYX_ALIGN_ITEMS_STRETCH = 0x0000,
    LAYX_ALIGN_ITEMS_FLEX_START = 0x0200,
    LAYX_ALIGN_ITEMS_CENTER = 0x0400,
    LAYX_ALIGN_ITEMS_FLEX_END = 0x0600,
    LAYX_ALIGN_ITEMS_BASELINE = 0x0800
} layx_align_items;

// Align content (bits 12-14)
// only applies if there is more than one row of items
typedef enum layx_align_content {
    LAYX_ALIGN_CONTENT_STRETCH = 0x0000,
    LAYX_ALIGN_CONTENT_FLEX_START = 0x1000,
    LAYX_ALIGN_CONTENT_CENTER = 0x2000,
    LAYX_ALIGN_CONTENT_FLEX_END = 0x3000,
    LAYX_ALIGN_CONTENT_SPACE_BETWEEN = 0x4000,
    LAYX_ALIGN_CONTENT_SPACE_AROUND = 0x5000
} layx_align_content;


// Align self (bits 15-17)
typedef enum layx_align_self {
    LAYX_ALIGN_SELF_AUTO = 0x0000,
    LAYX_ALIGN_SELF_FLEX_START = 0x08000,
    LAYX_ALIGN_SELF_CENTER = 0x10000,
    LAYX_ALIGN_SELF_FLEX_END = 0x18000,
    LAYX_ALIGN_SELF_STRETCH = 0x20000
} layx_align_self;

// 添加 overflow 属性枚举
typedef enum layx_overflow {
    LAYX_OVERFLOW_VISIBLE = 0,
    LAYX_OVERFLOW_HIDDEN,
    LAYX_OVERFLOW_SCROLL,
    LAYX_OVERFLOW_AUTO
} layx_overflow;

// Style structure
typedef struct layx_style {
    layx_display display;
    layx_flex_direction flex_direction;
    layx_flex_wrap flex_wrap;
    layx_justify_content justify_content;
    layx_align_items align_items;
    layx_align_content align_content;
    layx_scalar width, height;
    layx_scalar min_width, min_height;
    layx_scalar max_width, max_height;
    layx_scalar margin_top, margin_right, margin_bottom, margin_left;
    layx_scalar padding_top, padding_right, padding_bottom, padding_left;
    layx_scalar border_top, border_right, border_bottom, border_left;
    layx_align_self align_self;
    layx_scalar flex_grow, flex_shrink, flex_basis;
} layx_style;

// Bit masks for internal use
// Bit layout:
// Bits 0-1: FLEX_DIRECTION (0x0003)
// Bits 2-3: DISPLAY_TYPE (0x000C)
// Bits 4-5: FLEX_WRAP (0x0030)
// Bits 6-8: JUSTIFY_CONTENT (0x01C0)
// Bits 9-11: ALIGN_ITEMS (0x0E00)
// Bits 12-14: ALIGN_CONTENT (0x7000)
// Bits 15-17: ALIGN_SELF (0x38000)
// Bit 18: ITEM_INSERTED (0x40000)
// Bit 19: SIZE_FIXED_WIDTH (0x80000)
// Bit 20: SIZE_FIXED_HEIGHT (0x100000)
// Bit 21: BREAK (0x200000)
// Bit 22: HAS_VSCROLL (0x400000)
// Bit 23: HAS_HSCROLL (0x800000)

#define LAYX_FLEX_DIRECTION_MASK    0x0003
#define LAYX_DISPLAY_TYPE_MASK     0x000C
#define LAYX_FLEX_WRAP_MASK         0x0030
#define LAYX_JUSTIFY_CONTENT_MASK   0x01C0
#define LAYX_ALIGN_ITEMS_MASK       0x0E00
#define LAYX_ALIGN_CONTENT_MASK     0x7000
#define LAYX_ALIGN_SELF_MASK        0x38000  // Use bits 15-17 for align-self

// Internal flags
enum {
    LAYX_ITEM_INSERTED = 0x40000,
    LAYX_SIZE_FIXED_WIDTH = 0x80000,
    LAYX_SIZE_FIXED_HEIGHT = 0x100000,
    LAYX_SIZE_FIXED_MASK = LAYX_SIZE_FIXED_WIDTH | LAYX_SIZE_FIXED_HEIGHT,
    LAYX_BREAK = 0x200000,
    
    // 滚动条标志位 (在flags中使用)
    LAYX_HAS_VSCROLL = 0x400000,  // 垂直滚动条
    LAYX_HAS_HSCROLL = 0x800000,  // 水平滚动条
    LAYX_HAS_SCROLLBARS = LAYX_HAS_VSCROLL | LAYX_HAS_HSCROLL
};

// Context management
LAYX_EXPORT void layx_init_context(layx_context *ctx);
LAYX_EXPORT void layx_reserve_items_capacity(layx_context *ctx, layx_id count);
LAYX_EXPORT void layx_destroy_context(layx_context *ctx);
LAYX_EXPORT void layx_reset_context(layx_context *ctx);

// Layout calculation
LAYX_EXPORT void layx_run_context(layx_context *ctx);
LAYX_EXPORT void layx_run_item(layx_context *ctx, layx_id item);
LAYX_EXPORT void layx_clear_item_break(layx_context *ctx, layx_id item);

// Item management
LAYX_EXPORT layx_id layx_items_count(layx_context *ctx);
LAYX_EXPORT layx_id layx_items_capacity(layx_context *ctx);
LAYX_EXPORT layx_id layx_item(layx_context *ctx);
LAYX_EXPORT int layx_is_inserted(layx_context *ctx, layx_id child);
LAYX_EXPORT void layx_append(layx_context *ctx, layx_id parent, layx_id child);
LAYX_EXPORT void layx_insert_after(layx_context *ctx, layx_id earlier, layx_id later);
LAYX_EXPORT void layx_prepend(layx_context *ctx, layx_id parent, layx_id new_child);

// Display property
LAYX_EXPORT void layx_set_display(layx_context *ctx, layx_id item, layx_display display);
LAYX_EXPORT const char* layx_get_display_string(layx_display display);

// Display helper functions (internal use, but needed by scroll_utils.c)
LAYX_STATIC_INLINE layx_display layx_get_display_from_flags(uint32_t flags) {
    return (layx_display)((flags & LAYX_DISPLAY_TYPE_MASK) >> 2);
}

// Helper to check if flex container
static inline bool layx_is_flex_container(uint32_t flags) {
    return ((flags & LAYX_DISPLAY_TYPE_MASK) >> 2) == LAYX_DISPLAY_FLEX;
}

// Helper to check if block display
static inline bool layx_is_block_display(uint32_t flags) {
    return ((flags & LAYX_DISPLAY_TYPE_MASK) >> 2) == LAYX_DISPLAY_BLOCK;
}

// Helper to check if inline display
static inline bool layx_is_inline_display(uint32_t flags) {
    uint32_t display_type = (flags & LAYX_DISPLAY_TYPE_MASK) >> 2;
    return display_type == LAYX_DISPLAY_INLINE || display_type == LAYX_DISPLAY_INLINE_BLOCK;
}

// Helper to check if inline-block display
static inline bool layx_is_inline_block_display(uint32_t flags) {
    return ((flags & LAYX_DISPLAY_TYPE_MASK) >> 2) == LAYX_DISPLAY_INLINE_BLOCK;
}

// Flex properties
LAYX_EXPORT void layx_set_flex_direction(layx_context *ctx, layx_id item, layx_flex_direction direction);
LAYX_EXPORT void layx_set_flex_wrap(layx_context *ctx, layx_id item, layx_flex_wrap wrap);
LAYX_EXPORT void layx_set_justify_content(layx_context *ctx, layx_id item, layx_justify_content justify);
LAYX_EXPORT void layx_set_align_items(layx_context *ctx, layx_id item, layx_align_items align);
LAYX_EXPORT void layx_set_align_content(layx_context *ctx, layx_id item, layx_align_content align);
LAYX_EXPORT void layx_set_flex(layx_context *ctx, layx_id item,
                                layx_flex_direction direction,
                                layx_flex_wrap wrap,
                                layx_justify_content justify,
                                layx_align_items align_items,
                                layx_align_content align_content);

// String getters for flex properties
LAYX_EXPORT const char* layx_get_flex_direction_string(layx_flex_direction direction);
LAYX_EXPORT const char* layx_get_flex_wrap_string(layx_flex_wrap wrap);
LAYX_EXPORT const char* layx_get_justify_content_string(layx_justify_content justify);
LAYX_EXPORT const char* layx_get_align_items_string(layx_align_items align);
LAYX_EXPORT const char* layx_get_align_content_string(layx_align_content align);

// Size properties
LAYX_EXPORT void layx_set_width(layx_context *ctx, layx_id item, layx_scalar width);
LAYX_EXPORT void layx_set_height(layx_context *ctx, layx_id item, layx_scalar height);
LAYX_EXPORT void layx_set_min_width(layx_context *ctx, layx_id item, layx_scalar min_width);
LAYX_EXPORT void layx_set_min_height(layx_context *ctx, layx_id item, layx_scalar min_height);
LAYX_EXPORT void layx_set_max_width(layx_context *ctx, layx_id item, layx_scalar max_width);
LAYX_EXPORT void layx_set_max_height(layx_context *ctx, layx_id item, layx_scalar max_height);
LAYX_EXPORT void layx_set_size(layx_context *ctx, layx_id item, layx_scalar width, layx_scalar height);
LAYX_EXPORT void layx_set_min_size(layx_context *ctx, layx_id item, layx_scalar min_width, layx_scalar min_height);
LAYX_EXPORT layx_vec2 layx_get_size(layx_context *ctx, layx_id item);

// Position properties
LAYX_EXPORT void layx_set_position_lt(layx_context *ctx, layx_id item, layx_scalar left, layx_scalar top);
LAYX_EXPORT void layx_set_position_rb(layx_context *ctx, layx_id item, layx_scalar right, layx_scalar bottom);
LAYX_EXPORT void layx_set_left(layx_context *ctx, layx_id item, layx_scalar left);
LAYX_EXPORT void layx_set_right(layx_context *ctx, layx_id item, layx_scalar right);
LAYX_EXPORT void layx_set_top(layx_context *ctx, layx_id item, layx_scalar top);
LAYX_EXPORT void layx_set_bottom(layx_context *ctx, layx_id item, layx_scalar bottom);

// Flex item properties
LAYX_EXPORT void layx_set_flex_grow(layx_context *ctx, layx_id item, layx_scalar grow);
LAYX_EXPORT void layx_set_flex_shrink(layx_context *ctx, layx_id item, layx_scalar shrink);
LAYX_EXPORT void layx_set_flex_basis(layx_context *ctx, layx_id item, layx_scalar basis);
LAYX_EXPORT void layx_set_flex_properties(layx_context *ctx, layx_id item,
                                         layx_scalar grow, layx_scalar shrink, layx_scalar basis);

// Align self
LAYX_EXPORT void layx_set_align_self(layx_context *ctx, layx_id item, layx_align_self align);

// Box model properties
LAYX_EXPORT void layx_set_margin(layx_context *ctx, layx_id item, layx_scalar value);
LAYX_EXPORT void layx_set_margin_top(layx_context *ctx, layx_id item, layx_scalar top);
LAYX_EXPORT void layx_set_margin_right(layx_context *ctx, layx_id item, layx_scalar right);
LAYX_EXPORT void layx_set_margin_bottom(layx_context *ctx, layx_id item, layx_scalar bottom);
LAYX_EXPORT void layx_set_margin_left(layx_context *ctx, layx_id item, layx_scalar left);
LAYX_EXPORT void layx_set_margin_trbl(layx_context *ctx, layx_id item,
                                layx_scalar top, layx_scalar right,
                                layx_scalar bottom, layx_scalar left);
LAYX_EXPORT void layx_set_margin_ltrb(layx_context *ctx, layx_id item,
                                layx_scalar left, layx_scalar top,
                                layx_scalar right, layx_scalar bottom);

LAYX_EXPORT void layx_set_padding(layx_context *ctx, layx_id item, layx_scalar value);
LAYX_EXPORT void layx_set_padding_top(layx_context *ctx, layx_id item, layx_scalar top);
LAYX_EXPORT void layx_set_padding_right(layx_context *ctx, layx_id item, layx_scalar right);
LAYX_EXPORT void layx_set_padding_bottom(layx_context *ctx, layx_id item, layx_scalar bottom);
LAYX_EXPORT void layx_set_padding_left(layx_context *ctx, layx_id item, layx_scalar left);
LAYX_EXPORT void layx_set_padding_trbl(layx_context *ctx, layx_id item,
                                 layx_scalar top, layx_scalar right,
                                 layx_scalar bottom, layx_scalar left);
LAYX_EXPORT void layx_set_padding_ltrb(layx_context *ctx, layx_id item,
                                 layx_scalar left, layx_scalar top,
                                 layx_scalar right, layx_scalar bottom);

LAYX_EXPORT void layx_set_border(layx_context *ctx, layx_id item, layx_scalar value);
LAYX_EXPORT void layx_set_border_top(layx_context *ctx, layx_id item, layx_scalar top);
LAYX_EXPORT void layx_set_border_right(layx_context *ctx, layx_id item, layx_scalar right);
LAYX_EXPORT void layx_set_border_bottom(layx_context *ctx, layx_id item, layx_scalar bottom);
LAYX_EXPORT void layx_set_border_left(layx_context *ctx, layx_id item, layx_scalar left);
LAYX_EXPORT void layx_set_border_trbl(layx_context *ctx, layx_id item,
                                 layx_scalar top, layx_scalar right,
                                 layx_scalar bottom, layx_scalar left);
LAYX_EXPORT void layx_set_border_ltrb(layx_context *ctx, layx_id item,
                                 layx_scalar left, layx_scalar top,
                                 layx_scalar right, layx_scalar bottom);


// Getters for box model
LAYX_EXPORT void layx_get_margin_ltrb(layx_context *ctx, layx_id item, layx_scalar *left, layx_scalar *top, layx_scalar *right, layx_scalar *bottom);
LAYX_EXPORT void layx_get_padding_ltrb(layx_context *ctx, layx_id item, layx_scalar *left, layx_scalar *top, layx_scalar *right, layx_scalar *bottom);
LAYX_EXPORT void layx_get_border_ltrb(layx_context *ctx, layx_id item, layx_scalar *left, layx_scalar *top, layx_scalar *right, layx_scalar *bottom);

// Style application
LAYX_EXPORT void layx_style_reset(layx_style *style);
LAYX_EXPORT void layx_apply_style(layx_context *ctx, layx_id item, const layx_style *style);
LAYX_EXPORT layx_id layx_create_item_with_style(layx_context *ctx, const layx_style *style);

// Inline helpers
LAYX_STATIC_INLINE layx_vec4 layx_vec4_xyzw(layx_scalar x, layx_scalar y, layx_scalar z, layx_scalar w)
{
#if (defined(__GNUC__) || defined(__clang__)) && !defined(__cplusplus)
    return (layx_vec4){x, y, z, w};
#else
    layx_vec4 result;
    result[0] = x;
    result[1] = y;
    result[2] = z;
    result[3] = w;
    return result;
#endif
}

LAYX_STATIC_INLINE layx_item_t *layx_get_item(const layx_context *ctx, layx_id id)
{
    LAYX_ASSERT(id != LAYX_INVALID_ID && id < ctx->count);
    return ctx->items + id;
}

LAYX_STATIC_INLINE layx_id layx_first_child(const layx_context *ctx, layx_id id)
{
    const layx_item_t *pitem = layx_get_item(ctx, id);
    return pitem->first_child;
}

LAYX_STATIC_INLINE layx_id layx_next_sibling(const layx_context *ctx, layx_id id)
{
    const layx_item_t *pitem = layx_get_item(ctx, id);
    return pitem->next_sibling;
}

LAYX_STATIC_INLINE layx_vec4 layx_get_rect(const layx_context *ctx, layx_id id)
{
    LAYX_ASSERT(id != LAYX_INVALID_ID && id < ctx->count);
    return ctx->rects[id];
}

LAYX_STATIC_INLINE void layx_get_rect_xywh(
        const layx_context *ctx, layx_id id,
        layx_scalar *x, layx_scalar *y, layx_scalar *width, layx_scalar *height)
{
    LAYX_ASSERT(id != LAYX_INVALID_ID && id < ctx->count);
    layx_vec4 rect = ctx->rects[id];
    if (x) *x = rect[0];
    if (y) *y = rect[1];
    if (width) *width = rect[2];
    if (height) *height = rect[3];
}
LAYX_STATIC_INLINE int layx_hit_test(const layx_context *ctx, layx_id id, layx_scalar x, layx_scalar y){
    layx_vec4 rect = ctx->rects[id];
    return x >= rect[0] && x < rect[0] + rect[2] && y >= rect[1] && y < rect[1] + rect[3];
}
LAYX_STATIC_INLINE void layx_get_content_rect_xywh(
        const layx_context *ctx, layx_id id,
        layx_scalar *x, layx_scalar *y, layx_scalar *width, layx_scalar *height)
{
    LAYX_ASSERT(id != LAYX_INVALID_ID && id < ctx->count);
    layx_vec4 rect = ctx->rects[id];
    layx_vec4 padding = ctx->items[id].padding;
    layx_vec4 borders = ctx->items[id].border;
    
    *x = rect[0] + padding[0] + borders[0];
    *y = rect[1] + padding[1] + borders[1];
    *width = rect[2] - (padding[0] + padding[2] + borders[0] + borders[2]);
    *height = rect[3] - (padding[1] + padding[3] + borders[1] + borders[3]);
}


// Scroll functions (implemented in scroll_utils.c)
LAYX_EXPORT void layx_set_overflow_x(layx_context *ctx, layx_id item, layx_overflow overflow);
LAYX_EXPORT void layx_set_overflow_y(layx_context *ctx, layx_id item, layx_overflow overflow);
LAYX_EXPORT void layx_set_overflow(layx_context *ctx, layx_id item, layx_overflow overflow);
LAYX_EXPORT const char* layx_get_overflow_string(layx_overflow overflow);
LAYX_EXPORT void layx_scroll_to(layx_context *ctx, layx_id item, layx_scalar x, layx_scalar y);
LAYX_EXPORT void layx_scroll_by(layx_context *ctx, layx_id item, layx_scalar dx, layx_scalar dy);
LAYX_EXPORT void layx_get_visible_content_rect(layx_context *ctx, layx_id item, 
                                   layx_scalar *visible_left, layx_scalar *visible_top,
                                   layx_scalar *visible_right, layx_scalar *visible_bottom);
LAYX_EXPORT int layx_has_vertical_scrollbar(layx_context *ctx, layx_id item);
LAYX_EXPORT int layx_has_horizontal_scrollbar(layx_context *ctx, layx_id item);


LAYX_EXPORT void layx_get_scroll_offset(layx_context *ctx, layx_id item, layx_vec2 *offset);
void layx_get_scroll_offset_xy(layx_context *ctx, layx_id item, layx_scalar *x, layx_scalar *y);
LAYX_EXPORT void layx_get_scroll_max(layx_context *ctx, layx_id item, layx_vec2 *max);
LAYX_EXPORT void layx_get_content_size(layx_context *ctx, layx_id item, layx_vec2 *size);

// Web标准 API 命名 (遵循浏览器 DOM 属性规范)
//
// clientWidth/clientHeight: 绘制区域（内容+内边距，无滚动条）
//   相当于: rect.width - 边框 - 滚动条宽度
//   用途: 获取元素的实际可见区域大小
LAYX_EXPORT layx_scalar layx_get_client_width(layx_context *ctx, layx_id item);
LAYX_EXPORT layx_scalar layx_get_client_height(layx_context *ctx, layx_id item);
LAYX_EXPORT void layx_get_client_size(layx_context *ctx, layx_id item, layx_vec2 *size);
LAYX_EXPORT void layx_get_client_size_wh(layx_context *ctx, layx_id item, layx_scalar *width, layx_scalar *size);

// scrollWidth/scrollHeight: 内容区域（实际内容大小）
//   相当于: layx_get_content_size() 返回的值
//   用途: 获取元素的完整内容尺寸（可能超出可见区域）
LAYX_EXPORT layx_scalar layx_get_scroll_width(layx_context *ctx, layx_id item);
LAYX_EXPORT layx_scalar layx_get_scroll_height(layx_context *ctx, layx_id item);

// offsetWidth/offsetHeight: 视口（边框+内边距+内容，无margin）
//   相当于: ctx.rects[item].width 或 ctx.rects[item].height
//   用途: 获取元素的总占位大小（不含外边距）
LAYX_EXPORT layx_scalar layx_get_offset_width(layx_context *ctx, layx_id item);
LAYX_EXPORT layx_scalar layx_get_offset_height(layx_context *ctx, layx_id item);

// Debug functions
LAYX_EXPORT const char* layx_get_layout_properties_string(layx_context *ctx, layx_id item);
LAYX_EXPORT const char* layx_get_item_alignment_string(layx_context *ctx, layx_id item);

// Text measurement functions
LAYX_EXPORT void layx_set_mesure_fn(layx_context *ctx, layx_id item_id, layx_measure_text_fn fn, void *user_data);

LAYX_EXPORT void layx_dump_tree(layx_context *layout_ctx, layx_id layout_id, int indent);
#undef LAYX_EXPORT
#undef LAYX_STATIC_INLINE

#endif // LAYX_INCLUDE_HEADER
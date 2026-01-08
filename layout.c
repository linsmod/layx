#include "layout.h"
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

// Users of this library can define LAY_REALLOC to use a custom (re)allocator
// instead of stdlib's realloc. It should have the same behavior as realloc --
// first parameter type is a void pointer, and its value is either a null
// pointer or an existing pointer. The second parameter is a size_t of the new
// desired size. The buffer contents should be preserved across reallocations.
//
// And, if you define LAY_REALLOC, you will also need to define LAY_FREE, which
// should have the same behavior as free.
#ifndef LAY_REALLOC
#include <stdlib.h>
#define LAY_REALLOC(_block, _size) realloc(_block, _size)
#define LAY_FREE(_block) free(_block)
#endif

// Like the LAY_REALLOC define, LAY_MEMSET can be used for a custom memset.
// Otherwise, the memset from string.h will be used.
#ifndef LAY_MEMSET
#include <string.h>
#define LAY_MEMSET(_dst, _val, _size) memset(_dst, _val, _size)
#endif

#if defined(__GNUC__) || defined(__clang__)
#define LAY_FORCE_INLINE __attribute__((always_inline)) inline
#ifdef __cplusplus
#define LAY_RESTRICT __restrict
#else
#define LAY_RESTRICT restrict
#endif // __cplusplus
#elif defined(_MSC_VER)
#define LAY_FORCE_INLINE __forceinline
#define LAY_RESTRICT __restrict
#else
#define LAY_FORCE_INLINE inline
#ifdef __cplusplus
#define LAY_RESTRICT
#else
#define LAY_RESTRICT restrict
#endif // __cplusplus
#endif

// Useful math utilities
static LAY_FORCE_INLINE lay_scalar lay_scalar_max(lay_scalar a, lay_scalar b)
{ return a > b ? a : b; }
static LAY_FORCE_INLINE lay_scalar lay_scalar_min(lay_scalar a, lay_scalar b)
{ return a < b ? a : b; }
static LAY_FORCE_INLINE float lay_float_max(float a, float b)
{ return a > b ? a : b; }
static LAY_FORCE_INLINE float lay_float_min(float a, float b)
{ return a < b ? a : b; }

// New function to set container with individual properties
void lay_set_container(
    lay_context *ctx, 
    lay_id item, 
    lay_flex_direction direction,
    lay_layout_model model,
    lay_flex_wrap wrap,
    lay_justify_content justify,
    lay_align_items align_items,
    lay_align_content align_content) {
    
    uint32_t flags = lay_make_box_flags(direction, model, wrap, justify, align_items, align_content);
    lay_set_contain(ctx, item, flags);
}

// Helper function to determine if layout uses flexbox model
static LAY_FORCE_INLINE bool lay_is_flex_container(uint32_t flags) {
    return lay_get_layout_model(flags) == LAY_MODEL_FLEX;
}

// Helper function to get direction dimension (0 for horizontal, 1 for vertical)
static LAY_FORCE_INLINE int lay_get_direction_dim(uint32_t flags) {
    lay_flex_direction dir = lay_get_flex_direction(flags);
    return (dir == LAY_ROW || dir == LAY_ROW_REVERSE) ? 0 : 1;
}

void lay_init_context(lay_context *ctx)
{
    ctx->capacity = 0;
    ctx->count = 0;
    ctx->items = NULL;
    ctx->rects = NULL;
}

void lay_reserve_items_capacity(lay_context *ctx, lay_id count)
{
    if (count >= ctx->capacity) {
        ctx->capacity = count;
        const size_t item_size = sizeof(lay_item_t) + sizeof(lay_vec4);
        ctx->items = (lay_item_t*)LAY_REALLOC(ctx->items, ctx->capacity * item_size);
        const lay_item_t *past_last = ctx->items + ctx->capacity;
        ctx->rects = (lay_vec4*)past_last;
    }
}

void lay_destroy_context(lay_context *ctx)
{
    if (ctx->items != NULL) {
        LAY_FREE(ctx->items);
        ctx->items = NULL;
        ctx->rects = NULL;
    }
}

void lay_reset_context(lay_context *ctx)
{ ctx->count = 0; }

static void lay_calc_size(lay_context *ctx, lay_id item, int dim);
static void lay_arrange(lay_context *ctx, lay_id item, int dim);

void lay_run_context(lay_context *ctx)
{
    LAY_ASSERT(ctx != NULL);

    if (ctx->count > 0) {
        lay_run_item(ctx, 0);
    }
}

void lay_run_item(lay_context *ctx, lay_id item)
{
    LAY_ASSERT(ctx != NULL);

    lay_calc_size(ctx, item, 0);
    lay_arrange(ctx, item, 0);
    lay_calc_size(ctx, item, 1);
    lay_arrange(ctx, item, 1);
}

void lay_clear_item_break(lay_context *ctx, lay_id item)
{
    LAY_ASSERT(ctx != NULL);
    lay_item_t *pitem = lay_get_item(ctx, item);
    pitem->flags = pitem->flags & ~(uint32_t)LAY_BREAK;
}

lay_id lay_items_count(lay_context *ctx)
{
    LAY_ASSERT(ctx != NULL);
    return ctx->count;
}

lay_id lay_items_capacity(lay_context *ctx)
{
    LAY_ASSERT(ctx != NULL);
    return ctx->capacity;
}

lay_id lay_item(lay_context *ctx)
{
    lay_id idx = ctx->count++;

    if (idx >= ctx->capacity) {
        ctx->capacity = ctx->capacity < 1 ? 32 : (ctx->capacity * 4);
        const size_t item_size = sizeof(lay_item_t) + sizeof(lay_vec4);
        ctx->items = (lay_item_t*)LAY_REALLOC(ctx->items, ctx->capacity * item_size);
        const lay_item_t *past_last = ctx->items + ctx->capacity;
        ctx->rects = (lay_vec4*)past_last;
    }

    lay_item_t *item = lay_get_item(ctx, idx);
    // We can either do this here, or when creating/resetting buffer
    LAY_MEMSET(item, 0, sizeof(lay_item_t));
    item->first_child = LAY_INVALID_ID;
    item->next_sibling = LAY_INVALID_ID;
    // hmm
    LAY_MEMSET(&ctx->rects[idx], 0, sizeof(lay_vec4));
    return idx;
}

static LAY_FORCE_INLINE
void lay_append_by_ptr(
        lay_item_t *LAY_RESTRICT pearlier,
        lay_id later, lay_item_t *LAY_RESTRICT plater)
{
    plater->next_sibling = pearlier->next_sibling;
    plater->flags |= LAY_ITEM_INSERTED;
    pearlier->next_sibling = later;
}

lay_id lay_last_child(const lay_context *ctx, lay_id parent)
{
    lay_item_t *pparent = lay_get_item(ctx, parent);
    lay_id child = pparent->first_child;
    if (child == LAY_INVALID_ID) return LAY_INVALID_ID;
    lay_item_t *pchild = lay_get_item(ctx, child);
    lay_id result = child;
    for (;;) {
        lay_id next = pchild->next_sibling;
        if (next == LAY_INVALID_ID) break;
        result = next;
        pchild = lay_get_item(ctx, next);
    }
    return result;
}

void lay_append(lay_context *ctx, lay_id earlier, lay_id later)
{
    LAY_ASSERT(later != 0); // Must not be root item
    LAY_ASSERT(earlier != later); // Must not be same item id
    lay_item_t *LAY_RESTRICT pearlier = lay_get_item(ctx, earlier);
    lay_item_t *LAY_RESTRICT plater = lay_get_item(ctx, later);
    lay_append_by_ptr(pearlier, later, plater);
}

int lay_isinserted(lay_context *ctx, lay_id child){
    LAY_ASSERT(child != 0); // Must not be root item
    lay_item_t *LAY_RESTRICT pchild = lay_get_item(ctx, child);
    return pchild->flags & LAY_ITEM_INSERTED;
}

void lay_insert(lay_context *ctx, lay_id parent, lay_id child)
{
    LAY_ASSERT(child != 0); // Must not be root item
    LAY_ASSERT(parent != child); // Must not be same item id
    lay_item_t *LAY_RESTRICT pparent = lay_get_item(ctx, parent);
    lay_item_t *LAY_RESTRICT pchild = lay_get_item(ctx, child);
    LAY_ASSERT(!(pchild->flags & LAY_ITEM_INSERTED));
    // Parent has no existing children, make inserted item the first child.
    if (pparent->first_child == LAY_INVALID_ID) {
        pparent->first_child = child;
        pchild->flags |= LAY_ITEM_INSERTED;
    // Parent has existing items, iterate to find the last child and append the
    // inserted item after it.
    } else {
        lay_id next = pparent->first_child;
        lay_item_t *LAY_RESTRICT pnext = lay_get_item(ctx, next);
        for (;;) {
            next = pnext->next_sibling;
            if (next == LAY_INVALID_ID) break;
            pnext = lay_get_item(ctx, next);
        }
        lay_append_by_ptr(pnext, child, pchild);
    }
}

void lay_push(lay_context *ctx, lay_id parent, lay_id new_child)
{
    LAY_ASSERT(new_child != 0); // Must not be root item
    LAY_ASSERT(parent != new_child); // Must not be same item id
    lay_item_t *LAY_RESTRICT pparent = lay_get_item(ctx, parent);
    lay_id old_child = pparent->first_child;
    lay_item_t *LAY_RESTRICT pchild = lay_get_item(ctx, new_child);
    LAY_ASSERT(!(pchild->flags & LAY_ITEM_INSERTED));
    pparent->first_child = new_child;
    pchild->flags |= LAY_ITEM_INSERTED;
    pchild->next_sibling = old_child;
}

lay_vec2 lay_get_size(lay_context *ctx, lay_id item)
{
    lay_item_t *pitem = lay_get_item(ctx, item);
    return pitem->size;
}

void lay_get_size_xy(
        lay_context *ctx, lay_id item,
        lay_scalar *x, lay_scalar *y)
{
    lay_item_t *pitem = lay_get_item(ctx, item);
    lay_vec2 size = pitem->size;
    *x = size[0];
    *y = size[1];
}

void lay_set_size(lay_context *ctx, lay_id item, lay_vec2 size)
{
    lay_item_t *pitem = lay_get_item(ctx, item);
    pitem->size = size;
    uint32_t flags = pitem->flags;
    if (size[0] == 0)
        flags &= ~(uint32_t)LAY_ITEM_HFIXED;
    else
        flags |= LAY_ITEM_HFIXED;
    if (size[1] == 0)
        flags &= ~(uint32_t)LAY_ITEM_VFIXED;
    else
        flags |= LAY_ITEM_VFIXED;
    pitem->flags = flags;
}

void lay_set_size_xy(
        lay_context *ctx, lay_id item,
        lay_scalar width, lay_scalar height)
{
    lay_item_t *pitem = lay_get_item(ctx, item);
    pitem->size[0] = width;
    pitem->size[1] = height;
    // Kinda redundant, whatever
    uint32_t flags = pitem->flags;
    if (width == 0)
        flags &= ~(uint32_t)LAY_ITEM_HFIXED;
    else
        flags |= LAY_ITEM_HFIXED;
    if (height == 0)
        flags &= ~(uint32_t)LAY_ITEM_VFIXED;
    else
        flags |= LAY_ITEM_VFIXED;
    pitem->flags = flags;
}

void lay_set_behave(lay_context *ctx, lay_id item, uint32_t flags)
{
    LAY_ASSERT((flags & LAY_ITEM_LAYOUT_MASK) == flags);
    lay_item_t *pitem = lay_get_item(ctx, item);
    pitem->flags = (pitem->flags & ~(uint32_t)LAY_ITEM_LAYOUT_MASK) | flags;
}

const char* lay_get_behave_str(lay_context *ctx, lay_id item)
{
    lay_item_t *pitem = lay_get_item(ctx, item);
    uint32_t flags = pitem->flags & LAY_ITEM_LAYOUT_MASK;

    static char buf[128];
    int len = 0;
    int first = 1;  // 用于判断是否是第一个项

    if (flags & LAY_ITEM_HFIXED) {
        len += snprintf(buf + len, sizeof(buf) - len, "%sHFIXED", first ? "" : "|");
        first = 0;
    }
    if (flags & LAY_ITEM_VFIXED) {
        len += snprintf(buf + len, sizeof(buf) - len, "%sVFIXED", first ? "" : "|");
        first = 0;
    }
    if (flags & LAY_HFILL) {
        len += snprintf(buf + len, sizeof(buf) - len, "%sHFILL", first ? "" : "|");
        first = 0;
    }
    if (flags & LAY_VFILL) {
        len += snprintf(buf + len, sizeof(buf) - len, "%sVFILL", first ? "" : "|");
        first = 0;
    }
    if (flags & LAY_LEFT) {
        len += snprintf(buf + len, sizeof(buf) - len, "%sLEFT", first ? "" : "|");
        first = 0;
    }
    if (flags & LAY_TOP) {
        len += snprintf(buf + len, sizeof(buf) - len, "%sTOP", first ? "" : "|");
        first = 0;
    }
    if (flags & LAY_HCENTER) {
        len += snprintf(buf + len, sizeof(buf) - len, "%sHCENTER", first ? "" : "|");
        first = 0;
    }
    if (flags & LAY_VCENTER) {
        len += snprintf(buf + len, sizeof(buf) - len, "%sVCENTER", first ? "" : "|");
        first = 0;
    }
    if (flags & LAY_RIGHT) {
        len += snprintf(buf + len, sizeof(buf) - len, "%sRIGHT", first ? "" : "|");
        first = 0;
    }
    if (flags & LAY_BOTTOM) {
        len += snprintf(buf + len, sizeof(buf) - len, "%sBOTTOM", first ? "" : "|");
        first = 0;
    }

    if (len == 0) return "default";
    return buf;
}

// Updated to use new property masks
void lay_set_contain(lay_context *ctx, lay_id item, uint32_t flags)
{
    LAY_ASSERT((flags & (LAY_FLEX_DIRECTION_MASK | LAY_LAYOUT_MODEL_MASK | LAY_FLEX_WRAP_MASK |
                        LAY_JUSTIFY_CONTENT_MASK | LAY_ALIGN_ITEMS_MASK | LAY_ALIGN_CONTENT_MASK)) == flags);
    lay_item_t *pitem = lay_get_item(ctx, item);
    pitem->flags = (pitem->flags & ~(LAY_FLEX_DIRECTION_MASK | LAY_LAYOUT_MODEL_MASK | LAY_FLEX_WRAP_MASK |
                                    LAY_JUSTIFY_CONTENT_MASK | LAY_ALIGN_ITEMS_MASK | LAY_ALIGN_CONTENT_MASK)) | flags;
}

uint32_t lay_get_contain(lay_context *ctx, lay_id item)
{
    lay_item_t *pitem = lay_get_item(ctx, item);
    return pitem->flags & (LAY_FLEX_DIRECTION_MASK | LAY_LAYOUT_MODEL_MASK | LAY_FLEX_WRAP_MASK |
                          LAY_JUSTIFY_CONTENT_MASK | LAY_ALIGN_ITEMS_MASK | LAY_ALIGN_CONTENT_MASK);
}

const char* lay_get_contain_str(lay_context *ctx, lay_id item)
{
    lay_item_t *pitem = lay_get_item(ctx, item);
    uint32_t flags = pitem->flags;
    
    static char buf[256];
    int len = 0;
    
    // Direction
    lay_flex_direction dir = lay_get_flex_direction(flags);
    switch (dir) {
        case LAY_ROW: len += snprintf(buf + len, sizeof(buf) - len, "ROW"); break;
        case LAY_COLUMN: len += snprintf(buf + len, sizeof(buf) - len, "COLUMN"); break;
        case LAY_ROW_REVERSE: len += snprintf(buf + len, sizeof(buf) - len, "ROW_REVERSE"); break;
        case LAY_COLUMN_REVERSE: len += snprintf(buf + len, sizeof(buf) - len, "COLUMN_REVERSE"); break;
    }
    
    // Model
    lay_layout_model model = lay_get_layout_model(flags);
    if (model == LAY_MODEL_FLEX) {
        len += snprintf(buf + len, sizeof(buf) - len, "|FLEX");
    } else {
        len += snprintf(buf + len, sizeof(buf) - len, "|LAYOUT");
    }
    
    // Wrap
    lay_flex_wrap wrap = lay_get_flex_wrap(flags);
    switch (wrap) {
        case LAY_WRAP: len += snprintf(buf + len, sizeof(buf) - len, "|WRAP"); break;
        case LAY_WRAP_REVERSE: len += snprintf(buf + len, sizeof(buf) - len, "|WRAP_REVERSE"); break;
        case LAY_WRAP_NO: break; // Default, don't add
    }
    
    // Justify content
    lay_justify_content justify = lay_get_justify_content(flags);
    switch (justify) {
        case LAY_JUSTIFY_CENTER: len += snprintf(buf + len, sizeof(buf) - len, "|JUSTIFY_CENTER"); break;
        case LAY_JUSTIFY_END: len += snprintf(buf + len, sizeof(buf) - len, "|JUSTIFY_END"); break;
        case LAY_JUSTIFY_SPACE_BETWEEN: len += snprintf(buf + len, sizeof(buf) - len, "|JUSTIFY_SPACE_BETWEEN"); break;
        case LAY_JUSTIFY_SPACE_AROUND: len += snprintf(buf + len, sizeof(buf) - len, "|JUSTIFY_SPACE_AROUND"); break;
        case LAY_JUSTIFY_SPACE_EVENLY: len += snprintf(buf + len, sizeof(buf) - len, "|JUSTIFY_SPACE_EVENLY"); break;
        case LAY_JUSTIFY_START: break; // Default, don't add
    }
    
    // Align items
    lay_align_items align_items = lay_get_align_items(flags);
    switch (align_items) {
        case LAY_ALIGN_CENTER: len += snprintf(buf + len, sizeof(buf) - len, "|ALIGN_CENTER"); break;
        case LAY_ALIGN_END: len += snprintf(buf + len, sizeof(buf) - len, "|ALIGN_END"); break;
        case LAY_ALIGN_BASELINE: len += snprintf(buf + len, sizeof(buf) - len, "|ALIGN_BASELINE"); break;
        case LAY_ALIGN_START: break; // Default, don't add
        case LAY_ALIGN_STRETCH: break; // Default, don't add
    }
    
    // Align content
    lay_align_content align_content = lay_get_align_content(flags);
    switch (align_content) {
        case LAY_ALIGN_CONTENT_CENTER: len += snprintf(buf + len, sizeof(buf) - len, "|ALIGN_CONTENT_CENTER"); break;
        case LAY_ALIGN_CONTENT_END: len += snprintf(buf + len, sizeof(buf) - len, "|ALIGN_CONTENT_END"); break;
        case LAY_ALIGN_CONTENT_SPACE_BETWEEN: len += snprintf(buf + len, sizeof(buf) - len, "|ALIGN_CONTENT_SPACE_BETWEEN"); break;
        case LAY_ALIGN_CONTENT_SPACE_AROUND: len += snprintf(buf + len, sizeof(buf) - len, "|ALIGN_CONTENT_SPACE_AROUND"); break;
        case LAY_ALIGN_CONTENT_START: break; // Default, don't add
        case LAY_ALIGN_CONTENT_STRETCH: break; // Default, don't add
    }
    
    if (len == 0) return "ROW|LAYOUT|ALIGN_STRETCH|ALIGN_CONTENT_STRETCH";
    return buf;
}

void lay_set_margins(lay_context *ctx, lay_id item, lay_vec4 ltrb)
{
    lay_item_t *pitem = lay_get_item(ctx, item);
    pitem->margins = ltrb;
}

void lay_set_margins_ltrb(
        lay_context *ctx, lay_id item,
        lay_scalar l, lay_scalar t, lay_scalar r, lay_scalar b)
{
    lay_item_t *pitem = lay_get_item(ctx, item);
    pitem->margins[0] = l;
    pitem->margins[1] = t;
    pitem->margins[2] = r;
    pitem->margins[3] = b;
}

lay_vec4 lay_get_margins(lay_context *ctx, lay_id item)
{ return lay_get_item(ctx, item)->margins; }

void lay_get_margins_ltrb(
        lay_context *ctx, lay_id item,
        lay_scalar *l, lay_scalar *t, lay_scalar *r, lay_scalar *b)
{
    lay_item_t *pitem = lay_get_item(ctx, item);
    lay_vec4 margins = pitem->margins;
    *l = margins[0];
    *t = margins[1];
    *r = margins[2];
    *b = margins[3];
}

void lay_set_padding(lay_context *ctx, lay_id item, lay_vec4 ltrb)
{
    lay_item_t *pitem = lay_get_item(ctx, item);
    pitem->padding = ltrb;
}

void lay_set_padding_ltrb(
        lay_context *ctx, lay_id item,
        lay_scalar l, lay_scalar t, lay_scalar r, lay_scalar b)
{
    lay_item_t *pitem = lay_get_item(ctx, item);
    pitem->padding[0] = l;
    pitem->padding[1] = t;
    pitem->padding[2] = r;
    pitem->padding[3] = b;
}

lay_vec4 lay_get_padding(lay_context *ctx, lay_id item)
{ return lay_get_item(ctx, item)->padding; }

void lay_get_padding_ltrb(
        lay_context *ctx, lay_id item,
        lay_scalar *l, lay_scalar *t, lay_scalar *r, lay_scalar *b)
{
    lay_item_t *pitem = lay_get_item(ctx, item);
    lay_vec4 padding = pitem->padding;
    *l = padding[0];
    *t = padding[1];
    *r = padding[2];
    *b = padding[3];
}

void lay_set_border(lay_context *ctx, lay_id item, lay_vec4 ltrb)
{
    lay_item_t *pitem = lay_get_item(ctx, item);
    pitem->border = ltrb;
}

void lay_set_border_ltrb(
        lay_context *ctx, lay_id item,
        lay_scalar l, lay_scalar t, lay_scalar r, lay_scalar b)
{
    lay_item_t *pitem = lay_get_item(ctx, item);
    pitem->border[0] = l;
    pitem->border[1] = t;
    pitem->border[2] = r;
    pitem->border[3] = b;
}

lay_vec4 lay_get_border(lay_context *ctx, lay_id item)
{ return lay_get_item(ctx, item)->border; }

void lay_get_border_ltrb(
        lay_context *ctx, lay_id item,
        lay_scalar *l, lay_scalar *t, lay_scalar *r, lay_scalar *b)
{
    lay_item_t *pitem = lay_get_item(ctx, item);
    lay_vec4 border = pitem->border;
    *l = border[0];
    *t = border[1];
    *r = border[2];
    *b = border[3];
}

// Helper function to get the internal space available for children
// Returns the space available after subtracting padding and border
static LAY_FORCE_INLINE
lay_scalar lay_get_internal_space(
        lay_context *ctx, lay_id item, int dim)
{
    lay_item_t *pitem = lay_get_item(ctx, item);
    lay_vec4 rect = ctx->rects[item];
    // Internal space = content_size - padding_start - border_start - padding_end - border_end
    return rect[2 + dim] - pitem->padding[dim] - pitem->border[dim] 
                           - pitem->padding[dim + 2] - pitem->border[dim + 2];
}

// Helper function to get the offset where children should be positioned
// This is the starting position of the content area
static LAY_FORCE_INLINE
lay_scalar lay_get_content_offset(
        lay_context *ctx, lay_id item, int dim)
{
    lay_item_t *pitem = lay_get_item(ctx, item);
    lay_vec4 rect = ctx->rects[item];
    // Content offset = position + margin_start + padding_start + border_start
    return rect[dim] + pitem->margins[dim] + pitem->padding[dim] + pitem->border[dim];
}

// TODO restrict item ptrs correctly
static LAY_FORCE_INLINE
lay_scalar lay_calc_overlayed_size(
        lay_context *ctx, lay_id item, int dim)
{
    const int wdim = dim + 2;
    lay_item_t *LAY_RESTRICT pitem = lay_get_item(ctx, item);
    lay_scalar need_size = 0;
    lay_id child = pitem->first_child;
    while (child != LAY_INVALID_ID) {
        lay_item_t *pchild = lay_get_item(ctx, child);
        lay_vec4 rect = ctx->rects[child];
        // width = start margin + calculated width + end margin
        lay_scalar child_size = rect[dim] + rect[2 + dim] + pchild->margins[wdim];
        need_size = lay_scalar_max(need_size, child_size);
        child = pchild->next_sibling;
    }
    return need_size;
}

static LAY_FORCE_INLINE
lay_scalar lay_calc_stacked_size(
        lay_context *ctx, lay_id item, int dim)
{
    const int wdim = dim + 2;
    lay_item_t *LAY_RESTRICT pitem = lay_get_item(ctx, item);
    lay_scalar need_size = 0;
    lay_id child = pitem->first_child;
    while (child != LAY_INVALID_ID) {
        lay_item_t *pchild = lay_get_item(ctx, child);
        lay_vec4 rect = ctx->rects[child];
        need_size += rect[dim] + rect[2 + dim] + pchild->margins[wdim];
        child = pchild->next_sibling;
    }
    return need_size;
}

static LAY_FORCE_INLINE
lay_scalar lay_calc_wrapped_overlayed_size(
        lay_context *ctx, lay_id item, int dim)
{
    const int wdim = dim + 2;
    lay_item_t *LAY_RESTRICT pitem = lay_get_item(ctx, item);
    lay_scalar need_size = 0;
    lay_scalar need_size2 = 0;
    lay_id child = pitem->first_child;
    while (child != LAY_INVALID_ID) {
        lay_item_t *pchild = lay_get_item(ctx, child);
        lay_vec4 rect = ctx->rects[child];
        if (pchild->flags & LAY_BREAK) {
            need_size2 += need_size;
            need_size = 0;
        }
        lay_scalar child_size = rect[dim] + rect[2 + dim] + pchild->margins[wdim];
        need_size = lay_scalar_max(need_size, child_size);
        child = pchild->next_sibling;
    }
    return need_size2 + need_size;
}

// Equivalent to uiComputeWrappedStackedSize
static LAY_FORCE_INLINE
lay_scalar lay_calc_wrapped_stacked_size(
        lay_context *ctx, lay_id item, int dim)
{
    const int wdim = dim + 2;
    lay_item_t *LAY_RESTRICT pitem = lay_get_item(ctx, item);
    lay_scalar need_size = 0;
    lay_scalar need_size2 = 0;
    lay_id child = pitem->first_child;
    while (child != LAY_INVALID_ID) {
        lay_item_t *pchild = lay_get_item(ctx, child);
        lay_vec4 rect = ctx->rects[child];
        if (pchild->flags & LAY_BREAK) {
            need_size2 = lay_scalar_max(need_size2, need_size);
            need_size = 0;
        }
        need_size += rect[dim] + rect[2 + dim] + pchild->margins[wdim];
        child = pchild->next_sibling;
    }
    return lay_scalar_max(need_size2, need_size);
}

static void lay_calc_size(lay_context *ctx, lay_id item, int dim)
{
    lay_item_t *pitem = lay_get_item(ctx, item);
    uint32_t flags = pitem->flags;

    lay_id child = pitem->first_child;
    while (child != LAY_INVALID_ID) {
        // NOTE: this is recursive and will run out of stack space if items are
        // nested too deeply.
        lay_calc_size(ctx, child, dim);
        lay_item_t *pchild = lay_get_item(ctx, child);
        child = pchild->next_sibling;
    }

    // Set the mutable rect output data to the starting input data
    ctx->rects[item][dim] = pitem->margins[dim];

    // If we have an explicit input size, just set our output size (which other
    // calc_size and arrange procedures will use) to it.
    // Note: The explicit size should be the total size including content + padding + border
    if (pitem->size[dim] != 0) {
        // For fixed size elements, we use the explicit size directly
        ctx->rects[item][2 + dim] = pitem->size[dim];
        return;
    }

    // Calculate our size based on children items. Note that we've already
    // called lay_calc_size on our children at this point.
    lay_scalar cal_size;
    lay_flex_direction direction = lay_get_flex_direction(flags);
    lay_layout_model model = lay_get_layout_model(flags);
    lay_flex_wrap wrap = lay_get_flex_wrap(flags);
    
    if (model == LAY_MODEL_FLEX) {
        bool is_wrapped = (wrap != LAY_WRAP_NO);
        bool is_row_direction = (direction == LAY_ROW || direction == LAY_ROW_REVERSE);
        
        if (is_wrapped) {
            // Wrapped flex container
            if (is_row_direction) {
                // Row wrap: stacked horizontally, wrapped vertically
                if (dim == 0) // horizontal dimension
                    cal_size = lay_calc_wrapped_stacked_size(ctx, item, 0);
                else // vertical dimension
                    cal_size = lay_calc_wrapped_overlayed_size(ctx, item, 1);
            } else {
                // Column wrap: stacked vertically, wrapped horizontally
                if (dim == 1) // vertical dimension
                    cal_size = lay_calc_wrapped_stacked_size(ctx, item, 1);
                else // horizontal dimension
                    cal_size = lay_calc_wrapped_overlayed_size(ctx, item, 0);
            }
        } else {
            // Non-wrapped flex container
            if ((is_row_direction && dim == 0) || (!is_row_direction && dim == 1)) {
                // Main axis: stacked size
                cal_size = lay_calc_stacked_size(ctx, item, dim);
            } else {
                // Cross axis: overlayed size
                cal_size = lay_calc_overlayed_size(ctx, item, dim);
            }
        }
    } else {
        // Layout model (absolute positioning)
        cal_size = lay_calc_overlayed_size(ctx, item, dim);
    }

    // Set our output data size. Will be used by parent calc_size procedures.,
    // and by arrange procedures.
    // The calculated size should include padding + border + content
    cal_size += pitem->padding[dim] + pitem->border[dim] 
             + pitem->padding[dim + 2] + pitem->border[dim + 2];
    ctx->rects[item][2 + dim] = cal_size;
}

static LAY_FORCE_INLINE
void lay_arrange_stacked(
            lay_context *ctx, lay_id item, int dim, bool wrap)
{
    const int wdim = dim + 2;
    lay_item_t *pitem = lay_get_item(ctx, item);

    const uint32_t item_flags = pitem->flags;
    lay_vec4 rect = ctx->rects[item];
    // Use internal space (subtract padding and border)
    lay_scalar space = lay_get_internal_space(ctx, item, dim);
    lay_scalar content_offset = lay_get_content_offset(ctx, item, dim);

    float max_x2 = (float)(content_offset + space);

    lay_id start_child = pitem->first_child;
    while (start_child != LAY_INVALID_ID) {
        lay_scalar used = 0;
        uint32_t count = 0; // count of fillers
        uint32_t squeezed_count = 0; // count of squeezable elements
        uint32_t total = 0;
        bool hardbreak = false;
        // first pass: count items that need to be expanded,
        // and the space that is used
        lay_id child = start_child;
        lay_id end_child = LAY_INVALID_ID;
        while (child != LAY_INVALID_ID) {
            lay_item_t *pchild = lay_get_item(ctx, child);
            const uint32_t child_flags = pchild->flags;
            const uint32_t flags = (child_flags & LAY_ITEM_LAYOUT_MASK) >> dim;
            const uint32_t fflags = (child_flags & LAY_ITEM_FIXED_MASK) >> dim;
            const lay_vec4 child_margins = pchild->margins;
            lay_vec4 child_rect = ctx->rects[child];
            lay_scalar extend = used;
            if ((flags & LAY_HFILL) == LAY_HFILL) {
                ++count;
                extend += child_rect[dim] + child_margins[wdim];
            } else {
                if ((fflags & LAY_ITEM_HFIXED) != LAY_ITEM_HFIXED)
                    ++squeezed_count;
                extend += child_rect[dim] + child_rect[2 + dim] + child_margins[wdim];
            }
            // wrap on end of line or manual flag
            if (wrap && (
                    total && ((extend > space) ||
                    (child_flags & LAY_BREAK)))) {
                end_child = child;
                hardbreak = (child_flags & LAY_BREAK) == LAY_BREAK;
                // add marker for subsequent queries
                pchild->flags = child_flags | LAY_BREAK;
                break;
            } else {
                used = extend;
                child = pchild->next_sibling;
            }
            ++total;
        }

        lay_scalar extra_space = space - used;
        float filler = 0.0f;
        float spacer = 0.0f;
        float extra_margin = 0.0f;
        float eater = 0.0f;

        if (extra_space > 0) {
            if (count > 0)
                filler = (float)extra_space / (float)count;
            else if (total > 0) {
                // Use the new justify content property
                lay_justify_content justify = lay_get_justify_content(item_flags);
                switch (justify) {
                case LAY_JUSTIFY_SPACE_BETWEEN:
                    // justify when not wrapping or not in last line,
                    // or not manually breaking
                    if (!wrap || ((end_child != LAY_INVALID_ID) && !hardbreak))
                        spacer = (float)extra_space / (float)(total - 1);
                    break;
                case LAY_JUSTIFY_START:
                    break;
                case LAY_JUSTIFY_END:
                    extra_margin = extra_space;
                    break;
                case LAY_JUSTIFY_CENTER:
                    extra_margin = extra_space / 2.0f;
                    break;
                case LAY_JUSTIFY_SPACE_AROUND:
                    if (total > 0)
                        spacer = (float)extra_space / (float)total;
                    break;
                case LAY_JUSTIFY_SPACE_EVENLY:
                    if (total > 0)
                        spacer = (float)extra_space / (float)(total + 1);
                    break;
                }
            }
        }
#ifdef LAY_FLOAT
        // In floating point, it's possible to end up with some small negative
        // value for extra_space, while also have a 0.0 squeezed_count. This
        // would cause divide by zero. Instead, we'll check to see if
        // squeezed_count is > 0. I believe this produces the same results as
        // the original oui int-only code. However, I don't have any tests for
        // it, so I'll leave it if-def'd for now.
        else if (!wrap && (squeezed_count > 0))
#else
        // This is the original oui code
        else if (!wrap && (extra_space < 0))
#endif
            eater = (float)extra_space / (float)squeezed_count;

        // distribute width among items
        float x = (float)content_offset;
        float x1;
        // second pass: distribute and rescale
        child = start_child;
        while (child != end_child) {
            lay_scalar ix0, ix1;
            lay_item_t *pchild = lay_get_item(ctx, child);
            const uint32_t child_flags = pchild->flags;
            const uint32_t flags = (child_flags & LAY_ITEM_LAYOUT_MASK) >> dim;
            const uint32_t fflags = (child_flags & LAY_ITEM_FIXED_MASK) >> dim;
            const lay_vec4 child_margins = pchild->margins;
            lay_vec4 child_rect = ctx->rects[child];

            x += (float)child_rect[dim] + extra_margin;
            if ((flags & LAY_HFILL) == LAY_HFILL) // grow
                x1 = x + filler;
            else if ((fflags & LAY_ITEM_HFIXED) == LAY_ITEM_HFIXED)
                x1 = x + (float)child_rect[2 + dim];
            else // squeeze
                x1 = x + lay_float_max(0.0f, (float)child_rect[2 + dim] + eater);

            ix0 = (lay_scalar)x;
            if (wrap)
                ix1 = (lay_scalar)lay_float_min(max_x2 - (float)child_margins[wdim], x1);
            else
                ix1 = (lay_scalar)x1;
            child_rect[dim] = ix0; // pos
            child_rect[dim + 2] = ix1 - ix0; // size
            ctx->rects[child] = child_rect;
            x = x1 + (float)child_margins[wdim];
            child = pchild->next_sibling;
            extra_margin = spacer;
        }

        start_child = end_child;
    }
}

static LAY_FORCE_INLINE
void lay_arrange_overlay(lay_context *ctx, lay_id item, int dim)
{
    const int wdim = dim + 2;
    lay_item_t *pitem = lay_get_item(ctx, item);
    const lay_vec4 rect = ctx->rects[item];
    // Use internal space (subtract padding and border)
    const lay_scalar offset = lay_get_content_offset(ctx, item, dim);
    const lay_scalar space = lay_get_internal_space(ctx, item, dim);
    
    lay_id child = pitem->first_child;
    while (child != LAY_INVALID_ID) {
        lay_item_t *pchild = lay_get_item(ctx, child);
        const uint32_t b_flags = (pchild->flags & LAY_ITEM_LAYOUT_MASK) >> dim;
        const lay_vec4 child_margins = pchild->margins;
        lay_vec4 child_rect = ctx->rects[child];

        if (b_flags & ((dim == 0) ? LAY_HCENTER : LAY_VCENTER)) {
            child_rect[dim] += (space - child_rect[2 + dim]) / 2 - child_margins[wdim];
        } else if (b_flags & ((dim == 0) ? LAY_RIGHT : LAY_BOTTOM)) {
            child_rect[dim] += space - child_rect[2 + dim] - child_margins[dim] - child_margins[wdim];
        } else if (b_flags & ((dim == 0) ? LAY_HFILL : LAY_VFILL)) {
            child_rect[2 + dim] = lay_scalar_max(0, space - child_rect[dim] - child_margins[wdim]);
        }

        child_rect[dim] += offset;
        ctx->rects[child] = child_rect;
        child = pchild->next_sibling;
    }
}

static LAY_FORCE_INLINE
void lay_arrange_overlay_squeezed_range(
        lay_context *ctx, int dim,
        lay_id start_item, lay_id end_item,
        lay_scalar offset, lay_scalar space)
{
    int wdim = dim + 2;
    lay_id item = start_item;
    while (item != end_item) {
        lay_item_t *pitem = lay_get_item(ctx, item);
        const uint32_t b_flags = (pitem->flags & LAY_ITEM_LAYOUT_MASK) >> dim;
        const lay_vec4 margins = pitem->margins;
        lay_vec4 rect = ctx->rects[item];
        lay_scalar min_size = lay_scalar_max(0, space - rect[dim] - margins[wdim]);
        if (b_flags & ((dim == 0) ? LAY_HCENTER : LAY_VCENTER)) {
            rect[2 + dim] = lay_scalar_min(rect[2 + dim], min_size);
            rect[dim] += (space - rect[2 + dim]) / 2 - margins[wdim];
        } else if (b_flags & ((dim == 0) ? LAY_RIGHT : LAY_BOTTOM)) {
            rect[2 + dim] = lay_scalar_min(rect[2 + dim], min_size);
            rect[dim] = space - rect[2 + dim] - margins[wdim];
        } else if (b_flags & ((dim == 0) ? LAY_HFILL : LAY_VFILL)) {
            rect[2 + dim] = min_size;
        } else {
            rect[2 + dim] = lay_scalar_min(rect[2 + dim], min_size);
        }
        rect[dim] += offset;
        ctx->rects[item] = rect;
        item = pitem->next_sibling;
    }
}

static LAY_FORCE_INLINE
lay_scalar lay_arrange_wrapped_overlay_squeezed(
        lay_context *ctx, lay_id item, int dim)
{
    const int wdim = dim + 2;
    lay_item_t *pitem = lay_get_item(ctx, item);
    // Use content offset (start from padding + border)
    lay_scalar offset = lay_get_content_offset(ctx, item, dim);
    lay_scalar need_size = 0;
    lay_id child = pitem->first_child;
    lay_id start_child = child;
    while (child != LAY_INVALID_ID) {
        lay_item_t *pchild = lay_get_item(ctx, child);
        if (pchild->flags & LAY_BREAK) {
            lay_arrange_overlay_squeezed_range(ctx, dim, start_child, child, offset, need_size);
            offset += need_size;
            start_child = child;
            need_size = 0;
        }
        const lay_vec4 rect = ctx->rects[child];
        lay_scalar child_size = rect[dim] + rect[2 + dim] + pchild->margins[wdim];
        need_size = lay_scalar_max(need_size, child_size);
        child = pchild->next_sibling;
    }
    lay_arrange_overlay_squeezed_range(ctx, dim, start_child, LAY_INVALID_ID, offset, need_size);
    offset += need_size;
    return offset;
}

static void lay_arrange(lay_context *ctx, lay_id item, int dim)
{
    lay_item_t *pitem = lay_get_item(ctx, item);

    const uint32_t flags = pitem->flags;
    lay_layout_model model = lay_get_layout_model(flags);
    lay_flex_direction direction = lay_get_flex_direction(flags);
    lay_flex_wrap wrap = lay_get_flex_wrap(flags);
    
    if (model == LAY_MODEL_FLEX) {
        bool is_row_direction = (direction == LAY_ROW || direction == LAY_ROW_REVERSE);
        bool is_wrapped = (wrap != LAY_WRAP_NO);
        
        if (is_wrapped) {
            // Wrapped flex container
            if (is_row_direction) {
                // Row wrap: stacked horizontally, wrapped vertically
                if (dim == 0) {
                    lay_arrange_stacked(ctx, item, 0, true);
                } else {
                    lay_scalar offset = lay_arrange_wrapped_overlay_squeezed(ctx, item, 1);
                    // Update the container's height if needed
                    // ctx->rects[item][2 + 1] = offset - ctx->rects[item][1];
                }
            } else {
                // Column wrap: stacked vertically, wrapped horizontally
                if (dim == 1) {
                    lay_arrange_stacked(ctx, item, 1, true);
                    lay_scalar offset = lay_arrange_wrapped_overlay_squeezed(ctx, item, 0);
                    // Update the container's width if needed
                    // ctx->rects[item][2 + 0] = offset - ctx->rects[item][0];
                } else {
                    // discard return value
                    lay_arrange_wrapped_overlay_squeezed(ctx, item, 0);
                }
            }
        } else {
            // Non-wrapped flex container
            if ((is_row_direction && dim == 0) || (!is_row_direction && dim == 1)) {
                // Main axis: stacked arrangement
                lay_arrange_stacked(ctx, item, dim, false);
            } else {
                // Cross axis: overlay arrangement with alignment
                const lay_vec4 rect = ctx->rects[item];
                lay_arrange_overlay_squeezed_range(
                    ctx, dim, pitem->first_child, LAY_INVALID_ID,
                    lay_get_content_offset(ctx, item, dim), lay_get_internal_space(ctx, item, dim));
            }
        }
    } else {
        // Layout model (absolute positioning)
        lay_arrange_overlay(ctx, item, dim);
    }
    
    // Recursively arrange children
    lay_id child = pitem->first_child;
    while (child != LAY_INVALID_ID) {
        // NOTE: this is recursive and will run out of stack space if items are
        // nested too deeply.
        lay_arrange(ctx, child, dim);
        lay_item_t *pchild = lay_get_item(ctx, child);
        child = pchild->next_sibling;
    }
}
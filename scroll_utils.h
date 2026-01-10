#ifndef SCROLL_UTILS_H
#define SCROLL_UTILS_H

// 前向声明，避免包含layx.h
struct layx_context;
typedef unsigned int layx_id;
typedef float layx_scalar;

// 辅助函数声明
int layx_has_vertical_scrollbar(struct layx_context *ctx, layx_id item);
int layx_has_horizontal_scrollbar(struct layx_context *ctx, layx_id item);

#endif // SCROLL_UTILS_H
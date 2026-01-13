# Web 标准 API 命名总结

## 概述
根据 Web 标准，为 layx 布局引擎添加了 6 个新的 API 函数，遵循浏览器中 DOM 元素的属性命名规范。

## 新增 API 函数

### 1. clientWidth / clientHeight
```c
layx_scalar layx_get_client_width(layx_context *ctx, layx_id item);
layx_scalar layx_get_client_height(layx_context *ctx, layx_id item);
```
- **用途**：获取绘制区域大小
- **定义**：内容 + 内边距，无滚动条
- **计算公式**：
  - `clientWidth = rect.width - border.left - border.right - (垂直滚动条宽度? 15 : 0)`
  - `clientHeight = rect.height - border.top - border.bottom - (水平滚动条高度? 15 : 0)`

### 2. scrollWidth / scrollHeight
```c
layx_scalar layx_get_scroll_width(layx_context *ctx, layx_id item);
layx_scalar layx_get_scroll_height(layx_context *ctx, layx_id item);
```
- **用途**：获取内容区域的实际大小
- **定义**：实际内容大小（可能超出容器）
- **计算公式**：直接返回 `pitem->content_size`

### 3. offsetWidth / offsetHeight
```c
layx_scalar layx_get_offset_width(layx_context *ctx, layx_id item);
layx_scalar layx_get_offset_height(layx_context *ctx, layx_id item);
```
- **用途**：获取元素的总大小（视口）
- **定义**：边框 + 内边距 + 内容，无 margin
- **计算公式**：直接返回 `rect.width` 或 `rect.height`

## 测试结果

### 示例：容器设置
- 宽度：400px
- 高度：300px
- 边框：10px（四周）
- 内边距：20px（四周）
- 溢出：scroll

### 测试输出
```
1. clientWidth / clientHeight (绘制区域)
   clientWidth = 380.00
   clientHeight = 280.00
   说明: 内容+内边距，无滚动条

2. scrollWidth / scrollHeight (内容区域)
   scrollWidth = 360.00
   scrollHeight = 240.00
   说明: 实际内容大小（可能超出容器）

3. offsetWidth / offsetHeight (视口)
   offsetWidth = 400.00
   offsetHeight = 300.00
   说明: 边框+内边距+内容，无margin
```

### 验证计算
- ✓ offsetWidth = 400.00（等于容器宽度）
- ✓ clientWidth = 380.00 = 400.00 - 10.00 - 10.00 - 0

## Web 标准对照

| Web 属性 | layx API | 说明 |
|---------|---------|------|
| `element.clientWidth` | `layx_get_client_width()` | 内容+内边距，无滚动条 |
| `element.clientHeight` | `layx_get_client_height()` | 内容+内边距，无滚动条 |
| `element.scrollWidth` | `layx_get_scroll_width()` | 实际内容大小 |
| `element.scrollHeight` | `layx_get_scroll_height()` | 实际内容大小 |
| `element.offsetWidth` | `layx_get_offset_width()` | 边框+内边距+内容，无margin |
| `element.offsetHeight` | `layx_get_offset_height()` | 边框+内边距+内容，无margin |

## 使用示例

```c
layx_id container = layx_item(&ctx);
layx_set_width(&ctx, container, 400);
layx_set_height(&ctx, container, 300);
layx_set_border(&ctx, container, 10);
layx_set_padding(&ctx, container, 20);
layx_run_context(&ctx);

// 使用 Web 标准命名 API
layx_scalar client_w = layx_get_client_width(&ctx, container);
layx_scalar client_h = layx_get_client_height(&ctx, container);
layx_scalar scroll_w = layx_get_scroll_width(&ctx, container);
layx_scalar scroll_h = layx_get_scroll_height(&ctx, container);
layx_scalar offset_w = layx_get_offset_width(&ctx, container);
layx_scalar offset_h = layx_get_offset_height(&ctx, container);
```

## 实现细节

### 滚动条宽度/高度
- 当前实现假设滚动条宽度/高度为 15 像素
- 可以根据需要调整为动态值或配置值

### 边界处理
- 所有函数都确保返回值 ≥ 0
- 如果计算结果为负数，返回 0

## 兼容性
- 与现有的 layx API 完全兼容
- 新 API 只是增加了便利函数
- 不影响现有的功能和行为

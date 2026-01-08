# Layout CSS兼容性改进方案

## 1. 背景

当前布局系统基于Flexbox模型，与CSS Flexbox在概念上相似，但API设计和使用方式有较大差异。本方案旨在让API更接近CSS Flexbox，提升开发体验和可读性。

## 2. 当前API与CSS Flexbox对比

### 2.1 当前API
```c
// 设置布局属性
lay_set_contain(ctx, item, LAY_ROW | LAY_MODEL_FLEX);
lay_set_behave(ctx, item, LAY_HFILL | LAY_VFILL);
lay_set_size_xy(ctx, item, width, height);
lay_set_margins_ltrb(ctx, item, left, top, right, bottom);
```

### 2.2 CSS Flexbox
```css
.container {
  display: flex;
  flex-direction: row;
  flex-wrap: nowrap;
  justify-content: flex-start;
  align-items: stretch;
  align-content: stretch;
}

.item {
  width: 100px;
  height: 200px;
  margin: 10px;
  padding: 20px;
}
```

## 3. 改进方案

### 3.1 属性命名改进

#### 3.1.1 布局容器属性（对应CSS display + flex属性）

| 当前枚举 | CSS属性 | 新枚举名称 | 说明 |
|---------|---------|-----------|------|
| LAY_MODEL_LAYOUT | display: block | LAYOUT_DISPLAY_BLOCK | 默认布局模式 |
| LAY_MODEL_FLEX | display: flex | LAYOUT_DISPLAY_FLEX | Flex布局 |
| LAY_ROW | flex-direction: row | LAYOUT_FLEX_DIRECTION_ROW | 水平方向 |
| LAY_COLUMN | flex-direction: column | LAYOUT_FLEX_DIRECTION_COLUMN | 垂直方向 |
| LAY_ROW_REVERSE | flex-direction: row-reverse | LAYOUT_FLEX_DIRECTION_ROW_REVERSE | 反向水平 |
| LAY_COLUMN_REVERSE | flex-direction: column-reverse | LAYOUT_FLEX_DIRECTION_COLUMN_REVERSE | 反向垂直 |
| LAY_WRAP_NO | flex-wrap: nowrap | LAYOUT_FLEX_WRAP_NOWRAP | 不换行 |
| LAY_WRAP | flex-wrap: wrap | LAYOUT_FLEX_WRAP_WRAP | 换行 |
| LAY_WRAP_REVERSE | flex-wrap: wrap-reverse | LAYOUT_FLEX_WRAP_WRAP_REVERSE | 反向换行 |
| LAY_JUSTIFY_START | justify-content: flex-start | LAYOUT_JUSTIFY_FLEX_START | 起始对齐 |
| LAY_JUSTIFY_CENTER | justify-content: center | LAYOUT_JUSTIFY_CENTER | 居中对齐 |
| LAY_JUSTIFY_END | justify-content: flex-end | LAYOUT_JUSTIFY_FLEX_END | 结束对齐 |
| LAY_JUSTIFY_SPACE_BETWEEN | justify-content: space-between | LAYOUT_JUSTIFY_SPACE_BETWEEN | 两端对齐 |
| LAY_JUSTIFY_SPACE_AROUND | justify-content: space-around | LAYOUT_JUSTIFY_SPACE_AROUND | 环绕对齐 |
| LAY_JUSTIFY_SPACE_EVENLY | justify-content: space-evenly | LAYOUT_JUSTIFY_SPACE_EVENLY | 均匀对齐 |
| LAY_ALIGN_STRETCH | align-items: stretch | LAYOUT_ALIGN_ITEMS_STRETCH | 拉伸填充 |
| LAY_ALIGN_START | align-items: flex-start | LAYOUT_ALIGN_ITEMS_FLEX_START | 起始对齐 |
| LAY_ALIGN_CENTER | align-items: center | LAYOUT_ALIGN_ITEMS_CENTER | 居中对齐 |
| LAY_ALIGN_END | align-items: flex-end | LAYOUT_ALIGN_ITEMS_FLEX_END | 结束对齐 |
| LAY_ALIGN_BASELINE | align-items: baseline | LAYOUT_ALIGN_ITEMS_BASELINE | 基线对齐 |
| LAY_ALIGN_CONTENT_START | align-content: flex-start | LAYOUT_ALIGN_CONTENT_FLEX_START | 内容起始对齐 |
| LAY_ALIGN_CONTENT_CENTER | align-content: center | LAYOUT_ALIGN_CONTENT_CENTER | 内容居中对齐 |
| LAY_ALIGN_CONTENT_END | align-content: flex-end | LAYOUT_ALIGN_CONTENT_FLEX_END | 内容结束对齐 |
| LAY_ALIGN_CONTENT_SPACE_BETWEEN | align-content: space-between | LAYOUT_ALIGN_CONTENT_SPACE_BETWEEN | 内容两端对齐 |
| LAY_ALIGN_CONTENT_SPACE_AROUND | align-content: space-around | LAYOUT_ALIGN_CONTENT_SPACE_AROUND | 内容环绕对齐 |
| LAY_ALIGN_CONTENT_STRETCH | align-content: stretch | LAYOUT_ALIGN_CONTENT_STRETCH | 内容拉伸 |

#### 3.1.2 尺寸相关属性（对应CSS width/height/box-sizing）

| 当前枚举 | CSS属性 | 新枚举名称 | 说明 |
|---------|---------|-----------|------|
| LAY_ITEM_HFIXED | width: value | LAYOUT_SIZE_FIXED_WIDTH | 固定宽度 |
| LAY_ITEM_VFIXED | height: value | LAYOUT_SIZE_FIXED_HEIGHT | 固定高度 |
| - | box-sizing: content-box | LAYOUT_BOX_SIZING_CONTENT_BOX | 内容盒模型 |
| - | box-sizing: border-box | LAYOUT_BOX_SIZING_BORDER_BOX | 边框盒模型 |
| LAY_HFILL | flex-grow: 1 | LAYOUT_FLEX_GROW | 主轴增长 |
| LAY_VFILL | flex-grow: 1 | LAYOUT_FLEX_GROW | 交叉轴增长 |

#### 3.1.3 定位属性（对应CSS position/align-self）

| 当前枚举 | CSS属性 | 新枚举名称 | 说明 |
|---------|---------|-----------|------|
| LAY_LEFT | align-self: flex-start | LAYOUT_ALIGN_SELF_FLEX_START | 起始对齐 |
| LAY_TOP | align-self: flex-start | LAYOUT_ALIGN_SELF_FLEX_START | 起始对齐 |
| LAY_RIGHT | align-self: flex-end | LAYOUT_ALIGN_SELF_FLEX_END | 结束对齐 |
| LAY_BOTTOM | align-self: flex-end | LAYOUT_ALIGN_SELF_FLEX_END | 结束对齐 |
| LAY_CENTER | align-self: center | LAYOUT_ALIGN_SELF_CENTER | 居中对齐 |
| LAY_FILL | align-self: stretch | LAYOUT_ALIGN_SELF_STRETCH | 拉伸填充 |

### 3.2 函数API改进

#### 3.2.1 布局容器设置API

```c
// display属性
typedef enum {
    LAYOUT_DISPLAY_BLOCK,
    LAYOUT_DISPLAY_FLEX
} layout_display;

void layout_set_display(lay_context *ctx, lay_id item, layout_display display);

// flex-direction属性
typedef enum {
    LAYOUT_FLEX_DIRECTION_ROW,
    LAYOUT_FLEX_DIRECTION_COLUMN,
    LAYOUT_FLEX_DIRECTION_ROW_REVERSE,
    LAYOUT_FLEX_DIRECTION_COLUMN_REVERSE
} layout_flex_direction;

void layout_set_flex_direction(lay_context *ctx, lay_id item, layout_flex_direction direction);

// flex-wrap属性
typedef enum {
    LAYOUT_FLEX_WRAP_NOWRAP,
    LAYOUT_FLEX_WRAP_WRAP,
    LAYOUT_FLEX_WRAP_WRAP_REVERSE
} layout_flex_wrap;

void layout_set_flex_wrap(lay_context *ctx, lay_id item, layout_flex_wrap wrap);

// justify-content属性
typedef enum {
    LAYOUT_JUSTIFY_FLEX_START,
    LAYOUT_JUSTIFY_CENTER,
    LAYOUT_JUSTIFY_FLEX_END,
    LAYOUT_JUSTIFY_SPACE_BETWEEN,
    LAYOUT_JUSTIFY_SPACE_AROUND,
    LAYOUT_JUSTIFY_SPACE_EVENLY
} layout_justify_content;

void layout_set_justify_content(lay_context *ctx, lay_id item, layout_justify_content justify);

// align-items属性
typedef enum {
    LAYOUT_ALIGN_ITEMS_STRETCH,
    LAYOUT_ALIGN_ITEMS_FLEX_START,
    LAYOUT_ALIGN_ITEMS_CENTER,
    LAYOUT_ALIGN_ITEMS_FLEX_END,
    LAYOUT_ALIGN_ITEMS_BASELINE
} layout_align_items;

void layout_set_align_items(lay_context *ctx, lay_id item, layout_align_items align);

// align-content属性
typedef enum {
    LAYOUT_ALIGN_CONTENT_STRETCH,
    LAYOUT_ALIGN_CONTENT_FLEX_START,
    LAYOUT_ALIGN_CONTENT_CENTER,
    LAYOUT_ALIGN_CONTENT_FLEX_END,
    LAYOUT_ALIGN_CONTENT_SPACE_BETWEEN,
    LAYOUT_ALIGN_CONTENT_SPACE_AROUND
} layout_align_content;

void layout_set_align_content(lay_context *ctx, lay_id item, layout_align_content align);

// 简写API - 类似CSS flex简写
void layout_set_flex(lay_context *ctx, lay_id item,
                   layout_flex_direction direction,
                   layout_flex_wrap wrap,
                   layout_justify_content justify,
                   layout_align_items align_items,
                   layout_align_content align_content);

// 获取容器属性字符串
const char* layout_get_display_string(layout_display display);
const char* layout_get_flex_direction_string(layout_flex_direction direction);
const char* layout_get_flex_wrap_string(layout_flex_wrap wrap);
const char* layout_get_justify_content_string(layout_justify_content justify);
const char* layout_get_align_items_string(layout_align_items align);
const char* layout_get_align_content_string(layout_align_content align);
```

#### 3.2.2 尺寸设置API

```c
// width/height属性
void layout_set_width(lay_context *ctx, lay_id item, lay_scalar width);
void layout_set_height(lay_context *ctx, lay_id item, lay_scalar height);
void layout_get_width(lay_context *ctx, lay_id item, lay_scalar *width);
void layout_get_height(lay_context *ctx, lay_id item, lay_scalar *height);

// min-width/min-height属性
void layout_set_min_width(lay_context *ctx, lay_id item, lay_scalar min_width);
void layout_set_min_height(lay_context *ctx, lay_id item, lay_scalar min_height);

// max-width/max-height属性
void layout_set_max_width(lay_context *ctx, lay_id item, lay_scalar max_width);
void layout_set_max_height(lay_context *ctx, lay_id item, lay_scalar max_height);

// Flex项目属性
void layout_set_flex_grow(lay_context *ctx, lay_id item, lay_scalar grow);
void layout_set_flex_shrink(lay_context *ctx, lay_id item, lay_scalar shrink);
void layout_set_flex_basis(lay_context *ctx, lay_id item, lay_scalar basis);

void layout_set_flex_properties(lay_context *ctx, lay_id item,
                              lay_scalar grow, lay_scalar shrink, lay_scalar basis);

// align-self属性
void layout_set_align_self(lay_context *ctx, lay_id item, layout_align_items align);

// 简写API
void layout_set_size(lay_context *ctx, lay_id item, lay_scalar width, lay_scalar height);
```

#### 3.2.3 盒模型API

```c
// margin属性 - 单独设置各边
void layout_set_margin(lay_context *ctx, lay_id item, lay_scalar value);
void layout_set_margin_top(lay_context *ctx, lay_id item, lay_scalar top);
void layout_set_margin_right(lay_context *ctx, lay_id item, lay_scalar right);
void layout_set_margin_bottom(lay_context *ctx, lay_id item, lay_scalar bottom);
void layout_set_margin_left(lay_context *ctx, lay_id item, lay_scalar left);

// margin属性 - CSS顺序(top, right, bottom, left)
void layout_set_margin_trbl(lay_context *ctx, lay_id item,
                         lay_scalar top, lay_scalar right,
                         lay_scalar bottom, lay_scalar left);

// padding属性 - 单独设置各边
void layout_set_padding(lay_context *ctx, lay_id item, lay_scalar value);
void layout_set_padding_top(lay_context *ctx, lay_id item, lay_scalar top);
void layout_set_padding_right(lay_context *ctx, lay_id item, lay_scalar right);
void layout_set_padding_bottom(lay_context *ctx, lay_id item, lay_scalar bottom);
void layout_set_padding_left(lay_context *ctx, lay_id item, lay_scalar left);

// padding属性 - CSS顺序(top, right, bottom, left)
void layout_set_padding_trbl(lay_context *ctx, lay_id item,
                          lay_scalar top, lay_scalar right,
                          lay_scalar bottom, lay_scalar left);

// border属性 - 单独设置各边
void layout_set_border(lay_context *ctx, lay_id item, lay_scalar value);
void layout_set_border_top(lay_context *ctx, lay_id item, lay_scalar top);
void layout_set_border_right(lay_context *ctx, lay_id item, lay_scalar right);
void layout_set_border_bottom(lay_context *ctx, lay_id item, lay_scalar bottom);
void layout_set_border_left(lay_context *ctx, lay_id item, lay_scalar left);

// border属性 - CSS顺序(top, right, bottom, left)
void layout_set_border_trbl(lay_context *ctx, lay_id item,
                         lay_scalar top, lay_scalar right,
                         lay_scalar bottom, lay_scalar left);

// box-sizing属性
typedef enum {
    LAYOUT_BOX_SIZING_CONTENT_BOX,
    LAYOUT_BOX_SIZING_BORDER_BOX
} layout_box_sizing;

void layout_set_box_sizing(lay_context *ctx, lay_id item, layout_box_sizing sizing);

// 获取盒模型属性
void layout_get_margin(lay_context *ctx, lay_id item, lay_scalar *top, lay_scalar *right, lay_scalar *bottom, lay_scalar *left);
void layout_get_padding(lay_context *ctx, lay_id item, lay_scalar *top, lay_scalar *right, lay_scalar *bottom, lay_scalar *left);
void layout_get_border(lay_context *ctx, lay_id item, lay_scalar *top, lay_scalar *right, lay_scalar *bottom, lay_scalar *left);
```

### 3.3 声明式布局API

```c
// 定义一个布局配置结构
typedef struct layout_style {
    // display属性
    layout_display display;

    // flex容器属性
    layout_flex_direction flex_direction;
    layout_flex_wrap flex_wrap;
    layout_justify_content justify_content;
    layout_align_items align_items;
    layout_align_content align_content;

    // 尺寸属性
    lay_scalar width, height;
    lay_scalar min_width, min_height;
    lay_scalar max_width, max_height;

    // 盒模型属性
    lay_scalar margin_top, margin_right, margin_bottom, margin_left;
    lay_scalar padding_top, padding_right, padding_bottom, padding_left;
    lay_scalar border_top, border_right, border_bottom, border_left;
    layout_box_sizing box_sizing;

    // flex项目属性
    layout_align_items align_self;
    lay_scalar flex_grow, flex_shrink, flex_basis;

    // 布局行为标志
    unsigned int flags;
} layout_style;

// 应用样式
void layout_apply_style(lay_context *ctx, lay_id item, const layout_style *style);

// 创建并应用样式
lay_id layout_create_item_with_style(lay_context *ctx, const layout_style *style);

// 使用示例
layout_style container_style = {
    .display = LAYOUT_DISPLAY_FLEX,
    .flex_direction = LAYOUT_FLEX_DIRECTION_ROW,
    .flex_wrap = LAYOUT_FLEX_WRAP_WRAP,
    .justify_content = LAYOUT_JUSTIFY_CENTER,
    .align_items = LAYOUT_ALIGN_ITEMS_CENTER,
    .padding_top = 20, .padding_right = 20, .padding_bottom = 20, .padding_left = 20,
    .width = 600,
    .height = 400
};

layout_style item_style = {
    .width = 100,
    .height = 100,
    .margin_top = 10, .margin_right = 10, .margin_bottom = 10, .margin_left = 10
};

lay_id container = layout_create_item_with_style(&ctx, &container_style);

lay_id item = layout_create_item_with_style(&ctx, &item_style);
lay_insert(&ctx, container, item);
```

### 3.4 辅助API

```c
// 重置样式为默认值
void layout_style_reset(layout_style *style);

// 检查样式属性是否设置
int layout_style_has_width(const layout_style *style);
int layout_style_has_height(const layout_style *style);
int layout_style_has_margin(const layout_style *style);
int layout_style_has_padding(const layout_style *style);
int layout_style_has_border(const layout_style *style);

// 获取样式属性摘要
void layout_style_print(const layout_style *style, const char *name);
```

## 4. 实施计划

### 阶段1：枚举重构（1周）
1. 重命名所有枚举类型和值
2. 更新头文件中的枚举定义
3. 更新内部实现代码
4. 编写单元测试

### 阶段2：基础API实现（1-2周）
1. 实现所有新的语义化API函数
2. 更新布局引擎内部实现
3. 实现属性getter函数
4. 集成测试

### 阶段3：声明式布局（1周）
1. 实现layout_style结构
2. 实现layout_apply_style函数
3. 实现layout_create_item_with_style函数
4. 实现辅助API

### 阶段4：文档和示例（1周）
1. 编写API参考文档
2. 编写迁移指南
3. 编写使用示例
4. 编写最佳实践指南

### 阶段5：测试和优化（1周）
1. 完整的功能测试
2. 性能测试和优化
3. 内存泄漏检查
4. 文档最终审查

## 5. 代码示例对比

### 5.1 当前API使用示例

```c
lay_id root = lay_item(&ctx);
lay_set_size_xy(&ctx, root, 600, 400);
lay_set_contain(&ctx, root, LAY_ROW | LAY_MODEL_FLEX);
lay_set_padding_ltrb(&ctx, root, 10, 10, 10, 10);

lay_id item = lay_item(&ctx);
lay_set_size_xy(&ctx, item, 100, 100);
lay_set_margins_ltrb(&ctx, item, 10, 10, 10, 10);
lay_insert(&ctx, root, item);
```

### 5.2 新API使用示例（函数式）

```c
lay_id root = lay_item(&ctx);
layout_set_display(&ctx, root, LAYOUT_DISPLAY_FLEX);
layout_set_flex_direction(&ctx, root, LAYOUT_FLEX_DIRECTION_ROW);
layout_set_flex_wrap(&ctx, root, LAYOUT_FLEX_WRAP_WRAP);
layout_set_justify_content(&ctx, root, LAYOUT_JUSTIFY_CENTER);
layout_set_align_items(&ctx, root, LAYOUT_ALIGN_ITEMS_CENTER);
layout_set_width(&ctx, root, 600);
layout_set_height(&ctx, root, 400);
layout_set_padding(&ctx, root, 10);

lay_id item = lay_item(&ctx);
layout_set_width(&ctx, item, 100);
layout_set_height(&ctx, item, 100);
layout_set_margin(&ctx, item, 10);
lay_insert(&ctx, root, item);
```

### 5.3 新API使用示例（声明式）

```c
layout_style root_style = {
    .display = LAYOUT_DISPLAY_FLEX,
    .flex_direction = LAYOUT_FLEX_DIRECTION_ROW,
    .flex_wrap = LAYOUT_FLEX_WRAP_WRAP,
    .justify_content = LAYOUT_JUSTIFY_CENTER,
    .align_items = LAYOUT_ALIGN_ITEMS_CENTER,
    .padding_top = 10, .padding_right = 10, .padding_bottom = 10, .padding_left = 10,
    .width = 600,
    .height = 400
};

layout_style item_style = {
    .width = 100,
    .height = 100,
    .margin_top = 10, .margin_right = 10, .margin_bottom = 10, .margin_left = 10
};

lay_id root = layout_create_item_with_style(&ctx, &root_style);
lay_id item = layout_create_item_with_style(&ctx, &item_style);
lay_insert(&ctx, root, item);
```

## 6. 完整使用示例

### 6.1 函数式API - 创建响应式布局

```c
lay_context ctx;
lay_init_context(&ctx);
lay_reserve_items_capacity(&ctx, 20);

// 创建根容器
lay_id root = lay_item(&ctx);
layout_set_display(&ctx, root, LAYOUT_DISPLAY_FLEX);
layout_set_flex_direction(&ctx, root, LAYOUT_FLEX_DIRECTION_ROW);
layout_set_width(&ctx, root, 800);
layout_set_height(&ctx, root, 600);
layout_set_padding(&ctx, root, 20);

// 创建侧边栏
lay_id sidebar = lay_item(&ctx);
layout_set_width(&ctx, sidebar, 200);
layout_set_display(&ctx, sidebar, LAYOUT_DISPLAY_FLEX);
layout_set_flex_direction(&ctx, sidebar, LAYOUT_FLEX_DIRECTION_COLUMN);
layout_set_margin_right(&ctx, sidebar, 20);
lay_insert(&ctx, root, sidebar);

// 创建侧边栏按钮
lay_id btn1 = lay_item(&ctx);
layout_set_height(&ctx, btn1, 40);
layout_set_margin_bottom(&ctx, btn1, 10);
lay_insert(&ctx, sidebar, btn1);

lay_id btn2 = lay_item(&ctx);
layout_set_height(&ctx, btn2, 40);
layout_set_margin_bottom(&ctx, btn2, 10);
lay_insert(&ctx, sidebar, btn2);

// 创建主内容区域
lay_id content = lay_item(&ctx);
layout_set_display(&ctx, content, LAYOUT_DISPLAY_FLEX);
layout_set_flex_direction(&ctx, content, LAYOUT_FLEX_DIRECTION_COLUMN);
layout_set_flex_grow(&ctx, content, 1);
lay_insert(&ctx, root, content);

// 创建卡片
for (int i = 0; i < 5; i++) {
    lay_id card = lay_item(&ctx);
    layout_set_width(&ctx, card, 200);
    layout_set_height(&ctx, card, 150);
    layout_set_margin_bottom(&ctx, card, 20);
    layout_set_margin_right(&ctx, card, 20);
    lay_insert(&ctx, content, card);
}

// 运行布局计算
lay_run_context(&ctx);

// 获取布局结果
lay_vec4 rect = lay_get_rect(&ctx, root);
printf("Root: x=%.2f, y=%.2f, w=%.2f, h=%.2f\n", rect[0], rect[1], rect[2], rect[3]);

lay_destroy_context(&ctx);
```

### 6.2 声明式API - 创建响应式布局

```c
lay_context ctx;
lay_init_context(&ctx);
lay_reserve_items_capacity(&ctx, 20);

// 定义样式
layout_style root_style = {
    .display = LAYOUT_DISPLAY_FLEX,
    .flex_direction = LAYOUT_FLEX_DIRECTION_ROW,
    .padding = 20,
    .width = 800,
    .height = 600
};

layout_style sidebar_style = {
    .display = LAYOUT_DISPLAY_FLEX,
    .flex_direction = LAYOUT_FLEX_DIRECTION_COLUMN,
    .width = 200,
    .margin_right = 20
};

layout_style button_style = {
    .height = 40,
    .margin_bottom = 10
};

layout_style content_style = {
    .display = LAYOUT_DISPLAY_FLEX,
    .flex_direction = LAYOUT_FLEX_DIRECTION_COLUMN,
    .flex_grow = 1
};

layout_style card_style = {
    .width = 200,
    .height = 150,
    .margin_bottom = 20,
    .margin_right = 20
};

// 创建布局
lay_id root = layout_create_item_with_style(&ctx, &root_style);
lay_id sidebar = layout_create_item_with_style(&ctx, &sidebar_style);
lay_id content = layout_create_item_with_style(&ctx, &content_style);

lay_insert(&ctx, root, sidebar);
lay_insert(&ctx, root, content);

// 添加按钮
lay_id btn1 = layout_create_item_with_style(&ctx, &button_style);
lay_id btn2 = layout_create_item_with_style(&ctx, &button_style);
lay_insert(&ctx, sidebar, btn1);
lay_insert(&ctx, sidebar, btn2);

// 添加卡片
for (int i = 0; i < 5; i++) {
    lay_id card = layout_create_item_with_style(&ctx, &card_style);
    lay_insert(&ctx, content, card);
}

// 运行布局计算
lay_run_context(&ctx);

// 获取布局结果
lay_vec4 rect = lay_get_rect(&ctx, root);
printf("Root: x=%.2f, y=%.2f, w=%.2f, h=%.2f\n", rect[0], rect[1], rect[2], rect[3]);

lay_destroy_context(&ctx);
```

## 7. 优势分析

### 7.1 API设计优势
1. **语义清晰**：函数名直接对应CSS属性名，开发者无需记忆
2. **类型安全**：枚举类型提供编译时检查
3. **易于理解**：熟悉CSS Flexbox的开发者可以快速上手
4. **灵活使用**：支持函数式和声明式两种方式
5. **可维护性**：清晰的API结构降低维护成本

### 7.2 实施优势
1. **无需兼容旧API**：可以彻底重构，避免历史包袱
2. **纯C实现**：不依赖C++，保持轻量级
3. **渐进式升级**：可以分阶段实施，降低风险
4. **易于测试**：清晰的API设计便于编写测试用例

### 7.3 开发体验优势
1. **降低学习曲线**：熟悉CSS的开发者可以直接使用
2. **提升开发效率**：声明式API减少代码量
3. **便于调试**：清晰的属性名称和工具函数
4. **团队协作**：统一的API规范便于团队协作

## 8. 注意事项

### 8.1 性能考虑
1. **布局计算性能**：新API不改变底层算法，性能不受影响
2. **内存开销**：layout_style结构增加内存使用，但可接受
3. **缓存友好**：声明式API可能更适合缓存优化

### 8.2 使用建议
1. **选择合适的方式**：
   - 简单布局使用函数式API
   - 复杂布局使用声明式API
   - 动态布局混合使用两种方式

2. **避免过度设计**：
   - 不需要创建过多的style变量
   - 简单属性直接使用函数式API

3. **代码组织**：
   - 将常用样式定义为常量
   - 使用配置文件管理复杂样式

### 8.3 扩展性
1. **预留扩展空间**：layout_style结构预留扩展字段
2. **模块化设计**：属性设置函数相互独立
3. **文档完善**：清晰的API文档便于未来扩展

## 9. 总结

本方案通过三个方面改进布局系统：

1. **属性命名**：直接采用CSS Flexbox属性命名，提升可读性和熟悉度
2. **函数API**：提供语义化函数API，类型安全，易于使用
3. **使用方式**：支持函数式和声明式两种方式，满足不同场景需求

改进后的API将更加直观、易用，同时保持高性能和灵活性。纯C语言实现，无历史包袱，可以彻底优化。

## 10. 附录

### 10.1 CSS Flexbox属性对照表

| CSS属性 | 新API枚举 | 说明 |
|---------|-----------|------|
| display: flex | LAYOUT_DISPLAY_FLEX | 启用Flex布局 |
| display: block | LAYOUT_DISPLAY_BLOCK | 块级布局 |
| flex-direction: row | LAYOUT_FLEX_DIRECTION_ROW | 水平方向 |
| flex-direction: column | LAYOUT_FLEX_DIRECTION_COLUMN | 垂直方向 |
| flex-wrap: nowrap | LAYOUT_FLEX_WRAP_NOWRAP | 不换行 |
| flex-wrap: wrap | LAYOUT_FLEX_WRAP_WRAP | 换行 |
| justify-content: flex-start | LAYOUT_JUSTIFY_FLEX_START | 起始对齐 |
| justify-content: center | LAYOUT_JUSTIFY_CENTER | 居中对齐 |
| justify-content: flex-end | LAYOUT_JUSTIFY_FLEX_END | 结束对齐 |
| align-items: stretch | LAYOUT_ALIGN_ITEMS_STRETCH | 拉伸 |
| align-items: flex-start | LAYOUT_ALIGN_ITEMS_FLEX_START | 起始对齐 |
| align-items: center | LAYOUT_ALIGN_ITEMS_CENTER | 居中对齐 |
| align-self: stretch | LAYOUT_ALIGN_SELF_STRETCH | 拉伸 |
| flex-grow: 1 | layout_set_flex_grow(..., 1) | 主轴增长 |

### 10.2 迁移示例

```c
// 旧API
lay_set_contain(ctx, item, LAY_ROW | LAY_MODEL_FLEX);
lay_set_behave(ctx, item, LAY_HFILL);
lay_set_size_xy(ctx, item, 100, 200);
lay_set_margins_ltrb(ctx, item, 10, 10, 10, 10);

// 新API
layout_set_display(ctx, item, LAYOUT_DISPLAY_FLEX);
layout_set_flex_direction(ctx, item, LAYOUT_FLEX_DIRECTION_ROW);
layout_set_flex_grow(ctx, item, 1);
layout_set_width(ctx, item, 100);
layout_set_height(ctx, item, 200);
layout_set_margin(ctx, item, 10);
```

# LayX - 轻量级CSS Flexbox布局引擎

LayX是一个用C语言实现的CSS Flexbox布局引擎，提供高性能的二维布局计算能力。

## 特性

- 🎯 **完整的Flexbox支持**: 实现CSS Flexbox Level 1规范的核心功能
- ⚡ **高性能**: 纯C实现，无依赖，适合嵌入式和资源受限环境
- 🔧 **灵活的API**: 提供简洁的函数接口，易于集成到现有项目
- 📐 **精确的布局计算**: 两阶段布局算法，准确处理尺寸和位置计算
- 🎨 **CSS标准兼容**: 遵循CSS规范，支持主流布局属性

## 支持的CSS属性

### 布局模型
- `display: block | flex`
- `flex-direction: row | column | row-reverse | column-reverse`
- `flex-wrap: nowrap | wrap | wrap-reverse`

### 对齐属性
- `justify-content: flex-start | center | flex-end | space-between | space-around | space-evenly`
- `align-items: stretch | flex-start | center | flex-end | baseline`
- `align-content: stretch | flex-start | center | flex-end | space-between | space-around`
- `align-self: auto | flex-start | center | flex-end | stretch`

### 尺寸控制
- `width | height`
- `min-width | min-height`
- `max-width | max-height`
- `flex-grow | flex-shrink | flex-basis`

### 盒模型
- `margin: <value>` 及四方向独立设置
- `padding: <value>` 及四方向独立设置  
- `border: <value>` 及四方向独立设置

### 滚动和溢出
- `overflow-x | overflow-y | overflow: visible | hidden | scroll | auto`

## 滚动功能

LayX支持CSS标准的滚动行为，提供完整的溢出控制机制。

### Overflow属性
- **visible**: 内容溢出时正常显示（默认）
- **hidden**: 溢出内容被裁剪
- **scroll**: 总是显示滚动条，内容可滚动
- **auto**: 根据内容溢出情况自动显示滚动条

### 滚动条特性
- **浮动滚动条**: 滚动条覆盖在内容上，不占用布局空间
- **独立检测**: 水平和垂直滚动条独立计算，互不影响
- **自动范围**: 自动计算最大滚动距离（scroll_max）

### 滚动API
```c
// 设置overflow属性
layx_set_overflow(ctx, item, LAYX_OVERFLOW_AUTO);
layx_set_overflow_x(ctx, item, LAYX_OVERFLOW_SCROLL);
layx_set_overflow_y(ctx, item, LAYX_OVERFLOW_HIDDEN);

// 滚动操作
layx_scroll_to(ctx, item, 50.0f, 100.0f);    // 滚动到指定位置
layx_scroll_by(ctx, item, 10.0f, 20.0f);     // 相对滚动

// 获取滚动信息
layx_vec2 offset = layx_get_scroll_offset(ctx, item);
layx_vec2 max = layx_get_scroll_max(ctx, item);
layx_vec2 content = layx_get_content_size(ctx, item);

// 检测滚动条
int has_v = layx_has_vertical_scrollbar(ctx, item);
int has_h = layx_has_horizontal_scrollbar(ctx, item);

// 获取可见内容区域
layx_scalar left, top, right, bottom;
layx_get_visible_content_rect(ctx, item, &left, &top, &right, &bottom);
```

### 布局流程中的滚动
完整的滚动相关函数调用顺序：
```c
layx_init_scroll_fields(ctx, item);         // 初始化滚动字段
layx_calculate_content_size(ctx, item);    // 计算内容尺寸
layx_detect_scrollbars(ctx, item);          // 检测并配置滚动条
```

### 渲染集成建议
在渲染器中使用LayX的滚动功能时，遵循以下推荐方式：

```c
// 在渲染引擎中渲染元素时
void render_element(layx_context* ctx, layx_id item) {
    layx_item_t* pitem = layx_get_item(ctx, item);
    
    // 1. 应用裁剪（clip to bounds）
    if (pitem->overflow_x != LAYX_OVERFLOW_VISIBLE ||
        pitem->overflow_y != LAYX_OVERFLOW_VISIBLE) {
        // 设置裁剪区域为内容框（不含滚动条）
        layx_vec4 clip_rect = get_content_clip_rect(ctx, item);
        graphics_set_clip(clip_rect[0], clip_rect[1], 
                         clip_rect[2], clip_rect[3]);
    }
    
    // 2. 应用滚动偏移（内容向上/向左移动）
    layx_vec2 scroll_offset = layx_get_scroll_offset(ctx, item);
    graphics_translate(-scroll_offset[0], -scroll_offset[1]);
    
    // 3. 渲染子元素（可能被裁剪和滚动）
    layx_id child = pitem->first_child;
    while (child != LAYX_INVALID_ID) {
        render_element(ctx, child);
        child = layx_next_sibling(ctx, child);
    }
    
    // 4. 恢复滚动偏移
    graphics_translate(scroll_offset[0], scroll_offset[1]);
    
    // 5. 恢复裁剪
    if (pitem->overflow_x != LAYX_OVERFLOW_VISIBLE ||
        pitem->overflow_y != LAYX_OVERFLOW_VISIBLE) {
        graphics_reset_clip();
    }
    
    // 6. 在内容上方绘制滚动条（浮动层，不被裁剪）
    if (layx_has_vertical_scrollbar(ctx, item)) {
        draw_vertical_scrollbar(ctx, item);
    }
    if (layx_has_horizontal_scrollbar(ctx, item)) {
        draw_horizontal_scrollbar(ctx, item);
    }
}

// 获取内容裁剪区域（内容框）
layx_vec4 get_content_clip_rect(layx_context* ctx, layx_id item) {
    layx_item_t* pitem = layx_get_item(ctx, item);
    layx_vec4 rect;
    
    // 计算内容框（padding-box）
    rect[0] = pitem->layout_rect[0] + pitem->border[0] + pitem->padding[0];
    rect[1] = pitem->layout_rect[1] + pitem->border[1] + pitem->padding[1];
    rect[2] = pitem->size[0] - pitem->border[0] - pitem->border[2] - 
               pitem->padding[0] - pitem->padding[2];
    rect[3] = pitem->size[1] - pitem->border[1] - pitem->border[3] - 
               pitem->padding[1] - pitem->padding[3];
    
    return rect;
}

// 绘制垂直滚动条
void draw_vertical_scrollbar(layx_context* ctx, layx_id item) {
    layx_item_t* pitem = layx_get_item(ctx, item);
    
    // 滚动条位置和尺寸
    layx_vec2 scroll_offset = layx_get_scroll_offset(ctx, item);
    layx_vec2 scroll_max = layx_get_scroll_max(ctx, item);
    layx_vec2 content_size = layx_get_content_size(ctx, item);
    
    // 内容框尺寸
    layx_scalar client_width = pitem->size[0] - pitem->padding[0] - 
                               pitem->padding[2] - pitem->border[0] - pitem->border[2];
    layx_scalar client_height = pitem->size[1] - pitem->padding[1] - 
                                pitem->padding[3] - pitem->border[1] - pitem->border[3];
    
    // 滚动条位置（右侧覆盖在内容上）
    layx_scalar track_x = pitem->layout_rect[0] + pitem->size[0] - 
                         pitem->scrollbar_size - pitem->margin[2] - pitem->border[2];
    layx_scalar track_y = pitem->layout_rect[1] + pitem->border[1] + pitem->padding[1];
    layx_scalar track_height = client_height;
    
    // 滑块位置
    layx_scalar thumb_y = track_y;
    layx_scalar thumb_height = pitem->scrollbar_size;
    
    if (scroll_max[1] > 0) {
        float ratio = client_height / content_size[1];
        thumb_height = track_height * ratio;
        float scroll_ratio = scroll_offset[1] / scroll_max[1];
        float travel_distance = track_height - thumb_height;
        thumb_y = track_y + travel_distance * scroll_ratio;
    }
    
    // 绘制轨道和滑块
    draw_rect(track_x, track_y, pitem->scrollbar_size, track_height, track_color);
    draw_rect(track_x, thumb_y, pitem->scrollbar_size, thumb_height, thumb_color);
}
```

**渲染要点：**
- 裁剪区域为内容框，不包含滚动条
- 滚动偏移应用于内容渲染，不应用于滚动条
- 滚动条在恢复裁剪后绘制，位于内容上方
- 支持滚动条拖拽交互更新 `scroll_offset`

## 核心架构

### 数据结构
- **layx_context**: 布局上下文管理器
- **layx_item_t**: 布局项目，对应DOM元素
- **layx_style**: 样式描述结构
- **位标志系统**: 32位flags高效存储布局属性

### 布局算法
采用两阶段布局策略：
1. **计算阶段**: 递归计算每个项目的尺寸需求
2. **排列阶段**: 根据对齐属性确定最终位置和尺寸

支持智能空间分配、flex-grow填充、min/max约束等高级特性。

## 快速开始

### 编译
```bash
mkdir build && cd build
cmake ..
make
```

### 基本用法
```c
#define LAYX_IMPLEMENTATION
#include "layx.h"

int main() {
    layx_context ctx;
    layx_init_context(&ctx);
    layx_reserve_items_capacity(&ctx, 20);
    
    // 创建根容器
    layx_id root = layx_item(&ctx);
    layx_set_size(&ctx, root, 600, 400);
    layx_set_display(&ctx, root, LAYX_DISPLAY_FLEX);
    layx_set_flex_direction(&ctx, root, LAYX_FLEX_DIRECTION_ROW);
    
    // 添加子元素
    layx_id child = layx_item(&ctx);
    layx_set_size(&ctx, child, 100, 100);
    layx_insert(&ctx, root, child);
    
    // 设置滚动和溢出
    layx_set_overflow_x(&ctx, root, LAYX_OVERFLOW_AUTO);
    layx_set_overflow_y(&ctx, root, LAYX_OVERFLOW_AUTO);
    layx_init_scroll_fields(&ctx, root);
    
    // 执行布局
    layx_run_context(&ctx);
    
    // 计算内容尺寸和检测滚动条
    layx_calculate_content_size(&ctx, root);
    layx_detect_scrollbars(&ctx, root);
    
    // 获取结果
    layx_vec4 rect = layx_get_rect(&ctx, child);
    printf("Child position: (%.2f, %.2f), size: (%.2f, %.2f)\n", 
           rect[0], rect[1], rect[2], rect[3]);
    
    // 滚动操作
    if (layx_has_vertical_scrollbar(&ctx, root)) {
        layx_scroll_to(&ctx, root, 0.0f, 50.0f);
    }
    
    layx_destroy_context(&ctx);
    return 0;
}
```

## 测试

项目包含完整的测试用例：
- `test_layx.c` - 基础功能演示
- `test_layout_patterns.c` - 复杂布局模式测试
- `test_defaults.c` - 默认值和边界条件测试
- `test_scroll.c` - 滚动和溢出功能测试
- `debug_*.c` - 各对齐属性的可视化调试程序

运行测试：
```bash
cd build
./test_layx
./test_layout_patterns
./test_scroll
```

## 项目结构

```
mylayout/
├── layx.h              # 公共头文件，API定义
├── layx.c              # 核心实现
├── scroll_utils.h      # 滚动工具头文件
├── scroll_utils.c      # 滚动功能实现
├── test_layx.c         # 基础测试
├── test_layout_patterns.c # 布局模式测试
├── test_defaults.c     # 默认值测试
├── test_scroll.c       # 滚动功能测试
├── debug_*.c           # 调试工具
├── CMakeLists.txt      # 构建配置
└── README.md           # 项目文档
```

## 设计理念

1. **性能优先**: 避免动态内存分配，使用预分配和位运算优化
2. **标准兼容**: 严格遵循CSS规范，确保行为可预测
3. **轻量便携**: 单一头文件库设计，易于集成到各种平台
4. **调试友好**: 提供丰富的调试信息和可视化工具

## 浏览器兼容性

LayX是原生C库，可直接集成到：
- 桌面应用程序
- 游戏引擎
- 嵌入式系统
- 服务器端渲染服务
- 跨平台GUI框架

## 贡献指南

欢迎提交Issue和Pull Request来改进LayX：
1. 确保代码符合现有风格
2. 添加相应的测试用例
3. 更新文档说明
4. 验证CSS规范兼容性

## 许可证

MIT License - 详见LICENSE文件

## 参考

- [CSS Flexible Box Layout Module Level 1](https://www.w3.org/TR/css-flexbox-1/)
- [MDN Flexbox Guide](https://developer.mozilla.org/en-US/docs/Web/CSS/CSS_Flexible_Box_Layout)
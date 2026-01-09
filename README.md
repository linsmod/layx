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
    
    // 执行布局
    layx_run_context(&ctx);
    
    // 获取结果
    layx_vec4 rect = layx_get_rect(&ctx, child);
    printf("Child position: (%.2f, %.2f), size: (%.2f, %.2f)\n", 
           rect[0], rect[1], rect[2], rect[3]);
    
    layx_destroy_context(&ctx);
    return 0;
}
```

## 测试

项目包含完整的测试用例：
- `test_layx.c` - 基础功能演示
- `test_layout_patterns.c` - 复杂布局模式测试
- `test_defaults.c` - 默认值和边界条件测试
- `debug_*.c` - 各对齐属性的可视化调试程序

运行测试：
```bash
cd build
./test_layx
./test_layout_patterns
```

## 项目结构

```
mylayout/
├── layx.h          # 公共头文件，API定义
├── layx.c          # 核心实现
├── test_layx.c     # 基础测试
├── test_layout_patterns.c # 布局模式测试
├── test_defaults.c # 默认值测试
├── debug_*.c       # 调试工具
├── CMakeLists.txt  # 构建配置
└── README.md       # 项目文档
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

# LayX 位掩码设计方案

## 1. 问题背景

### 1.1 原始问题
在早期的layx实现中，由于位掩码设计不合理，导致多个属性之间存在位冲突，造成以下问题：

```c
// 原始设计（有问题）
#define LAYX_ALIGN_SELF_MASK     0x7000   // 位12-14
#define LAYX_SIZE_FIXED_HEIGHT   0x1000   // 位12

// 问题：当item设置了固定高度时，flags中有0x1000，
// 会被误认为是align-self的值，导致对齐逻辑失效
```

### 1.2 问题表现
- align-items/align-self对齐属性无法正常工作
- 设置固定宽度/高度后，对齐方式被错误解析
- item的位置计算错误

### 1.3 根本原因
多个属性共享相同的位，导致：
1. 提取属性时无法区分来源
2. 位掩码重叠（0x7000 包含 0x1000）
3. 互斥的属性同时被设置

## 2. 位掩码设计原则

### 2.1 核心原则
1. **独占性原则**：每个属性必须独占不重叠的位区间
2. **对齐原则**：属性值应该对齐到其掩码的起始位
3. **预留原则**：为未来扩展预留足够的位空间
4. **分类原则**：相关属性使用相邻的位区间

### 2.2 实施规则
- 每个枚举值 = 基础值 << 起始位
- 掩码计算：((1 << 位数) - 1) << 起始位
- 避免使用跨越多个区间的位
- 单个属性使用的位数 = ⌈log2(枚举值数量)⌉

## 3. 最终位分配方案

### 3.1 位分配总览（32位flags）

| 位区间 | 掩码 | 属性 | 用途说明 |
|-------|------|------|---------|
| 0-1 | 0x0003 | FLEX_DIRECTION | flex-direction（4个值） |
| 2 | 0x0004 | LAYOUT_MODEL | display属性（block/flex） |
| 3-4 | 0x0018 | FLEX_WRAP | flex-wrap（3个值） |
| 5-7 | 0x00E0 | JUSTIFY_CONTENT | justify-content（6个值） |
| 8-10 | 0x0700 | ALIGN_ITEMS | align-items（4个值） |
| 11-13 | 0x3800 | ALIGN_CONTENT | align-content（6个值） |
| 14-16 | 0x1C000 | ALIGN_SELF | align-self（4个值） |
| 17 | 0x20000 | ITEM_INSERTED | 内部状态标志 |
| 18 | 0x40000 | SIZE_FIXED_WIDTH | 固定宽度标志 |
| 19 | 0x80000 | SIZE_FIXED_HEIGHT | 固定高度标志 |
| 20 | 0x100000 | BREAK | 换行标志 |
| 21-31 | 0xFFE00000 | RESERVED | 预留给未来扩展 |

### 3.2 位分配图示

```
31                                  21 20 19 18 17 16 15 14 13 12 11 10  9  8  7  6  5  4  3  2  1  0
|-----------------------------------|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|
|            RESERVED (11位)        | B | H | W | I |  ALIGN_SELF  |ALIGN_CONTENT |ALIGN_ITEMS |JUSTIFY |WRAP| MD |DIR|
|                                   |   |   |   |   |   (14-16)    |  (11-13)    |  (8-10)  |(5-7)|(3-4)| 2 |0-1|

位说明：
DIR: FLEX_DIRECTION (2位，0-1)       - 行/列/反向行/反向列
MD:  LAYOUT_MODEL (1位，2)           - BLOCK/FLEX
WRAP: FLEX_WRAP (2位，3-4)           - 不换行/换行/反向换行
JUSTIFY: JUSTIFY_CONTENT (3位，5-7)  - start/center/end/between/around/evenly
ALIGN_ITEMS: align-items (3位，8-10) - stretch/start/center/end
ALIGN_CONTENT: align-content (3位，11-13) - stretch/start/center/end/space-between/space-around
ALIGN_SELF: align-self (3位，14-16)  - stretch/start/center/end
I: ITEM_INSERTED (1位，17)           - item是否已插入
W: SIZE_FIXED_WIDTH (1位，18)        - 固定宽度
H: SIZE_FIXED_HEIGHT (1位，19)       - 固定高度
B: BREAK (1位，20)                   - 强制换行
```

## 4. 枚举值定义规范

### 4.1 定义公式
```c
// 基础枚举值（从0开始递增）
enum base_enum {
    VALUE_0 = 0,
    VALUE_1 = 1,
    VALUE_2 = 2,
    ...
};

// 实际枚举值 = 基础值 << 起始位
enum layx_property {
    LAYX_VALUE_0 = VALUE_0 << START_BIT,
    LAYX_VALUE_1 = VALUE_1 << START_BIT,
    LAYX_VALUE_2 = VALUE_2 << START_BIT,
    ...
};

// 掩码 = ((1 << 位数) - 1) << 起始位
#define LAYX_PROPERTY_MASK  (((1 << NUM_BITS) - 1) << START_BIT)
```

### 4.2 实际实现示例

#### 示例1: justify-content (3位，起始位5)
```c
// 需要表示6个值：3位足够 (2^3 = 8 > 6)
// 起始位: 5

typedef enum layx_justify_content {
    LAYX_JUSTIFY_FLEX_START = 0x000 << 5,  // 0x000
    LAYX_JUSTIFY_CENTER     = 0x001 << 5,  // 0x020
    LAYX_JUSTIFY_FLEX_END    = 0x002 << 5,  // 0x040
    LAYX_JUSTIFY_SPACE_BETWEEN = 0x003 << 5,  // 0x060
    LAYX_JUSTIFY_SPACE_AROUND  = 0x004 << 5,  // 0x080
    LAYX_JUSTIFY_SPACE_EVENLY  = 0x005 << 5   // 0x0A0
} layx_justify_content;

#define LAYX_JUSTIFY_CONTENT_MASK  0x00E0  // ((1 << 3) - 1) << 5
```

#### 示例2: align-items (3位，起始位8)
```c
// 需要表示4个值：2位也足够，但使用3位保持对齐
// 起始位: 8

typedef enum layx_align_items {
    LAYX_ALIGN_ITEMS_STRETCH    = 0x0 << 8,  // 0x000
    LAYX_ALIGN_ITEMS_FLEX_START = 0x1 << 8,  // 0x100
    LAYX_ALIGN_ITEMS_CENTER     = 0x2 << 8,  // 0x200
    LAYX_ALIGN_ITEMS_FLEX_END   = 0x3 << 8   // 0x300
} layx_align_items;

#define LAYX_ALIGN_ITEMS_MASK  0x0700  // ((1 << 3) - 1) << 8
```

#### 示例3: flex-direction (2位，起始位0)
```c
// 需要表示4个值：2位正好
// 起始位: 0

typedef enum layx_flex_direction {
    LAYX_FLEX_DIRECTION_ROW          = 0x0 << 0,  // 0x0
    LAYX_FLEX_DIRECTION_COLUMN       = 0x1 << 0,  // 0x1
    LAYX_FLEX_DIRECTION_ROW_REVERSE  = 0x2 << 0,  // 0x2
    LAYX_FLEX_DIRECTION_COLUMN_REVERSE = 0x3 << 0  // 0x3
} layx_flex_direction;

#define LAYX_FLEX_DIRECTION_MASK  0x0003  // ((1 << 2) - 1) << 0
```

### 4.3 单个位标志定义
```c
// 单个位的标志直接使用位值
enum {
    LAYX_LAYOUT_MODEL_FLEX    = 0x0004,  // 位2
    LAYX_ITEM_INSERTED        = 0x20000, // 位17
    LAYX_SIZE_FIXED_WIDTH     = 0x40000, // 位18
    LAYX_SIZE_FIXED_HEIGHT    = 0x80000, // 位19
    LAYX_BREAK                = 0x100000 // 位20
};
```

## 5. 使用示例

### 5.1 设置属性
```c
layx_item_t *item = layx_get_item(ctx, item_id);

// 设置flex-direction为row
item->flags &= ~LAYX_FLEX_DIRECTION_MASK;
item->flags |= LAYX_FLEX_DIRECTION_ROW;

// 设置justify-content为center
item->flags &= ~LAYX_JUSTIFY_CONTENT_MASK;
item->flags |= LAYX_JUSTIFY_CENTER;

// 设置固定宽度
item->flags |= LAYX_SIZE_FIXED_WIDTH;

// 组合设置（不冲突）
item->flags = (item->flags & ~LAYX_ALIGN_ITEMS_MASK) | LAYX_ALIGN_ITEMS_CENTER;
item->flags |= LAYX_SIZE_FIXED_WIDTH;  // 不会影响align-items
```

### 5.2 提取属性
```c
// 提取justify-content
layx_justify_content justify = (layx_justify_content)(item->flags & LAYX_JUSTIFY_CONTENT_MASK);

// 提取align-items
layx_align_items align = (layx_align_items)(item->flags & LAYX_ALIGN_ITEMS_MASK);

// 检查是否设置了固定宽度
int is_fixed_width = (item->flags & LAYX_SIZE_FIXED_WIDTH) != 0;
```

### 5.3 检查属性
```c
// 检查item是否设置了align-self覆盖
if ((item->flags & LAYX_ALIGN_SELF_MASK) != 0) {
    // 使用item的align-self
    layx_align_self align_self = (layx_align_self)(item->flags & LAYX_ALIGN_SELF_MASK);
} else {
    // 使用容器的align-items
    layx_align_items align_items = (layx_align_items)(container->flags & LAYX_ALIGN_ITEMS_MASK);
}

// 检查是否需要换行
if (item->flags & LAYX_BREAK) {
    // 强制换行
}
```

## 6. 调试和验证

### 6.1 位冲突检查工具
```c
#include <stdio.h>

void check_mask_conflicts(void) {
    printf("=== Mask Conflict Check ===\n");
    
    // 检查所有掩码是否重叠
    uint32_t masks[] = {
        LAYX_FLEX_DIRECTION_MASK,
        LAYX_LAYOUT_MODEL_MASK,
        LAYX_FLEX_WRAP_MASK,
        LAYX_JUSTIFY_CONTENT_MASK,
        LAYX_ALIGN_ITEMS_MASK,
        LAYX_ALIGN_CONTENT_MASK,
        LAYX_ALIGN_SELF_MASK
    };
    
    const char *mask_names[] = {
        "FLEX_DIRECTION",
        "LAYOUT_MODEL",
        "FLEX_WRAP",
        "JUSTIFY_CONTENT",
        "ALIGN_ITEMS",
        "ALIGN_CONTENT",
        "ALIGN_SELF"
    };
    
    for (int i = 0; i < 7; i++) {
        for (int j = i + 1; j < 7; j++) {
            if (masks[i] & masks[j]) {
                printf("CONFLICT: %s (0x%04X) & %s (0x%04X)\n",
                       mask_names[i], masks[i],
                       mask_names[j], masks[j]);
            }
        }
    }
    printf("=== End Check ===\n");
}
```

### 6.2 枚举值验证
```c
void validate_enum_values(void) {
    // 验证justify-content的值是否正确对齐到mask
    layx_justify_content values[] = {
        LAYX_JUSTIFY_FLEX_START,
        LAYX_JUSTIFY_CENTER,
        LAYX_JUSTIFY_FLEX_END,
        LAYX_JUSTIFY_SPACE_BETWEEN,
        LAYX_JUSTIFY_SPACE_AROUND,
        LAYX_JUSTIFY_SPACE_EVENLY
    };
    
    for (int i = 0; i < 6; i++) {
        uint32_t extracted = values[i] & LAYX_JUSTIFY_CONTENT_MASK;
        printf("Value %d: 0x%04X, Extracted: 0x%04X\n",
               i, values[i], extracted);
        if (extracted != values[i]) {
            printf("WARNING: Value mismatch!\n");
        }
    }
}
```

### 6.3 运行时调试
```c
void debug_item_flags(layx_item_t *item) {
    printf("Item flags: 0x%08X\n", item->flags);
    printf("  FLEX_DIRECTION: 0x%04X\n", item->flags & LAYX_FLEX_DIRECTION_MASK);
    printf("  JUSTIFY_CONTENT: 0x%04X\n", item->flags & LAYX_JUSTIFY_CONTENT_MASK);
    printf("  ALIGN_ITEMS: 0x%04X\n", item->flags & LAYX_ALIGN_ITEMS_MASK);
    printf("  ALIGN_SELF: 0x%04X\n", item->flags & LAYX_ALIGN_SELF_MASK);
    printf("  FIXED_WIDTH: %d\n", (item->flags & LAYX_SIZE_FIXED_WIDTH) != 0);
    printf("  FIXED_HEIGHT: %d\n", (item->flags & LAYX_SIZE_FIXED_HEIGHT) != 0);
}
```

## 7. 扩展指南

### 7.1 添加新属性

#### 步骤1: 确定需要的位数
```c
// 计算需要的位数
int num_values = ...; // 枚举值的数量
int num_bits = 0;
while ((1 << num_bits) < num_values) {
    num_bits++;
}
```

#### 步骤2: 在预留区选择起始位
```c
// 预留区: 位21-31 (0xFFE00000)
// 选择一个合适的起始位，确保不与现有属性冲突
#define NEW_PROPERTY_START_BIT  22
#define NEW_PROPERTY_NUM_BITS   2
```

#### 步骤3: 定义枚举和掩码
```c
typedef enum layx_new_property {
    LAYX_NEW_PROPERTY_VALUE_0 = 0x0 << 22,
    LAYX_NEW_PROPERTY_VALUE_1 = 0x1 << 22,
    LAYX_NEW_PROPERTY_VALUE_2 = 0x2 << 22,
    ...
} layx_new_property;

#define LAYX_NEW_PROPERTY_MASK  (((1 << 2) - 1) << 22)  // 0x00C00000
```

#### 步骤4: 更新文档
```c
// 更新位分配表
// Bits 22-23: NEW_PROPERTY (0x00C00000)
```

### 7.2 位空间使用情况
- 已使用: 位0-20 (21位)
- 预留: 位21-31 (11位)
- 使用率: 65.6%
- 剩余空间: 11位

### 7.3 扩展建议
1. **高频属性**：使用低位（如位21-23）
2. **未来属性**：使用高位（如位28-31）
3. **单比特标志**：优先使用位21-27
4. **多值属性**：优先使用位28-31（4位空间）

## 8. 最佳实践

### 8.1 编码规范
1. **总是使用掩码**：设置或提取属性时，先清除掩码区域
   ```c
   // ✓ 正确
   item->flags = (item->flags & ~LAYX_PROPERTY_MASK) | LAYX_VALUE;
   
   // ✗ 错误
   item->flags |= LAYX_VALUE;
   ```

2. **使用类型转换**：提取属性后转换为正确的枚举类型
   ```c
   layx_justify_content justify = 
       (layx_justify_content)(item->flags & LAYX_JUSTIFY_CONTENT_MASK);
   ```

3. **检查默认值**：枚举的第一个值应该是0（默认值）
   ```c
   typedef enum layx_property {
       LAYX_PROPERTY_DEFAULT = 0x0 << START_BIT,  // = 0，默认值
       ...
   } layx_property;
   ```

### 8.2 性能考虑
1. **位运算优化**：编译器会将位运算优化为单个指令
2. **避免移位运算**：在性能敏感代码中，预计算枚举值
3. **使用位掩码比较**：使用 `(flags & MASK) == VALUE` 而不是 `flags == VALUE`

### 8.3 调试技巧
1. **添加断言**：在关键位置添加位冲突检查
   ```c
   assert((item->flags & LAYX_PROPERTY_MASK) == expected_value);
   ```

2. **日志输出**：记录flags变化
   ```c
   printf("Setting justify-content from 0x%04X to 0x%04X\n",
          old_flags & LAYX_JUSTIFY_CONTENT_MASK,
          new_flags & LAYX_JUSTIFY_CONTENT_MASK);
   ```

3. **单元测试**：为每个属性编写测试用例
   ```c
   void test_justify_content(void) {
       item->flags = LAYX_JUSTIFY_CENTER;
       assert((item->flags & LAYX_JUSTIFY_CONTENT_MASK) == LAYX_JUSTIFY_CENTER);
       assert((item->flags & LAYX_ALIGN_ITEMS_MASK) == 0);  // 不应影响其他属性
   }
   ```

## 9. 总结

### 9.1 关键要点
1. **独占性**：每个属性独占不重叠的位区间
2. **对齐性**：枚举值对齐到掩码的起始位
3. **预留性**：为未来扩展预留足够空间
4. **规范性**：遵循统一的定义和使用规范

### 9.2 设计优势
1. **高效性**：单个uint32_t存储所有属性，节省内存
2. **快速性**：位运算快速提取和设置属性
3. **安全性**：位掩码隔离，避免属性冲突
4. **可扩展性**：预留11位用于未来扩展

### 9.3 维护建议
1. **文档同步**：每次修改位分配都要更新文档
2. **代码审查**：添加新属性时检查位冲突
3. **测试覆盖**：为所有位操作编写测试
4. **定期审计**：定期检查位空间使用情况

## 10. 附录

### 10.1 完整的位分配表

| 属性 | 起始位 | 位数 | 掩码 | 枚举数量 | 空间利用率 |
|------|-------|------|------|---------|-----------|
| FLEX_DIRECTION | 0 | 2 | 0x0003 | 4 | 100% |
| LAYOUT_MODEL | 2 | 1 | 0x0004 | 2 | 50% |
| FLEX_WRAP | 3 | 2 | 0x0018 | 3 | 75% |
| JUSTIFY_CONTENT | 5 | 3 | 0x00E0 | 6 | 75% |
| ALIGN_ITEMS | 8 | 3 | 0x0700 | 4 | 50% |
| ALIGN_CONTENT | 11 | 3 | 0x3800 | 6 | 75% |
| ALIGN_SELF | 14 | 3 | 0x1C000 | 4 | 50% |
| ITEM_INSERTED | 17 | 1 | 0x20000 | 2 | 50% |
| SIZE_FIXED_WIDTH | 18 | 1 | 0x40000 | 2 | 50% |
| SIZE_FIXED_HEIGHT | 19 | 1 | 0x80000 | 2 | 50% |
| BREAK | 20 | 1 | 0x100000 | 2 | 50% |
| RESERVED | 21-31 | 11 | 0xFFE00000 | - | - |

### 10.2 位空间利用率统计
- 已使用位：21位 (65.6%)
- 预留位：11位 (34.4%)
- 单比特标志：4个
- 多值属性：7个
- 总属性数：11个

### 10.3 常见错误案例
```c
// 错误1: 枚举值不对齐
typedef enum layx_bad_property {
    LAYX_BAD_VALUE_0 = 0x0,     // 应该是 0x0 << START_BIT
    LAYX_BAD_VALUE_1 = 0x1      // 错误：未左移到起始位
} layx_bad_property;

// 错误2: 掩码定义错误
#define LAYX_BAD_MASK  0x0F00   // 错误：起始位不对
// 应该是：((1 << NUM_BITS) - 1) << START_BIT

// 错误3: 提取时未使用掩码
layx_justify_content justify = (layx_justify_content)(item->flags);
// 正确：
layx_justify_content justify = 
    (layx_justify_content)(item->flags & LAYX_JUSTIFY_CONTENT_MASK);

// 错误4: 直接比较flags
if (item->flags == LAYX_JUSTIFY_CENTER) {
    // 错误：可能还有其他属性被设置
}
// 正确：
if ((item->flags & LAYX_JUSTIFY_CONTENT_MASK) == LAYX_JUSTIFY_CENTER) {
    // ...
}
```

---

**文档版本**: 1.0  
**最后更新**: 2026-01-08  
**维护者**: LayX Team

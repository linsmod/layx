#include <stdio.h>
#include <stdlib.h>
#include "layout.h"

const char* flex_direction_to_string(lay_flex_direction dir) {
    switch (dir) {
        case LAY_ROW: return "ROW";
        case LAY_COLUMN: return "COLUMN";
        case LAY_ROW_REVERSE: return "ROW_REVERSE";
        case LAY_COLUMN_REVERSE: return "COLUMN_REVERSE";
        default: return "UNKNOWN";
    }
}

const char* layout_model_to_string(lay_layout_model model) {
    switch (model) {
        case LAY_MODEL_LAYOUT: return "LAYOUT";
        case LAY_MODEL_FLEX: return "FLEX";
        default: return "UNKNOWN";
    }
}

const char* flex_wrap_to_string(lay_flex_wrap wrap) {
    switch (wrap) {
        case LAY_WRAP_NO: return "WRAP_NO";
        case LAY_WRAP: return "WRAP";
        case LAY_WRAP_REVERSE: return "WRAP_REVERSE";
        default: return "UNKNOWN";
    }
}

const char* justify_content_to_string(lay_justify_content justify) {
    switch (justify) {
        case LAY_JUSTIFY_START: return "JUSTIFY_START";
        case LAY_JUSTIFY_CENTER: return "JUSTIFY_CENTER";
        case LAY_JUSTIFY_END: return "JUSTIFY_END";
        case LAY_JUSTIFY_SPACE_BETWEEN: return "JUSTIFY_SPACE_BETWEEN";
        case LAY_JUSTIFY_SPACE_AROUND: return "JUSTIFY_SPACE_AROUND";
        case LAY_JUSTIFY_SPACE_EVENLY: return "JUSTIFY_SPACE_EVENLY";
        default: return "UNKNOWN";
    }
}

const char* align_items_to_string(lay_align_items align) {
    switch (align) {
        case LAY_ALIGN_STRETCH: return "ALIGN_STRETCH";
        case LAY_ALIGN_START: return "ALIGN_START";
        case LAY_ALIGN_CENTER: return "ALIGN_CENTER";
        case LAY_ALIGN_END: return "ALIGN_END";
        case LAY_ALIGN_BASELINE: return "ALIGN_BASELINE";
        default: return "UNKNOWN";
    }
}

const char* align_content_to_string(lay_align_content align) {
    switch (align) {
        case LAY_ALIGN_CONTENT_START: return "ALIGN_CONTENT_START";
        case LAY_ALIGN_CONTENT_CENTER: return "ALIGN_CONTENT_CENTER";
        case LAY_ALIGN_CONTENT_END: return "ALIGN_CONTENT_END";
        case LAY_ALIGN_CONTENT_SPACE_BETWEEN: return "ALIGN_CONTENT_SPACE_BETWEEN";
        case LAY_ALIGN_CONTENT_SPACE_AROUND: return "ALIGN_CONTENT_SPACE_AROUND";
        case LAY_ALIGN_CONTENT_STRETCH: return "ALIGN_CONTENT_STRETCH";
        default: return "UNKNOWN";
    }
}

void print_item_layout_properties(lay_context *ctx, lay_id item) {
    uint32_t contain_flags = lay_get_contain(ctx, item);

    // 提取各个属性
    lay_flex_direction direction = lay_get_flex_direction(contain_flags);
    lay_layout_model model = lay_get_layout_model(contain_flags);
    lay_flex_wrap wrap = lay_get_flex_wrap(contain_flags);
    lay_justify_content justify = lay_get_justify_content(contain_flags);
    lay_align_items align_items = lay_get_align_items(contain_flags);
    lay_align_content align_content = lay_get_align_content(contain_flags);

    printf("  Container Properties:\n");
    printf("    Direction: %s\n", flex_direction_to_string(direction));
    printf("    Model: %s\n", layout_model_to_string(model));
    printf("    Wrap: %s\n", flex_wrap_to_string(wrap));
    printf("    Justify: %s\n", justify_content_to_string(justify));
    printf("    Align Items: %s\n", align_items_to_string(align_items));
    printf("    Align Content: %s\n", align_content_to_string(align_content));
}

void print_item_info(lay_context *ctx, lay_id item, int indent) {
    lay_vec4 rect = lay_get_rect(ctx, item);

    // 打印缩进
    for (int i = 0; i < indent; i++) {
        printf("  ");
    }

    // 打印项目ID和矩形信息
    printf("Item %d: pos=(%.2f, %.2f) size=(%.2f, %.2f)\n",
           item, rect[0], rect[1], rect[2], rect[3]);

    // 递归打印子项
    lay_id child = lay_first_child(ctx, item);
    while (child != LAY_INVALID_ID) {
        print_item_info(ctx, child, indent + 1);
        child = lay_next_sibling(ctx, child);
    }
}

void print_layout_info(lay_context *ctx) {
    printf("\n=== Layout Information ===\n");
    printf("Total items: %d\n", lay_items_count(ctx));
    printf("Capacity: %d\n", lay_items_capacity(ctx));
    printf("\nItem Hierarchy:\n");
    
    if (lay_items_count(ctx) > 0) {
        print_item_info(ctx, 0, 0);
    }
    printf("===========================\n\n");
}

int main() {
    lay_context ctx;
    
    // 初始化上下文
    lay_init_context(&ctx);
    
    // 预留足够的项目容量
    lay_reserve_items_capacity(&ctx, 20);
    
    printf("=== Layout Test Example ===\n\n");
    
    // 创建根容器 (0)
    lay_id root = lay_item(&ctx);
    lay_set_size_xy(&ctx, root, 600, 400);  // 600x400 的容器
    lay_set_contain(&ctx, root, LAY_ROW | LAY_MODEL_FLEX);  // 水平布局
    lay_set_padding_ltrb(&ctx, root, 10, 10, 10, 10);  // 10px 内边距
    
    printf("Created root container (id: %d)\n", root);
    printf("  Size: 600x400\n");
    printf("  Layout: ROW|FLEX\n");
    printf("  Padding: 10px\n\n");
    
    // 创建左侧边栏
    lay_id sidebar = lay_item(&ctx);
    lay_set_size_xy(&ctx, sidebar, 150, 0);  // 宽度150, 高度自适应
    lay_set_contain(&ctx, sidebar, LAY_COLUMN | LAY_MODEL_FLEX);  // 垂直布局
    lay_set_behave(&ctx, sidebar, LAY_VFILL);  // 垂直填充
    lay_set_margins_ltrb(&ctx, sidebar, 0, 0, 10, 0);  // 右边距10
    lay_insert(&ctx, root, sidebar);
    
    printf("Created sidebar (id: %d)\n", sidebar);
    printf("  Size: 150x(auto)\n");
    printf("  Layout: COLUMN|FLEX\n");
    printf("  Behavior: VFILL\n\n");
    
    // 创建左侧边栏按钮
    lay_id btn1 = lay_item(&ctx);
    lay_set_size_xy(&ctx, btn1, 0, 40);  // 宽度自适应, 高度40
    lay_set_margins_ltrb(&ctx, btn1, 0, 0, 0, 5);  // 下边距5
    lay_insert(&ctx, sidebar, btn1);
    
    lay_id btn2 = lay_item(&ctx);
    lay_set_size_xy(&ctx, btn2, 0, 40);
    lay_set_margins_ltrb(&ctx, btn2, 0, 0, 0, 5);
    lay_insert(&ctx, sidebar, btn2);
    
    lay_id btn3 = lay_item(&ctx);
    lay_set_size_xy(&ctx, btn3, 0, 40);
    lay_insert(&ctx, sidebar, btn3);
    
    printf("Created 3 buttons in sidebar (ids: %d, %d, %d)\n", btn1, btn2, btn3);
    printf("  Size: (auto)x40\n\n");
    
    // 创建主内容区域
    lay_id content = lay_item(&ctx);
    lay_set_contain(&ctx, content, LAY_COLUMN | LAY_MODEL_FLEX);  // 垂直布局
    lay_set_behave(&ctx, content, LAY_HFILL | LAY_VFILL);  // 填充剩余空间
    lay_insert(&ctx, root, content);
    
    printf("Created main content area (id: %d)\n", content);
    printf("  Layout: COLUMN|FLEX\n");
    printf("  Behavior: HFILL|VFILL\n\n");
    
    // 创建标题栏
    lay_id header = lay_item(&ctx);
    lay_set_size_xy(&ctx, header, 0, 60);  // 宽度自适应, 高度60
    lay_set_margins_ltrb(&ctx, header, 0, 0, 0, 10);  // 下边距10
    lay_insert(&ctx, content, header);
    
    printf("Created header (id: %d)\n", header);
    printf("  Size: (auto)x60\n\n");
    
    // 创建内容区域
    lay_id body = lay_item(&ctx);
    lay_set_behave(&ctx, body, LAY_HFILL | LAY_VFILL);  // 填充剩余空间
    lay_insert(&ctx, content, body);
    
    printf("Created body (id: %d)\n", body);
    printf("  Behavior: HFILL|VFILL\n\n");
    
    // 在body中创建一些卡片
    lay_id card1 = lay_item(&ctx);
    lay_set_size_xy(&ctx, card1, 0, 100);  // 高度100
    lay_set_margins_ltrb(&ctx, card1, 0, 0, 0, 10);  // 下边距10
    lay_insert(&ctx, body, card1);
    
    lay_id card2 = lay_item(&ctx);
    lay_set_size_xy(&ctx, card2, 0, 100);
    lay_set_margins_ltrb(&ctx, card2, 0, 0, 0, 10);
    lay_insert(&ctx, body, card2);
    
    printf("Created 2 cards in body (ids: %d, %d)\n", card1, card2);
    printf("  Size: (auto)x100\n\n");
    
    // 运行布局计算
    printf("Running layout calculation...\n");
    lay_run_context(&ctx);
    
    // 打印布局信息
    print_layout_info(&ctx);
    
    // 详细打印每个项目的位置和大小
    printf("=== Detailed Item Information ===\n");
    for (lay_id i = 0; i < lay_items_count(&ctx); i++) {
        lay_vec4 rect = lay_get_rect(&ctx, i);
        printf("Item %d:\n", i);
        printf("  Position: x=%.2f, y=%.2f\n", rect[0], rect[1]);
        printf("  Size: width=%.2f, height=%.2f\n", rect[2], rect[3]);
        printf("  %s\n", lay_get_contain_str(&ctx, i));
        print_item_layout_properties(&ctx, i);
        printf("  %s\n", lay_get_behave_str(&ctx, i));
        printf("\n");
    }
    printf("=================================\n\n");
    
    // 清理
    lay_destroy_context(&ctx);
    
    printf("Layout test completed successfully!\n");
    return 0;
}

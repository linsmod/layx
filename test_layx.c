#include <stdio.h>
#include <stdlib.h>
#include "layx.h"

const char* layx_flex_direction_to_string(layx_flex_direction dir) {
    switch (dir) {
        case LAYX_FLEX_DIRECTION_ROW: return "ROW";
        case LAYX_FLEX_DIRECTION_COLUMN: return "COLUMN";
        case LAYX_FLEX_DIRECTION_ROW_REVERSE: return "ROW_REVERSE";
        case LAYX_FLEX_DIRECTION_COLUMN_REVERSE: return "COLUMN_REVERSE";
        default: return "UNKNOWN";
    }
}

const char* layx_justify_content_to_string(layx_justify_content justify) {
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

void print_item_properties(layx_context *ctx, layx_id item) {
    layx_vec4 rect = layx_get_rect(ctx, item);

    printf("Item %d: pos=(%.2f, %.2f) size=(%.2f, %.2f)\n",
           item, rect[0], rect[1], rect[2], rect[3]);
    printf("  %s\n", layx_get_layout_properties_string(ctx, item));
}

void print_layout_info(layx_context *ctx) {
    printf("\n=== Layout Information ===\n");
    printf("Total items: %d\n", layx_items_count(ctx));
    printf("Capacity: %d\n\n", layx_items_capacity(ctx));
    
    for (layx_id i = 0; i < layx_items_count(ctx); i++) {
        print_item_properties(ctx, i);
    }
    printf("===========================\n\n");
}

int main() {
    layx_context ctx;
    layx_init_context(&ctx);
    layx_reserve_items_capacity(&ctx, 20);
    
    printf("=== LAYX Test Example ===\n\n");
    
    // Create root container
    layx_id root = layx_item(&ctx);
    layx_set_size(&ctx, root, 600, 400);
    layx_set_display(&ctx, root, LAYX_DISPLAY_FLEX);
    layx_set_flex_direction(&ctx, root, LAYX_FLEX_DIRECTION_ROW);
    layx_set_padding(&ctx, root, 10);
    
    printf("Created root container (id: %d)\n", root);
    printf("  Size: 600x400\n");
    printf("  Display: FLEX\n");
    printf("  Flex Direction: ROW\n");
    printf("  Padding: 10px\n\n");
    
    // Create sidebar
    layx_id sidebar = layx_item(&ctx);
    layx_set_size(&ctx, sidebar, 150, 0);
    layx_set_display(&ctx, sidebar, LAYX_DISPLAY_FLEX);
    layx_set_flex_direction(&ctx, sidebar, LAYX_FLEX_DIRECTION_COLUMN);
    layx_set_margin_trbl(&ctx, sidebar, 0, 10, 0, 0);
    layx_insert(&ctx, root, sidebar);
    
    printf("Created sidebar (id: %d)\n", sidebar);
    printf("  Size: 150x(auto)\n");
    printf("  Display: FLEX\n");
    printf("  Flex Direction: COLUMN\n\n");
    
    // Create sidebar buttons
    layx_id btn1 = layx_item(&ctx);
    layx_set_size(&ctx, btn1, 0, 40);
    layx_set_margin_trbl(&ctx, btn1, 0, 0, 5, 0);
    layx_insert(&ctx, sidebar, btn1);
    
    layx_id btn2 = layx_item(&ctx);
    layx_set_size(&ctx, btn2, 0, 40);
    layx_set_margin_trbl(&ctx, btn2, 0, 0, 5, 0);
    layx_insert(&ctx, sidebar, btn2);
    
    layx_id btn3 = layx_item(&ctx);
    layx_set_size(&ctx, btn3, 0, 40);
    layx_insert(&ctx, sidebar, btn3);
    
    printf("Created 3 buttons in sidebar (ids: %d, %d, %d)\n", btn1, btn2, btn3);
    printf("  Size: (auto)x40\n\n");
    
    // Create main content area
    layx_id content = layx_item(&ctx);
    layx_set_display(&ctx, content, LAYX_DISPLAY_FLEX);
    layx_set_flex_direction(&ctx, content, LAYX_FLEX_DIRECTION_COLUMN);
    layx_insert(&ctx, root, content);
    
    printf("Created main content area (id: %d)\n", content);
    printf("  Display: FLEX\n");
    printf("  Flex Direction: COLUMN\n\n");
    
    // Create header
    layx_id header = layx_item(&ctx);
    layx_set_size(&ctx, header, 0, 60);
    layx_set_margin_trbl(&ctx, header, 0, 0, 10, 0);
    layx_insert(&ctx, content, header);
    
    printf("Created header (id: %d)\n", header);
    printf("  Size: (auto)x60\n\n");
    
    // Create body
    layx_id body = layx_item(&ctx);
    layx_insert(&ctx, content, body);
    
    printf("Created body (id: %d)\n\n", body);
    
    // Create cards in body
    layx_id card1 = layx_item(&ctx);
    layx_set_size(&ctx, card1, 0, 100);
    layx_set_margin_trbl(&ctx, card1, 0, 0, 10, 0);
    layx_insert(&ctx, body, card1);
    
    layx_id card2 = layx_item(&ctx);
    layx_set_size(&ctx, card2, 0, 100);
    layx_set_margin_trbl(&ctx, card2, 0, 0, 10, 0);
    layx_insert(&ctx, body, card2);
    
    printf("Created 2 cards in body (ids: %d, %d)\n", card1, card2);
    printf("  Size: (auto)x100\n\n");
    
    // Run layout calculation
    printf("Running layout calculation...\n");
    layx_run_context(&ctx);
    
    // Print layout information
    print_layout_info(&ctx);
    
    // Cleanup
    layx_destroy_context(&ctx);
    
    printf("LayX test completed successfully!\n");
    return 0;
}

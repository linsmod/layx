#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "layx.h"

// Test result tracking
static int tests_passed = 0;
static int tests_failed = 0;

// Helper: Check if two floats are approximately equal
int float_equals(float a, float b, float epsilon) {
    return fabsf(a - b) < epsilon;
}

// Helper: Assert condition
void assert_true(int condition, const char* test_name) {
    if (condition) {
        printf("✓ %s\n", test_name);
        tests_passed++;
    } else {
        printf("✗ %s\n", test_name);
        tests_failed++;
    }
}

// Helper: Print item rect
void print_rect(layx_context *ctx, layx_id item, const char* label) {
    layx_vec4 rect = layx_get_rect(ctx, item);
    printf("  %s: pos=(%.1f, %.1f) size=(%.1f, %.1f)\n",
           label, rect[0], rect[1], rect[2], rect[3]);
}

// Test 1: Horizontal Layout (Flex Row)
void test_horizontal_layout() {
    printf("\n=== Test 1: Horizontal Layout (Flex Row) ===\n");

    layx_context ctx;
    layx_init_context(&ctx);
    layx_reserve_items_capacity(&ctx, 10);

    // Create container
    layx_id container = layx_item(&ctx);
    layx_set_size(&ctx, container, 400, 100);
    layx_set_display(&ctx, container, LAYX_DISPLAY_FLEX);
    layx_set_flex_direction(&ctx, container, LAYX_FLEX_DIRECTION_ROW);
    layx_set_padding(&ctx, container, 10);

    // Create 3 items
    layx_id item1 = layx_item(&ctx);
    layx_set_size(&ctx, item1, 80, 60);
    layx_insert(&ctx, container, item1);

    layx_id item2 = layx_item(&ctx);
    layx_set_size(&ctx, item2, 100, 60);
    layx_insert(&ctx, container, item2);

    layx_id item3 = layx_item(&ctx);
    layx_set_size(&ctx, item3, 60, 60);
    layx_insert(&ctx, container, item3);

    // Run layout
    layx_run_context(&ctx);

    // Verify positions (horizontal layout: x positions increase)
    layx_vec4 rect1 = layx_get_rect(&ctx, item1);
    layx_vec4 rect2 = layx_get_rect(&ctx, item2);
    layx_vec4 rect3 = layx_get_rect(&ctx, item3);

    print_rect(&ctx, item1, "Item 1");
    print_rect(&ctx, item2, "Item 2");
    print_rect(&ctx, item3, "Item 3");

    // Items should be arranged left to right
    assert_true(rect1[0] < rect2[0], "Item 1 x < Item 2 x");
    assert_true(rect2[0] < rect3[0], "Item 2 x < Item 3 x");

    // Check padding (first item starts at 10)
    assert_true(float_equals(rect1[0], 10, 0.1f), "Item 1 starts at padding (10)");

    // Check sizes
    assert_true(float_equals(rect1[2], 80, 0.1f), "Item 1 width is 80");
    assert_true(float_equals(rect2[2], 100, 0.1f), "Item 2 width is 100");
    assert_true(float_equals(rect3[2], 60, 0.1f), "Item 3 width is 60");

    layx_destroy_context(&ctx);
}

// Test 2: Vertical Layout (Flex Column)
void test_vertical_layout() {
    printf("\n=== Test 2: Vertical Layout (Flex Column) ===\n");

    layx_context ctx;
    layx_init_context(&ctx);
    layx_reserve_items_capacity(&ctx, 10);

    // Create container
    layx_id container = layx_item(&ctx);
    layx_set_size(&ctx, container, 100, 400);
    layx_set_display(&ctx, container, LAYX_DISPLAY_FLEX);
    layx_set_flex_direction(&ctx, container, LAYX_FLEX_DIRECTION_COLUMN);
    layx_set_padding(&ctx, container, 10);

    // Create 3 items
    layx_id item1 = layx_item(&ctx);
    layx_set_size(&ctx, item1, 60, 80);
    layx_insert(&ctx, container, item1);

    layx_id item2 = layx_item(&ctx);
    layx_set_size(&ctx, item2, 60, 100);
    layx_insert(&ctx, container, item2);

    layx_id item3 = layx_item(&ctx);
    layx_set_size(&ctx, item3, 60, 60);
    layx_insert(&ctx, container, item3);

    // Run layout
    layx_run_context(&ctx);

    // Verify positions (vertical layout: y positions increase)
    layx_vec4 rect1 = layx_get_rect(&ctx, item1);
    layx_vec4 rect2 = layx_get_rect(&ctx, item2);
    layx_vec4 rect3 = layx_get_rect(&ctx, item3);

    print_rect(&ctx, item1, "Item 1");
    print_rect(&ctx, item2, "Item 2");
    print_rect(&ctx, item3, "Item 3");

    // Items should be arranged top to bottom
    assert_true(rect1[1] < rect2[1], "Item 1 y < Item 2 y");
    assert_true(rect2[1] < rect3[1], "Item 2 y < Item 3 y");

    // Check padding (first item starts at 10)
    assert_true(float_equals(rect1[1], 10, 0.1f), "Item 1 starts at padding (10)");

    // Check sizes
    assert_true(float_equals(rect1[3], 80, 0.1f), "Item 1 height is 80");
    assert_true(float_equals(rect2[3], 100, 0.1f), "Item 2 height is 100");
    assert_true(float_equals(rect3[3], 60, 0.1f), "Item 3 height is 60");

    layx_destroy_context(&ctx);
}

// Test 3: Justify Content (Flex Start, Center, Flex End)
void test_justify_content() {
    printf("\n=== Test 3: Justify Content ===\n");

    // Test FLEX_START
    {
        layx_context ctx;
        layx_init_context(&ctx);

        layx_id container = layx_item(&ctx);
        layx_set_size(&ctx, container, 400, 100);
        layx_set_display(&ctx, container, LAYX_DISPLAY_FLEX);
        layx_set_flex_direction(&ctx, container, LAYX_FLEX_DIRECTION_ROW);
        layx_set_justify_content(&ctx, container, LAYX_JUSTIFY_FLEX_START);

        layx_id item = layx_item(&ctx);
        layx_set_size(&ctx, item, 50, 50);
        layx_insert(&ctx, container, item);

        layx_run_context(&ctx);
        layx_vec4 rect = layx_get_rect(&ctx, item);

        assert_true(rect[0] < 50, "FLEX_START: Item is near left");

        layx_destroy_context(&ctx);
    }

    // Test CENTER
    {
        layx_context ctx;
        layx_init_context(&ctx);

        layx_id container = layx_item(&ctx);
        layx_set_size(&ctx, container, 400, 100);
        layx_set_display(&ctx, container, LAYX_DISPLAY_FLEX);
        layx_set_flex_direction(&ctx, container, LAYX_FLEX_DIRECTION_ROW);
        layx_set_justify_content(&ctx, container, LAYX_JUSTIFY_CENTER);
        layx_set_padding(&ctx, container, 0);

        layx_id item = layx_item(&ctx);
        layx_set_size(&ctx, item, 50, 50);
        layx_insert(&ctx, container, item);

        layx_run_context(&ctx);
        layx_vec4 rect = layx_get_rect(&ctx, item);

        assert_true(float_equals(rect[0], 175, 5.0f), "CENTER: Item is centered (400-50)/2");

        layx_destroy_context(&ctx);
    }

    // Test FLEX_END
    {
        layx_context ctx;
        layx_init_context(&ctx);

        layx_id container = layx_item(&ctx);
        layx_set_size(&ctx, container, 400, 100);
        layx_set_display(&ctx, container, LAYX_DISPLAY_FLEX);
        layx_set_flex_direction(&ctx, container, LAYX_FLEX_DIRECTION_ROW);
        layx_set_justify_content(&ctx, container, LAYX_JUSTIFY_FLEX_END);

        layx_id item = layx_item(&ctx);
        layx_set_size(&ctx, item, 50, 50);
        layx_insert(&ctx, container, item);

        layx_run_context(&ctx);
        layx_vec4 rect = layx_get_rect(&ctx, item);

        assert_true(rect[0] >= 350, "FLEX_END: Item is near right");

        layx_destroy_context(&ctx);
    }
}

// Test 4: Sidebar Layout
void test_sidebar_layout() {
    printf("\n=== Test 4: Sidebar Layout ===\n");

    layx_context ctx;
    layx_init_context(&ctx);
    layx_reserve_items_capacity(&ctx, 20);

    // Root container
    layx_id root = layx_item(&ctx);
    layx_set_size(&ctx, root, 800, 600);
    layx_set_display(&ctx, root, LAYX_DISPLAY_FLEX);
    layx_set_flex_direction(&ctx, root, LAYX_FLEX_DIRECTION_ROW);

    // Sidebar
    layx_id sidebar = layx_item(&ctx);
    layx_set_size(&ctx, sidebar, 200, 0);
    layx_set_display(&ctx, sidebar, LAYX_DISPLAY_FLEX);
    layx_set_flex_direction(&ctx, sidebar, LAYX_FLEX_DIRECTION_COLUMN);
    layx_insert(&ctx, root, sidebar);

    // Sidebar items
    layx_id sidebar_item1 = layx_item(&ctx);
    layx_set_size(&ctx, sidebar_item1, 0, 50);
    layx_set_margin_trbl(&ctx, sidebar_item1, 0, 0, 10, 0);
    layx_insert(&ctx, sidebar, sidebar_item1);

    layx_id sidebar_item2 = layx_item(&ctx);
    layx_set_size(&ctx, sidebar_item2, 0, 50);
    layx_set_margin_trbl(&ctx, sidebar_item2, 0, 0, 10, 0);
    layx_insert(&ctx, sidebar, sidebar_item2);

    // Main content
    layx_id main_content = layx_item(&ctx);
    layx_set_display(&ctx, main_content, LAYX_DISPLAY_FLEX);
    layx_set_flex_direction(&ctx, main_content, LAYX_FLEX_DIRECTION_COLUMN);
    layx_insert(&ctx, root, main_content);

    // Header
    layx_id header = layx_item(&ctx);
    layx_set_size(&ctx, header, 0, 60);
    layx_insert(&ctx, main_content, header);

    // Content body
    layx_id body = layx_item(&ctx);
    layx_set_size(&ctx, body, 0, 400);
    layx_insert(&ctx, main_content, body);

    layx_run_context(&ctx);

    layx_vec4 sidebar_rect = layx_get_rect(&ctx, sidebar);
    layx_vec4 main_rect = layx_get_rect(&ctx, main_content);

    print_rect(&ctx, sidebar, "Sidebar");
    print_rect(&ctx, main_content, "Main Content");

    // Sidebar should be on the left
    assert_true(sidebar_rect[0] < main_rect[0], "Sidebar is left of main content");
    // Sidebar width should be 200
    assert_true(float_equals(sidebar_rect[2], 200, 0.1f), "Sidebar width is 200");

    layx_destroy_context(&ctx);
}

// Test 5: Navbar Layout
void test_navbar_layout() {
    printf("\n=== Test 5: Navbar Layout ===\n");

    layx_context ctx;
    layx_init_context(&ctx);
    layx_reserve_items_capacity(&ctx, 10);

    // Navbar container
    layx_id navbar = layx_item(&ctx);
    layx_set_size(&ctx, navbar, 800, 60);
    layx_set_display(&ctx, navbar, LAYX_DISPLAY_FLEX);
    layx_set_flex_direction(&ctx, navbar, LAYX_FLEX_DIRECTION_ROW);
    layx_set_justify_content(&ctx, navbar, LAYX_JUSTIFY_SPACE_BETWEEN);

    // Logo
    layx_id logo = layx_item(&ctx);
    layx_set_size(&ctx, logo, 100, 40);
    layx_insert(&ctx, navbar, logo);

    // Links container
    layx_id links = layx_item(&ctx);
    layx_set_display(&ctx, links, LAYX_DISPLAY_FLEX);
    layx_set_flex_direction(&ctx, links, LAYX_FLEX_DIRECTION_ROW);
    layx_insert(&ctx, navbar, links);

    // Nav links
    layx_id link1 = layx_item(&ctx);
    layx_set_size(&ctx, link1, 80, 40);
    layx_set_margin_trbl(&ctx, link1, 0, 10, 0, 0);
    layx_insert(&ctx, links, link1);

    layx_id link2 = layx_item(&ctx);
    layx_set_size(&ctx, link2, 80, 40);
    layx_set_margin_trbl(&ctx, link2, 0, 10, 0, 0);
    layx_insert(&ctx, links, link2);

    layx_id link3 = layx_item(&ctx);
    layx_set_size(&ctx, link3, 80, 40);
    layx_insert(&ctx, links, link3);

    layx_run_context(&ctx);

    layx_vec4 logo_rect = layx_get_rect(&ctx, logo);
    layx_vec4 links_rect = layx_get_rect(&ctx, links);

    print_rect(&ctx, logo, "Logo");
    print_rect(&ctx, links, "Links");

    // Logo should be on the left, links on the right (space-between)
    assert_true(logo_rect[0] < links_rect[0], "Logo is left of links");
    // Logo should be near left edge
    assert_true(logo_rect[0] < 450, "Logo is near left edge");
    // Links should be near right edge
    assert_true(links_rect[0] > 400, "Links are near right edge");

    layx_destroy_context(&ctx);
}

// Test 6: Card Grid Layout (Nested Layouts)
void test_card_grid_layout() {
    printf("\n=== Test 6: Card Grid Layout ===\n");

    layx_context ctx;
    layx_init_context(&ctx);
    layx_reserve_items_capacity(&ctx, 20);

    // Container
    layx_id container = layx_item(&ctx);
    layx_set_size(&ctx, container, 600, 400);
    layx_set_display(&ctx, container, LAYX_DISPLAY_FLEX);
    layx_set_flex_direction(&ctx, container, LAYX_FLEX_DIRECTION_COLUMN);
    layx_set_padding(&ctx, container, 20);

    // Row 1
    layx_id row1 = layx_item(&ctx);
    layx_set_size(&ctx, row1, 0, 120);
    layx_set_display(&ctx, row1, LAYX_DISPLAY_FLEX);
    layx_set_flex_direction(&ctx, row1, LAYX_FLEX_DIRECTION_ROW);
    layx_insert(&ctx, container, row1);

    layx_id card1 = layx_item(&ctx);
    layx_set_size(&ctx, card1, 100, 100);
    layx_set_margin_trbl(&ctx, card1, 0, 10, 0, 0);
    layx_insert(&ctx, row1, card1);

    layx_id card2 = layx_item(&ctx);
    layx_set_size(&ctx, card2, 100, 100);
    layx_set_margin_trbl(&ctx, card2, 0, 10, 0, 0);
    layx_insert(&ctx, row1, card2);

    layx_id card3 = layx_item(&ctx);
    layx_set_size(&ctx, card3, 100, 100);
    layx_insert(&ctx, row1, card3);

    // Row 2
    layx_id row2 = layx_item(&ctx);
    layx_set_size(&ctx, row2, 0, 120);
    layx_set_display(&ctx, row2, LAYX_DISPLAY_FLEX);
    layx_set_flex_direction(&ctx, row2, LAYX_FLEX_DIRECTION_ROW);
    layx_set_margin_trbl(&ctx, row2, 10, 0, 0, 0);
    layx_insert(&ctx, container, row2);

    layx_id card4 = layx_item(&ctx);
    layx_set_size(&ctx, card4, 100, 100);
    layx_set_margin_trbl(&ctx, card4, 0, 10, 0, 0);
    layx_insert(&ctx, row2, card4);

    layx_id card5 = layx_item(&ctx);
    layx_set_size(&ctx, card5, 100, 100);
    layx_set_margin_trbl(&ctx, card5, 0, 10, 0, 0);
    layx_insert(&ctx, row2, card5);

    layx_id card6 = layx_item(&ctx);
    layx_set_size(&ctx, card6, 100, 100);
    layx_insert(&ctx, row2, card6);

    layx_run_context(&ctx);

    layx_vec4 rect1 = layx_get_rect(&ctx, card1);
    layx_vec4 rect2 = layx_get_rect(&ctx, card2);
    layx_vec4 rect4 = layx_get_rect(&ctx, card4);

    print_rect(&ctx, card1, "Card 1");
    print_rect(&ctx, card2, "Card 2");
    print_rect(&ctx, card4, "Card 4");

    // Row 1 cards should be arranged horizontally
    assert_true(rect1[0] < rect2[0], "Card 1 left of Card 2");
    // Row 2 should be below Row 1
    assert_true(rect1[1] < rect4[1], "Row 1 above Row 2");

    layx_destroy_context(&ctx);
}

// Test 7: Form Layout
void test_form_layout() {
    printf("\n=== Test 7: Form Layout ===\n");

    layx_context ctx;
    layx_init_context(&ctx);
    layx_reserve_items_capacity(&ctx, 20);

    // Form container
    layx_id form = layx_item(&ctx);
    layx_set_size(&ctx, form, 400, 0);
    layx_set_display(&ctx, form, LAYX_DISPLAY_FLEX);
    layx_set_flex_direction(&ctx, form, LAYX_FLEX_DIRECTION_COLUMN);
    layx_set_padding(&ctx, form, 20);

    // Form fields
    for (int i = 0; i < 4; i++) {
        layx_id field = layx_item(&ctx);
        layx_set_size(&ctx, field, 360, 40);
        layx_set_margin_trbl(&ctx, field, 0, 0, 10, 0);
        layx_insert(&ctx, form, field);
    }

    // Submit button
    layx_id button = layx_item(&ctx);
    layx_set_size(&ctx, button, 100, 40);
    layx_set_margin_trbl(&ctx, button, 10, 0, 0, 0);
    layx_insert(&ctx, form, button);

    layx_run_context(&ctx);

    // All fields should have the same width
    layx_vec4 rect1 = layx_get_rect(&ctx, form);

    print_rect(&ctx, form, "Form");
    print_rect(&ctx, 2, "Field 1");

    // Form should accommodate all fields
    assert_true(rect1[3] > 200, "Form height > 200 (4 fields + button)");

    layx_destroy_context(&ctx);
}

// Test 8: Margin and Padding
void test_margin_and_padding() {
    printf("\n=== Test 8: Margin and Padding ===\n");

    layx_context ctx;
    layx_init_context(&ctx);

    // Container with padding
    layx_id container = layx_item(&ctx);
    layx_set_size(&ctx, container, 400, 200);
    layx_set_display(&ctx, container, LAYX_DISPLAY_FLEX);
    layx_set_padding(&ctx, container, 20);

    // Item with margin
    layx_id item = layx_item(&ctx);
    layx_set_size(&ctx, item, 100, 50);
    layx_set_margin_trbl(&ctx, item, 10, 20, 10, 20);
    layx_insert(&ctx, container, item);

    layx_run_context(&ctx);

    layx_vec4 rect = layx_get_rect(&ctx, item);

    print_rect(&ctx, item, "Item with margin");

    // Item should start at padding (20) + margin (20) = 40
    assert_true(float_equals(rect[0], 40, 0.1f), "Item x position includes padding and margin");
    // Item y should be padding (20) + margin (10) = 30
    assert_true(float_equals(rect[1], 30, 0.1f), "Item y position includes padding and margin");

    layx_destroy_context(&ctx);
}

// Test 9: Nested Layouts
void test_nested_layouts() {
    printf("\n=== Test 9: Nested Layouts ===\n");

    layx_context ctx;
    layx_init_context(&ctx);
    layx_reserve_items_capacity(&ctx, 30);

    // Outer container (row)
    layx_id outer = layx_item(&ctx);
    layx_set_size(&ctx, outer, 600, 400);
    layx_set_display(&ctx, outer, LAYX_DISPLAY_FLEX);
    layx_set_flex_direction(&ctx, outer, LAYX_FLEX_DIRECTION_ROW);

    // Left panel (column)
    layx_id left = layx_item(&ctx);
    layx_set_size(&ctx, left, 200, 0);
    layx_set_display(&ctx, left, LAYX_DISPLAY_FLEX);
    layx_set_flex_direction(&ctx, left, LAYX_FLEX_DIRECTION_COLUMN);
    layx_insert(&ctx, outer, left);

    // Items in left panel
    for (int i = 0; i < 3; i++) {
        layx_id item = layx_item(&ctx);
        layx_set_size(&ctx, item, 150, 50);
        layx_set_margin_trbl(&ctx, item, 0, 0, 10, 0);
        layx_insert(&ctx, left, item);
    }

    // Right panel (column)
    layx_id right = layx_item(&ctx);
    layx_set_size(&ctx, right, 0, 0);
    layx_set_display(&ctx, right, LAYX_DISPLAY_FLEX);
    layx_set_flex_direction(&ctx, right, LAYX_FLEX_DIRECTION_COLUMN);
    layx_insert(&ctx, outer, right);

    // Items in right panel
    for (int i = 0; i < 4; i++) {
        layx_id item = layx_item(&ctx);
        layx_set_size(&ctx, item, 150, 40);
        layx_set_margin_trbl(&ctx, item, 0, 0, 8, 0);
        layx_insert(&ctx, right, item);
    }

    layx_run_context(&ctx);

    layx_vec4 left_rect = layx_get_rect(&ctx, left);
    layx_vec4 right_rect = layx_get_rect(&ctx, right);

    print_rect(&ctx, left, "Left Panel");
    print_rect(&ctx, right, "Right Panel");

    // Left panel should be on the left
    assert_true(left_rect[0] < right_rect[0], "Left panel is on the left");
    // Left panel width should be 200
    assert_true(float_equals(left_rect[2], 200, 0.1f), "Left panel width is 200");

    layx_destroy_context(&ctx);
}

// Test 10: Flex Grow and Shrink
void test_flex_grow_shrink() {
    printf("\n=== Test 10: Flex Grow and Shrink ===\n");

    layx_context ctx;
    layx_init_context(&ctx);

    // Container
    layx_id container = layx_item(&ctx);
    layx_set_size(&ctx, container, 400, 100);
    layx_set_display(&ctx, container, LAYX_DISPLAY_FLEX);
    layx_set_flex_direction(&ctx, container, LAYX_FLEX_DIRECTION_ROW);

    // Fixed width item
    layx_id item1 = layx_item(&ctx);
    layx_set_size(&ctx, item1, 100, 80);
    layx_insert(&ctx, container, item1);

    // Flexible item (flex-grow: 1)
    layx_id item2 = layx_item(&ctx);
    layx_set_size(&ctx, item2, 0, 80);
    layx_set_flex_grow(&ctx, item2, 1);
    layx_insert(&ctx, container, item2);

    layx_run_context(&ctx);

    layx_vec4 rect1 = layx_get_rect(&ctx, item1);
    layx_vec4 rect2 = layx_get_rect(&ctx, item2);

    print_rect(&ctx, item1, "Fixed Item");
    print_rect(&ctx, item2, "Flexible Item");

    // Item 1 should have fixed width
    assert_true(float_equals(rect1[2], 100, 0.1f), "Item 1 has fixed width 100");
    // Item 2 should fill remaining space
    assert_true(rect2[2] > 200, "Item 2 fills remaining space");

    layx_destroy_context(&ctx);
}

// Test 11: Align Items (Cross-axis alignment)
void test_align_items() {
    printf("\n=== Test 11: Align Items ===\n");

    // Test ALIGN_START (top for row)
    {
        layx_context ctx;
        layx_init_context(&ctx);

        layx_id container = layx_item(&ctx);
        layx_set_size(&ctx, container, 400, 100);
        layx_set_display(&ctx, container, LAYX_DISPLAY_FLEX);
        layx_set_flex_direction(&ctx, container, LAYX_FLEX_DIRECTION_ROW);
        layx_set_align_items(&ctx, container, LAYX_ALIGN_ITEMS_FLEX_START);

        layx_id item = layx_item(&ctx);
        layx_set_size(&ctx, item, 50, 50);
        layx_insert(&ctx, container, item);

        layx_run_context(&ctx);
        layx_vec4 rect = layx_get_rect(&ctx, item);

        assert_true(rect[1] < 50, "ALIGN_START: Item is near top");

        layx_destroy_context(&ctx);
    }

    // Test ALIGN_CENTER
    {
        layx_context ctx;
        layx_init_context(&ctx);

        layx_id container = layx_item(&ctx);
        layx_set_size(&ctx, container, 400, 100);
        layx_set_display(&ctx, container, LAYX_DISPLAY_FLEX);
        layx_set_flex_direction(&ctx, container, LAYX_FLEX_DIRECTION_ROW);
        layx_set_align_items(&ctx, container, LAYX_ALIGN_ITEMS_CENTER);

        layx_id item = layx_item(&ctx);
        layx_set_size(&ctx, item, 50, 50);
        layx_insert(&ctx, container, item);

        layx_run_context(&ctx);
        layx_vec4 rect = layx_get_rect(&ctx, item);

        assert_true(float_equals(rect[1], 25, 5.0f), "ALIGN_CENTER: Item is centered vertically");

        layx_destroy_context(&ctx);
    }

    // Test ALIGN_END (bottom for row)
    {
        layx_context ctx;
        layx_init_context(&ctx);

        layx_id container = layx_item(&ctx);
        layx_set_size(&ctx, container, 400, 100);
        layx_set_display(&ctx, container, LAYX_DISPLAY_FLEX);
        layx_set_flex_direction(&ctx, container, LAYX_FLEX_DIRECTION_ROW);
        layx_set_align_items(&ctx, container, LAYX_ALIGN_ITEMS_FLEX_END);

        layx_id item = layx_item(&ctx);
        layx_set_size(&ctx, item, 50, 50);
        layx_insert(&ctx, container, item);

        layx_run_context(&ctx);
        layx_vec4 rect = layx_get_rect(&ctx, item);

        assert_true(rect[1] >= 50, "ALIGN_END: Item is near bottom");

        layx_destroy_context(&ctx);
    }
}

// Test 12: Space Between, Around, and Evenly
void test_space_distribution() {
    printf("\n=== Test 12: Space Distribution ===\n");

    // SPACE_BETWEEN
    {
        layx_context ctx;
        layx_init_context(&ctx);

        layx_id container = layx_item(&ctx);
        layx_set_size(&ctx, container, 400, 100);
        layx_set_display(&ctx, container, LAYX_DISPLAY_FLEX);
        layx_set_flex_direction(&ctx, container, LAYX_FLEX_DIRECTION_ROW);
        layx_set_justify_content(&ctx, container, LAYX_JUSTIFY_SPACE_BETWEEN);

        layx_id item1 = layx_item(&ctx);
        layx_set_size(&ctx, item1, 50, 50);
        layx_insert(&ctx, container, item1);

        layx_id item2 = layx_item(&ctx);
        layx_set_size(&ctx, item2, 50, 50);
        layx_insert(&ctx, container, item2);

        layx_run_context(&ctx);
        layx_vec4 rect1 = layx_get_rect(&ctx, item1);
        layx_vec4 rect2 = layx_get_rect(&ctx, item2);

        // First item should be at left edge
        assert_true(rect1[0] < 100, "SPACE_BETWEEN: First item at left");
        // Second item should be at right edge
        assert_true(rect2[0] > 340, "SPACE_BETWEEN: Second item at right");

        layx_destroy_context(&ctx);
    }

    // SPACE_AROUND
    {
        layx_context ctx;
        layx_init_context(&ctx);

        layx_id container = layx_item(&ctx);
        layx_set_size(&ctx, container, 400, 100);
        layx_set_display(&ctx, container, LAYX_DISPLAY_FLEX);
        layx_set_flex_direction(&ctx, container, LAYX_FLEX_DIRECTION_ROW);
        layx_set_justify_content(&ctx, container, LAYX_JUSTIFY_SPACE_AROUND);

        layx_id item = layx_item(&ctx);
        layx_set_size(&ctx, item, 50, 50);
        layx_insert(&ctx, container, item);

        layx_run_context(&ctx);
        layx_vec4 rect = layx_get_rect(&ctx, item);

        // Item should be centered (space around)
        assert_true(float_equals(rect[0], 175, 5.0f), "SPACE_AROUND: Item centered");

        layx_destroy_context(&ctx);
    }

    // SPACE_EVENLY
    {
        layx_context ctx;
        layx_init_context(&ctx);

        layx_id container = layx_item(&ctx);
        layx_set_size(&ctx, container, 400, 100);
        layx_set_display(&ctx, container, LAYX_DISPLAY_FLEX);
        layx_set_flex_direction(&ctx, container, LAYX_FLEX_DIRECTION_ROW);
        layx_set_justify_content(&ctx, container, LAYX_JUSTIFY_SPACE_EVENLY);

        layx_id item = layx_item(&ctx);
        layx_set_size(&ctx, item, 50, 50);
        layx_insert(&ctx, container, item);

        layx_run_context(&ctx);
        layx_vec4 rect = layx_get_rect(&ctx, item);

        // Item should be centered
        assert_true(float_equals(rect[0], 175, 5.0f), "SPACE_EVENLY: Item centered");

        layx_destroy_context(&ctx);
    }
}

int main() {
    printf("===========================================\n");
    printf("   LAYX Common Layout Patterns Test Suite\n");
    printf("===========================================\n");

    // Run all tests
    test_horizontal_layout();
    test_vertical_layout();
    test_justify_content();
    test_sidebar_layout();
    test_navbar_layout();
    test_card_grid_layout();
    test_form_layout();
    test_margin_and_padding();
    test_nested_layouts();
    test_flex_grow_shrink();
    test_align_items();
    test_space_distribution();

    // Print summary
    printf("\n===========================================\n");
    printf("           Test Summary\n");
    printf("===========================================\n");
    printf("Tests Passed: %d\n", tests_passed);
    printf("Tests Failed: %d\n", tests_failed);
    printf("Total Tests:  %d\n", tests_passed + tests_failed);
    printf("===========================================\n");

    if (tests_failed == 0) {
        printf("\n✓ All tests passed!\n");
    } else {
        printf("\n✗ Some tests failed!\n");
    }
    return 0;
}
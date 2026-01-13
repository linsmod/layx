/**
 * @file test_scroll_max.c
 * @brief 专门测试 scroll_max (最大滚动范围) 功能
 */

#include <stdio.h>
#include <math.h>
#include "layx.h"

// 测试计数
static int tests_passed = 0;
static int tests_failed = 0;

// 辅助函数：检查两个浮点数是否近似相等
static int float_equals(float a, float b, float epsilon) {
    return fabsf(a - b) < epsilon;
}

// 辅助函数：测试断言
#define TEST_ASSERT(condition, message) \
    do { \
        if (condition) { \
            printf("  ✓ %s\n", message); \
            tests_passed++; \
        } else { \
            printf("  ✗ %s\n", message); \
            tests_failed++; \
        } \
    } while(0)

// 辅助函数：打印向量内容
void print_vec2(const char* name, layx_vec2 vec) {
#if defined(__GNUC__) || defined(__clang__)
    printf("%s: (%.2f, %.2f)\n", name, vec[0], vec[1]);
#else
    printf("%s: (%.2f, %.2f)\n", name, vec.xy[0], vec.xy[1]);
#endif
}

/**
 * 测试1: 内容不溢出时scroll_max应为0
 */
void test_scroll_max_no_overflow(void) {
    printf("\n=== Test 1: scroll_max Should Be 0 When Content Doesn't Overflow ===\n");

    layx_context ctx;
    layx_init_context(&ctx);
    layx_reserve_items_capacity(&ctx, 10);

    // 创建容器
    layx_id container = layx_item(&ctx);
    layx_set_size(&ctx, container, 200, 150);
    layx_set_overflow(&ctx, container, LAYX_OVERFLOW_AUTO);

    // 创建子项，内容不溢出容器
    layx_id child = layx_item(&ctx);
    layx_set_size(&ctx, child, 100, 100);
    layx_push(&ctx, container, child);

    // 运行布局
    layx_run_context(&ctx);

    // 获取scroll_max
    layx_vec2 scroll_max;
    layx_get_scroll_max(&ctx, container, &scroll_max);

    // 验证scroll_max应该为0
    TEST_ASSERT(float_equals(scroll_max[0], 0.0f, 0.01f), "内容宽度100 < 容器宽度200时，scroll_max[0]应为0");
    TEST_ASSERT(float_equals(scroll_max[1], 0.0f, 0.01f), "内容高度100 < 容器高度150时，scroll_max[1]应为0");

    // 验证内容尺寸
    layx_vec2 content_size;
    layx_get_content_size(&ctx, container, &content_size);
    TEST_ASSERT(content_size[0] <= 200.0f, "内容宽度应不大于容器宽度");
    TEST_ASSERT(content_size[1] <= 150.0f, "内容高度应不大于容器高度");

    layx_destroy_context(&ctx);
}

/**
 * 测试2: 内容溢出时scroll_max正确计算
 */
void test_scroll_max_with_overflow(void) {
    printf("\n=== Test 2: scroll_max Correct When Content Overflows ===\n");

    layx_context ctx;
    layx_init_context(&ctx);
    layx_reserve_items_capacity(&ctx, 10);

    layx_id container = layx_item(&ctx);
    layx_set_size(&ctx, container, 200, 150);
    layx_set_overflow(&ctx, container, LAYX_OVERFLOW_AUTO);

    // 创建宽度超出容器的子项
    layx_id child = layx_item(&ctx);
    layx_set_size(&ctx, child, 300, 100);
    layx_push(&ctx, container, child);

    layx_run_context(&ctx);

    layx_vec2 scroll_max, content_size;
    layx_get_scroll_max(&ctx, container, &scroll_max);
    layx_get_content_size(&ctx, container, &content_size);

    // scroll_max = content_size - client_size
    // 客户区宽度 = 200 (容器宽度)
    // 内容宽度应该包含子项宽度300
    // scroll_max[0] 应该约为 300 - 200 = 100
    printf("  content_size: (%.2f, %.2f)\n", content_size[0], content_size[1]);
    printf("  scroll_max: (%.2f, %.2f)\n", scroll_max[0], scroll_max[1]);

    TEST_ASSERT(scroll_max[0] > 0.0f, "内容宽度300 > 容器宽度200时，scroll_max[0]应大于0");
    TEST_ASSERT(scroll_max[0] >= 90.0f && scroll_max[0] <= 110.0f, "scroll_max[0]应该约为100 (300-200)");
    TEST_ASSERT(float_equals(scroll_max[1], 0.0f, 0.01f), "内容高度100 < 容器高度150时，scroll_max[1]应为0");

    layx_destroy_context(&ctx);
}

/**
 * 测试3: padding对scroll_max的影响
 */
void test_scroll_max_with_padding(void) {
    printf("\n=== Test 3: scroll_max with Padding ===\n");

    layx_context ctx;
    layx_init_context(&ctx);
    layx_reserve_items_capacity(&ctx, 10);

    layx_id container = layx_item(&ctx);
    layx_set_size(&ctx, container, 200, 150);
    layx_set_padding(&ctx, container, 10);  // padding: 10px
    layx_set_overflow(&ctx, container, LAYX_OVERFLOW_AUTO);

    layx_id child = layx_item(&ctx);
    layx_set_size(&ctx, child, 300, 100);
    layx_push(&ctx, container, child);

    layx_run_context(&ctx);

    layx_vec2 scroll_max, content_size;
    layx_get_scroll_max(&ctx, container, &scroll_max);
    layx_get_content_size(&ctx, container, &content_size);

    // 客户区宽度 = 200 - 10 - 10 = 180
    // 内容宽度应该包含子项宽度300
    // scroll_max[0] = 300 - 180 = 120
    printf("  content_size: (%.2f, %.2f)\n", content_size[0], content_size[1]);
    printf("  scroll_max: (%.2f, %.2f)\n", scroll_max[0], scroll_max[1]);

    TEST_ASSERT(scroll_max[0] > 0.0f, "scroll_max[0]应大于0");
    TEST_ASSERT(scroll_max[0] >= 110.0f && scroll_max[0] <= 130.0f, "scroll_max[0]应该约为120 (考虑padding)");
    TEST_ASSERT(float_equals(scroll_max[1], 0.0f, 0.01f), "scroll_max[1]应为0");

    layx_destroy_context(&ctx);
}

/**
 * 测试4: border对scroll_max的影响
 */
void test_scroll_max_with_border(void) {
    printf("\n=== Test 4: scroll_max with Border ===\n");

    layx_context ctx;
    layx_init_context(&ctx);
    layx_reserve_items_capacity(&ctx, 10);

    layx_id container = layx_item(&ctx);
    layx_set_size(&ctx, container, 200, 150);
    layx_set_border(&ctx, container, 5);  // border: 5px
    layx_set_overflow(&ctx, container, LAYX_OVERFLOW_AUTO);

    layx_id child = layx_item(&ctx);
    layx_set_size(&ctx, child, 300, 100);
    layx_push(&ctx, container, child);

    layx_run_context(&ctx);

    layx_vec2 scroll_max, content_size;
    layx_get_scroll_max(&ctx, container, &scroll_max);
    layx_get_content_size(&ctx, container, &content_size);

    // 客户区宽度 = 200 - 5 - 5 = 190
    // 内容宽度应该包含子项宽度300
    // scroll_max[0] = 300 - 190 = 110
    printf("  content_size: (%.2f, %.2f)\n", content_size[0], content_size[1]);
    printf("  scroll_max: (%.2f, %.2f)\n", scroll_max[0], scroll_max[1]);

    TEST_ASSERT(scroll_max[0] >= 100.0f && scroll_max[0] <= 120.0f, "scroll_max[0]应该约为110 (考虑border)");

    layx_destroy_context(&ctx);
}

/**
 * 测试5: 同时有padding和border
 */
void test_scroll_max_with_padding_and_border(void) {
    printf("\n=== Test 5: scroll_max with Both Padding and Border ===\n");

    layx_context ctx;
    layx_init_context(&ctx);
    layx_reserve_items_capacity(&ctx, 10);

    layx_id container = layx_item(&ctx);
    layx_set_size(&ctx, container, 200, 150);
    layx_set_padding(&ctx, container, 10);
    layx_set_border(&ctx, container, 5);
    layx_set_overflow(&ctx, container, LAYX_OVERFLOW_AUTO);

    layx_id child = layx_item(&ctx);
    layx_set_size(&ctx, child, 300, 100);
    layx_push(&ctx, container, child);

    layx_run_context(&ctx);

    layx_vec2 scroll_max, content_size;
    layx_get_scroll_max(&ctx, container, &scroll_max);
    layx_get_content_size(&ctx, container, &content_size);

    // 客户区宽度 = 200 - 10 - 10 - 5 - 5 = 170
    // scroll_max[0] = 300 - 170 = 130
    printf("  content_size: (%.2f, %.2f)\n", content_size[0], content_size[1]);
    printf("  scroll_max: (%.2f, %.2f)\n", scroll_max[0], scroll_max[1]);

    TEST_ASSERT(scroll_max[0] >= 120.0f && scroll_max[0] <= 140.0f, "scroll_max[0]应该约为130");

    layx_destroy_context(&ctx);
}

/**
 * 测试6: 动态内容变化时scroll_max更新
 */
void test_scroll_max_dynamic_content(void) {
    printf("\n=== Test 6: scroll_max Updates with Dynamic Content ===\n");

    layx_context ctx;
    layx_init_context(&ctx);
    layx_reserve_items_capacity(&ctx, 10);

    layx_id container = layx_item(&ctx);
    layx_set_size(&ctx, container, 200, 150);
    layx_set_overflow(&ctx, container, LAYX_OVERFLOW_AUTO);

    layx_id child = layx_item(&ctx);
    layx_set_size(&ctx, child, 100, 100);
    layx_push(&ctx, container, child);

    layx_run_context(&ctx);

    // 初始：内容不溢出
    layx_vec2 scroll_max1;
    layx_get_scroll_max(&ctx, container, &scroll_max1);
    TEST_ASSERT(float_equals(scroll_max1[0], 0.0f, 0.01f), "初始状态scroll_max[0]应为0");

    // 动态增大子项
    layx_set_size(&ctx, child, 300, 300);
    layx_run_context(&ctx);

    // 重新计算后scroll_max应该更新
    layx_vec2 scroll_max2;
    layx_get_scroll_max(&ctx, container, &scroll_max2);
    TEST_ASSERT(scroll_max2[0] > 0.0f, "内容增大后scroll_max[0]应大于0");
    TEST_ASSERT(scroll_max2[1] > 0.0f, "内容增大后scroll_max[1]应大于0");
    printf("  scroll_max after resize: (%.2f, %.2f)\n", scroll_max2[0], scroll_max2[1]);

    layx_destroy_context(&ctx);
}

/**
 * 测试7: overflow:hidden时scroll_max
 */
void test_scroll_max_overflow_hidden(void) {
    printf("\n=== Test 7: scroll_max with overflow:hidden ===\n");

    layx_context ctx;
    layx_init_context(&ctx);
    layx_reserve_items_capacity(&ctx, 10);

    layx_id container = layx_item(&ctx);
    layx_set_size(&ctx, container, 200, 150);
    layx_set_overflow(&ctx, container, LAYX_OVERFLOW_HIDDEN);

    layx_id child = layx_item(&ctx);
    layx_set_size(&ctx, child, 300, 300);
    layx_push(&ctx, container, child);

    layx_run_context(&ctx);

    layx_vec2 scroll_max;
    layx_get_scroll_max(&ctx, container, &scroll_max);

    // overflow:hidden时仍然会计算scroll_max，只是不显示滚动条
    // scroll_max应该仍然正确计算
    TEST_ASSERT(scroll_max[0] > 0.0f, "overflow:hidden时scroll_max[0]仍应正确计算");
    TEST_ASSERT(scroll_max[1] > 0.0f, "overflow:hidden时scroll_max[1]仍应正确计算");

    layx_destroy_context(&ctx);
}

/**
 * 测试8: overflow:visible时scroll_max
 */
void test_scroll_max_overflow_visible(void) {
    printf("\n=== Test 8: scroll_max with overflow:visible ===\n");

    layx_context ctx;
    layx_init_context(&ctx);
    layx_reserve_items_capacity(&ctx, 10);

    layx_id container = layx_item(&ctx);
    layx_set_size(&ctx, container, 200, 150);
    layx_set_overflow(&ctx, container, LAYX_OVERFLOW_VISIBLE);

    layx_id child = layx_item(&ctx);
    layx_set_size(&ctx, child, 300, 300);
    layx_push(&ctx, container, child);

    layx_run_context(&ctx);

    layx_vec2 scroll_max;
    layx_get_scroll_max(&ctx, container, &scroll_max);

    // overflow:visible时不应该有scroll_max（或者应该为0）
    TEST_ASSERT(float_equals(scroll_max[0], 0.0f, 0.01f), "overflow:visible时scroll_max[0]应为0");
    TEST_ASSERT(float_equals(scroll_max[1], 0.0f, 0.01f), "overflow:visible时scroll_max[1]应为0");

    layx_destroy_context(&ctx);
}

/**
 * 测试9: 多个子项时的scroll_max
 */
void test_scroll_max_multiple_children(void) {
    printf("\n=== Test 9: scroll_max with Multiple Children ===\n");

    layx_context ctx;
    layx_init_context(&ctx);
    layx_reserve_items_capacity(&ctx, 10);

    layx_id container = layx_item(&ctx);
    layx_set_size(&ctx, container, 200, 150);
    layx_set_overflow(&ctx, container, LAYX_OVERFLOW_AUTO);
    layx_set_display(&ctx, container, LAYX_DISPLAY_FLEX);
    layx_set_flex_direction(&ctx, container, LAYX_FLEX_DIRECTION_COLUMN);

    // 添加多个子项
    layx_id child1 = layx_item(&ctx);
    layx_set_size(&ctx, child1, 100, 50);
    layx_push(&ctx, container, child1);

    layx_id child2 = layx_item(&ctx);
    layx_set_size(&ctx, child2, 100, 50);
    layx_push(&ctx, container, child2);

    layx_id child3 = layx_item(&ctx);
    layx_set_size(&ctx, child3, 100, 100);
    layx_push(&ctx, container, child3);

    layx_id child4 = layx_item(&ctx);
    layx_set_size(&ctx, child4, 100, 50);
    layx_push(&ctx, container, child4);

    layx_run_context(&ctx);

    layx_vec2 scroll_max, content_size;
    layx_get_scroll_max(&ctx, container, &scroll_max);
    layx_get_content_size(&ctx, container, &content_size);

    // 总内容高度 = 50 + 50 + 100 + 50 = 250
    // scroll_max[1] = 250 - 150 = 100
    printf("  content_size: (%.2f, %.2f)\n", content_size[0], content_size[1]);
    printf("  scroll_max: (%.2f, %.2f)\n", scroll_max[0], scroll_max[1]);

    TEST_ASSERT(scroll_max[0] == 0.0f, "水平方向scroll_max[0]应为0");
    TEST_ASSERT(scroll_max[1] >= 90.0f && scroll_max[1] <= 110.0f, "垂直方向scroll_max[1]应该约为100");

    layx_destroy_context(&ctx);
}

/**
 * 测试10: 嵌套容器中的scroll_max独立性
 */
void test_scroll_max_nested_containers(void) {
    printf("\n=== Test 10: scroll_max Independence in Nested Containers ===\n");

    layx_context ctx;
    layx_init_context(&ctx);
    layx_reserve_items_capacity(&ctx, 10);

    // 外层容器
    layx_id outer = layx_item(&ctx);
    layx_set_size(&ctx, outer, 400, 300);
    layx_set_overflow(&ctx, outer, LAYX_OVERFLOW_AUTO);

    // 内层容器
    layx_id inner = layx_item(&ctx);
    layx_set_size(&ctx, inner, 500, 200);
    layx_set_overflow(&ctx, inner, LAYX_OVERFLOW_AUTO);
    layx_push(&ctx, outer, inner);

    // 内容项
    layx_id content = layx_item(&ctx);
    layx_set_size(&ctx, content, 800, 600);
    layx_push(&ctx, inner, content);

    layx_run_context(&ctx);

    layx_vec2 outer_scroll_max, inner_scroll_max;
    layx_get_scroll_max(&ctx, outer, &outer_scroll_max);
    layx_get_scroll_max(&ctx, inner, &inner_scroll_max);

    printf("  outer scroll_max: (%.2f, %.2f)\n", outer_scroll_max[0], outer_scroll_max[1]);
    printf("  inner scroll_max: (%.2f, %.2f)\n", inner_scroll_max[0], inner_scroll_max[1]);

    // 外层：内层宽度500 > 外层宽度400
    // outer_scroll_max[0] = 500 - 400 = 100
    TEST_ASSERT(outer_scroll_max[0] >= 90.0f && outer_scroll_max[0] <= 110.0f, "外层scroll_max[0]应该约为100");

    // 内层：内容宽度800 > 内层宽度500
    // inner_scroll_max[0] = 800 - 500 = 300
    TEST_ASSERT(inner_scroll_max[0] >= 290.0f && inner_scroll_max[0] <= 310.0f, "内层scroll_max[0]应该约为300");
    TEST_ASSERT(inner_scroll_max[1] >= 390.0f && inner_scroll_max[1] <= 410.0f, "内层scroll_max[1]应该约为400");

    layx_destroy_context(&ctx);
}

/**
 * 测试11: scroll_max与content_size的关系
 */
void test_scroll_max_content_size_relation(void) {
    printf("\n=== Test 11: scroll_max and content_size Relationship ===\n");

    layx_context ctx;
    layx_init_context(&ctx);
    layx_reserve_items_capacity(&ctx, 10);

    layx_id container = layx_item(&ctx);
    layx_set_size(&ctx, container, 200, 150);
    layx_set_padding(&ctx, container, 10);
    layx_set_overflow(&ctx, container, LAYX_OVERFLOW_AUTO);

    layx_id child = layx_item(&ctx);
    layx_set_size(&ctx, child, 300, 200);
    layx_push(&ctx, container, child);

    layx_run_context(&ctx);

    layx_vec2 scroll_max, content_size;
    layx_get_scroll_max(&ctx, container, &scroll_max);
    layx_get_content_size(&ctx, container, &content_size);

    // scroll_max = max(0, content_size - client_size)
    // client_width = 200 - 20 = 180
    // client_height = 150 - 20 = 130
    float expected_scroll_max_x = content_size[0] - 180.0f;
    float expected_scroll_max_y = content_size[1] - 130.0f;

    printf("  content_size: (%.2f, %.2f)\n", content_size[0], content_size[1]);
    printf("  scroll_max: (%.2f, %.2f)\n", scroll_max[0], scroll_max[1]);
    printf("  expected scroll_max: (%.2f, %.2f)\n", expected_scroll_max_x, expected_scroll_max_y);

    TEST_ASSERT(float_equals(scroll_max[0], expected_scroll_max_x, 1.0f), "scroll_max[0]应该等于content_size[0] - client_width");
    TEST_ASSERT(float_equals(scroll_max[1], expected_scroll_max_y, 1.0f), "scroll_max[1]应该等于content_size[1] - client_height");

    layx_destroy_context(&ctx);
}

/**
 * 测试12: scroll_max边界条件 - 空容器
 */
void test_scroll_max_empty_container(void) {
    printf("\n=== Test 12: scroll_max with Empty Container ===\n");

    layx_context ctx;
    layx_init_context(&ctx);

    layx_id container = layx_item(&ctx);
    layx_set_size(&ctx, container, 200, 150);
    layx_set_overflow(&ctx, container, LAYX_OVERFLOW_AUTO);

    layx_run_context(&ctx);

    layx_vec2 scroll_max, content_size;
    layx_get_scroll_max(&ctx, container, &scroll_max);
    layx_get_content_size(&ctx, container, &content_size);

    // 空容器：内容尺寸等于客户区尺寸
    TEST_ASSERT(float_equals(scroll_max[0], 0.0f, 0.01f), "空容器scroll_max[0]应为0");
    TEST_ASSERT(float_equals(scroll_max[1], 0.0f, 0.01f), "空容器scroll_max[1]应为0");

    layx_destroy_context(&ctx);
}

/**
 * 测试13: scroll_max在正好匹配容器时
 */
void test_scroll_max_exact_match(void) {
    printf("\n=== Test 13: scroll_max with Exact Match ===\n");

    layx_context ctx;
    layx_init_context(&ctx);
    layx_reserve_items_capacity(&ctx, 10);

    layx_id container = layx_item(&ctx);
    layx_set_size(&ctx, container, 200, 150);
    layx_set_overflow(&ctx, container, LAYX_OVERFLOW_AUTO);

    // 创建正好匹配容器的子项
    layx_id child = layx_item(&ctx);
    layx_set_size(&ctx, child, 200, 150);
    layx_push(&ctx, container, child);

    layx_run_context(&ctx);

    layx_vec2 scroll_max;
    layx_get_scroll_max(&ctx, container, &scroll_max);

    // 正好匹配时scroll_max应该为0
    TEST_ASSERT(float_equals(scroll_max[0], 0.0f, 0.01f), "内容宽度正好匹配时scroll_max[0]应为0");
    TEST_ASSERT(float_equals(scroll_max[1], 0.0f, 0.01f), "内容高度正好匹配时scroll_max[1]应为0");

    layx_destroy_context(&ctx);
}

/**
 * 测试14: scroll_max在只有水平溢出时
 */
void test_scroll_max_horizontal_only(void) {
    printf("\n=== Test 14: scroll_max with Horizontal Overflow Only ===\n");

    layx_context ctx;
    layx_init_context(&ctx);
    layx_reserve_items_capacity(&ctx, 10);

    layx_id container = layx_item(&ctx);
    layx_set_size(&ctx, container, 200, 150);
    layx_set_overflow(&ctx, container, LAYX_OVERFLOW_AUTO);

    layx_id child = layx_item(&ctx);
    layx_set_size(&ctx, child, 300, 100);  // 宽度超出，高度不超出
    layx_push(&ctx, container, child);

    layx_run_context(&ctx);

    layx_vec2 scroll_max;
    layx_get_scroll_max(&ctx, container, &scroll_max);

    TEST_ASSERT(scroll_max[0] > 0.0f, "水平溢出时scroll_max[0]应大于0");
    TEST_ASSERT(float_equals(scroll_max[1], 0.0f, 0.01f), "垂直不溢出时scroll_max[1]应为0");

    layx_destroy_context(&ctx);
}

/**
 * 测试15: scroll_max在只有垂直溢出时
 */
void test_scroll_max_vertical_only(void) {
    printf("\n=== Test 15: scroll_max with Vertical Overflow Only ===\n");

    layx_context ctx;
    layx_init_context(&ctx);
    layx_reserve_items_capacity(&ctx, 10);

    layx_id container = layx_item(&ctx);
    layx_set_size(&ctx, container, 200, 150);
    layx_set_overflow(&ctx, container, LAYX_OVERFLOW_AUTO);
    layx_set_display(&ctx, container, LAYX_DISPLAY_FLEX);
    layx_set_flex_direction(&ctx, container, LAYX_FLEX_DIRECTION_COLUMN);

    layx_id child = layx_item(&ctx);
    layx_set_size(&ctx, child, 100, 200);  // 宽度不超出，高度超出
    layx_push(&ctx, container, child);

    layx_run_context(&ctx);

    layx_vec2 scroll_max;
    layx_get_scroll_max(&ctx, container, &scroll_max);

    TEST_ASSERT(float_equals(scroll_max[0], 0.0f, 0.01f), "水平不溢出时scroll_max[0]应为0");
    TEST_ASSERT(scroll_max[1] > 0.0f, "垂直溢出时scroll_max[1]应大于0");

    layx_destroy_context(&ctx);
}

int main(void) {
    printf("===========================================\n");
    printf("   LAYX scroll_max Functionality Tests\n");
    printf("===========================================\n");

    // 运行所有测试
    test_scroll_max_no_overflow();
    test_scroll_max_with_overflow();
    test_scroll_max_with_padding();
    test_scroll_max_with_border();
    test_scroll_max_with_padding_and_border();
    test_scroll_max_dynamic_content();
    test_scroll_max_overflow_hidden();
    test_scroll_max_overflow_visible();
    test_scroll_max_multiple_children();
    test_scroll_max_nested_containers();
    test_scroll_max_content_size_relation();
    test_scroll_max_empty_container();
    test_scroll_max_exact_match();
    test_scroll_max_horizontal_only();
    test_scroll_max_vertical_only();

    // 输出测试总结
    printf("\n===========================================\n");
    printf("           Test Summary\n");
    printf("===========================================\n");
    printf("Tests Passed: %d\n", tests_passed);
    printf("Tests Failed: %d\n", tests_failed);
    printf("Total Tests:  %d\n", tests_passed + tests_failed);
    printf("===========================================\n");

    if (tests_failed == 0) {
        printf("✓ All tests passed!\n");
    } else {
        printf("✗ Some tests failed!\n");
    }

    return 0;
}

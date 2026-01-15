#define LAYX_IMPLEMENTATION
#include "layx.h"
#include <stdio.h>
#include <assert.h>

void test_create_and_destroy() {
    layx_context ctx;
    layx_init_context(&ctx);
    
    // 创建容器和子元素
    layx_id container = layx_item(&ctx);
    layx_id child1 = layx_item(&ctx);
    layx_id child2 = layx_item(&ctx);
    layx_id child3 = layx_item(&ctx);
    
    layx_append(&ctx, container, child1);
    layx_append(&ctx, container, child2);
    layx_append(&ctx, container, child3);
    
    printf("初始状态:\n");
    printf("  items_count = %d\n", layx_items_count(&ctx));
    printf("  container 的第一个子元素 = %d\n", layx_first_child(&ctx, container));
    
    // 测试 layx_remove - 仅从父元素移除
    printf("\n测试 layx_remove:\n");
    layx_remove(&ctx, child2);
    printf("  移除 child2 后:\n");
    printf("  items_count = %d\n", layx_items_count(&ctx));
    printf("  container 的第一个子元素 = %d\n", layx_first_child(&ctx, container));
    
    // 检查 child2 是否还存在（未销毁）
    layx_item_t *pchild2 = layx_get_item(&ctx, child2);
    printf("  child2 仍然存在: parent = %d, first_child = %d\n", 
           pchild2->parent, pchild2->first_child);
    
    // 测试 layx_destroy_item - 销毁元素
    printf("\n测试 layx_destroy_item:\n");
    layx_destroy_item(&ctx, child3);
    printf("  销毁 child3 后:\n");
    printf("  items_count = %d\n", layx_items_count(&ctx));
    printf("  container 的第一个子元素 = %d\n", layx_first_child(&ctx, container));
    
    // 测试销毁整个容器（包括所有子元素）
    printf("\n测试销毁整个容器:\n");
    layx_destroy_item(&ctx, container);
    printf("  销毁 container 后:\n");
    printf("  items_count = %d\n", layx_items_count(&ctx));
    
    layx_destroy_context(&ctx);
    printf("\n测试完成!\n");
}

void test_free_list_reuse() {
    printf("\n测试空闲链表重用:\n");
    layx_context ctx;
    layx_init_context(&ctx);
    
    // 创建并销毁多个元素
    layx_id id1 = layx_item(&ctx);
    layx_id id2 = layx_item(&ctx);
    layx_id id3 = layx_item(&ctx);
    
    printf("  创建 3 个元素: count = %d\n", layx_items_count(&ctx));
    printf("  id1=%d, id2=%d, id3=%d\n", id1, id2, id3);
    
    // 销毁 id2
    layx_destroy_item(&ctx, id2);
    printf("  销毁 id2 后: count = %d\n", layx_items_count(&ctx));
    
    // 创建新元素，应该重用 id2
    layx_id id4 = layx_item(&ctx);
    printf("  创建 id4: count = %d, id4=%d\n", layx_items_count(&ctx), id4);
    assert(id4 == id2); // 应该重用 id2
    
    // 创建 id5，应该是新分配的
    layx_id id5 = layx_item(&ctx);
    printf("  创建 id5: count = %d, id5=%d\n", layx_items_count(&ctx), id5);
    assert(id5 == 3); // 应该是新的 ID
    
    layx_destroy_context(&ctx);
    printf("  测试通过!\n");
}

int main() {
    test_create_and_destroy();
    test_free_list_reuse();
    return 0;
}

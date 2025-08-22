#ifndef __HI_LIST_H__
#define __HI_LIST_H__

#ifdef __cplusplus
#if __cplusplus
extern "C"
{
#endif
#endif /* __cplusplus */

    /***************          list Function  **************************/
    
    /**
     * @brief 双向链表头结构体
     * @details 包含指向下一个和上一个节点的指针，实现双向链表
     */
    typedef struct __dof_list_head
    {
        struct __dof_list_head *next, *prev;  /**< 指向下一个和上一个节点的指针 */
    } dof_list_head;

    /* 类型定义 */
    typedef unsigned int uword32;  /**< 无符号32位整数类型 */
    typedef int word32;            /**< 有符号32位整数类型 */
#define dof_uint32 unsigned int   /**< 无符号32位整数类型宏定义 */

    /**
     * @brief 初始化链表头
     * @param ptr 链表头指针
     * @details 将链表头的next和prev都指向自己，形成空链表
     */
#define dof_list_init_head(ptr) \
    do                          \
    {                           \
        (ptr)->next = (ptr);    \
        (ptr)->prev = (ptr);    \
    } while (0)

    /**
     * @brief 安全遍历链表（正向）
     * @param pos 当前节点指针
     * @param n 下一个节点指针（用于安全删除）
     * @param head 链表头指针
     * @details 在遍历过程中可以安全删除当前节点，不会影响遍历
     */
#define dof_list_for_each_safe(pos, n, head)               \
    for (pos = (head)->next, n = pos->next; pos != (head); \
         pos = n, n = pos->next)

    /**
     * @brief 从指定位置开始安全遍历链表（正向）
     * @param pos 当前节点指针
     * @param n 下一个节点指针
     * @param head 链表头指针
     * @param cur 开始遍历的节点指针
     * @details 从cur节点开始遍历，而不是从头开始
     */
#define dof_list_for_each_safe_init(pos, n, head, cur)    \
    for (pos = (cur)->next, n = pos->next; pos != (head); \
         pos = n, n = pos->next)

    /**
     * @brief 安全遍历链表（反向）
     * @param pos 当前节点指针
     * @param n 上一个节点指针
     * @param head 链表头指针
     * @details 从后往前遍历链表，可以安全删除节点
     */
#define dof_list_for_each_safe_reverse(pos, n, head)       \
    for (pos = (head)->prev, n = pos->prev; pos != (head); \
         pos = n, n = pos->prev)

    /**
     * @brief 计算结构体成员在结构体中的偏移量
     * @param TYPE 结构体类型
     * @param MEMBER 结构体成员名
     * @return 成员在结构体中的偏移字节数
     * @details 通过将0强制转换为结构体指针来计算偏移量
     */
#define dof_offsetof(TYPE, MEMBER) \
    ((dof_uint32) & ((TYPE *)0)->MEMBER)

    /**
     * @brief 通过链表节点指针获取包含该节点的结构体指针
     * @param ptr 链表节点指针
     * @param type 结构体类型
     * @param member 链表节点在结构体中的成员名
     * @return 包含该节点的结构体指针
     * @details 这是container_of宏的实现，用于从链表节点反向获取宿主结构体
     */
#define dof_list_entry(ptr, type, member) /*lint -save -e413*/ ({          \
        const typeof( ((type *)0)->member ) *__mptr = (ptr);    \
        (type *)( (char *)__mptr - dof_offsetof(type,member) ); }) /*lint -restore*/

    /**
     * @brief 获取链表头后的第一个节点
     * @param head 链表头指针
     * @return 第一个节点指针，如果链表为空则返回NULL
     */
#define dof_list_getnext(head) ((head != (head)->next) ? ((head)->next) : (NULL))

    /**
     * @brief 获取链表头前的最后一个节点
     * @param head 链表头指针
     * @return 最后一个节点指针，如果链表为空则返回NULL
     */
#define dof_list_getprev(head) ((head != (head)->prev) ? ((head)->prev) : (NULL))

    /**
     * @brief 遍历链表中的所有节点（正向）
     * @param pos 当前节点结构体指针
     * @param head 链表头指针
     * @param member 链表节点在结构体中的成员名
     * @details 遍历过程中不能删除节点，否则会破坏遍历
     */
#define dof_list_for_each_entry(pos, head, member)                 \
    for (pos = dof_list_entry((head)->next, typeof(*pos), member); \
         &pos->member != (head);                                   \
         pos = dof_list_entry(pos->member.next, typeof(*pos), member))

    /**
     * @brief 安全遍历链表中的所有节点（正向）
     * @param pos 当前节点结构体指针
     * @param n 下一个节点结构体指针
     * @param head 链表头指针
     * @param member 链表节点在结构体中的成员名
     * @details 可以安全删除当前节点，不会影响遍历过程
     */
#define dof_list_for_each_entry_safe(pos, n, head, member) /*lint -e413*/ \
    for (pos = dof_list_entry((head)->next, typeof(*pos), member),        \
        n = dof_list_entry(pos->member.next, typeof(*pos), member);       \
         &pos->member != (head);                                          \
         pos = n, n = dof_list_entry(n->member.next, typeof(*n), member)) /*lint +e413*/

    /**
     * @brief 遍历链表中的所有节点（反向）
     * @param pos 当前节点结构体指针
     * @param head 链表头指针
     * @param member 链表节点在结构体中的成员名
     * @details 从后往前遍历，不能删除节点
     */
#define dof_list_for_each_entry_reverse(pos, head, member)         \
    for (pos = dof_list_entry((head)->prev, typeof(*pos), member); \
         &pos->member != (head);                                   \
         pos = dof_list_entry(pos->member.prev, typeof(*pos), member)) /*lint +e413*/

    /**
     * @brief 安全遍历链表中的所有节点（反向）
     * @param pos 当前节点结构体指针
     * @param n 上一个节点结构体指针
     * @param head 链表头指针
     * @param member 链表节点在结构体中的成员名
     * @details 从后往前遍历，可以安全删除节点
     */
#define dof_list_for_each_entry_reverse_safe(pos, n, head, member) /*lint -e413*/ \
    for (pos = dof_list_entry((head)->prev, typeof(*pos), member),                \
        n = dof_list_entry(pos->member.prev, typeof(*pos), member);               \
         &pos->member != (head);                                                  \
         pos = n, n = dof_list_entry(n->member.prev, typeof(*n), member)) /*lint +e413*/

    /**
     * @brief 从指定位置继续遍历链表（正向）
     * @param pos 当前节点结构体指针
     * @param head 链表头指针
     * @param member 链表节点在结构体中的成员名
     * @details 从pos的下一个节点开始遍历，用于跳过某些节点
     */
#define dof_list_for_each_entry_continue(pos, head, member)            \
    for (pos = dof_list_entry(pos->member.next, typeof(*pos), member); \
         &pos->member != (head);                                       \
         pos = dof_list_entry(pos->member.next, typeof(*pos), member))

    /**
     * @brief 从指定位置继续遍历链表（反向）
     * @param pos 当前节点结构体指针
     * @param head 链表头指针
     * @param member 链表节点在结构体中的成员名
     * @details 从pos的上一个节点开始反向遍历
     */
#define dof_list_for_each_entry_continue_reverse(pos, head, member)    \
    for (pos = dof_list_entry(pos->member.prev, typeof(*pos), member); \
         &pos->member != (head);                                       \
         pos = dof_list_entry(pos->member.prev, typeof(*pos), member))

    /**
     * @brief 判断节点是否为链表的最后一个节点
     * @param list 要检查的节点指针
     * @param head 链表头指针
     * @return 1表示是最后一个节点，0表示不是
     */
    static inline int dof_list_is_last(dof_list_head *list,
                                       dof_list_head *head)
    {
        return list->next == head;
    }

    /**
     * @brief 在指定位置插入新节点（内部函数）
     * @param newlist 要插入的新节点指针
     * @param prev 前一个节点指针
     * @param next 后一个节点指针
     * @details 在prev和next之间插入newlist节点
     */
    static inline void __dof_list_add(dof_list_head *newlist,
                                      dof_list_head *prev,
                                      dof_list_head *next)
    {
        next->prev = newlist;
        newlist->next = next;
        newlist->prev = prev;
        prev->next = newlist;
    }

    /**
     * @brief 在链表头部插入新节点
     * @param newlist 要插入的新节点指针
     * @param head 链表头指针
     * @details 新节点会插入到链表头之后，成为第一个节点
     */
    static inline void dof_list_add(dof_list_head *newlist, dof_list_head *head)
    {
        __dof_list_add(newlist, head, head->next);
    }

    /**
     * @brief 在链表尾部插入新节点
     * @param newlist 要插入的新节点指针
     * @param head 链表头指针
     * @details 新节点会插入到链表头之前，成为最后一个节点
     */
    static inline void dof_list_add_tail(dof_list_head *newlist, dof_list_head *head)
    {
        __dof_list_add(newlist, head->prev, head);
    }

    /**
     * @brief 删除指定位置之间的节点（内部函数）
     * @param prev 前一个节点指针
     * @param next 后一个节点指针
     * @details 将prev和next节点连接起来，跳过中间的节点
     */
    static inline void __dof_list_del(dof_list_head *prev, dof_list_head *next)
    {
        next->prev = prev;
        prev->next = next;
    }

    /**
     * @brief 从链表中删除指定节点
     * @param entry 要删除的节点指针
     * @details 删除节点后，将节点的next和prev设为NULL，防止悬空指针
     */
    static inline void dof_list_del(dof_list_head *entry)
    {
        __dof_list_del(entry->prev, entry->next);
        entry->next = NULL;
        entry->prev = NULL;
    }

    /**
     * @brief 判断链表是否为空
     * @param head 链表头指针
     * @return 1表示链表为空，0表示链表不为空
     * @details 通过检查head的next是否指向自己来判断
     */
    static inline int dof_list_empty(dof_list_head *head)
    {
        return head->next == head;
    }

    /**
     * @brief 判断节点是否已挂载到链表中
     * @param entry 要检查的节点指针
     * @return 1表示已挂载，0表示未挂载
     * @details 检查节点的prev和next是否都不为NULL
     */
    static inline int dof_list_hang(dof_list_head *entry)
    {
        return ((NULL != entry->prev) && (NULL != entry->next));
    }

    /**
     * @brief 在指定节点之前插入新节点
     * @param pst_cur 当前节点指针
     * @param newlist 要插入的新节点指针
     * @details 新节点会插入到pst_cur之前
     */
    static inline void dof_list_insert(dof_list_head *pst_cur, dof_list_head *newlist)
    {
        pst_cur->prev->next = newlist;
        newlist->next = pst_cur;
        newlist->prev = pst_cur->prev;
        pst_cur->prev = newlist;
    }

    /**
     * @brief 在指定节点之后插入新节点
     * @param pst_cur 当前节点指针
     * @param newlist 要插入的新节点指针
     * @details 新节点会插入到pst_cur之后
     */
    static inline void dof_list_insert_after(dof_list_head *pst_cur, dof_list_head *newlist)
    {
        pst_cur->next->prev = newlist;
        newlist->next = pst_cur->next;
        newlist->prev = pst_cur;
        pst_cur->next = newlist;
    }

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

#endif /* __HI_LIST_H__ */

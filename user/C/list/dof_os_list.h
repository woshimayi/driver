#ifndef __HI_LIST_H__
#define __HI_LIST_H__

#ifdef __cplusplus
#if __cplusplus
extern "C"
{
#endif
#endif /* __cplusplus */

    /***************          list Function  **************************/
    typedef struct __dof_list_head
    {
        struct __dof_list_head *next, *prev;
    } dof_list_head;

    typedef unsigned int uword32;
    typedef int word32;
#define dof_uint32 unsigned int

#define dof_list_init_head(ptr) \
    do                         \
    {                          \
        (ptr)->next = (ptr);   \
        (ptr)->prev = (ptr);   \
    } while (0)

#define dof_list_for_each_safe(pos, n, head)                \
    for (pos = (head)->next, n = pos->next; pos != (head); \
         pos = n, n = pos->next)

#define dof_list_for_each_safe_init(pos, n, head, cur)     \
    for (pos = (cur)->next, n = pos->next; pos != (head); \
         pos = n, n = pos->next)

#define dof_list_for_each_safe_reverse(pos, n, head)        \
    for (pos = (head)->prev, n = pos->prev; pos != (head); \
         pos = n, n = pos->prev)

#define dof_offsetof(TYPE, MEMBER) \
    ((dof_uint32) & ((TYPE *)0)->MEMBER)

#define dof_list_entry(ptr, type, member) /*lint -save -e413*/ ({          \
        const typeof( ((type *)0)->member ) *__mptr = (ptr);    \
        (type *)( (char *)__mptr - dof_offsetof(type,member) ); }) /*lint -restore*/

#define dof_list_getnext(head) ((head != (head)->next) ? ((head)->next) : (NULL))

#define dof_list_getprev(head) ((head != (head)->prev) ? ((head)->prev) : (NULL))

#define dof_list_for_each_entry(pos, head, member)                 \
    for (pos = dof_list_entry((head)->next, typeof(*pos), member); \
         &pos->member != (head);                                  \
         pos = dof_list_entry(pos->member.next, typeof(*pos), member))

#define dof_list_for_each_entry_safe(pos, n, head, member) /*lint -e413*/ \
    for (pos = dof_list_entry((head)->next, typeof(*pos), member),        \
        n = dof_list_entry(pos->member.next, typeof(*pos), member);       \
         &pos->member != (head);                                         \
         pos = n, n = dof_list_entry(n->member.next, typeof(*n), member)) /*lint +e413*/

#define dof_list_for_each_entry_reverse(pos, head, member)         \
    for (pos = dof_list_entry((head)->prev, typeof(*pos), member); \
         &pos->member != (head);                                  \
         pos = dof_list_entry(pos->member.prev, typeof(*pos), member)) /*lint +e413*/

#define dof_list_for_each_entry_reverse_safe(pos, n, head, member) /*lint -e413*/ \
    for (pos = dof_list_entry((head)->prev, typeof(*pos), member),                \
        n = dof_list_entry(pos->member.prev, typeof(*pos), member);               \
         &pos->member != (head);                                                 \
         pos = n, n = dof_list_entry(n->member.prev, typeof(*n), member)) /*lint +e413*/

#define dof_list_for_each_entry_continue(pos, head, member)            \
    for (pos = dof_list_entry(pos->member.next, typeof(*pos), member); \
         &pos->member != (head);                                      \
         pos = dof_list_entry(pos->member.next, typeof(*pos), member))

#define dof_list_for_each_entry_continue_reverse(pos, head, member)    \
    for (pos = dof_list_entry(pos->member.prev, typeof(*pos), member); \
         &pos->member != (head);                                      \
         pos = dof_list_entry(pos->member.prev, typeof(*pos), member))

    static inline int dof_list_is_last(dof_list_head *list,
                                      dof_list_head *head)
    {
        return list->next == head;
    }

    static inline void __dof_list_add(dof_list_head *newlist,
                                     dof_list_head *prev,
                                     dof_list_head *next)
    {
        next->prev = newlist;
        newlist->next = next;
        newlist->prev = prev;
        prev->next = newlist;
    }

    static inline void dof_list_add(dof_list_head *newlist, dof_list_head *head)
    {
        __dof_list_add(newlist, head, head->next);
    }

    static inline void dof_list_add_tail(dof_list_head *newlist, dof_list_head *head)
    {
        __dof_list_add(newlist, head->prev, head);
    }

    static inline void __dof_list_del(dof_list_head *prev, dof_list_head *next)
    {
        next->prev = prev;
        prev->next = next;
    }

    static inline void dof_list_del(dof_list_head *entry)
    {
        __dof_list_del(entry->prev, entry->next);
        entry->next = NULL;
        entry->prev = NULL;
    }

    static inline int dof_list_empty(dof_list_head *head)
    {
        return head->next == head;
    }

    static inline int dof_list_hang(dof_list_head *entry)
    {
        return ((NULL != entry->prev) && (NULL != entry->next));
    }

    static inline void dof_list_insert(dof_list_head *pst_cur, dof_list_head *newlist)
    {
        pst_cur->prev->next = newlist;
        newlist->next = pst_cur;
        newlist->prev = pst_cur->prev;
        pst_cur->prev = newlist;
    }

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

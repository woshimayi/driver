#ifndef __HI_LIST_H__
#define __HI_LIST_H__

#ifdef __cplusplus
#if __cplusplus
extern "C"
{
#endif
#endif /* __cplusplus */

    /***************          list Function  **************************/
    typedef struct __hi_list_head
    {
        struct __hi_list_head *next, *prev;
    } hi_list_head;

    typedef unsigned int uword32;
    typedef int word32;
#define hi_uint32 unsigned int

#define hi_list_init_head(ptr) \
    do                         \
    {                          \
        (ptr)->next = (ptr);   \
        (ptr)->prev = (ptr);   \
    } while (0)

#define hi_list_for_each_safe(pos, n, head)                \
    for (pos = (head)->next, n = pos->next; pos != (head); \
         pos = n, n = pos->next)

#define hi_list_for_each_safe_init(pos, n, head, cur)     \
    for (pos = (cur)->next, n = pos->next; pos != (head); \
         pos = n, n = pos->next)

#define hi_list_for_each_safe_reverse(pos, n, head)        \
    for (pos = (head)->prev, n = pos->prev; pos != (head); \
         pos = n, n = pos->prev)

#define hi_offsetof(TYPE, MEMBER) \
    ((hi_uint32) & ((TYPE *)0)->MEMBER)

#define hi_list_entry(ptr, type, member) /*lint -save -e413*/ ({          \
        const typeof( ((type *)0)->member ) *__mptr = (ptr);    \
        (type *)( (char *)__mptr - hi_offsetof(type,member) ); }) /*lint -restore*/

#define hi_list_getnext(head) ((head != (head)->next) ? ((head)->next) : (NULL))

#define hi_list_getprev(head) ((head != (head)->prev) ? ((head)->prev) : (NULL))

#define hi_list_for_each_entry(pos, head, member)                 \
    for (pos = hi_list_entry((head)->next, typeof(*pos), member); \
         &pos->member != (head);                                  \
         pos = hi_list_entry(pos->member.next, typeof(*pos), member))

#define hi_list_for_each_entry_safe(pos, n, head, member) /*lint -e413*/ \
    for (pos = hi_list_entry((head)->next, typeof(*pos), member),        \
        n = hi_list_entry(pos->member.next, typeof(*pos), member);       \
         &pos->member != (head);                                         \
         pos = n, n = hi_list_entry(n->member.next, typeof(*n), member)) /*lint +e413*/

#define hi_list_for_each_entry_reverse(pos, head, member)         \
    for (pos = hi_list_entry((head)->prev, typeof(*pos), member); \
         &pos->member != (head);                                  \
         pos = hi_list_entry(pos->member.prev, typeof(*pos), member)) /*lint +e413*/

#define hi_list_for_each_entry_reverse_safe(pos, n, head, member) /*lint -e413*/ \
    for (pos = hi_list_entry((head)->prev, typeof(*pos), member),                \
        n = hi_list_entry(pos->member.prev, typeof(*pos), member);               \
         &pos->member != (head);                                                 \
         pos = n, n = hi_list_entry(n->member.prev, typeof(*n), member)) /*lint +e413*/

#define hi_list_for_each_entry_continue(pos, head, member)            \
    for (pos = hi_list_entry(pos->member.next, typeof(*pos), member); \
         &pos->member != (head);                                      \
         pos = hi_list_entry(pos->member.next, typeof(*pos), member))

#define hi_list_for_each_entry_continue_reverse(pos, head, member)    \
    for (pos = hi_list_entry(pos->member.prev, typeof(*pos), member); \
         &pos->member != (head);                                      \
         pos = hi_list_entry(pos->member.prev, typeof(*pos), member))

    static inline int hi_list_is_last(hi_list_head *list,
                                      hi_list_head *head)
    {
        return list->next == head;
    }

    static inline void __hi_list_add(hi_list_head *newlist,
                                     hi_list_head *prev,
                                     hi_list_head *next)
    {
        next->prev = newlist;
        newlist->next = next;
        newlist->prev = prev;
        prev->next = newlist;
    }

    static inline void hi_list_add(hi_list_head *newlist, hi_list_head *head)
    {
        __hi_list_add(newlist, head, head->next);
    }

    static inline void hi_list_add_tail(hi_list_head *newlist, hi_list_head *head)
    {
        __hi_list_add(newlist, head->prev, head);
    }

    static inline void __hi_list_del(hi_list_head *prev, hi_list_head *next)
    {
        next->prev = prev;
        prev->next = next;
    }

    static inline void hi_list_del(hi_list_head *entry)
    {
        __hi_list_del(entry->prev, entry->next);
        entry->next = NULL;
        entry->prev = NULL;
    }

    static inline int hi_list_empty(hi_list_head *head)
    {
        return head->next == head;
    }

    static inline int hi_list_hang(hi_list_head *entry)
    {
        return ((NULL != entry->prev) && (NULL != entry->next));
    }

    static inline void hi_list_insert(hi_list_head *pst_cur, hi_list_head *newlist)
    {
        pst_cur->prev->next = newlist;
        newlist->next = pst_cur;
        newlist->prev = pst_cur->prev;
        pst_cur->prev = newlist;
    }

    static inline void hi_list_insert_after(hi_list_head *pst_cur, hi_list_head *newlist)
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "harness.h"
#include "queue.h"

/* Notice: sometimes, Cppcheck would find the potential NULL pointer bugs,
 * but some of them cannot occur. You can suppress them by adding the
 * following line.
 *   cppcheck-suppress nullPointer
 */

/*
 * Create empty queue.
 * Return NULL if could not allocate space.
 */
struct list_head *q_new()
{
    struct list_head *new = malloc(sizeof(struct list_head));

    // Directly call the INIT function defined in "list.h"
    if (new)
        INIT_LIST_HEAD(new);
    return new;
}

/* Free all storage used by queue */
void q_free(struct list_head *l)
{
    if (!l)
        return;
    while (!list_empty(l)) {
        l->next = l->next->next;
        q_release_element(list_entry(l->next->prev, element_t, list));
        l->next->prev = l;
    }
    free(l);
}

/*
 * Attempt to insert element at head of queue.
 * Return true if successful.
 * Return false if q is NULL or could not allocate space.
 * Argument s points to the string to be stored.
 * The function must explicitly allocate space and copy the string into it.
 */
bool q_insert_head(struct list_head *head, char *s)
{
    if (!head)
        return false;
    element_t *new = (element_t *) malloc(sizeof(element_t));
    if (!new)
        return false;
    new->value = (char *) malloc(strlen(s) + 1);
    if (!(new->value)) {
        free(new);
        return false;
    }
    strncpy(new->value, s, strlen(s) + 1);
    list_add(&new->list, head);
    return true;
}

/*
 * Attempt to insert element at tail of queue.
 * Return true if successful.
 * Return false if q is NULL or could not allocate space.
 * Argument s points to the string to be stored.
 * The function must explicitly allocate space and copy the string into it.
 */
bool q_insert_tail(struct list_head *head, char *s)
{
    if (!head)
        return false;
    element_t *new = (element_t *) malloc(sizeof(element_t));
    if (!new)
        return false;
    new->value = (char *) malloc(strlen(s) + 1);
    if (!(new->value)) {
        free(new);
        return false;
    }
    strncpy(new->value, s, strlen(s) + 1);
    list_add_tail(&new->list, head);
    return true;
}

/*
 * Attempt to remove element from head of queue.
 * Return target element.
 * Return NULL if queue is NULL or empty.
 * If sp is non-NULL and an element is removed, copy the removed string to *sp
 * (up to a maximum of bufsize-1 characters, plus a null terminator.)
 *
 * NOTE: "remove" is different from "delete"
 * The space used by the list element and the string should not be freed.
 * The only thing "remove" need to do is unlink it.
 *
 * REF:
 * https://english.stackexchange.com/questions/52508/difference-between-delete-and-remove
 */
element_t *q_remove_head(struct list_head *head, char *sp, size_t bufsize)
{
    if (!head || list_empty(head))
        return NULL;
    element_t *tgt = list_entry(head->next, element_t, list);
    list_del(&(tgt->list));
    memset(sp, '\0', bufsize);
    strncpy(sp, tgt->value, bufsize - 1);
    return tgt;
}

/*
 * Attempt to remove element from tail of queue.
 * Other attribute is as same as q_remove_head.
 */
element_t *q_remove_tail(struct list_head *head, char *sp, size_t bufsize)
{
    if (!head || list_empty(head))
        return NULL;
    element_t *tgt = list_entry(head->prev, element_t, list);
    list_del(&(tgt->list));
    memset(sp, '\0', bufsize);
    strncpy(sp, tgt->value, bufsize - 1);
    return tgt;
}

/*
 * WARN: This is for external usage, don't modify it
 * Attempt to release element.
 */
void q_release_element(element_t *e)
{
    free(e->value);
    free(e);
}

/*
 * Return number of elements in queue.
 * Return 0 if q is NULL or empty
 */
int q_size(struct list_head *head)
{
    if (!head || list_empty(head))
        return 0;
    int cnt = 0;
    struct list_head *pos;
    list_for_each (pos, head)
        ++cnt;
    return cnt;
}

/*
 * Delete the middle node in list.
 * The middle node of a linked list of size n is the
 * ⌊n / 2⌋th node from the start using 0-based indexing.
 * If there're six element, the third member should be return.
 * Return true if successful.
 * Return false if list is NULL or empty.
 */
bool q_delete_mid(struct list_head *head)
{
    // https://leetcode.com/problems/delete-the-middle-node-of-a-linked-list/
    if (!head || list_empty(head))
        return false;
    for (int i = q_size(head) >> 1; !(i < 0); --i)
        head = head->next;
    element_t *el = list_entry(head, element_t, list);
    list_del(head);
    q_release_element(el);
    return true;
}

/*
 * Delete all nodes that have duplicate string,
 * leaving only distinct strings from the original list.
 * Return true if successful.
 * Return false if list is NULL.
 *
 * Note: this function always be called after sorting, in other words,
 * list is guaranteed to be sorted in ascending order.
 */
bool q_delete_dup(struct list_head *head)
{
    // https://leetcode.com/problems/remove-duplicates-from-sorted-list-ii/
    if (!head)
        return false;
    struct list_head *node, *safe;
    int f = 0;
    list_for_each_safe (node, safe, head) {
        printf("%s\n", list_entry(node, element_t, list)->value);
        if (safe != head && !strcmp(list_entry(node, element_t, list)->value,
                                    list_entry(safe, element_t, list)->value)) {
            f = 1;
            list_del(node);
            q_release_element(list_entry(node, element_t, list));
        } else if (f) {
            f = 0;
            list_del(node);
            q_release_element(list_entry(node, element_t, list));
        }
    }
    return true;
}

/*
 * Attempt to swap every two adjacent nodes.
 */
void q_swap(struct list_head *head)
{
    // https://leetcode.com/problems/swap-nodes-in-pairs/
    struct list_head *node, *safe;
    int cnt = 0;
    list_for_each_safe (node, safe, head) {
        if (cnt++ % 2) {
            safe->prev = node->prev;
            node->prev = node->prev->prev;
            safe->prev->prev = node;
            node->prev->next = node;
            safe->prev->next = safe;
            node->next = safe->prev;
        }
    }
}

/*
 * Reverse elements in queue
 * No effect if q is NULL or empty
 * This function should not allocate or free any list elements
 * (e.g., by calling q_insert_head, q_insert_tail, or q_remove_head).
 * It should rearrange the existing ones.
 */
void q_reverse(struct list_head *head)
{
    struct list_head *node, *safe, *tmp;
    list_for_each_safe (node, safe, head) {
        tmp = node->next;
        node->next = node->prev;
        node->prev = tmp;
    }
    tmp = node->next;
    node->next = node->prev;
    node->prev = tmp;
}

/*
 * Sort elements of queue in ascending order
 * No effect if q is NULL or empty. In addition, if q has only one
 * element, do nothing.
 */
void myInsert(struct list_head *pt, struct list_head *node);

void merge(struct list_head *lptr,
           struct list_head *rptr,
           int size,
           struct list_head *head);

void q_sort(struct list_head *head)
{
    if (!head)
        return;
    int size = q_size(head);
    for (int i = 1; i < size; i *= 2) {
        int l = 2 * i, r = i, cnt = 0;
        struct list_head *lptr = head->next, *rptr = NULL, *node;
        list_for_each (node, head) {
            if (cnt == r)
                rptr = node;
            if (cnt == l) {
                l += 2 * i;
                r += 2 * i;
                merge(lptr, rptr, i, head);
                lptr = node;
                rptr = NULL;
            }
            cnt++;
        }
        if (lptr && rptr)
            merge(lptr, rptr, i, head);
    }
}

/*
 * Insert @node before @pt.
 * Both of its value should not be NULL.
 */
void myInsert(struct list_head *pt, struct list_head *node)
{
    node->next = pt;
    node->prev = pt->prev;
    node->prev->next = node;
    pt->prev = node;
}

/*
 * Merge two successive segment on linked list.
 * @lptr indicates the starting point of first segment and @rptr
 * indicates that of secend segment.
 * @size indicate length of each segment.
 */
void merge(struct list_head *lptr,
           struct list_head *rptr,
           int size,
           struct list_head *head)
{
    int lcnt = 0, rcnt = 0;
    while (lcnt < size && rcnt < size && rptr != head) {
        if (strcmp(list_entry(lptr, element_t, list)->value,
                   list_entry(rptr, element_t, list)->value) > 0) {
            struct list_head *tmp = rptr->next;
            list_del(rptr);
            myInsert(lptr, rptr);
            rptr = tmp;
            rcnt++;
        } else {
            lptr = lptr->next;
            lcnt++;
        }
    }
}
/*
 * Copyright (c) 2010-2013 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

/*
 * tests for simple doubly linked list
 */

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#include <pfc/base.h>
#include <pfc/list.h>

#include "test.h"


/*
 *
 * types for tests
 *
 */

typedef struct {
    pfc_list_t list_hdr;
    int        data;
} list_ent_t;


/*
 *
 * prototype of static functions
 *
 */

PFC_ATTR_UNUSED static void dump_list (pfc_list_t *head);
static int  list_size (pfc_list_t *head);
static void verify_size (pfc_list_t *head, int size);
static void verify_links (pfc_list_t *head);
static void base_list_use_case_1 (int ent_num, int push_tail, int pop_tail);
static void base_list_use_case_2 (int ent_num, int remove_odd, int in_order);
static void base_list_use_case_3 (int ent_num);


/*
 *
 * test cases
 *
 */

TEST(simple_list, use_case_1_1)
{
    int push_tail;
    int pop_tail;
    const int int_end_mark = 999999;
    int ent_num_set[] = { 1, 2, 3, 10, 20, 100, int_end_mark };

    for (push_tail = 0; push_tail != 2; ++push_tail) {
        for (pop_tail = 0; pop_tail != 2; ++pop_tail) {
            int *ep;
            for (ep = ent_num_set; *ep != int_end_mark; ++ep) {
                base_list_use_case_1(*ep, push_tail, pop_tail);
            }
        }
    }
}


TEST(simple_list, use_case_1_2)
{
    const int int_end_mark = 999999;
    int in_order;
    int remove_odd;
    int ent_num_set[] = { 1, 2, 3, 10, 20, 100, int_end_mark };
    int *ep;
    for (in_order = 0; in_order < 2; ++in_order) {
        for (remove_odd = 0; remove_odd < 2; ++remove_odd) {
            for (ep = ent_num_set; *ep != int_end_mark; ++ep) {
                base_list_use_case_2(*ep, remove_odd, in_order);
            }
        }
    }
}


TEST(simple_list, use_case_1_3)
{
    const int int_end_mark = 999999;
    int ent_num_set[] = { 1, 2, 3, 10, 20, 100, int_end_mark };
    int *ep;
    for (ep = ent_num_set; *ep != int_end_mark; ++ep) {
        base_list_use_case_3(*ep);
    }
}


/*
 *
 * static functions (utility)
 *
 */

static void
dump_list (pfc_list_t *head)
{
    int cnt;
    pfc_list_t *plist_p;
    fprintf(stderr, "dump list (head = %p)\n", head);
    for (cnt =0, plist_p = head->pl_next;
         plist_p != head;
         plist_p = plist_p->pl_next)
    {
        list_ent_t *ent_p = (list_ent_t *)plist_p;
        fprintf(stderr,
                "ent[%d] = (addr = %p, prev = %p, next = %p, data = %d)\n",
                cnt, plist_p, plist_p->pl_prev, plist_p->pl_next, ent_p->data);
        ++cnt;
    }
}


static int
list_size (pfc_list_t *head)
{
    int cnt;
    pfc_list_t *plist_p;
    for (cnt =0, plist_p = head->pl_next;
         plist_p != head;
         plist_p = plist_p->pl_next)
    {
        ++cnt;
    }
    return cnt;
}


static void
verify_size (pfc_list_t *head, int size)
{
    EXPECT_EQ(list_size(head), size);
}


static void
verify_links (pfc_list_t *head)
{
    pfc_list_t *plist_p;
    for (plist_p = head->pl_next;
         plist_p != head;
         plist_p = plist_p->pl_next)
    {
        EXPECT_EQ(plist_p->pl_next->pl_prev, plist_p);
    }
}


/*
 *
 * static functions (use case)
 *
 */

/*
 * test combination of (push | push_tail) * (pop | pop_tail)
 */
static void
base_list_use_case_1 (int ent_num, int push_tail, int pop_tail)
{
    int i;
    int size;
    pfc_list_t head;
    list_ent_t *ent = (list_ent_t *)calloc(sizeof(list_ent_t), ent_num);
    if (ent == NULL) abort();

    /* init unchained entries */
    for (i = 0; i < ent_num; ++i) {
        ent[i].data = i;
    }

    /* init list */
    pfc_list_init(&head);
    EXPECT_TRUE(pfc_list_is_empty(&head));
    verify_links(&head);
    verify_size(&head, size = 0);

    /* push or push_tail all */
    for (i = 0; i < ent_num; ++i) {
        if (!push_tail) {
            pfc_list_push(&head, (pfc_list_t *)(ent + i));
        } else {
            pfc_list_push_tail(&head, (pfc_list_t *)(ent + i));
        }
        verify_links(&head);
        verify_size(&head, ++size);
    }

    /* pop or pop_tail all */
    for (i = 0; i < ent_num; ++i) {
        int expect = (push_tail == pop_tail) ? ent_num - i - 1 : i;
        list_ent_t *ent_p;
        if (!pop_tail) {
            ent_p = (list_ent_t *)pfc_list_pop(&head);
        } else {
            ent_p = (list_ent_t *)pfc_list_pop_tail(&head);
        }
        EXPECT_EQ(ent_p->data, expect);
        verify_links(&head);
        verify_size(&head, --size);
    }

    /* list shuld be empty */
    EXPECT_TRUE(pfc_list_is_empty(&head));
    EXPECT_EQ(size, 0);
    verify_size(&head, size);

    /* clean up */
    free(ent);
}


/*
 * test push_tail, remove and pop
 */
static void
base_list_use_case_2 (int ent_num, int remove_odd, int in_order)
{
    int i;
    int size;
    pfc_list_t head;
    list_ent_t *ent = (list_ent_t *)calloc(sizeof(list_ent_t), ent_num);
    if (ent == NULL) abort();

    /* init unchained entries */
    for (i = 0; i < ent_num; ++i) {
        ent[i].data = i;
    }

    /* init list */
    pfc_list_init(&head);
    EXPECT_TRUE(pfc_list_is_empty(&head));
    verify_links(&head);
    verify_size(&head, size = 0);

    /* push_tail all */
    for (i = 0; i < ent_num; ++i) {
        pfc_list_push_tail(&head, (pfc_list_t *)(ent + i));
        verify_links(&head);
        verify_size(&head, ++size);
    }

    /* remove half */
    {
        pfc_list_t **plist_addr =
                (pfc_list_t **)calloc(sizeof(pfc_list_t *), ent_num + 1);
        if (plist_addr == NULL) abort();

        /* list up addresses to remove */
        {
            int pos = in_order ? 0 : (ent_num - 1);
            pfc_list_t **plist_addr_next = plist_addr;
            pfc_list_t  *plist_p;
            if (in_order) {
                PFC_LIST_FOREACH(&head, plist_p) {
                    if ( (pos % 2 == 0 &&  remove_odd) ||
                         (pos % 2 == 1 && !remove_odd) )
                    {
                        *plist_addr_next = plist_p;
                        ++plist_addr_next;
                    }
                    ++pos;
                }
            } else {
                PFC_LIST_REV_FOREACH(&head, plist_p) {
                    if ( (pos % 2 == 0 &&  remove_odd) ||
                         (pos % 2 == 1 && !remove_odd) )
                    {
                        *plist_addr_next = plist_p;
                        ++plist_addr_next;
                    }
                    --pos;
                }
            }
        }

        /* remove addresses */
        {
            pfc_list_t **plist_pp;
            for (plist_pp = plist_addr; *plist_pp != NULL; ++plist_pp) {
                pfc_list_remove(*plist_pp);
                verify_links(&head);
                verify_size(&head, --size);
            }
        }

        /* clean up */
        free(plist_addr);
    }

    /* pop others */
    for (i = 0; i < ent_num; ++i) {
        list_ent_t *ent_p;
        if ( (i % 2 == 0 &&  remove_odd) ||
             (i % 2 == 1 && !remove_odd) )
        {
            /* already removed */
            continue;
        }
        ent_p = (list_ent_t *)pfc_list_pop(&head);
        EXPECT_EQ(ent_p->data, i);
        verify_links(&head);
        verify_size(&head, --size);
    }

    /* list shuld be empty */
    EXPECT_TRUE(pfc_list_is_empty(&head));
    EXPECT_EQ(size, 0);
    verify_size(&head, size);

    /* clean up */
    free(ent);
}


/*
 * test push_tail, replace and move_all
 */
static void
base_list_use_case_3 (int ent_num)
{
    int i;
    int size;
    pfc_list_t ohead;
    pfc_list_t nhead;
    list_ent_t *oent = (list_ent_t *)calloc(sizeof(list_ent_t), ent_num);
    list_ent_t *nent = (list_ent_t *)calloc(sizeof(list_ent_t), ent_num);
    if (oent == NULL) abort();
    if (nent == NULL) abort();

    /* init unchained entries */
    for (i = 0; i < ent_num; ++i) {
        oent[i].data = i;
        nent[i].data = i * 2;
    }

    /* init list */
    pfc_list_init(&ohead);
    EXPECT_TRUE(pfc_list_is_empty(&ohead));
    verify_links(&ohead);
    verify_size(&ohead, size = 0);

    /* push_tail all */
    for (i = 0; i < ent_num; ++i) {
        pfc_list_push_tail(&ohead, (pfc_list_t *)(oent + i));
        verify_links(&ohead);
        verify_size(&ohead, ++size);
    }

    /* replace all */
    for (i = 0; i < ent_num; ++i) {
        pfc_list_replace((pfc_list_t *)(oent + i), (pfc_list_t *)(nent + i));
        verify_links(&ohead);
        verify_size(&ohead, size);
    }

    /* move all */
    pfc_list_move_all(&ohead, &nhead);
    verify_links(&nhead);
    verify_size(&nhead, size);


    /* pop all */
    for (i = 0; i < ent_num; ++i) {
        list_ent_t *ent_p;
        ent_p = (list_ent_t *)pfc_list_pop(&nhead);
        EXPECT_EQ(ent_p->data, i * 2);
        verify_links(&nhead);
        verify_size(&nhead, --size);
    }

    /* list shuld be empty */
    EXPECT_TRUE(pfc_list_is_empty(&nhead));
    EXPECT_EQ(size, 0);
    verify_size(&nhead, size);

    /* clean up */
    free(oent);
    free(nent);
}

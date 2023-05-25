/**
 * \file llist.c
 *
 * \copyright 2017 John Harwell, All rights reserved.
 *
 * SPDX-License-Identifier: MIT
 */

/*******************************************************************************
 * Includes
 ******************************************************************************/
#include "rcsw/ds/llist.h"
#include "rcsw/common/dbg.h"
#include "rcsw/ds/llist_node.h"
#include "rcsw/utils/utils.h"

/*******************************************************************************
 * Constant Definitions
 ******************************************************************************/
#define MODULE_ID M_DS_LLIST

/*******************************************************************************
 * API Functions
 ******************************************************************************/
BEGIN_C_DECLS

struct llist* llist_init(struct llist* list_in,
                         const struct ds_params* const params) {
  RCSW_FPC_NV(NULL,
            params != NULL,
            params->tag == DS_LLIST,
            params->max_elts != 0,
            params->elt_size > 0);

  struct llist* list = NULL;
  if (params->flags & RCSW_DS_NOALLOC_HANDLE) {
    RCSW_CHECK_PTR(list_in);
    list = list_in;
  } else {
    list = malloc(sizeof(struct llist));
    RCSW_CHECK_PTR(list);
  }
  list->current = 0;
  list->flags = params->flags;
  list->first = NULL;

  if (params->flags & RCSW_DS_NOALLOC_NODES) {
    RCSW_CHECK_PTR(params->nodes);
    RCSW_ER_CHECK(
        params->max_elts != -1,
        "ERROR: Cannot have uncapped list length with RCSW_DS_NOALLOC_NODES");

    /* initialize free list of llist_nodes */
    list->space.node_map = (struct allocm_entry*)params->nodes;
    list->space.nodes = (struct llist_node*)(list->space.node_map + params->max_elts);
    allocm_init(list->space.node_map, params->max_elts);
  }

  if (params->flags & RCSW_DS_NOALLOC_DATA) {
    RCSW_CHECK_PTR(params->elements);
    RCSW_ER_CHECK(
        params->max_elts != -1,
        "ERROR: Cannot have uncapped list length with RCSW_DS_NOALLOC_DATA");

    /* initialize free list of data elements */
    list->space.db_map = (struct allocm_entry*)params->elements;
    list->space.datablocks = (uint8_t*)(list->space.db_map + params->max_elts);
    allocm_init(list->space.db_map, params->max_elts);
  }

  if (params->cmpe == NULL && !(params->flags & RCSW_DS_LLIST_PTR_CMP)) {
    DBGW(
        "WARNING: No compare function provided and RCSW_DS_LLIST_PTR_CMP not "
        "passed\n");
  }

  list->first = NULL;
  list->last = NULL;
  list->elt_size = params->elt_size;
  list->cmpe = params->cmpe;
  list->printe = params->printe;
  list->max_elts = params->max_elts;
  list->sorted = FALSE;

  DBGD("elt_size=%zu max_elts=%d flags=0x%08x\n",
       list->elt_size,
       list->max_elts,
       list->flags);
  return list;

error:
  llist_destroy(list);
  errno = ENOMEM;
  return NULL;
} /* llist_init() */

void llist_destroy(struct llist* list) {
  RCSW_FPC_V(NULL != list);

  struct llist_node* curr = list->first;
  struct llist_node* next;

  while (list->current > 0 && curr != NULL) {
    next = curr->next;
    llist_node_destroy(list, curr);
    curr = next;
    --list->current;
  } /* while() */

  if (!(list->flags & RCSW_DS_NOALLOC_HANDLE)) {
    free(list);
  }
} /* llist_destroy() */

status_t llist_clear(struct llist* const list) {
  RCSW_FPC_NV(ERROR, list != NULL);

  struct llist_node* curr = list->first;
  struct llist_node* next;
  while (list->current > 0 && curr != NULL) {
    next = curr->next;
    llist_node_destroy(list, curr);
    curr = next;
  }
  list->current = 0;
  list->first = NULL;
  list->last = NULL;

  return OK;
} /* llist_clear() */

status_t llist_remove(struct llist* const list, const void* const e) {
  RCSW_FPC_NV(ERROR, list != NULL, e != NULL);

  /* can't remove from an empty list */
  if (llist_isempty(list)) {
    DBGE("ERROR: list is empty: cannot remove element\n");
    errno = EINVAL;
    return ERROR;
  }

  struct llist_node* node = llist_node_query(list, e);
  if (node == NULL) { /* node not in list: nothing to do */
    return OK;
  }
  return llist_delete(list, node, NULL);
} /* llist_remove() */

status_t llist_delete(struct llist* const list,
                      struct llist_node* victim,
                      void* const e) {
  /* only one node in list */
  if (list->first == victim && list->last == victim) {
    list->first = NULL;
    list->last = NULL;
  } else if (list->first == victim) { /* victim was first node in list */
    list->first = victim->next;
    list->first->prev = NULL;
  } else if (list->last == victim) { /* victim was last node in list */
    list->last = list->last->prev;
    list->last->next = NULL;
  } else { /* general case */
    victim->next->prev = victim->prev;
    victim->prev->next = victim->next;
  }

  list->current--;
  if (NULL != e) {
    ds_elt_copy(e, victim->data, list->elt_size);
  }
  llist_node_destroy(list, victim);
  return OK;
} /* llist_delete() */

status_t llist_append(struct llist* const list, void* const data) {
  RCSW_FPC_NV(ERROR, list != NULL, data != NULL);

  if (llist_isfull(list) && list->max_elts != -1) {
    DBGE("ERROR: Cannot insert element: no space\n");
    errno = ENOSPC;
    return ERROR;
  }

  struct llist_node* node = llist_node_create(list, data);
  status_t rval = ERROR;
  RCSW_CHECK_PTR(node);

  if (list->last == NULL) { /* empty list */
    list->last = node;
    list->first = node;
    node->next = NULL;
    node->prev = NULL;
  } else { /* general case */
    node->next = NULL;
    list->last->next = node;
    node->prev = list->last;
    list->last = node;
  }
  list->current++;
  if (list->flags & RCSW_DS_SORTED) {
    list->sorted = FALSE;
    llist_sort(list, MSORT_REC);
  }
  rval = OK;

error:
  return rval;
} /* llist_append() */

status_t llist_prepend(struct llist* const list, void* const data) {
  RCSW_FPC_NV(ERROR, list != NULL, data != NULL);

  if (llist_isfull(list) && list->max_elts != -1) {
    DBGE("ERROR: Cannot insert element: no space\n");
    errno = ENOSPC;
    return ERROR;
  }

  struct llist_node* node = llist_node_create(list, data);
  status_t rval = ERROR;
  RCSW_CHECK_PTR(node);

  if (list->first == NULL) { /* empty list */
    list->first = node;
    list->last = node;
    node->prev = NULL;
    node->next = NULL;
  } else { /* general case */
    node->prev = NULL;
    node->next = list->first;
    list->first->prev = node;
    list->first = node;
  }
  list->current++;

  if (list->flags & RCSW_DS_SORTED) {
    list->sorted = FALSE;
    llist_sort(list, MSORT_REC);
  }
  rval = OK;

error:
  return rval;
} /* llist_prepend() */

void llist_print(struct llist* const list) {
  if (list == NULL) {
    DPRINTF("LLIST: < NULL list >\n");
    return;
  }
  if (llist_isempty(list)) {
    DPRINTF("LLIST: < Empty list >\n");
    return;
  }
  if (list->printe == NULL) {
    DPRINTF("LLIST: < No print function >\n");
    return;
  }

  LLIST_FOREACH(list, next, curr) { list->printe(curr->data); }

  DPRINTF("\n");
} /* llist_print() */

void* llist_data_query(struct llist* const list, const void* const e) {
  RCSW_FPC_NV(NULL, list != NULL, e != NULL);

  if (list->cmpe == NULL && !(list->flags & RCSW_DS_LLIST_PTR_CMP)) {
    DBGE("ERROR: Cannot search list: NULL cmpe()\n");
    return NULL;
  }

  struct llist_node* node = llist_node_query(list, e);
  return (NULL == node) ? NULL : node->data;
} /* llist_data_query() */

struct llist_node* llist_node_query(struct llist* const list,
                                    const void* const e) {
  RCSW_FPC_NV(NULL, list != NULL, e != NULL);

  if (list->cmpe == NULL && !(list->flags & RCSW_DS_LLIST_PTR_CMP)) {
    DBGE("ERROR: Cannot search list: NULL cmpe()\n");
    return NULL;
  }

  void* match = NULL;
  LLIST_FOREACH(list, next, curr) {
    if (list->flags & RCSW_DS_LLIST_PTR_CMP) {
      if (curr->data == e) {
        match = curr;
        break;
      }
    } else {
      if (list->cmpe(curr->data, e) == 0) {
        match = curr;
        break;
      }
    }
  } /* LLIST_FOREACH() */
  return match;
} /* llist_node_query() */

status_t llist_sort(struct llist* const list, enum alg_sort_type type) {
  RCSW_FPC_NV(ERROR, list != NULL, list->cmpe != NULL);

  status_t rval = OK;

  /*
   * 0 or 1 element lists already sorted, as are those that have the sorted
   * flag set
   */
  if (list->current <= 1 || list->sorted) {
    DBGD("Already sorted: nothing to do\n");
  } else {
    if (type == MSORT_REC) {
      list->first = mergesort_rec(list->first, list->cmpe, TRUE);
    } else if (type == MSORT_ITER) {
      list->first = mergesort_iter(list->first, list->cmpe, TRUE);
    }

    /* find new list->last */
    list->sorted = TRUE;
    struct llist_node* tmp = list->first;
    size_t count = 1;
    while (tmp->next != NULL) {
      tmp = tmp->next;
      count++;
    }

    if (count != list->current) {
      DBGE("ERROR: Sort truncated list to %zu elements\n", count);
      errno = EAGAIN;
      rval = ERROR;
    }
    list->last = tmp;
  }

  return rval;
} /* llist_sort() */

struct llist* llist_copy(struct llist* const list,
                         const struct ds_params* const cparams) {
  RCSW_FPC_NV(NULL, list != NULL);

  struct ds_params params = {.cmpe = list->cmpe,
                             .printe = list->printe,
                             .elt_size = list->elt_size,
                             .max_elts = list->max_elts,
                             .tag = DS_LLIST,
                             .flags = (cparams == NULL) ? 0 : cparams->flags,
                             .elements =
                                 (cparams == NULL) ? NULL : cparams->elements,
                             .nodes = (cparams == NULL) ? NULL : cparams->nodes};

  struct llist* clist = llist_init(NULL, &params);
  RCSW_CHECK_PTR(clist);

  LLIST_FOREACH(list, next, curr) { llist_append(clist, curr->data); }

  DBGD("Copied list: %zu %zu-byte elements\n", list->current, list->elt_size);
error:
  return clist;
} /* llist_copy() */

struct llist* llist_copy2(struct llist* const list,
                          bool_t (*pred)(const void* const e),
                          const struct ds_params* const cparams) {
  RCSW_FPC_NV(NULL, list != NULL, pred != NULL);

  struct ds_params params = {.cmpe = list->cmpe,
                             .printe = list->printe,
                             .elt_size = list->elt_size,
                             .max_elts = list->max_elts,
                             .tag = DS_LLIST,
                             .flags = (cparams == NULL) ? 0 : cparams->flags,
                             .elements =
                                 (cparams == NULL) ? NULL : cparams->elements,
                             .nodes = (cparams == NULL) ? NULL : cparams->nodes};

  struct llist* clist = llist_init(NULL, &params);
  RCSW_CHECK_PTR(clist);

  LLIST_FOREACH(list, next, curr) {
    if (!pred(curr->data)) {
      continue;
    }
    llist_append(clist, curr->data);
  }
  DBGD("Copied list: %zu %zu-byte elements matched copy predicate\n",
       clist->current,
       clist->elt_size);

error:
  return clist;
} /* llist_copy2() */

struct llist* llist_filter(struct llist* list,
                           bool_t (*pred)(const void* const e),
                           const struct ds_params* const fparams) {
  RCSW_FPC_NV(NULL, list != NULL, pred != NULL);

  struct ds_params params = {.cmpe = list->cmpe,
                             .printe = list->printe,
                             .elt_size = list->elt_size,
                             .max_elts = list->max_elts,
                             .tag = DS_LLIST,
                             .flags = (fparams == NULL) ? 0 : fparams->flags,
                             .elements =
                                 (fparams == NULL) ? NULL : fparams->elements,
                             .nodes = (fparams == NULL) ? NULL : fparams->nodes};

  struct llist* flist = llist_init(NULL, &params);
  RCSW_CHECK_PTR(flist);

  /*
   * Iterate through list, removing matching elements AFTER you have advanced
   * passed
   * them in the iteration, using match, not curr.
   */
  struct llist_node* match = NULL;
  LLIST_FOREACH(list, next, curr) {
    if (match != NULL) {
      llist_append(flist, match->data);
      RCSW_CHECK(llist_remove(list, match->data) == OK);
      match = NULL;
    }
    if (pred(curr->data)) {
      match = curr;
    }
  }
  /* catch corner case where last item in list matched */
  if (match != NULL) {
    llist_append(flist, match->data);
    RCSW_CHECK(llist_remove(list, match->data) == OK);
    match = NULL;
  }

  DBGD(
      "Filtered list: %zu %zu-byte elements filtered out. %zu elements "
      "remain.\n",
      flist->current,
      flist->elt_size,
      list->current);

error:
  return flist;
} /* llist_filter() */

status_t llist_filter2(struct llist* list, bool_t (*pred)(const void* const e)) {
  RCSW_FPC_NV(ERROR, list != NULL, pred != NULL);

  /*
   * Iterate through list, removing matching elements AFTER you have advanced
   * passed them in the iteration, using match, not curr.
   */
  status_t rval = ERROR;
  size_t count = 0;
  struct llist_node* match = NULL;
  LLIST_FOREACH(list, next, curr) {
    if (match != NULL) {
      count++;
      RCSW_ER_CHECK(llist_remove(list, match->data) == OK,
                  "ERROR: Llist_Node remove failed");
      match = NULL;
    }
    if (pred(curr->data)) {
      match = curr;
    }
  }

  /* catch corner case where last item in list matched */
  if (match != NULL) {
    RCSW_ER_CHECK(llist_remove(list, match->data) == OK,
                "ERROR: Llist_Node remove failed");
  }

  rval = OK;
  DBGD(
      "Filtered list: %zu %zu-byte elements filtered out. %zu elements "
      "remain.\n",
      count,
      list->elt_size,
      list->current);

error:
  return rval;
} /* llist_filter2() */

status_t llist_splice(struct llist* list1,
                      struct llist* list2,
                      const struct llist_node* const node) {
  RCSW_FPC_NV(ERROR, list1 != NULL, list2 != NULL, node != NULL);

  if (list1->current + list2->current > (size_t)list1->max_elts &&
      list1->max_elts != -1) {
    DBGE("ERROR: Cannot splice: %zu + %zu > %d (max elements exceeded)\n",
         list1->current,
         list2->current,
         list1->max_elts);
    errno = ENOSPC;
    return ERROR;
  } else if (list1->current == 0 || list2->current == 0) {
    DBGE("ERROR: Cannot splice an empty list\n");
    errno = EINVAL;
    return ERROR;
  }

  size_t count = list1->current;
  struct llist_node* pos = NULL;
  status_t rval = ERROR;

  LLIST_FOREACH(list1, next, curr) {
    /* locate the llist_node to insert at */
    if (curr != node) {
      count--;
      continue;
    }

    if (curr == list1->first) { /* splice at start of list == prepend */
      list1->first->prev = list2->last;
      list2->last->next = list1->first;
      list1->first = list2->first;
      list1->current += list2->current;

    } else if (curr == list1->last) { /* splice at end of list == append */
      list1->last->next = list2->first;
      list2->first->prev = list1->last;
      list1->last = list2->last;
      list1->current += list2->current;

    } else { /* general case: insert list2 into the middle of list1 */
      pos = curr;
      curr = curr->prev;
      curr->next = list2->first;
      list2->first->prev = curr;
      list2->last->next = pos;
      pos->prev = list2->last;
      list1->current += list2->current;
    }

    if (!(list2->flags & RCSW_DS_NOALLOC_HANDLE)) {
      free(list2);
    }
    break;
  }

  if (count == 0) {
    DBGE("ERROR: Could not splice: splice node not found in list1\n");
    errno = EAGAIN;
    goto error;
  }

  rval = OK;
error:
  return rval;
} /* llist_splice() */

status_t llist_map(struct llist* list, void (*f)(void* e)) {
  RCSW_FPC_NV(ERROR, list != NULL, f != NULL);

  LLIST_FOREACH(list, next, curr) { f(curr->data); }

  return OK;
} /* llist_map() */

status_t llist_inject(struct llist* const list,
                      void (*f)(void* e, void* res),
                      void* result) {
  RCSW_FPC_NV(ERROR, list != NULL, f != NULL, result != NULL);

  LLIST_FOREACH(list, next, curr) { f(curr->data, result); }

  return OK;
} /* llist_inject() */

size_t llist_heap_footprint(const struct llist* const list) {
  RCSW_FPC_NV(0, NULL != list);

  size_t size = 0;
  if (list->flags & RCSW_DS_NOALLOC_HANDLE) {
    size += sizeof(struct llist);
  }
  if (list->flags & RCSW_DS_NOALLOC_DATA) {
    size += llist_element_space((size_t)list->max_elts, list->elt_size);
  }
  if (list->flags & RCSW_DS_NOALLOC_NODES) {
    size += llist_node_space((size_t)list->max_elts);
  }

  return size;
} /* llist_heap_footprint() */

END_C_DECLS

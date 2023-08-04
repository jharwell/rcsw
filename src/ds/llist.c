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

#define RCSW_ER_MODNAME RCSW_ER_MODNAME_BUILDER("rcsw","ds","list")
#define RCSW_ER_MODID ekLOG4CL_DS_LLIST
#include "rcsw/ds/llist_node.h"
#include "rcsw/er/client.h"
#include "rcsw/utils/utils.h"
#include "rcsw/common/alloc.h"

/*******************************************************************************
 * API Functions
 ******************************************************************************/
BEGIN_C_DECLS

struct llist* llist_init(struct llist* list_in,
                         const struct llist_params* const params) {
  RCSW_FPC_NV(NULL,
              params != NULL,
              params->max_elts != 0,
              params->elt_size > 0);
  RCSW_ER_MODULE_INIT();


  struct llist* list = rcsw_alloc(list_in,
                                  sizeof(struct llist),
                                  params->flags & RCSW_NOALLOC_HANDLE);
  RCSW_CHECK_PTR(list);
  list->current = 0;
  list->flags = params->flags;
  list->first = NULL;

  if (params->flags & RCSW_NOALLOC_META) {
    RCSW_CHECK_PTR(params->meta);
    ER_CHECK(params->max_elts != -1,
             "Cannot have uncapped list length with RCSW_NOALLOC_META");

    /* initialize free list of llist_nodes */
    list->space.node_map = (struct allocm_entry*)params->meta;
    list->space.nodes = (struct llist_node*)(list->space.node_map + params->max_elts);
    allocm_init(list->space.node_map, params->max_elts);
  }

  if (params->flags & RCSW_NOALLOC_DATA) {
    RCSW_CHECK_PTR(params->elements);
    ER_CHECK(params->max_elts != -1,
             "Cannot have uncapped list length with RCSW_NOALLOC_DATA");

    /* initialize free list of data elements */
    list->space.db_map = (struct allocm_entry*)params->elements;
    list->space.datablocks = (dptr_t*)(list->space.db_map + params->max_elts);
    allocm_init(list->space.db_map, params->max_elts);
  }

  if (params->cmpe == NULL && !(params->flags & RCSW_DS_LLIST_DB_PTR)) {
    ER_WARN("No compare function provided and RCSW_DS_LLIST_DB_PTR not "
            "passed\n");
  }

  list->first = NULL;
  list->last = NULL;
  list->elt_size = params->elt_size;
  list->cmpe = params->cmpe;
  list->printe = params->printe;
  list->max_elts = params->max_elts;
  list->sorted = false;

  ER_DEBUG("elt_size=%zu max_elts=%d flags=0x%08x",
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

  rcsw_free(list, list->flags & RCSW_NOALLOC_HANDLE);
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
    ER_ERR("list is empty: cannot remove element");
    errno = EINVAL;
    return ERROR;
  }

  struct llist_node* node = llist_node_query(list, e);
  if (node == NULL) { /* node not in list: nothing to do */
    return OK;
  }
  return llist_delete(list, node, NULL);
} /* llist_remove() */

status_t
llist_delete(struct llist* const list, struct llist_node* victim, void* const e) {
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
    ER_ERR("Cannot insert element: no space");
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
    list->sorted = false;
    llist_sort(list, ekEXEC_REC);
  }
  rval = OK;

error:
  return rval;
} /* llist_append() */

status_t llist_prepend(struct llist* const list, void* const data) {
  RCSW_FPC_NV(ERROR, list != NULL, data != NULL);

  if (llist_isfull(list) && list->max_elts != -1) {
    ER_ERR("Cannot insert element: no space");
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
    list->sorted = false;
    llist_sort(list, ekEXEC_REC);
  }
  rval = OK;

error:
  return rval;
} /* llist_prepend() */

void llist_print(struct llist* const list) {
  if (list == NULL) {
    DPRINTF(RCSW_ER_MODNAME ": < NULL >\n");
    return;
  }
  if (llist_isempty(list)) {
    DPRINTF(RCSW_ER_MODNAME ": < Empty >\n");
    return;
  }
  if (list->printe == NULL) {
    DPRINTF(RCSW_ER_MODNAME ": < No print function >\n");
    return;
  }

  LLIST_FOREACH(list, next, curr) { list->printe(curr->data); }

  DPRINTF("\n");
} /* llist_print() */

void* llist_data_query(struct llist* const list, const void* const e) {
  RCSW_FPC_NV(NULL, list != NULL, e != NULL);

  if (list->cmpe == NULL && !(list->flags & RCSW_DS_LLIST_DB_PTR)) {
    ER_ERR("Cannot search list: NULL cmpe()");
    return NULL;
  }

  struct llist_node* node = llist_node_query(list, e);
  return (NULL == node) ? NULL : node->data;
} /* llist_data_query() */

struct llist_node* llist_node_query(struct llist* const list,
                                    const void* const e) {
  RCSW_FPC_NV(NULL, list != NULL, e != NULL);

  if (list->cmpe == NULL && !(list->flags & RCSW_DS_LLIST_DB_PTR)) {
    ER_ERR("Cannot search list: NULL cmpe()");
    return NULL;
  }

  void* match = NULL;
  LLIST_FOREACH(list, next, curr) {
    if (list->flags & RCSW_DS_LLIST_DB_PTR) {
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

status_t llist_sort(struct llist* const list, enum exec_type type) {
  RCSW_FPC_NV(ERROR, list != NULL, list->cmpe != NULL);

  status_t rval = OK;

  /*
   * 0 or 1 element lists already sorted, as are those that have the sorted
   * flag set
   */
  if (list->current <= 1 || list->sorted) {
    ER_DEBUG("Already sorted: nothing to do");
  } else {
    if (type == ekEXEC_REC) {
      list->first = mergesort_rec(list->first, list->cmpe, true);
    } else if (type == ekEXEC_ITER) {
      list->first = mergesort_iter(list->first, list->cmpe, true);
    } else {
      ER_ERR("Bad exec_type for sort '%d'", type);
      return ERROR;
    }

    /* find new list->last */
    list->sorted = true;
    struct llist_node* tmp = list->first;
    size_t count = 1;
    while (tmp->next != NULL) {
      tmp = tmp->next;
      count++;
    }

    if (count != list->current) {
      ER_ERR("Sort truncated list to %zu elements", count);
      errno = EAGAIN;
      rval = ERROR;
    }
    list->last = tmp;
  }

  return rval;
} /* llist_sort() */

struct llist* llist_copy(struct llist* const list,
                         uint32_t flags,
                         void* elements,
                         void* nodes) {
  RCSW_FPC_NV(NULL, list != NULL, !(flags & RCSW_NOALLOC_HANDLE));

  struct llist_params params = {
    .cmpe = list->cmpe,
    .printe = list->printe,
    .elt_size = list->elt_size,
    .max_elts = list->max_elts,
    .flags = flags,
    .elements = elements,
    .meta = nodes,
  };

  struct llist* clist = llist_init(NULL, &params);
  RCSW_CHECK_PTR(clist);

  LLIST_FOREACH(list, next, curr) { llist_append(clist, curr->data); }

  ER_DEBUG("Copied list: %zu %zu-byte elements", list->current, list->elt_size);
error:
  return clist;
} /* llist_copy() */

struct llist* llist_copy2(struct llist* const list,
                          bool_t (*pred)(const void* const e),
                          uint32_t flags,
                          void* elements,
                          void* nodes) {
  RCSW_FPC_NV(NULL,
              list != NULL,
              pred != NULL,
              !(flags & RCSW_NOALLOC_HANDLE));

  struct llist_params params = {
    .cmpe = list->cmpe,
    .printe = list->printe,
    .elt_size = list->elt_size,
    .max_elts = list->max_elts,
    .flags = flags,
    .elements = elements,
    .meta = nodes,
  };

  struct llist* clist = llist_init(NULL, &params);
  RCSW_CHECK_PTR(clist);

  LLIST_FOREACH(list, next, curr) {
    if (!pred(curr->data)) {
      continue;
    }
    llist_append(clist, curr->data);
  }
  ER_DEBUG("Copied list: %zu %zu-byte elements matched copy predicate",
           clist->current,
           clist->elt_size);

error:
  return clist;
} /* llist_copy2() */

struct llist* llist_filter(struct llist* list,
                           bool_t (*pred)(const void* const e),
                           uint32_t flags,
                           void* elements,
                           void* nodes) {
  RCSW_FPC_NV(NULL,
              list != NULL,
              pred != NULL,
              !(flags & RCSW_NOALLOC_HANDLE));

  struct llist_params params = {
    .cmpe = list->cmpe,
    .printe = list->printe,
    .elt_size = list->elt_size,
    .max_elts = list->max_elts,
    .flags = flags,
    .elements = nodes,
    .meta = elements,
  };

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

  ER_DEBUG("Filtered list: %zu %zu-byte elements filtered out. %zu elements "
           "remain.",
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
      ER_CHECK(llist_remove(list, match->data) == OK, "Llist_Node remove failed");
      match = NULL;
    }
    if (pred(curr->data)) {
      match = curr;
    }
  }

  /* catch corner case where last item in list matched */
  if (match != NULL) {
    ER_CHECK(llist_remove(list, match->data) == OK, "Llist_Node remove failed");
  }

  rval = OK;
  ER_DEBUG("Filtered list: %zu %zu-byte elements filtered out. %zu elements "
           "remain.",
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
    ER_ERR("Cannot splice: %zu + %zu > %d (max elements exceeded)",
           list1->current,
           list2->current,
           list1->max_elts);
    errno = ENOSPC;
    return ERROR;
  } else if (list1->current == 0 || list2->current == 0) {
    ER_ERR("Cannot splice an empty list");
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

    rcsw_free(list2, list2->flags & RCSW_NOALLOC_HANDLE);
    break;
  }

  if (count == 0) {
    ER_ERR("Could not splice: splice node not found in list1");
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
  if (list->flags & RCSW_NOALLOC_HANDLE) {
    size += sizeof(struct llist);
  }
  if (list->flags & RCSW_NOALLOC_DATA) {
    size += llist_element_space((size_t)list->max_elts, list->elt_size);
  }
  if (list->flags & RCSW_NOALLOC_META) {
    size += llist_meta_space((size_t)list->max_elts);
  }

  return size;
} /* llist_heap_footprint() */

END_C_DECLS

#ifndef STATIC_BINARY_HEAP_H
#define STATIC_BINARY_HEAP_H

#include "defs.h"

#include <stdlib.h>

#ifdef __MACH__
typedef __darwin_ssize_t ssize_t;
#endif

/**
 * Simple max-size binary heap with add, arbitrary delete, delete_root, and top
 * operations.
 *
 * Style matches binheap, which is meant to conform with list.h.
 *
 * Motivation: Linux's prio_heap.h is of fixed size. Litmus's binomial
 * heap may be overkill (and perhaps not general enough) for some applications.
 */

/* Opaque type for sbinheap_node used by users of sbinheap. */
typedef struct sbinheap_node* sbinheap_node_t;

#define SBINHEAP_NODE_INIT() 0
#define SBINHEAP_NODE(name) \
	sbinheap_node_t name = SBINHEAP_NODE_INIT()
/* Use to initialize sbinheap_node_t */
#define INIT_SBINHEAP_NODE(n) \
	(*(n) = 0)


/* Internal node data structure */
struct sbinheap_node {
	/* pointer to the start of the heap buffer */
	ssize_t idx;

	/* facilitates node swapping. */
	struct sbinheap_node **ref_ptr;

	/* pointer to user data */
	void	*data;
};

#define SBINHEAP_BADIDX (-1)
#define SBINHEAP_POISON ((void*)(0xdeadbeef))
#define __SBINHEAP_NODE_INIT \
	{SBINHEAP_BADIDX, SBINHEAP_NODE_INIT(), SBINHEAP_POISON}


/**
 * Signature of compator function.  Assumed 'less-than' (min-heap).
 * Pass in 'greater-than' for max-heap.
 */
typedef int (*sbinheap_order_t)(const struct sbinheap_node *a,
				const struct sbinheap_node *b);

struct sbinheap {
	/* comparator function pointer */
	sbinheap_order_t compare;

	/* current size of the heap */
	size_t size;

	/* maximum size of the heap */
	size_t max_size;

	/* pointer to the allocated heap */
	struct sbinheap_node* buf;
};

#define DECLARE_SBINHEAP(name, compare, size) \
	struct sbinheap_node __sbinheap_buf_##name[size]; \
	struct sbinheap name = {compare, 0, size, __sbinheap_buf_##name}

#define DECLARE_STATIC_SBINHEAP(name, compare, size) \
	static struct sbinheap_node __sbinheap_buf_##name[size] = \
		{[0 ... ((size)-1)] = __SBINHEAP_NODE_INIT}; \
	static struct sbinheap name = {compare, 0, size, __sbinheap_buf_##name}

/**
 * sbinheap_entry - get the struct for this heap node.
 *  Only valid when called upon heap nodes other than the root heap.
 * @ptr:	the heap node.
 * @type:	the type of struct pointed to by binheap_node::data.
 * @member:	unused.
 */
#define sbinheap_entry(ptr, type, member) \
((type *)((ptr)->data))

/**
 * sbinheap_top_entry - get the struct for the node at the top of the heap.
 *  Only valid when called upon the heap heap node.
 * @ptr:	the special heap-heap node.
 * @type:   the type of the struct the head is embedded in.
 * @member:	the name of the binheap_struct within the (type) struct.
 */
#define sbinheap_top_entry(ptr, type, member) \
sbinheap_entry((ptr)->buf, type, member)

/**
 * binheap_delete_root - remove the root element from the heap.
 * @heap:	 heap to the heap.
 */
#define sbinheap_delete_root(heap) \
__sbinheap_delete_root(heap)

/**
 * sbinheap_delete - remove an arbitrary element from the heap.
 * @to_delete:  pointer to node to be removed.
 * @heap:	 heap to the heap.
 */
#define sbinheap_delete(to_delete, heap) \
__sbinheap_delete(*(to_delete), (heap))

/**
 * sbinheap_add - insert an element to the heap
 * new_node: node to add.
 * @heap:	 heap to the heap.
 * @type:	the type of the struct the head is embedded in.
 * @member:	 the name of the binheap_struct within the (type) struct.
 */
#define sbinheap_add(new_node, heap, type, member) \
__sbinheap_add((heap), container_of((new_node), type, member), (new_node))

/**
 * binheap_decrease - re-eval the position of a node (based upon its
 * original data pointer).
 * @heap: heap to the heap.
 * @orig_node: node that was associated with the data pointer
 *			 (whose value has changed) when said pointer was
 *			 added to the heap.
 */
#define sbinheap_decrease(orig_node, heap) \
__sbinheap_decrease((orig_node), (heap))


static inline void INIT_SBINHEAP(struct sbinheap *heap)
{
	static const struct sbinheap_node init_node = __SBINHEAP_NODE_INIT;
	struct sbinheap_node* step;
	for(step = heap->buf; step < heap->buf + heap->max_size; ++step) {
		*step = init_node;
	}
}

/* Returns true if sbinheap is empty. */
static inline int sbinheap_empty(struct sbinheap *heap)
{
	return(heap->size == 0);
}

/* Get the maximum size of the heap */
static inline size_t sbinheap_max_size(struct sbinheap *heap)
{
	return heap->max_size;
}

/* Returns true if sbinheap node is in a heap. */
static inline int sbinheap_is_in_heap(const struct sbinheap_node *node)
{
	return node && (node->idx != SBINHEAP_BADIDX);
}

/* Returns true if sbinheap node is in given heap. */
static inline int sbinheap_is_in_this_heap(const struct sbinheap_node *node,
				const struct sbinheap* heap)
{
	return sbinheap_is_in_heap(node) && ((node - node->idx) == heap->buf);
}

typedef void (*sbinheap_for_each_t)(sbinheap_node_t node, void* args);
/* Visit every node in heap with function fn(args). Visit order undefined. */
void sbinheap_for_each(struct sbinheap *heap,
				sbinheap_for_each_t fn, void* args);

/* Insert an allocated node into a heap */
int __sbinheap_insert(struct sbinheap_node *new_node, struct sbinheap *heap);

/* Allocates, initializes, and adds a node to the heap */
static inline void __sbinheap_add(struct sbinheap* heap,
				void* data, struct sbinheap_node** ret)
{
	struct sbinheap_node *n = 0;
	if (heap->size < heap->max_size) {
		size_t idx = (heap->size)++;

		n = heap->buf + idx;
		n->idx = idx;
		n->data = data;
		n->ref_ptr = ret;
		*ret = n;

		(void)__sbinheap_insert(n, heap);
	}
	else {
		*ret = 0;
	}
}

/**
 * Removes the root node from the heap. The node is removed after coalescing
 * the binheap_node with its original data pointer at the root of the tree.
 *
 * The 'last' node in the tree is then swapped up to the root and bubbled
 * down.
 */
void __sbinheap_delete_root(struct sbinheap *heap);

/**
 * Delete an arbitrary node.  Bubble node to delete up to the root,
 * and then delete to root.
 */
void __sbinheap_delete(struct sbinheap_node *node,
				struct sbinheap *heap);

/**
 * Bubble up a node whose pointer has decreased in value.
 */
void __sbinheap_decrease(struct sbinheap_node *node,
				struct sbinheap *heap);

#endif

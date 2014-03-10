#include "sbinheap.h"

#include <stdio.h>
#include <assert.h>

/* Swaps data between two nodes and track references */
static inline void __sbinheap_swap(struct sbinheap_node *restrict a,
				struct sbinheap_node *restrict b)
{
	*(a->ref_ptr) = b;
	*(b->ref_ptr) = a;
	swap(a->ref_ptr, b->ref_ptr);
	swap(a->data, b->data);
}

static inline struct sbinheap_node* parent(const struct sbinheap_node* n)
{
	size_t p_idx = (n->idx - 1) / 2;
	size_t offset = n->idx - p_idx;
	return ((struct sbinheap_node*)n - offset);
}

static inline struct sbinheap_node* left(const struct sbinheap_node* n,
				size_t limit)
{
	size_t l_idx = 2*(n->idx) + 1;
	if (l_idx < limit) {
		size_t offset = l_idx - n->idx;
		return ((struct sbinheap_node*)n + offset);
	}
	return 0;
}

static inline struct sbinheap_node* right(const struct sbinheap_node* n,
				size_t limit)
{
	size_t r_idx = 2*(n->idx) + 2;
	if (r_idx < limit) {
		size_t offset = r_idx - n->idx;
		return ((struct sbinheap_node*)n + offset);
	}
	return 0;
}

static inline struct sbinheap_node* last(const struct sbinheap *h)
{
	return (h->buf + h->size)-1;
}


static void __sbinheap_for_each(struct sbinheap *heap,
				struct sbinheap_node *n,
				sbinheap_for_each_t fn, void* args)
{
	/* Apply fn to all nodes. Beware of recursion. */

	/* pre-order */
	fn(n, args);

	if(left(n, heap->size))
		__sbinheap_for_each(heap, left(n, heap->size), fn, args);
	if(right(n, heap->size))
		__sbinheap_for_each(heap, right(n, heap->size), fn, args);
}


/* Apply fn to each node. */
void sbinheap_for_each(struct sbinheap *heap,
				sbinheap_for_each_t fn, void* args)
{
	if (!sbinheap_empty(heap))
		__sbinheap_for_each(heap, heap->buf, fn, args);
}


/* bubble node up towards root */
static void __sbinheap_bubble_up(struct sbinheap *heap,
				struct sbinheap_node *node)
{
	const struct sbinheap_node* root = heap->buf;
	const sbinheap_order_t cmp = heap->compare;

	/* let SBINHEAP_POISON data bubble to the top */
	while((node != root) &&
		  ((node->data == SBINHEAP_POISON) || cmp(node, parent(node)))) {
		__sbinheap_swap(parent(node), node);
		node = parent(node);
	}
}


/* bubble node down, swapping with min-child */
static void __sbinheap_bubble_down(struct sbinheap *heap)
{
	const sbinheap_order_t cmp = heap->compare;
	const size_t limit = heap->size;
	struct sbinheap_node *node = heap->buf;

	while(left(node, limit) != 0) {
		if(right(node, limit) && cmp(right(node, limit), left(node, limit))) {
			if(cmp(right(node, limit), node)) {
				__sbinheap_swap(node, right(node, limit));
				node = right(node, limit);
			}
			else {
				break;
			}
		}
		else {
			if(cmp(left(node, limit), node)) {
				__sbinheap_swap(node, left(node, limit));
				node = left(node, limit);
			}
			else {
				break;
			}
		}
	}
}


/**
 * Insert an allocated node into the heap.
 */
int __sbinheap_insert(struct sbinheap_node *new_node,
				struct sbinheap *heap)
{
	/* new_node should point to last(heap->buf) */
	__sbinheap_bubble_up(heap, new_node);

	return 0;
}


/**
 * Removes the root node from the heap.
 *
 * The 'last' node in the tree is then swapped up to the root and bubbled down.
 */
void __sbinheap_delete_root(struct sbinheap *heap)
{
	/* calling delete on empty heap is a bug */

	struct sbinheap_node* l = last(heap);

	/* swap the last node up to the top and bubble it down */
	if (likely(heap->size > 1)) {
		/* reset owner's reference to root node */
		*(heap->buf->ref_ptr) = SBINHEAP_NODE_INIT();

		/* move last node up to root */
		heap->buf->ref_ptr = l->ref_ptr;
		*(heap->buf->ref_ptr) = heap->buf;
		heap->buf->data = l->data;

		/* free the node and shrink the heap */
		l->idx = SBINHEAP_BADIDX;
		heap->size--;

		__sbinheap_bubble_down(heap);
	}
	else {
		/* free the node and shrink the heap */
		*(l->ref_ptr) = SBINHEAP_NODE_INIT();
		l->idx = SBINHEAP_BADIDX;
		heap->size--;
	}
}


/**
 * Delete an arbitrary node.  Bubble node to delete up to the root,
 * and then delete to root.
 */
void __sbinheap_delete(struct sbinheap_node *node,
				struct sbinheap *heap)
{
	/* Unlike __binheap_delete(), we don't preserve the data pointer of
	 * node_to_delete because the node memory is 'freed' by the user when they
	 * call __sbinheap_delete(). In contrast, the user may reuse node memory
	 * of removed nodes with binheaps.
	 */

	/* set data to null to allow node to bubble up to the top. */
	node->data = SBINHEAP_POISON;
	__sbinheap_bubble_up(heap, node);
	__sbinheap_delete_root(heap);
}


/**
 * Bubble up a node whose pointer has decreased in value.
 */
void __sbinheap_decrease(struct sbinheap_node *node,
				struct sbinheap *heap)
{
	__sbinheap_bubble_up(heap, node);
}

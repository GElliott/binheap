#include "sbinheap.h"

/* Swaps data between two nodes and track references */
static inline void __sbinheap_swap(struct sbinheap_node *a, struct sbinheap_node *b)
{
	swap(a->data, b->data);

	*(a->ref_ptr) = b;
	*(b->ref_ptr) = a;
	swap(a->ref_ptr, b->ref_ptr);
}

static inline struct sbinheap_node* parent(struct sbinheap_node* n)
{
	size_t p_idx = (n->idx - 1) / 2;
	size_t offset = n->idx - p_idx;
	return (n - offset);
}

static inline struct sbinheap_node* left(struct sbinheap_node* n, size_t limit)
{
	size_t l_idx = 2*(n->idx) + 1;
	if (l_idx < limit) {
		size_t offset = l_idx - n->idx;
		return (n + offset);
	}
	return 0;
}

static inline struct sbinheap_node* right(struct sbinheap_node* n, size_t limit)
{
	size_t r_idx = 2*(n->idx) + 2;
	if (r_idx < limit) {
		size_t offset = r_idx - n->idx;
		return (n + offset);
	}
	return 0;
}

static inline struct sbinheap_node* last(struct sbinheap *h)
{
	return (h->buf + h->size)-1;
}

static inline struct sbinheap_node* last_alloc(struct sbinheap *h)
{
	return (h->buf + h->alloc_size)-1;
}

static inline int is_root(struct sbinheap_node* n)
{
	return (n->idx == 0);
}

static void __sbinheap_for_each(struct sbinheap *heap, struct sbinheap_node *n, sbinheap_for_each_t fn, void* args)
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
void sbinheap_for_each(struct sbinheap *heap, sbinheap_for_each_t fn, void* args)
{
	if (!sbinheap_empty(heap))
		__sbinheap_for_each(heap, heap->buf, fn, args);
}

/* bubble node up towards root */
static void __sbinheap_bubble_up(struct sbinheap *heap,
				struct sbinheap_node *node)
{
	struct sbinheap_node* root = heap->buf;

	/* let SBINHEAP_POISON data bubble to the top */
	while((node != root) &&
		  ((node->data == SBINHEAP_POISON) ||
		   heap->compare(node, parent(node)))) {
			  __sbinheap_swap(parent(node), node);
			  node = parent(node);
	}
}


/* bubble node down, swapping with min-child */
static void __sbinheap_bubble_down(struct sbinheap *heap)
{
	struct sbinheap_node *node = heap->buf;
	size_t limit = heap->size;
	while(left(node, limit) != 0) {
		if(right(node, limit) &&
		   heap->compare(right(node, limit), left(node, limit))) {
			if(heap->compare(right(node, limit), node)) {
				__sbinheap_swap(node, right(node, limit));
				node = right(node, limit);
			}
			else {
				break;
			}
		}
		else {
			if(heap->compare(left(node, limit), node)) {
				__sbinheap_swap(node, left(node, limit));
				node = left(node, limit);
			}
			else {
				break;
			}
		}
	}
}


int __sbinheap_insert(struct sbinheap_node *new_node,
				struct sbinheap *heap)
{
	/* Unlikely, but the node we're inserting might not be the last one
	 allocated. Swap our node with the next node in the heap. */
	if (unlikely(new_node->idx != heap->size++)) {
		__sbinheap_swap(new_node, last(heap));
		new_node = last(heap);
	}

	__sbinheap_bubble_up(heap, new_node);

	return 0;
}


/**
 * Removes the root node from the heap. The node is removed after coalescing
 * the sbinheap_node with its original data pointer at the root of the tree.
 *
 * The 'last' node in the tree is then swapped up to the root and bubbled
 * down.
 */
void __sbinheap_delete_root(struct sbinheap *heap)
{
	/* swap the last node up to the top and bubble it down */
	if (likely(heap->size != 1)) {
		__sbinheap_swap(heap->buf, last(heap));
		__sbinheap_bubble_down(heap);
	}

	if(heap->alloc_size != heap->size) {
		struct sbinheap_node *last_alloced = last_alloc(heap);
		__sbinheap_swap(last_alloced, last(heap));
		last_alloced->idx = SBINHEAP_BADIDX;
	}

	heap->size--;
	heap->alloc_size--;
}


/**
 * Delete an arbitrary node.  Bubble node to delete up to the root,
 * and then delete to root.
 */
void __sbinheap_delete(struct sbinheap_node *node_to_delete,
				struct sbinheap *heap)
{
	struct sbinheap_node *target = node_to_delete->ref;
	void *temp_data = target->data;

	/* temporarily set data to null to allow node to bubble up to the top. */
	target->data = SBINHEAP_POISON;

	__sbinheap_bubble_up(heap, target);
	__sbinheap_delete_root(heap);

	node_to_delete->data = temp_data;  /* restore node data pointer */
}


/**
 * Bubble up a node whose pointer has decreased in value.
 */
void __sbinheap_decrease(struct sbinheap_node *orig_node,
				struct sbinheap *heap)
{
	struct sbinheap_node *target = orig_node->ref;

	__sbinheap_bubble_up(heap, target);
}

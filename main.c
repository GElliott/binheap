#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <assert.h>
#include <math.h>

#include <stdint.h>

#include <unistd.h>
#include <sys/time.h>

#include "binheap.h"

struct Data
{
	int val;
	struct binheap_node heap_node;
};

int less(struct binheap_node* A, struct binheap_node* B)
{
	struct Data* a = binheap_entry(A, struct Data, heap_node);
	struct Data* b = binheap_entry(B, struct Data, heap_node);

	return(a->val < b->val);
}

void print(struct binheap_node* n, int depth)
{
	if(n == 0)
	{
		printf("+-> %p\n", (void*)0);
		return;
	}

	if(depth > 10) assert(0);

	int i;
	int p=-1, l=-1, r=-1;
	struct Data* d = binheap_entry(n, struct Data, heap_node);

	for(i = 0; i < depth; ++i) printf("    ");

	if(n->parent)
    {
        if(n->parent->data)
            p = binheap_entry(n->parent, struct Data, heap_node)->val;
        else {
            printf("parent has null data!\n"); fflush(0); assert(0);}
    }
	if(n->left)
    {
        if(n->left->data)
            l = binheap_entry(n->left, struct Data, heap_node)->val;
        else {
            printf("left child has null data!\n"); fflush(0); assert(0);}        
        
    }
	if(n->right)
    {
        if(n->right->data)
            r = binheap_entry(n->right, struct Data, heap_node)->val;
        else {
            printf("right child has null data!\n"); fflush(0); assert(0);}        
    }

	printf("+-> %d\t(%p)\t(p = %d, l = %d, r = %d)\n", d->val, n, p, l, r);
	
    if(n->left) { print(n->left, depth+1); }
    if(n->right) { print(n->right, depth+1); }
}

inline void timediff(const struct timespec* start,
	const struct timespec* end,
	struct timespec* out)
{
	if ((end->tv_nsec - start->tv_nsec) < 0)
	{
		out->tv_sec = end->tv_sec - start->tv_sec - 1;
		out->tv_nsec = 1000000000 + end->tv_nsec - start->tv_nsec;
	}
	else
	{
		out->tv_sec = end->tv_sec - start->tv_sec;
		out->tv_nsec = end->tv_nsec - start->tv_nsec;
	}
}

void usage(const char* msg)
{
	if(msg)
	{
		fprintf(stderr, "Error: %s\n", msg);
	}

	fprintf(stderr, "usage: heaptest num_trials num_deletes heap_size\n");

	exit(-1);
}

int main(int argc, char** argv)
{
	if(argc == 1)
	{
		usage(0);
	}
	else if(argc != 4)
	{
		usage("Invalid options.");
	}

	int numTrials = atoi(argv[1]);
	int flip = atoi(argv[2]);
	int SZ = atoi(argv[3]);
	
	int range = 10000;

	struct binheap heap;
	struct Data nodes[SZ];
	int i, t, f;

	uint64_t heapData[numTrials];
	struct timespec start, end;

	clock_gettime(CLOCK_REALTIME, &start);
	srand(start.tv_nsec);

	INIT_BINHEAP(&heap, less);
	for(i = 0; i < SZ; ++i)
	{
		INIT_BINHEAP_NODE(&nodes[i].heap_node);

		nodes[i].val = (int)fabs((float)(rand() % range));
	}

	printf("Starting Heap Test...\n");
	for(t = 0; t < numTrials; ++t)
	{
		clock_gettime(CLOCK_THREAD_CPUTIME_ID, &start);
		for(i = 0; i < SZ; ++i)
		{
			binheap_add(&nodes[i].heap_node, &heap, struct Data, heap_node);
		}
		for(f = 0; f < flip; ++f)
		{
			struct Data* d = binheap_top_entry(&heap, struct Data, heap_node);
            binheap_delete_root(&heap, struct Data, heap_node);
			d->val = (int)fabs((float)(rand() % range));
			binheap_add(&d->heap_node, &heap, struct Data, heap_node);
		}
		while(!binheap_empty(&heap))
		{
			binheap_delete_root(&heap, struct Data, heap_node);
		}
		clock_gettime(CLOCK_THREAD_CPUTIME_ID, &end);
	
		struct timespec diff;
		timediff(&start, &end, &diff);
		uint64_t elapsed = diff.tv_sec*1e6 + diff.tv_nsec/1e3;
		heapData[t] = elapsed;
	}

	printf("Computing Results...\n");

	float sum_h = 0, sum_l = 0;
	for(t = 0; t < numTrials; ++t)
	{
		sum_h += heapData[t];
	}

	printf("heap time (microseconds): %f\n", sum_h/numTrials);

	return(0);
}

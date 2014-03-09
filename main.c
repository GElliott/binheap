#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <math.h>

#include <stdint.h>

#include <unistd.h>

#include "time.h"

#include "binheap.h"
#include "sbinheap.h"

struct Data
{
	int val;
	struct binheap_node heap_node;

	sbinheap_node_t sheap_node;
};

int less(struct binheap_node* A, struct binheap_node* B)
{
	struct Data* a = binheap_entry(A, struct Data, heap_node);
	struct Data* b = binheap_entry(B, struct Data, heap_node);

	return(a->val < b->val);
}

int sless(struct sbinheap_node* A, struct sbinheap_node* B)
{
	struct Data* a = sbinheap_entry(A, struct Data, heap_node);
	struct Data* b = sbinheap_entry(B, struct Data, heap_node);

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

	for(i = 0; i < depth; ++i) printf("	");

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

void timediff(const struct timespec* start,
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


const int RANGE = 10000;


void func(struct binheap_node* node, void* args)
{
	struct Data* d = binheap_entry(node, struct Data, heap_node);
	printf("%d\n", d->val);
}

float test_binheap(int numTrials, int flip, int size, unsigned int seed)
{
	if(size <= 0)
		return 0;

	struct binheap heap;
	struct Data nodes[size];
	int i, t, f;

	uint64_t heapData[numTrials];
	struct timespec start, end;

	srand(seed);

	INIT_BINHEAP(&heap, less);
	for(i = 0; i < size; ++i)
	{
		INIT_BINHEAP_NODE(&nodes[i].heap_node);

		nodes[i].val = (int)fabs((float)(rand() % RANGE));
	}

	for(t = 0; t < numTrials; ++t)
	{
		clk_gettime(CLK_THREAD_CPUTIME, &start);
		for(i = 0; i < size; ++i)
		{
			binheap_add(&nodes[i].heap_node, &heap, struct Data, heap_node);
		}
		for(f = 0; f < flip; ++f)
		{
			struct Data* d = binheap_top_entry(&heap, struct Data, heap_node);
			binheap_delete_root(&heap, struct Data, heap_node);
			d->val = (int)fabs((float)(rand() % RANGE));
			binheap_add(&d->heap_node, &heap, struct Data, heap_node);
		}
		while(!binheap_empty(&heap))
		{
			binheap_delete_root(&heap, struct Data, heap_node);
		}
		clk_gettime(CLK_THREAD_CPUTIME, &end);

		struct timespec diff;
		timediff(&start, &end, &diff);
		uint64_t elapsed = diff.tv_sec*1e6 + diff.tv_nsec/1e3;
		heapData[t] = elapsed;
	}

	float sum_h = 0;
	for(t = 0; t < numTrials; ++t)
	{
		sum_h += heapData[t];
	}
	return sum_h / numTrials;
}


void sfunc(sbinheap_node_t node, void* args)
{
	struct Data* d = sbinheap_entry(node, struct Data, sheap_node);
	printf("%d\n", d->val);
}

float test_sbinheap(int numTrials, int flip, int size, unsigned int seed)
{
	if(size <= 0)
		return 0;

	DECLARE_SBINHEAP(heap, sless, size);
	struct Data nodes[size];
	int i, t, f;

	uint64_t heapData[numTrials];
	struct timespec start, end;

	INIT_SBINHEAP(&heap);

	srand(seed);

	for(i = 0; i < size; ++i)
	{
		nodes[i].val = (int)fabs((float)(rand() % RANGE));
	}

	for(t = 0; t < numTrials; ++t)
	{
		clk_gettime(CLK_THREAD_CPUTIME, &start);
		for(i = 0; i < size; ++i)
		{
			sbinheap_add(&nodes[i].sheap_node, &heap, struct Data, sheap_node);
		}
		for(f = 0; f < flip; ++f)
		{
			struct Data* d = sbinheap_top_entry(&heap, struct Data, sheap_node);
			sbinheap_delete_root(&heap);
			d->val = (int)fabs((float)(rand() % RANGE));
			sbinheap_add(&d->sheap_node, &heap, struct Data, sheap_node);
		}
		while(!sbinheap_empty(&heap))
		{
			sbinheap_delete_root(&heap);
		}
		clk_gettime(CLK_THREAD_CPUTIME, &end);

		struct timespec diff;
		timediff(&start, &end, &diff);
		uint64_t elapsed = diff.tv_sec*1e6 + diff.tv_nsec/1e3;
		heapData[t] = elapsed;
	}

	float sum_h = 0;
	for(t = 0; t < numTrials; ++t)
	{
		sum_h += heapData[t];
	}
	return sum_h / numTrials;
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
	int size = atoi(argv[3]);
	unsigned int seed;
	float avgTrialTime;
	struct timespec t;

	clk_gettime(CLK_REALTIME, &t);
	seed = (unsigned int)t.tv_nsec;

	printf("starting binheap test...\n");
	avgTrialTime = test_binheap(numTrials, flip, size, seed);
	printf("binheap time (microseconds): %f\n", avgTrialTime);

	printf("starting sbinheap test...\n");
	avgTrialTime = test_sbinheap(numTrials, flip, size, seed);
	printf("sbinheap time (microseconds): %f\n", avgTrialTime);

	return(0);
}

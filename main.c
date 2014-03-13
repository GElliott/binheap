#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <math.h>

#include <stdint.h>

#include <unistd.h>

#include "time.h"

#include "binheap.h"
#include "sbinheap.h"

const int RANGE = 10000;

struct Data
{
	int val;
	struct binheap_node heap_node;

	sbinheap_node_t sheap_node;
};

int less(const struct binheap_node* A, const struct binheap_node* B)
{
	struct Data* a = binheap_entry(A, struct Data, heap_node);
	struct Data* b = binheap_entry(B, struct Data, heap_node);

	return(a->val < b->val);
}

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
			(void)binheap_delete_root(&heap, struct Data, heap_node);
			d->val = (int)fabs((float)(rand() % RANGE));
			binheap_add(&d->heap_node, &heap, struct Data, heap_node);
		}
		for(f = 0; f < flip; ++f)
		{
			struct Data* d = &nodes[rand()% size];
			(void)binheap_delete(&d->heap_node, &heap);
			d->val = (int)fabs((float)(rand() % RANGE));
			binheap_add(&d->heap_node, &heap, struct Data, heap_node);
		}
		while(!binheap_empty(&heap))
		{
			(void)binheap_delete_root(&heap, struct Data, heap_node);
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


int sless(const struct sbinheap_node* A, const struct sbinheap_node* B)
{
	struct Data* a = sbinheap_entry(A, struct Data, heap_node);
	struct Data* b = sbinheap_entry(B, struct Data, heap_node);

	return(a->val < b->val);
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
			(void)sbinheap_delete_root(&heap, struct Data, sheap_node);
			d->val = (int)fabs((float)(rand() % RANGE));
			sbinheap_add(&d->sheap_node, &heap, struct Data, sheap_node);
		}
		for(f = 0; f < flip; ++f)
		{
			struct Data* d = &nodes[rand()% size];
			(void)sbinheap_delete(&d->sheap_node, &heap);
			d->val = (int)fabs((float)(rand() % RANGE));
			sbinheap_add(&d->sheap_node, &heap, struct Data, sheap_node);
		}
		while(!sbinheap_empty(&heap))
		{
			(void)sbinheap_delete_root(&heap, struct Data, sheap_node);
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
	int size = atoi(argv[3]);
	unsigned int seed;
	float avgTrialTime;
	struct timespec t;

	clk_gettime(CLK_REALTIME, &t);
	seed = (unsigned int)t.tv_nsec;
	printf("seed: %u\n\n", seed);

	printf("starting binheap test...\n"); fflush(0);
	avgTrialTime = test_binheap(numTrials, flip, size, seed);
	printf("binheap time (microseconds): %f\n\n", avgTrialTime); fflush(0);

	printf("starting sbinheap test...\n"); fflush(0);
	avgTrialTime = test_sbinheap(numTrials, flip, size, seed);
	printf("sbinheap time (microseconds): %f\n\n", avgTrialTime); fflush(0);

	return(0);
}

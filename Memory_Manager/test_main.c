/*
 * Binghamton CS 451/551 Project "Memory manager".
 * You do not need to turn in this file.	
 */

#include "memory_manager.h"
#include "interposition.h"
#include <stdio.h>
#include <assert.h>
#include <limits.h>
#include <unistd.h>
#include<string.h>

#define NUM_ITERATIONS 1000
#define MAX_MALLOC_SIZE 50
#define MAX_ARRAY_SIZE 500
#define MAX_STR_SIZE 50

struct ds {
	size_t sz;
	void* addr;
	int isOccupied; //0 means block is free
	int isUsed; //0 means not used
	char data;
};

int freeEntry(struct ds addresses[]) {
	for (int i = 0; i < MAX_ARRAY_SIZE; ++i) {
		if (addresses[i].isUsed == 0) {
			return i;
		}
	}

	// Increase size of MAX_ARRAY_SIZE
	assert(-1);
	return -1;
}

void initialize(struct ds addresses[]) {
	for (int i = 0; i < MAX_ARRAY_SIZE; ++i) {
		addresses[i].isOccupied = 0;
		addresses[i].isUsed = 0;
		addresses[i].data = '\0';
	}
}

// Check if there exists one free block 
// which was allocated previously and we 
// are exepcting that block to be present now.
int getFreeAddress(struct ds addresses[], size_t actual_size) {
	for (int i = 0; i < MAX_ARRAY_SIZE; ++i) {
		if (addresses[i].sz == actual_size && addresses[i].isUsed == 1
				&& addresses[i].isOccupied == 0) {
			return i;
		}
	}
	return -1;
}

void runLoop() {
	mem_mngr_init();

	struct ds addresses[MAX_ARRAY_SIZE];
	initialize(addresses);

	for (int i = 0; i < NUM_ITERATIONS; ++i) {

		int choice = rand() % 4;

		fprintf(stderr, "Iteration: %d, Choice: %d ", i, choice);
		if (choice == 0) {
			int malSize = rand() % MAX_MALLOC_SIZE;

			size_t actual_size = SLOT_ALLINED_SIZE(malSize);

			fprintf(stderr, ",Malloc Size: %d, Actual Sz: %d ", malSize,
					actual_size);

			void* p = malloc(malSize);

			if (p == NULL) {
				assert(malSize == 0);
				continue;
			}

			struct ds temp;
			temp.sz = actual_size;

			// Check if address is as per expectation
			int ret = getFreeAddress(addresses, actual_size);
			if (ret == -1) {
				temp.addr = p;
				temp.isOccupied = 1;
				temp.isUsed = 1;
				int freeIdx = freeEntry(addresses);
				addresses[freeIdx] = temp;

				// Write something on the data.
				// if we write something and run valgrind
				// and in case address doesn't belong to process
				// then we would memory errors
				addresses[freeIdx].data = 'A' + (random() % 26);
				;
				memset(p, addresses[freeIdx].data, temp.sz);
			} else {
				fprintf(stderr, ",Expecting address: %p ", addresses[ret].addr);
				fprintf(stderr, ",Got address: %p ", p);

				if (addresses[ret].addr != p) {

					mem_mngr_print_snapshot();
					assert(addresses[ret].addr == p);
				}

				addresses[ret].isOccupied = 1;
			}

			fprintf(stderr, ",Add: %p", p);
		} else if (choice == 1) {
			// Free
			for (int i = 0; i < 500; ++i) {
				int idx = rand() % MAX_ARRAY_SIZE;

				if (addresses[idx].isUsed == 1
						&& addresses[idx].isOccupied == 1) {
					// Check data on memory is still present.
					int* p = (int*) addresses[idx].addr;

					if (*((char*) p) != addresses[idx].data) {

						fprintf(stderr, ",Expecting char: %c ",
								addresses[idx].data);
						fprintf(stderr, "Actual Char: %c ", *((char*) p));

						assert(-1);
					}

					free(addresses[idx].addr);
					addresses[idx].isOccupied = 0;

					fprintf(stderr, ",Add: %p", addresses[idx].addr);
					break; // To do this process atmost once.
				}
			}

		} else if (choice == 2) {
			// double free
			for (int i = 0; i < 500; ++i) {
				int idx = rand() % MAX_ARRAY_SIZE;

				if (addresses[idx].isUsed == 1
						&& addresses[idx].isOccupied == 0) {
					// Check data on memory is still present.
					int* p = (int*) addresses[idx].addr;

					free(addresses[idx].addr);

					break; // To do this process atmost once.
				}
			}
		} else if (choice == 3) {

			for (int i = 0; i < 500; ++i) {
				int idx = rand() % MAX_ARRAY_SIZE;

				if (addresses[idx].isUsed == 1
						&& addresses[idx].isOccupied == 1) {
					// Check data on memory is still present.
					int* p = (int*) addresses[idx].addr;
					void *q = (void*)((char*)p+1);

					free(q);
					break; // To do this process atmost once.
				}
			}
		}

		fprintf(stderr, "\n");
		//mem_mngr_print_snapshot();
		//fflush(stdout);

	}
	mem_mngr_leave();
}

int main(int argc, char * argv[]) {

	{
		runLoop();
	}
}

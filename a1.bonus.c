/*
    The Merge Sort to use for Operating Systems Assignment 1 2019
    written by Robert Sheehan

    Modified by: Kungeng Wu
    UPI: kwu849

    By submitting a program you are claiming that you and only you have made
    adjustments and additions to this code.
 */

#include <stdio.h> 
#include <stdlib.h> 
#include <unistd.h> 
#include <string.h>
#include <sys/resource.h>
#include <stdbool.h>
#include <sys/mman.h>
#include <sys/wait.h>

#define SIZE    2
#define STK_SIZE 100000000

int cores, count = 0;

struct block {
    int size;
    int *first;
};

// void print_block_data(struct block *blk) {
//     printf("size: %d address: %p\n", blk->size, blk->first);
// }

/* Combine the two halves back together. */
void merge(struct block *left, struct block *right) {
	int* combined = malloc(sizeof(int) * (left->size + right->size));
	int dest = 0, l = 0, r = 0;
	while (l < left->size && r < right->size) {
		if (left->first[l] < right->first[r])
			combined[dest++] = left->first[l++];
		else
			combined[dest++] = right->first[r++];
	}
	while (l < left->size)
		combined[dest++] = left->first[l++];
	while (r < right->size)
		combined[dest++] = right->first[r++];
    memmove(left->first, combined, (left->size + right->size) * sizeof(int));
    free(combined);
}

/* Merge sort the data. */
void merge_sort(struct block *my_data) {
    // print_block_data(my_data);
    if (my_data->size > 1) {
        struct block left_block;
        struct block right_block;
        left_block.size = my_data->size >> 2;
        left_block.first = my_data->first;
        right_block.size = left_block.size + (my_data->size % 2);
        right_block.first = my_data->first + left_block.size;
	if(count < cores){
		int pid,status;
		if((pid = fork()) > 0){
			count++;
			merge_sort(&left_block);
			waitpid(pid, &status, 0);
			merge(&left_block, &right_block);
		} else {
			count = cores+1;
			merge_sort(&right_block);
			exit(EXIT_SUCCESS);
		}
	} else {
		merge_sort(&left_block);
		merge_sort(&right_block);
		merge(&left_block, &right_block);
	}
    }
}

/* Check to see if the data is sorted. */
bool is_sorted(int data[], int size) {

    bool sorted = true;
    for (int i = 0; i < size - 1; i++) {
        if (data[i] > data[i + 1])
            sorted = false;
    }
    return sorted;
}

int main(int argc, char *argv[]) {
    cores = sysconf(_SC_NPROCESSORS_ONLN);

    printf("cpu numbers =%d",cores);
    long size;
    int pid, status; 
    if (argc < 2) {
	size = SIZE;
    } else {
	size = atol(argv[1]);
    }
    struct block start_block, l_block, r_block;
    int *data = mmap(NULL, size*sizeof(int), PROT_READ|PROT_WRITE, MAP_SHARED|MAP_ANONYMOUS, -1, 0); 
    start_block.size = size;
    start_block.first = data;
    for (int i = 0; i < size; i++) {
        data[i] = rand();
    }

    l_block.size = start_block.size / 2;
    l_block.first = start_block.first;
    r_block.size = l_block.size + (start_block.size % 2);
    r_block.first = start_block.first + l_block.size;
    
    printf("starting---\n");
    if((pid = fork()) > 0){
        merge_sort(&l_block);
	waitpid(pid, &status, 0);
        merge(&l_block, &r_block);
        printf("---ending\n");
        printf(is_sorted(data, size) ? "sorted\n" : "not sorted\n");
    } else {
        merge_sort(&r_block);
    }
    exit(EXIT_SUCCESS);
}

#include <cstdlib>
#include <list>
#include <unistd.h>
#include <cstring>
#include <stdlib.h>
#include <cmath>
#include <sys/mman.h>

//TODO: for debugging (delete later)
#include <iostream>
#include <cstdio>

#define MAX_ALLOCATION_SIZE pow(10,8)
#define MMAP_THRESHOLD 128000

//TODO: for debugging
//#include <stdio.h>

int number = 1;
int number_2 = 1;
int number_3 = 1;

class MetaData {
public:
	void* start_of_space;
	unsigned int block_size;  //block size here is without the metadata size
	bool is_free;
	MetaData* next;

	//TODO: check size
	int k;
	int j;
	int q;

	inline bool is_mmaped(){
		return this->block_size >= MMAP_THRESHOLD;
	}

};

void* problem_1(MetaData* iterator, size_t size);
void problem_2();
void* problem_3(size_t size);
void* smalloc(size_t size);
void* scalloc(size_t num, size_t size);
void sfree(void* p);
void* srealloc(void* oldp, size_t size);
size_t _num_free_blocks();
size_t _num_free_bytes();
size_t _num_allocated_blocks();
size_t _num_allocated_bytes();
size_t _size_meta_data();
size_t _num_meta_data_bytes();



MetaData* list_head = NULL;
MetaData* list_tail = NULL;
MetaData* mmaped_list_head = NULL;
//TODO: what's this?
MetaData* test = NULL;



void* problem_1(MetaData* iterator, size_t size) {
		
        //the new free metadata in the leftoff of the block
	MetaData* leftoff = (MetaData*)(iterator->start_of_space);
	leftoff += size / sizeof(MetaData);
         
        //the new free metadata's attributes
	leftoff->start_of_space = leftoff + 1;
	leftoff->block_size = iterator->block_size - size - sizeof(MetaData);
	leftoff->is_free = true;
	leftoff->next = iterator->next;
	
	// The current MetaData's attributes
	iterator->next = leftoff;
	iterator->block_size = size;
	iterator->is_free = false;
	
        // tail
	if (leftoff->next == NULL) {
		list_tail = leftoff;
	}

	return (iterator->start_of_space);
}

void problem_2() {
	
	//TODO: for debugging
	printf("entering problem #2, entrance #%d \n", number_2);
	number_2++;
	
	//printf ("Entering Problem #2\n");
	MetaData* iterator = list_head;
	while (iterator != NULL) {
		if (iterator->next != NULL) {
			if (iterator->is_free == true && iterator->next->is_free == true) {
				//printf ("Problem #2 is Activated!\n");
				iterator->block_size = iterator->block_size 
					+ iterator->next->block_size + sizeof(MetaData);
				iterator->next = iterator->next->next;
				if (iterator->next == NULL) {
					list_tail = iterator;
				}
				if (iterator->next != NULL) {
					//if 3 blocks are free in a row
					if (iterator->next->is_free == true) {
						printf ("Problem #2 is Activated with triple merge!!!\n"); // TODO: for debugging
						iterator->block_size = iterator->block_size 
							+ iterator->next->block_size + sizeof(MetaData);
						iterator->next = iterator->next->next;
						if (iterator->next == NULL) {
							list_tail = iterator;
						}
					}
				}
			}
		}
		iterator = iterator->next;
	}
}

void* problem_3(size_t size) {
	
	//TODO: for debugging
	printf("Entering problem #3, entrance #%d\n", number_3);
	number_3++;
	
	//printf("Entering Problem #3\n");
	
	void* temp = sbrk((int)size - (int)list_tail->block_size);
	
	//check whether sbrk had succeeded
	if (temp == (void*)-1) {
		return NULL;
	}
	
	list_tail->block_size = size;
	list_tail->is_free = false;
	list_tail->next = NULL;
	return list_tail->start_of_space;
	
}


void* smalloc(size_t size) {
	
	printf("Entering >Malloc< Part || #1\n");
	//Check that size is legit
	if (size == 0 || size > MAX_ALLOCATION_SIZE) {
		printf("Entering >Malloc< Part || FAIL || size = %zu\n", size);
		return NULL;
	}

	//#problem_4
	if(size >= MMAP_THRESHOLD){
		//allocate by mmap
		void* new_area = mmap(NULL, size,
				PROT_WRITE | PROT_READ, MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);
		MetaData* metaData = (MetaData*)new_area;

		//MetaData attributes
		metaData->start_of_space = metaData + 1;
		metaData->block_size = size;
		metaData->is_free = false;
		metaData->next = NULL;

		//insert to list
		if(mmaped_list_head == NULL)
			mmaped_list_head = metaData;
		else {
			//find end of list
			MetaData* iterator = mmaped_list_head;
			while(iterator->next != NULL)
				iterator = iterator->next;
			iterator->next = metaData;
		}
		return metaData->start_of_space;
	}

	//// TODO: For malloc_4
	//if (size % 4 != 0) {
    //    printf("Entering Problem #5 (formerly Problem #4)\n");
	//	size = size + 4 - (size%4);
	//}
	
	// Creating a new block (MetaData)
	void* new_meta_data = sbrk(sizeof(MetaData));
	if (new_meta_data == (void*)-1) {
		return NULL;
	}
	
	// Check if this is the first block ever
	if (list_head == NULL) {
		printf("Entering >Malloc< Part || #1 (list_head == NULL) First metadata.\n");
		((MetaData*)new_meta_data)->block_size = size;

		((MetaData*)new_meta_data)->is_free = false;
		((MetaData*)new_meta_data)->start_of_space = sbrk(size);
		
		if (((MetaData*)new_meta_data)->start_of_space == (void*)-1) {
			return NULL;
		}
	
		list_head = (MetaData*)new_meta_data;
		
		list_tail = list_head;
		return ((MetaData*)new_meta_data)->start_of_space;
	}

	
	MetaData* iterator = list_head;
	
	// check if there is space available
	printf("Entering >Malloc< Part || #2\n");	
	while (iterator != NULL) {
		printf("Entering >Malloc< Part || #2.1\n");
		if (iterator->is_free == true && iterator->block_size >= size) {
			printf("Entering >Malloc< Part || #2.2\n");
			// Malloc 3 problem #1
			if (iterator->block_size >= size + 128 + sizeof(MetaData)) {
				printf("Entering >Malloc< Part || #2.3\n");
				return problem_1(iterator, size);
			}
			printf("Entering >Malloc< Part || #2.4\n");
			iterator->is_free = false;
			printf("Entering >Malloc< Part || #2.5\n");
			return (iterator->start_of_space);
		}
		printf("Entering >Malloc< Part || #2.6\n");
		iterator = iterator->next;
	}
	
	printf("Entering >Malloc< Part || #2.7\n");
	if (list_tail->is_free == true) {
		printf("Entering >Malloc< Part || #2.8\n");
		// Malloc_3 Problem #4
		if (size < MMAP_THRESHOLD) {
			return problem_3(size);
		}
	}
	printf("Entering >Malloc< Part || #2.9\n");
	iterator = list_head;
	
	
	// Make iterate point to begining and incerement it one by one till it reaches the end of list.
	printf("Entering >Malloc< Part || #3\n");
	
	((MetaData*)new_meta_data)->block_size = size;
	
	// Malloc_3 Problem #4
	if (size >= MMAP_THRESHOLD) {
		printf("Entering MMAP #2\n");
		//TODO: fix this.
		((MetaData*)new_meta_data)->start_of_space = 
				mmap(0, size, PROT_WRITE | PROT_READ, MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);
	}
	else {
		((MetaData*)new_meta_data)->start_of_space = sbrk(size);
	}
	
	if (((MetaData*)new_meta_data)->start_of_space == (void*)-1) {
		//printf("Entering >Malloc< Part || FAIL || previous_program_break == -1\n");
		return NULL;
	}
		
	printf("Entering >Malloc< Part || #4\n");
	
	iterator = list_head;
	while (iterator != NULL) {
		if (iterator->next == NULL) {
			iterator->next = (MetaData*)new_meta_data;
			((MetaData*)new_meta_data)->next = NULL;
			break;
		}
		iterator = iterator->next;
	}
	
	list_tail = ((MetaData*)new_meta_data);
	
	return ((MetaData*)new_meta_data)->start_of_space;
}


void* scalloc(size_t num, size_t size) {
	
	//Check that size is legit
	//printf("Entering >Calloc< Part || #1\n");
	if (num * size == 0 || num * size > MAX_ALLOCATION_SIZE) {
		return NULL;
	}
	
	//printf("Entering >Calloc< Part || #2\n");
	void* result = smalloc (size * num);
	void* memset_result = memset(result, 0, (size * num));
	
	return result;
}

void sfree(void* p) {
	if (p == NULL) {
		return;
	}

	//find the metadata of the block
	MetaData* meta_data = (MetaData*)p;
	meta_data--;

	//free it
	if(meta_data->is_mmaped()){
		//remove from list
		if(mmaped_list_head == meta_data)
			mmaped_list_head = meta_data->next;
		else{
			MetaData* iterator = mmaped_list_head;
			while(iterator->next != meta_data)
				iterator = iterator->next;
			iterator->next = meta_data->next;
		}
		//unmap both metadata and block
		munmap(meta_data, sizeof(MetaData) + meta_data->block_size);
	} else
		meta_data->is_free = true;

	printf("number of allocated blocks before problem_2() == %d", _num_allocated_blocks());

	// problem #2
	problem_2();
	return;
}

void* srealloc(void* oldp, size_t size) {
	//Check that size is legit
	if (size == 0 || size > MAX_ALLOCATION_SIZE) {
		return NULL;
	}
	
	
	//if oldp is null just allocate new place and return it.
	if (oldp == NULL) { 
		void* new_place = smalloc(size);
		return new_place;
	}


	// go to the MetaData of the oldp.
	MetaData* meta_data = (MetaData*)oldp;
	meta_data--;

	meta_data->is_free = true;
	problem_2();
	

    // if there's enough space in the block for the reallocation, keep it
    //unless it's mmaped
    if ((meta_data->block_size >= size) and !meta_data->is_mmaped()) {
        if (meta_data->block_size >= size + 128 + sizeof(MetaData)) {
			meta_data->is_free = false;
            return problem_1(meta_data, size);
        }

        return oldp;
    }
    // if it doesn't but it is the wilderness, keep it
    if (meta_data == list_tail) {
		// Malloc_3 Problem #4
		if (size < MMAP_THRESHOLD) {
			return problem_3(size);
		}
    }

    // else allocate a new block
	//replace oldp with new_place and free its
	void* new_place = smalloc(size);
	if(new_place == NULL)
		return NULL;
	memcpy(new_place, oldp, meta_data->block_size);
	sfree(oldp);
	return new_place;
}

size_t _num_free_blocks() {
	
	size_t counter = 0;
	MetaData* iterator = list_head; 
	while (iterator) {
		if (iterator->is_free == true) {
			counter ++;
		}
		iterator = iterator->next;
	}
	//iterator = mmaped_list_head;
	//while (iterator) {
	//	if (iterator->is_free == true) {
	//		counter ++;
	//	}
	//	iterator = iterator->next;
	//}
	
	return counter;
	//return free_block_counter;
}

size_t _num_allocated_blocks() {
	size_t counter = 0;
	MetaData* iterator = list_head; 
	while (iterator) {
		counter++;
		iterator = iterator->next;
	}
	//iterator = mmaped_list_head;
	//while (iterator) {
	//	if (iterator->is_free == false) {
	//		counter++;
	//	}
	//	iterator = iterator->next;
	//}
	//
	return counter;
	//return block_counter;
}

size_t _num_free_bytes() {
	
	size_t counter = 0;
	MetaData* iterator = list_head;
	while (iterator != NULL) {
		if (iterator->is_free)
			counter += iterator->block_size;
		iterator = iterator->next;
	}

	//iterator = mmaped_list_head;
	//while (iterator != NULL) {
	//	if (iterator->is_free)
	//		counter += iterator->block_size;
	//	iterator = iterator->next;
	//}
	
	return counter;
	//return free_byte_counter;
}

size_t _num_allocated_bytes() {
	
	size_t counter = 0;
	MetaData* iterator = list_head; 
	while (iterator != NULL) {
		counter += iterator->block_size;
		iterator = iterator->next;
	}

	//iterator = mmaped_list_head;
	//while (iterator != NULL) {
	//	counter += iterator->block_size;
	//	iterator = iterator->next;
	//}

	return counter;
	//return byte_counter;
}

size_t _size_meta_data() {
	MetaData metadata;
	size_t size_of_meta_data = sizeof(metadata);
	return size_of_meta_data;
}

size_t _num_meta_data_bytes() {
	
	return _num_allocated_blocks() * (_size_meta_data());
}



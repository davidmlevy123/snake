#pragma once
#include <tchar.h>
#include "additions.h"
#include <windows.h>
typedef struct {
	void* data;
	size_t type_size;
	UINT head, size, max;
}queue;

// Funtion to create queue normally: 
// data - what we want in array(NULL if nothing for now).
// type_size - sizeof(type) when type is the type of elements we put in array, needed.
// max - max size of the queue, 0 for default(100).
// size - the current amount of elements that we are putting in, meaning amount of items in data. 0 for not putting in now.
// head - the current spot in data to count as the first element, 0 for all of data goes into queue.
queue create_queue(void* data, UINT type_size, UINT max, UINT size, UINT head);

void copy(const void* from, void* into, const size_t type_size); // copys from var to var.

// Returns TRUE if copied and FALSE if it didnt.
BOOL pop(queue* q, void* out_item);

// Return TRUE if added and FALSE if not(full so no room to add).
BOOL push(queue* q, void* item_to_add);

// Look at the front of queue without removing.
void* front(const queue* q);
void* place(const queue* q, const int index);
void* tail(const queue* q);

// Functions to tell if the queue is full/empty
BOOL is_empty(const queue* q);
BOOL is_full(const queue* q);

// Frees the mem that the queue took.
void destroy_queue(queue* q);


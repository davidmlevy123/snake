#include "queue.h" 

queue create_queue(void* data, UINT type_size, UINT max, UINT size, UINT head) {
	if (type_size == 0) {
		queue error = { 0 };
		// max can never be 0 because then we dont have a queue so we return a queue with max=0 indecating there is an error.
		error.max = 0;
		return error;
	}
	queue new_q = { 0 };
	// If we dont have a spesific max we set it to the default(100).
	if (max == 0) {
		new_q.max = DEFAULT_MAX_Q;
	}
	else {
		new_q.max = max;
	}
	new_q.data = HeapAlloc(GetProcessHeap(), 0, new_q.max * type_size);
	new_q.type_size = type_size;
	new_q.head = head;
	new_q.size = size;

	// If we dont have any data too put in we just return it without any data in data but mem was allocated.
	if (data == NULL) {
		return new_q;
	}

	copy(data, new_q.data, size * type_size);
	return new_q;
}

void copy(const void* from, void* into, const int type_size)
{
	const char* from_bytes = (const char*)from;
	char* into_bytes = (char*)into;

	// copy the info byte by byte.
	for (int i = 0; i < type_size; ++i) {
		into_bytes[i] = from_bytes[i];
	}
}

BOOL pop(queue* q, void* out_item) {
	void* first = front(q);
	if (first == NULL) {
		return FALSE;
	}
	if (out_item != NULL) {
		copy(first, out_item, q->type_size);
	}
	// Move the head forward 1(loop at end).
	q->head = (q->head + 1) % q->max;
	// We lower the size becasue we took one element out.
	q->size--;

	// We return true becsause it worked.
	return TRUE;
}
BOOL push(queue* q, void* item_to_add) {
	if (is_full(q)) {
		return FALSE;
	}

	// Same idea as pop just flipped.
	int offset = q->type_size * ((q->size + q->head) % (q->max));
	if (item_to_add != NULL)
	{
		copy(item_to_add, (char*)q->data + offset, q->type_size);
		q->size++;
	}
	// No need to change head because we didnt touch it.
	// We add to size becasue we added an element.

	// We return true becsause it worked.
	return TRUE;
}

void* front(const queue* q) {
	if (is_empty(q)) {
		return NULL;
	}
	return place(q, 0);
}
void* place(const queue* q, const int index) {
	// Because we start at head we need to go index places from there meaning there can be a loop around so we mod the answer by the size of the queue.
	int real_index = (index + q->head) % q->max;
	// Get the offset
	int offset = real_index * q->type_size;

	// We get all the bytes of data and put them into the ans_bytes
	char* ans_bytes = (char*)q->data;
	
	// We return the address of the idex spot.
	return (void*)(ans_bytes + offset);
}
void* tail(const queue* q) {
	if (is_empty(q)) { 
		return NULL; 
	}
	return place(q, q->size - 1);
}

BOOL is_empty(const queue* q) {
	return (q->size == 0);
}
BOOL is_full(const queue* q) {
	return (q->size == q->max);
}

void destroy_queue(queue* q) {
	if (q->data != NULL)
	{
		HeapFree(GetProcessHeap(), 0, q->data);
		q->data = NULL;
		q->size = 0;
	}
}

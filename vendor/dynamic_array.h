#ifndef SIMPLE_DYNAMIC_ARRAY
#define SIMPLE_DYNAMIC_ARRAY
#include <stdlib.h>

//the simplest dyarr implementation i could come up with in 3 minutes.

void __da_append(void* array, void* to_append, size_t size);
void __da_pop_back(void* array);
void __da_free(void* array);
#define da_append(array, to_append)\
	__da_append(&(array), &to_append, sizeof(to_append))
#define da_pop(array) __da_pop_back(&(array))
#define da_free(array) __da_free(&(array))
#define da_top(array) ((array).data[(array).size-1])
#define da_at(array, index) ((array).data[index])

#define da_type(type) struct {type* data; size_t size, capacity;}

#endif //SIMPLE_DYNAMIC_ARRAY

#ifdef SIMPLE_DA_IMPL
#include <string.h>

struct __DA {
	void* data;
	size_t size;
	size_t capacity;
};

void __da_append(void* array, void* to_append, size_t size) {
	struct __DA* a = (struct __DA*)array;
	if(!a->data) {
		a->capacity = size*2;
		a->data = calloc(1, a->capacity);
		a->size = 0;
	}

	const size_t used = a->size*size;

	if(used >= a->capacity) {
		a->capacity *= 2;
		a->data = realloc(a->data, a->capacity);
	}

	memcpy((char*)(a->data)+used, to_append, size);
	++a->size;
}

void __da_pop_back(void* array) {
	struct __DA* a = (struct __DA*)array;
	if(a->size > 0)
		--a->size;
}

void __da_free(void* array) {
	if(!array) return;
	struct __DA* a = (struct __DA*)array;
	if(a->data)
		free(a->data);
	a->data = 0;
	a->size = 0;
	a->capacity = 0;
}

#endif // SIMPLE_DA_IMPL

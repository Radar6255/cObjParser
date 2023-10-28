#ifndef DATASTRUCT_H
#define DATASTRUCT_H
#include <stdint.h>
#include <stdbool.h>

typedef struct HashSetElemS{
	uint64_t hash;
	void* element;
} HashSetElem;

typedef struct HashSetS{
	int maxSize;
	int curSize;
	HashSetElem** elements;
} HashSet;

HashSet createHashSet();
bool insertSetElement(HashSet* input, uint64_t hash, void* element);
void** getElements(HashSet* input);

#endif

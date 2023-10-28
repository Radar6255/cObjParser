#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>

#include "dataStruct.h"

#define EXPANDING_FACTOR 2
#define RESIZE_PERCENT 0.8
#define DEFAULT_SIZE 10

HashSet createHashSet(){
	HashSet out;
	out.maxSize = DEFAULT_SIZE;
	out.curSize = 0;

	out.elements = calloc(out.maxSize, sizeof(HashSetElem*));

	return out;
}

bool resizeHashSet(HashSet* input){
	int lastMax = input->maxSize;
	input->maxSize = input->maxSize * EXPANDING_FACTOR;
	input->elements = realloc(input->elements, sizeof(HashSetElem*) * input->maxSize);
	if(!input->elements){
		printf("Got NULL back from realloc\n");
		return false;
	}

	for(int i = lastMax; i < input->maxSize; i++){
		input->elements[i] = NULL;
	}

	return true;
}

bool insertSetElement(HashSet* input, uint64_t hash, void* element){
	/* printf("Cur: %d, Max: %d\n", input->curSize, input->maxSize); */
	if (input->curSize / ((float) input->maxSize) > RESIZE_PERCENT){
		resizeHashSet(input);
		printf("Resized set\n");
	}

	if (!input->elements[hash % input->maxSize]){
		// This means that there isn't already an entry for that hash and that we can add it
		input->elements[hash % input->maxSize] = malloc(sizeof(HashSetElem));
		input->elements[hash % input->maxSize]->hash = hash;
		input->elements[hash % input->maxSize]->element = element;
		input->curSize++;
		return true;
	}else{
		for(int i = 0; i < input->maxSize; i++){
			if (!input->elements[(hash + i) % input->maxSize]){
				// Means that something was in the spot but it isn't already in the set
				input->elements[(hash + i) % input->maxSize] = malloc(sizeof(HashSetElem));
				input->elements[(hash + i) % input->maxSize]->hash = hash;
				input->elements[(hash + i) % input->maxSize]->element = element;
				input->curSize++;
				return true;
			}else{
				if(input->elements[(hash + i) % input->maxSize]->hash == hash){
					// This means that the hash is already in the set and wasn't added
					return false;
				}
			}
		}

        return false;
	}
}

void** getElements(HashSet* input){
	void** out = malloc(sizeof(void*) * input->curSize);
	int curPos = 0;

	for(int i = 0; i < input->maxSize; i++){
		if (input->elements[i]){
			out[curPos] = input->elements[i]->element;
			curPos++;
			free(input->elements[i]);

			if(curPos > input->curSize){
				printf("WARNING: Size of set got off track at some point\n");
				printf("CurSize was %d and the new size is %d\n", input->curSize, curPos);
			}
		}
	}

    return out;
}

#ifdef DEBUG
int main(){
	HashSet test = createHashSet();

	bool out = insertSetElement(&test, 432, NULL);
	printf("%d == 1\n", out);

	out = insertSetElement(&test, 432, NULL);
	printf("%d == 0\n", out);

	out = insertSetElement(&test, 252, NULL);
	printf("%d == 1\n", out);

	out = insertSetElement(&test, 20, NULL);
	printf("%d == 1\n", out);

	out = insertSetElement(&test, 21, NULL);
	out = insertSetElement(&test, 22, NULL);
	out = insertSetElement(&test, 23, NULL);
	out = insertSetElement(&test, 24, NULL);
	out = insertSetElement(&test, 25, NULL);
	out = insertSetElement(&test, 26, NULL);
	out = insertSetElement(&test, 27, NULL);
	out = insertSetElement(&test, 28, NULL);
	out = insertSetElement(&test, 29, NULL);
	out = insertSetElement(&test, 37, NULL);
	void** temp = getElements(&test);
}
#endif

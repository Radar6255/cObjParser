#include <stdio.h>
#include <stdlib.h>

typedef struct list
{
    unsigned int size;
    unsigned int maxSize;
    void** elements;
} listS;


#define LIST_EXPANDING_FACTOR 2
#define LIST_RESIZE_PERCENT 0.90f

void* createList(int maxSize){
    listS* out = malloc(sizeof(listS));

    if (!maxSize){
        maxSize = 10;
    }
    out->maxSize = maxSize;

    out->elements = malloc(sizeof(void*) * maxSize);
    out->size = 0;

    return out;
}

unsigned int getListSize(void* in){
    listS* inList = (listS*) in;

    return inList->size;
}

void** getListElements(void* in){
    listS* inList = (listS*) in;

    return inList->elements;
}

void** listToArray(void* in, unsigned int* size){
    listS* inList = (listS*) in;

    inList->elements = realloc(inList->elements, sizeof(void*) * inList->size);
    *size = inList->size;

    return inList->elements;
}

void* listRemoveLastElem(void* in){
    listS* inList = (listS*) in;

    inList->size--;
    return inList->elements[inList->size];
}

void listAppend(void* in, void* element){
    listS* inList = (listS*) in;

    if(inList->size + 1 > inList->maxSize * LIST_RESIZE_PERCENT){
        inList->maxSize = inList->maxSize * LIST_EXPANDING_FACTOR;
        inList->elements = realloc(inList->elements, sizeof(void*) * inList->maxSize);
        //printf("Resizing to %u\n", inList->maxSize);
    }

    if (!inList->elements){
        printf("Ran out of memory to realloc! Array size was %d\n", inList->maxSize);
    }

    //printf("Before size: %d, Max size: %d\n", inList->size, inList->maxSize);
    inList->elements[inList->size++] = element;
}

void* getListElement(void* in, unsigned int index){
    listS* inList = (listS*) in;

    if (index < 0 || index >= inList->size){
        printf("Got invalid index(%d) for array access, max size %d\n", index, inList->size);
        return NULL;
    }

    return inList->elements[index];
}

void freeList(void* in){
    listS* inList = (listS*) in;

    free(inList->elements);
    free(inList);
}

void freeElements(void* in){
    listS* inList = (listS*) in;
    for(int i = 0; i < inList->size; i++){
        free(inList->elements[i]);
    }

    freeList(in);
}

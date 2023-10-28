extern void listAppend(void* in, void* element);
void* listRemoveLastElem(void* in);
extern void* createList(int maxSize);
extern void* getListElement(void* in, unsigned int index);
extern void freeList(void* in);
extern void freeElements(void* in);
extern unsigned int getListSize(void* in);
void** getListElements(void* in);
void** listToArray(void* in, unsigned int* size);

#include "da.h"
#include <stdlib.h>
#include <assert.h>

struct da{
    int size;
    int cap;
	 int flag;
    void** arr;
    void (*display)(void *, FILE *);
	 void (*free)(void *);
} da;

static void printAddress(void* a, FILE* fp);
static void expand(DA *items);
static void shift(DA *items, int index);
static void shiftBack(DA *items, int index);
static void checkSize(DA* items);
static void shrink(DA* items);

DA*   newDA(void){
   DA *fun = (DA*) malloc(sizeof(da));
	assert(fun != 0);
   fun->arr = malloc(sizeof(void*));
	assert(fun->arr != 0);
	fun->arr[0] = NULL;
   fun->size = 0;
   fun->cap = 1;
	fun->display = printAddress;
	fun->free = NULL;
	fun->flag = 0;
   return fun;
}

static void printAddress(void* a, FILE* fp){
   fprintf(fp, "@%p", a);
   return;
}

void  setDAdisplay(DA *items, void (*hi)(void *,FILE *)){
	 if(hi == 0){
		 items->display = printAddress;
		 return;
	 }
	 items->display = hi;
    return;
}

void  setDAfree(DA *items,void (*freeify)(void *)){
	items->free = freeify;
	return;
}

void insertDA(DA *items,int index,void *value){
	assert(index <= items->size && index >= 0);
	if(items->size == items->cap)
		expand(items);
	if(index < items->size)
		shift(items, index);
	items->arr[index] = value;
	assert(items != NULL);
	items->size++;
	return;
}

void *removeDA(DA *items,int index){
	assert(index >= 0 && index < items->size && items->size != 0);
	void* temp = items->arr[index];
	shiftBack(items, index);
	items->size--;
	checkSize(items);
	return temp;
}

void  unionDA(DA *recipient,DA *donor){
	int h = donor->size;
	 for(int i = 0; i < h; i++)
	 	insertDA(recipient, recipient->size, removeDA(donor, 0));
   return;
}

void *getDA(DA *items,int index){
	assert(index>=0 && index < items->size);
   return items->arr[index];
}

void *setDA(DA *items,int index,void *value){
	assert(index <= items->size && index >= 0);
	if(index != items->size){
		void* temp = items->arr[index];
		items->arr[index] = value;
		return temp;
	}
	insertDA(items, index, value);
   return NULL;
}

int sizeDA(DA *items){
    return items->size;
}

void  displayDA(DA *items,FILE *fp){
    int k = items->size;
    fprintf(fp, "[");
	 if(k == 0){
		 if(items->flag != 0 && items->size < items->cap){
			 fprintf(fp, "[%d]", items->cap-k);
		 }
		 fprintf(fp, "]");
		 return;
	 }
	 items->display(items->arr[0], fp);
    for(int i = 1; i < k; i++){
		 fprintf(fp,",");
       items->display(items->arr[i], fp);
    }
	 if(items->flag > 0){
		 fprintf(fp, ",[%d]", items->cap-k);
	 }
    fprintf(fp, "]");
	 //TESTING
	 //fprintf(fp, " %d", items->size);
}

int   debugDA(DA *items,int level){
	int a = items->flag;
	items->flag = level;
	return a;
}

void  freeDA(DA *items){
	if(items->size == 0 || items->free == NULL){
		free(items->arr);
		free(items);
		return;
	}
	for(int i = 0; i < items->size; i++){
		items->free(items->arr[i]);
	}
	free(items->arr);
	free(items);
   return;
}

static void expand(DA *items){
	//printf("\nexpanding\n");
	items->arr = realloc(items->arr, items->cap*2*sizeof(void*));
	assert(items->arr != 0);
	for(int i = items->cap; i < items->cap*2; i++)
		items->arr[i] = NULL;
	items->cap *= 2;
	return;
}

static void shift(DA *items, int index){
	if(index+1 < items->size)
		shift(items, index+1);
	items->arr[index+1] = items->arr[index];
	return;
}

static void shiftBack(DA *items, int index){
	//printf("start shift " );
	for(int i = index+1; i < items->size; i++){
		items->arr[i-1] = items->arr[i];
	}
	//printf("end shift\n");
	return;
}

static void checkSize(DA* items){
	if(items->size < (items->cap/4) && items->cap != 1){
		shrink(items);
		checkSize(items);
	}
	else if(items->size == 0 && items->cap != 1){
		shrink(items);
		checkSize(items);
	}
}

static void shrink(DA* items){
	items->arr = realloc(items->arr, items->cap/2*sizeof(void*));
	assert(items->arr != 0);
	items->cap/=2;
	return;
}

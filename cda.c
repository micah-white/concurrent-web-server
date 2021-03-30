#include "cda.h"
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

struct cda{
	int size;
	int cap;
	int start;
	int flag;
	void (*display)(void *,FILE *);
	void (*free)(void*);
	void** arr;
} cda;

static void printAddress(void* a, FILE* fp);
static void expand(CDA *items);
static void shift(CDA *items, int index);
static void shiftBack(CDA *items, int index);
static void shiftDown(CDA *items, int index);
static void shiftUp(CDA *items, int index);
static void checkSize(CDA* items);
static void shrink(CDA* items);
static void decrementStart(CDA* items);
static int isOccupied(CDA*items, int index);

CDA *newCDA(void){
	 CDA* fun = (CDA*) malloc(sizeof(cda));
	 assert(fun != 0);
	 fun->size = 0;
	 fun->cap = 1;
	 fun->start = 0;
	 fun->arr = malloc(sizeof(void*));
	 assert(fun->arr != 0);
	 fun->flag = 0;
	 fun->display = printAddress;
	 fun->free = NULL;
	 return fun;
}

void insertCDA(CDA *items,int index, void *value){
	assert(index <= items->size && index >= 0);
 	if(items->size == items->cap)
 		expand(items);
	//displayCDA(items, stdout); fprintf(stdout, "\n");
	if(index == 0)
		decrementStart(items);
	else if(index == items->size){}
	else if(index > items->size/2)
	 	shift(items, index);
	else
		shiftDown(items, index);
	assert(items != NULL);
	items->arr[(index+items->start)%items->cap] = value;
	items->size++;
	return;
}

void *removeCDA(CDA *items, int index){
	assert(index >= 0 && index < items->size && items->size != 0);
 	void* temp = items->arr[(index+items->start)%items->cap];
	if(index >= items->size/2)
			shiftBack(items, index);
	else
		shiftUp(items,index);
 	items->size--;
 	checkSize(items);
 	return temp;
}

void setCDAdisplay(CDA *items, void (*hi)(void *,FILE *)){
	 if(hi == 0){
		 items->display = printAddress;
		 return;
	 }
	 items->display = hi;
}

void setCDAfree(CDA *items, void (*freeify)(void *)){
	 items->free = freeify;
}

void unionCDA(CDA *recipient,CDA *donor){
	 int h = donor->size;
	 for(int i = 0; i < h; i++)
	 	insertCDAback(recipient, removeCDA(donor, 0));
	 return;
}

void *getCDA(CDA *items,int index){
	 assert(index>=0 && index < items->size);
    return items->arr[(index+items->start)%items->cap];
}

void *setCDA(CDA *items,int index,void *value){
	assert(index <= items->size && index >= -1);
	if(index == -1){
		insertCDAfront(items, value);
		return NULL;
	}
	if(index == items->size){
		insertCDAback(items, value);
		return NULL;
	}
	void* temp = items->arr[(index+items->start)%items->cap];
	items->arr[(index+items->start)%items->cap] = value;
	return temp;
}

int sizeCDA(CDA *items){
	 return items->size;
}

void displayCDA(CDA *items,FILE *fp){
	int k = items->size;
	fprintf(fp, "(");

	//size == 0
	if(k == 0){
	 	if(items->flag != 0 && items->size < items->cap){
			fprintf(fp, "(%d)", items->cap-k);
	 	}
		fprintf(fp, ")");
		return;
	}

	items->display(items->arr[items->start], fp);
	for(int i = 1; i < k; i++){
	 	fprintf(fp,",");
	 	items->display(items->arr[(items->start+i)%items->cap], fp);
	}
	if(items->flag > 0){
		fprintf(fp, ",(%d)", items->cap-k);
	}
	fprintf(fp, ")");
}

int debugCDA(CDA *items, int level){
	int a = items->flag;
 	items->flag = level;
 	return a;
 }

void freeCDA(CDA *items){
	 if(items->size == 0 || items->free == NULL){
 		free(items->arr);
 		free(items);
 		return;
 	}
 	for(int i = 0; i < items->size; i++){
 		items->free(items->arr[(i+items->start)%items->cap]);
 	}
 	free(items->arr);
 	free(items);
   return;
}

/*
	PRIVATE FUNCTIONS
*/

static void decrementStart(CDA* items){
	items->start--;
	if(items->start < 0)
	 	items->start += items->cap;
	return;
}

static void printAddress(void* a, FILE* fp){
     fprintf(fp, "@%p", a);
     return;
 }

static void expand(CDA *items){
	items->arr = realloc(items->arr, items->cap*2*sizeof(void*));
	assert(items->arr != 0);
	for(int i = items->cap; i < items->cap + items->start; i++)
		items->arr[i] = NULL;
	if(items->start + items->size >= items->cap){
		for(int i = items->cap + items->start; i < items->cap*2; i++)
			items->arr[i] = items->arr[i-items->cap];
		items->start += items->cap;
	}
	items->cap *= 2;
	return;
 }

//moves items from index up
static void shift(CDA *items, int index){
	if(index+1 < items->size)
		shift(items, index+1);
	items->arr[(items->start+index+1)%items->cap] = items->arr[(items->start+index)%items->cap];
 	return;
}

//moves items from index to end down 1
static void shiftBack(CDA *items, int index){
	for(int i = items->start + index; i < items->start +items->size-1; i++){
 		items->arr[(i)%items->cap] = items->arr[(i+1)%items->cap];
 	}
	return;
}

//moves items up to the index, increments start
static void shiftUp(CDA *items, int index){
	for(int i = items->start + index-1; i >= items->start; i--){
 		items->arr[(i+1)%items->cap] = items->arr[(i)%items->cap];
 	}
	items->start++;
	items->start%=items->cap;
	return;
}

//moves items back from index, decrements start
static void shiftDown(CDA *items, int index){
	decrementStart(items);
	for(int i = items->start; i < index+items->start; i++)
		items->arr[(i)%items->cap] = items->arr[(i+1)%items->cap];
	return;
}

static void checkSize(CDA* items){
	 if(items->size < (items->cap/4) && items->cap != 1){
 		shrink(items);
 		checkSize(items);
 	}
 	else if(items->size == 0 && items->cap != 1){
 		shrink(items);
 		checkSize(items);
 	}
}

static void shrink(CDA* items){
	if(items->start < items->cap/2 && items->start+items->size <= items->cap/2){
		items->arr = realloc(items->arr, (items->cap/2)*sizeof(void*));
		assert(items->arr != 0);
	 	items->cap/=2;
	 	return;
	}
	if(items->start < items->cap/2 && items->start+items->size > items->cap/2){
		int end = 0;
		for(int i = items->cap/2; i < items->start + items->size; i++){
			items->arr[end] = items->arr[i];
			end++;
		}
		items->arr = realloc(items->arr, (items->cap/2)*sizeof(void*));
		assert(items->arr != 0);
	 	items->cap/=2;
		return;
	}
	if(isOccupied(items,0)){
		for(int i = items->start; i < items->cap; i++){
			int l = items->cap - i;
			items->arr[items->cap/2-l] = items->arr[i];
		}
		items->start = items->cap/2 - (items->cap - items->start);
		items->arr = realloc(items->arr, (items->cap/2)*sizeof(void*));
		assert(items->arr != 0);
	 	items->cap/=2;
		return;
	}
	if(!isOccupied(items,0)){
		int end = 0;
		for(int i = items->start; i < items->start + items->size; i++){
			items->arr[end] = items->arr[i];
			end++;
		}
		items->arr = realloc(items->arr, (items->cap/2)*sizeof(void*));
		assert(items->arr != 0);
	 	items->cap/=2;
		items->start = 0;
		return;
	}
 	return;
}

//returns if a raw index is occupied
static int isOccupied(CDA* items, int index){
	if(items->start <= index && items->start+items->size > index){
		return 1;
	}
	else if(items->start > index && (((items->start+items->size)%items->cap)>index) && items->start+items->size > items->cap){
		return 1;
	}
	return 0;
}

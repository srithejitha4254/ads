#include <stdio.h> 
#include <stdlib.h>  
void swap(int *a, int *b) { 
    int temp = *a; 
    *a = *b; 
    *b = temp; 
} 
typedef struct { 
    int *array;  
    int capacity;  
    int size;
    int is_min_heap; 
} Heap; 
Heap *createHeap(int capacity, int is_min_heap)  
{ 
    Heap *heap = (Heap *)malloc(sizeof(Heap)); 
    if (heap == NULL)  
   {                                                                                          
        perror("Memory allocation failed"); 
        exit(EXIT_FAILURE); 
    } 
    heap->capacity = capacity; 
    heap->size = 0; 
    heap->is_min_heap = is_min_heap; 
    heap->array = (int *)malloc(capacity * sizeof(int)); 
    if (heap->array == NULL) { 
        perror("Memory allocation failed"); 
        exit(EXIT_FAILURE); 
    } 
    return heap;
}  
void heapify(Heap *heap, int i)  
{ 
    int smallest_or_largest = i; 
    int left = 2 * i + 1; 
    int right = 2 * i + 2; 
    if (heap->is_min_heap) 
   { 
        if (left < heap->size && heap->array[left] < heap->array[smallest_or_largest])  
       { 
            smallest_or_largest = left; 
        } 
        if (right < heap->size && heap->array[right] < heap->array[smallest_or_largest])  
       { 
            smallest_or_largest = right; 
        } 
    } else  
       { 
        if (left < heap->size && heap->array[left] > heap->array[smallest_or_largest])  
       { 
            smallest_or_largest = left; 
        } 
        if (right < heap->size && heap->array[right] > heap->array[smallest_or_largest]) 
       { 
            smallest_or_largest = right; 
        } 
    } 
    if (smallest_or_largest != i)  
   { 
       swap(&heap->array[i], &heap->array[smallest_or_largest]); 
        heapify(heap, smallest_or_largest); 
    } 
}  
void deleteElement(Heap *heap, int key)  
{ 
    int i; 
    for (i = 0; i < heap->size; i++)  
   { 
        if (heap->array[i] == key) 
            break; 
    } 
    if (i == heap->size)  
    { 
        printf("Element %d not found in the heap.\n", key); 
        return; 
    }  
    heap->array[i] = heap->array[heap->size - 1]; 
     heap->size--; 
    heapify(heap, i); 
}  
void displayHeap(Heap *heap)  
{ 
    printf("Heap elements: "); 
    int i; 
    for (i= 0; i < heap->size; i++)  
   { 
        printf("%d ", heap->array[i]); 
    } 
    printf("\n"); 
}  
void freeHeap(Heap *heap)  
{ 
     free(heap->array); 
    free(heap); 
} 
int main()  
{  
    Heap *minHeap = createHeap(20, 1);
    Heap *maxHeap = createHeap(20, 0); 
    int minHeapElements[] = {3, 2, 1, 7, 8, 4, 10, 16},i; 
    int numElements = sizeof(minHeapElements) / sizeof(minHeapElements[0]); 
    for ( i = 0; i < numElements; i++) 
   { 
        minHeap->array[minHeap->size++] = minHeapElements[i]; 
    }
    for ( i = (minHeap->size - 2) / 2; i >= 0; i--)  
    { 
        heapify(minHeap, i); 
    }  
    int maxHeapElements[] = {10, 8, 16, 14, 7, 9, 3, 2}; 
    numElements = sizeof(maxHeapElements) / sizeof(maxHeapElements[0]); 
    for (i = 0; i < numElements; i++) 
   { 
        maxHeap->array[maxHeap->size++] = maxHeapElements[i];                                                         
    }  
    for (i = (maxHeap->size - 2) / 2; i >= 0; i--)  
   { 
        heapify(maxHeap, i); 
    }  
    printf("Min Heap:\n"); 
    displayHeap(minHeap); 
    printf("Max Heap:\n"); 
    displayHeap(maxHeap);  
    deleteElement(minHeap, 4);  
    printf("After deleting 4 from Min Heap:\n"); 
    displayHeap(minHeap); 
    deleteElement(maxHeap, 16); 
    printf("After deleting 16 from Max Heap:\n"); 
    displayHeap(maxHeap); 
    freeHeap(minHeap); 
    freeHeap(maxHeap); 
   return 0; 
} 

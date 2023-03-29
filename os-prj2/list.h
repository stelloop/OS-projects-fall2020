#ifndef LIST_H
#define LIST_H


// Generic implementation using void pointers.

// < 0  iff a < b
//   0  iff a and b are equivalent 
// > 0  iff a > b
typedef int (*CompareFunc)(void* a, void* b);

typedef void (*DestroyFunc)(void* value);

typedef struct list* List;
typedef struct list_node* ListNode;



List list_create(DestroyFunc destroy_value);

int list_size(List list);

// If node == NULL, inserts node at the start of the list.
void list_insert_next(List list, ListNode node, void* value);

void list_remove(List list, ListNode node); 

// If node == NULL, removes the first node of the list.
void list_remove_next(List list, ListNode node);

// Returns the first occurence of the value, else, if value isn't found, returns NULL.
void* list_find(List list, void* value, CompareFunc compare);

void list_destroy(List list);

// list traversal 

// If the list is empty, returns NULL.
ListNode list_first(List list);
ListNode list_last(List list);



// If node is the last node of the list, returns NULL.
ListNode list_next(List list, ListNode node);

void* list_node_value(ListNode node);


ListNode list_find_node(List list, void* value, CompareFunc compare);



#endif
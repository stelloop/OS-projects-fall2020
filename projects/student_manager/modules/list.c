

#include <stdlib.h>
#include <assert.h>
#include "../includes/list.h"

struct list {
	ListNode first;				
	ListNode last;				
	int size;					
	DestroyFunc destroy_value;	
};

struct list_node {
	ListNode next;		
	void* value;
};


List list_create(DestroyFunc destroy_value) 
{
	List list = malloc(sizeof(*list));
	list->size = 0;
	list->destroy_value = destroy_value;
	list->first = NULL;
	list->last = NULL;

	return list;
}

int list_size(List list) {
	return list->size;
}

void list_insert_next(List list, ListNode node, void* value) 
{
	ListNode new = malloc(sizeof(*new));
	new->value = value;
	if(node == NULL) // insert at start
	{
		if(list->first == NULL) // empty list
		{
			new->next = NULL; 
			list->last = new;
		}
		else  // non empty
			new->next = list->first;

		list->first = new;
	}
	else 
	{
		new->next = node->next;
		node->next = new;
	}
	list->size++;
	if(list->last == node)
		list->last = new;
}


void list_destroy(List list) 
{
	ListNode node = list->first;
	while (node != NULL) 
    {				
		ListNode next = node->next;		
		if (list->destroy_value != NULL)
			list->destroy_value(node->value);

		free(node);
		node = next;
	}
	free(list);
}

void list_remove_next(List list, ListNode node) 
{
	ListNode to_remove = NULL;
	if(node == NULL) // we want to remove the first node
	{
		assert(list->first != NULL); // we can't remove from an empty list
		to_remove = list->first;
		list->first = to_remove->next;
	}
	else
	{
		to_remove = node->next;
		assert(to_remove != NULL);		
		node->next = to_remove->next;		
	}
	if (list->destroy_value != NULL)
		list->destroy_value(to_remove->value);

	free(to_remove);
	// update size & last
	list->size--;
	if (list->last == to_remove)
		list->last = node;
}

ListNode list_first(List list) {
	return list->first;
}

ListNode list_last(List list) {
	return list->last;
}

ListNode list_next(List list, ListNode node) {
	return node->next;
}

void* list_node_value(ListNode node) {
	assert(node != NULL);
	return node->value;
}

void* list_find(List list, void* value, CompareFunc compare) {
	ListNode node = list_find_node(list, value, compare);
	return node == NULL ? NULL : node->value;
}

ListNode list_find_node(List list, void* value, CompareFunc compare) 
{
	for (ListNode node = list->first; node != NULL; node = node->next)
		if (compare(value, node->value) == 0) return node;		// found
	return NULL;	// not found
}


void list_remove(List list, ListNode node)
{
	ListNode to_remove = NULL;
	ListNode prev = NULL;
	if(node == list->first)  // the first node gets deleted
	{
		assert(list->first != NULL); // we can't remove from an empty list
		to_remove = list->first;
		list->first = to_remove->next;
	}
	else
	{
		
		for(prev = list->first; prev != NULL; prev = prev->next)
		{
			if(prev->next == node)
			{
				to_remove = node; 
				assert(to_remove != NULL);
				prev->next = node->next;
				break;
			}
		}
		
		
	}
	
	// update size & last
	list->size--;
	if(list->last == node)
		list->last = prev;
	
	
	if (list->destroy_value != NULL)
		list->destroy_value(to_remove->value);

	free(to_remove);
}
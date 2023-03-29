// a generic hashtable, using seperate chaining

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include "../includes/hashtable.h"
#include "../includes/list.h"

#define INITIAL_SIZE 50

struct ht_node {
	void* key;		// key used for hashing
	void* value;  	// the value associated with the key
};

// Δομή του Map (περιέχει όλες τις πληροφορίες που χρεαζόμαστε για το HashTable)
struct hashtable {
	List* array;				// the array of lists (buckets)
	int buckets;				// num of buckets
	int size;					// num o f entries
	CompareFunc compare;		// compare function, given from the user
	HashFunc hash_function;		
	DestroyFunc destroy_key;	
	DestroyFunc destroy_value;
};

HashTable ht_create(CompareFunc compare, DestroyFunc destroy_key, DestroyFunc destroy_value, int initial_size) 
{
	HashTable ht = malloc(sizeof(*ht));
    if(initial_size == 0) 
        ht->buckets = INITIAL_SIZE; //define'd
    else 
        ht->buckets = initial_size; //given from the user of the module
	ht->array = malloc(ht->buckets * sizeof(List));

	for(int i = 0; i < ht->buckets; i++)
		ht->array[i] = list_create(NULL); // initialize buckets with empty lists

	ht->size = 0;
	ht->compare = compare;
	ht->destroy_key = destroy_key;
	ht->destroy_value = destroy_value;

	return ht;
}



bool ht_insert(HashTable ht, void* key, void* value) 
{
	unsigned int hashcode = ht->hash_function(key) % ht->buckets;
	// lookup key, if found, don't insert it and return false
	void* found = ht_find(ht, key);
	if(found != NULL) 
		return false;
	else 
	{
		HTNode new = malloc(sizeof(*new));
		new->key = key;
		new->value = value;
		list_insert_next(ht->array[hashcode], NULL, new);
		ht->size++;
		return true; // successful insertion 
	}
}


bool ht_remove(HashTable ht, void* key)
{
	unsigned int hashcode = ht->hash_function(key) % ht->buckets;
	ListNode prev = NULL;
	for(ListNode node = list_first(ht->array[hashcode]);
		node != NULL; 
		node = list_next(ht->array[hashcode], node))
	{
		HTNode ht_node = list_node_value(node);
		if(ht->compare(ht_node->key, key) == 0) // found
		{

			if (ht->destroy_key != NULL)
				ht->destroy_key(ht_node->key);

			if (ht->destroy_value != NULL)
				ht->destroy_value(ht_node->value);
			free(ht_node);
			list_remove_next(ht->array[hashcode], prev);
			return true;
		}
		prev = node; // continue searching
	}


	return false; // element doesn't exist
}


void ht_destroy(HashTable ht)
{
	// destroy all the buckets and their contents
	for(int i = 0; i < ht->buckets; i++)
	{
		for(ListNode node = list_first(ht->array[i]);
			node != NULL;
			node = list_next(ht->array[i], node))
		{
			HTNode ht_node = list_node_value(node);
			if (ht->destroy_key != NULL)
				ht->destroy_key(ht_node->key);
			if (ht->destroy_value != NULL)
				ht->destroy_value(ht_node->value);
			free(ht_node);
		}
		list_destroy(ht->array[i]); 
	}
	free(ht->array); 
	free(ht);
}



void* ht_find(HashTable ht, void* key)
{
	uint hashcode = ht->hash_function(key) % ht->buckets;
	for(ListNode node = list_first(ht->array[hashcode]);
		node != NULL; 
		node = list_next(ht->array[hashcode], node))
	{
		HTNode ht_node = list_node_value(node);
		if(ht->compare(ht_node->key, key) == 0)
			return ht_node->value; // found, return value
	}
	return NULL; // not found
}



// return num of entries
int ht_size(HashTable ht) {
	return ht->size;
}

void ht_set_hash_function(HashTable ht, HashFunc func) {
	ht->hash_function = func;
}

unsigned int hash_int(void* value) {
	return *(int*)value;
}


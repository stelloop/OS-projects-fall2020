#ifndef HASHTABLE_H
#define HASHTABLE_h

#include <stdbool.h>

// Generic implementation using void pointers.

// < 0  iff a < b
//   0  iff a and b are equivalent 
// > 0  iff a > b
typedef int (*CompareFunc)(void* a, void* b);

typedef void (*DestroyFunc)(void* value);

typedef struct hashtable* HashTable;

HashTable ht_create(CompareFunc compare, DestroyFunc destroy_key, DestroyFunc destroy_value, int initial_size);

int ht_size(HashTable ht);

int ht_buckets(HashTable ht);

// returns true if insertion was sucessful, false if the key already exists
bool ht_insert(HashTable ht, void* key, void* value);

// returns true if removal was successful. else false if the key was not found
bool ht_remove(HashTable ht, void* key);

void* ht_find(HashTable ht, void* key);

// frees all the allocated memory from the HashTable ht.
void ht_destroy(HashTable ht);


typedef struct ht_node* HTNode;

HTNode ht_first(HashTable ht);

HTNode ht_next(HashTable ht, HTNode node);

void* ht_node_key(HashTable ht, HTNode node);

void* ht_node_value(HashTable ht, HTNode node);


typedef unsigned int (*HashFunc)(void*);



unsigned int hash_int(void* value);			

// sets the hash function for the HashTable ht.
// must be called immidiately after ht_create().
void ht_set_hash_function(HashTable ht, HashFunc hash_func);

#endif
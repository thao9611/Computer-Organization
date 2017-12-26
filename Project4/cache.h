#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

/* cache.h : Declare functions and data necessary for your project*/

int miss_penalty; // number of cycles to stall when a cache miss occurs
uint32_t ***Cache; // data cache storing data [set][way][byte]

uint32_t **tags;
int **dbit;// dirty bit
//int **vbit; // valid bit
int **lrubit;
int lru;
//int num_way;

int cache_stall;


void setupCache(int, int, int);
void setCacheMissPenalty(int);

uint32_t cache_read_32(uint32_t address);
void cache_write_32(uint32_t address, uint32_t value);
uint32_t* find_cache(uint32_t address);//match the address with the empty cache block
int find_block(uint32_t address);
uint32_t cache_read_miss(uint32_t address);
void cache_write_miss(uint32_t address, uint32_t value);
int find_way( int index);





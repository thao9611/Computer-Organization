#include "cache.h"
#include "util.h"

/* cache.c : Implement your functions declared in cache.h */





/***************************************************************/
/*                                                             */
/* Procedure: setupCache                  		       */
/*                                                             */
/* Purpose: Allocates memory for your cache                    */
/*                                                             */
/***************************************************************/
void setupCache(int capacity, int num_way, int block_size)
{
/*	code for initializing and setting up your cache	*/
/*	You may add additional code if you need to	*/
	
	int i,j; //counter
	int nset=0; // number of sets
	int _wpb=0; //words per block   
	nset=capacity/(block_size*num_way); 
	_wpb = block_size/BYTES_PER_WORD;
	lru = 0;
	
	cache_stall = 0;
	
	Cache = (uint32_t  ***)malloc(nset*sizeof(uint32_t **)); // point to the whole, each part associate with one set

	dbit = (int **)malloc(nset*sizeof(int*));
	lrubit = (int **)malloc(nset *sizeof(int*));
	tags = (uint32_t **)malloc(nset *sizeof(uint32_t*));

	for (i=0;i<nset;i++) {
		Cache[i] = (uint32_t ** )malloc(num_way*sizeof(uint32_t*));

		dbit[i] = (int *)malloc(num_way*sizeof(int));
		lrubit[i] = (int *)malloc(num_way*sizeof(int));
		tags[i] = (uint32_t *)malloc(num_way*sizeof(uint32_t));
		
		 // point to one cache, each associate with one way
	}
	for (i=0; i<nset; i++){	// refer to set
		
		for (j=0; j<num_way; j++){ // refer to way 
			dbit[i][j] = 0;
			lrubit[i][j]= 0;
			tags[i][j] = 0;			//(uint32_t*)malloc(sizeof(uint32_t));

			Cache[i][j]=(uint32_t*)malloc(sizeof(uint32_t)*(_wpb));// size of one block
		}
	}

}


/***************************************************************/
/*                                                             */
/* Procedure: setCacheMissPenalty                  	       */
/*                                                             */
/* Purpose: Sets how many cycles your pipline will stall       */
/*                                                             */
/***************************************************************/

void setCacheMissPenalty(int penalty_cycles)
{
/*	code for setting up miss penaly			*/
/*	You may add additional code if you need to	*/	
	miss_penalty = penalty_cycles;

}

/* Please declare and implement additional functions for your cache */

/*find cache match for data, return 0 if miss,otherwise return the cache block */
uint32_t* find_cache(uint32_t address){
	int i;
	int set = (address>>3) & 0x1;
	//int valid = 0;

	uint32_t tag = (address >>4) & 0xfffffff;

	for (i= 0; i<4; i++){
		if(tags[set][i] == tag){// hit
			//valid = 1;
			lru++;
			lrubit[set][i] = lru;
			return Cache[set][i];
		}
	}
	return 0;
}

int find_block(uint32_t address){
	int i;
	int set = (address>>3) & 0x1;
	uint32_t tag = (address >>4) & 0xfffffff;

	for (i= 0; i<4; i++){
		if(tags[set][i] == tag){// hit
			//valid = 1;
			lru++;
			lrubit[set][i] = lru;
			return i;
		}
	}
	return -1;
}


/* read data from cache*/
uint32_t cache_read_32(uint32_t address){
	uint32_t* block = find_cache(address);

	if (block==0){
		uint32_t data = cache_read_miss(address);
		return data;
	}else{
		int offset =(address>>2) % 2;
		return block[offset];
	}
	
	
}



/* write data to cache */
void cache_write_32( uint32_t address, uint32_t value){
	uint32_t* block = find_cache(address);


	if (block == 0){
		cache_write_miss(address, value);
		return;
	}else{
		//mem_read_block(address, cache);
		int set = (address>>3) & 0x1;
		int way = find_block(address);
		dbit[set][way] = 1;

		int offset =(address>>2) % 2;
		block[offset] = value;
		return;
	}
}

/* handle read miss: stall + read from mem to cache + return that data

*/
uint32_t cache_read_miss(uint32_t address){
	cache_stall = 30;
	int set = (address>>3) & 0x1;
	int way = find_way(set);
	int offset = (address>>2) % 2;

	//Check dirty bit
	if (dbit[set][way]==1){
		uint32_t old_address = (tags[set][way]<<4) + (set<<3);
		mem_write_block(old_address, Cache[set][way]);
		dbit[set][way]=0;
	}

	mem_read_block(address, Cache[set][way]);
	lru ++;
	lrubit[set][way] = lru;

	uint32_t tag =  (address >>4) & 0xfffffff;
	tags[set][way] = tag; // reassign tag 

	return Cache[set][way][offset];
}

void cache_write_miss(uint32_t address, uint32_t value){
	cache_stall = 30;
	int set = (address>>3) & 0x1;
	int way = find_way(set);
	int offset = (address>>2) % 2;

	lru++;
	lrubit[set][way] = lru;

	if (dbit[set][way] == 1) {
		// dirty bit , write back, evict the block out of cache
		uint32_t old_tag = tags[set][way];
		uint32_t old_address = (old_tag<<4) + (set<<3);

		mem_write_block(old_address, Cache[set][way]);
	}

	mem_read_block(address, Cache[set][way]);// fetch the data from memory to the cache first

	uint32_t tag = (address>>4)&0xfffffff;
	tags[set][way] = tag;

	Cache[set][way][offset] = value;// enough????
	dbit[set][way] =1;
	return;
}

// find the way based on lRU policy
int find_way(int set){
	int i;
	int way = 0;
	int lru_min = lrubit[set][way];
	for (i = 1; i<4; i++){
		if (lrubit[set][i] < lru_min){
			lru_min = lrubit[set][i];
			way = i;
		}
	}
	return way;
}

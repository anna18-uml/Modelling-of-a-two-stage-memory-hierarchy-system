#include <iostream>
#include "memory.h"
using namespace std;
#define _CRT_SECURE_NO_WARNINGS

unsigned int clockX;
unsigned int numMisses;


int Block::isEmpty()
{
	if (Block::valid == 0)
		return 0;
	else
		return 1;
}

Block::Block()
{

	Block::last_used = 0;
	Block::valid = 0;
}

MainMem::MainMem()
{
	int i, j;
	for (i = 0; i < BLOCKS_IN_MEMORY; i++)
	{
		for (j = 0; j < 4; j++)
		{
			blocks[i].data[j] = 0;
			blocks[i].words[j] = 0;
		}
	}
}


int MainMem::getData(int x)
{
	int blk_addr = x / WORDS_PER_BLOCK;                // block address
	int word_offset = x % WORDS_PER_BLOCK;             // word offset
	clockX = clockX + 100;                             
	return(blocks[blk_addr].data[word_offset]);
}

void MainMem::putData(int x, int y)
{
	int blk_addr = x / WORDS_PER_BLOCK;                // block address
	int word_offset = x % WORDS_PER_BLOCK;
	clockX = clockX + 100;
	blocks[blk_addr].data[word_offset] = y;
}

Cache::Cache()                                         // initialize with 0
{
	int i, j, k;
	for (i = 0; i < (BLOCKS_IN_CACHE / NUM_OF_SET); i++)
	{
		for (j = 0; j < NUM_OF_SET; j++)
		{
			for (k = 0; k < WORDS_PER_BLOCK; k++)
			{
				cblocks[i][j].data[k] = 0;
				cblocks[i][j].words[k] = 0;
			}
		}
	}
}


int Cache::getData(int x)
{
	int hit=0;
	int blk_addr = x / WORDS_PER_BLOCK;                            // block address
	int cache_addr = blk_addr % (BLOCKS_IN_CACHE/NUM_OF_SET);      // cache index
	int word_offset = x % WORDS_PER_BLOCK;                         // word offset
	int tag = x*NUM_OF_SET/ (BLOCKS_IN_CACHE*WORDS_PER_BLOCK);     // tag (for directed mapped, NUM_OF_SET=1; for fully associative, NUM_OF_SET = BLOCKS_IN_CACHE
	total_cache_accesses++;

	clockX += 2;  // for tag and valid extraction
	for (int i = 0; i < NUM_OF_SET; i++)
	{
		if (cblocks[cache_addr][i].valid == 1 && cblocks[cache_addr][i].words[0] == tag)
		{
			hit = 1;
			clockX += 2;     // for accessing data and sending it
			Cache::setBlockUsage(cache_addr,i);
			return(cblocks[cache_addr][i].data[word_offset]);
		
		}
	}
	if(hit==0)
	{
		return(Cache::resolveMiss(x, 0));     // calculate the miss cycles in resolve function
	}
}

void Cache::putData(int x, int z)
{
	int i = 0;
	int hit = 0;
	int blk_addr = x / WORDS_PER_BLOCK;                             
	int cache_addr = blk_addr %(BLOCKS_IN_CACHE/NUM_OF_SET);       // cache index
	int word_offset = x % WORDS_PER_BLOCK;                           
	int tag = x *NUM_OF_SET/ (BLOCKS_IN_CACHE*WORDS_PER_BLOCK);    // tag
	
	total_cache_accesses++;
	clockX += 2;   // for accessing valid and tag
	for (i = 0; i < NUM_OF_SET; i++)
	{
		if ((cblocks[cache_addr][i].valid == 1) && cblocks[cache_addr][i].words[0] == tag)
		{
			hit = 1;
			clockX += 2;			// for writing into the cache block
			cblocks[cache_addr][i].data[word_offset] = z;  // ASSUMPTION: don't have to do read-modify-write
			cblocks[cache_addr][i].valid = 1;
			Cache::setBlockUsage(cache_addr, i);
			MainMemory.putData(x, z);
			break;
		}
	}
	if(hit==0)
	{	
		MainMemory.putData(x, z);
		Cache::resolveMiss(x, 1);
	}
}

int Cache::resolveMiss(int x, int resolve_while_put = 0)
{
	int blk_addr = x / WORDS_PER_BLOCK;
	int cache_addr = blk_addr % (BLOCKS_IN_CACHE/NUM_OF_SET);      // cache_index
	int word_offset = x % WORDS_PER_BLOCK;
	int tag = x*NUM_OF_SET/ (BLOCKS_IN_CACHE*WORDS_PER_BLOCK);     // tag = addr*[BLOCKS_IN_CACHE/NUM_OF_SET]*[1/WORDS_PER_BLOCK]
	int block_aligned_addr = blk_addr*WORDS_PER_BLOCK;             // block alligned address = {block_address,2'b00}
	int set_number = 0;
	int set_count = 0;
	int least_used_set = 0;
	int blk_addr_to_be_evicted;
	int evict_one_block = 0;

	// Index into cache line and search for empty blocks if set associative. 
	// If there are no empty blocks, search for least recently used block
	// for a given index (search through it's sets) and evict the least used block
	if (NUM_OF_SET > 1)
	{
		least_used_set = cblocks[cache_addr][set_count].last_used;
		for (set_count = 0; set_count < NUM_OF_SET; set_count++)
		{
			if (cblocks[cache_addr][set_count].valid == 0)    // search for empty blocks; break if found         
			{
				evict_one_block = 0;
				set_number = set_count;
				break;
			}
			if (cblocks[cache_addr][set_count].valid == 1)    // if no empty blocks, search for least used block to evict
			{
				evict_one_block = 1;
				if (least_used_set > cblocks[cache_addr][set_count].last_used)
				{
					least_used_set = cblocks[cache_addr][set_count].last_used;
					set_number = set_count;
				}
			}
		}
	}
	// ASSUMPTION: The evict block is always "dirty" - so always writing back to memory here.
	if (evict_one_block == 1)
	{
		blk_addr_to_be_evicted = cblocks[cache_addr][set_number].words[0]*(BLOCKS_IN_CACHE / NUM_OF_SET)*WORDS_PER_BLOCK   // TAG
								+ (cache_addr*WORDS_PER_BLOCK);
		MainMemory.putData(blk_addr_to_be_evicted, cblocks[cache_addr][set_number].data[0]);
		MainMemory.putData(blk_addr_to_be_evicted+1, cblocks[cache_addr][set_number].data[1]);
		MainMemory.putData(blk_addr_to_be_evicted+2, cblocks[cache_addr][set_number].data[2]);
		MainMemory.putData(blk_addr_to_be_evicted+3, cblocks[cache_addr][set_number].data[3]);
	}

	clockX = clockX + 100;
	// use the evicted place (set_number) to replace it with new cache block
	// for direct mapped, set_number will be always 0 whether evicted or not
	cblocks[cache_addr][set_number].data[0] = MainMemory.getData(block_aligned_addr);
	cblocks[cache_addr][set_number].data[1] = MainMemory.getData(block_aligned_addr + 1);
	cblocks[cache_addr][set_number].data[2] = MainMemory.getData(block_aligned_addr + 2);
	cblocks[cache_addr][set_number].data[3] = MainMemory.getData(block_aligned_addr + 3);

	cblocks[cache_addr][set_number].words[0] = tag;
	cblocks[cache_addr][set_number].words[1] = tag;
	cblocks[cache_addr][set_number].words[2] = tag;
	cblocks[cache_addr][set_number].words[3] = tag;
	cblocks[cache_addr][set_number].valid = 1;
	clockX += 2 * 4;         // since each word is getting extracted from memory, we need to access cache also each time a new word arrives from memory

	Cache::setBlockUsage(cache_addr, set_number);

	numMisses++;
	if (resolve_while_put == 0)  clockX += 2;         // to access the word finally	while getting the data; for put, you should not incur this clocks since we don't read the data after putting.
	return(cblocks[cache_addr][set_number].data[word_offset]);
}

int Cache::getOldestBlock()
{
	int i,j;
	int m, n;
	m = cblocks[0][0].last_used;
	n = 0;
	for (i = 0; i < BLOCKS_IN_CACHE / NUM_OF_SET; i++)
	{
		for (j = 0; j < NUM_OF_SET; j++)
		{
			if (cblocks[i][j].last_used < m)
			{
				m = cblocks[i][j].last_used;
				n = i;
			}
		}
	}
	return(n);
}

int Cache::showCacheBlock(int x, int y, int z)
{
	if (cblocks[x][y].valid == 1)
	{
		return((cblocks[x][y].words[0] * BLOCKS_IN_CACHE*WORDS_PER_BLOCK/NUM_OF_SET) +    //TAG
			(x*WORDS_PER_BLOCK) +                                           //INDEX
			z);    	//WORD         
	}
	else
		return 0;


}

void Cache::setBlockUsage(int x, int set)
{
	cblocks[x][set].last_used++;
}

Memory::Memory()
{
	clockX = 0;
	numMisses = 0;
//	int i;
	//cout << "Enter your choice 1.direct mapping 2.2 way asscociativity 3.full associativity" << endl
	//	cin >> &i;
//	if (i == 1)
		//NUM_OF_SET = 1;
	//else if (i == 2)
		//NUM_OF_SET = 2;
	//else if (i == 3)
		//NUM_OF_SET = 8;

}

int Memory::getData(int address)
{
	return(myCache.getData(address));
	
}

void Memory::putData(int address, int value)
{
	myCache.putData(address, value);
	
}

void Memory::resetClock()
{
	clockX = 0;
}

unsigned int Memory::getClock()
{
	return(clockX);
}

void Memory::setClock(unsigned int newtime)
{
	clockX = newtime;
}

unsigned int Memory::getNumCacheMisses()
{
	return(numMisses);
}

void Memory::setNumCacheMisses(unsigned int newval)
{
	numMisses = newval;
}

void Memory::showCacheAddress()
{
	int i, j, k, p;
	for (i = 0; i < (BLOCKS_IN_CACHE/NUM_OF_SET); i++)
	{
		for (j = 0; j < NUM_OF_SET; j++)
		{
			for (k = 0; k < WORDS_PER_BLOCK; k++)
			{
				cout << myCache.showCacheBlock(i, j, k) << " ";
			}
			cout << endl;
		}
	}
}


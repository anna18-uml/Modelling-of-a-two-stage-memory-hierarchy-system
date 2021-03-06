#pragma once
#ifndef MEMORY_H_
#define MEMORY_H_

/*
This file contains class interfaces, simple constructors,
and simple destructors.  It also defines some global constants
for use in the classes and main program.
*/

//================
//==  Global vars
//================
extern unsigned int clockX;
extern unsigned int numMisses;

//=================
//==  Constants
//=================
#define WORDS_PER_BLOCK 4
#define BLOCKS_IN_MEMORY 512
#define BLOCKS_IN_CACHE 8
#define NUM_OF_SET 8
//-------------------------------------


//=================
//==  Block
//=================
class Block {
public:
	Block();//constructor
	~Block() { }//destructor
	int isEmpty();

	int words[WORDS_PER_BLOCK];
	int data[WORDS_PER_BLOCK];
	int last_used;
	int valid;
};//class block
  //-------------------------------------


  //=================
  //==  MainMem
  //=================
class MainMem {
private:
	Block blocks[BLOCKS_IN_MEMORY];
public:
	MainMem(); //constructor
	~MainMem() { }  //destructor
	int getData(int);
	void putData(int, int);
	// int resolveFault(int);
};//class MainMem
  //-------------------------------------


  //=================
  //==  Cache
  //=================
class Cache {
private:
	Block cblocks[BLOCKS_IN_CACHE/NUM_OF_SET][NUM_OF_SET];
	MainMem MainMemory;

	// int billsSuperFunk(int);

public:
	Cache(); //constructor
	~Cache() { }  //destructor
	int getData(int);
	int total_cache_accesses = 0;
	void putData(int, int);
	int resolveMiss(int, int);
	int getOldestBlock();
	// int getOldestBlock(int);
	void setBlockUsage(int, int);
	int showCacheBlock(int, int, int);
};//class Cache
  //-------------------------------------


class Memory
{
private:
	Cache myCache;

public:

	Memory();                                   // constructor

	int  getData(int address);                // load
	void putData(int address, int value);     // store

	void resetClock();                          // set timer to zero
	unsigned int  getClock();                   // get current timer value
	void setClock(unsigned int newtime);      // set new timer value

	unsigned int  getNumCacheMisses();               // get number of cache misses
	void setNumCacheMisses(unsigned int newval);   // record number of cache misses
	void showCacheAddress();								// show the cache contents
};

#endif



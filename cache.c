#include <stdio.h>
#include <math.h>

#define LOAD 0
#define STORE 1

//we'll assume a constant miss penalty of 35
#define miss_penalty 35

// You can change the parameters here.  Your program should work for any 
//     reasonable values of CACHESIZE, BLOCKSIZE, or associativity.

#define CACHESIZE 1024*256
#define BLOCKSIZE 64
#define associativity 4 
#define SETS CACHESIZE / (BLOCKSIZE * associativity)

#define ADDRESSBITS 32

// a good place to declare your storage for tags, etc.  Obviously,
//   you don't need to actually store the data.


unsigned int indices[SETS];
unsigned int    tags[SETS][associativity];
unsigned int offsets[SETS][associativity];

int           valids[SETS][associativity];
unsigned int    LRUs[SETS][associativity];

unsigned int tagBits = 0;
unsigned int indexBits = 0;
unsigned int offsetBits = 0;

long hits=0, misses=0, readhits=0, readmisses=0;

main()
{
	long address, references;
	int loadstore, icount;
	char marker;
	long cycles = 0;
	long basecycles = 0;
	
	int i,j;
	
	//a good place to initialize your structures.
	
	int distinctOffsets = 1;
	while (distinctOffsets < BLOCKSIZE)
	{
		distinctOffsets *= 2;
		offsetBits++;
	}

	int distinctIndices = 1;
	while (distinctIndices < CACHESIZE / (BLOCKSIZE * associativity))
	{
		distinctIndices *= 2;
		indexBits++;
	}

	tagBits = ADDRESSBITS - indexBits - offsetBits;

	printf("%d bit address had been partitioned into:\n", ADDRESSBITS);
	printf("  %d\t tag         bits\n", tagBits);
	printf("  %d\t index       bits\n", indexBits);
	printf("  %d\t byte-offset bits\n", offsetBits);

	for (i = 0; i < CACHESIZE / (BLOCKSIZE * associativity); ++i)
	{
		for (j = 0; j < associativity; ++j)
		{
			valids[i][j] = 0; // set all cache lines to invalid (cold start)
		}
	}

	printf("Cache parameters:\n");
	printf("\tCache size %d\n", CACHESIZE);
	printf("\tCache block size %d\n", BLOCKSIZE);
	printf("\tCache associativity %d\n", associativity);
	
	// the format of the trace is
	//    # loadstore address instcount
	//    where loadstore is 0 (load) or 1 (store)
	//          address is the address of the memory access
	//          instcount is the number of instructions (including the load
	//            or store) between the previous access and this one.
	
	while (scanf("%c %d %lx %d\n",&marker,&loadstore,&address,&icount) != EOF)
	{
	  
		if (marker == '#')
		{
			references++;
		}
		else
		{ 
			printf("Oops\n");
			continue;
		}
	
		// for (crude) performance modeling, we will assume a base CPI of 1,
		//     thus every instruction takes one cycle, plus memory access time.
		cycles += icount;
		basecycles += icount;
		cycles += is_cache_miss(loadstore,address,cycles) * miss_penalty;
	}
	
	printf("Simulation results:\n");
	printf("\texecution cycles %ld cycles\n",cycles);
	printf("\tinstructions %ld\n", basecycles);
	printf("\tmemory accesses %ld\n", hits+misses);
	printf("\toverall miss rate %.2f%%\n", 100.0 * (float) misses / ((float) (hits + misses)) );
	printf("\tread miss rate %.2f%%\n", 100.0 * (float) readmisses / ((float) (readhits + readmisses)) );
	printf("\tmemory CPI %.2f\n", (float) (cycles - basecycles) / (float) basecycles);
	printf("\ttotal CPI %.2f\n", (float) 1.0 + (cycles - basecycles) / (float) basecycles);
	printf("\taverage memory access time %.2f cycles\n",  (float) (cycles - basecycles) / (float) (hits + misses));
	printf("load_misses %ld\n", readmisses);
	printf("store_misses %ld\n", misses - readmisses);
	printf("load_hits %ld\n", readhits);
	printf("store_hits %ld\n", hits - readhits);
}

// you will complete this function.  Notice that we pass the 
//    cycle count to this routine as an argument.  That may make
//    it easier to implement lru.
// this routine returns either 0 (hit) or 1 (miss)

int is_cache_miss(int loadstore, long address, int cycles)
{
	int i, j;

	long tag_index_offset = (1 << ADDRESSBITS == 0) ? address : address % (1 << ADDRESSBITS);
	long index_offset = tag_index_offset % (1 << (indexBits + offsetBits));

	unsigned int offset = index_offset % (1 << offsetBits);
	unsigned int index = index_offset >> offsetBits;
	unsigned int tag = tag_index_offset >> (indexBits + offsetBits);

	int *setValids = valids[index];
	int *setTags   =   tags[index];
	int *setLRUs   =   LRUs[index];

	int hit = -1;
	int vacancy = -1;
	int oldest = 0;
	int maxLRU = 0;

	for (i = 0; i < associativity; ++i)
	{
		if (setValids[i] == 1)
		{
			if (setTags[i] == tag)
			{
				hit = i;
				break;
			}
			if (setLRUs[i] > maxLRU)
			{
				maxLRU = setLRUs[i];
				oldest = i;
			}
		}
		else if (vacancy < 0) // We encountered an invalid line
		{
			vacancy = i;
		}
	}

	if (hit >= 0)
	{
		if (loadstore == LOAD) 
		{
			readhits++;
		}
		hits++;

		// Set this line to the most recently used
		for (i = 0; i < associativity; ++i)
		{
			if (setLRUs[i] < setLRUs[hit])
			{
				setLRUs[i]++; 
			}
		}
		setLRUs[hit] = 0;

		return 0;
	}
	else
	{
		if (loadstore == LOAD) 
		{
			readmisses++;
		}
		misses++;

		// Increment all LRU counters in the cache-set
		for (i = 0; i < associativity; ++i)
		{
			setLRUs[i]++; 
		}

		if (vacancy >= 0) // fill the nonvalid cache-line
		{
			setTags[vacancy] = tag;
			setLRUs[vacancy] = 0;
			setValids[vacancy] = 1;
		}
		else // boot out the oldest entry
		{
			setTags[oldest] = tag;
			setLRUs[oldest] = 0;
		}

		return 1;
	}
}

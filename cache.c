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

// a good place to declare your storage for tags, etc.  Obviously,
//   you don't need to actually store the data.

#define BUFFERSIZE 1048576

unsigned int    tags[BUFFERSIZE]; // We allocate more than we actually need
unsigned int indices[BUFFERSIZE];
unsigned int offsets[BUFFERSIZE];

unsigned int  valids[BUFFERSIZE];
unsigned int    LRUs[BUFFERSIZE];

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
	int hit;
	hit = 0;
	if (hit)
	{
		hits++;
		if (loadstore == LOAD) 
		{
			readhits++;
		}
		return 0;
	}
	
	/* miss */
	misses++;
	if (loadstore == LOAD) 
	{
		readmisses++;
	}
	return 1;
}


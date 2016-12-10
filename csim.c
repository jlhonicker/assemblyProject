/* Cache Simulator. 
 * The cache is stored as a series of structs, cache has cacheSet s, the cacheSet has cacheLine s
 * and the cacheLine has a cache's line information.
 *
 *
 * Authors: Theodore Bieber (tjbieber), James Honicker (jlhonicker)
*/
#include "cachelab.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Contains the information about the cache
typedef struct {
	int sets; // -s
	int blocks; // -b

	int S; // 2^s
	int B; // 2^b
	int E; // -e

	int hits;
	int misses;
	int evictions;
} cacheData;

typedef unsigned long long int address_t; // this would be a pain to type out more than once

// Type definitions for the cache data structure
typedef struct {
	int lastUsed;
	int valid;
	address_t tag;
	char *block;
} cacheSetLine;

typedef struct {
	cacheSetLine *lines;
} cacheSet;

typedef struct {
	cacheSet *sets;
} cache;
// end of cache

int verbose; // -v option flag

// function prototypes
void printHelp(char *argv[]);
cache generateCache(long long sets, int lines, long long blockSize);
void freeCache(cache theCache, long long sets, int lines, long long blockSize);
int getEmptyLine(cacheSet set, cacheData data);
int findEvictee(cacheSet set, cacheData cData, int *usedLines);
cacheData simulateCache(cache theCache, cacheData cData, address_t address);

int main(int argc, char *argv[]) {

	int AMT_ARGS = 9;
	int TRACEFILE_NAME_MAX_SIZE = 32;

	cacheData cData;
	cData.hits = 0;
	cData.misses = 0;
	cData.evictions = 0;

	// name of the tracefile -t
	char *traceFileName = malloc(TRACEFILE_NAME_MAX_SIZE);

	if(argc < AMT_ARGS) {
		printHelp(argv);

	} else if(argc == AMT_ARGS) {
		// parse argv with no optional flags
		// allows for the -s -E -b and -t commands to be in any order
		int i = 2;
		for(i = 2; i < argc; i+=2) {
			if(!strncmp(argv[i-1], "-s", 3)) {
				cData.sets = atoi(argv[i]);
			} else if(!strncmp(argv[i-1], "-E", 3)) {
				cData.E = atoi(argv[i]);
			} else if(!strncmp(argv[i-1], "-b", 3)) {
				cData.blocks = atoi(argv[i]);
			} else if(!strncmp(argv[i-1], "-t", 3)) {
				strcpy(traceFileName, argv[i]);
			}
		}

	} else if(argc > AMT_ARGS) {
		// parse argv with optional flags
		// allows for the -s -E -b and -t commands to be in any order (after the optional commands)
		// meaning that the optional commands MUST come first if you want the program to work
		if(!strncmp(argv[1], "-h", 3)) {
			printHelp(argv);
		} else if(!strncmp(argv[1], "-v", 3)) {
			// verbose mode
			verbose = 1;
		}

		int i = 3;
		for(i = 3; i < argc; i+=2) {
			if(!strncmp(argv[i-1], "-s", 3)) {
				cData.sets = atoi(argv[i]);
			} else if(!strncmp(argv[i-1], "-E", 3)) {
				cData.E = atoi(argv[i]);
			} else if(!strncmp(argv[i-1], "-b", 3)) {
				cData.blocks = atoi(argv[i]);
			} else if(!strncmp(argv[i-1], "-t", 3)) {
				strcpy(traceFileName, argv[i]);
			}

		}
	}

	// Finally, assign S and B
	cData.S = 1<<(cData.sets);
	cData.B = 1<<(cData.blocks);
	
	cache myCache = generateCache(cData.S, cData.E, cData.B);

	// Open the file for reading, line by line
	FILE *trace = fopen(traceFileName, "r");

	if(trace == NULL) {
		printf("%s: No such file or directory\n", traceFileName);
		return 1;
	}

	// MAIN LOOP
	char op;
	address_t address;
	int size;

	// filter the trace file line by line, store info into variables
	while(fscanf(trace, " %c %llx,%d", &op, &address, &size) == 3) {
		if(op != 'I') { // Ignore these
			// The "before" data
			int hits = cData.hits;
			int misses = cData.misses;
			int evictions = cData.evictions;
			
			cData = simulateCache(myCache, cData, address);
			if(op == 'M') {
				cData = simulateCache(myCache, cData, address);
			}
			// turn the numbers into booleans
			hits = cData.hits - hits;
			misses = cData.misses - misses;
			evictions = cData.evictions - evictions;
		
			if(verbose) {
				printf("%c %llx,%d", op, address, size);
				
				int counter;
				if(misses) {
					printf(" miss");
				}
				if(evictions) {
					printf(" eviction");
				}
				for(counter = 0; counter < hits; counter++) {
					printf(" hit");
				}
				printf("\n");
			}
		}
		
	}

	// Deallocate all memory and close file(s)
	free(traceFileName);
	freeCache(myCache, cData.S, cData.E, cData.B);
	fclose(trace);

	printSummary(cData.hits, cData.misses, cData.evictions);
    return 0;
}

/* Simulates the cache, updating the summary data for each call.
 * Uses the below functions (other than printHelp), and is called in the main loop
 * Parameters:
 *     theCache: the cache struct that we are using as a cache
 *     cData: the information about the cache
 *     address: the address that we are trying to access.
 *              this is the only parameter that changes between each run of this function
 * return: a new cacheData with updated summary data (hits, misses, evictions)
*/
cacheData simulateCache(cache theCache, cacheData cData, address_t address) {
	int linecounter; // line index
	int cacheFull = 1;

	int lines = cData.E; 
	int previousHits = cData.hits; // so we can check the before and after values

	int tagSize = (64 - (cData.sets + cData.blocks));
	address_t cacheLineTag = address >> (cData.sets + cData.blocks);
	unsigned long long tempaddr = address << (tagSize); 
	unsigned long long setcounter = tempaddr >> (tagSize + cData.blocks);

	cacheSet set = theCache.sets[setcounter];

	for(linecounter = 0; linecounter < lines; linecounter++) {
		cacheSetLine line = set.lines[linecounter];

		if(line.valid) {
			if(line.tag == cacheLineTag) {
				line.lastUsed++;
				cData.hits++;
				set.lines[linecounter] = line;
			}
		} else if(!line.valid && cacheFull) {
			// update flag because we know there's an empty line
			cacheFull = 0;
		}
	}

	if(previousHits == cData.hits) {
		cData.misses++;
	} else {
		return cData;
	}

	// Since we missed, we need to find a spot to take, either evict or find empty space

	int *usedLines = (int*)malloc(sizeof(int)*2); // index 0 is minUsed, 1 is maxUsed
	// Find the least recently used line
	int minUsedIndex = findEvictee(set, cData, usedLines);

	if(cacheFull) {
		cData.evictions++;
		// Evict the least recently used line
		set.lines[minUsedIndex].tag = cacheLineTag;
		set.lines[minUsedIndex].lastUsed = usedLines[1] + 1;
	} else { // there is an empty spot, so we just need to find it
		int emptycounter = getEmptyLine(set, cData);
		set.lines[emptycounter].tag = cacheLineTag;
		set.lines[emptycounter].valid = 1;
		set.lines[emptycounter].lastUsed = usedLines[1] + 1;
	}
	free(usedLines);
	return cData;

}

// Prints out the help message for this program
void printHelp(char *argv[]) {
	printf("Usage: %s [-hv] -s <num> -E <num> -b <num> -t <file>\n", argv[0]);
    printf("Options:\n");
    printf("  -h         Print this help message.\n");
    printf("  -v         Optional verbose flag.\n");
    printf("  -s <num>   Number of set index bits.\n");
    printf("  -E <num>   Number of lines per set.\n");
    printf("  -b <num>   Number of block offset bits.\n");
    printf("  -t <file>  Trace file.\n");
    printf("\nExamples:\n");
    printf("  %s -s 4 -E 1 -b 4 -t traces/yi.trace\n", argv[0]);
    printf("  %s -v -s 8 -E 2 -b 4 -t traces/yi.trace\n", argv[0]);
    exit(0);
}


/* Build the cache according to the specifications.
 * Allocated the space using malloc
 * From the cacheData: sets = S, lines = E, blockSize = B
 * returns the cache.
 * Note: we don't ever actually use blockSize, but I'm putting it there because
 *     if we were actually creating a real cache it would be needed.
*/
cache generateCache(long long sets, int lines, long long blockSize) {
	cache generatedCache;
	cacheSet set;
	cacheSetLine line;

	// allocate the space for the cache's sets
	generatedCache.sets = (cacheSet *) malloc(sizeof(cacheSet) * sets);
	
	// allocate the space for all of the sets' lines
	int setcounter; // set index
	int linecounter; // line index
	for(setcounter = 0; setcounter < sets; setcounter++) {
		set.lines = (cacheSetLine *) malloc(sizeof(cacheSetLine) * lines);
		generatedCache.sets[setcounter] = set;

		for(linecounter = 0; linecounter < lines; linecounter++) {
			line.lastUsed = 0;
			line.valid = 0;
			line.tag = 0;
			set.lines[linecounter] = line;
		}
	}

	return generatedCache;
}

/* Deallocates the memory used by the cache.
 * Note: we don't ever actually use blockSize, but I'm putting it there because
 *     if we were actually creating a real cache it would be needed.
*/
void freeCache(cache theCache, long long sets, int lines, long long blockSize) {
	int setcounter;
	for(setcounter = 0; setcounter < sets; setcounter++) {
		cacheSet set = theCache.sets[setcounter];
		if(set.lines != NULL) {
			free(set.lines); 
		}
	}
	if(theCache.sets != NULL) {
		free(theCache.sets);
	}
}

/* Finds an empty line in the given set by checking the valid bit
 * If for some reason there are no empty lines, it returns -1
 * Parameters:
 * 	   set: the set you are looking for an empty line in
 *     data: the information about the cache
 * return: the index of the line that is free
 		-1 indicates that there were no free lines
*/
int getEmptyLine(cacheSet set, cacheData data) {
	int lines = data.E;
	int index;
	cacheSetLine line;

	for(index = 0; index < lines; index++) {
		line = set.lines[index];
		if(line.valid == 0) {
			return index;
		}
	}
	return -1;
}

/* Finds the index of the least recently used line with a simple search
 * Parameters:
 *     set: the set you are evicting from
 *     cData: the information about the cache
 *     usedLines: a small array that is given minUsed and maxUsed as a side effect
 * return: the index of the line being evicted
*/
int findEvictee(cacheSet set, cacheData cData, int *usedLines) {
	int lines = cData.E;
	int maxUsed = set.lines[0].lastUsed;
	int minUsed = set.lines[0].lastUsed;
	int minUsedIndex = 0;

	cacheSetLine line;
	int linecounter;

	for(linecounter = 1; linecounter < lines; linecounter++) {
		line = set.lines[linecounter];

		if(minUsed > line.lastUsed) {
			minUsedIndex = linecounter;
			minUsed = line.lastUsed;
		}

		if(maxUsed < line.lastUsed) {
			maxUsed = line.lastUsed;
		}
	}
	usedLines[0] = minUsed;
	usedLines[1] = maxUsed;
	return minUsedIndex;
}
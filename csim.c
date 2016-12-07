/* Cache Simulator
 * Authors: Theodore Bieber (tjbieber), James Honicker (jlhonicker)
*/
#include "stdio.h"
#include "cachelab.h"
#include "stdlib.h"
#include "string.h"

void parseValgrind(char *op, char *address, int *size, char *valgrind);

int main(int argc, char *argv[]) {

	int AMT_ARGS = 9;
	int TRACEFILE_NAME_MAX_SIZE = 32;
	// optional variables
	int verbose = 0; // -v

	// required variables
	int sets = 0; // -s
	int associativity = 0; // -e
	int blocks = 0; // -b

	// name of the tracefile -t
	char *traceFileName = malloc(TRACEFILE_NAME_MAX_SIZE);

	// counters
	int hits = 0;
	int misses = 0;
	int evictions = 0;

	if(argc < AMT_ARGS) {
		printf("Usage:\n./csim [-hv] -s <s> -E <E> -b <b> -t <tracefile>\n");
		// check for -h command
		if(argc > 1 && !strncmp(argv[1], "-h", 4)) {
			// print out useful information
			printf("Optional Commands [-hv]:\n\t-h: prints help information\n\t-v: verbose mode, displays trace info\n");
			printf("Required Commands:\n\t-s: number of set index bits\n\t-E: associativity, the number of lines per set\n");
			printf("\t-b: number of block bits\n\t-t: name of the valgrind trace to replay\n");
		}
		return 1;

	} else if(argc == AMT_ARGS) {
		// parse argv with no optional flags
		// allows for the -s -E -b and -t commands to be in any order
		int i = 2;
		for(i = 2; i < AMT_ARGS; i+=2) {
			if(!strncmp(argv[i-1], "-s", 3)) {
				sets = atoi(argv[i]);

			} else if(!strncmp(argv[i-1], "-E", 3)) {
				associativity = atoi(argv[i]);

			} else if(!strncmp(argv[i-1], "-b", 3)) {
				blocks = atoi(argv[i]);

			} else if(!strncmp(argv[i-1], "-t", 3)) {
				strcpy(traceFileName, argv[i]);

			}
		}

	} else if(argc > AMT_ARGS) {
		// parse argv with optional flags
		// allows for the -s -E -b and -t commands to be in any order (after the optional commands)
		// meaning that the optional commands MUST come first if you want the program to work
		if(!strncmp(argv[1], "-h", 3)) {
			// print out useful information
			printf("Usage:\n./csim [-hv] -s <s> -E <E> -b <b> -t <tracefile>\n");
			printf("Optional Commands [-hv]:\n\t-h: prints help information\n\t-v: verbose mode, displays trace info\n");
			printf("Required Commands:\n\t-s: number of set index bits\n\t-E: associativity, the number of lines per set\n");
			printf("\t-b: number of block bits\n\t-t: name of the valgrind trace to replay\n");
			return 1;

		} else if(!strncmp(argv[1], "-v", 3)) {
			// verbose mode
			verbose = 1;

		}

		int i = 3;
		for(i = 3; i < AMT_ARGS; i+=2) {
			if(!strncmp(argv[i-1], "-s", 3)) {
				sets = atoi(argv[i]);

			} else if(!strncmp(argv[i-1], "-E", 3)) {
				associativity = atoi(argv[i]);

			} else if(!strncmp(argv[i-1], "-b", 3)) {
				blocks = atoi(argv[i]);

			} else if(!strncmp(argv[i-1], "-t", 3)) {
				strcpy(traceFileName, argv[i]);

			}

		}
	}
	
	// Open the file for reading, line by line
	FILE *trace = fopen(traceFileName, "r");
	int length = 30;
	char *line = malloc(length);
	
	// just to get rid of warnings for now
	line = line;
	blocks = blocks;
	associativity = associativity;
	sets = sets;
	verbose = verbose;


	if(trace == NULL) {
		printf("%s: No such file or directory\n", traceFileName);
		return 1;
	}


	// MAIN LOOP
	char op;
	char *address = malloc(length);
	int size;
	while(fgets(line, length, trace)) {
		printf("%s", line);
		parseValgrind(&op, address, &size, line);
		
	}

	// Deallocate all memory and close file(s)
	free(traceFileName);
	free(line);
	free(address);
	fclose(trace);

	printSummary(hits, misses, evictions);
    return 0;
}

/* Parses a single line of  valgrind information
 * Parameters:
 *     int *op: the operation,
 *	   int *address: the address
 *     int *size: the size
 *     char *valgrind: the source string being parsed
 * Does not return anything, just stores the values in the parameters, like sscanf
 * Can't use sscanf because of the minor inconsistency in the formatting
*/
void parseValgrind(char *op, char *address, int *size, char *valgrind) {
	
	*op = 0;// initialize it to 0, so it can be used as a flag as well

	if(*valgrind == 'I') {
		*op = *valgrind; // because I's are followed by two spaces
	}
	valgrind++;
	if(*valgrind == 'M') {
		*op = *valgrind;
	} else if(*valgrind == 'L') {
		*op = *valgrind;
	} else if(*valgrind == 'S') {
		*op = *valgrind;
	}
	valgrind++;

	if(*op && (*valgrind == ' ')) {
		int i = 0;
		for(i = 0; *valgrind != ','; i++) {
			if(valgrind[i] == ',') {
				address[i] = '\0';
				break;
			}
			address[i] = valgrind[i];
		}
		address[i+1] = '\0';
		valgrind+=(i);
	}
	if(*valgrind == ',') {
		valgrind++;
		*size = atoi(valgrind);
	}
}

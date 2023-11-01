#include "cachelab.h"
#include "getopt.h"
#include "stdbool.h"
#include "math.h"
#include "stdio.h"
#include "string.h"
#include "stdlib.h"

#define addrSize 64;  //8 byte address 

// Global variables
int hits;
int misses;
int evictions;
int dirty_evicted;
int dirty_active;
int double_accesses;
int s;      //set bits
int S;      //set amount
int E;      //associativity
int B;      //block size
int b;      //block offset bits


struct Trace {
    long address;
    //…
};

struct Line { //provide information for each line
    long tag;
    bool valid;
    bool dirty;
};

struct Set {
    struct Line *lines; //array of lines
};

struct Cache {
    struct Set *sets; //array of sets
};

//TODO: make function to allocate memory for cache
struct Cache makeCache(int S, int E, int B){
    struct Cache newCache;
    struct Set newSet;
    struct Line newLine;

    //alloc space for S sets
    newCache.sets = (struct Set *) malloc(S * sizeof(struct Set));

    //alloc space for E lines in each of set
    for (int i=0; i < S; i++){
        newSet.lines = (struct Line *) malloc(E * sizeof(struct Line));
        newCache.sets[i] = newSet;

        //set initial characteristics of each line
        for (int j; i < E; j++){
            newLine.valid = false;
            newLine.dirty = false;
            newLine.tag = 0;
            newSet.lines[j] = newLine;
        }
    }
    return newCache;
}

//freeCache frees space allocated for the cache
void freeCache(struct Cache thisCache, int S, int E, int B){
    for (int i = S; i >= 0; i--){     
        if(thisCache.sets[i].lines != NULL){
            free(thisCache.sets[i].lines);
        }
    }

    //free space for sets once alloc'd space for lines are freed
    if(thisCache.sets != NULL){
        free(thisCache.sets);
    }    
}

//TODO: make function for load and store
int load(struct Cache thisCache, unsigned long address){
    return 0;
}

int store(struct Cache thisCache, unsigned long address){
    return 0;
}

int main(int argc, char * argv[]){
    S = pow(2.0, s); //calc S = 2^s
    B = pow(2.0, b); //calc B = 2^b
    
    // iniitializes the cache
    struct Cache simCache = makeCache(S, E, B);
    
    //gets params s,E,b,t,v, and h from cmd line argument
    char opt; // hold current arg from cmd
    char * trace; //trace file

    //TODO: if time allows, work on v and h args
    while ((opt = getopt(argc, argv, "s:E:b:t:vh")) != -1) {
        switch (opt) {
            case 's':
                s = strtoul(optarg, (char **) trace, 10); //gets s value
                break;
            case 'E':
                E = strtoul(optarg, (char **) trace, 10); //gets E value
                break;
            case 'b':
                b = strtoul(optarg, (char **) trace, 10); //gets b value
                break;
            case 't':
                trace = optarg; //puts filename in current arg
                break;
        }
    }

    // reading from trace file
    FILE * f = fopen(trace, "r");
    char op;
    int size;
    unsigned long addr;

    while (fscanf(f, "%c %lx, %d", &op, &addr, &size) != EOF)
    {
        switch (op) {
        case 'I': //ignore I
            break;
        case 'L':
            //load(simCache, addr);
            break;
        
        case 'S':
            //store(simCache, addr);
            break;

        case 'M':
            //load(simCache, addr);
            //store(simCache, addr);
            break;
        }
    }

    printSummary(hits, misses, evictions, dirty_evicted, dirty_active, double_accesses);
    
    // to prevent memory leak, free up the space held up cache
    freeCache(simCache, S, E, B);
    fclose(f);

    return 0;
}

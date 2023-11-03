#include "cachelab.h"
#include "getopt.h"
#include "stdbool.h"
#include "math.h"
#include "stdio.h"
#include "string.h"
#include "stdlib.h"

#define addrSize 64;  //8 byte address 

//Global variables
int hits;
int misses;
int evictions;
int dirty_evicted;
int dirty_active;
int double_accesses;

//Useful structs to access parts of cache and its characteristics

struct Values { 
    int s;      //set bits
    int S;      //set amount
    int E;      //associativity
    int B;      //block size
    int b;      //block offset bits
};

struct Trace {
    long address;
};

struct Line { 
    long tag;
    bool valid;
    bool dirty;
    bool lru;
};

struct Set {
    struct Line *lines;
};

struct Cache {
    struct Set *sets; 
};


//Helper Functions

//makeCache allocates memory for cache (for use in initializing cache)
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

//setNotFull checks if a valid bit in set has not been set to true
bool setNotFull(struct Set thisSet){
    struct Values V;
    for (int i=0; i<V.E; i++){
        if(!set.lines[i].valid){
            return true;
        }
    }
    return false; //all valid bits set to true, so it's full
}

//emptyLineIndex checks for empty lines in a specifc set in the case of a cold miss
int emptyLineIndex(struct Set thisSet){
    struct Values V;
    struct Line thisLine;

    for (int i = 0; i < V.E; i++){
        if(!thisSet.lines[i].valid){
            return i;
        }
    }
    return 0;
}

int LRUindex(struct Set thisSet){

}

int LRUindex(struct Set thisSet){
    
}

//Cache Operations
//TODO: make function for load and store
void load(struct Cache thisCache, long address){
    struct Values V;
    int t = addrSize - V.s - V.b;
    long addressTag = address >> (V.s + V.b);
    long setVal = (address << (t)) >> (t + V.b); 
    struct Set thisSet = thisCache.sets[setVal]; 

    for (int i = 0; i < V.E; i++) {
        struct Line thisLine = thisSet.lines[i]; 
        if ((thisLine.tag == address) && (thisLine.valid)) { // Hit
            hits++;
        }
    }

}

void store(struct Cache thisCache, long address){


}
///
int main(int argc, char * argv[]){  
    struct Values V;
    
    V.S = pow(2.0, V.s); //calc S = 2^s
    V.B = pow(2.0, V.b); //calc B = 2^b
    
    // iniitializes the cache
    struct Cache simCache = makeCache(V.S, V.E, V.B);
    
    //gets params s,E,b,t,v, and h from cmd line argument
    char opt; // hold current arg from cmd
    char * trace; //trace file

    //TODO: if time allows, work on v and h args
    while ((opt = getopt(argc, argv, "s:E:b:t:vh")) != -1) {
        switch (opt) {
            case 's':
                V.s = strtoul(optarg, (char **) trace, 10); //gets s value
                break;
            case 'E':
                V.E = strtoul(optarg, (char **) trace, 10); //gets E value
                break;
            case 'b':
                V.b = strtoul(optarg, (char **) trace, 10); //gets b value
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
            //load(simCache, Values, addr);
            break;
        
        case 'S':
            //store(simCache, Values, addr);
            break;

        case 'M':
            //load(simCache, Values, addr);
            //store(simCache, Values, addr);
            break;
        }
    }

    printSummary(hits, misses, evictions, dirty_evicted, dirty_active, double_accesses);
    
    // to prevent memory leak, free up the space held up cache
    freeCache(simCache, V.S, V.E, V.B);
    fclose(f);

    return 0;
}

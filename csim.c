#include "cachelab.h"
#include "stdbool.h"
#include "stdio.h"
#include "getopt.h"
#include "stdlib.h"
#include "math.h"


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

struct Line { 
    int tag;
    int use;
    bool valid;
    bool dirty;
};


struct Set {
    struct Line *lines;
};

struct Cache {
    struct Set *sets; 
};

struct Policy
{
    int MRU;
    int LRU;
};

#define addrSize 64  //8 byte address 

//Helper Functions

//makeCache allocates memory for cache (for use in initializing cache)
struct Cache makeCache(struct Values V){
    struct Cache newCache;
    struct Set newSet;
    struct Line newLine;

    newCache.sets = (struct Set *) malloc(V.S * sizeof(struct Set));

    for (int i = 0; i < V.S; i++){
        newSet.lines = (struct Line *) malloc (V.E * sizeof(struct Line));
        newCache.sets[i] = newSet;

        for (int j = 0; j < V.E; j++){
            newLine.use = 0;
            newLine.valid = false;
            newLine.dirty = false;
            newLine.tag = 0;
            newSet.lines[j] = newLine;
        }
    }
    return newCache;
}
//freeCache frees space allocated for the cache
void freeCache(struct Cache thisCache, struct Values V){
    for (int i = V.S; i >= 0; i--){     
        if (thisCache.sets[i].lines != NULL){
            free(thisCache.sets[i].lines);
        }
    }

    //free space for sets once alloc'd space for lines are freed
    if (thisCache.sets != NULL){
        free(thisCache.sets);
    }    
}

//isHit checks to see if there has been a hit
bool isHit(struct Line thisCache, int address){
    return thisCache.tag == address && thisCache.valid;
}

//setNotFull checks if a valid bit in set has not been set to true
bool setNotFull(struct Set thisSet, struct Values V){
    for (int i=0; i<V.E; i++){
        if (!thisSet.lines[i].valid){
            return true;
        }
    }
    return false; //all valid bits set to true, so it's full
}

//emptyLineIndex checks for empty lines in a set in the case of cold miss
int emptyLineIndex(struct Set thisSet, struct Values V){
    for (int i = 0; i < V.E; i++){
        if (!thisSet.lines[i].valid){
            return i;
        }
    }
    return 0;
}

//recentlyUsed allows for finding most recently used and least recently used
struct Policy recentlyUsed(struct Set thisSet, struct Values V){
    struct Policy P;
    int lruItem = thisSet.lines[0].use;
    int mruItem = thisSet.lines[0].use;
    P.LRU = 0;
    P.MRU = 0;

    for (int i=0; i < V.E; i++){
        if (thisSet.lines[i].use < lruItem){
            P.LRU = i;
            lruItem = thisSet.lines[i].use;
        }
    }

    for (int i=0; i < V.E; i++){
        if (thisSet.lines[i].use > mruItem){
            P.MRU = i;
            mruItem = thisSet.lines[i].use;
        }
    }
    return P;
}

//Cache Operations

void load(struct Cache thisCache, unsigned int address, struct Values V){
    int t = addrSize - V.s - V.b;
    int addressTag = address >> (V.s + V.b);
    long setVal = address << t >> (t + V.b); 
    struct Set thisSet = thisCache.sets[setVal]; 
    int thisMRU = recentlyUsed(thisSet, V).MRU;
    int thisLRU = recentlyUsed(thisSet, V).LRU;
    
    bool hit = false;
    
    for (int i = 0; i < V.E; i++) {
        struct Line thisLine = thisSet.lines[i];
        
        if (isHit(thisLine, addressTag)) {
            hits++;
            hit = true;
        
            if (thisMRU == i){ //for if we JUST accessed this block
                double_accesses++;
            }
        
            thisCache.sets[setVal].lines[i].use = thisCache.sets[setVal].lines[thisMRU].use + 1;
            break;
        }
    }

    if (!hit && setNotFull(thisSet, V)) { //cold miss
        misses++;
        int emptyLine = emptyLineIndex(thisSet, V);
        thisSet.lines[emptyLine].tag = addressTag;
        thisSet.lines[emptyLine].dirty = false;
        thisSet.lines[emptyLine].valid = true;
        thisCache.sets[setVal].lines[emptyLine].use = thisCache.sets[setVal].lines[thisMRU].use + 1;

    }

    else if (!hit) {//conflict miss
        misses++;
        evictions++;
        
        if(thisSet.lines[thisLRU].dirty) {
            dirty_evicted += pow(2.0, V.b);
        }
        
        thisSet.lines[thisLRU].tag = addressTag;
        thisSet.lines[thisLRU].dirty = false;
        thisCache.sets[setVal].lines[thisLRU].use = thisCache.sets[setVal].lines[thisMRU].use + 1;
    }
}

void store(struct Cache thisCache, unsigned int address, struct Values V){
    int t = addrSize - V.s - V.b;
    int addressTag = address >> (V.s + V.b);
    long setVal = address << t >> (t + V.b); 
    struct Set thisSet = thisCache.sets[setVal]; 
    int thisMRU = recentlyUsed(thisSet, V).MRU;
    int thisLRU = recentlyUsed(thisSet, V).LRU;
    
    bool hit = false;

    for (int i = 0; i < V.E; i++) {
        struct Line thisLine = thisSet.lines[i];
        
        if (isHit(thisLine, addressTag)) {
            hit = true;
            hits++;

            if (thisMRU == i){ //for if we JUST accessed this block
                double_accesses++;
            }

            thisCache.sets[setVal].lines[i].dirty = true;
            thisCache.sets[setVal].lines[i].use = thisCache.sets[setVal].lines[thisMRU].use + 1;
            break;
        }
    }

        
        if (!hit && setNotFull(thisSet, V)) { //cold miss
            misses++;

            int emptyLine = emptyLineIndex(thisSet, V);

            thisSet.lines[emptyLine].tag = addressTag;
            thisSet.lines[emptyLine].dirty = true;
            thisSet.lines[emptyLine].valid = true;
            thisCache.sets[setVal].lines[emptyLine].use = thisCache.sets[setVal].lines[thisMRU].use + 1;

        }

       
        else if (!hit) { //conflict miss
            misses++;
            evictions++;

            if(thisSet.lines[thisLRU].dirty) {
                dirty_evicted += pow(2.0, V.b);
            }

            thisSet.lines[thisLRU].tag = addressTag;
            thisSet.lines[thisLRU].dirty = true;
            thisCache.sets[setVal].lines[thisLRU].use = thisCache.sets[setVal].lines[thisMRU].use + 1;
        }
}

int main(int argc, char * argv[]){  
    struct Cache simCache;
    struct Values V;
    
    //gets params s,E,b,t,v, and h from cmd line argument
    char opt; // hold current arg from cmd
    char * trace; //trace file

    //TODO: if time allows, work on v and h args
    while ((opt = getopt(argc, argv, "s:E:b:t:vh")) != -1) {
        switch (opt) {
            case 'h':
                exit(0);
                break;
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

    V.B = pow(2.0, V.b); //calc B = 2^b
    V.S = pow(2.0, V.s); //calc S = 2^s
    
    // iniitializes the cache
    simCache = makeCache(V);
    
    
     //reading from trace file
    FILE * f = fopen(trace, "r");
    char op;
    int size;
    unsigned int addr;
    
    if (f != NULL) {
        while (fscanf(f, "%c %x, %d", &op, &addr, &size) != EOF)
        {
            switch (op) {
            case 'I': //ignore I
                break;
            case 'L':
                load(simCache, addr, V);
                break;
            
            case 'S':
                store(simCache, addr, V);
                break;

            case 'M':
                load(simCache, addr, V);
                store(simCache, addr, V);
                break;
            }
        }
    }
    
    //for counting the active dirty bytes
    for (int i = 0; i < pow(2.0, V.s); i++) {
        for (int j = 0; j < V.E; j++) {
            if (simCache.sets[i].lines[j].dirty) {
                dirty_active += pow(2.0, V.b);
            }
        }
    }

    printSummary(hits, misses, evictions, dirty_evicted, dirty_active, double_accesses);
    
    //to prevent memory leak, free up the space held up cache and close file
    freeCache(simCache,V);
    fclose(f);
    return 0;
}

/*
GEORGIY DANIELYAN
COMP 222 - COMPUTER ORGANIZATION AND ARCHITECTURE
SEPTEMBER 30TH 2015
PROFESSOR GEORGE LAZIK
*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

//booleans for error checking
typedef int bool;
#define true 1
#define false 0
 
//Used for bold printing of messages
char ESC=27;

//Method Prototypes
void printMenu();
void setParameters();
void readCache();
void writeToCache();
bool isPowerOfTwo(int a);

//Struct to represent cache line
//tag (integer) & block (integer pointer)
//define cache to be a pointer to the struct
struct cacheLine {
	int tag;
	int* block;
};

//Global Variables
int* mainMemory;
int mainMemorySize;
struct cacheLine* cache;
int numOfCacheLines; //same as number of cache blocks
int cacheSize;
int blockSize;
int numOfMainMemBlocks;
int memoryAddress;
int value;

int main(){
	int choice;
	do{
		choice = 0;
		printMenu();
		scanf("%d", &choice);
		switch(choice){
			case 1: 
				setParameters();
				break;
			case 2: 
				readCache();
				break;
			case 3: 
				writeToCache(); 
				break;
			case 4:
				free(cache);
				free(mainMemory);
				exit(0);
				break;
			default:
				printf("Invalid selection\n\n");
				//Clear the input stream in case of error inputs
				while ((choice = getchar()) != '\n' && choice != EOF);
				break;	
		}
	}while(choice != 4);
	return 0;
}

//Methods 
void printMenu(){
	printf("\nGeorgiy Danielyan\n");
	printf("\nMain memory to Cache memory mapping:\n");
	printf("------------------------------------\n");
	printf("1) Set parameters\n");
	printf("2) Read cache\n");
	printf("3) Write to cache\n");
	printf("4) Exit\n");
	printf("\nEnter selection: ");
}

void setParameters(){
	printf("\nEnter main memory size (words): ");
	scanf("%d", &mainMemorySize);
	printf("Enter cache size (words): ");
	scanf("%d", &cacheSize);
	printf("Enter block size (words/block): ");
	scanf("%d", &blockSize);

	// Error checking
	printf("%c[1m",ESC);  /*- turn on bold */
	if(!isPowerOfTwo(mainMemorySize)){
		printf("\n** Error - main memory size not a power of 2!\n");
	}else if(!isPowerOfTwo(cacheSize)){
		printf("\n** Error - cache size  not a power of 2!\n");
	}else if(blockSize > cacheSize){
		printf("\n** Error - block size larger than cache size!\n");
	}else{
		numOfMainMemBlocks = mainMemorySize/blockSize;//might not need
		numOfCacheLines = cacheSize/blockSize;
		cache = (struct cacheLine*)malloc(sizeof(struct cacheLine)*numOfCacheLines);
		mainMemory = (int*)malloc(sizeof(int)*mainMemorySize);
		int i;
		for(i=0; i<mainMemorySize; i++){
			mainMemory[i] = mainMemorySize-i;
		}
		for(i=0; i<numOfCacheLines; i++){
			cache[i].tag = -1;
			cache[i].block = NULL;
		}
		printf("\n** Data Accepted\n");
	}
	printf("%c[0m",ESC); /* turn off bold */
}

void readCache(){
	printf("\nEnter main memory address to read from: ");
	scanf("%d", &memoryAddress);
	int blockNum = ((memoryAddress-1)%cacheSize/blockSize);
	int wordNum = memoryAddress%blockSize;
	int tagNum = memoryAddress/cacheSize;
	int startingMemory = memoryAddress - memoryAddress%16; //starting address of the block that needs to be copied into cache.

	printf("%c[1m",ESC); //- turn on bold
	//if the block has not been allocated or the tag within block does not match the tag we need.
	if(cache[blockNum].block == NULL || cache[blockNum].tag != tagNum){
		if(cache[blockNum].block == NULL){
			cache[blockNum].block = malloc(sizeof(int)*blockSize);//allocate a new block if our block was null
			int i;
			for(i=0; i < blockSize; i++){
				cache[blockNum].block[i] = mainMemory[startingMemory++];
				cache[blockNum].tag = tagNum;
			}
		}
		value = cache[blockNum].block[wordNum];//get the value from cache. if the value is incorrect then we must have some problem within the calculations.
		printf("\n** Read Miss...First Load Block from Memory!");
		printf("\n** Word %d of cache line %d with tag %d contains value %d\n", wordNum, blockNum, tagNum, value);
	}
	//if the tag does match then return the value from cache. if the value is incorrect then we must have a problem within calculations.
	else{
		value = cache[blockNum].block[wordNum];
		printf("\n** Block in cache");
		printf("\n** Word %d of cache line %d with tag %d contains value %d\n", wordNum, blockNum, tagNum, value);
	}
	printf("%c[0m",ESC); /* turn off bold */

}

void writeToCache(){
	printf("\nEnter main memory address to write to: ");
	scanf("%d", &memoryAddress);
	printf("Enter value to write: ");
	scanf("%d", &value);
	int blockNum = ((memoryAddress-1)%cacheSize/blockSize);
	int wordNum = memoryAddress%blockSize;
	int tagNum = memoryAddress/cacheSize;
	int startingMemory = memoryAddress - memoryAddress%16; //starting address of the block that needs to be copied into cache.

	// Error checking
	printf("%c[1m",ESC);  /*- turn on bold */
	//if the block has not been allocate then create a new block from memory and copy content into cache.
	if(cache[blockNum].block == NULL){
		printf("\n** Write Miss...First Load Block from Memory!");
		cache[blockNum].block = malloc(sizeof(int)*blockSize);
		int i;
		mainMemory[memoryAddress] = value;
		for(i=0; i < blockSize; i++){
			cache[blockNum].block[i] = mainMemory[startingMemory++];
			cache[blockNum].tag = tagNum;
		}
		printf("\n** Word %d of cache line %d with tag %d contains value %d\n", wordNum, blockNum, tagNum, value);
		/*
		startingMemory = memoryAddress - memoryAddress%16;
		for(i=0; i < blockSize; i++){
			printf("\n%d", cache[blockNum].block[i]);
			startingMemory++;
		}*/
	}
	//if the block has been allocated and the block that we need is in cache then do a write through and return the value.
	else if(cache[blockNum].block != NULL && cache[blockNum].tag == tagNum){
		cache[blockNum].block[wordNum] = value;
		mainMemory[memoryAddress] = cache[blockNum].block[wordNum];
		printf("\n** Block in Cache");
		printf("\n** Word %d of cache line %d with tag %d contains value %d\n", wordNum, blockNum, tagNum, value);
	}
	//if the block has been allocated but we need to replace it then copy contents from main memory into the correct cache block.
	else{
		int i;
		mainMemory[memoryAddress] = value;
		for(i=0; i < blockSize; i++){
			cache[blockNum].block[i] = mainMemory[startingMemory++];
			cache[blockNum].tag = tagNum;
		}
	}
	printf("%c[0m",ESC); /* turn off bold */
}

//Helper method to test for power of 2
bool isPowerOfTwo(int a){
	return a && (!(a&(a-1)));
}

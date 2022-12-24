#include<stdio.h>
#include<stdlib.h>
#include<time.h>
#include<unistd.h>
#include<wait.h>
#include<sys/ipc.h>
#include<sys/shm.h>
#include<sys/types.h>
#include<signal.h>
#include<semaphore.h>
#include<sys/msg.h>
#include<string.h>
#include <pthread.h>
#define true 1
#define false 0
#define TRUE 1
#define FALSE 0
#define and &&
#define or ||
#define not !
#define allowedConnections 120
typedef struct Connection Connection;
struct Connection{

};
int createSharedMemoryId(const int accessKey){
    int sharedMemoryId;
    do{
        sharedMemoryId = shmget(accessKey,sizeof(int),IPC_CREAT|0666);
        if(sharedMemoryId < 0) printf("Error creating the shared memory\nTrying again...\n");
    }while(sharedMemoryId < 0);
    return sharedMemoryId;
}
int* createSharedMemory(const int sharedMemoryId){
    int *sharedMemory;
    do{
        sharedMemory = shmat(sharedMemoryId,0,0);
        if(*sharedMemory == -1) printf("Error accessing\nTrying again...\n");
    }while(*sharedMemory == -1);
    return sharedMemory;
}
int* clientConnection(){
    int* sharedMemory;
    int sharedMemoryId = createSharedMemoryId((key_t)ftok("ServerConnection",67321));
    do{
        sharedMemory = shmat(sharedMemoryId,0,0);
        if(sharedMemory < 0){ printf("Error trying to connect to the server\nTrying again...\n"); sleep(1);}
    }while(sharedMemory < 0);
    return sharedMemory;
}
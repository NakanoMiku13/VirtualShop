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
#define clientKey (key_t)ftok("ActiveServer",79653)
#define serverKey (key_t)ftok("ServerConnection",67321)
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
int createSharedMemoryListId(const int accessKey){
    int sharedMemoryId;
    do{
        printf("%d\n",sharedMemoryId);
        sharedMemoryId = shmget(accessKey,sizeof(int)*allowedConnections,IPC_CREAT|0666);
        printf("%d\n",sharedMemoryId);
        if(sharedMemoryId < 0) {printf("Error creating the connection list\nTrying again...\n"); sleep(1);}
    }while(sharedMemoryId < 0);
    return sharedMemoryId;
}
int* createSharedMemoryList(const int sharedMemoryId){
    int* sharedMemory;
    do{
        sharedMemory = (int*)shmat(sharedMemoryId,0,0);
    }while(sharedMemory != NULL);
    return sharedMemory;
}
int* clientConnection(){
    int* sharedMemory;
    int sharedMemoryId = createSharedMemoryId(serverKey);
    do{
        sharedMemory = shmat(sharedMemoryId,0,0);
        if(sharedMemory < 0){ printf("Error trying to connect to the server\nTrying again...\n"); sleep(1);}
    }while(sharedMemory < 0);
    return sharedMemory;
}
void releaseConnection(){
    int* sharedMemory = createSharedMemory(createSharedMemoryId((key_t)ftok("ServerConnection",67321)));
}
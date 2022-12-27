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
#include<pthread.h>
#define true 1
#define false 0
#define TRUE 1
#define FALSE 0
#define and &&
#define or ||
#define not !
#define allowedConnections 120
#define key 89
#define clientKey (key_t)ftok("shm/ActiveServer",key)
#define serverKey (key_t)ftok("shm/ServerConnection",key)
#define connectionKey (key_t)ftok("shm/Connections",key)
#define userConnectedKey (key_t)ftok("shm/UserConnection",key)
#define userDbKey (key_t)ftok("shm/userDB",key)
#define user 10
#define client 20
static int connected;
typedef struct Product Product;
typedef struct User User;
struct Product{
    unsigned int id, amount;
    char productName[35];
    float price;
};
struct User{
    int id;
    int permission;
    char username[35];
    char password[35];
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
        sharedMemoryId = shmget(accessKey,(sizeof(int)*allowedConnections),IPC_CREAT|0666);
        if(sharedMemoryId < 0) {printf("Error creating the connection list\nTrying again...\n"); sleep(1);}
    }while(sharedMemoryId < 0);
    return sharedMemoryId;
}
int* createSharedMemoryList(const int sharedMemoryId){
    int* sharedMemory;
    do{
        sharedMemory = (int*)shmat(sharedMemoryId,0,0);
        if(sharedMemory == NULL) printf("Error getting the connections list...\nTrying again...\n");
    }while(sharedMemory == NULL);
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
int stillConnected(const int begin,const int end, int* users, const pid_t pid){
    if(begin >= end){
        if(users[begin] == pid) return begin;
        else return -1;
    }else{
        int mid = (begin + end) / 2;
        if(users[mid] == pid) return mid;
        else if(users[mid] > pid) return stillConnected(begin,mid-1,users,pid);
        else return stillConnected(mid,end,users,pid);
    }
}
void releaseConnection(const pid_t pid){
    int* sharedMemory = createSharedMemoryList(createSharedMemoryListId(connectionKey));
    int* connected = createSharedMemory(createSharedMemoryId(userConnectedKey));
    int pos = stillConnected(0,*connected,sharedMemory,pid);
    if(pos != -1){
        int* arr = (int*)malloc(sizeof(int)*(*connected)-1);
        for(int i = 0 ; i < pos ; i++) arr[i] = sharedMemory[i];
        pos++;
        for(int i = pos ; i < *connected ; i++) arr[i] = sharedMemory[i];
        sharedMemory = arr;
        *connected -= 1;
        printf("User %d disconnected\n",pid);
    }else{
        printf("User not found\n");
    }
}

void closeConnection(FILE* file){
    fclose(file);
}
FILE* stablishConnectionUserDB(){
    FILE *file = fopen("db/users","r+");
    return file;
}
FILE* stablishConnectionProductDB(){
    return fopen("db/products","r+");
}
int userCount(){
    FILE* file = stablishConnectionUserDB();
    size_t count = 0;
    char temp[250];
    while(fgets(temp,512,file)!=NULL){if(temp[0] == '|') count++;}
    closeConnection(file);
    return count;
}
void createUser(User newUser){
    FILE *db = stablishConnectionUserDB();
    int users = userCount();
    newUser.id = users+1;
    char buffer[35];
    char* temp;
    fprintf(db,"|\n");
    /*sprintf(buffer,"%d",newUser.id);
    fputs(buffer,db);
    fputs("\n",db);
    fputs(newUser.username,db);
    fputs("\n",db);
    fputs(newUser.password,db);
    fputs("\n",db);
    sprintf(buffer,"%d",newUser.permission);
    fputs(buffer,db);
    fputs("\n",db);*/
    //closeConnection(db);
}

int createSharedUsersId(){
    int sharedMemoryId;
    do{
        sharedMemoryId = shmget(userDbKey,sizeof(User)*250,IPC_CREAT|0666);
        if(sharedMemoryId < 0) printf("Error setting the space\n");
    }while(sharedMemoryId < 0);
    return sharedMemoryId;
}
User* createVirtualSpace(){
    User* users;
    do{
        users = shmat(createSharedUsersId(),0,0);
        if(users == NULL) printf("Error getting space data...\n");
    }while(users == NULL);
    return users;
}
User* loadDataFromDB(FILE* db){
    int count = userCount();
    if(count > 250){
        printf("MaxLoad from database is 250...\n");
        return NULL;
    }
    User* users = (User*)malloc(sizeof(User)*count+2);
    User newUser;
    char* data = (char*)malloc(sizeof(char)*35);
    for(int i = 0 ; i < count ; i++){
        users = malloc(sizeof(User));
        char buffer[512];
        fgets(buffer,512,db);
        printf("%d) %s\n",i,buffer);
        int pos = 2, index = 0;
        data = (char*) malloc(sizeof(char)*35);
        while(buffer[pos] != ' '){
            data[index] = buffer[pos];
            index++;
            pos++;
        }
        users[i].id = atoi(data);
        pos++;
        index = 0;
        data = (char*) malloc(sizeof(char)*35);
        while(buffer[pos] != ' '){
            data[index] = buffer[pos];
            index++;
            pos++;
        }
        users[i].permission = atoi(data);
        printf("permission created\n");
        pos++;
        index = 0;
        data = (char*) malloc(sizeof(char)*35);
        while(buffer[pos] != ' '){
            data[index] = buffer[pos];
            index++;
            pos++;
        }
        strcpy(users[i].username,data);
        pos++;
        index = 0;
        data = (char*) malloc(sizeof(char)*35);
        while(buffer[pos] != ' '){
            data[index] = buffer[pos];
            index++;
            pos++;
        }
        strcpy(users[i].password,data);
        printf("hi\n");
        data = (char*)malloc(sizeof(char)*35);
    }
    return users;
}
void loadUsersToDataBase(){

}
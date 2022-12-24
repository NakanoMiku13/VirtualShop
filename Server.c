#include "VShopLib.h"
int* connections,*connection;
int usersConnected;
sem_t mutex;
void* createConnection(void* args){
    sem_wait(&mutex);
    connections[usersConnected] = *connection;
    printf("User (id: %d) connected\n",*connection);
    *connection = 0;
    usersConnected++;
    sem_post(&mutex);
}
int main(){
    sleep(2);
    key_t keyConnection = (key_t)ftok("ServerConnection",67321);
    connections = (int*)malloc(sizeof(int)*allowedConnections);
    printf("Connection list created...\n");
    sleep(2);
    connection = createSharedMemory(createSharedMemoryId(keyConnection));
    *connection = 0;
    printf("Connection access key created...\n");
    sleep(2);
    sem_init(&mutex,0,1);
    pthread_t connect;
    usersConnected = 0;
    printf("Server initialization complete...\n");
    sleep(2);
    do{
        if(*connection != 0 && usersConnected < allowedConnections){
            printf("User connection detected...\n");
            pthread_create(&connect,NULL,createConnection,NULL);
            pthread_join(connect,NULL);
        }else if(*connection == 0 && usersConnected == 0){
            printf("Waiting a connection...\n");
            sleep(1);
        }
        if(usersConnected > 0){
            printf("%d user (s) connected\n",usersConnected);
            for(int i = 0 ; i < usersConnected ; i++){
                printf("%d) %d\n",i,connections[i]);
            }
            sleep(1);
        }
    }while(1 == 1);
}
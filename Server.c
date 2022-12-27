#include "VShopLib.h"
int* connections,*connection;
int* usersConnected;
sem_t mutex;
FILE *userDB,*productDB;
User* users;
void* createConnection(void* args){
    sem_wait(&mutex);
    connections[*usersConnected] = *connection;
    printf("User (id: %d) connected\n",*connection);
    *connection = 0;
    *usersConnected += 1;
    printf("dd: %d\n",*usersConnected);
    sem_post(&mutex);
}
int main(){
    printf("Initializing server...\n");
    int* activeServer = createSharedMemory(createSharedMemoryId(clientKey));
    *activeServer = false;
    printf("Starting server...\n");
    sleep(2);
    printf("Creating connection list...\n");
    connections = createSharedMemoryList(createSharedMemoryListId(connectionKey));
    usersConnected = createSharedMemory(createSharedMemoryId(userConnectedKey));
    *usersConnected = 0;
    printf("Connection list created...\n");
    sleep(2);
    printf("Creating access key connection...\n");
    connection = createSharedMemory(createSharedMemoryId(serverKey));
    *connection = 0;
    printf("Connection access key created...\n");
    sem_init(&mutex,0,1);
    pthread_t connect;
    sleep(2);
    printf("Establishing connection with databases...\n");
    userDB = stablishConnectionUserDB();
    users = createVirtualSpace();
    users = loadDataFromDB(userDB);
    int t = userCount();
    for(int i = 0 ; i < t ; i++){
        printf("id: %d\nName: %s\n",users[i].id,users[i].username);
    }
    sleep(10);
    closeConnection(userDB);
    printf("Server initialization complete...\n");
    sleep(2);
    *activeServer = true;
    do{
        if(*usersConnected == 1);
        else if(*connection != 0 && *usersConnected < allowedConnections){
            printf("User connection detected...\n");
            pthread_create(&connect,NULL,createConnection,NULL);
            pthread_join(connect,NULL);
        }else if(*connection == 0 && *usersConnected == 0){
            printf("Waiting a connection...\n");
            sleep(1);
        }
        if(*usersConnected > 0){
            printf("%d user (s) connected\n",*usersConnected);
            for(int i = 0 ; i < *usersConnected ; i++){
                printf("%d) %d\n",i,connections[i]);
            }
            sleep(1);
        }
    }while(1 == 1);
}
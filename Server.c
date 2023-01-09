#include "VShopLib.h"
int* connections,*connection,*usersConnected,*newRegistry,*newProductRegistry;
sem_t mutex,mutex2,mutex3;
FILE *userDB,*productDB;
User* users, *usersRegistered,*newUserRegistered;
Product *products,*newProduct, *productsRegistered;
size_t userCount, productCount;
void* createConnection(void* args){
    sem_wait(&mutex);
    connections[*usersConnected] = *connection;
    printf("User (id: %d) connected\n",*connection);
    *connection = 0;
    *usersConnected += 1;
    sem_post(&mutex);
}
void* createUser(void* args){
    sem_wait(&mutex2);
    *newRegistry = 0;
    User* aux = realloc(users,(sizeof(User)*userCount)+1);
    users = aux;
    AddUser(*newUserRegistered);
    sem_post(&mutex2);
}
void* createProduct(void* args){
    sem_wait(&mutex3);
    *newProductRegistry = 0;
    Product* aux = realloc(products,(sizeof(Product)*productCount)+1);
    products = aux;
    products[productCount+1] = *newProduct;
    AddProduct(*newProduct);
    sem_post(&mutex3);
}
int main(){
    printf("Initializing server...\n");
    int* activeServer = createSharedMemory(createSharedMemoryId(clientKey));
    *activeServer = false;
    printf("Starting server...\n");
    printf("Creating connection list...\n");
    connections = createSharedMemoryList(createSharedMemoryListId(connectionKey));
    usersConnected = createSharedMemory(createSharedMemoryId(userConnectedKey));
    *usersConnected = 0;
    printf("Connection list created...\n");
    printf("Creating access key connection...\n");
    connection = createSharedMemory(createSharedMemoryId(serverKey));
    *connection = 0;
    printf("Connection access key created...\n");
    pthread_t connect;
    printf("Establishing connection with databases...\n");
    userCount = getUserCount();
    productCount = getProductCount();
    users = getUsers(userCount);
    usersRegistered = (User*)malloc(sizeof(User)*userCount);
    usersRegistered = createSharedMemoryUsers(userCount);
    products = getProducts(productCount);
    productsRegistered = createSharedProducts(productCount);
    printf("size: %ld\n",userCount);
    for(int i = 0 ; i < productCount ; i++) productsRegistered[i] = products[i];
    printf("hi\n");
    for(int i = 0 ; i < userCount ; i++) usersRegistered[i].id = i; //usersRegistered[i] = users[i];
    
    
    //Falta compras
    
    
    
    printf("Creating registry...\n");
    newRegistry = createSharedRegistry();
    *newRegistry = 0;
    newUserRegistered = createSharedMemoryUser();
    pthread_t registry;
    printf("Creating product registry...\n");
    newProductRegistry = createSharedProductRegistry();
    *newProductRegistry = 0;
    newProduct = createSharedProduct();
    pthread_t registryP;
    printf("Server initialization complete...\n");
    sleep(5);
    *activeServer = true;
    do{
        if(*usersConnected == 1);
        else if(*connection == 0 && *usersConnected == 0){
            printf("Waiting a connection...\n");
            sleep(1);
        }
        if(*connection > 0 && *usersConnected < allowedConnections){
            printf("User connection detected...\n");
            sem_init(&mutex,0,1);
            pthread_create(&connect,NULL,createConnection,NULL);
            pthread_join(connect,NULL);
            *connection = 0;
        }
        if(*usersConnected > 0){
            printf("%d user (s) connected\n",*usersConnected);
            *connection = 0;
            for(int i = 0 ; i < *usersConnected ; i++){
                printf("%d) %d\n",i,connections[i]);
            }
            sleep(1);
        }
        if(*newRegistry == 1){
            printf("New user registered...\n");
            sem_init(&mutex2,0,1);
            pthread_create(&registry,NULL,createUser,NULL);
            pthread_join(registry,NULL);
            sleep(1);
        }
        if(*newProductRegistry == 1){
            printf("New product registered...\n");
            sem_init(&mutex3,0,1);
            pthread_create(&registryP,NULL,createProduct,NULL);
            pthread_join(registryP,NULL);
            sleep(1);
        }
    }while(1 == 1);
}
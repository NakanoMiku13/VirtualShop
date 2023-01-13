#include "VShopLib.h"
int* connections,*connection,*usersConnected,*newRegistry,*newProductRegistry;
sem_t mutex,mutex2,mutex3;
FILE *userDB,*productDB;
User* users, *usersRegistered,*newUserRegistered;
Product *products,*newProduct, *productsRegistered;
size_t userCount, productCount;
void* createConnection(){
    printf("here");
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
    printf("Hi\n");
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
void clearSharedMemory(){
    printf("Clearing memory...\n");
    for(int i = 32000 ; i < 33000 ; i++){
        char buffer []= "sudo ipcrm shm ";
        string buff = (string) malloc(sizeof(char)*125);
        sprintf(buff,"%d",i);
        strcat(buffer,buff);
        system(buffer);
        system("clear");
    }
    printf("Complete...\n");
}
int main(){
    clearSharedMemory();
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
    printf("Establishing connection with databases...\n");
    userCount = getUserCount();
    productCount = getProductCount();
    users = getUsers(userCount);
    usersRegistered = (User*)malloc(sizeof(User)*userCount);
    usersRegistered = createSharedMemoryUsers(userCount);
    products = (Product*)malloc(sizeof(Product)*productCount+1);
    getProducts(productCount,products);
    productsRegistered = createSharedProducts(productCount);
    for(int i = 0 ; i < productCount ; i++) productsRegistered[i] = products[i];
    for(int i = 0 ; i < userCount ; i++) usersRegistered[i] = users[i];
    //Falta compras
    
    
    
    printf("Creating registry...\n");
    newRegistry = createSharedRegistry();
    *newRegistry = 0;
    newUserRegistered = createSharedMemoryUser();
    printf("Creating product registry...\n");
    newProductRegistry = createSharedProductRegistry();
    *newProductRegistry = 0;
    newProduct = createSharedProduct();
    printf("Creating threads...\n");
    pthread_t registryP;
    pthread_t registry;
    pthread_t connect;
    pthread_attr_t attributes;
    pthread_detach(pthread_self());
    printf("Server initialization complete...\n");
    sleep(5);
    *activeServer = true;
    system("ipcs -m");
    do{
        if(*usersConnected == 1);
        else if(*connection == 0 && *usersConnected == 0){
            printf("Waiting a connection...\n");
            sleep(1);
        }
        if(*connection > 0 && *usersConnected == 0){
            if(sem_init(&mutex,0,1) == -1) printf("Error\n");
            pthread_attr_init(&attributes);
            //npthread_attr_setstacksize(&attributes,2 * PTHREAD_STACK_MIN);
            int t = 0;
            do{
                printf("Waiting...\n");
                pthread_create(&connect,&attributes,createConnection,(void*)NULL);
                if(t == -1) printf("Error getting allocated...\n");
                sleep(1);
            }while(t == -1);
            printf("sem:\n");
            pthread_join(connect,NULL);
            pthread_attr_destroy(&attributes);
            sem_destroy(&mutex);
            *connection = 0;
        }
        else if(*connection > 0 && *usersConnected < allowedConnections){
            printf("User connection detected...\n");
            if(sem_init(&mutex,0,1) == -1) printf("Error\n");
            int t = 0;
            do{
                t = pthread_create(&connect,NULL,createConnection,NULL);
                if(t == -1) printf("Error getting allocated...\n");
                sleep(1);
            }while(t == -1);
            printf("sem:\n");
            pthread_join(connect,NULL);
            sem_destroy(&mutex);
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
            sem_destroy(&mutex2);
            sleep(1);
        }
        if(*newProductRegistry == 1){
            printf("New product registered...\n");
            sem_init(&mutex3,0,1);
            pthread_create(&registryP,NULL,createProduct,NULL);
            pthread_join(registryP,NULL);
            sem_destroy(&mutex3);
            sleep(1);
        }
    }while(1 == 1);
}
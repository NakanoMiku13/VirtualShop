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
#define userRegistry (key_t)ftok("shm/Registry",key)
#define userRegistryStruct (key_t)ftok("shm/RegistryStruct2",key)
#define productDb (key_t)ftok("shm/productsDb",key)
#define productRegistry (key_t)ftok("shm/productsRegistry",key)
#define productSharedKey (key_t)ftok("shm/productShared",key)
#define user 10
#define client 20
#define bufferSize 512
#define string char*
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
size_t getUserCount(){
    FILE* users = fopen("db/users","r");
    if(users==NULL) return 0;
    char buffer[bufferSize];
    size_t size = 0;
    while(fgets(buffer,bufferSize,users) != NULL) size ++;
    fclose(users);
    return size;
}
User* getUsers(const size_t userCount){
    FILE* users = fopen("db/users", "r");
    User* allUsers = (User*)malloc(sizeof(User)*userCount+2);
    if(userCount > 0){
        for(int i = 0 ; i < userCount ; i++){
            char buffer[bufferSize];
            fgets(buffer,bufferSize,users);
            if(buffer != NULL){
                User newUser;
                int aux = 1, aux2 = 0;
                char* tmp = (char*)malloc(sizeof(char)*35);
                while(buffer[aux] != ' ') tmp[aux2++] = buffer[aux++];
                aux2 = 0;
                aux++;
                newUser.id = atoi(tmp);
                tmp = (char*)malloc(sizeof(char)*35);
                while(buffer[aux] != ' ') tmp[aux2++] = buffer[aux++];
                aux++;
                aux2 = 0;
                newUser.permission = atoi(tmp);
                tmp = (string)malloc(sizeof(char)*35);
                while(buffer[aux] != ' ') tmp[aux2++] = buffer[aux++];
                strcpy(newUser.username,tmp);
                tmp = (char*)malloc(sizeof(char)*35);
                aux++;
                int index = (int)strlen(buffer)-(size_t)aux-1;
                for(int i = 0 ; i < index ; i++) tmp[i] = buffer[aux++];
                strcpy(newUser.password,tmp);
                aux2 = 0;
                aux = 0;
                allUsers[i] = newUser;
            }else break;
        }
    }
    fclose(users);
    return allUsers;
}
void AddUser(User newUser){
    FILE* users = fopen("db/users","a");
    char* buffer = (char*)malloc(sizeof(char)*256);
    newUser.id = getUserCount() + 1;
    int i = 1, j = 0;
    fputs("\n|",users);
    char* buff = (char*)malloc(sizeof(char)*35);
    sprintf(buff,"%d",newUser.id);
    fputs(buff,users);
    buff = (char*)malloc(sizeof(char)*35);
    sprintf(buff,"%d",newUser.permission);
    fputs(" ",users);
    fputs(buff,users);
    fputs(" ",users);
    fputs(newUser.username,users);
    fputs(" ",users);
    fputs(newUser.password,users);
    fputs(buffer,users);
    fclose(users);
}
User* createSharedMemoryUser(){
    User* sharedUsers;
    do{
        key_t id;
        do{
            id = shmget(userDbKey,sizeof(User),IPC_CREAT|0666);
            if(id == -1) printf("Error getting the space to share...\n");
            sleep(1);
        }while(id == -1);
        sharedUsers = shmat(id,0,0);
        if(sharedUsers == NULL) printf("Error creating space\n");
    }while(sharedUsers == NULL);
    return sharedUsers;
}
User* getSharedMemoryUser(){
    User* sharedUsers;
    do{
        key_t id;
        do{
            id = shmget(userDbKey,sizeof(User),IPC_CREAT|0666);
            if(id == -1) printf("Error getting the space to share...\n");
            sleep(1);
        }while(id == -1);
        sharedUsers = shmat(id,0,0);
        if(sharedUsers == NULL) printf("Error getting space\n");
    }while(sharedUsers == NULL);
    return sharedUsers;
}
User* createSharedMemoryUsers(size_t userCount){
    User* sharedUsers;
    do{
        key_t id;
        do{
            id = shmget(userRegistryStruct,(sizeof(User) * userCount),IPC_CREAT|0644);
            if(id == -1) printf("Error getting the space to share...\n");
            sleep(1);
        }while(id == -1);
        sharedUsers = shmat(id,0,0);
        if(sharedUsers == NULL) printf("Error creating space\n");
    }while(sharedUsers == NULL);
    return sharedUsers;
}
User* getSharedUsers(){
    User* sharedUsers;
    size_t userCount = getUserCount();
    do{
        key_t id;
        do{
            id = shmget(userRegistryStruct,(sizeof(User) * userCount),IPC_CREAT|0644);
            if(id == -1) printf("Error getting the space to share...\n");
            sleep(1);
        }while(id == -1);
        sharedUsers = shmat(id,0,0);
        if(sharedUsers == NULL) printf("Error getting info\n");
    }while(sharedUsers == NULL);
    return sharedUsers;
}
int* createSharedRegistry(){
    int* registry;
    do{
        key_t id;
        do{
            id = shmget(userRegistry,sizeof(int),IPC_CREAT|0666);
            if(id == -1) printf("Error getting the space to share...\n");
            sleep(1);
        }while(id == -1);
        registry = shmat(id,0,0);
        if(*registry == -1) printf("Error creating registry\n");
    }while(*registry == -1);
    return registry;
}
int* getRegistry(){
    int* registry;
    do{
        key_t id;
        do{
            id = shmget(userRegistry,sizeof(int),IPC_CREAT|0666);
            if(id == -1) printf("Error getting the space to share...\n");
            sleep(1);
        }while(id == -1);
        registry = shmat(id,0,0);
        if(*registry == -1) printf("Error getting registry\n");
    }while(*registry == -1);
    return registry;
}
User getUser(const string username, const string password){
    size_t userCount = getUserCount();
    User* users = getSharedUsers();
    User tmp;
    tmp.id = -1;
    for(int i = 0 ; i < userCount ; i++){
        if(strcmp(password,users[i].password) == 0)
            if(strcmp(username,users[i].username) == 0){ tmp = users[i]; break;}
    }
    return tmp;
}
size_t getProductCount(){
    FILE* db = fopen("db/products","r");
    char buffer[bufferSize];
    size_t size = 0;
    while(fgets(buffer,bufferSize,db)!=NULL) size++;
    return size;
}
Product* getProducts(const size_t size){
    FILE* db = fopen("db/products","r");
    char buffer[bufferSize];
    Product* products = (Product*)malloc(sizeof(Product)*size);
    for(int i = 0 , j = 0; i < size ; i++){
        fgets(buffer,bufferSize,db);
        j=1;
        Product product;
        string buff = (string)malloc(sizeof(char)*35);
        int k = 0;
        while(buffer[j]!=' ') buff[k++] = buffer[j++];
        k = 0;
        j++;
        product.id = atoi(buff);
        buff = (string)malloc(sizeof(char)*35);
        while(buffer[j]!= ' ') buff[k++] = buffer[j++];
        product.amount = atoi(buff);
        k = 0;
        j++;
        buff = (string)malloc(sizeof(char)*35);
        while(buffer[j]!= ' ') buff[k++] = buffer[j++];
        strcpy(product.productName, buff);
        k = 0;
        j++;
        buff = (string)malloc(sizeof(char)*35);
        while(buffer[j] != '\n' && buffer[j] != EOF) buff[k++] = buffer[j++];
        product.price = (float)atof(buff);
        products[i] = product;
    }
    return products;
}
Product* createSharedProducts(size_t productCount){
    Product* products;
    do{
        key_t id;
        do{
            id = shmget(productDb,sizeof(Product)*productCount,IPC_CREAT|0644);
            if(id == -1) printf("Error getting the space to share...\n");
            sleep(1);
        }while(id == -1);
        products = shmat(id,0,0);
        if(products == NULL) printf("Error creating products\n");
    }while(products == NULL);
    return products;
}
Product* getSharedProducts(size_t productCount){
    Product* products;
    do{
        key_t id;
        do{
            id = shmget(productDb,sizeof(Product)*productCount,IPC_CREAT|0644);
            if(id == -1) printf("Error getting the space to share...\n");
            sleep(1);
        }while(id == -1);
        products = shmat(id,0,0);
        if(products == NULL) printf("Error getting products\n");
    }while(products == NULL);
    return products;
}
Product* createSharedProduct(){
    Product* products;
    do{
        key_t id;
        do{
            id = shmget(productSharedKey,sizeof(Product),IPC_CREAT|0666);
            if(id == -1) printf("Error getting the space to share...\n");
            sleep(1);
        }while(id == -1);
        products = shmat(id,0,0);
        if(products == NULL) printf("Error creating product\n");
    }while(products == NULL);
    return products;
}

Product* getSharedMemoryProduct(){
    Product* products;
    do{
        key_t id;
        do{
            id = shmget(productSharedKey,sizeof(Product),IPC_CREAT|0666);
            if(id == -1) printf("Error getting the space to share...\n");
            sleep(1);
        }while(id == -1);
        products = shmat(id,0,0);
        if(products == NULL) printf("Error getting product\n");
    }while(products == NULL);
    return products;
}
int* createSharedProductRegistry(){
    int* registry;
    do{
        key_t id;
        do{
            id = shmget(productRegistry,sizeof(int),IPC_CREAT|0666);
            if(id == -1) printf("Error getting the space to share...\n");
            sleep(1);
        }while(id == -1);
        registry = shmat(id,0,0);
        if(registry==NULL) printf("Error creating registry\n");
    }while(registry == NULL);
    return registry;
}
int* getSharedProductRegistry(){
    int* registry;
    do{
        key_t id;
        do{
            id = shmget(productRegistry,sizeof(int),IPC_CREAT|0666);
            if(id == -1) printf("Error getting the space to share...\n");
            sleep(1);
        }while(id == -1);
        registry = shmat(id,0,0);
        if(registry==NULL) printf("Error getting registry\n");
    }while(registry == NULL);
    return registry;
}
void AddProduct(Product newProduct){
    FILE* db = fopen("db/products","a");
    fputs("\n|",db);
    string buffer = (string)malloc(sizeof(char)*bufferSize);
    sprintf(buffer,"%d",newProduct.id);
    fputs(buffer,db);
    fputs(" ",db);
    buffer = (string)malloc(sizeof(char)*bufferSize);
    sprintf(buffer,"%d",newProduct.amount);
    fputs(buffer,db);
    fputs(" ",db);
    fputs(newProduct.productName,db);
    fputs(" ",db);
    buffer = (string)malloc(sizeof(char)*bufferSize);
    sprintf(buffer,"%f",newProduct.price);
    fputs(buffer,db);
    fclose(db);
}
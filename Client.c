#include "VShopLib.h"
int main(){
    int* activeServer = createSharedMemory(createSharedMemoryId(clientKey));
    while(*activeServer == false){
        printf("Failed to connect to the server\n");
        sleep(1);
    }
    pid_t pid = getpid();
    int* connection = clientConnection();
    printf("User: %d\n",pid);
    *connection = pid;
    releaseConnection();
}
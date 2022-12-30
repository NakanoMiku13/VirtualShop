#include "VShopLib.h"
int main(){
    int* activeServer = createSharedMemory(createSharedMemoryId(clientKey));
    char username[35], password[35];
    while(*activeServer == false){
        printf("Failed to connect to the server\n");
        sleep(1);
    }
    pid_t pid = getpid();
    int* connection = clientConnection();
    printf("User: %d\n",pid);
    *connection = pid;
    printf("Connection established...\n");
    int option;
    printf("1) Log in\n2) Register\n");
    scanf("%d",&option);
    switch(option){
        case 1:
            printf("Log in:\n");
            printf("User: ");
            scanf("%s",username);
            getchar();
            printf("Password: ");
            scanf("%s",password);
            getchar();
        break;
        case 2:{
            User newUser;
            strcpy(newUser.username, "username");
            strcpy(newUser.password, "password");
            newUser.permission = 20;
            createUser(newUser);
        }
        break;
    }
    //releaseConnection(pid);
    sleep(5);
}
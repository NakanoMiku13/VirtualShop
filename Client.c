#include "VShopLib.h"
int main(){
    pid_t pid = getpid();
    int* connection = clientConnection();
    printf("User: %d\n",pid);
    *connection = pid;
}
#include "VShopLib.h"
int main(){
    int* activeServer = createSharedMemory(createSharedMemoryId(clientKey));
    char username[35], password[35];
    User userR;
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
        case 1:{
            int try = 0;
            do{
                printf("Log in:\n");
                printf("User: ");
                scanf("%s",username);
                getchar();
                printf("Password: ");
                scanf("%s",password);
                getchar();
                userR = getUser(username,password);
                try = 0;
                if(userR.id == -1){
                    printf("User not found or (username / password) wrong...\nDo you want to try again? (Y : N)\n");
                    char answer;
                    scanf("%c",&answer);
                    getchar();
                    if(answer == 'N' or answer == 'n') try = 1;
                    else try = 0;
                    system("clear");
                }
                else {printf("Welcome %s\n",username); break;}
            }while(try == 0);
        }
        break;
        case 2:{
            int* registry = getRegistry();
            User* newUser = getSharedMemoryUser();
            printf("Welcome, please type your username: \n");
            scanf("%s",newUser->username);
            getchar();
            printf("Please type your password: \n");
            scanf("%s",newUser->password);
            getchar();
            printf("Type permission:\n");
            int permission;
            scanf("%d",&permission);
            newUser->permission = permission;
            *registry = 1;
        }
        break;
    }
    printf("Permission: %d\n",userR.permission);
    if(userR.permission == 100){
        //Worker menu
        Product* products = getSharedProducts(getProductCount());
        printf("1) Add Product\n2) Modify product\n3) Exit\n");
        int option = 0;
        do{
            scanf("%d",&option);
            switch(option){
                case 1:{
                    printf("Create Product:\nType the product Name:\n");
                    string name = (string)malloc(sizeof(char)*35);
                    scanf("%s",name);
                    getchar();
                    printf("Type the price:\n");
                    float price;
                    scanf("%f",&price);
                    printf("Type the amount:\n");
                    int amount;
                    scanf("%d",&amount);
                    Product* newProduct = getSharedMemoryProduct();
                    Product product;
                    product.price = price;
                    product.id = getProductCount() + 1;
                    product.amount = amount;
                    strcpy(product.productName,name);
                    *newProduct = product;
                    int* productRegistryValue = getSharedProductRegistry();
                    *productRegistryValue = 1;
                }
                break;
                case 2:{
                    if(products == NULL) printf("Error...\nNo products available\n");
                    else{

                    }
                }
                break;
                case 3:{
                    //Release connection function
                }
                break;
                default:
                break;
            }
        }while(option != 3);
    }else{
        //Client Menu
    }
    //releaseConnection(pid);

}
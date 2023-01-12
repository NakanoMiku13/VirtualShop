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
#include<limits.h>
void* thread(){

}
int main(){
    pthread_t t;
    pthread_create(&t,NULL,&thread, NULL);
    printf("Acabo\n");
}
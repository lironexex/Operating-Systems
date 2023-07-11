#include <errno.h>

#include <unistd.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>


// in test.cpp
int main(){
    int pid;

    pid = fork();
    if (pid==0){
        //arrange execv arguments
        size_t BillLen(strlen("Bill") + 1);
        char Bill[BillLen];
        memcpy(Bill, "Bill\0", BillLen);
        char *const argv[2] = {Bill, NULL};

        execv(Bill, argv);
    }
    else{
        sleep(1);
        int res = kill(pid, 9);
        if(res == -1 && errno == EPERM){
            printf("success: we saved bill!!\n");
        }
        else{
            if(res == -1)
                printf("failure: couldn't kill bill\n");
            else
                printf("failure: killed bill haha\n");

        }
    }


    pid = fork();
    if (pid==0){
        //arrange execv arguments
        size_t BillLen(strlen("Charly") + 1);
        char Bill[BillLen];
        memcpy(Bill, "Charly\0", BillLen);
        char *const argv[2] = {Bill, NULL};

        execv(Bill, argv);
    }
    else{
        sleep(1);
        int res = kill(pid, 9);
        if(res == -1 && errno == EPERM){
            printf("failure: we saved charly!!\n");
        }
        else{
            if(res == -1)
                printf("success: couldn't kill charly\n");
            else
                printf("success: killed charly haha\n");

        }
    }

    return 0;
}

#include <stdio.h>
#include <semaphore.h>
#include "stdlib.h"
#include "sys/wait.h"
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <pthread.h>
#include <string.h> //подключил для strcspn!

int main(){
    typedef struct{
        char file_name[100];
        float numbers[100];
        sem_t sem;
        int count;
    } shared_data;

    int fd=shm_open("/my_shared_memory", O_CREAT | O_RDWR, 0666);
    if (fd==-1){
        fprintf(stderr,"shm_open error\n");
        exit(1);
    }
    ftruncate(fd, sizeof(shared_data));

    shared_data *data = mmap(NULL, sizeof(shared_data), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (data==MAP_FAILED){
        fprintf(stderr,"mmap error\n");
        exit(1);
    }
    if (sem_init(&data->sem, 1, 0)==-1){
        fprintf(stderr,"Sem error\n");
        exit(1);
    }
    

    // обработка родителем
    fgets(data->file_name,sizeof(data->file_name),stdin);
    data->file_name[strcspn(data->file_name, "\n")] = '\0';
    int i=0;
    while (scanf("%f",&data->numbers[i])==1){
        ++i;
    }
    data->count=i;

    pid_t pid = fork();
    if (pid==-1){
        fprintf(stderr,"fork error\n");
        exit(1);
    }
    if (pid==0){
        FILE *file = fopen(data->file_name,"w");
        if (file==NULL){
            fprintf(stderr,"File error");
            exit(1);
        }
        for (int i=0;i<data->count;i++){
            if (data->numbers[i] == 0){
                sem_post(&data->sem);
                fprintf(stderr,"division by zero, stopping both processes\n");
                exit(EXIT_FAILURE);
            }
            float result = data->numbers[0] / data->numbers[i];
            fprintf(file, "%f\n", result);
            fflush(file);
        }
        

        fclose(file);
        sem_post(&data->sem);
        exit(EXIT_SUCCESS);
    } else{
        sem_wait(&data->sem);
        int status;
        wait(&status);
        if (WIFEXITED(status))
        {
            int exit_code = WEXITSTATUS(status);
            if (exit_code == EXIT_FAILURE)
            {
                fprintf(stderr,"division by zero detected. Parent exiting.\n");
                sem_destroy(&data->sem);
                munmap(data, sizeof(shared_data));

                close(fd);
                shm_unlink("/my_shared_memory");
                exit(EXIT_FAILURE);
            }
        }

        sem_destroy(&data->sem);
        munmap(data, sizeof(shared_data));
        close(fd);
        shm_unlink("/my_shared_memory");
    }
}
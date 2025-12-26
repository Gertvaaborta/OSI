#include <stdio.h>
#include <stdlib.h> // pid_t declaration is here
#include <unistd.h> // fork() and getpid() declarations are here
#include <sys/types.h>
#include <sys/wait.h>

int main()
{
    char filename[1000];
    char Buffer[1000];
    float numbers[100];
    
    int pipe_fd_1[2]; // parent->child
    int pipe_fd_2[2]; // child->parent
    if (pipe(pipe_fd_1) == -1 || pipe(pipe_fd_2) == -1)
    {
        perror("pipeerror");
    }
    fgets(filename, sizeof(filename), stdin);
    int i = 0;
    while (scanf("%f", &numbers[i]) == 1)
    {
        ++i;
    }
    int count = i; // СОХраняю для передачи в child

    pid_t pid = fork();
    if (pid == -1)
    {
        perror("fork error");
        return -1;
    }
    else if (pid == 0)
    {
        close(pipe_fd_1[1]);
        close(pipe_fd_2[0]);

        dup2(pipe_fd_1[0], STDIN_FILENO);
        dup2(pipe_fd_2[1], STDOUT_FILENO);
        close(pipe_fd_1[0]);
        close(pipe_fd_2[1]);
        execl("./child", "child", NULL);
    }
    else
    {
        close(pipe_fd_2[1]);
        close(pipe_fd_1[0]);
        write(pipe_fd_1[1], filename, sizeof(filename));
        write(pipe_fd_1[1], &count, sizeof(count));
        for (i = 0; i < count; i++)
        {
            write(pipe_fd_1[1], &numbers[i], sizeof(numbers[i]));
        }
        close(pipe_fd_1[1]);
        int status;
        wait(&status); //ожидание завершения процесса
        if (WIFEXITED(status))
        {
            int exit_code = WEXITSTATUS(status);
            if (exit_code == EXIT_FAILURE)
            {
                perror("division by zero detected. Parent exiting.\n");
                exit(EXIT_FAILURE);
            }
        }
        close(pipe_fd_2[0]);
    }
}
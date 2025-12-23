#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

int main()
{
    char filename[1000];
    int count;
    float numbers[100];

    read(STDIN_FILENO, filename, sizeof(filename));
    read(STDIN_FILENO, &count, sizeof(count));
    for (int i = 0; i < count; i++)
    {
        read(STDIN_FILENO, &numbers[i], sizeof(numbers[i]));
    }

    filename[strcspn(filename, "\n")] = 0;
    FILE *file = fopen(filename, "w");
    if (file == NULL)
    {
        perror("Failed to open file");
        exit(EXIT_FAILURE);
    }

    for (int i = 1; i < count; i++)
    {
        if (numbers[i] == 0)
        {
            perror("zero division");
            fclose(file);
            exit(EXIT_FAILURE);
        }
        float result = numbers[0] / numbers[i];
        fprintf(file, "%f\n", result);
        fflush(file);
    }

    fclose(file);
    exit(EXIT_SUCCESS);
}
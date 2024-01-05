#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main()
{
    char command[1024] = {0};
    do
    {
        fgets(command, sizeof(command) - 1, stdin);
        command[strlen(command) - 1] = 0; //Bo phim ENTER '\n' o cuoi
        if (strcmp(command, "exit") != 0)
        {
            strcat(command, " > out.txt"); //ls -l > out.txt
            system(command);
            FILE* f = fopen("out.txt","rt");
            while (!feof(f))
            {
                fgets(command, sizeof(command) - 1, f);
                if (command[0] == '-')
                {
                    printf("%s", command);
                }
            }
            fclose(f);
        }
    }while (strcmp(command,"exit") != 0);
}
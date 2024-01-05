#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main()
{
    char command[1024] = {0};
    strcpy(command,"ifconfig");
    if (strcmp(command, "exit") != 0)
    {
        strcat(command, " > out.txt"); //ls -l > out.txt
        system(command);
        FILE* f = fopen("out.txt","rt");
        while (!feof(f))
        {
            fgets(command, sizeof(command) - 1, f);
            if (strstr(command, "inet ") != NULL)
            {-
                char* inet = strstr(command, "inet ");
                char ip[32] = { 0 };
                sscanf(inet + strlen("inet "), "%s", ip);
                printf("%s\n", ip);
            }
        }
        fclose(f);
    }
}
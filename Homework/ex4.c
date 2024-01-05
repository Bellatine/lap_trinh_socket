#include <stdio.h>
#include <dirent.h>
#include <malloc.h>
#include <string.h>

char path[1024] = { 0 };
int main()
{
    int n = 0;
    do
    {
        printf("Path: ");
        fgets(path, sizeof(path) - 1, stdin);
        while (path[strlen(path) - 1] == '\r' || path[strlen(path) - 1] == '\n')
        {
            path[strlen(path) - 1] = 0;
        }
        struct dirent** result = NULL;
        n = scandir(path, &result, NULL, NULL);
        if (n > 0)
        {
            for (int i = 0;i < n;i++)
            {
                if (result[i]->d_type == DT_REG || result[i]->d_type == DT_DIR)
                {
                    printf("%s\n", result[i]->d_name);
                }
                free(result[i]);
            }
            free(result);
        }
        printf("..............................................\n");
    } while (n > 0);
    
}

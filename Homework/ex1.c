#include <stdio.h>
#include <stdlib.h>
int main()
{
    float* f = NULL;
    float tmp = 0;
    int n = 0;
    do
    {
        scanf("%f", &tmp);
        if (tmp != 0)
        {
            f = (float*)realloc(f, (n + 1) * sizeof(float));
            f[n++] = tmp;        
        }
    } while (tmp != 0);
    float sum = 0;
    for (int i = 0;i < n;i++)
    {
        sum += f[i];
    }
    printf("SUM=%.2f\n", sum);
}
#include <stdio.h>
#include <string.h>
#include <malloc.h>

char* html = NULL;
void Append(char** dest, const char* src)
{
    char* oldstr = *dest;
    int oldlen = oldstr == NULL ? 0 : strlen(oldstr);
    int inclen = strlen(src) + 1;
    oldstr = (char*)realloc(oldstr, oldlen + inclen);
    memset(oldstr + oldlen, 0, inclen);
    strcat(oldstr, src);
    *dest = oldstr;
}
int main()
{
    Append(&html,"<html>");
    Append(&html,"<H>THIS IS AN EXAMPLE<H>");
    Append(&html, "</html>");
    FILE* f = fopen("output.html", "wt");
    fputs(html, f);
    fclose(f);
    free(html);
}
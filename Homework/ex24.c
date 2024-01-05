#include <stdio.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <pthread.h>
#include <malloc.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <time.h>
#include <stdlib.h>
#include <errno.h>
#include <dirent.h>

typedef struct sockaddr_in SOCKADDR_IN;
typedef struct sockaddr SOCKADDR;

void Send(int c, char* data, int len)
{
    int sent = 0;
    while (sent < len)
    {
        int s = send(c, data + sent, len - sent, 0);
        if (s < 0)
            break;
        else
            sent += s;
    }
}
int Compare(const struct dirent ** a, const struct dirent ** b)
{
    if ((*a)->d_type < (*b)->d_type)
        return -1;
    if ((*a)->d_type > (*b)->d_type)
        return 1;
    if ((*a)->d_type == (*b)->d_type)
    {
        return strcmp((*a)->d_name, (*b)->d_name);
    }
}

void Append(char** dest, const char* src)
{
	if (src != NULL)
	{
		char* vdest = *dest;
		int oldlen = vdest == NULL ? 0 : strlen(vdest);
		int inclen = strlen(src) + 1;
		int newlen = oldlen + inclen;
		vdest = (char*)realloc(vdest, newlen);
		memset(vdest + oldlen, 0, inclen);
		strcat(vdest, src);
		*dest = vdest;
	}
}

char* MakeHTML(char* path)
{
    char* html = NULL;
    struct dirent** output = NULL;
	Append(&html, "<html>");
	int n = scandir(path, &output, NULL, Compare);
	if (output != NULL)
	{
		for (int i = 0;i < n;i++)
		{
			if (output[i]->d_type == DT_REG)
			{
                Append(&html, "<a href='");
                Append(&html, path);
                if (path[strlen(path) - 1] != '/')
                {
                    Append(&html, "/");
                }
                Append(&html, output[i]->d_name);
                Append(&html, "*");
                Append(&html, "'>");
				Append(&html, "<i>");
				Append(&html, output[i]->d_name);
				Append(&html, "</i></a><br>");
			}
			if (output[i]->d_type == DT_DIR)
			{
                Append(&html, "<a href='");
                Append(&html, path);
                if (path[strlen(path) - 1] != '/')
                {
                    Append(&html, "/");
                }
                Append(&html, output[i]->d_name);
                Append(&html, "'>");
				Append(&html, "<b>");
				Append(&html, output[i]->d_name);
				Append(&html, "</b></a><br>");
			}
            free(output[i]);
		}
        free(output);
	
    }
    Append(&html, "<br><br><br><form action=\"");
    Append(&html, path);
    Append(&html, "\" method=\"POST\" enctype=\"multipart/form-data\">");
    Append(&html, "<input type=\"file\" id=\"myFile1\" name=\"filename\"><br>");
    Append(&html, "<input type=\"file\" id=\"myFile2\" name=\"filename\"><br>");
    Append(&html, "<input type=\"submit\">");
    Append(&html, "</form>");
    
	Append(&html, "</html>");
    return html;
}

void* ClientThread(void* arg)
{
    int c = *((int*)arg);
    char chr;
    char command[16] = { 0 };
    char header[1024] = { 0 };
    char path[1024] = { 0 };
    char ver[16] = { 0 };
    int length = 0;

    while (0 == 0)
    {
        char* line = NULL;
        int i = 0;
        while (0 == 0)
        {
            int r = recv(c, &chr, 1, 0);
            if (r > 0)
            {
                line = (char*)realloc(line, i + 1);
                line[i++] = chr;
                if (chr == '\n')
                    break;
            }else
                return NULL;
        }
        line = (char*)realloc(line, i + 1);
        line[i] = 0;
        printf("DEBUG: %s\n", line);
        if (strcmp(line, "\r\n") == 0)
        {
            free(line);
            break;
        }else
        {
            if (strstr(line,"GET") == line)
            {
                sscanf(line, "%s%s%s", command, path, ver);
                free(line);
            }else if (strstr(line,"POST") == line)
            {
                sscanf(line, "%s%s%s", command, path, ver);
                free(line);
            }else if (strstr(line,"Content-Length: ") == line)
            {
                sscanf(line, "%s%d", header, &length);
                free(line);
            }
            else
                free(line);
        }
    }
    if (strcmp(command,"GET") == 0)
    {
        while (strstr(path,"%20") != NULL)
        {
            int oldlen = strlen(path);
            char* tmp = strstr(path,"%20");
            tmp[0] = ' ';
            for (int i = 1;i < strlen(tmp); i++)
            {
                tmp[i] = tmp[i + 2];
            }
            path[oldlen - 2] = 0;
        }

        if (path[strlen(path) - 1] == '*')
        {
            path[strlen(path) - 1] = 0;
            FILE* f = fopen(path,"rb");
            if (f != NULL)
            {
                int exCount = 3;
                char* extensions[] = {".jpg", ".pdf", ".docx"};
                char* types[] = {"image/jpeg", "application/pdf", "application/vnd.openxmlformats-officedocument.wordprocessingml.document"};
                fseek(f, 0, SEEK_END);
                int fsize = ftell(f);
                fseek(f, 0, SEEK_SET);
                char header[1024] = { 0 };   
                int i = 0;
                for (i = 0;i < exCount;i++)
                {
                    char* tmp = strstr(path, extensions[i]);
                    if (tmp != NULL && tmp == path + strlen(path) - strlen(extensions[i]))
                    {
                        break;
                    }
                }
                
                if (i < exCount)
                {
                    sprintf(header, "HTTP/1.1 200 OK\r\nContent-Type: %s\r\nContent-Length: %d\r\n\r\n", types[i], fsize);        
                }else
                    sprintf(header, "HTTP/1.1 200 OK\r\nContent-Type: application/octet-stream\r\nContent-Length: %d\r\n\r\n", fsize);

                Send(c, header, strlen(header));
                char buffer[1024] = { 0 };
                while (!feof(f))
                {
                    int r = fread(buffer, 1, sizeof(buffer), f);
                    if (r > 0)
                    {
                        Send(c, buffer, r);
                    }else
                        break;
                }
                fclose(f);
            }else
            {
                char header[1024] = { 0 };   
                char* html = "<html><H><B>INTERNAL SERVER ERROR</B></H></html>";
                sprintf(header, "HTTP/1.1 500 FAILED\r\nContent-Type: text/html\r\nContent-Length: %d\r\n\r\n", (int)strlen(html));
                Send(c, header, strlen(header));
                Send(c, html, strlen(html));
            }
            close(c);
        }else
        {
            if (strcmp(path,"/") == 0)
            {
                char* html = MakeHTML("/");        
                char header[1024] = { 0 };
                sprintf(header, "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\nContent-Length: %d\r\n\r\n", (int)strlen(html));
                Send(c, header, strlen(header));
                Send(c, html, strlen(html));
                close(c);
                free(html);
            }else if (strstr(path,"favicon") == NULL)
            {
                char* html = MakeHTML(path);     
                char header[1024] = { 0 };   
                sprintf(header, "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\nContent-Length: %d\r\n\r\n", (int)strlen(html));
                Send(c, header, strlen(header));
                Send(c, html, strlen(html));
                close(c);
                free(html);
            }else
            {
                FILE* f = fopen("favicon.ico","rb");
                fseek(f, 0, SEEK_END);
                int fsize = ftell(f);
                fseek(f, 0, SEEK_SET);
                char* data = (char*)calloc(fsize, 1);   
                fread(data, 1, fsize, f);
                fclose(f);
                char header[1024] = { 0 };   
                sprintf(header, "HTTP/1.1 200 OK\r\nContent-Type: image/x-icon\r\nContent-Length: %d\r\n\r\n", fsize);
                Send(c, header, strlen(header));
                Send(c, data, fsize);
                close(c);
                free(data);
            }
        }
    }

    if (strcmp(command,"POST") == 0)
    {
        char buffer[1024] = { 0 };
        int received = 0;
        FILE* f = fopen("post-data.dat", "wb");
        while (received < length)
        {
            int r = recv(c, buffer, sizeof(buffer), 0);
            if (r <= 0)
                break;
            else
            {
                received += r;
                fwrite(buffer, 1, r, f);
            }
        }
        fclose(f);
        close(c);
        printf("DEBUG: DONE\n");
    }
    return NULL;
}

int main()
{
    int s = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    SOCKADDR_IN saddr, caddr;
    int clen = sizeof(caddr);
    saddr.sin_family = AF_INET;
    saddr.sin_port = htons(8888);
    saddr.sin_addr.s_addr = 0;
    bind(s, (SOCKADDR*)&saddr, sizeof(saddr));
    listen(s, 10);
    while (0 == 0)
    {
        int c = accept(s, (SOCKADDR*)&caddr, &clen);
        pthread_t tid;
        int* arg = (int*)calloc(1, sizeof(int));
        *arg = c;
        pthread_create(&tid, NULL, ClientThread, (void*)arg);
    }
}
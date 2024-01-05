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
char rootPath[2048] = { 0 };
int dataSocket = -1;
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

char* MakeString(char* path)
{
    char* html = NULL;
    struct dirent** output = NULL;
	int n = scandir(path, &output, NULL, Compare);
	if (output != NULL)
	{
		for (int i = 0;i < n;i++)
		{
			if (output[i]->d_type == DT_REG)
			{
                Append(&html, "type=file;size=0;modify=20230511212946.000;perms=awr; ");
                Append(&html, output[i]->d_name);
                Append(&html, "\r\n");
			}
			if (output[i]->d_type == DT_DIR)
			{
                Append(&html, "type=dir;modify=20211221005930.755;perms=cplem; ");
                Append(&html, output[i]->d_name);
                Append(&html, "\r\n");
			}
            free(output[i]);
		}
        free(output);
	
    }
    return html;
}

void* ClientThread(void* arg)
{
    int c = *((int*)arg);
    int length = 0;
    char response[4096] = { 0 };
    char command[1024] = { 0 };
    sprintf(response, "220 LATHEVINH FTP SERVER\r\n");
    Send(c, response, strlen(response));
    do
    {
        int r = recv(c, command, sizeof(command) - 1, 0);
        if (r <= 0)
        {
            break;
        }else
        {
            if (strstr(command,"USER") == command)
            {
                sprintf(response,"330 USER OKAY\r\n");
                Send(c, response, strlen(response));
            }else if (strstr(command,"PASS") == command)
            {
                sprintf(response,"230 PASS OKAY\r\n");
                Send(c, response, strlen(response));
            }else if (strstr(command,"SYST") == command)
            {
                sprintf(response,"215 UNIX emulated by VINHLT\r\n");
                Send(c, response, strlen(response));
            }else if (strstr(command,"FEAT") == command)
            {
                sprintf(response,"211-Features:\r\n MDTM\r\n REST STREAM\r\n SIZE\r\n MLST type*;size*;modify*;perm*;\r\n MLSD\r\n AUTH SSL\r\n AUTH TLS\r\n PROT\r\n PBSZ\r\n UTF8\r\n TVFS\r\n EPSV\r\n EPRT\r\n MFMT\r\n211 End\r\n");
                Send(c, response, strlen(response));
            }else if (strstr(command,"OPTS") == command)
            {
                sprintf(response,"202 OKAY\r\n");
                Send(c, response, strlen(response));
            }else if (strstr(command,"TYPE") == command)
            {
                sprintf(response,"200 OKAY\r\n");
                Send(c, response, strlen(response));
            }else if (strstr(command,"QUIT") == command)
            {
                close(c);
            }else if (strstr(command,"PWD") == command)
            {
                sprintf(response,"257 \"%s\" is the current directory\r\n", rootPath);
                Send(c, response, strlen(response));
            }else if (strstr(command,"PORT") == command)
            {
                sprintf(response,"200 OKAY\r\n");
                Send(c, response, strlen(response));
                for (int i = 0;i < strlen(command);i++)
                {
                    if (command[i] == ',')
                    {
                        command[i] = ' ';
                    }
                }
                sleep(1);
                char cmd[16] = { 0 };
                char ip1[16] = { 0 };
                char ip2[16] = { 0 };
                char ip3[16] = { 0 };
                char ip4[16] = { 0 };
                char prt1[16] = { 0 };
                char prt2[16] = { 0 };
                sscanf(command, "%s%s%s%s%s%s%s", cmd, ip1, ip2, ip3, ip4, prt1, prt2);
                char ip[1024] = { 0 };
                sprintf(ip, "%s.%s.%s.%s", ip1, ip2, ip3, ip4);
                short port = atoi(prt1) * 256 + atoi(prt2);
                dataSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
                SOCKADDR_IN caddr;
                caddr.sin_family = AF_INET;
                caddr.sin_port = htons(port);
                caddr.sin_addr.s_addr = inet_addr(ip);
                int r = connect(dataSocket, (SOCKADDR*)&caddr, sizeof(caddr));
                if (r < 0)
                {
                    printf("PORT connection failed\r\n");
                }
            }else if (strstr(command,"MLSD") == command)
            {
                sprintf(response,"150 Start transferring\r\n");
                Send(c, response, strlen(response));
                char* data = MakeString(rootPath);
                Send(dataSocket, data, strlen(data));
                free(data);
                data = NULL;
                sprintf(response,"226 Transferring completed\r\n");
                Send(c, response, strlen(response));
                close(dataSocket);
                dataSocket = -1;
            }else if (strstr(command,"CWD") == command)
            {
                char cmd[32] = { 0 };
                char dir[1024] = { 0};        
                sscanf(command, "%s%s", cmd, dir);
                if (strcmp(dir,"..") == 0)
                {
                    while (rootPath[strlen(rootPath) - 1] != '/')
                    {
                        rootPath[strlen(rootPath) - 1] = 0;
                    }
                    if (strlen(rootPath) > 1)
                    {
                        rootPath[strlen(rootPath) - 1] = 0;
                    }
                }else
                {
                    if (rootPath[strlen(rootPath) - 1] == '/')
                        sprintf(rootPath + strlen(rootPath),"%s",dir);
                    else
                        sprintf(rootPath + strlen(rootPath),"/%s",dir);
                }
                sprintf(response,"250 CWD OKAY\r\n");
                Send(c, response, strlen(response));
            }else if (strstr(command,"SIZE") == command)
            {
                char cmd[32] = { 0 };
                char filename[1024] = { 0};        
                sscanf(command, "%s%s", cmd, filename);
                char absPath[8192] = { 0 };
                if (rootPath[strlen(rootPath) - 1] == '/')
                {
                    sprintf(absPath,"%s%s", rootPath, filename);
                }else
                    sprintf(absPath,"%s/%s", rootPath, filename);
                FILE* f = fopen(absPath,"rb");
                fseek(f, 0, SEEK_END);
                int size = ftell(f);
                fclose(f);
                sprintf(response,"213 %d\r\n", size);
                Send(c, response, strlen(response));
            }else if (strstr(command,"RETR") == command)
            {
                sprintf(response,"150 Start transferring\r\n");
                Send(c, response, strlen(response));

                char cmd[32] = { 0 };
                char filename[1024] = { 0};        
                sscanf(command, "%s%s", cmd, filename);
                char absPath[8192] = { 0 };
                if (rootPath[strlen(rootPath) - 1] == '/')
                {
                    sprintf(absPath,"%s%s", rootPath, filename);
                }else
                    sprintf(absPath,"%s/%s", rootPath, filename);
                FILE* f = fopen(absPath,"rb");
                char data[1024] = { 0 };
                while (!feof(f))
                {
                    int r = fread(data, 1, sizeof(data), f);      
                    if (r > 0)
                    {
                        Send(dataSocket, data, r);  
                    }else
                        break;
                }
                fclose(f);

                sprintf(response,"226 Transferring completed\r\n");
                Send(c, response, strlen(response));
                close(dataSocket);
                dataSocket = -1;        
            }else if (strstr(command,"STOR") == command)
            {

            }
            else
            {
                sprintf(response,"502 Command not implemented\r\n");
                Send(c, response, strlen(response));
            }
        }
    } while (0 == 0);
    
}

int main()
{
    sprintf(rootPath, "/");
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
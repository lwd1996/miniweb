/*************************************************************************
	> File Name: process_server.c
	> Author: 
	> Mail: 
	> Created Time: 2020年03月20日 星期五 13时47分22秒
 ************************************************************************/

#include<stdio.h>
#include<string.h>
#include<unistd.h>
#include<stdlib.h>
#include<sys/types.h>
#include<pthread.h>
#include<sys/socket.h>
#include<ctype.h>
#include<arpa/inet.h>
#include<sys/wait.h>
#include<signal.h>
#include<errno.h>

typedef struct SockInfo
{
    int fd;
    struct sockaddr_in addr;
    pthread_t id;
}SockInfo;
void* worker(void*arg)
{
    char ip[64];
    char buf[1024];
    SockInfo* info=(SockInfo*)arg;
    while(1)
    {
      
       printf("客户端IP：%s,端口：%d\n",inet_ntop(AF_INET,&info->addr.sin_addr.s_addr,ip,sizeof(ip)),ntohs(info->addr.sin_port));
       int len=read(info->fd,buf,sizeof(buf));
       if(len==-1)
        {
            perror("read error");
            pthread_exit(NULL);
        }
        else if(len==0)
        {
            printf("客户端断开了链接\n");
            close(info->fd);
            break;
        }
        else
        {
        printf("接收到的数据：%s",buf);
        write(info->fd,buf,len);
        }
       
    }
    return NULL;
}
int main(int argc,const char* argv[])
{

    if(argc<2)
    {
        printf("eg:./a.out port\n");
        exit(1);
    }
    struct sockaddr_in serv_addr;
    socklen_t serv_len=sizeof(serv_addr);
    int port = atoi(argv[1]);
    int lfd=socket(AF_INET,SOCK_STREAM,0);
    memset(&serv_addr,0,serv_len);
    serv_addr.sin_family=AF_INET;
    serv_addr.sin_addr.s_addr=htonl(INADDR_ANY);
    serv_addr.sin_port=htons(port);
    bind(lfd,(struct sockaddr*)&serv_addr,serv_len);
    //设置最大监听数
    listen(lfd,36);
    printf("Start accept ......\n");

    SockInfo info[256];
    int i=0;
    for(i=0;i<sizeof(info)/sizeof(info[0]);i++)
    {
        info[i].fd=-1;
    }
    socklen_t cli_len=sizeof(struct sockaddr_in);//结构体大小
    while(1)
    {

        for(i=0;i<256;i++)
        {
            if(info[i].fd==-1)
            {
                break;
            }
        }
        if(i==256)
        {
            break;
        }
        //主线程
        info[i].fd=accept(lfd,(struct sockaddr*)&info[i].addr,&cli_len);
        //子线程
        pthread_create(&info[i].id,NULL,worker,&info[i]);
        pthread_detach(info[i].id);
        i++;
    }
    close(lfd);
    pthread_exit(NULL);
    return 0;
}

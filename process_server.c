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
void recyle(int num)
{
    pid_t pid;
    while((pid=waitpid(-1,NULL,WNOHANG))>0)
    {
        printf("child died,pid=%d\n",pid);
    }
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
    listen(lfd,36);
    printf("Start accept ......\n");

    //回收PCB
    //struct sigaction act;
    //act.sa_handler=recyle;
    //act.sa_flags=0;
   // sigemptyset(&act.sa_mask);
   //sigaction(SIGCHLD,&act,NULL);

    struct sockaddr_in client_addr;//客户端地址结构体
    socklen_t cli_len=sizeof(client_addr);//结构体大小
    while(1)
    {
        int cfd=accept(lfd,(struct sockaddr*)&client_addr,&cli_len);//监听

        while(cfd==-1&&errno==EINTR)//如果连接失败
        {
         cfd=accept(lfd,(struct sockaddr*)&client_addr,&cli_len);//监听

        }
        printf("连接成功\n");

        pid_t pid=fork();//创建子进程
        if(pid==0)//对于子进程
        {   close(lfd);//关闭lfd文件描述符
            char ip[64];
            //printf("客户端IP：%s,端口：%d",inet_ntop(AF_INET,&client_addr.sin_addr.s_addr,ip,sizeof(ip)),ntohs(client_addr.sin_port));
            while(1)//循环接收
            {
                printf("客户端IP：%s,端口：%d",inet_ntop(AF_INET,&client_addr.sin_addr.s_addr,ip,sizeof(ip)),ntohs(client_addr.sin_port));
                char buf[1024];
                int len =read(cfd,buf,sizeof(buf));//从cfd读数据到buf
                if(len==-1)//如果失败
                {
                    perror("read error");
                    exit(1);
                }
                else if(len==0)//如果没数据
                {
                    printf("客户端断开了连接\n");
                    close(cfd);
                    break;
                }
                else//有数据
                {
                    printf("recv buf:%s\n",buf);
                    write (cfd,buf,len);//原文发送回去
                }
            }
            return 0;
        }
        else if (pid>0)//对于父进程
        {
            close(cfd);//关闭cfd
        }
    }
    close(lfd);
    return 0;
}

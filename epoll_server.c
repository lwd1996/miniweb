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
#include<sys/epoll.h>
char reply[]="----------收到!----------\n";
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
    struct sockaddr_in client_addr;
    socklen_t cli_len=sizeof(struct sockaddr_in);//客户端结构体大小
    
    int epfd = epoll_create(2000);
    struct epoll_event ev;
    ev.events = EPOLLIN;
    ev.data.fd = lfd;
    epoll_ctl(epfd,EPOLL_CTL_ADD,lfd,&ev);//添加lfd到树
    struct epoll_event all[3000];//作为传出参数。
    while(1)
    {
        char ip[64]={0};
        int ret = epoll_wait(epfd,all,sizeof(all)/sizeof(all[0]),0);
        for(int i=0;i<ret;i++)
        {
            int fd = all[i].data.fd;
            if(fd==lfd)
            {
                int cfd=accept(lfd,(struct sockaddr*)&client_addr,&cli_len);
                if(cfd==-1)
                {
                    perror("accept error");
                    exit(1);
                }
                struct epoll_event temp;
                temp.events=EPOLLIN;
                temp.data.fd=cfd;
                epoll_ctl(epfd,EPOLL_CTL_ADD,cfd,&temp);
                printf("一个客户端连接，IP：%s,端口：%d\n",inet_ntop(AF_INET,&client_addr.sin_addr.s_addr,ip,sizeof(ip)),ntohs(client_addr.sin_port));
            }
            else
            {
                if(!all[i].events&EPOLLERR)
                {
                    continue;
                }
                char buf[1024]={0};
                int len=read(fd,buf,sizeof(buf));
                if(len==-1)
                {
                    perror("recv error");
                    exit(1);
                }
                else if(len==0)
                {
                    printf("一个客户端断开连接-----IP:%s,端口:%d\n",inet_ntop(AF_INET,&client_addr.sin_addr.s_addr,ip,sizeof(ip)),ntohs(client_addr.sin_port));
                    close(fd);
                    epoll_ctl(epfd,EPOLL_CTL_DEL,fd,NULL);
                }
                else
                {
                    printf("收到的数据:%s",buf);
                    write(fd,reply,sizeof(reply)/sizeof(reply[0]));
                }
            }
        }
    }
    close(lfd);
    return 0;
}

#ifndef NETWORK
#define NETWORK

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/time.h>

#include <netinet/in.h>
#include <netinet/ip.h>
#include <arpa/inet.h>

#include <unistd.h>
#include <fcntl.h>

#include <pthread.h>

#include <math.h>

#define IP_ADDR "192.168.2.222"
#define PORT 8500
#define DOMAIN "http://192.168.43.126:8080/?action=stream"

typedef struct message
{
    int sockfd;
    char flag;             // 是不是要夹的东西
    pthread_mutex_t mutex; // 锁
} message;

void *recv_msg(void *arg)
{
    puts("reveive message thread running");
    // 设置time
    long last_time = 0, now_time = 0;
    struct timeval time;
    // 初始化message
    message *msg = (message *)arg;
    char tmp;
    while (1)
    {
        int readByte = read(msg->sockfd, &tmp, sizeof(tmp));
        pthread_mutex_lock(&(msg->mutex));
        msg->flag = tmp;
        // 检测time
        // gettimeofday(&time, NULL);
        // now_time = time.tv_sec * 1000 + time.tv_usec / 1000;
        // if (now_time - last_time < 500) {
        //     // 如果两次接受数据的时间在0.5s之内，则认为是同一个物品，不做任何处理
        //     last_time = now_time;
        //     msg->flag = '2';
        //     pthread_mutex_unlock(&(msg->mutex));
        //     continue;
        // }
        // 读取结果
        if (readByte < 0)
        {
            perror("read error, network thread quit");
            msg->flag = 'q';        // 向控制线程通知退出
            pthread_mutex_unlock(&(msg->mutex));
            break;
        }
        else if (readByte == 0)
        {
            puts("server close, network thread quit");
            msg->flag = 'q';        // 向控制线程通知退出
            pthread_mutex_unlock(&(msg->mutex));
            break;
        }
        if (msg->flag == 'q')
        {
            puts("receive quit signal from server, network thread quit");
            pthread_mutex_unlock(&(msg->mutex));
            break;
        }
        pthread_mutex_unlock(&(msg->mutex));
    }
    pthread_exit(NULL);
}

// 建立tcp连接，返回sockfd
int create_sock()
{
    // 创建socket
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1)
    {
        perror("create socket failed");
        return -1;
    }

    // 创建连接
    struct sockaddr_in srvaddr;
    memset(&srvaddr, 0, sizeof(srvaddr));
    srvaddr.sin_family = AF_INET;
    srvaddr.sin_port = htons(PORT);
    srvaddr.sin_addr.s_addr = inet_addr(IP_ADDR);

    // 连接server
    puts("connecting...");
    if (0 > connect(sockfd, (struct sockaddr *)&srvaddr, sizeof(srvaddr)))
    {
        perror("connect to server failed");
        return -1;
    }
    puts("connect to server success");

    // 发送域名
    int tmp = write(sockfd, DOMAIN, strlen(DOMAIN));

    // 返回
    return sockfd;
}

#endif

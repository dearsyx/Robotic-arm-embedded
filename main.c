#include <stdio.h>
#include <pthread.h>

#include "./include/network.h"
#include "./include/control.h"

// 需要管理员权限

int main()
{
    // 创建连接
    int sockfd = create_sock();
    if (sockfd < 0)
    {
        perror("create socket failed");
        return -1;
    }

    // 初始化message
    pthread_mutex_t mutex;
    message msg = {
        sockfd : sockfd,
        flag : '2',
        mutex : mutex,
    };
    
    // 启动通信线程
    pthread_t sock_tid;
    pthread_create(&sock_tid, NULL, recv_msg, &msg);

    // 连接机械臂
    ARM_Connect();

    // 启动机械臂控制线程
    pthread_t control_tid;
    pthread_create(&control_tid, NULL, control_arm, &msg);

    pthread_join(sock_tid, NULL);
    pthread_join(control_tid, NULL);
    return 0;
}

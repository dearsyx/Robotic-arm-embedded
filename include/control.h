#ifndef CONTROL
#define CONTROL

#include "network.h"
#include "arm.h"
#include "action.h"

#include <stdio.h>
#include <pthread.h>

int fd;

void *control_arm(void *arg)
{
    message *msg = (message *)arg;
    puts("arm control thread running");
    // 机械臂复位
    turn_origin(fd);
    puts("arm set origin");
    while (1)
    {
        if (msg->flag == '1')
        {
            puts("receive positive signal from server");
            pthread_mutex_lock(&(msg->mutex));
            // 下移并夹住
            turn_down(fd);
            // 抬起，右移，松开
            turn_right(fd);
            // 回正
            turn_center(fd);
            msg->flag = '2';
            pthread_mutex_unlock(&(msg->mutex));
        }
        else if (msg->flag == '0')
        {
            puts("receive negative signal from server");
            pthread_mutex_lock(&(msg->mutex));
            msg->flag = '2';
            // 下移并夹住
            turn_down(fd);
            // 抬起，左移，松开
            turn_left(fd);
            // 回正
            turn_center(fd);
            msg->flag = '2';
            pthread_mutex_unlock(&(msg->mutex));
        }
        else if (msg->flag == 'q')
        {
            puts("receive quit signal, control thread quit");
            close(fd);
            break;
        }
        else
        {
            sleep(0.1);
            continue;
        }
    }
    pthread_exit(NULL);
}

void ARM_Connect()
{
    fd = open_dev();
    return;
}

#endif
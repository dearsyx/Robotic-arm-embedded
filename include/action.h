#ifndef ACTION
#define ACTION

#include "network.h"
#include "arm.h"

#include <stdio.h>
#include <pthread.h>

// 复位
unsigned char origin[7][5] = {
    {0xff, 0x02, 0x04, 0x1f, 0x04},
    {0xff, 0x02, 0x05, 0xdc, 0x05},
    {0xff, 0x02, 0x06, 0xdc, 0x05},
    {0xff, 0x02, 0x07, 0x20, 0x03},
    {0xff, 0x02, 0x08, 0xdc, 0x05},
    {0xff, 0x02, 0x09, 0x29, 0x06},
};

void turn_origin(int fd)
{
    // 底盘复位
    write(fd, origin[5], sizeof(origin[5]));
    sleep(4);
    // 复位
    int i;
    for (i = 4; i >= 0; i--)
    {
        if (0 > write(fd, origin[i], sizeof(origin[i])))
        {
            perror("sent origin order failed");
        }
        sleep(2.7);
    }
}

// 下移，并夹住
unsigned char down[2][5] = {
    {0xff, 0x02, 0x06, 0x78, 0x03},
    {0xff, 0x02, 0x04, 0xd0, 0x06},
};
void turn_down(int fd) {
    int i;
    for (i = 0; i < 2; i++) {
        write(fd, down[i], sizeof(down[i]));
        sleep(2.5);
    }
}

// 底盘转到中间
unsigned char center[5] = {0xff, 0x02, 0x09, 0x29, 0x06};
void turn_center(int fd)
{
    write(fd, center, sizeof(center));
    sleep(3);
}

// 转到右边
unsigned char right[3][5] = {
    {0xff, 0x02, 0x06, 0xdc, 0x05},
    {0xff, 0x02, 0x09, 0xae, 0x08},
    {0xff, 0x02, 0x04, 0x1f, 0x04},
};
void turn_right(int fd)
{
    /*
    // 现在夹子已经夹紧，需要移动到左边放下
    // 1.抬起机械臂
    // 2.机械臂向左移动
    // 3.延时1s，松开夹子
    */
    int i;
    for (i = 0; i < 3; i++)
    {
        write(fd, right[i], sizeof(right[i]));
        sleep(2);
    }
}

// 转到左边
unsigned char left[3][5] = {
    {0xff, 0x02, 0x06, 0xdc, 0x05},
    {0xff, 0x02, 0x09, 0x78, 0x03},
    {0xff, 0x02, 0x04, 0x1f, 0x04},
};
void turn_left(int fd)
{
    /*
    // 现在夹子已经夹紧，需要移动到右边放下
    // 1.抬起机械臂
    // 2.机械臂向右移动
    // 3.延时1s，松开夹子
    */
    int i;
    for (i = 0; i < 3; i++)
    {
        write(fd, left[i], sizeof(left[i]));
        sleep(2);
    }
}

#endif
#ifndef ARM
#define ARM

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <errno.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <arpa/inet.h>
#include <string.h>
#include <pthread.h>

#define FALSE -1
#define TRUE 0
#define DEV_DIR "/dev/ttyUSB0"

int UART0_Set(int fd, int speed, int flow_ctrl, int databits, int stopbits, int parity);
int UART0_Init(int fd, int speed, int flow_ctrl, int databits, int stopbits, int parity);

struct termio
{
    unsigned short c_iflag; /* 输入模式标志 */
    unsigned short c_oflag; /* 输出模式标志 */
    unsigned short c_cflag; /* 控制模式标志*/
    unsigned short c_lflag; /* local mode flags */
    unsigned char c_line;   /* line discipline */
};

// 打开文件描述符
int UART0_Open(int fd, char *port)
{
    fd = open(port, O_RDWR | O_NOCTTY | O_NDELAY);
    if (fd < 0)
    {
        perror("Can't Open Serial Port");
        return (FALSE);
    }
    //恢复串口为阻塞状态
    if (fcntl(fd, F_SETFL, 0) < 0)
    {
        printf("fcntl failed!\n");
        return (FALSE);
    }
    else
    {
        fcntl(fd, F_SETFL, 0);
        // printf("fcntl=%d\n", fcntl(fd, F_SETFL, 0));
    }
    //测试是否为终端设备
    if (0 == isatty(STDIN_FILENO))
    {
        printf("standard input is not a terminal device\n");
        return (FALSE);
    }
    else
    {
        // printf("isatty success!\n");
    }
    // printf("fd->open=%d\n", fd);
    return fd;
}

// 设置连接参数
int UART0_Set(int fd, int speed, int flow_ctrl, int databits, int stopbits, int parity)
{

    int i;
    int status;
    int speed_arr[] = {B115200, B19200, B9600, B4800, B2400, B1200, B300};
    int name_arr[] = {115200, 19200, 9600, 4800, 2400, 1200, 300};

    struct termios options;

    /*  tcgetattr(fd,&options)得到与fd指向对象的相关参数，并将它们保存于options,该函数还可以测试配置是否正确，
        该串口是否可用等。若调用成功，函数返回值为0，若调用失败，函数返回值为1.  */
    if (tcgetattr(fd, &options) != 0)
    {
        perror("SetupSerial 1");
        return (FALSE);
    }

    //设置串口输入波特率和输出波特率
    for (i = 0; i < sizeof(speed_arr) / sizeof(int); i++)
    {
        if (speed == name_arr[i])
        {
            cfsetispeed(&options, speed_arr[i]);
            cfsetospeed(&options, speed_arr[i]);
        }
    }

    //修改控制模式，保证程序不会占用串口
    options.c_cflag |= CLOCAL;
    //修改控制模式，使得能够从串口中读取输入数据
    options.c_cflag |= CREAD;

    //设置数据流控制
    switch (flow_ctrl)
    {

    case 0: //不使用流控制
        options.c_cflag &= ~CRTSCTS;
        break;

    case 1: //使用硬件流控制
        options.c_cflag |= CRTSCTS;
        break;
    case 2: //使用软件流控制
        options.c_cflag |= IXON | IXOFF | IXANY;
        break;
    }
    //设置数据位
    //屏蔽其他标志位
    options.c_cflag &= ~CSIZE;
    switch (databits)
    {
    case 5:
        options.c_cflag |= CS5;
        break;
    case 6:
        options.c_cflag |= CS6;
        break;
    case 7:
        options.c_cflag |= CS7;
        break;
    case 8:
        options.c_cflag |= CS8;
        break;
    default:
        fprintf(stderr, "Unsupported data size\n");
        return (FALSE);
    }
    //设置校验位
    switch (parity)
    {
    case 'n':
    case 'N': //无奇偶校验位。
        options.c_cflag &= ~PARENB;
        options.c_iflag &= ~INPCK;
        break;
    case 'o':
    case 'O': //设置为奇校验
        options.c_cflag |= (PARODD | PARENB);
        options.c_iflag |= INPCK;
        break;
    case 'e':
    case 'E': //设置为偶校验
        options.c_cflag |= PARENB;
        options.c_cflag &= ~PARODD;
        options.c_iflag |= INPCK;
        break;
    case 's':
    case 'S': //设置为空格
        options.c_cflag &= ~PARENB;
        options.c_cflag &= ~CSTOPB;
        break;
    default:
        fprintf(stderr, "Unsupported parity\n");
        return (FALSE);
    }
    // 设置停止位
    switch (stopbits)
    {
    case 1:
        options.c_cflag &= ~CSTOPB;
        break;
    case 2:
        options.c_cflag |= CSTOPB;
        break;
    default:
        fprintf(stderr, "Unsupported stop bits\n");
        return (FALSE);
    }

    //修改输出模式，原始数据输出
    options.c_oflag &= ~OPOST;

    options.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);
    //options.c_lflag &= ~(ISIG | ICANON);

    //设置等待时间和最小接收字符
    options.c_cc[VTIME] = 1; /* 读取一个字符等待1*(1/10)s */
    options.c_cc[VMIN] = 1;  /* 读取字符的最少个数为1 */

    //如果发生数据溢出，接收数据，但是不再读取 刷新收到的数据但是不读
    tcflush(fd, TCIFLUSH);

    //激活配置 (将修改后的termios数据设置到串口中）
    if (tcsetattr(fd, TCSANOW, &options) != 0)
    {
        perror("com set error!\n");
        return (FALSE);
    }
    return (TRUE);
}

// 连接初始化
int UART0_Init(int fd, int speed, int flow_ctrl, int databits, int stopbits, int parity)
{
    int err;
    //设置串口数据帧格式
    if (UART0_Set(fd, 115200, 0, 8, 1, 'N') == FALSE)
    {
        return FALSE;
    }
    else
    {
        return TRUE;
    }
}

// 连接机械臂，返回文件描述符
int open_dev()
{
    int fd;
    fd = UART0_Open(fd, "/dev/ttyUSB0");
    int err;
    err = UART0_Init(fd, 115200, 0, 8, 1, 'N');
    // printf("Set Port Exactly!\n");
    return fd;
}

#endif
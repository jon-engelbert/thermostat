#include "si705X.h"
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <fcntl.h>
//#include <sys/mman.h>
#include <unistd.h>
#include <linux/i2c.h>
#include <linux/i2c-dev.h>
#include <errno.h>
#include <string.h>
#include <sys/ioctl.h>

#define DEBUG 0

int bus_no = -1;
int addr = -1;
char device_path[12];
int file_desc = -1;

void startup(int in_bus_no, int in_addr)
{
    bus_no = in_bus_no & 0x07;
    addr = in_addr & 0x7f;
    
    // TODO: check input parameters
    
    sprintf(device_path, "/dev/i2c-%c", bus_no + '0');
#if DEBUG
    printf("%s(%d) DEVICE PATH = %s\n", __FILE__, __LINE__, device_path);
#endif

    file_desc = open(device_path, O_RDWR);
    if (file_desc < 0)
    {
        printf("%s(%d): cannot open I2C device: %s\n", __FILE__, __LINE__, strerror(errno));
        return;
    }
    if (ioctl(file_desc, I2C_SLAVE, addr) < 0)
    {
        printf("%s(%d): cannot ioctl I2C bus: %s\n", __FILE__, __LINE__, strerror(errno));
        close(file_desc);
        file_desc = -1;
        return;
    }
    close(file_desc);
    file_desc = -1;
}

void reset(void)
{
    struct i2c_rdwr_ioctl_data msg_rdwr;
    struct i2c_msg i2cmsg;
    unsigned char buf[1];
    
    buf[0] = 0xFE;

    msg_rdwr.msgs = &i2cmsg;
    msg_rdwr.nmsgs = 1;
    i2cmsg.addr  = addr;
    i2cmsg.flags = 0;
    i2cmsg.len   = 1;
    i2cmsg.buf   = buf;
    
    file_desc = open(device_path, O_RDWR);
    if (file_desc < 0)
    {
        printf("%s(%d): cannot open I2C device: %s\n", __FILE__, __LINE__, strerror(errno));
        return;
    }
    if (ioctl(file_desc, I2C_RDWR, &msg_rdwr) < 0)
    {
        printf("%s(%d): write_to_device(): %s\n", __FILE__, __LINE__, strerror(errno));
        close(file_desc);
        return;
    }
    close(file_desc);

    // per datasheet, we may not be ready for (max) 15 ms    
    DelayMicrosecondsNoSleep(15000); // 15 milliseconds
    
    return;
}

int get_revision(void)
{
    struct i2c_rdwr_ioctl_data msg_rdwr;
    struct i2c_msg i2cmsg;
    unsigned char buf[2];
    
    buf[0] = 0x84;
    buf[1] = 0xB8;

    msg_rdwr.msgs = &i2cmsg;
    msg_rdwr.nmsgs = 1;
    i2cmsg.addr  = addr;
    i2cmsg.flags = 0;
    i2cmsg.len   = 2;
    i2cmsg.buf   = buf;
    
    file_desc = open(device_path, O_RDWR);
    if (file_desc < 0)
    {
        printf("%s(%d): cannot open I2C device: %s\n", __FILE__, __LINE__, strerror(errno));
        return -1;
    }
    // write command
    if (ioctl(file_desc, I2C_RDWR, &msg_rdwr) < 0)
    {
        printf("%s(%d): write_to_device(): %s\n", __FILE__, __LINE__, strerror(errno));
        close(file_desc);
        return -1;
    }
    
    int rev;
    
    i2cmsg.flags = I2C_M_RD;
    i2cmsg.len   = 1;
  
    // read rev
    if (ioctl(file_desc, I2C_RDWR, &msg_rdwr) < 0)
    {
        printf("%s(%d): read_from_device(): %s\n", __FILE__, __LINE__, strerror(errno));
        close(file_desc);
        return -1;
    }
    rev = buf[0];
    close(file_desc);
    
    return rev;
}

unsigned long long get_ESN(void)
{
    struct i2c_rdwr_ioctl_data msg_rdwr;
    struct i2c_msg i2cmsg;
    unsigned char buf[8];
    
    buf[0] = 0xFA;
    buf[1] = 0x0F;

    msg_rdwr.msgs = &i2cmsg;
    msg_rdwr.nmsgs = 1;
    i2cmsg.addr  = addr;
    i2cmsg.flags = 0;
    i2cmsg.len   = 2;
    i2cmsg.buf   = buf;
    
    file_desc = open(device_path, O_RDWR);
    if (file_desc < 0)
    {
        printf("%s(%d): cannot open I2C device: %s\n", __FILE__, __LINE__, strerror(errno));
        return 0LLU;
    }
    // write command
    if (ioctl(file_desc, I2C_RDWR, &msg_rdwr) < 0)
    {
        printf("%s(%d): write_to_device(): %s\n", __FILE__, __LINE__, strerror(errno));
        close(file_desc);
        return 0LLU;
    }
    
    unsigned long long rev;
    unsigned long long t64;
    
    i2cmsg.flags = I2C_M_RD;
    i2cmsg.len   = 8;
  
    // read rev
    if (ioctl(file_desc, I2C_RDWR, &msg_rdwr) < 0)
    {
        printf("%s(%d): read_from_device(): %s\n", __FILE__, __LINE__, strerror(errno));
        close(file_desc);
        return 0LLU;
    }
    
#if DEBUG
    int i;
    for (i=0; i < 8; i++)
       printf("A%d = %02x\n", i, buf[i]);
#endif
    
    rev = 0LLU;
    t64 = buf[0] & 0xFF;
    rev = rev | t64<<56;
    t64 = buf[2] & 0xFF;
    rev = rev | t64<<48;
    t64 = buf[4] & 0xFF;
    rev = rev | t64<<40;
    t64 = buf[6] & 0xFF;
    rev = rev | t64<<32;

    buf[0] = 0xFC;
    buf[1] = 0xC9;
    i2cmsg.flags = 0;
    i2cmsg.len   = 2;
    // write command
    if (ioctl(file_desc, I2C_RDWR, &msg_rdwr) < 0)
    {
        printf("%s(%d): write_to_device(): %s\n", __FILE__, __LINE__, strerror(errno));
        close(file_desc);
        return 0LLU;
    }
    
    i2cmsg.flags = I2C_M_RD;
    i2cmsg.len   = 6;
  
    // read rev
    if (ioctl(file_desc, I2C_RDWR, &msg_rdwr) < 0)
    {
        printf("%s(%d): read_from_device(): %s\n", __FILE__, __LINE__, strerror(errno));
        close(file_desc);
        return 0LLU;
    }
    close(file_desc);

#if DEBUG
    for (i=0; i < 6; i++)
       printf("B%d = %02x\n", i, buf[i]);
#endif

    t64 = buf[0] & 0xFF;
    rev = rev | t64<<24;
    t64 = buf[1] & 0xFF;
    rev = rev | t64<<16;
    t64 = buf[3] & 0xFF;
    rev = rev | t64<<8;
    t64 = buf[4] & 0xFF;
    rev = rev | t64;
    
    return rev;
}

unsigned char get_reg1(void)
{
    struct i2c_rdwr_ioctl_data msg_rdwr;
    struct i2c_msg i2cmsg;
    unsigned char buf[1];
    
    buf[0] = 0xE7;

    msg_rdwr.msgs = &i2cmsg;
    msg_rdwr.nmsgs = 1;
    i2cmsg.addr  = addr;
    i2cmsg.flags = 0;
    i2cmsg.len   = 1;
    i2cmsg.buf   = buf;
    
    file_desc = open(device_path, O_RDWR);
    if (file_desc < 0)
    {
        printf("%s(%d): cannot open I2C device: %s\n", __FILE__, __LINE__, strerror(errno));
        return 0;
    }
    // write command
    if (ioctl(file_desc, I2C_RDWR, &msg_rdwr) < 0)
    {
        printf("%s(%d): write_to_device(): %s\n", __FILE__, __LINE__, strerror(errno));
        return 0;
    }
    
    int reg1;
    
    i2cmsg.flags = I2C_M_RD;
    i2cmsg.len   = 1;
  
    // read reg1
    if (ioctl(file_desc, I2C_RDWR, &msg_rdwr) < 0)
    {
        printf("%s(%d): read_from_device(): %s\n", __FILE__, __LINE__, strerror(errno));
        close(file_desc);
        return 0;
    }
    reg1 = buf[0];
    close(file_desc);
    
    return reg1;
}

int set_reg1(unsigned char data)
{
    struct i2c_rdwr_ioctl_data msg_rdwr;
    struct i2c_msg i2cmsg;
    unsigned char buf[2];
    
    buf[0] = 0xE6;
    buf[1] = data;

    msg_rdwr.msgs = &i2cmsg;
    msg_rdwr.nmsgs = 1;
    i2cmsg.addr  = addr;
    i2cmsg.flags = 0;
    i2cmsg.len   = 2;
    i2cmsg.buf   = buf;
    
    file_desc = open(device_path, O_RDWR);
    if (file_desc < 0)
    {
        printf("%s(%d): cannot open I2C device: %s\n", __FILE__, __LINE__, strerror(errno));
        return -1;
    }
    // write command
    if (ioctl(file_desc, I2C_RDWR, &msg_rdwr) < 0)
    {
        printf("%s(%d): write_to_device(): %s\n", __FILE__, __LINE__, strerror(errno));
        close(file_desc);
        return -1;
    }
    close(file_desc);
    
    return 0;
}

int get_temp_code(void)
{
    struct i2c_rdwr_ioctl_data msg_rdwr;
    struct i2c_msg i2cmsg;
    unsigned char buf[3];
    
    buf[0] = 0xF3;

    msg_rdwr.msgs = &i2cmsg;
    msg_rdwr.nmsgs = 1;
    i2cmsg.addr  = addr;
    i2cmsg.flags = 0;
    i2cmsg.len   = 1;
    i2cmsg.buf   = buf;
    
    file_desc = open(device_path, O_RDWR);
    if (file_desc < 0)
    {
        printf("%s(%d): cannot open I2C device: %s\n", __FILE__, __LINE__, strerror(errno));
        return -1;
    }
    // write command
    if (ioctl(file_desc, I2C_RDWR, &msg_rdwr) < 0)
    {
        printf("%s(%d): write_to_device(): %s\n", __FILE__, __LINE__, strerror(errno));
        close(file_desc);
        return -1;
    }
    
    i2cmsg.flags = I2C_M_RD;
    i2cmsg.len   = 3;
  
    // LOOPY
    unsigned char okay = 0;
    long start_time;
    long time_difference;
    long delay_ns = 1000000000L;
    struct timespec gettime_now;
    
    clock_gettime(CLOCK_REALTIME, &gettime_now);
    start_time = gettime_now.tv_nsec;		//Get nS value
    while (1)
    {
        // read temperature
        if (ioctl(file_desc, I2C_RDWR, &msg_rdwr) >= 0)
        {
            okay = 1;
            break;
        }
    
        clock_gettime(CLOCK_REALTIME, &gettime_now);
        time_difference = gettime_now.tv_nsec - start_time;
        if (time_difference < 0)
            time_difference += 1000000000L;				//(Rolls over every 1 second)
        if (time_difference > delay_ns)		//Delay for # nS
            break;
    }

    close(file_desc);
    
    if (!okay)
    {
        printf("%s(%d): read_from_device(): %s\n", __FILE__, __LINE__, strerror(errno));
        return -1;
    }

    int temp_code;
    temp_code = buf[0]<<8 | buf[1];
    
    return temp_code;
}

double get_tempC(void)
{
    int temp_code = get_temp_code();
    double whammy = ((175.72D * ((double) temp_code)) / 65536.0D) - 46.85D;
    
    return whammy;
}

double get_tempF(void)
{
    int temp_code = get_temp_code();
    double whammy = ((175.72D * ((double) temp_code)) / 65536.0D) - 46.85D;
    whammy = (whammy * 1.8D) + 32.0D; // convert C to F
    
    return whammy;
}

//*****************************************************
//*****************************************************
//********** DELAY FOR # uS WITHOUT SLEEPING **********
//*****************************************************
//*****************************************************
//Using delayMicroseconds lets the linux scheduler decide to jump to another process.  Using this function avoids letting the
//scheduler know we are pausing and provides much faster operation if you are needing to use lots of delays.
void DelayMicrosecondsNoSleep (int delay_us)
{
    long int start_time;
    long int time_difference;
    struct timespec gettime_now;
    
    clock_gettime(CLOCK_REALTIME, &gettime_now);
    start_time = gettime_now.tv_nsec;		//Get nS value
    while (1)
    {
        clock_gettime(CLOCK_REALTIME, &gettime_now);
        time_difference = gettime_now.tv_nsec - start_time;
        if (time_difference < 0)
            time_difference += 1000000000;				//(Rolls over every 1 second)
        if (time_difference > (delay_us * 1000))		//Delay for # nS
            break;
    }
}

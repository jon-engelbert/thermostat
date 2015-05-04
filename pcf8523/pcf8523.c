#include "pcf8523.h"
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

#define DEBUG 1

int bus_no = -1;
int addr = -1;
char device_path[12];
int file_desc = -1;
unsigned char reg[19];

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
    unsigned char buf[2];
    
    buf[0] = 0x00;
    buf[1] = 0x58;

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
        return;
    }
    if (ioctl(file_desc, I2C_RDWR, &msg_rdwr) < 0)
    {
        printf("%s(%d): write_to_device(): %s\n", __FILE__, __LINE__, strerror(errno));
        close(file_desc);
        return;
    }
    close(file_desc);

    DelayMicrosecondsNoSleep(100000); // 0.1 seconds
    
    return;
}

unsigned char fetch_reg(int regnum)
{
    struct i2c_rdwr_ioctl_data msg_rdwr;
    struct i2c_msg i2cmsg;
    unsigned char buf[1];
    
    buf[0] = regnum & 0x1F;

    msg_rdwr.msgs = &i2cmsg;
    msg_rdwr.nmsgs = 1;
    i2cmsg.addr  = addr;
    i2cmsg.buf   = buf;
    i2cmsg.flags = 0;
    i2cmsg.len   = 1;
    
    file_desc = open(device_path, O_RDWR);
    if (file_desc < 0)
    {
        printf("%s(%d): cannot open I2C device: %s\n", __FILE__, __LINE__, strerror(errno));
        return (unsigned char *)0;
    }
    if (ioctl(file_desc, I2C_RDWR, &msg_rdwr) < 0)
    {
        printf("%s(%d): write_to_device(): %s\n", __FILE__, __LINE__, strerror(errno));
        close(file_desc);
        return (unsigned char *)0;
    }

    i2cmsg.flags = I2C_M_RD;
  
    // read
    if (ioctl(file_desc, I2C_RDWR, &msg_rdwr) < 0)
    {
        printf("%s(%d): read: %s\n", __FILE__, __LINE__, strerror(errno));
        close(file_desc);
        return (unsigned char *)0;
    }

    close(file_desc);
    
    return buf[0];
}

int set_reg(int regnum, int value)
{
    struct i2c_rdwr_ioctl_data msg_rdwr;
    struct i2c_msg i2cmsg;
    unsigned char buf[2];
    
    buf[0] = regnum & 0x1F;
    buf[1] = value & 0xFF;

    msg_rdwr.msgs = &i2cmsg;
    msg_rdwr.nmsgs = 1;
    i2cmsg.addr  = addr;
    i2cmsg.buf   = buf;
    i2cmsg.flags = 0;
    i2cmsg.len   = 2;
    
    file_desc = open(device_path, O_RDWR);
    if (file_desc < 0)
    {
        printf("%s(%d): cannot open I2C device: %s\n", __FILE__, __LINE__, strerror(errno));
        return -1;
    }
    if (ioctl(file_desc, I2C_RDWR, &msg_rdwr) < 0)
    {
        printf("%s(%d): write_to_device(): %s\n", __FILE__, __LINE__, strerror(errno));
        close(file_desc);
        return -1;
    }

    close(file_desc);
    
    return 0;
}

int fetch_chip_time(unsigned char* yr, unsigned char* mo, unsigned char* dow, unsigned char* dy,
               unsigned char* ho, unsigned char* mi, unsigned char* sc)
{
    struct i2c_rdwr_ioctl_data msg_rdwr;
    struct i2c_msg i2cmsg;
    unsigned char buf[7];
    
    // get 7 registers from 0x03 to 0x09
    buf[0] = 0x03;

    msg_rdwr.msgs = &i2cmsg;
    msg_rdwr.nmsgs = 1;
    i2cmsg.addr  = addr;
    i2cmsg.buf   = buf;
    i2cmsg.flags = 0;
    i2cmsg.len   = 1;
    
    file_desc = open(device_path, O_RDWR);
    if (file_desc < 0)
    {
        printf("%s(%d): cannot open I2C device: %s\n", __FILE__, __LINE__, strerror(errno));
        return -1;
    }
    if (ioctl(file_desc, I2C_RDWR, &msg_rdwr) < 0)
    {
        printf("%s(%d): write_to_device(): %s\n", __FILE__, __LINE__, strerror(errno));
        close(file_desc);
        return -1;
    }

    i2cmsg.flags = I2C_M_RD;
    i2cmsg.len   = 7;
  
    // read
    if (ioctl(file_desc, I2C_RDWR, &msg_rdwr) < 0)
    {
        printf("%s(%d): read: %s\n", __FILE__, __LINE__, strerror(errno));
        close(file_desc);
        return -1;
    }

    close(file_desc);
    
#if DEBUG
    int i;
    for (i=0; i < 7; i++)
       printf("reg[%02x] = %02x\n", i+3, buf[i]);
#endif

    unsigned char z;
    z = buf[0] & 0x7F;
    *sc = (z>>4) * 10 + (z & 0xF);
    z = buf[1] & 0x7F;
    *mi = (z>>4) * 10 + (z & 0xF);
    z = buf[2] & 0x3F;
    *ho = (z>>4) * 10 + (z & 0xF);
    z = buf[3] & 0x3F;
    *dy = (z>>4) * 10 + (z & 0xF);
    z = buf[4] & 0x7;
    *dow = z;
    z = buf[5] & 0x1F;
    *mo = (z>>4) * 10 + (z & 0xF);
    z = buf[6];
    *yr = (z>>4) * 10 + (z & 0xF);
    
    return 0;
}

int set_chip_time(unsigned char yr, unsigned char mo, unsigned char dow, unsigned char dy,
               unsigned char ho, unsigned char mi, unsigned char sc)
{
    struct i2c_rdwr_ioctl_data msg_rdwr;
    struct i2c_msg i2cmsg;
    unsigned char buf[8];
    
    buf[0] = 0x03;
    buf[1] = ((sc/10)<<4) + (sc%10);
    buf[2] = ((mi/10)<<4) + (mi%10);
    buf[3] = ((ho/10)<<4) + (ho%10);
    buf[4] = ((dy/10)<<4) + (dy%10);
    buf[5] = (dow%10);
    buf[6] = ((mo/10)<<4) + (mo%10);
    buf[7] = ((yr/10)<<4) + (yr%10);

    msg_rdwr.msgs = &i2cmsg;
    msg_rdwr.nmsgs = 1;
    i2cmsg.addr  = addr;
    i2cmsg.buf   = buf;
    i2cmsg.flags = 0;
    i2cmsg.len   = 8;
    
    file_desc = open(device_path, O_RDWR);
    if (file_desc < 0)
    {
        printf("%s(%d): cannot open I2C device: %s\n", __FILE__, __LINE__, strerror(errno));
        return -1;
    }
    if (ioctl(file_desc, I2C_RDWR, &msg_rdwr) < 0)
    {
        printf("%s(%d): write_to_device(): %s\n", __FILE__, __LINE__, strerror(errno));
        close(file_desc);
        return -1;
    }

    close(file_desc);
    
    return 0;
}

char check_battery(void)
{
    struct i2c_rdwr_ioctl_data msg_rdwr;
    struct i2c_msg i2cmsg;
    unsigned char buf[1];
    
    buf[0] = 0x2;

    msg_rdwr.msgs = &i2cmsg;
    msg_rdwr.nmsgs = 1;
    i2cmsg.addr  = addr;
    i2cmsg.buf   = buf;
    i2cmsg.flags = 0;
    i2cmsg.len   = 1;
    
    file_desc = open(device_path, O_RDWR);
    if (file_desc < 0)
    {
        printf("%s(%d): cannot open I2C device: %s\n", __FILE__, __LINE__, strerror(errno));
        return -1;
    }
    if (ioctl(file_desc, I2C_RDWR, &msg_rdwr) < 0)
    {
        printf("%s(%d): write_to_device(): %s\n", __FILE__, __LINE__, strerror(errno));
        close(file_desc);
        return -1;
    }

    i2cmsg.flags = I2C_M_RD;
  
    // read
    if (ioctl(file_desc, I2C_RDWR, &msg_rdwr) < 0)
    {
        printf("%s(%d): read: %s\n", __FILE__, __LINE__, strerror(errno));
        close(file_desc);
        return -1;
    }

    close(file_desc);

    char ok;    
    ok = 1 - ((buf[0] & 0x04)>>2);
    
    return ok;
}

char set_systime_from_chip(void)
{
    struct i2c_rdwr_ioctl_data msg_rdwr;
    struct i2c_msg i2cmsg;
    unsigned char buf[7];
    
    // get 7 registers from 0x03 to 0x09
    buf[0] = 0x03;

    msg_rdwr.msgs = &i2cmsg;
    msg_rdwr.nmsgs = 1;
    i2cmsg.addr  = addr;
    i2cmsg.buf   = buf;
    i2cmsg.flags = 0;
    i2cmsg.len   = 1;
    
    file_desc = open(device_path, O_RDWR);
    if (file_desc < 0)
    {
        printf("%s(%d): cannot open I2C device: %s\n", __FILE__, __LINE__, strerror(errno));
        return -1;
    }
    if (ioctl(file_desc, I2C_RDWR, &msg_rdwr) < 0)
    {
        printf("%s(%d): write_to_device(): %s\n", __FILE__, __LINE__, strerror(errno));
        close(file_desc);
        return -1;
    }

    i2cmsg.flags = I2C_M_RD;
    i2cmsg.len   = 7;
  
    // read
    if (ioctl(file_desc, I2C_RDWR, &msg_rdwr) < 0)
    {
        printf("%s(%d): read: %s\n", __FILE__, __LINE__, strerror(errno));
        close(file_desc);
        return -1;
    }

    close(file_desc);
    
#if DEBUG
    int i;
    for (i=0; i < 7; i++)
       printf("reg[%02x] = %02x\n", i+3, buf[i]);
#endif

    unsigned char yr,mo,dy,ho,mi,sc;
    
    unsigned char z;
    z = buf[0] & 0x7F;
    sc = (z>>4) * 10 + (z & 0xF);
    z = buf[1] & 0x7F;
    mi = (z>>4) * 10 + (z & 0xF);
    z = buf[2] & 0x3F;
    ho = (z>>4) * 10 + (z & 0xF);
    z = buf[3] & 0x3F;
    dy = (z>>4) * 10 + (z & 0xF);
    z = buf[5] & 0x1F;
    mo = (z>>4) * 10 + (z & 0xF);
    z = buf[6];
    yr = (z>>4) * 10 + (z & 0xF);

    time_t rawtime;
    struct tm * timeinfo;
    time ( &rawtime );
    timeinfo = localtime ( &rawtime );
    timeinfo->tm_year = yr + 100; // 2000 - 1900
    timeinfo->tm_mon = mo - 1;
    timeinfo->tm_mday = dy;
    timeinfo->tm_hour = ho;
    timeinfo->tm_min = mi;
    timeinfo->tm_sec = sc;
    timeinfo->tm_isdst = -1; // figure out DST
    rawtime = mktime ( timeinfo );
    
    if (rawtime == -1)
    {
        printf("%s(%d): time conversion error: %s\n", __FILE__, __LINE__, strerror(errno));
        return -1;
    }
    
    struct timespec when;
    when.tv_sec = rawtime;
    when.tv_nsec = 0;
    if (clock_settime(CLOCK_REALTIME, &when) < 0)
    {
        printf("%s(%d): clock_settime: %s\n", __FILE__, __LINE__, strerror(errno));
        return -1;
    }
    
    return 0;
}

char set_chip_to_systime(void)
{
    struct i2c_rdwr_ioctl_data msg_rdwr;
    struct i2c_msg i2cmsg;
    unsigned char buf[8];
    int z;
    time_t rawtime;
    struct tm* timeinfo;

    time ( &rawtime );
    timeinfo = localtime( &rawtime );

    buf[0] = 0x03;
    
    z = timeinfo->tm_sec;
    buf[1] = ((z/10)<<4) + (z%10);
    z = timeinfo->tm_min;
    buf[2] = ((z/10)<<4) + (z%10);
    z = timeinfo->tm_hour;
    buf[3] = ((z/10)<<4) + (z%10);
    z = timeinfo->tm_mday;
    buf[4] = ((z/10)<<4) + (z%10);
    z = timeinfo->tm_wday;
    buf[5] = (z%10);
    z = timeinfo->tm_mon + 1;
    buf[6] = ((z/10)<<4) + (z%10);
    z = timeinfo->tm_year - 100;
    buf[7] = ((z/10)<<4) + (z%10);

    msg_rdwr.msgs = &i2cmsg;
    msg_rdwr.nmsgs = 1;
    i2cmsg.addr  = addr;
    i2cmsg.buf   = buf;
    i2cmsg.flags = 0;
    i2cmsg.len   = 8;
    
    file_desc = open(device_path, O_RDWR);
    if (file_desc < 0)
    {
        printf("%s(%d): cannot open I2C device: %s\n", __FILE__, __LINE__, strerror(errno));
        return -1;
    }
    if (ioctl(file_desc, I2C_RDWR, &msg_rdwr) < 0)
    {
        printf("%s(%d): write_to_device(): %s\n", __FILE__, __LINE__, strerror(errno));
        close(file_desc);
        return -1;
    }

    close(file_desc);
    
    return 0;
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

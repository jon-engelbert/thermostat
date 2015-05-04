void startup(int bus_no, int addr);
void reset(void);
unsigned char fetch_reg(int regnum);
int set_reg(int regnum, int value);
int fetch_chip_time(unsigned char* yr, unsigned char* mo, unsigned char* dow, unsigned char* dy,
               unsigned char* ho, unsigned char* mi, unsigned char* sc);
int set_chip_time(unsigned char yr, unsigned char mo, unsigned char dow, unsigned char dy,
               unsigned char ho, unsigned char mi, unsigned char sc);
char check_battery(void);
char set_systime_from_chip(void);
char set_chip_to_systime(void);
void DelayMicrosecondsNoSleep (int delay_us);

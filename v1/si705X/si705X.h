void startup(int bus_no, int addr);
void reset(void);
int get_revision(void);
unsigned long long get_ESN(void);
unsigned char get_reg1(void);
int set_reg1(unsigned char data);
double get_tempC(void);
double get_tempF(void);
void DelayMicrosecondsNoSleep (int delay_us);
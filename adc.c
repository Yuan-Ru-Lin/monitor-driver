#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <linux/i2c-dev.h>
#include <linux/i2c.h>
#include <stdint.h>
#include <errno.h>
#include <math.h>

#define ADS1115_ADDRESS        0x48  // Default I2C address of ADS1115
#define ADS1115_CONFIG_REG     0x01
#define ADS1115_CONVERSION_REG 0x00

// These features are bundled in one byte
const int OS        = 0b1   << 15;
const int MUX       = 0b000 << 12;
const int PGA       = 0b001 <<  9;
const int MODE      = 0b0   <<  8;

// These features are bundled in one byte
const int DR        = 0b100 <<  5;
const int COMP_MODE = 0b0   <<  4;
const int COMP_POL  = 0b0   <<  3;
const int COMP_LAT  = 0b0   <<  2;
const int COMP_QUE  = 0b11;

// COMMAND1 needs to be sent as a byte so we shift right by 1 byte here
const int COMMAND1 = (OS | MUX | PGA | MODE) >> 8;
const int COMMAND2 = DR | COMP_MODE | COMP_POL | COMP_LAT | COMP_QUE;

const char CONFIGCODE[3] = {ADS1115_CONFIG_REG, COMMAND1, COMMAND2};
const char READCODE[1] = {ADS1115_CONVERSION_REG};

int setup(const char* path)
{
    int file;
    if ((file = open(path, O_RDWR)) < 0) {
        perror("Failed to open the bus");
        exit(1);
    }
    if (ioctl(file, I2C_SLAVE, ADS1115_ADDRESS) < 0) {
        perror("Failed to acquire bus access and/or talk to slave");
        exit(1);
    }
    return file;
}

void configurate(int file)
{
    write(file, CONFIGCODE, 3);  // 3 is just the length of CONFIGCODE
    sleep(1);
    write(file, READCODE, 1);    // 1 is just the length of READCODE
}

int convertADCOutputToInt16(char* data)
{
    int raw_adc = (data[0] << 8) | data[1];
    if (raw_adc > 32767) raw_adc -= 65535;
    return raw_adc;
}

int readADCOutputAsInt16From(int file)
{
    char data[2] = {0};
    if (read(file, data, 2) != 2) {
        perror("I/O error");
        exit(5);
    };
    return convertADCOutputToInt16(data);
}

int main()
{
    int file = setup("/dev/i2c-1");
    configurate(file);

    int value = 0;
    while (1) {
        value = readADCOutputAsInt16From(file);
        fprintf(stdout, "%d\n", value); fflush(stdout);
        usleep(10000);
    }

    return 0;
}
